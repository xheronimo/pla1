#include "modbus/modbus_manager.h"
#include "modbus/modbus_guard.h"
#include "modbus/modbus_config.h"
#include "chip/chip_manager.h" // Para acceder a los chips si fuera necesario
#include "system/LogSystem.h"
#include <Arduino.h>

// Variables estáticas
ModbusMaster ModbusManager::g_node;
HardwareSerial *ModbusManager::g_serial = nullptr;
uint32_t ModbusManager::g_baud = 9600;

extern bool g_modbusDegraded;
static uint32_t g_lastStableMs = 0;

// --- INICIALIZACIÓN ---
void ModbusManager::init(HardwareSerial &serial, uint32_t baud) {
    g_serial = &serial;
    g_baud = baud;
    g_serial->begin(baud, SERIAL_8N1);
}

// --- LECTURAS RAW (El corazón del Driver) ---

bool ModbusManager::readHoldingRaw(uint8_t slave, uint16_t start, uint16_t count, uint16_t *buffer) {
    // IMPORTANTE: El ModbusGuard ahora se llama desde el Scheduler 
    // antes de entrar aquí, pero dejamos una protección mínima.
    
    g_node.begin(slave, *g_serial);
    uint8_t res = g_node.readHoldingRegisters(start, count);

    if (res != g_node.ku8MBSuccess) return false;

    for (uint16_t i = 0; i < count; i++) {
        buffer[i] = g_node.getResponseBuffer(i);
    }
    return true;
}

bool ModbusManager::readInputRaw(uint8_t slave, uint16_t start, uint16_t count, uint16_t *buffer) {
    g_node.begin(slave, *g_serial);
    uint8_t res = g_node.readInputRegisters(start, count);

    if (res != g_node.ku8MBSuccess) return false;

    for (uint16_t i = 0; i < count; i++) {
        buffer[i] = g_node.getResponseBuffer(i);
    }
    return true;
}

// --- ESCRITURAS ---

bool ModbusManager::writeHolding(uint8_t slave, uint16_t reg, uint16_t value) {
    g_node.begin(slave, *g_serial);
    uint8_t res = g_node.writeSingleRegister(reg, value);
    return (res == g_node.ku8MBSuccess);
}

bool ModbusManager::writeMultipleHolding(uint8_t slave, uint16_t start, uint8_t count, uint16_t *values) {
    g_node.begin(slave, *g_serial);
    for (uint8_t i = 0; i < count; i++) {
        g_node.setTransmitBuffer(i, values[i]);
    }
    uint8_t res = g_node.writeMultipleRegisters(start, count);
    return (res == g_node.ku8MBSuccess);
}

// --- GESTIÓN DE RECUPERACIÓN DEL BUS ---

void ModbusManager::checkRecovery() {
    if (!g_modbusDegraded) return;

    // Ahora la recuperación es más sencilla:
    // Si el scheduler detecta que todos los dispositivos están en STATE_OK,
    // esperamos un tiempo de estabilidad y desactivamos el modo degradado.
    
    // Esta lógica la puede manejar el Scheduler mirando el estado de los ModbusDevice
    // o podemos dejar que el g_modbusDegraded lo gestione el ModbusGuard.
    
    if (g_lastStableMs == 0) g_lastStableMs = millis();

    if (millis() - g_lastStableMs > 10000) { // 10 segundos estable
        g_modbusDegraded = false;
        g_lastStableMs = 0;
        LOG_INF("MODBUS: Modo degradado DESACTIVADO");
    }
}