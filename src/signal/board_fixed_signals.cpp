#include "signal/board_fixed_signals.h"

#include "signal/signal_manager.h"
#include "signal/signal_struct.h"
#include "i2c/i2c_chip.h"
#include <Arduino.h>

static void registerPCFInputs(uint8_t addr)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Signal s{};
        memset(&s, 0, sizeof(Signal));

        char idBuf[24];
        snprintf(idBuf, sizeof(idBuf),
                 "PCF_%02X_%u", addr, i);

        s.id = strdup(idBuf);

        s.kind    = SignalKind::SENSOR_DIGITAL;
        s.bus     = BusType::BUS_PCF;
        s.chip    = I2CDevice::PCF8574;
        s.address = addr;
        s.channel = i;

        s.systemReserved = true;
        s.lockedConfig   = true;
        s.writable       = false;
        s.invertido      = false;
        s.debounceMs     = 0;

        signalManagerAdd(s);
    }
}

void registerBoardFixedSignals()
{
#ifdef BOARD_V3
    registerPCFInputs(0x21);
    registerPCFInputs(0x22);
#endif

#ifdef BOARD_V1
    registerPCFInputs(0x21);
    registerPCFInputs(0x22);
#endif
}
