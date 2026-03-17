#include "modbus/modbus_scheduler.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_profiles.h"
#include "modbus/modbus_state.h"
#include "chip/chip_manager.h"         // Nuevo: Interfaz con el hardware
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"
#include "system/system_safe_mode.h"
#include <Arduino.h>

// Variables globales de estado del bus (asumimos que están definidas en modbus_state.h)
extern ModbusBusState g_modbusBusState;
extern float g_modbusLoad;
extern uint32_t g_peticionesOk;
extern String g_busStatusStr;

/**
 * @brief Ejecuta el polling físico y deposita los datos en el Chip correspondiente.
 */
bool executePolling(ModbusDevice& dev) {
    // 1. Buscamos el Chip asociado a este dispositivo Modbus
    // Usamos el nombre o el ID único del dispositivo para vincularlo al ChipMgr
    Chip* chip = ChipMgr::getById(dev.name); 
    if (!chip) return false;

    bool success = false;

    // 2. Lógica por Fingerprint (Perfiles Nativos)
    // Nota: Debes actualizar estas funciones en modbus_profiles.cpp para que acepten (Chip* chip)
    if (dev.fingerprint == 0x5172)      success = ModbusProfiles::Native::pollSHT20(chip);
    else if (dev.fingerprint == 0x0630) success = ModbusProfiles::Native::pollSDM630(chip);
    else if (dev.fingerprint == 0x0420) success = ModbusProfiles::Native::pollADC4CH(chip);

    // 3. Perfiles Dinámicos (Lectura por bloques JSON)
    else {
        for (auto& prof : ModbusProfiles::g_dynamicProfiles) {
            if (prof.fp == dev.fingerprint) {
                for (auto& block : prof.blocks) {
                    uint16_t buffer[block.count];
                    
                    // Lectura física del bus
                    bool blockOk = (block.fc == 4) ? 
                        ModbusManager::readInputRaw(dev.id, block.start, block.count, buffer) :
                        ModbusManager::readHoldingRaw(dev.id, block.start, block.count, buffer);
                    
                    if (blockOk) {
                        // RELLENAR CANALES DEL CHIP
                        // Mapeamos los datos del buffer a los canales del chip según el perfil
                        for (auto& sigDef : block.signals) {
                            float val = 0;
                            if (strcmp(sigDef.type, "float32") == 0) {
                                val = ModbusProfiles::pf32(buffer[sigDef.pos], buffer[sigDef.pos+1]);
                            } else {
                                val = (int16_t)buffer[sigDef.pos] * sigDef.factor;
                            }

                            // Actualizamos el canal físico del chip (posicionamiento por índice)
                            if (sigDef.channelIndex < chip->channels.size()) {
                                chip->channels[sigDef.channelIndex].rawValue = val;
                                chip->channels[sigDef.channelIndex].valid = true;
                                chip->channels[sigDef.channelIndex].lastUpdateMs = millis();
                            }
                        }
                        success = true;
                    } else {
                        success = false;
                        break; // Si falla un bloque, el chip entero se marca como error
                    }
                }
                break;
            }
        }
    }

    // Actualizamos la salud del hardware en el ChipMgr
    chip->online = success;
    if (success) chip->lastSeenMs = millis();

    return success;
}

/**
 * @brief Tarea de FreeRTOS para el escaneo cíclico de dispositivos.
 */
void taskModbusPoll(void* pv) {
    uint32_t startTime = millis();
    uint16_t counterOk = 0;
    uint32_t activeBusTime = 0;

    LOG_INF("Modbus Task iniciada en Core 1");

    for (;;) {
        // Registro de actividad para el Watchdog
        watchdogFeed(WDT_MODBUS);

        // Si el bus está bloqueado o en recuperación, esperamos
        if (g_modbusBusState != ModbusBusState::NORMAL) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        uint32_t devOk = 0, devFail = 0;

        // Recorremos los dispositivos configurados (usando for-each para modernizar)
        for (size_t i = 0; i < g_modbusDeviceCount; i++) {
            ModbusDevice& dev = g_modbusDevices[i];
            
            if (!dev.enabled || dev.provisioning) continue;

            // LÓGICA DE TIEMPOS (Polling + Backoff por errores)
            uint32_t nextPollInterval = dev.pollMs * (1 + dev.errorCount);
            if (nextPollInterval > 60000) nextPollInterval = 60000; // Cap a 1 min

            if (millis() - dev.lastPollMs < nextPollInterval) continue;
            
            // EJECUCIÓN DE LECTURA
            uint32_t tStart = millis();
            bool ok = executePolling(dev); 
            uint32_t duration = millis() - tStart;
            
            activeBusTime += duration;
            dev.lastPollMs = millis();

            if (ok) {
                dev.errorCount = 0;
                dev.state = ModbusDeviceState::STATE_OK;
                devOk++;
                counterOk++;
            } else {
                dev.errorCount++;
                dev.state = (dev.errorCount > 3) ? ModbusDeviceState::STATE_FAULT : ModbusDeviceState::STATE_TIMEOUT;
                devFail++;
                
                // Si falla demasiado, auto-desactivamos para no saturar el bus
                if (dev.errorCount >= 20) {
                    dev.enabled = false;
                    LOG_ERR("Dispositivo %s auto-desactivado por errores críticos", dev.name);
                }
            }
            
            // Pequeño respiro entre dispositivos para no bloquear otras tareas I/O
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // CÁLCULO DE ESTADÍSTICAS (Cada 2 segundos)
        if (millis() - startTime > 2000) {
            g_modbusLoad = (activeBusTime / 2000.0f) * 100.0f;
            g_peticionesOk = counterOk / 2;
            g_busStatusStr = (devOk > 0 && devFail == 0) ? "OK" : (devOk > 0 ? "DEGRADED" : "DOWN");
            
            startTime = millis();
            activeBusTime = 0;
            counterOk = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Lanza la tarea de Modbus.
 */
void startModbusTask() {
    if (systemInSafeMode()) return;

    // Inicialización del driver de bajo nivel (Asumimos parámetros configurados)
    ModbusManager::init(Serial2, 9600); 

    xTaskCreatePinnedToCore(
        taskModbusPoll,
        "MODBUS_TASK",
        4096,
        nullptr,
        2,        // Prioridad media-alta
        nullptr,
        1         // Ejecutar en Core 1 (Core 0 para WiFi/Sistema)
    );
}