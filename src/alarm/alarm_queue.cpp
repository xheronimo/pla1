#include "alarm/alarm_queue.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Definimos un tamaño máximo para no agotar la RAM si hay una ráfaga
#define ALARM_QUEUE_SIZE 50

static QueueHandle_t _alarmQueue = nullptr;

void alarm_queueInit() {
    if (_alarmQueue == nullptr) {
        _alarmQueue = xQueueCreate(ALARM_QUEUE_SIZE, sizeof(AlarmEvent));
    }
}

bool alarm_queuePush(const AlarmEvent& event) {
    if (_alarmQueue == nullptr) return false;

    // xQueueSend es thread-safe. 
    // Usamos portMAX_DELAY = 0 para que no bloquee la tarea de control (PLC)
    // Si la cola está llena, el evento se descarta para no frenar el PLC.
    BaseType_t ret = xQueueSend(_alarmQueue, &event, 0);
    return (ret == pdPASS);
}

bool alarm_queuePop(AlarmEvent* event) {
    if (_alarmQueue == nullptr) return false;

    // xQueueReceive bloquea la tarea Dispatcher hasta que llegue algo
    // o hasta que pase el tiempo de timeout (ej: 100ms)
    BaseType_t ret = xQueueReceive(_alarmQueue, event, pdMS_TO_TICKS(100));
    return (ret == pdPASS);
}