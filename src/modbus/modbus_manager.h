#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <ModbusMaster.h>

#include "modbus/modbus_slave_context.h"
#include "signal/signal_struct.h"

class ModbusManager
{
public:
    // -------- INIT ----------
    static void init(HardwareSerial& serial, uint32_t baud);

    // -------- LECTURA SIMPLE ----------
    static bool readHolding(uint8_t slave, uint16_t reg, uint16_t& out);
    static bool readInput  (uint8_t slave, uint16_t reg, uint16_t& out);

    // -------- LECTURA RAW (bloques) ----------
    static bool readHoldingRaw(uint8_t slave,
                               uint16_t start,
                               uint16_t count,
                               uint16_t* buffer);

    static bool readInputRaw(uint8_t slave,
                             uint16_t start,
                             uint16_t count,
                             uint16_t* buffer);

    // -------- BLOQUE GENÉRICO ----------
    static bool readBlock(uint8_t slave,
                          ModbusRegType type,
                          uint16_t start,
                          uint16_t count,
                          uint16_t* buffer);

    static bool readBlockRaw(uint8_t slave,
                             ModbusRegType type,
                             uint16_t start,
                             uint16_t count,
                             uint16_t* buffer);

    // -------- ESCRITURA ----------
    static bool writeHolding(uint8_t slave,
                             uint16_t reg,
                             uint16_t value);

    static bool writeMultipleHolding(uint8_t slave,
                                     uint16_t start,
                                     uint8_t count,
                                     uint16_t* values);
                                       

private:
    static ModbusMaster   g_node;
    static HardwareSerial* g_serial;
};
void modbusCheckRecovery();  