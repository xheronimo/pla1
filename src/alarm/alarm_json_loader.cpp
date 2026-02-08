#include "alarm_json_loader.h"
#include "alarm_expr_builder.h"
#include "alarm_registry.h"
#include "alarm_utils.h"


bool loadAlarmsFromJson(const JsonArrayConst& arr)
{
    for (JsonObjectConst o : arr)
    {
        AlarmRule r{};

        r.alarmId = o["id"] | 0;
        r.latch   = o["latch"] | false;

        r.group    = parseAlarmGroup(o["group"] | "info");
        r.severity = parseAlarmSeverity(o["severity"] | "low");

        // ---- acciones ----
        JsonObjectConst actions =
            o["actions"].is<JsonObjectConst>()
                ? o["actions"].as<JsonObjectConst>()
                : JsonObjectConst();

        r.sendMqtt     = actions["mqtt"]     | true;
        r.sendWeb      = actions["web"]      | true;
        r.sendTelegram = actions["telegram"] | false;
        r.sendSms      = actions["sms"]      | false;

        // ---- expresión ----
        if (!o["expr"].is<JsonObjectConst>())
            return false;

        r.expr = buildExpr(o["expr"].as<JsonObjectConst>());
        if (!r.expr)
            return false;

        alarmRegistryAdd(r);
    }

    return true;
}
