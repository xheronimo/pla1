#pragma once
#include <stdint.h>

class ModbusManager
{
public:
    static void init();        // inicialización física
    static void loop();        // task o loop futuro

    static bool isEnabled();   // si está activo por config
    static bool leerRegistro(uint16_t reg, float& out);
};
