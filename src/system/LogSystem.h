#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <Arduino.h>

#define LOG_RAM_MAX 2500
#define LOG_QUEUE_SIZE 15

class LogSystem {
private:
    char buffer[LOG_RAM_MAX];
    size_t head;
    bool isFull;
    SemaphoreHandle_t mutex;

public:
    LogSystem();
    void addLog(const char* text);
    String getLogs();
    void clear();
};

// Funciones globales y de inicialización
bool inicializarSD();
void escribirLog(const char *fmt, ...);
String obtenerLogSistema();
void limpiarLogSistema();

#endif