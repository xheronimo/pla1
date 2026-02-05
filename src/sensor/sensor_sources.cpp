#include "sensor_sources.h"

// ⚠️ STUBS — luego se conectan a HW real

bool leerPCFBit(uint8_t, uint16_t)        { return false; }
bool leerI2CDigital(uint8_t, uint16_t)    { return false; }
bool leerModbusDigital(uint8_t, uint16_t) { return false; }

float leerI2CAnalogico(uint8_t, uint16_t) { return 0.0f; }
float leerModbusAnalogico(uint8_t, uint16_t) { return 0.0f; }
