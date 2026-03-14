#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include <vector>
#include <string.h>
#include <cmath>

namespace SignalMgr {
    static std::vector<Signal> _signals;

    Signal* getById(const char *id) {
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
        // Guardamos el valor actual como 'prev' antes de actualizarlo
        // Esto es vital para que las alarmas detecten flancos (RISE/FALL)
        s.prev = s.value;

        float val = s.raw;

        // 1. Escalamiento Lineal
        if (s.calib.usaescala && std::abs(s.calib.rawMax - s.calib.rawMin) > 0.001f) {
            val = ((val - s.calib.rawMin) * (s.calib.realMax - s.calib.realMin) /
                   (s.calib.rawMax - s.calib.rawMin)) + s.calib.realMin;
        }

        val += s.calib.offset;

        // 2. Filtro EMA (Exponential Moving Average)
        if (!s.calib.emaInit) {
            s.calib.emaValue = val;
            s.calib.emaInit = true;
        } else {
            // Si emaAlpha es 1.0, no hay filtro. Si es 0.1, el filtro es muy fuerte.
            s.calib.emaValue = (s.calib.emaAlpha * val) + ((1.0f - s.calib.emaAlpha) * s.calib.emaValue);
        }
        val = s.calib.emaValue;

        // 3. Inversión Lógica/Analógica
        if (s.invertido) {
            if (s.kind == SignalKind::SENSOR_DIGITAL || s.kind == SignalKind::ACTUATOR_DIGITAL)
                val = (val > 0.5f) ? 0.0f : 1.0f;
            else
                val = (s.calib.realMax - val + s.calib.realMin);
        }

        s.value = val;
        s.lastUpdateMs = millis();
    }

    void _updateVirtual(Signal &s) {
        Signal *sA = getById(s.sourceA);
        Signal *sB = getById(s.sourceB);
        
        if (!sA || !sB) {
            s.quality = SignalQuality::ERROR;
            return;
        }

        switch (s.virtualOp) {
            case VirtualOp::SUM:  s.raw = sA->value + sB->value; break;
            case VirtualOp::DIFF: s.raw = sA->value - sB->value; break;
            case VirtualOp::MUL:  s.raw = sA->value * sB->value; break;
            case VirtualOp::AND:  s.raw = (sA->value > 0.5f && sB->value > 0.5f) ? 1.0f : 0.0f; break;
            case VirtualOp::OR:   s.raw = (sA->value > 0.5f || sB->value > 0.5f) ? 1.0f : 0.0f; break;
            default: break;
        }
        
        // La calidad de una señal virtual depende de sus fuentes
        s.quality = (sA->quality == SignalQuality::GOOD && sB->quality == SignalQuality::GOOD) 
                    ? SignalQuality::GOOD : SignalQuality::ERROR;
    }

    // Renombramos pollAll a updateAll para coincidir con el main.cpp
    void updateAll() {
        // PASO 1: Captura de Hardware
        for (auto &s : _signals) {
            if (s.mode == SignalMode::OFFLINE || s.bus == BusType::BUS_VIRTUAL) continue;

            // En mantenimiento, el valor se congela (no se actualiza 'raw')
            if (s.mode == SignalMode::MANTENIMIENTO) continue;

            float rawData = 0.0f;
            bool ok = false;

            if (s.mode == SignalMode::TEST) {
                rawData = s.manualValue;
                ok = true;
            } else {
                // LLAMADA AL CHIP MANAGER (Hardware real)
                ok = ChipMgr::getRawValue(s.chipId, s.channel, rawData);
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

        // PASO 2: Procesamiento de Virtuales
        for (auto &s : _signals) {
            if (s.bus != BusType::BUS_VIRTUAL) continue;
            _updateVirtual(s);
            _applyLogic(s);
        }
    }

    bool writeSignal(const char *id, float value) {
        Signal *s = getById(id);
        if (!s || !s->writable || s.mode == SignalMode::OFFLINE || s.mode == SignalMode::MANTENIMIENTO) {
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

        if (s->mode == SignalMode::TEST) {
            s->value = value;
            return true;
        }

        // ESCRITURA FÍSICA
        if (ChipMgr::setRawValue(s->chipId, s->channel, valueToSend)) {
            s->value = value;
            s->quality = SignalQuality::GOOD;
            return true;
        }

        s->quality = SignalQuality::ERROR;
        return false;
    }

    std::vector<Signal>& getAll() { return _signals; }

} // namespace SignalMgr