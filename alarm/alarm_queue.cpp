#include "alarm_queue.h"
#include "system/system_sync.h"

#include <Arduino.h>
#include <queue>

static std::queue<AlarmEvent> alarmQueue;

void alarm_queuePush(const AlarmEvent& ev)
{
    if (!semQueue) return;

    xSemaphoreTake(semQueue, portMAX_DELAY);
    alarmQueue.push(ev);
    xSemaphoreGive(semQueue);
}

bool alarm_queuePop(AlarmEvent& ev)
{
    if (!semQueue) return false;

    xSemaphoreTake(semQueue, portMAX_DELAY);

    if (alarmQueue.empty())
    {
        xSemaphoreGive(semQueue);
        return false;
    }

    ev = alarmQueue.front();
    alarmQueue.pop();

    xSemaphoreGive(semQueue);
    return true;
}

size_t alarm_queueSize()
{
    if (!semQueue) return 0;

    xSemaphoreTake(semQueue, portMAX_DELAY);
    size_t s = alarmQueue.size();
    xSemaphoreGive(semQueue);
    return s;
}
