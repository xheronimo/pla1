#include "ds3231_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include <Arduino.h>

#define RTC_REG_TIME    0x00
#define RTC_REG_TEMP    0x11

struct RtcCache {
    bool valid = false;
    uint8_t buffer[7] = {0}; 
    float temp = 0.0f;
    uint32_t lastReadMs = 0;
};

static RtcCache rtc_cache;

#define DS3231_ADDR 0x68

// Conversiones de formato
static uint8_t bcdToDec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }
static uint8_t decToBcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

bool ds3231Init(uint8_t addr, uint8_t options) {
    ChipContext* cc = i2cGetChipContext(I2CDevice::DS3231, addr);
    if (!cc) return false;

    cc->state = ChipState::STATE_READY;
    cc->warmupMs = 0;
    cc->retryMs = 5000;
    return true;
}

bool ds3231Detect(uint8_t addr) {
    return i2cPing(addr);
}

static bool rtcReadAll(uint8_t addr, RtcCache& c) {
    if (!i2cReadBytes(addr, RTC_REG_TIME, c.buffer, 7)) return false;

    uint8_t t[2];
    if (i2cReadBytes(addr, RTC_REG_TEMP, t, 2)) {
        c.temp = (float)((int8_t)t[0]) + ((t[1] >> 6) * 0.25f);
    }

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

bool ds3231ReadSignal(const Signal& s, float& out) {
    ChipContext* cc = i2cGetChipContext(I2CDevice::DS3231, s.address);
    if (!cc) return false;

    uint32_t now = millis();
    if (!i2cChipGuardBeforeRead(cc, now)) return false;

    if (!rtc_cache.valid || now - rtc_cache.lastReadMs > 500) {
        if (!rtcReadAll(s.address, rtc_cache)) {
            i2cChipGuardOnError(cc);
            return false;
        }
        i2cChipGuardOnSuccess(cc, now);
    }

    switch(s.channel) {
        case 0: out = bcdToDec(rtc_cache.buffer[0]); break; // Seg
        case 1: out = bcdToDec(rtc_cache.buffer[1]); break; // Min
        case 2: out = bcdToDec(rtc_cache.buffer[2] & 0x3F); break; // Hor
        case 3: out = rtc_cache.temp; break; // Temp interna
        default: return false;
    }
    return true;
}

void ds3231GetMetadata(ChipMetadata& meta) {
    static const char* channels[] = {"Segundos", "Minutos", "Horas", "Temperatura"};
    meta.name = "DS3231 (RTC)";
    meta.channelCount = 4;
    meta.opt1 = {"Info", channels, 4, 0};
}

void ds3231Reset() { rtc_cache = {}; }


/**
 * Lee la hora completa del RTC y la devuelve en variables decimales normales.
 * Útil para sincronizar el sistema al arrancar.
 */
bool ds3231ReadTime(uint16_t &year, uint8_t &month, uint8_t &day, 
                    uint8_t &hour, uint8_t &minute, uint8_t &second) {
    uint8_t buf[7];

    // Leemos los 7 registros de tiempo (del 0x00 al 0x06) en una sola ráfaga
    if (!i2cReadBytes(DS3231_ADDR, RTC_REG_TIME, buf, 7)) {
        return false;
    }

    // Convertimos de BCD a Decimal usando la lógica que vimos
    second = bcdToDec(buf[0] & 0x7F);
    minute = bcdToDec(buf[1]);
    hour   = bcdToDec(buf[2] & 0x3F); // Registros de hora (bit 6 define 12/24h)
    day    = bcdToDec(buf[4]);
    month  = bcdToDec(buf[5] & 0x1F);
    year   = bcdToDec(buf[6]) + 2000;

    return true;
}

/**
 * Escribe la hora completa en el RTC
 */
bool ds3231WriteTime(uint16_t year, uint8_t month, uint8_t day, 
                     uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t buf[7];

    buf[0] = decToBcd(second);
    buf[1] = decToBcd(minute);
    buf[2] = decToBcd(hour);
    buf[3] = 1; // Día de la semana (1-7, obligatorio aunque no lo usemos)
    buf[4] = decToBcd(day);
    buf[5] = decToBcd(month);
    buf[6] = decToBcd(year % 100); // El chip solo guarda los últimos dos dígitos

    // Enviamos la ráfaga de 7 bytes empezando desde el registro 0x00
    return i2cWriteBytes(DS3231_ADDR, RTC_REG_TIME, buf, 7);
}