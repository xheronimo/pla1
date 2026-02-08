#include "signal_reader.h"
#include <Arduino.h>

// --------------------------------------------------
// GPIO DIGITAL
// --------------------------------------------------
bool leerSignalGPIO(const Signal &s, float &out)
{
    // channel = número de pin GPIO
    uint8_t pin = s.channel;

    // Configurar pin SOLO una vez
    static bool initialized[40] = {false}; // ESP32 máx ~40 GPIO

    if (pin >= 40)
        return false;

    if (!initialized[pin])
    {
        if (s.kind == SignalKind::SENSOR_DIGITAL)
        {
            pinMode(pin, INPUT_PULLUP); // 🔧 por defecto seguro
        }
        else
        {
            pinMode(pin, INPUT);
            analogReadResolution(12);       // 0..4095
            analogSetAttenuation(ADC_11db); // ~0-3.3V
        }
        initialized[pin] = true;
    }

    if (s.kind == SignalKind::SENSOR_DIGITAL)
    {
        int level = digitalRead(pin);
        out = (level == HIGH) ? 1.0f : 0.0f;
    }
    else
    {
        int raw = analogRead(pin);
        // Convertimos a float (RAW, sin escalar)
        out = (float)raw;
    }

    return true;
}
