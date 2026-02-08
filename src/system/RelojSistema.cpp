#include "system/RelojSistema.h"

#include "config/config_global.h"
#include "system/system_sync.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

#include <RTClib.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <time.h>

// ==================================================
// RTC
// ==================================================
static RTC_DS3231 rtc;
static bool rtcPresente = false;
static TaskHandle_t ntpTaskHandle = nullptr;
static time_t ultimaHoraFS = 0;

extern Configuracion cfg;

// ==================================================
// UTILIDADES
// ==================================================
static bool horaValida(time_t t)
{
    return t > 1000000000;
}

static void setSystemTime(time_t t)
{
    timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
}

// ==================================================
// RTC INIT
// ==================================================
void inicializarRTC()
{
    if (!cfg.rtc.enabled)
        return;

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        rtcPresente = rtc.begin();
        xSemaphoreGive(semI2C);
    }

    escribirLog(rtcPresente ? "RTC:OK" : "RTC:NO");
}

// ==================================================
// CARGA HORA INICIAL
// ==================================================
void cargarHoraInicial()
{
    time_t rtcTime = 0;
    time_t fsTime  = 0;

    if (rtcPresente)
    {
        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            DateTime dt = rtc.now();
            xSemaphoreGive(semI2C);

            struct tm t {};
            t.tm_year = dt.year() - 1900;
            t.tm_mon  = dt.month() - 1;
            t.tm_mday = dt.day();
            t.tm_hour = dt.hour();
            t.tm_min  = dt.minute();
            t.tm_sec  = dt.second();
            rtcTime = mktime(&t);
        }
    }

    if (xSemaphoreTake(semFS, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        if (LittleFS.exists("/time.dat"))
        {
            File f = LittleFS.open("/time.dat", "r");
            if (f)
            {
                fsTime = f.parseInt();
                f.close();
            }
        }
        xSemaphoreGive(semFS);
    }

    if (horaValida(rtcTime))
        setSystemTime(rtcTime);
    else if (horaValida(fsTime))
        setSystemTime(fsTime);
}

// ==================================================
// NTP
// ==================================================
static void taskNtp(void*)
{
    vTaskDelay(pdMS_TO_TICKS(5000));

    configTime(cfg.rtc.timezone * 3600, 0, "pool.ntp.org");

    for (;;)
    {
        watchdogKick(WDT_RELOJ);

        time_t now = time(nullptr);
        if (horaValida(now))
        {
            if (xSemaphoreTake(semFS, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                File f = LittleFS.open("/time.dat", "w");
                if (f)
                {
                    f.print((uint32_t)now);
                    f.close();
                }
                xSemaphoreGive(semFS);
            }

            if (rtcPresente)
            {
                struct tm t;
                localtime_r(&now, &t);
                DateTime dt(
                    t.tm_year + 1900,
                    t.tm_mon + 1,
                    t.tm_mday,
                    t.tm_hour,
                    t.tm_min,
                    t.tm_sec
                );

                if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE)
                {
                    rtc.adjust(dt);
                    xSemaphoreGive(semI2C);
                }
            }

            vTaskDelete(nullptr);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void iniciarTaskNtp()
{
    if (ntpTaskHandle)
        return;

    xTaskCreatePinnedToCore(
        taskNtp,
        "NTP",
        4096,
        nullptr,
        1,
        &ntpTaskHandle,
        0
    );
}

// ==================================================
// FORMATO HORA
// ==================================================
void obtenerISO8601(char* buffer, size_t size)
{
    struct tm t;
    if (!getLocalTime(&t))
    {
        strncpy(buffer, "1970-01-01 00:00:00", size);
        buffer[size - 1] = 0;
        return;
    }

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
}

void obtenerHoraHHMM(char* buf)
{
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    snprintf(buf, 6, "%02d:%02d", t.tm_hour, t.tm_min);
}

void ajustarRelojInterno(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    struct tm t {};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;

    setSystemTime(mktime(&t));
}
