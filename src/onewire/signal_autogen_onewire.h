#pragma once
#include <stdint.h>

namespace SignalAutogenOneWire {

    /**
     * @brief Escanea las entradas OneWire del sistema y genera señales
     *
     * - Escanea cada GPIO OneWire definido en board_config.h
     * - Genera una Signal por cada ROM detectada
     * - Marca las señales como systemReserved y lockedConfig
     */
    void generateAll();

}