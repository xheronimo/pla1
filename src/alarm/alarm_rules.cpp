#include "alarm_rules.h"

#include "system/LogSystem.h"
#include <ArduinoJson>

static std::vector<AlarmRule> rules;

// =====================================================

const std::vector<AlarmRule>& alarmRulesGetAll()
{
    return rules;
}

// =====================================================
// CARGA DESDE JSON
// =====================================================

bool alarmRulesLoadFromJson(const JsonArray& arr)
{
    rules.clear();

    for (JsonObject o : arr)
    {
        AlarmRule r{};

        r.id       = o["ID"]  | "";
        r.mensaje  = o["MSG"] | "";
        r.activa   = o["EN"]  | true;
        r.grupo    = o["G"]   | 0;
        r.retardoMs= o["DELAY"] | 0;

        // ---- expresión ----
        r.condicion = parseAlarmExpr(o["COND"]);
        if (!r.condicion)
        {
            escribirLog("ALM:RULE_ERR:%s", r.id);
            continue;
        }

        rules.push_back(r);
    }

    escribirLog("ALM:RULES:%d", rules.size());
    return true;
}
