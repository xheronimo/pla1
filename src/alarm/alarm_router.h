#pragma once

#include "alarm/alarm_struct.h"
#include "config/config_struct.h"

#include "alarm/alarm_destination.h"
// ==================================================
// ROUTER CENTRAL DE ALARMAS
// ==================================================

class AlarmRouter
{
public:
    // Emite un evento según destinos/configuración
    static bool emitirEvento(const AlarmEvent& ev);

private:
    // Canales concretos
    static void enviarSMS(const AlarmEvent& ev, const AlarmDestination& dst);
    static void enviarTelegram(const AlarmEvent& ev, const AlarmDestination& dst);
};
