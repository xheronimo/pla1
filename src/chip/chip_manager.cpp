#include "chip_manager.h"
#include <unordered_map>

namespace ChipMgr {

    static std::unordered_map<std::string, Chip> _chips;

    // ======================================================
    // GESTIÓN DE CHIPS
    // ======================================================

    Chip* getById(const std::string& id) {
        auto it = _chips.find(id);
        return (it != _chips.end()) ? &it->second : nullptr;
    }

    void add(const Chip& c) {
        _chips[c.id] = c;
    }

    // ======================================================
    // GESTIÓN DE CANALES (Tus funciones originales + mejoras)
    // ======================================================

    bool markChannelUsed(const std::string& chipId, int channel, const std::string& signalId) {
        Chip* c = getById(chipId);
        if (!c || channel < 0 || channel >= (int)c->channels.size()) return false;

        if (c->channels[channel].flags.used) return false; // Ya ocupado

        c->channels[channel].flags.used = true;
        c->channels[channel].signalId = signalId;
        return true;
    }

    void markChannelFree(const std::string& chipId, int channel) {
        Chip* c = getById(chipId);
        if (!c || channel < 0 || channel >= (int)c->channels.size()) return;

        c->channels[channel].flags.used = false;
        c->channels[channel].signalId.clear();
        c->channels[channel].flags.valid = false; // El dato ya no es confiable
    }

    std::vector<int> getFreeChannels(const std::string& chipId) {
        std::vector<int> free;
        Chip* c = getById(chipId);
        if (!c) return free;

        for (int i = 0; i < (int)c->channels.size(); i++) {
            if (!c->channels[i].flags.used) {
                free.push_back(i);
            }
        }
        return free;
    }

    // ======================================================
    // ACCESO A DATOS (Runtime para SignalManager)
    // ======================================================

    bool getRawValue(const std::string& chipId, int channel, float& outValue) {
        Chip* c = getById(chipId);
        if (!c || channel < 0 || channel >= (int)c->channels.size()) return false;
        
        // Si el canal no ha sido actualizado nunca por el hardware, no es válido
        if (!c->channels[channel].flags.valid) return false;

        outValue = c->channels[channel].rawValue;
        return true;
    }

    bool setRawValue(const std::string& chipId, int channel, float value) {
        Chip* c = getById(chipId);
        
        // 🛡️ Seguridad: No escribir en chips configurados solo como entrada
        if (!c || c->mode == ChipMode::INPUT_ONLY) return false;
        
        if (channel < 0 || channel >= (int)c->channels.size()) return false;
        
        c->channels[channel].rawValue = value;
        c->channels[channel].lastUpdateMs = millis();
        c->channels[channel].flags.valid = true; 
        
        // Aquí los drivers de bus (Modbus/I2C) verán el cambio y actuarán
        return true;
    }

} // namespace ChipMgr