#include "system_signals.h"
#include "signal_manager.h"
#include "board/board_config.h"

void registerSystemSignals()
{
    Signal s;

    s.id = "FAULT_WDT";
    s.kind = SignalKind::ACTUATOR_DIGITAL;
    s.bus = BusType::BUS_I2C;
    s.chip = I2CDevice::PCF8574;
    s.address = RELAY_I2C_ADDR_2;
    s.channel = 7;
    s.writable = true;
    s.systemReserved = true;
    s.initialized = false;
    s.valid = true;
    s.error = false;


    signalManagerAdd(s);
}