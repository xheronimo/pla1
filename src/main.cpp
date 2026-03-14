#include <Arduino.h>
#include <SD.h>

// --- Sistema y Logs ---
#include "system/boot_reason.h"
#include "system/LogSystem.h"
#include "system/nvs_store.h"
#include "system/system_fault.h"
#include "system/fault_output.h"
#include "system/system_safe_mode.h"

// --- Hardware y Señales (NUEVOS) ---
#include "chip/chip_manager.h"
#include "signal/signal_manager.h"

// --- Alarmas (NUEVOS) ---
#include "alarm/alarm_manager.h"
#include "alarm/alarm_json_loader.h"

// --- Comunicaciones y Web ---
#include "web/web_manager.h"
#include "config/config_loader.h"
#include "config/config_apply.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_profiles.h"
#include "modbus/modbus_rollback.h"

// --- Tareas y Autogen ---
#include "task/task_boot.h"
#include "onewire/signal_autogen_onewire.h"

// --------------------------------------------------
// VARIABLES GLOBALES Y SAFE MODE
// --------------------------------------------------
extern bool g_safeMode;
extern uint32_t g_lastResetWdt; // Viene de boot_reason

void setup() {
    g_safeMode = false;
    Serial.begin(115200);
    delay(500); // Estabilización

    // 1️⃣ INICIALIZACIÓN FÍSICA Y LOGS
    // Intentamos arrancar la SD primero para poder escribir logs de boot
    inicializarSD(); 
    logBootReason();

    // 2️⃣ NVS Y SEGURIDAD (Safe Mode)
    systemSafeModeInit(); 
    escribirLog("SYS: SafeMode=%s", systemInSafeMode() ? "ON" : "OFF");

    // 3️⃣ INICIALIZACIÓN DE HARDWARE (ChipManager)
    // Registro de PCF8574, RTC, EEPROM y asignación de pines FREE_GPIO
    // Se asegura de apagar relés antes de que corra la lógica
    ChipMgr::init();

    // 4️⃣ SISTEMA DE FALLOS (Fault System)
    systemFaultInit();
    faultOutputInit();

    // 5️⃣ CARGA DE CONFIGURACIÓN GLOBAL
    Configuracion cfg;
    cargarConfiguracion(cfg, SystemMode::NORMAL);

    if (systemInSafeMode()) {
        escribirLog("SYS: Arranque en SAFE MODE detectado.");
        cfg.systemMode = SystemMode::RECOVERY;
    }
    aplicarConfiguracionGlobal(cfg);

    // 6️⃣ CARGA DINÁMICA DE SEÑALES Y ALARMAS
    // Inicializamos el gestor de señales
    SignalMgr::init(); 

    if (!systemInSafeMode()) {
        // Cargamos las alarmas desde el JSON de la SD (Sin límites de MAX_ALARM)
        // Esto crea el vector dinámico y los árboles de expresiones
        if (AlarmMgr::loadFromFS(SD, "/config/alarms.json")) {
            escribirLog("ALARM: Configuración cargada desde SD");
        } else {
            escribirLog("ALARM: Error cargando configuración o archivo inexistente");
        }
    }

    // 7️⃣ CONFIGURACIÓN DE BUSES (Modbus / OneWire)
    if (!systemInSafeMode()) {
        // A. Perfiles Modbus desde LittleFS/NVS
        ModbusProfiles::loadDynamicProfiles();
        
        if (ModbusRollback::performRollback()) {
            escribirLog("MODBUS: Configuración estable restaurada.");
        }

        // B. Inicializar Modbus en el puerto Serial configurado en ChipMgr
        ModbusManager::init(Serial1, 9600); 

        // C. Autogeneración de señales basada en hardware detectado
        SignalAutogenOneWire::generateAll();
        // SignalAutogenModbus::generateAll(); // Pendiente de implementación
    }

    // 8️⃣ LANZAMIENTO DE TAREAS (RTOS)
    // Las tareas mínimas incluyen Servidor Web básico y mantenimiento de sistema
    arrancarTareasMinimas();

    if (!systemInSafeMode()) {
        // Las tareas normales incluyen el "PLC Scan" que ejecuta:
        // SignalMgr::updateAll() y AlarmMgr::update()
        arrancarTareasNormales();
    }

    escribirLog("SYS: Setup finalizado correctamente.");
}

/**
 * El loop de Arduino se mantiene casi vacío porque el sistema
 * corre sobre FreeRTOS. Se usa para tareas de muy baja prioridad
 * o simplemente para liberar CPU.
 */
void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}