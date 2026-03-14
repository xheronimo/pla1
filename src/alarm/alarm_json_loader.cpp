#include "alarm/alarm_json_loader.h"
#include "alarm/alarm_manager.h"
#include <ArduinoJson.h>
#include <FS.h>

namespace AlarmMgr {

    // Helper recursivo para cargar el árbol de expresiones
    static AlarmExpr* parseExpr(JsonObject obj) {
        if (obj.isNull()) return nullptr;

        // Determinar el tipo de expresión
        const char* typeStr = obj["type"] | "COND";
        AlarmExprType type = AlarmExprType::EXPR_COND;
        
        if (strcmp(typeStr, "AND") == 0) type = AlarmExprType::EXPR_AND;
        else if (strcmp(typeStr, "OR") == 0)  type = AlarmExprType::EXPR_OR;
        else if (strcmp(typeStr, "NOT") == 0) type = AlarmExprType::EXPR_NOT;

        AlarmExpr* node = new AlarmExpr(type);

        if (type == AlarmExprType::EXPR_COND) {
            // Cargar datos de comparación
            node->signalId = obj["signalId"] | "";
            node->value = obj["threshold"] | 0.0f;
            node->hysteresis = obj["hysteresis"] | 0.0f;
            node->useHys = obj.containsKey("hysteresis");
            
            const char* opStr = obj["op"] | "GT";
            if (strcmp(opStr, "GT") == 0)      node->op = Op::GT;
            else if (strcmp(opStr, "LT") == 0) node->op = Op::LT;
            else if (strcmp(opStr, "GE") == 0) node->op = Op::GE;
            else if (strcmp(opStr, "LE") == 0) node->op = Op::LE;
            else if (strcmp(opStr, "EQ") == 0) node->op = Op::EQ;
            else if (strcmp(opStr, "NE") == 0) node->op = Op::NE;
            else if (strcmp(opStr, "RISE") == 0) node->op = Op::RISE;
            else if (strcmp(opStr, "FALL") == 0) node->op = Op::FALL;
        } else {
            // Cargar ramas recursivamente
            if (obj.containsKey("left"))  node->left = parseExpr(obj["left"]);
            if (obj.containsKey("right")) node->right = parseExpr(obj["right"]);
            if (obj.containsKey("child")) node->child = parseExpr(obj["child"]);
        }

        return node;
    }

    bool loadFromFS(FS &fs, const char* path) {
        File file = fs.open(path, "r");
        if (!file) {
            Serial.println("[AlarmLoader] Error: No se pudo abrir el archivo de alarmas.");
            return false;
        }

        // Usamos DynamicJsonDocument para manejar el tamaño variable del vector
        DynamicJsonDocument doc(16384); // Ajustar según complejidad de alarmas
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.print("[AlarmLoader] Error parseando JSON: ");
            Serial.println(error.c_str());
            return false;
        }

        // Limpiamos las reglas actuales antes de cargar las nuevas
        AlarmMgr::clearRules();

        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obj : arr) {
            AlarmRule rule;
            rule.alarmId = obj["id"] | 0;
            rule.name = obj["name"] | "Sin nombre";
            rule.latch = obj["latch"] | false;
            
            // Mapeo de Severidad
            const char* sevStr = obj["severity"] | "INFO";
            if (strcmp(sevStr, "CRITICAL") == 0) rule.severity = AlarmSeverity::CRITICAL;
            else if (strcmp(sevStr, "WARNING") == 0) rule.severity = AlarmSeverity::WARNING;
            else if (strcmp(sevStr, "MAINTENANCE") == 0) rule.severity = AlarmSeverity::MAINTENANCE;
            else rule.severity = AlarmSeverity::INFO;

            // Mapeo de Grupo
            rule.group = (AlarmGroup)(obj["group"] | 0);

            // --- CARGAR ACCIÓN FÍSICA ---
            rule.hasAction = obj.containsKey("action");
            if (rule.hasAction) {
                JsonObject act = obj["action"];
                strncpy(rule.actionChipId, act["chipId"] | "NONE", 16);
                rule.actionChannel = act["channel"] | 0;
                rule.actionValue = act["value"] | 0.0f;
            }

            // --- CARGAR NOTIFICACIONES ---
            rule.sendMqtt = obj["mqtt"] | true;
            rule.sendTelegram = obj["telegram"] | false;
            rule.sendWeb = obj["web"] | true;

            // Cargar árbol de expresiones
            rule.expr = parseExpr(obj["expression"]);

            // Estado inicial
            rule.isActive = false;
            rule.isAcknowledged = false;

            // Añadir al vector dinámico
            AlarmMgr::addRule(rule);
        }

        Serial.printf("[AlarmLoader] %d alarmas cargadas correctamente.\n", arr.size());
        return true;
    }
}