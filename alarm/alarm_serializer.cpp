#include "alarm_serializer.h"
#include "alarm_struct.h"
#include "system/RelojSistema.h"


// =======================================================
// SERIALIZAR EVENTO DE ALARMA
// =======================================================


void serializeAlarmEvent(
    JsonObject obj,
    const AlarmEvent& ev
)
{
    obj["ID"]    = ev.id;
    obj["NAME"]  = ev.nombre;
    obj["MSG"]   = ev.mensaje;

    obj["G"]     = ev.grupo;
    obj["TYPE"]  = alarmTriggerTypeToString(ev.tipo);
    obj["STATE"] = ev.estado == ALARM_ON ? "ON" : "OFF";

    obj["VAL"]   = ev.valor;

    // Timestamp ISO
    char ts[24];
    obtenerISO8601(ts, sizeof(ts));
    obj["TS"] = ts;
}
