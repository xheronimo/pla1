#include "alarm_queue.h"
#include "system/system_sync.h"

#include <queue>

static std::queue<AlarmEvent> q;
static SemaphoreHandle_t queueMutex = nullptr;

void alarm_queueInit()
{
    if (!queueMutex)
        queueMutex = xSemaphoreCreateMutex();
}

void alarm_queuePush(const AlarmEvent& ev)
{
    if (!queueMutex) return;

    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE)
    {
        q.push(ev);
        xSemaphoreGive(queueMutex);
    }
}

bool alarm_queuePop(AlarmEvent& ev)
{
    if (!queueMutex) return false;

    bool ok = false;
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE)
    {
        if (!q.empty())
        {
            ev = q.front();
            q.pop();
            ok = true;
        }
        xSemaphoreGive(queueMutex);
    }
    return ok;
}

size_t alarm_queueSize()
{
    if (!queueMutex) return 0;

    size_t s = 0;
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE)
    {
        s = q.size();
        xSemaphoreGive(queueMutex);
    }
    return s;
}
