#include "system/LogSystem.h"
#include "system/system_sync.h"
#include <SD.h>
#include <stdarg.h>
#include "board/board_config.h"

// Instancia interna y Cola
static LogSystem internalLog;
QueueHandle_t logQueue = nullptr;

static bool sdDisponible = false; // Flag para saber si hay SD física

// ==================================================
// IMPLEMENTACIÓN DE LA CLASE LogSystem
// ==================================================
LogSystem::LogSystem() : head(0), isFull(false) {
    memset(buffer, 0, LOG_RAM_MAX);
    mutex = xSemaphoreCreateMutex();
}

void LogSystem::addLog(const char* text) {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10))) {
        size_t len = strlen(text);
        for (size_t i = 0; i < len; i++) {
            buffer[head] = text[i];
            head = (head + 1) % LOG_RAM_MAX;
            if (head == 0) isFull = true;
        }
        // Aseguramos terminador nulo para coherencia interna
        if (head < LOG_RAM_MAX) buffer[head] = '\0';
        
        xSemaphoreGive(mutex);
    }
}

String LogSystem::getLogs() {
    String out = "";
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50))) {
        out.reserve(LOG_RAM_MAX);
        if (!isFull) {
            out = String(buffer);
        } else {
            for (size_t i = head; i < LOG_RAM_MAX; i++) {
                if (buffer[i] != '\0') out += buffer[i];
            }
            for (size_t i = 0; i < head; i++) {
                if (buffer[i] != '\0') out += buffer[i];
            }
        }
        xSemaphoreGive(mutex);
    }
    return out;
}

void LogSystem::clear() {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50))) {
        memset(buffer, 0, LOG_RAM_MAX);
        head = 0;
        isFull = false;
        xSemaphoreGive(mutex);
    }
}

// ==================================================
// TASK ESCRITURA SD
// ==================================================


void tareaEscrituraSD(void *parameter)
{
    char *msg;

    for (;;)
    {
        if (xQueueReceive(logQueue, &msg, portMAX_DELAY))
        {
            if (sdDisponible && xSemaphoreTake(semSD, pdMS_TO_TICKS(200)))
            {
                File f = SD.open("/L.txt", FILE_APPEND);
                if (f)
                {
                    f.println(msg);
                    f.close();
                }
                xSemaphoreGive(semSD);
            }

            free(msg);   // 🔐 SIEMPRE liberar
        }
    }
}
// ==================================================
// FUNCIONES GLOBALES (API COMPATIBLE)
// ==================================================
bool inicializarSD() {
    // 1. Intentar arrancar hardware SD
    // Si falla (V1 sin módulo), marcamos sdDisponible como false
    if (SD_CS == -1) {
        sdDisponible = false;
        Serial.println("W:SD: No configurada (Ignorando escritura en archivo)");
    } else if (!SD.begin(SD_CS)) {
        sdDisponible = false;
        Serial.println("W:SD: No detectada (Ignorando escritura en archivo)");
    } else {
        sdDisponible = true;
        Serial.println("I:SD: Detectada y lista");
    }
    if (!SD.begin(SD_CS)) {
        sdDisponible = false;
        Serial.println("W:SD: No detectada (Ignorando escritura en archivo)");
    } else {
        sdDisponible = true;
        Serial.println("I:SD: Detectada y lista");
    }

    // 2. Creamos la cola de todos modos (para que escribirLog no falle)
    logQueue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(char *));
    
    // 3. Creamos la tarea (siempre viva para vaciar la cola, incluso sin SD)
    xTaskCreatePinnedToCore(
        tareaEscrituraSD,
        "LOG_SD_TASK",
        4096,
        nullptr,
        1,
        nullptr,
        0);

    escribirLog("SYS:LogSystem iniciado (%s SD)", sdDisponible ? "CON" : "SIN");
    return sdDisponible;
}

void escribirLog(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // 1. Guardar en RAM (Clase interna circular)
    internalLog.addLog(buf);
    internalLog.addLog("\n");

    // 2. Enviar a SD
    if (logQueue) {
        char *m = strdup(buf);
        if (m) {
            if (xQueueSend(logQueue, &m, 0) != pdPASS) {
                free(m); // Evita fuga si la cola se llena
            }
        }
    }
}

String obtenerLogSistema() {
    return internalLog.getLogs();
}

void limpiarLogSistema() {
    internalLog.clear();
}