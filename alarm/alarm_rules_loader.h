#pragma once

#include <ArduinoJson.h>

// =====================================================
// CARGA DE REGLAS DESDE JSON
// =====================================================

// Espera:
// {
//   "AR": [
//     {
//       "ID": "A01",
//       "EN": 1,
//       "G": 0,
//       "DELAY": 3,
//       "MSG": "Fallo motor",
//       "COND": { ... }
//     }
//   ]
// }

bool alarmRulesLoadFromJson(const JsonDocument& doc);
