#pragma once

#include "alarm/alarm_struct.h"
#include "config/config_struct.h"

class AlarmRouter
{
public:
    static bool emitirEvento(const AlarmEvent& ev);

private:
    static void enviarSMS(const AlarmEvent& ev, const AlarmDestination& dst);
    static void enviarTelegram(const AlarmEvent& ev, const AlarmDestination& dst);
};
