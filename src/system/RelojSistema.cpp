#include "system/RelojSistema.h"
#include "storage/SDManager.h"
#include "i2c/driver/ds3231_driver.h"
#include "system/WatchdogManager.h"
#include "config/config_global.h"
#include <sys/time.h>

extern SemaphoreHandle_t semI2C;
extern Configuracion cfg;
static bool rtcPresente = false;
static TaskHandle_t ntpTaskHandle = nullptr;

namespace RelojSistema {

    // Helper para validar si el timestamp es coherente (posterior a 2001)
    bool horaValida(time_t t) {
        return t > 1000000000;
    }

    static void setSystemTime(time_t t) {
        struct timeval tv = {.tv_sec = t, .tv_usec = 0};
        settimeofday(&tv, nullptr);
    }

    void inicializar() {
        if (!cfg.rtc.enabled) return;

        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(500)) == pdTRUE) {
            rtcPresente = ds3231Detect(0x68);
            xSemaphoreGive(semI2C);
        }
        
        SDMgr::logEvent(rtcPresente ? "INFO" : "WARN", 
                        rtcPresente ? "RTC: Detectado en 0x68" : "RTC: No detectado");
    }

    void cargarHoraInicial() {
        time_t rtcTime = 0;
        time_t sdBackupTime = 0;

        // 1. Intentar leer del RTC Físico
        if (rtcPresente) {
            if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(500)) == pdTRUE) {
                uint16_t y; uint8_t m, d, hh, mm, ss;
                if (ds3231ReadTime(y, m, d, hh, mm, ss)) {
                    struct tm t = {0};
                    t.tm_year = y - 1900;
                    t.tm_mon  = m - 1;
                    t.tm_mday = d;
                    t.tm_hour = hh;
                    t.tm_min  = mm;
                    t.tm_sec  = ss;
                    rtcTime = mktime(&t);
                }
                xSemaphoreGive(semI2C);
            }
        }

        // 2. Intentar leer último tiempo guardado en SD (Sustituye a LittleFS)
        JsonDocument doc;
        if (SDMgr::loadConfig("/config/time_backup.json", doc)) {
            sdBackupTime = doc["last_unix"] | 0;
        }

        // 3. Lógica de prioridad: RTC > SD
        if (horaValida(rtcTime)) {
            setSystemTime(rtcTime);
            SDMgr::logEvent("INFO", "RELOJ: Sincronizado con RTC.");
        } else if (horaValida(sdBackupTime)) {
            setSystemTime(sdBackupTime);
            SDMgr::logEvent("WARN", "RELOJ: RTC inválido. Usando backup de SD.");
        } else {
            SDMgr::logEvent("CRITICAL", "RELOJ: Sin hora válida. El sistema inicia en 1970.");
        }
    }

    static void taskNtp(void*) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Esperar estabilidad de red
        configTime(cfg.rtc.timezone * 3600, 0, "pool.ntp.org", "time.google.com");

        for (;;) {
            watchdogKick(WDT_RELOJ);
            time_t now = time(nullptr);

            if (horaValida(now)) {
                // 1. Guardar en SD como backup 💾
                JsonDocument doc;
                doc["last_unix"] = (uint32_t)now;
                SDMgr::saveConfig("/config/time_backup.json", doc);

                // 2. Actualizar RTC físico si está presente 🕒
                if (rtcPresente) {
                    struct tm t;
                    localtime_r(&now, &t);
                    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
                        ds3231WriteTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                                        t.tm_hour, t.tm_min, t.tm_sec);
                        xSemaphoreGive(semI2C);
                    }
                }
                
                SDMgr::logEvent("INFO", "NTP: Sincronización exitosa. Backup en SD actualizado.");
                
                ntpTaskHandle = nullptr;
                vTaskDelete(nullptr);
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    void iniciarTaskNtp() {
        if (ntpTaskHandle) return;
        xTaskCreatePinnedToCore(taskNtp, "NTP", 4096, nullptr, 1, &ntpTaskHandle, 1);
    }

    void ajustarRelojInterno(uint16_t year, uint8_t month, uint8_t day, 
                            uint8_t hour, uint8_t minute, uint8_t second) {
        struct tm t = {0};
        t.tm_year = year - 1900;
        t.tm_mon  = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min  = minute;
        t.tm_sec  = second;
        
        time_t t_now = mktime(&t);
        setSystemTime(t_now);

        if (rtcPresente) {
            if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
                ds3231WriteTime(year, month, day, hour, minute, second);
                xSemaphoreGive(semI2C);
            }
        }

        JsonDocument doc;
        doc["last_unix"] = (uint32_t)t_now;
        SDMgr::saveConfig("/config/time_backup.json", doc);
    }

    void obtenerISO8601(char* buffer, size_t size) {
        struct tm t;
        if (!getLocalTime(&t)) {
            strncpy(buffer, "1970-01-01 00:00:00", size);
            return;
        }
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
    }

    void obtenerHoraHHMM(char* buf) {
        time_t now = time(nullptr);
        struct tm t;
        localtime_r(&now, &t);
        snprintf(buf, 6, "%02d:%02d", t.tm_hour, t.tm_min);
    }
}