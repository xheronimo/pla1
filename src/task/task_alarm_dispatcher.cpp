#include "task_alarm_dispatcher.h"
#include "alarm/alarm_manager.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_router.h" // <--- Importante: El router ahora gestiona los envíos
#include "system/LogSystem.h"
#include <Arduino.h>

/**
 * Tarea de despacho de eventos de alarma.
 * Corre en el Core 0 para no interferir con el ciclo de control del PLC (Core 1).
 */
static void taskAlarmDispatcher(void* pvParameters) {
    (void) pvParameters;
    AlarmEvent event;

    escribirLog("DISPATCHER: Tarea iniciada en Core 0. Esperando eventos...");

    for (;;) {
        // 1. Esperar evento de la cola (Bloqueo eficiente mediante xQueueReceive dentro de pop)
        // Si no hay alarmas, esta tarea no consume CPU.
        if (alarm_queuePop(&event)) {
            
            // 2. Delegar el envío al Router
            // El Router decidirá si envía MQTT, Telegram, Web, etc.
            if (!AlarmRouter::dispatch(event)) {
                // Si falla, puede ser porque el ID de alarma ya no existe
                // Serial.println("[Dispatcher] Error al procesar evento.");
            }
        }

        // Delay mínimo de cortesía para el planificador de FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startAlarmDispatcherTask() {
    xTaskCreatePinnedToCore(
        taskAlarmDispatcher,
        "AlarmDispatch",
        8192,         // Stack suficiente para TLS/SSL (Telegram/MQTT)
        nullptr,
        3,            // Prioridad baja: Las comunicaciones no deben frenar al hardware
        nullptr,
        0             // Core 0: Especializado en tareas de red y WiFi
    );
}