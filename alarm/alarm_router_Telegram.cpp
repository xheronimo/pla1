#include "alarm_router.h"
#include "net/TelegramHelper.h"

void AlarmRouter::enviarTelegram(
    const AlarmEvent& ev,
    const AlarmDestination& d)
{
    char msg[200];
    snprintf(msg, sizeof(msg),
        "%s\n%s\n%.2f",
        ev.estado == ALARM_ON ? "⚠️ ALARMA" : "✅ RECUPERACIÓN",
        ev.mensaje,
        ev.valor);

    enviarAlertaTelegram(d.telegramChat, msg);
}
