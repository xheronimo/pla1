#include "system/RelojSistema.h"
#include "config/config_global.h"
#include "system/system_sync.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"
#include "i2c/driver/ds3231_driver.h" // Driver nativo
#include "i2c/i2c_bus.h"
#include <LittleFS.h>
#include <WiFi.h>
#include <time.h>

static bool rtcPresente = false;
static TaskHandle_t ntpTaskHandle = nullptr;

extern Configuracion cfg;
extern SemaphoreHandle_t semI2C; 
extern SemaphoreHandle_t semFS;  

// ==================================================
// 🛠️ UTILIDADES
// ==================================================

static bool horaValida(time_t t) {
    return t > 1000000000; // Posterior a 2001
}

static void setSystemTime(time_t t) {
    struct timeval tv = {.tv_sec = t, .tv_usec = 0};
    settimeofday(&tv, nullptr);
}

// ==================================================
// 🕒 INICIALIZACIÓN
// ==================================================

void inicializarRTC() {
    if (!cfg.rtc.enabled) return;

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
        rtcPresente = ds3231Detect(0x68);
        xSemaphoreGive(semI2C);
    }
    escribirLog(rtcPresente ? "RTC: OK" : "RTC: NO");
}

void cargarHoraInicial() {
    time_t rtcTime = 0;
    time_t fsTime = 0;

    if (rtcPresente) {
        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
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

    if (xSemaphoreTake(semFS, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (LittleFS.exists("/time.dat")) {
            File f = LittleFS.open("/time.dat", "r");
            if (f) { fsTime = f.parseInt(); f.close(); }
        }
        xSemaphoreGive(semFS);
    }

    if (horaValida(rtcTime)) setSystemTime(rtcTime);
    else if (horaValida(fsTime)) setSystemTime(fsTime);
}

// ==================================================
// 🌐 TAREA NTP (ACTUALIZADA)
// ==================================================

static void taskNtp(void*) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    configTime(cfg.rtc.timezone * 3600, 0, "pool.ntp.org");

    for (;;) {
        watchdogKick(WDT_RELOJ);
        time_t now = time(nullptr);

        if (horaValida(now)) {
            // 1. Guardar en LittleFS 💾
            if (xSemaphoreTake(semFS, pdMS_TO_TICKS(200)) == pdTRUE) {
                File f = LittleFS.open("/time.dat", "w");
                if (f) { f.print((uint32_t)now); f.close(); }
                xSemaphoreGive(semFS);
            }

            // 2. Actualizar RTC físico 🕒
            if (rtcPresente) {
                struct tm t;
                localtime_r(&now, &t);
                if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
                    ds3231WriteTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                                    t.tm_hour, t.tm_min, t.tm_sec);
                    xSemaphoreGive(semI2C);
                }
            }
            vTaskDelete(nullptr);
            ntpTaskHandle = nullptr;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void iniciarTaskNtp() {
    if (ntpTaskHandle) return;
    xTaskCreatePinnedToCore(taskNtp, "NTP", 4096, nullptr, 1, &ntpTaskHandle, 0);
}

// ==================================================
// 📝 AJUSTE Y FORMATO
// ==================================================

void ajustarRelojInterno(uint16_t year, uint8_t month, uint8_t day, 
                          uint8_t hour, uint8_t minute, uint8_t second) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;
    setSystemTime(mktime(&t));

    if (rtcPresente) {
        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
            ds3231WriteTime(year, month, day, hour, minute, second);
            xSemaphoreGive(semI2C);
        }
    }
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