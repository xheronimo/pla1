#pragma once

#include <stdint.h>
#include <vector>

namespace ModbusProvisioning {

    /**
     * @brief Estructura para almacenar los resultados de un escaneo de bus.
     */
    struct ScanResult {
        uint8_t  id;            // ID del esclavo encontrado
        uint32_t baudrate;      // Velocidad a la que respondió
        uint16_t fingerprint;   // Identificador del dispositivo (si responde en 0x0101)

        ScanResult(uint8_t _id = 0, uint32_t _baud = 9600, uint16_t _fp = 0)
            : id(_id), baudrate(_baud), fingerprint(_fp) {}
    };

    /**
     * @brief Intenta comunicación básica con un ID específico.
     * Útil para verificar presencia física antes de configurar.
     */
    bool ping(uint8_t id);

    /**
     * @brief Escanea el bus RS485 buscando esclavos.
     * @param baudrate Velocidad a la que se realizará el escaneo.
     * @param results Vector donde se volcarán los dispositivos encontrados.
     * @return Cantidad de dispositivos detectados.
     */
    size_t scan(uint32_t baudrate, std::vector<ScanResult>& results);

    /**
     * @brief Cambia el ID de un esclavo de forma segura.
     * Verifica que el ID de destino esté libre antes de proceder.
     * @param fromId ID actual del sensor.
     * @param toId Nuevo ID deseado.
     * @param regIdConfig Dirección del registro Modbus donde el sensor guarda su ID.
     */
    bool changeIdSafe(uint8_t fromId, uint8_t toId, uint16_t regIdConfig);

    /**
     * @brief Cambia la velocidad de comunicación de un sensor.
     * @param id ID del sensor a configurar.
     * @param regBaudConfig Registro de configuración de baudrate del fabricante.
     * @param newBaudCode Código de velocidad (ej: 3 para 9600, 4 para 19200 según manual).
     */
    bool changeBaudrateSafe(uint8_t id, uint16_t regBaudConfig, uint16_t newBaudCode);

} // namespace ModbusProvisioning