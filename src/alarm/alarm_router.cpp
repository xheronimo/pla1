#include "alarm/alarm_router.h"
#include "alarm/alarm_manager.h"
#include "net/mqtt_payloads.h" // Mantengo tu header de MQTT
#include "system/LogSystem.h"
#include <Arduino.h>

namespace AlarmRouter {

    bool dispatch(const AlarmEvent& ev) {
        // 1. Obtener la regla desde el nuevo Manager dinámico
        AlarmRule* rule = AlarmMgr::getById(ev.alarmId);
        if (!rule) return false;

        // 2. Filtro de Spam (Basado en tu lógica original)
        // Si el evento es una activación pero la regla ya estaba en ACK, 
        // podrías decidir no volver a enviar notificaciones ruidosas.
        if (ev.active && rule->isAcknowledged) {
            return false; 
        }

        // 3. Distribución según la configuración de la regla
        if (rule->sendMqtt)     enviarMQTT(ev, *rule);
        if (rule->sendWeb)      enviarWeb(ev, *rule);
        if (rule->sendTelegram) enviarTelegram(ev, *rule);
        if (rule->sendSms)      enviarSMS(ev, *rule);

        // Nota: La persistencia de estado (ACK/Active) ya NO se hace aquí.
        // El AlarmManager gestiona el estado en RAM y el JSON Loader en SD.

        return true;
    }

    void enviarMQTT(const AlarmEvent& ev, const AlarmRule& rule) {
        // Llamada a tu función existente en net/mqtt_payloads.h
        mqttPublishAlarm(ev);
    }

    void enviarWeb(const AlarmEvent& ev, const AlarmRule& rule) {
        // Aquí puedes integrar WebSockets o Server-Sent Events en el futuro
        // Serial.printf("[Web] Notificando alarma %d\n", rule.alarmId);
    }

    void enviarTelegram(const AlarmEvent& ev, const AlarmRule& rule) {
        if (!ev.active) return; // Opcional: solo avisar por Telegram al activar
        // Tu lógica de TelegramBot::sendMessage(...)
    }

    void enviarSMS(const AlarmEvent& ev, const AlarmRule& rule) {
        // Stub para módulo GSM
    }

} // namespace AlarmRouter