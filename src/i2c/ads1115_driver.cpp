#include "ads1115_driver.h"
#include "i2c_chip_guard.h"
#include <Wire.h>

// =====================================================
// REGISTROS
// =====================================================
#define ADS_REG_CONVERSION  0x00
#define ADS_REG_CONFIG      0x01

// =====================================================
// CACHE POR CHIP
// =====================================================
struct AdsCache {
    bool valid = false;
    int16_t raw[4] = {0};
    uint32_t lastReadMs = 0;
};

static AdsCache cache[ADS1115_MAX_ADDR_SLOTS];

// =====================================================
// UTILIDADES CONFIG
// =====================================================
static uint16_t gainBits(uint8_t gain)
{
    switch (gain)
    {
        case 0: return 0x0000; // ±6.144V
        case 1: return 0x0200; // ±4.096V
        case 2: return 0x0400; // ±2.048V
        case 3: return 0x0600; // ±1.024V
        case 4: return 0x0800; // ±0.512V
        default:return 0x0A00; // ±0.256V
    }
}

static uint16_t rateBits(uint8_t rate)
{
    return (rate & 0x07) << 5;
}

static float lsbFromGain(uint8_t gain)
{
    switch (gain)
    {
        case 0: return 0.0001875f;
        case 1: return 0.000125f;
        case 2: return 0.0000625f;
        case 3: return 0.00003125f;
        case 4: return 0.000015625f;
        default:return 0.0000078125f;
    }
}

// =====================================================
// INIT
// =====================================================
bool ads1115Init(uint8_t addr, uint8_t options)
{
    ChipContext* cc = i2cGetChipContext(I2CDevice::ADS1115, addr);
    if (!cc) return false;

    uint8_t gain = (options >> 4) & 0x07;
    uint8_t rate = options & 0x07;

    uint16_t cfg =
        0x8101 |
        gainBits(gain) |
        rateBits(rate);

    if (!i2cWrite16(addr, ADS_REG_CONFIG, cfg))
        return false;

    cc->state = ChipState::STATE_READY;
    cc->consecutiveErrors = 0;
    cc->errorCount = 0;
    cc->warmupMs = 2;
    cc->retryMs = 2000;

    return true;
}


// =====================================================
// READ ALL CANALES
// =====================================================
static bool adsReadAll(uint8_t addr, AdsCache& c, uint8_t gain)
{
    static const uint16_t mux[4] = {
        0x4000, 0x5000, 0x6000, 0x7000
    };

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        uint16_t cfg = 0x8101 | mux[ch] | gainBits(gain);

        if (!i2cWrite16(addr, ADS_REG_CONFIG, cfg))
            return false;

        vTaskDelay(pdMS_TO_TICKS(10));

        uint16_t raw;
        if (!i2cRead16(addr, ADS_REG_CONVERSION, raw))
            return false;

        c.raw[ch] = (int16_t)raw;
    }

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// =====================================================
// READ SIGNAL
// =====================================================
bool ads1115ReadSignal(const Signal& s, float& out)
{
    ChipContext* cc = i2cGetChipContext(I2CDevice::ADS1115, s.address);
    if (!cc) return false;

    uint32_t now = millis();

    // 🔐 GUARD CENTRAL
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    uint8_t idx = s.address & 0x07;
    if (idx >= ADS1115_MAX_ADDR_SLOTS)
        return false;

    AdsCache& c = cache[idx];

    // ---------------- INIT ----------------
    if (cc->state == ChipState::STATE_UNINITIALIZED)
    {
        if (!ads1115Init(s.address, s.options))
        {
            i2cChipGuardOnError(cc);
            return false;
        }
        return false; // esperar siguiente ciclo
    }

    if (cc->state != ChipState::STATE_READY)
        return false;

    uint8_t gain = (s.options >> 4) & 0x07;

    // ---------------- CACHE ----------------
    if (!c.valid || now - c.lastReadMs > ADS1115_CACHE_MS)
    {
        if (!adsReadAll(s.address, c, gain))
        {
            i2cChipGuardOnError(cc);
            return false;
        }

        i2cChipGuardOnSuccess(cc, now);
    }

    if (s.channel > 3)
        return false;

    out = c.raw[s.channel] * lsbFromGain(gain);
    return true;
}


// =====================================================
// METADATA
// =====================================================
void ads1115GetMetadata(ChipMetadata& meta)
{
    static const char* gains[] = {
        "±6.144V","±4.096V","±2.048V",
        "±1.024V","±0.512V","±0.256V"
    };

    static const char* rates[] = {
        "8","16","32","64",
        "128","250","475","860"
    };

    meta.name = "ADS1115";
    meta.channelCount = 4;

    meta.opt1 = {"Gain", gains, 6, 1};
    meta.opt2 = {"Rate (SPS)", rates, 8, 4};
}

// =====================================================
void ads1115Reset()
{
    for (int i = 0; i < ADS1115_MAX_ADDR_SLOTS; i++)
        cache[i] = {};
}

bool ads1115Detect(uint8_t addr)
{
    return i2cPing(addr);
}
