#pragma once
#include <stdint.h>

// Definición de límites para evitar desbordamiento de memoria
#define MAX_ALARM_DESTINATIONS 5 

/**
 * @brief Canales de notificación soportados (Máscara de bits)
 */
enum AlarmChannel : uint8_t {
    CH_NONE     = 0,
    CH_MQTT     = 1 << 0,
    CH_SMS      = 1 << 1,
    CH_TELEGRAM = 1 << 2,
    CH_LOG      = 1 << 3  // Añadido por seguridad
};

/**
 * @brief Estructura de destino: Define QUIÉN y CÓMO recibe la alarma
 */
struct AlarmDestination {
    bool activo;
    uint8_t canales;        // Combinación de AlarmChannel
    uint64_t grupos;        // Máscara de bits para filtrar por AlarmGroup
    bool enviarRecuperacion; // ¿Avisar cuando la alarma vuelve a OK?

    char nombre[24];        // Alias del técnico o sistema

    // Configuración específica por canal
    char telefono[20];      // Para SMS
    int64_t telegramChatId; // Para Telegram
    char mqttTopic[64];     // Para MQTT personalizado
};

// --- Variables Globales ---
extern AlarmDestination alarmDestinations[MAX_ALARM_DESTINATIONS];
extern uint8_t alarmDestinationCount;

// --- Funciones de Gestión ---
namespace AlarmDestMgr {
    void init();
    bool addDestination(const AlarmDestination& dest);
    void clearAll();
}