#include "chip_manager.h"
#include "i2c/i2c_chip_registry.h"
#include "system/LogSystem.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <unordered_map>
#include <string>
#include <vector>

namespace ChipMgr {

    // Estructura interna de los canales para el Manager
    struct ChannelState {
        float rawValue = 0.0f;
        uint32_t lastUpdateMs = 0;
        std::string signalId = "";
        struct {
            bool used = false;
            bool valid = false;
        } flags;
    };

    // Objeto Chip extendido para gestión de estados
    struct Chip {
        std::string id;
        I2CDevice type;
        uint8_t address;
        ChipMode mode;
        std::vector<ChannelState> channels;
    };

    // Almacenamiento y Sincronización
    static std::unordered_map<std::string, Chip> _chips;
    static SemaphoreHandle_t _chipMutex = nullptr;

    // --- Inicialización Interna ---
    void _ensureMutex() {
        if (_chipMutex == nullptr) {
            _chipMutex = xSemaphoreCreateMutex();
        }
    }

    // ======================================================
    // GESTIÓN DE CHIPS (ADMIN)
    // ======================================================

    void add(const std::string& id, I2CDevice type, uint8_t addr, ChipMode mode, uint8_t channelCount) {
        _ensureMutex();
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            Chip c;
            c.id = id;
            c.type = type;
            c.address = addr;
            c.mode = mode;
            c.channels.resize(channelCount);
            
            _chips[id] = c;
            xSemaphoreGive(_chipMutex);
        }
    }

    Chip* _getInternal(const std::string& id) {
        auto it = _chips.find(id);
        return (it != _chips.end()) ? &it->second : nullptr;
    }

    // ======================================================
    // GESTIÓN DE CANALES
    // ======================================================

    bool markChannelUsed(const std::string& chipId, int channel, const std::string& signalId) {
        bool result = false;
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            Chip* c = _getInternal(chipId);
            if (c && channel >= 0 && channel < (int)c->channels.size()) {
                if (!c->channels[channel].flags.used) {
                    c->channels[channel].flags.used = true;
                    c->channels[channel].signalId = signalId;
                    result = true;
                }
            }
            xSemaphoreGive(_chipMutex);
        }
        return result;
    }

    void markChannelFree(const std::string& chipId, int channel) {
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            Chip* c = _getInternal(chipId);
            if (c && channel >= 0 && channel < (int)c->channels.size()) {
                c->channels[channel].flags.used = false;
                c->channels[channel].flags.valid = false;
                c->channels[channel].signalId.clear();
            }
            xSemaphoreGive(_chipMutex);
        }
    }

    std::vector<int> getFreeChannels(const std::string& chipId) {
        std::vector<int> free;
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            Chip* c = _getInternal(chipId);
            if (c) {
                for (int i = 0; i < (int)c->channels.size(); i++) {
                    if (!c->channels[i].flags.used) free.push_back(i);
                }
            }
            xSemaphoreGive(_chipMutex);
        }
        return free;
    }

    // ======================================================
    // ACCESO A DATOS (Runtime para SignalManager)
    // ======================================================

    bool getRawValue(const std::string& chipId, int channel, float& outValue) {
        bool ok = false;
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            Chip* c = _getInternal(chipId);
            if (c && channel >= 0 && channel < (int)c->channels.size()) {
                if (c->channels[channel].flags.valid) {
                    outValue = c->channels[channel].rawValue;
                    ok = true;
                }
            }
            xSemaphoreGive(_chipMutex);
        }
        return ok;
    }

    bool setRawValue(const std::string& chipId, int channel, float value) {
        bool ok = false;
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            Chip* c = _getInternal(chipId);
            if (c && channel >= 0 && channel < (int)c->channels.size()) {
                // Protección: No escribir si el hardware es solo de entrada
                if (c->mode != ChipMode::INPUT_ONLY) {
                    c->channels[channel].rawValue = value;
                    c->channels[channel].lastUpdateMs = millis();
                    c->channels[channel].flags.valid = true;
                    ok = true;
                }
            }
            xSemaphoreGive(_chipMutex);
        }
        return ok;
    }

    // --- Sincronización con Drivers Reales ---
    void updateHardware() {
        // Esta función recorre los chips y llama a los drivers de I2C_Registry
        // Se llama desde la tarea de control principal
        if (xSemaphoreTake(_chipMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            for (auto& pair : _chips) {
                Chip& c = pair.second;
                // Aquí se implementaría la llamada al driver específico 
                // para refrescar los rawValue de los canales usados.
            }
            xSemaphoreGive(_chipMutex);
        }
    }

} // namespace ChipMgr