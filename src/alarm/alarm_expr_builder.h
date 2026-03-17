#pragma once
#include <ArduinoJson.h>
#include "alarm_expr.h"

// Construye recursivamente un árbol de expresiones
AlarmExpr* buildExpr(const JsonObjectConst& o);
