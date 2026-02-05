#include "alarm_rules_serializer.h"

#include "alarm_rules.h"
#include "alarm_expr.h"
#include "system/LogSystem.h"

// =======================================================
// SERIALIZAR EXPRESIÓN (RECURSIVO)
// =======================================================

static void serializeExpr(JsonObject obj, const AlarmExpr* e)
{
    if (!e) return;

    switch (e->type)
    {
        case EXPR_AND:
        {
            JsonArray arr = obj["AND"].to<JsonArray>();
            for (auto* c : e->children)
            {
                JsonObject child = arr.add<JsonObject>();
                serializeExpr(child, c);
            }
            break;
        }

        case EXPR_OR:
        {
            JsonArray arr = obj["OR"].to<JsonArray>();
            for (auto* c : e->children)
            {
                JsonObject child = arr.add<JsonObject>();
                serializeExpr(child, c);
            }
            break;
        }

        case EXPR_NOT:
        {
            JsonObject n = obj["NOT"].to<JsonObject>();
            if (!e->children.empty())
                serializeExpr(n, e->children[0]);
            break;
        }

        case EXPR_COND:
        {
            obj["SRC"] = e->src;
            obj["OP"]  = alarmOpToString(e->op);
            obj["VAL"] = e->value;
            break;
        }
    }
}

// =======================================================
// SERIALIZAR TODAS LAS REGLAS
// =======================================================

bool alarmRulesSaveToJson(JsonDocument& doc)
{
    const auto& rules = alarmRulesGetAll();

    if (rules.empty())
        return true;

    JsonArray arr = doc["AR"].to<JsonArray>();

    for (const auto& r : rules)
    {
        if (!r.condicion)
        {
            escribirLog("ALM:SKIP_RULE:%s", r.id);
            continue;
        }

        JsonObject o = arr.add<JsonObject>();

        o["ID"]    = r.id;
        o["EN"]    = r.activa ? 1 : 0;
        o["G"]     = r.grupo;
        o["DELAY"] = r.retardoMs;
        o["MSG"]   = r.mensaje;

        JsonObject cond = o["COND"].to<JsonObject>();
        serializeExpr(cond, r.condicion);
    }

    return true;
}
