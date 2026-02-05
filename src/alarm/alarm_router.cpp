#include "alarm_router.h"

#include "config/config_global.h"
#include "alarm_runtime.h"
#include "system/LogSystem.h"
#include "config/config_struct.h"
#include "alarm_destination.h"
#include "alarm_polity.h"
#include <WiFi.h>

// ==================================================

static AlarmRuntime runtime[MAX_ALARM_DESTINATIONS][16];
static SemaphoreHandle_t alarmMutex = nullptr;

// ==================================================

bool AlarmRouter::emitirEvento(const AlarmEvent& ev)
{
    if (!cfg.state.alarms)
        return false;

    if (!alarmMutex)
        alarmMutex = xSemaphoreCreateMutex();

    if (xSemaphoreTake(alarmMutex, pdMS_TO_TICKS(200)) != pdTRUE)
        return false;

    bool enviado = false;

    bool internet =
        WiFi.isConnected();

    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        AlarmDestination& dst = cfg.alarmDestinations[i];
        if (!dst.activo) continue;
        if ((dst.grupos & ev.grupo) == 0) continue;

        AlarmRuntime& rt = runtime[i][(uint8_t)ev.tipo];

        if (!AlarmPolicyEngine::permitirEvento(
                ev.grupo,
                ev.tipo,
                ev.estado,
                rt.ultimoValor,
                ev.valor,
                rt))
        {
            continue;
        }

        if ((dst.canales & CH_SMS) && cfg.state.sms)
        {
            enviarSMS(ev, dst);
            enviado = true;
        }

        if (internet)
        {
            if (dst.canales & CH_TELEGRAM)
            {
                enviarTelegram(ev, dst);
                enviado = true;
            }
        }
    }

    xSemaphoreGive(alarmMutex);
    return enviado;
}

// ==================================================

void AlarmRouter::enviarTelegram(
    const AlarmEvent& ev,
    const AlarmDestination&)
{
    // Implementación real más adelante
    escribirLog("ALM:TG");
}

void AlarmRouter::enviarSMS(
    const AlarmEvent& ev,
    const AlarmDestination& dst)
{
    if (strlen(dst.telefono) == 0)
        return;

    escribirLog("ALM:SMS:%s", dst.telefono);
}
