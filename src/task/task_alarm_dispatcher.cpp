#include "tasks/task_alarm_dispatcher.h"
#include "alarm/alarm_manager.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_router.h"
#include "system/LogSystem.h"
#include "system/WatchdogManager.h" // <--- Inyectado
#include <Arduino.h>

static void taskAlarmDispatcher(void* pvParameters) {
    (void) pvParameters;
    AlarmEvent event;

    // Registro en el canal correspondiente
    watchdogRegister(WDT_ALARM_DISPATCH);

    escribirLog("DISPATCHER: Tarea iniciada en Core 0.");

    for (;;) {
        // Kick constante: "Estoy listo para procesar eventos"
        watchdogKick(WDT_ALARM_DISPATCH);

        // 1. Esperar evento (Pop bloqueante)
        // Nota: Si el timeout de la cola es infinito, el WDT fallará. 
        // Se recomienda que alarm_queuePop tenga un timeout máximo de 1s.
        if (alarm_queuePop(&event)) {
            
            // 2. Delegar al Router (MQTT, Telegram, etc.)
            if (!AlarmRouter::dispatch(event)) {
                // Error de enrutamiento
            }
        }

        // Delay de cortesía
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startAlarmDispatcherTask() {
    xTaskCreatePinnedToCore(
        taskAlarmDispatcher,
        "AlarmDispatch",
        8192, 
        nullptr,
        3,    // Prioridad 3
        nullptr,
        0     // Core 0
    );
}