#include "alarm/alarm_router.h"

#include "system/LogSystem.h"
#include "system/WatchdogManager.h"

#include <WiFi.h>

extern Configuracion cfg;

// ==================================================
// ROUTER CENTRAL
// ==================================================
bool AlarmRouter::emitirEvento(const AlarmEvent& ev)
{
    if (!cfg.state.alarms)
        return false;

    bool enviado = false;

    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        AlarmDestination& dst = cfg.alarmDestinations[i];

        if (!dst.activo)
            continue;

        if ((dst.grupos & ev.grupo) == 0)
            continue;

        if (ev.estado == ALARM_OFF && !dst.enviarRecuperacion)
            continue;

        // ======================
        // SMS
        // ======================
        if ((dst.canales & CH_SMS) && cfg.state.sms)
        {
            enviarSMS(ev, dst);
            enviado = true;
        }

        // ======================
        // TELEGRAM (dummy-safe)
        // ======================
        if (dst.canales & CH_TELEGRAM)
        {
            enviarTelegram(ev, dst);
            enviado = true;
        }
    }

    watchdogKick(WDT_ALARM_DISPATCHER);
    return enviado;
}

// ==================================================
// CANALES
// ==================================================

void AlarmRouter::enviarSMS(
    const AlarmEvent& ev,
    const AlarmDestination& dst)
{
    if (!dst.telefono[0])
        return;

    escribirLog(
        "SMS:%s:%s",
        ev.estado == ALARM_ON ? "ON" : "OFF",
        ev.mensaje
    );
}

void AlarmRouter::enviarTelegram(
    const AlarmEvent& ev,
    const AlarmDestination&)
{
    escribirLog(
        "TG:%s:%s",
        ev.estado == ALARM_ON ? "ON" : "OFF",
        ev.mensaje
    );
}
