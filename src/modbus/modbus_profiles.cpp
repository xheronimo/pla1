#include "modbus_profiles.h"
#include "modbus_manager.h"
#include "modbus_utils.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"
#include <Arduino.h>

namespace ModbusProfiles {

    // Repositorio global de perfiles cargados desde JSON
    std::vector<DynamicProfile> g_dynamicProfiles;

    // --- PERFILES NATIVOS (Optimizados para sensores específicos) ---
    namespace Native {

        /**
         * @brief Perfil SHT20 (Temperatura y Humedad)
         * Dirección registros: 0x0001 (Temp), 0x0002 (Hum) - Input Registers
         */
        bool pollSHT20(Chip* chip) {
            if (!chip || chip->channels.size() < 2) return false;

            uint16_t buffer[2];
            // Leemos 2 registros de un tiro (FC 04)
            if (ModbusManager::readInputRaw(chip->deviceId, 1, 2, buffer)) {
                // Registro 1: Temperatura (con signo, factor 0.1)
                chip->channels[0].rawValue = (float)((int16_t)buffer[0]) / 10.0f;
                chip->channels[0].valid = true;
                chip->channels[0].lastUpdateMs = millis();

                // Registro 2: Humedad (sin signo, factor 0.1)
                chip->channels[1].rawValue = (float)buffer[1] / 10.0f;
                chip->channels[1].valid = true;
                chip->channels[1].lastUpdateMs = millis();
                return true;
            }
            return false;
        }

        /**
         * @brief Perfil SDM630 (Medidor de Energía)
         * Registros 32-bit Float. Ejemplo: Voltaje L1 en 0x0000
         */
        bool pollSDM630(Chip* chip) {
            if (!chip || chip->channels.empty()) return false;

            uint16_t buffer[2];
            if (ModbusManager::readInputRaw(chip->deviceId, 0x0000, 2, buffer)) {
                // Usamos la utilidad de decodificación universal
                chip->channels[0].rawValue = ModbusUtils::decode(buffer, 
                                             ModbusDataType::FLOAT32, 
                                             ModbusWordOrder::AB);
                chip->channels[0].valid = true;
                chip->channels[0].lastUpdateMs = millis();
                return true;
            }
            return false;
        }
    }

    // --- MOTOR DE EJECUCIÓN DINÁMICA (JSON) ---

    bool pollDynamic(Chip* chip, const DynamicProfile& prof) {
        if (!chip) return false;

        bool success = true;

        // Recorremos cada bloque definido en el perfil (sin límites)
        for (const auto& block : prof.blocks) {
            uint16_t buffer[block.count];
            bool blockOk = false;

            // Decidir Function Code (3 o 4)
            if (block.fc == 4) {
                blockOk = ModbusManager::readInputRaw(chip->deviceId, block.start, block.count, buffer);
            } else {
                blockOk = ModbusManager::readHoldingRaw(chip->deviceId, block.start, block.count, buffer);
            }

            if (blockOk) {
                // Repartimos los datos del buffer en los canales del Chip
                for (const auto& sig : block.signals) {
                    if (sig.channelIndex >= chip->channels.size()) continue;

                    // Apuntamos al dato dentro del bloque leído
                    const uint16_t* dataPtr = &buffer[sig.posInBlock];
                    
                    // Decodificamos y aplicamos el factor de escala
                    float val = ModbusUtils::decode(dataPtr, sig.type, sig.order);
                    val *= sig.factor;

                    // Guardado directo en la caché del hardware (O(1))
                    chip->channels[sig.channelIndex].rawValue = val;
                    chip->channels[sig.channelIndex].valid = true;
                    chip->channels[sig.channelIndex].lastUpdateMs = millis();
                }
            } else {
                success = false; // Si falla un bloque, marcamos error en el chip
                break;
            }
        }
        return success;
    }

    /**
     * @brief Función maestra de despacho. 
     * Decide si usar un perfil nativo o buscar uno dinámico.
     */
    bool execute(Chip* chip, uint16_t fingerprint) {
        // 1. Intentar Nativos
        if (fingerprint == 0x5172) return Native::pollSHT20(chip);
        if (fingerprint == 0x0630) return Native::pollSDM630(chip);

        // 2. Buscar en Dinámicos (JSON)
        for (const auto& prof : g_dynamicProfiles) {
            if (prof.fp == fingerprint) {
                return pollDynamic(chip, prof);
            }
        }

        LOG_WRN("[Profiles] No se encontró perfil para FP: 0x%04X", fingerprint);
        return false;
    }

} // namespace ModbusProfiles