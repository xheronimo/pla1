#include "signal_manager.h"
#include "signal_reader.h"
#include "signal_calibration.h"
#include "bus/bus_struct.h"
#include <string.h>
#include <Arduino.h>

#define MAX_SIGNALS 64

static Signal signalTable[MAX_SIGNALS];
static size_t signalCount = 0;

void signalManagerInit()
{
    signalCount = 0;
}

bool signalManagerAdd(const Signal &s)
{
    if (signalCount >= MAX_SIGNALS)
        return false;

    signalTable[signalCount] = s;
    signalTable[signalCount].raw = 0.0f;
    signalTable[signalCount].error = false;

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

void signalManagerPollAll()
{
    uint32_t nowMs = millis();

    for (size_t i = 0; i < signalCount; i++)
    {
        Signal &sig = signalTable[i];

        ChipContext* cc = i2cGetChipContext(sig.bus, sig.chip, sig.address);
        if (cc && cc->state == ChipState::ERROR)
        {
            sig.error = true;
            sig.valid = false;   // 👈 opcional
            continue;
        }

        float v = 0.0f;

        // -----------------------------
        // Leer señal física
        // -----------------------------
        if (!leerSignal(sig, v))
        {
            sig.error = true;
 
            sig.errorCount++;
  
            // watchdog por señal

            if (sig.errorCount >= 5) {

                sig.valid = false;

            }
    
            continue;   // ⛔ no tocar valores
        }

        sig.error = false;
        sig.errorCount = 0;
        sig.valid = true;

        // -----------------------------
        // Invertido (solo digital)
        // -----------------------------
        if (sig.kind == SignalKind::SENSOR_DIGITAL && sig.invertido)
        {
            v = (v != 0.0f) ? 0.0f : 1.0f;
        }

        // -----------------------------
        // Primera lectura
        // -----------------------------
        if (!sig.initialized)
        {
            sig.raw = v;
            sig.prev = v;
            sig.stableValue = v;
            sig.value = v;
            sig.lastChangeTs = 0;
            sig.initialized = true;
            continue;
        }

        // -----------------------------
        // Guardar valor previo (estable)
        // -----------------------------
        sig.prev = sig.value;

        // -----------------------------
        // DIGITAL con debounce
        // -----------------------------
        if (sig.kind == SignalKind::SENSOR_DIGITAL && sig.debounceMs > 0)
        {
            if (v != sig.stableValue)
            {
                if (sig.lastChangeTs == 0)
                {
                    sig.lastChangeTs = nowMs;
                }
                else if ((nowMs - sig.lastChangeTs) >= sig.debounceMs)
                {
                    sig.stableValue = v;
                    sig.lastChangeTs = 0;
                }
            }
            else
            {
                sig.lastChangeTs = 0;
            }

            sig.raw = v;
            sig.value = sig.stableValue;
            continue;
        }

        // -----------------------------
        // SIN debounce
        // -----------------------------
        sig.raw = v;

        if (sig.kind == SignalKind::SENSOR_ANALOG)
        {
            sig.value = applyCalibration(sig.raw, sig.calib);
        }
        else
        {
            sig.value = sig.raw;
        }
    }
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
