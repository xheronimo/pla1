#include "signal_write.h"
#include "bus/bus_struct.h"
#include "i2c/i2c_chip.h"
#include "modbus/modbus_manager.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_chip_context.h"
#include "signal/signal_manager.h"
#include "i2c/i2c_chip_guard.h"
#include "system/LogSystem.h"

// #include "gpio/gpio_driver.h"

bool signalWrite(Signal &s, float value)
{
    // ⛔ protección
    if (!s.writable)
        return false;

    s.prev = s.value;
    s.raw = value;
    s.value = value;

    bool ok = false;

    switch (s.bus)
    {
    // -------------------------
    // GPIO DIRECTO
    // -------------------------
    case BusType::BUS_GPIO:
        writeGpioSignal(s, value);
        break;

    // -------------------------
    // I2C (PCF, etc.)
    // -------------------------
    case BusType::BUS_I2C:
    {
        const I2CChipDriver* drv = i2cGetDriver(s.chip);
            if (!drv || !drv->write)
                return false;

            return drv->write(s, value);
    }

    // -------------------------
    // MODBUS
    // -------------------------
    case BusType::BUS_MODBUS:
    {
        if (!s.writable)
            return false;

        uint16_t reg = (uint16_t)value;

        ok = ModbusManager::writeHolding(
            s.address,
            s.channel,
            reg);
        break;
    }

    default:
        return false;
    }

    // -------------------------
    // RESULTADO
    // -------------------------
    if (!ok)
    {
        s.error = true;
        s.valid = false;
        s.errorCount++;
    }
    else
    {
        s.error = false;
        s.valid = true;
        s.errorCount = 0;
        s.lastOkTs = millis();
    }

    return ok;
}

static bool writeGpioSignal(const Signal& s, float value)
{
    digitalWrite(s.channel, value > 0.5f ? HIGH : LOW);
    return true;
}

bool writeSignalById(const char* id, float value)

{
    size_t count = signalManagerCount();

    for (size_t i = 0; i < count; i++)
    {
        Signal* s = signalManagerGet(i);
        if (!s) continue;

        if (strcmp(s->id, id) == 0)
        {
            if (!s->writable)
                return false;

            return signalWrite(*s, value);
        }
    }
    return false;
}

bool escribirSignal(const Signal& s, float value)
{
    // --------------------------------
    // Validaciones básicas
    // --------------------------------
    if (!s.writable)
        return false;

    // =====================================================
    // GPIO DIRECTO
    // =====================================================
    if (s.bus == BusType::BUS_GPIO)
    {
        bool level = (value > 0.5f);
        pinMode(s.channel, OUTPUT);
        digitalWrite(s.channel, level ? HIGH : LOW);
        return true;
    }

    // =====================================================
    // I2C
    // =====================================================
    if (s.bus == BusType::BUS_I2C)
    {
        ChipContext* ctx = i2cGetChipContext(s.chip, s.address);
        if (!ctx)
            return false;

        uint32_t now = millis();

        // Guardia antes de usar el chip
        if (!i2cChipGuardBeforeRead(ctx, now))
            return false;

        const I2CChipDriver* drv = i2cGetDriver(s.chip);
        if (!drv || !drv->write)
        {
            escribirLog(
                "I2C: Chip %u no soporta WRITE",
                (uint8_t)s.chip
            );
            return false;
        }

        bool ok = drv->write(s, value);

        if (!ok)
        {
            i2cChipGuardOnError(ctx);
            return false;
        }

        i2cChipGuardOnSuccess(ctx, now);
        return true;
    }

    return false;
}