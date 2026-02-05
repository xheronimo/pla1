#include "alarm_event_serializer.h"

void alarmEventToJson(const AlarmEvent& ev, JsonObject obj)
{
    obj["G"]  = ev.grupo;
    obj["T"]  = ev.tipo;
    obj["S"]  = ev.estado;
    obj["ID"] = ev.id;
    obj["N"]  = ev.nombre;
    obj["MSG"] = ev.mensaje;
    obj["V"]  = ev.valor;
    obj["TS"] = ev.timestamp;
}
