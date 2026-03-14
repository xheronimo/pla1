#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include "chip/chip_struct.h"
#include "modbus_utils.h"

namespace ModbusProfiles {

    /**
     * @brief Definición de una señal dentro de un bloque Modbus (Perfil Dinámico)
     */
    struct DynamicSignal {
        uint8_t  posInBlock;    // Posición del registro dentro del buffer leído
        uint8_t  channelIndex;  // Índice del canal en el Chip donde se guardará
        ModbusDataType  type;   // Float32, Int16, etc.
        ModbusWordOrder order;  // AB, BA, CDAB...
        float    factor;        // Multiplicador (ej: 0.1 para pasar de 255 a 25.5)
    };

    /**
     * @brief Definición de un bloque de lectura Modbus
     */
    struct DynamicBlock {
        uint8_t  fc;            // Function Code (3: Holding, 4: Input)
        uint16_t start;         // Registro inicial
        uint16_t count;         // Cantidad de registros a leer
        std::vector<DynamicSignal> signals; // Señales contenidas en este bloque
    };

    /**
     * @brief Perfil completo cargado desde JSON
     */
    struct DynamicProfile {
        uint16_t fp;            // Fingerprint único
        std::vector<DynamicBlock> blocks;
    };

    // --- Repositorio de Perfiles Dinámicos ---
    extern std::vector<DynamicProfile> g_dynamicProfiles;

    // --- Motor de Ejecución ---
    
    /**
     * @brief Ejecuta la lectura de un perfil dinámico sobre un Chip.
     */
    bool pollDynamic(Chip* chip, const DynamicProfile& prof);

    /**
     * @brief Espacio para perfiles "Nativos" (Hardcoded para máxima velocidad)
     */
    namespace Native {
        bool pollSHT20(Chip* chip);
        bool pollSDM630(Chip* chip);
    }

} // namespace ModbusProfiles