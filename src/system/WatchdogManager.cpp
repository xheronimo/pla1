#include "system/WatchdogManager.h"
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "system/LogSystem.h"

static uint32_t g_timeoutMs = 30000;
static uint32_t g_lastKick[WDT_MAX];
static bool g_registered[WDT_MAX];

void watchdogInit(uint32_t timeoutMs) {
    g_timeoutMs = timeoutMs;
    uint32_t now = millis();

    for (int i = 0; i < WDT_MAX; i++) {
        g_registered[i] = false;
        g_lastKick[i] = now;
    }

    // --- SOLUCIÓN AL ERROR DE COMPILACIÓN ---
    // En versiones anteriores (v4.x), esp_task_wdt_init recibe directamente 
    // los segundos y un booleano para el pánico.
    
    uint32_t timeoutSeconds = g_timeoutMs / 1000;
    if (timeoutSeconds < 1) timeoutSeconds = 1;

    // Inicialización compatible: segundos, habilitar pánico (reset)
    esp_task_wdt_init(timeoutSeconds, true); 
    
    // Suscribir la tarea actual (normalmente la de polling o main)
    esp_task_wdt_add(NULL); 

    escribirLog("WDT: Sistema de vigilancia iniciado. Timeout: %u ms", g_timeoutMs);
}

void watchdogRegister(WatchdogId id) {
    if (id < WDT_MAX) {
        g_registered[id] = true;
        g_lastKick[id] = millis();
    }
}

void watchdogKick(WatchdogId id) {
    if (id < WDT_MAX && g_registered[id]) {
        g_lastKick[id] = millis();
    }
}

void watchdogCheck() {
    uint32_t now = millis();
    bool faultDetected = false;
    int faultId = -1;

    for (int i = 0; i < WDT_MAX; i++) {
        if (!g_registered[i]) continue;

        if (now - g_lastKick[i] > g_timeoutMs) {
            faultDetected = true;
            faultId = i;
            break; 
        }
    }

    if (faultDetected) {
        // Log previo al reset
        escribirLog("WDT FATAL: Modulo ID %d colgado. Ultimo kick hace %lu ms. Reiniciando...", 
                    faultId, now - g_lastKick[faultId]);
        
        // Pequeño delay para que el driver de la SD o el Serial terminen de escribir
        delay(100); 
        esp_restart();
    } else {
        // Alimentamos al hardware WDT si todo el software ha reportado salud
        esp_task_wdt_reset();
    }
}

uint32_t watchdogLastKickAgo(WatchdogId id) {
    if (id >= WDT_MAX || !g_registered[id]) return 0;
    return millis() - g_lastKick[id];
}

bool watchdogIsRegistered(WatchdogId id) {
    return (id < WDT_MAX) ? g_registered[id] : false;
}