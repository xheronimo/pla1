#include "modbus/modbus_manager.h"
#include "modbus/modbus_guard.h"
#include "modbus/modbus_config.h"
#include "system/system_mode.h"
#include "system/LogSystem.h"
extern bool g_modbusDegraded;
// ================== STATIC ==================
ModbusMaster ModbusManager::g_node;
HardwareSerial *ModbusManager::g_serial = nullptr;

static uint32_t g_lastStableMs = 0;
// ================== INIT ==================
void ModbusManager::init(HardwareSerial &serial, uint32_t baud)
{
    g_serial = &serial;
    g_serial->begin(baud);
}

// ================== SIMPLE READ ==================
bool ModbusManager::readHolding(uint8_t slave, uint16_t reg, uint16_t &out)
{
    uint16_t buf;
    if (!readHoldingRaw(slave, reg, 1, &buf))
        return false;

    out = buf;
    return true;
}

bool ModbusManager::readInput(uint8_t slave, uint16_t reg, uint16_t &out)
{
    uint16_t buf;
    if (!readInputRaw(slave, reg, 1, &buf))
        return false;

    out = buf;
    return true;
}

// ================== RAW READ ==================
bool ModbusManager::readHoldingRaw(uint8_t slave,
                                   uint16_t start,
                                   uint16_t count,
                                   uint16_t *buffer)
{
    ModbusSlaveContext *ctx = modbusGetContext(slave);
    if (!modbusGuardBeforeRead(ctx))
        return false;

    g_node.begin(slave, *g_serial);

    uint8_t res = g_node.readHoldingRegisters(start, count);

    if (res != g_node.ku8MBSuccess)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    for (uint16_t i = 0; i < count; i++)
        buffer[i] = g_node.getResponseBuffer(i);

    modbusGuardOnSuccess(ctx);
    return true;
}

bool ModbusManager::readInputRaw(uint8_t slave,
                                 uint16_t start,
                                 uint16_t count,
                                 uint16_t *buffer)
{
    ModbusSlaveContext *ctx = modbusGetContext(slave);
    if (!modbusGuardBeforeRead(ctx))
        return false;

    g_node.begin(slave, *g_serial);

    uint8_t res = g_node.readInputRegisters(start, count);

    if (res != g_node.ku8MBSuccess)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    for (uint16_t i = 0; i < count; i++)
        buffer[i] = g_node.getResponseBuffer(i);

    modbusGuardOnSuccess(ctx);
    return true;
}

// ================== GENERIC BLOCK ==================
bool ModbusManager::readBlock(uint8_t slave,
                              ModbusRegType type,
                              uint16_t start,
                              uint16_t count,
                              uint16_t *buffer)
{
    return readBlockRaw(slave, type, start, count, buffer);
}

bool ModbusManager::readBlockRaw(uint8_t slave,
                                 ModbusRegType type,
                                 uint16_t start,
                                 uint16_t count,
                                 uint16_t *buffer)
{
    if (type == ModbusRegType::MODBUS_HOLDING)
        return readHoldingRaw(slave, start, count, buffer);

    if (type == ModbusRegType::MODBUS_INPUT)
        return readInputRaw(slave, start, count, buffer);

    return false;
}

// ================== WRITE ==================
bool ModbusManager::writeHolding(uint8_t slave,
                                 uint16_t reg,
                                 uint16_t value)
{
    ModbusSlaveContext *ctx = modbusGetContext(slave);
    if (!modbusGuardBeforeRead(ctx))
        return false;

    g_node.begin(slave, *g_serial);

    uint8_t res = g_node.writeSingleRegister(reg, value);

    if (res != g_node.ku8MBSuccess)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    modbusGuardOnSuccess(ctx);
    return true;
}

bool ModbusManager::writeMultipleHolding(uint8_t slave,
                                         uint16_t start,
                                         uint8_t count,
                                         uint16_t *values)
{
    ModbusSlaveContext *ctx = modbusGetContext(slave);
    if (!modbusGuardBeforeRead(ctx))
        return false;

    g_node.begin(slave, *g_serial);

    for (uint8_t i = 0; i < count; i++)
        g_node.setTransmitBuffer(i, values[i]);

    uint8_t res = g_node.writeMultipleRegisters(start, count);

    if (res != g_node.ku8MBSuccess)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    modbusGuardOnSuccess(ctx);
    return true;
}

void modbusCheckRecovery()
{
    if (!g_modbusDegraded)
        return;

    bool anyTimeout = false;

    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        ModbusSlaveContext *ctx = modbusGetContext(i);
        if (!ctx || !ctx->inUse)
            continue;

        if (ctx->state == ModbusSlaveState::STATE_TIMEOUT ||
            ctx->state == ModbusSlaveState::STATE_ERROR)
        {
            anyTimeout = true;
            break;
        }
    }

    if (!anyTimeout)
    {
        if (g_lastStableMs == 0)
            g_lastStableMs = millis();

        // estable durante X segundos
        if (millis() - g_lastStableMs > 10000)
        {
            g_modbusDegraded = false;
            g_lastStableMs = 0;
            escribirLog("MODBUS: Modo degradado DESACTIVADO");
        }
    }
    else
    {
        g_lastStableMs = 0;
    }
}