#pragma once
#include <Arduino.h>

class MQTTManager
{
public:
    static void init();
    static bool conectado();

    static void publicarAlarma(
        const char* id,
        bool estado,
        const char* mensaje,
        float valor
    );
};
