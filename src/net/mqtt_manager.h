#pragma once

#include <stdint.h>

class MQTTManager
{
public:
    // Inicializa cliente y callbacks
    static void init();

    // Loop periódico (desde task o loop principal)
    static void loop();

    // Publicar mensaje
    static bool publish(
        const char* topic,
        const char* payload,
        bool retain = false
    );

    // Estado de conexión
    static bool isConnected();

    // Hook al conectar (suscripciones + snapshot)
    static void onConnect();
};
