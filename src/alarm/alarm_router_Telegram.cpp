#include "alarm_router.h"
#include "TelegramHelper.h"

void AlarmRouter::enviarTelegram(
    const AlarmEvent& ev,
    const AlarmDestination&)
{
    char buf[240];

    snprintf(buf, sizeof(buf),
        "%s\n%s\n%.2f",
        ev.estado == ALARM_ON ? "⚠️ ALARMA" : "✅ RECUPERACIÓN",
        ev.mensaje,
        ev.valor);

    enviarAlertaTelegram(buf);
}
