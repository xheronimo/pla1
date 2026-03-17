#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace SDMgr {
    const size_t MAX_PATH_LEN = 64;

    // Inicialización
    bool begin(int csPin);

    // Gestión de Archivos JSON (ArduinoJson 7)
    bool loadConfig(const char* filename, JsonDocument& doc);
    bool saveConfig(const char* filename, const JsonDocument& doc);

    // Sistema de Logs con Rotación y Recirculación
    void logEvent(const char* level, const char* message);

    // Mantenimiento de Espacio (FIFO)
    void manageSpace();

    // Utilidades de Bloqueo para Multi-tarea
    void lock();
    void unlock();
}

#endif