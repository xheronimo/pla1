#include "alarm_router.h"
#include "net/mqtt_payloads.h"
#include "alarm_persistence.h"
#include "alarm_runtime.h"
#include "alarm_actions.h"
#include "alarm_registry.h"
#include "alarm_utils.h"

#include "alarm_router.h"

#include "alarm_registry.h"
#include "alarm_runtime.h"
#include "alarm_persistence.h"
#include "alarm_utils.h"

#include "net/mqtt_payloads.h"

bool AlarmRouter::emitirEvento(const AlarmEvent& ev)
{
    const AlarmRule* rule = alarmRegistryGet(ev.alarmId);
    if (!rule)
        return false;

    // 🔕 No spam si está ACK y sigue activa
    if (ev.kind == AlarmEventKind::STATE_CHANGE &&
        ev.active &&
        alarmRuntimeIsAcked(ev.alarmId))
    {
        return false;
    }

    // ----------------------------
    // ENVÍOS SEGÚN REGLA
    // ----------------------------
    if (rule->sendMqtt)
        enviarMQTT(ev, *rule);

    if (rule->sendWeb)
        enviarWeb(ev, *rule);

    if (rule->sendTelegram)
        enviarTelegram(ev, *rule);

    if (rule->sendSms)
        enviarSMS(ev, *rule);

    // ----------------------------
    // PERSISTENCIA
    // ----------------------------
    AlarmRuntime* rt = alarmRuntimeGet(ev.alarmId);
    if (rt)
        alarmPersistenceSave(ev.alarmId, rt->active, rt->acked);

    return true;
}



void AlarmRouter::enviarMQTT( const AlarmEvent& ev,
    const AlarmRule& rule)
{
    mqttPublishAlarm(ev);
}

void AlarmRouter::enviarSMS( const AlarmEvent& ev,
    const AlarmRule& rule)
{
    // stub
}

void AlarmRouter::enviarTelegram( const AlarmEvent& ev,
    const AlarmRule& rule)
{
    // stub
}

void AlarmRouter::enviarMQTTAck( const AlarmEvent& ev,
    const AlarmRule& rule)
{
    mqttPublishAlarmAck(ev.alarmId);
}

void AlarmRouter::enviarWeb( const AlarmEvent& ev,
    const AlarmRule& rule)
{
    // Por ahora NO hace nada:
    // - La web consulta estado por REST
    // - Más adelante aquí puedes:
    //   - hacer WebSocket push
    //   - notificar SSE
    (void)ev;
}
