#pragma once
#include <ArduinoJson.h>
#include "alarm/alarm_struct.h"

void alarmEventToJson(const AlarmEvent& ev, JsonObject obj);
