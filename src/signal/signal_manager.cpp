#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"
#include "net/mqtt_manager.h"
#include <vector>
#include <string.h>
#include <cmath>

namespace SignalMgr {
    static std::vector<Signal> _signals;
    static const unsigned long REMOTE_TIMEOUT_MS = 15000; // 15 segundos

    Signal* getById(const char *id) {
        if (!id) return nullptr;
        for (auto &s : _signals) {
            if (strcmp(s.id, id) == 0) return &s;
        }
        return nullptr;
    }

    void addSignal(const Signal& s) {
        if (getById(s.id)) return;
        _signals.push_back(s);
    }

    // --- Lógica de Procesamiento (Escalado, Filtro, Inversión) ---
    void _applyLogic(Signal &s) {
        s.prev = s.value;
        float val = s.raw;

        // 1. Escalamiento Lineal con protección de división por cero
        if (s.calib.usaescala) {
            float rawDiff = s.calib.rawMax - s.calib.rawMin;
            if (std::abs(rawDiff) > 0.0001f) {
                val = ((val - s.calib.rawMin) * (s.calib.realMax - s.calib.realMin) / rawDiff) + s.calib.realMin;
            }
        }

        val += s.calib.offset;

        // 2. Filtro EMA (Exponential Moving Average)
        if (!s.calib.emaInit) {
            s.calib.emaValue = val;
            s.calib.emaInit = true;
        } else {
            // S_t = (α * Y_t) + ((1 - α) * S_{t-1})
            s.calib.emaValue = (s.calib.emaAlpha * val) + ((1.0f - s.calib.emaAlpha) * s.calib.emaValue);
        }
        val = s.calib.emaValue;

        // 3. Inversión Lógica/Analógica
        if (s.invertido) {
            if (s.kind == SignalKind::SENSOR_DIGITAL || s.kind == SignalKind::ACTUATOR_DIGITAL) {
                val = (val > 0.5f) ? 0.0f : 1.0f;
            } else {
                val = (s.calib.realMax - val + s.calib.realMin);
            }
        }

        s.value = val;
        s.lastUpdateMs = millis();
    }

    // --- Actualización para datos remotos (Inyectado por MQTT) ---
    void updateRemote(const char* id, float val, SignalQuality quality) {
        Signal* s = getById(id);
        if (s) {
            s->raw = val;
            s->quality = quality;
            s->valid = (quality == SignalQuality::GOOD);
            _applyLogic(*s); 
        }
    }

    // --- Watchdog de Red (P2P) ---
    void processWatchdog() {
        unsigned long now = millis();
        for (auto &s : _signals) {
            if (s.bus == BusType::BUS_MQTT && s.mode == SignalMode::NORMAL) {
                if (s.quality != SignalQuality::ERROR && (now - s.lastUpdateMs > REMOTE_TIMEOUT_MS)) {
                    s.quality = SignalQuality::ERROR;
                    s.valid = false;
                    escribirLog("SIGNAL: Timeout P2P en señal remota [%s]", s.id);
                }
            }
        }
    }

    // --- Actualización de Señales Virtuales ---
    void _updateVirtual(Signal &s) {
        Signal *sA = getById(s.sourceA);
        Signal *sB = getById(s.sourceB);
        
        if (!sA || !sB) {
            s.quality = SignalQuality::ERROR;
            s.valid = false;
            return;
        }

        // Ejecución de la operación
        switch (s.virtualOp) {
            case VirtualOp::SUM:  s.raw = sA->value + sB->value; break;
            case VirtualOp::DIFF: s.raw = sA->value - sB->value; break;
            case VirtualOp::MUL:  s.raw = sA->value * sB->value; break;
            case VirtualOp::DIV:  
                s.raw = (std::abs(sB->value) > 0.00001f) ? (sA->value / sB->value) : 0.0f; 
                break;
            case VirtualOp::AND:  s.raw = (sA->value > 0.5f && sB->value > 0.5f) ? 1.0f : 0.0f; break;
            case VirtualOp::OR:   s.raw = (sA->value > 0.5f || sB->value > 0.5f) ? 1.0f : 0.0f; break;
            default: break;
        }
        
        // Propagación de calidad: Si una fuente falla, la virtual falla
        if (sA->quality == SignalQuality::GOOD && sB->quality == SignalQuality::GOOD) {
            s.quality = SignalQuality::GOOD;
            s.valid = true;
        } else {
            s.quality = SignalQuality::ERROR;
            s.valid = false;
        }
    }

    // --- Ciclo Principal de Actualización ---
    void updateAll() {
        // 1. Verificar Timeouts de red primero
        processWatchdog();

        // 2. CAPTURA DE HARDWARE LOCAL
        for (auto &s : _signals) {
            // Ignoramos virtuales y remotas en este paso
            if (s.mode == SignalMode::OFFLINE || s.bus == BusType::BUS_VIRTUAL || s.bus == BusType::BUS_MQTT) continue;
            if (s.mode == SignalMode::MANTENIMIENTO) continue;

            float rawData = 0.0f;
            bool ok = false;

            if (s.mode == SignalMode::TEST) {
                rawData = s.manualValue;
                ok = true;
            } else {
                ok = ChipMgr::getRawValue(s.chipId, s.channel, rawData);
            }

            if (ok) {
                s.raw = rawData;
                s.quality = SignalQuality::GOOD;
                s.valid = true;
                _applyLogic(s); // Procesar localmente
            } else {
                s.quality = SignalQuality::ERROR;
                s.valid = false;
            }
        }

        // 3. PROCESAMIENTO DE VIRTUALES (Después de las físicas)
        for (auto &s : _signals) {
            if (s.bus != BusType::BUS_VIRTUAL) continue;
            _updateVirtual(s);
            _applyLogic(s); // La lógica de escalado/filtro también se aplica a la virtual
        }
    }

    bool writeSignal(const char *id, float value) {
        Signal *s = getById(id);
        
        if (!s || !s->writable) return false;
        if (s->mode == SignalMode::OFFLINE || s->mode == SignalMode::MANTENIMIENTO) return false;

        float valueToSend = value;
        // Tratar la inversión antes de salir al hardware
        if (s->invertido) {
            if (s->kind == SignalKind::ACTUATOR_DIGITAL) {
                valueToSend = (value > 0.5f) ? 0.0f : 1.0f;
            } else {
                valueToSend = (s->calib.realMax - value + s->calib.realMin);
            }
        }

        // Envío por Red (MQTT)
        if (s->bus == BusType::BUS_MQTT) {
            MQTTManager::publishSignal(s->id, valueToSend); 
            s->value = value;
            s->lastUpdateMs = millis();
            return true;
        }

        // Simulación TEST
        if (s->mode == SignalMode::TEST) {
            s->value = value;
            return true;
        }

        // Escritura Física (Chips/Modbus)
        if (ChipMgr::setRawValue(s->chipId, s->channel, valueToSend)) {
            s->value = value;
            s->quality = SignalQuality::GOOD;
            s->lastUpdateMs = millis();
            return true;
        }

        s->quality = SignalQuality::ERROR;
        return false;
    }

    std::vector<Signal>& getAll() { return _signals; }
    size_t getCount() { return _signals.size(); }

} // namespace SignalMgr#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"
#include "net/mqtt_manager.h"
#include <vector>
#include <string.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace SignalMgr {

    static std::vector<Signal> _signals;
    static SemaphoreHandle_t _sigMutex = nullptr; 
    extern SemaphoreHandle_t semI2C; // Semáforo global del bus I2C
    
    static const unsigned long REMOTE_TIMEOUT_MS = 15000;

    // --- Inicialización del Manager ---
    void init() {
        if (!_sigMutex) {
            _sigMutex = xSemaphoreCreateMutex();
        }
    }

    // --- Búsqueda Segura ---
    Signal* getById(const char *id) {
        if (!id) return nullptr;
        // Nota: Idealmente llamar a esto bajo lock si es desde fuera
        for (auto &s : _signals) {
            if (strcmp(s.id, id) == 0) return &s;
        }
        return nullptr;
    }

    void addSignal(const Signal& s) {
        if (!_sigMutex) init();
        
        if (xSemaphoreTake(_sigMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (!getById(s.id)) {
                _signals.push_back(s);
            }
            xSemaphoreGive(_sigMutex);
        }
    }

    // --- Lógica de Procesamiento (Escalado, Filtro, Inversión) ---
    void _applyLogic(Signal &s) {
        s.prev = s.value;
        float val = s.raw;

        // 1. Escalamiento Lineal
        if (s.calib.usaescala) {
            float rawDiff = s.calib.rawMax - s.calib.rawMin;
            if (std::abs(rawDiff) > 0.0001f) {
                val = ((val - s.calib.rawMin) * (s.calib.realMax - s.calib.realMin) / rawDiff) + s.calib.realMin;
            }
        }

        val += s.calib.offset;

        // 2. Filtro EMA (Protección contra Alpha inválido o NaN)
        float alpha = (s.calib.emaAlpha <= 0.0f || s.calib.emaAlpha > 1.0f) ? 1.0f : s.calib.emaAlpha;
        
        if (!s.calib.emaInit || std::isnan(s.calib.emaValue)) {
            s.calib.emaValue = val;
            s.calib.emaInit = true;
        } else {
            s.calib.emaValue = (alpha * val) + ((1.0f - alpha) * s.calib.emaValue);
        }
        val = s.calib.emaValue;

        // 3. Inversión Lógica/Analógica
        if (s.invertido) {
            if (s.kind == SignalKind::SENSOR_DIGITAL || s.kind == SignalKind::ACTUATOR_DIGITAL) {
                val = (val > 0.5f) ? 0.0f : 1.0f;
            } else {
                val = (s.calib.realMax - val + s.calib.realMin);
            }
        }

        s.value = val;
        s.lastUpdateMs = millis();
    }

    // --- Actualización remota (MQTT/P2P) ---
    void updateRemote(const char* id, float val, SignalQuality quality) {
        if (xSemaphoreTake(_sigMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            Signal* s = getById(id);
            if (s) {
                s->raw = val;
                s->quality = quality;
                s->valid = (quality == SignalQuality::GOOD);
                _applyLogic(*s); 
            }
            xSemaphoreGive(_sigMutex);
        }
    }

    // --- Watchdog de Señales Externas ---
    void processWatchdog() {
        unsigned long now = millis();
        for (auto &s : _signals) {
            if (s.bus == BusType::BUS_MQTT && s.mode == SignalMode::NORMAL) {
                if (s.quality != SignalQuality::ERROR && (now - s.lastUpdateMs > REMOTE_TIMEOUT_MS)) {
                    s.quality = SignalQuality::ERROR;
                    s.valid = false;
                    escribirLog("SIGNAL: Timeout P2P en [%s]", s.id);
                }
            }
        }
    }

    // --- Lógica de Operaciones Virtuales ---
    void _updateVirtual(Signal &s) {
        Signal *sA = getById(s.sourceA);
        Signal *sB = getById(s.sourceB);
        
        if (!sA || !sB || !sA->valid || !sB->valid) {
            s.quality = SignalQuality::ERROR;
            s.valid = false;
            return;
        }

        switch (s.virtualOp) {
            case VirtualOp::SUM:  s.raw = sA->value + sB->value; break;
            case VirtualOp::DIFF: s.raw = sA->value - sB->value; break;
            case VirtualOp::MUL:  s.raw = sA->value * sB->value; break;
            case VirtualOp::DIV:  
                s.raw = (std::abs(sB->value) > 0.00001f) ? (sA->value / sB->value) : 0.0f; 
                break;
            case VirtualOp::AND:  s.raw = (sA->value > 0.5f && sB->value > 0.5f) ? 1.0f : 0.0f; break;
            case VirtualOp::OR:   s.raw = (sA->value > 0.5f || sB->value > 0.5f) ? 1.0f : 0.0f; break;
            default: break;
        }
        
        s.quality = SignalQuality::GOOD;
        s.valid = true;
    }

    // --- CICLO PRINCIPAL: Captura de todo el PLC ---
    void updateAll() {
        if (!_sigMutex) return;
        if (xSemaphoreTake(_sigMutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

        processWatchdog();

        for (auto &s : _signals) {
            // Filtrar señales que no se actualizan localmente
            if (s.mode == SignalMode::OFFLINE || s.mode == SignalMode::MANTENIMIENTO) continue;
            if (s.bus == BusType::BUS_VIRTUAL || s.bus == BusType::BUS_MQTT) continue;

            float rawData = 0.0f;
            bool ok = false;

            if (s.mode == SignalMode::TEST) {
                rawData = s.manualValue;
                ok = true;
            } else {
                // Bloqueo de bus físico según el protocolo
                if (s.bus == BusType::BUS_I2C) {
                    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
                        ok = ChipMgr::getRawValue(s.chipId, s.channel, rawData);
                        xSemaphoreGive(semI2C);
                    }
                } else {
                    ok = ChipMgr::getRawValue(s.chipId, s.channel, rawData);
                }
            }

            if (ok) {
                s.raw = rawData;
                s.quality = SignalQuality::GOOD;
                s.valid = true;
                _applyLogic(s);
            } else {
                s.quality = SignalQuality::ERROR;
                s.valid = false;
            }
        }

        // Procesar virtuales al final (necesitan los datos físicos frescos)
        for (auto &s : _signals) {
            if (s.bus == BusType::BUS_VIRTUAL) {
                _updateVirtual(s);
                _applyLogic(s);
            }
        }

        xSemaphoreGive(_sigMutex);
    }

    // --- Escritura Unificada ---
    bool writeSignal(const char *id, float value) {
        if (xSemaphoreTake(_sigMutex, pdMS_TO_TICKS(100)) != pdTRUE) return false;

        Signal *s = getById(id);
        if (!s || !s->writable || s->mode == SignalMode::OFFLINE) {
            xSemaphoreGive(_sigMutex);
            return false;
        }

        float valueToSend = value;
        if (s->invertido) {
            if (s->kind == SignalKind::ACTUATOR_DIGITAL) {
                valueToSend = (value > 0.5f) ? 0.0f : 1.0f;
            } else {
                valueToSend = (s->calib.realMax - value + s->calib.realMin);
            }
        }

        bool success = false;

        // Desvío según bus
        if (s->bus == BusType::BUS_MQTT) {
            success = MQTTManager::publishSignal(s->id, valueToSend);
        } else if (s->mode == SignalMode::TEST) {
            success = true;
        } else {
            // Llamada al manager de hardware (EscribirSignal que corregimos antes)
            // Se asume que setRawValue ya maneja el semI2C o es seguro
            success = ChipMgr::setRawValue(s->chipId, s->channel, valueToSend);
        }

        if (success) {
            s->value = value;
            s->lastUpdateMs = millis();
            s->quality = SignalQuality::GOOD;
        }

        xSemaphoreGive(_sigMutex);
        return success;
    }

    std::vector<Signal>& getAll() { return _signals; }
    size_t getCount() { return _signals.size(); }

} // namespace SignalMgr