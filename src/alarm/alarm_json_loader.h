#pragma once

#include "alarm_expr.h"
#include "alarm_utils.h"
#include <ArduinoJson.h>

bool loadAlarmsFromJson(const JsonArrayConst& arr);
