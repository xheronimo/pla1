#pragma once
#include "alarm_struct.h"

class AlarmRouter
{
public:
    bool emitirEvento(const AlarmEvent &ev);

private:
    void enviarMQTT(const AlarmEvent &ev, const AlarmRule &rule);
    void enviarSMS(const AlarmEvent &ev, const AlarmRule &rule);
    void enviarTelegram(const AlarmEvent &ev, const AlarmRule &rule);
    void enviarMQTTAck(const AlarmEvent &ev, const AlarmRule &rule);
    void enviarWeb(const AlarmEvent &ev, const AlarmRule &rule);
};
