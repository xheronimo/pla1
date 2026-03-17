#include "ds3231_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_registry.h"
#include <Arduino.h>
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// Conversiones BCD para el hardware del RTC
static uint8_t bcdToDec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }
static uint8_t decToBcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

// ============================================================
// API PRINCIPAL: READ SIGNAL
// ============================================================

bool ds3231ReadSignal(const Signal& s, float& out)
{
    // 1. Obtener contexto por chipId
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARDIA DE SEGURIDAD
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) 
    {
        bool ok = true;

        // Actualizar caché cada 500ms
        if (!rtc_cache.valid || now - rtc_cache.lastReadMs > 500) 
        {
            if (!rtcReadAll(addr, rtc_cache)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                i2cChipGuardOnSuccess(cc, now);
            }
        }

        xSemaphoreGive(semI2C);
        if (!ok) return false;
    } 
    else {
        return false; // Bus ocupado
    }

    // 4. MAPEO DE CANALES (Seg, Min, Hor, Temp)
    switch(s.channel) {
        case 0: out = (float)bcdToDec(rtc_cache.buffer[0] & 0x7F); break; 
        case 1: out = (float)bcdToDec(rtc_cache.buffer[1]); break; 
        case 2: out = (float)bcdToDec(rtc_cache.buffer[2] & 0x3F); break; 
        case 3: out = rtc_cache.temp; break; 
        default: return false;
    }
    return true;
}

// ============================================================
// LÓGICA DE TIEMPO (Sincronización del Sistema)
// ============================================================

bool ds3231ReadTime(uint16_t &year, uint8_t &month, uint8_t &day, 
                    uint8_t &hour, uint8_t &minute, uint8_t &second) 
{
    uint8_t buf[7];
    // IMPORTANTE: El RTC suele tener una dirección fija 0x68
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
        bool ok = i2cReadBytes(0x68, 0x00, buf, 7);
        xSemaphoreGive(semI2C);
        
        if (!ok) return false;

        second = bcdToDec(buf[0] & 0x7F);
        minute = bcdToDec(buf[1]);
        hour   = bcdToDec(buf[2] & 0x3F);
        day    = bcdToDec(buf[4]);
        month  = bcdToDec(buf[5] & 0x1F);
        year   = bcdToDec(buf[6]) + 2000;
        return true;
    }
    return false;
}

bool ds3231WriteTime(uint16_t year, uint8_t month, uint8_t day, 
                     uint8_t hour, uint8_t minute, uint8_t second) 
{
    uint8_t buf[7];
    buf[0] = decToBcd(second);
    buf[1] = decToBcd(minute);
    buf[2] = decToBcd(hour);
    buf[3] = 1; // Day of week
    buf[4] = decToBcd(day);
    buf[5] = decToBcd(month);
    buf[6] = decToBcd(year % 100);

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
        bool ok = i2cWriteBytes(0x68, 0x00, buf, 7);
        xSemaphoreGive(semI2C);
        return ok;
    }
    return false;
}

// ============================================================
// METADATA
// ============================================================

void ds3231GetMetadata(ChipMetadata& meta) {
    static const char* channels[] = {"Segundos", "Minutos", "Horas", "Temperatura"};
    meta.name = "DS3231 (RTC)";
    meta.channelCount = 4;
    meta.mode = ChipMode::INPUT_ONLY;
    meta.opt1 = {"Canales", channels, 4, 0};
    meta.opt2 = {nullptr, nullptr, 0, 0};
}

// ============================================================
// RESET
// ============================================================
/**
 * @brief Limpia la caché del RTC. 
 * Se llama cuando el sistema detecta una anomalía en el bus o tras un recovery.
 */
void ds3231Reset() 
{
    rtc_cache.valid = false;
    rtc_cache.lastReadMs = 0;
    memset(rtc_cache.buffer, 0, sizeof(rtc_cache.buffer));
    rtc_cache.temp = 0.0f;
}

// ============================================================
// DETECTION
// ============================================================
/**
 * @brief Verifica la presencia física del RTC en el bus.
 * El DS3231 tiene una dirección fija de 0x68.
 */
bool ds3231Detect(uint8_t addr) 
{
    // El DS3231 es muy estable al ping I2C.
    // Simplemente verificamos que el dispositivo responda ACK.
    return i2cPing(addr);
}

// ============================================================
// INIT (REFINADO)
// ============================================================
bool ds3231Init(uint32_t chipId) 
{
    ChipContext* cc = i2cGetChipContextById(chipId);
    if (!cc) return false;

    // Tomamos el bus para verificar que el RTC está vivo
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        bool alive = i2cPing(cc->address);
        xSemaphoreGive(semI2C);

        if (alive) {
            cc->state = ChipState::STATE_READY;
            cc->warmupMs = 0;
            cc->retryMs = 5000;
            return true;
        }
    }
    
    return false;
}