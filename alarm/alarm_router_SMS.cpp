#include "alarm_router.h"
#include "net/ModemManager.h"

void AlarmRouter::enviarSMS(
    const AlarmEvent& ev,
    const AlarmDestination& d)
{
    if (!strlen(d.telefono)) return;

    char msg[160];
    snprintf(msg, sizeof(msg),
        "%s\n%s\n%.2f",
        ev.estado == ALARM_ON ? "ALARMA" : "RECUPERACION",
        ev.mensaje,
        ev.valor);

    ModemManager::sendSMS(d.telefono, msg);
}
