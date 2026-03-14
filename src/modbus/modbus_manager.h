#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>
#include "modbus_utils.h" // Para ModbusRegType y otros enums

/**
 * @brief Driver de bajo nivel para la comunicación RS485.
 * Se encarga exclusivamente de la transacción de tramas Modbus.
 */
class ModbusManager {
public:
    // --- Inicialización ---
    static void init(HardwareSerial &serial, uint32_t baud = 9600);
    static void init(uint32_t baud);
    static void init();

    // --- Lectura de Registros (Raw) ---
    // Estas funciones llenan un buffer de 16 bits directamente desde el bus
    static bool readHoldingRaw(uint8_t slave, uint16_t start, uint16_t count, uint16_t *buffer);
    static bool readInputRaw(uint8_t slave, uint16_t start, uint16_t count, uint16_t *buffer);

    // --- Funciones de Conveniencia (Single Register) ---
    static bool readHolding(uint8_t slave, uint16_t reg, uint16_t &out);
    static bool readInput(uint8_t slave, uint16_t reg, uint16_t &out);

    // --- Escritura ---
    static bool writeHolding(uint8_t slave, uint16_t reg, uint16_t value);
    static bool writeMultipleHolding(uint8_t slave, uint16_t start, uint8_t count, uint16_t *values);

    // --- Gestión Genérica ---
    // Útil para perfiles dinámicos que reciben el tipo de registro por variable
    static bool readBlockRaw(uint8_t slave, ModbusRegType type, uint16_t start, uint16_t count, uint16_t *buffer);

    // --- Mantenimiento del Bus ---
    static void checkRecovery();

private:
    static ModbusMaster g_node;
    static HardwareSerial *g_serial;
    static uint32_t g_baud;
};