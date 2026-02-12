#include "system/LogSystem.h"
#include "system/system_sync.h"

#include <SD.h>

QueueHandle_t logQueue;
const int QUEUE_SIZE = 15;

static String logRAM;
static const size_t LOG_RAM_MAX = 2500;

// ==================================================
// TASK ESCRITURA SD
// ==================================================
void tareaEscrituraSD(void * parameter)
{
    char* msg;

    for (;;)
    {
        if (xQueueReceive(logQueue, &msg, portMAX_DELAY))
        {
            #ifdef MODO_DEBUG
                Serial.println(msg);
            #endif

            if (xSemaphoreTake(semSD, pdMS_TO_TICKS(100)))
            {
                File f = SD.open("/L.txt", FILE_APPEND);
                if (f)
                {
                    f.println(msg);
                    f.close();
                }
                xSemaphoreGive(semSD);
            }

            free(msg);
        }
    }
}

// ==================================================
// INIT SD + LOG
// ==================================================
bool inicializarSD()
{
    if (!SD.begin(SD_CS))
        return false;

    logQueue = xQueueCreate(QUEUE_SIZE, sizeof(char*));
    if (!logQueue)
        return false;

    xTaskCreatePinnedToCore(
        tareaEscrituraSD,
        "LOG_SD",
        2048,
        nullptr,
        1,
        nullptr,
        0
    );

    escribirLog("I:SD:OK");
    return true;
}

// ==================================================
// LOG API
// ==================================================
void escribirLog(const char* fmt, ...)
{
    if (!logQueue)
        return;

    char buf[80];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // RAM circular
    logRAM += buf;
    logRAM += "\n";
    if (logRAM.length() > LOG_RAM_MAX)
        logRAM.remove(0, logRAM.length() - LOG_RAM_MAX);

    char* m = strdup(buf);
    if (!m)
        return;

    if (xQueueSend(logQueue, &m, 0) != pdPASS)
        free(m);
}

String obtenerLogSistema()
{
    return logRAM;
}

void limpiarLogSistema()
{
    logRAM = "";
}
