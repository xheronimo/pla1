#include "signal_manager.h"
#include "signal_reader.h"
#include "signal_calibration.h"
#include "bus/bus_struct.h"
#include <string.h>
#include <Arduino.h>
#include "i2c/i2c_chip_context.h"

#include "signal/signal_modbus_poll.h"
#include "modbus/modbus_blocks.h"
#include "modbus/modbus_scheduler.h"
#include "system/WatchdogManager.h"
#include "signal/signal_struct.h"
#include "system/system_mode.h"
#include "signal/signal_persist.h"
#include "signal/system_signals.h"
#include "system/fault_output.h"
#include "system/system_fault.h"
#include "system/nvs_store.h"
#include "system/system_fault.h"
#include "system/boot_reason.h"
#include "system/nvs_store.h"
#include "signal/signal_manager.h"
#include "board_fixed_signals.h"



#define MAX_SIGNALS 64

static Signal signalTable[MAX_SIGNALS];
static size_t signalCount = 0;
extern bool g_faultAcked;

void signalManagerInit()
{
     signalCount = 0;
    watchdogRegister(WDT_SENSORS);

    registerSystemSignals();          // OK
    registerBoardFixedSignals();      // ✅ este es el nombre correcto

    loadSignalCalibrations();         // OK

    faultOutputInit();                // OK
    systemFaultInit();                // ✅ RESTAURA ACK desde NVS
}



bool signalManagerAdd(const Signal& s)
{
    if (signalCount >= MAX_SIGNALS)
        return false;

    signalTable[signalCount] = s;
    signalTable[signalCount].raw = 0.0f;
    signalTable[signalCount].error = false;
    signalTable[signalCount].initialized = false;
    

    signalCount++;
    return true;
}



size_t signalManagerCount()
{
    return signalCount;
}

Signal *signalManagerGet(size_t index)
{
    if (index >= signalCount)
        return nullptr;
    return &signalTable[index];
}




bool signalManagerGetValue(const char *id, float &outValue)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        if (strcasecmp(signalTable[i].id, id) == 0)
        {
            outValue = signalTable[i].raw;
            return true;
        }
    }
    return false;
}

bool signalManagerGetValueById(uint32_t id, float &outValue)
{
    Signal *s = signalManagerGet(id);
    if (!s)
        return false;

    outValue = s->raw;
    return true;
}

bool signalManagerGetErrorById(uint32_t id, bool &outError)
{
    Signal *s = signalManagerGet(id);
    if (!s)
        return false;

    outError = s->error;
    return true;
}

bool signalPhysicalExists(const Signal &s, const char *ignoreId)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        const Signal &other = signalTable[i];

        if (ignoreId && strcmp(other.id, ignoreId) == 0)
            continue;

        if (other.bus == s.bus &&
            other.address == s.address &&
            other.channel == s.channel)
        {
            return true;
        }
    }

    return false;
}

Signal *signalManagerGetAll(size_t &count)
{
    count = signalCount;
    return signalTable;
}

size_t signalManagerGetCount()
{
    return signalCount;
}

Signal* signalManagerGetById(const char* id)
{
    if (!id)
        return nullptr;

    for (size_t i = 0; i < signalCount; i++)
    {
        if (signalTable[i].id && strcmp(signalTable[i].id, id) == 0)
            return &signalTable[i];
    }
    return nullptr;
}

void signalManagerPollAll()
{
    for (size_t i = 0; i < signalCount; i++)
    {
        Signal& s = signalTable[i];

        float raw = 0.0f;
        bool ok = leerSignal(s, raw);

        if (!ok)
        {
            s.error = true;
            s.valid = false;
            s.quality = SignalQuality::ERROR;
            continue;
        }

        s.raw = raw;

        bool stable = true;
        bool clamped = false;

        float value = raw;

        if (s.kind == SignalKind::SENSOR_ANALOG)
        {
            value = applyCalibration(
                raw,
                s.calib,
                stable,
                clamped
            );
        }

        s.value = value;
        s.error = false;
        s.valid = true;
        s.initialized = true;

        if (!stable)
            s.quality = SignalQuality::UNSTABLE;
        else if (clamped)
            s.quality = SignalQuality::CLAMPED;
        else
            s.quality = SignalQuality::GOOD;
    }
}