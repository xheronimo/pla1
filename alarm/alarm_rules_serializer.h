#pragma once

#include <ArduinoJson.h>

// Guarda todas las reglas de alarma desde RAM a un JsonDocument
// Devuelve true si todo fue correcto
bool alarmRulesSaveToJson(JsonDocument& doc);
