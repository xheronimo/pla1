#include "onewire_driver.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "onewire_metadata.h"

bool onewireReadTemperature(uint8_t busPin, const uint8_t rom[8], float& out)
{
    OneWire ow(busPin);

    if (!ow.reset()) return false;

    ow.select(rom);
    ow.write(0x44); // Convert T
    delay(750);

    ow.reset();
    ow.select(rom);
    ow.write(0xBE); // Read scratchpad

    uint8_t data[9];
    for (int i = 0; i < 9; i++)
        data[i] = ow.read();

    int16_t raw = (data[1] << 8) | data[0];
    out = raw / 16.0f;
    return true;
}/**
 * @brief Lee la humedad de un sensor DS2438 (Familia 0x26/0x3A)
 */
bool onewireReadHumidity(uint8_t busPin, const uint8_t rom[8], float& out)
{
    OneWire ow(busPin);
    uint8_t data[9];

    // 1. Iniciar conversión de voltaje (VAD)
    if (!ow.reset()) return false;
    ow.select(rom);
    ow.write(0xB4); // Comando: Convert V
    delay(10);      // Conversión rápida

    // 2. Leer Scratchpad (Página 0)
    ow.reset();
    ow.select(rom);
    ow.write(0xBE); // Read Scratchpad
    ow.write(0x00); // Página 0
    for (int i = 0; i < 9; i++) data[i] = ow.read();

    // 3. Obtener Voltaje de entrada (VAD) y de alimentación (VDD)
    uint16_t vad_raw = (data[4] << 8) | data[3];
    uint16_t vdd_raw = (data[2] << 8) | data[1];
    
    float vad = vad_raw / 100.0f;
    float vdd = vdd_raw / 100.0f;

    if (vdd <= 0) return false;

    // 4. Fórmula estándar HIH-4000 (RH ratiométrica)
    // RH = (VAD/VDD - 0.16) / 0.0062
    float rh = (vad / vdd - 0.16f) / 0.0062f;
    
    // Limitar rango 0-100%
    out = (rh < 0) ? 0 : (rh > 100 ? 100 : rh);
    return true;
}