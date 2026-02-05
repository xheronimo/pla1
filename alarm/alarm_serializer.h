#pragma once

#include <ArduinoJson.h>
#include "alarm_struct.h"

// Serializa un AlarmEvent a JSON
void serializeAlarmEvent(
    JsonObject obj,
    const AlarmEvent& ev
);
