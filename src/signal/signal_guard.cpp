#include "signal/signal_guard.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"

namespace SignalGuard {

    SignalUpdateResult validateUpdate(const Signal& current, JsonObjectConst update) {
        
        // 1. Proteger señales del sistema (Ej: Temperatura CPU, Status Bus)
        if (current.isReserved) {
            // No permitimos cambiar a qué hardware apuntan
            if (update.containsKey("chip") || update.containsKey("channel")) {
                LOG_WRN("[Guard] Intento de modificar estructura de señal reservada: %s", current.name.c_str());
                return SignalUpdateResult::ERR_RESERVED;
            }
        }

        // 2. Validar existencia del Chip
        if (update.containsKey("chip")) {
            const char* targetChip = update["chip"];
            if (!ChipMgr::getById(targetChip)) {
                LOG_ERR("[Guard] El chip '%s' no existe", targetChip);
                return SignalUpdateResult::ERR_CHIP_NOT_FOUND;
            }
        }

        // 3. Validar límites del Canal
        if (update.containsKey("channel")) {
            int channelIdx = update["channel"];
            const char* chipName = update["chip"] | current.chipName.c_str();
            auto* chip = ChipMgr::getById(chipName);
            
            if (chip && (channelIdx < 0 || (size_t)channelIdx >= chip->channels.size())) {
                LOG_ERR("[Guard] Índice de canal %d fuera de rango para chip %s", channelIdx, chipName);
                return SignalUpdateResult::ERR_INVALID_CHANNEL;
            }
        }

        return SignalUpdateResult::OK;
    }
}