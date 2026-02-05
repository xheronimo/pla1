#pragma once
#include <stdint.h>

class MQTTManager
{
public:
    static void configurar(
        const char* host,
        uint16_t port,
        const char* user,
        const char* pass,
        const char* baseTopic
    );

    static void init();

    static void habilitar(bool en);

    static bool conectado();

    // Para alarmas
    static void publicarEvento(const struct AlarmEvent& ev);
};
