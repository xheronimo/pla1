#include "SDManager.h"

namespace SDMgr {

    static SemaphoreHandle_t _sdMutex = NULL;

    bool begin(int csPin) {
        if (_sdMutex == NULL) {
            _sdMutex = xSemaphoreCreateMutex();
        }

        lock();
        if (!SD.begin(csPin)) {
            unlock();
            Serial.println(F("[SD] Error de montaje hardware."));
            return false;
        }

        // Crear directorios si no existen
        if (!SD.exists("/logs"))   SD.mkdir("/logs");
        if (!SD.exists("/config")) SD.mkdir("/config");
        unlock();

        manageSpace(); 
        return true;
    }

    void lock() {
        if (_sdMutex != NULL) xSemaphoreTake(_sdMutex, portMAX_DELAY);
    }

    void unlock() {
        if (_sdMutex != NULL) xSemaphoreGive(_sdMutex);
    }

    bool loadConfig(const char* filename, JsonDocument& doc) {
        lock();
        File file = SD.open(filename, FILE_READ);
        if (!file) {
            unlock();
            return false;
        }
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        unlock();
        return (error == DeserializationError::Ok);
    }

    bool saveConfig(const char* filename, const JsonDocument& doc) {
        lock();
        // Abrimos en modo escritura (sobreescribe el archivo anterior)
        File file = SD.open(filename, FILE_WRITE);
        if (!file) {
            unlock();
            return false;
        }
        
        // Serializamos el JSON directamente al archivo
        size_t bytesWritten = serializeJson(doc, file);
        file.close();
        unlock();
        
        return (bytesWritten > 0);
    }

    void logEvent(const char* level, const char* message) {
        char path[MAX_PATH_LEN];
        struct tm timeinfo;
        bool hasTime = getLocalTime(&timeinfo);

        // Rotación Diaria
        if (hasTime) {
            strftime(path, sizeof(path), "/logs/%Y%m%d.log", &timeinfo);
        } else {
            snprintf(path, sizeof(path), "/logs/system.log");
        }

        lock();
        File logFile = SD.open(path, FILE_APPEND);
        if (logFile) {
            char t_str[32] = "00:00:00";
            if (hasTime) strftime(t_str, sizeof(t_str), "%H:%M:%S", &timeinfo);
            
            logFile.printf("[%s] [%s] %s\n", t_str, level, message);
            logFile.close();
        }
        unlock();

        // Verificación de espacio cada 100 logs
        static int entryCount = 0;
        if (++entryCount >= 100) {
            manageSpace();
            entryCount = 0;
        }
    }

    void manageSpace() {
        lock();
        uint64_t total = SD.totalBytes();
        uint64_t used = SD.usedBytes();
        if (total == 0) { unlock(); return; }

        float occupancy = (used * 100.0) / total;

        // Recirculación FIFO (First-In-First-Out)
        if (occupancy > 90.0) {
            File root = SD.open("/logs");
            if (root) {
                File file = root.openNextFile();
                if (file) {
                    char toDelete[MAX_PATH_LEN];
                    snprintf(toDelete, sizeof(toDelete), "/logs/%s", file.name());
                    file.close();
                    SD.remove(toDelete);
                }
                root.close();
            }
        }
        unlock();
    }
}