#include "system_health.h"

#include "i2c/i2c_bus.h"
#include "i2c/i2c_metrics.h"
#include "modbus/modbus_metrics.h"
#include "system/nvs_store.h"
#include "system/system_fault.h"

#include <Arduino.h>

extern bool g_faultAcked ;

SystemHealth getSystemHealth()
{
    SystemHealth h = {};

    h.modbusOk = true;
    h.i2cOk    = (i2cGetBusHealth() > 70);

    h.modbusErrors = modbusGetTotalErrors();
    h.i2cErrors    = i2cGetTotalErrors();

    h.uptimeSec = millis() / 1000;
    return h;
}


