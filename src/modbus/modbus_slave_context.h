#pragma once
#include <stdint.h>
#include "modbus_config.h"

enum class ModbusSlaveState : uint8_t
{
    STATE_UNINITIALIZED = 0,
    STATE_READY,
    STATE_ERROR,
    STATE_TIMEOUT,
    STATE_DISABLED
};

enum class ModbusPriority : uint8_t
{
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL,
    PRIORITY_CRITICAL
};

struct ModbusSlaveContext {
    // --- Identificación y Estado Maestro ---
    uint8_t id = 0;
    bool inUse = false;
    bool critical = false;
    bool configFault = false;
    ModbusSlaveState state = ModbusSlaveState::STATE_UNINITIALIZED;
    ModbusPriority priority = ModbusPriority::PRIORITY_NORMAL;

    // --- Tiempos y Control de Flujo (Agrupados para evitar padding) ---
    uint32_t pollIntervalMs = MODBUS_DEFAULT_POLL_MS;
    uint32_t timeoutMs = MODBUS_DEFAULT_TIMEOUT_MS;
    uint32_t retryMs = MODBUS_DEFAULT_RETRY_MS;
    uint32_t disabledUntilMs = 0;

    // --- Timestamps (Estado Temporal) ---
    uint32_t lastPollTs = 0;
    uint32_t lastOkTs = 0;
    uint32_t lastErrorTs = 0;

    // --- Gestión de Errores y Límites ---
    uint16_t minRegister = 0;
    uint16_t maxRegister = 65535;
    uint8_t consecutiveErrors = 0;
    uint8_t maxErrors = 5;

    // --- Estadísticas y Telemetría ---
    uint32_t totalReads = 0;
    uint32_t totalErrors = 0;
    uint32_t configErrors = 0;
};

ModbusSlaveContext *modbusGetContext(uint8_t id);

bool modbusRegisterAllowed(ModbusSlaveContext *ctx,
                           uint16_t start,
                           uint16_t count);

                   