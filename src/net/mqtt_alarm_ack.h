#pragma once
#include <ArduinoJson.h>

void handleAlarmAck(const char* payload);
void handleAlarmAck(uint32_t alarmId);