#include "alarm_rules_loader.h"
#include "alarm_rules.h"
#include "alarm_expr.h"
#include "system/LogSystem.h"

// ================= PARSER EXPRESIÓN =================

static AlarmExpr* parseExpr(JsonVariant v)
{
    if (!v.is<JsonObject>())
        return nullptr;

    JsonObject o = v.as<JsonObject>();

    if (o.containsKey("AND")) {
        AlarmExpr* e = new AlarmExpr(EXPR_AND);
        for (auto c : o["AND"].as<JsonArray>())
            if (auto* ch = parseExpr(c)) e->children.push_back(ch);
        return e;
    }

    if (o.containsKey("OR")) {
        AlarmExpr* e = new AlarmExpr(EXPR_OR);
        for (auto c : o["OR"].as<JsonArray>())
            if (auto* ch = parseExpr(c)) e->children.push_back(ch);
        return e;
    }

    if (o.containsKey("NOT")) {
        AlarmExpr* e = new AlarmExpr(EXPR_NOT);
        if (auto* ch = parseExpr(o["NOT"])) e->children.push_back(ch);
        return e;
    }

    if (!o.containsKey("SRC") ||
        !o.containsKey("OP")  ||
        !o.containsKey("VAL"))
        return nullptr;

    AlarmOp op;
    const char* sop = o["OP"];

    if      (!strcmp(sop,"==")) op = OP_EQ;
    else if (!strcmp(sop,"!=")) op = OP_NE;
    else if (!strcmp(sop,">"))  op = OP_GT;
    else if (!strcmp(sop,"<"))  op = OP_LT;
    else if (!strcmp(sop,">=")) op = OP_GE;
    else if (!strcmp(sop,"<=")) op = OP_LE;
    else return nullptr;

    return new AlarmExpr(o["SRC"], op, o["VAL"]);
}

// ================= CARGA REGLAS =================

bool alarmRulesLoadFromJson(const JsonArray& arr)
{
    alarmRulesClear();

    for (JsonObject rj : arr)
    {
        AlarmRule r;

        r.id        = rj["ID"]    | "";
        r.mensaje   = rj["MSG"]   | r.id;
        r.activa    = rj["EN"]    | 0;
        r.grupo     = rj["G"]     | 0;
        r.retardoMs = (rj["DELAY"] | 0) * 1000;

        r.condicion = parseExpr(rj["COND"]);
        if (!r.condicion) {
            escribirLog("ALM:RULE_INVALID:%s", r.id);
            continue;
        }

        alarmRulesAdd(r);
    }
    return true;
}
