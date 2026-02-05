#pragma once
#include <ArduinoJson.h>

bool alarmDestinationsLoadFromJson(const JsonArray& arr);
bool alarmDestinationsSaveToJson(JsonDocument& doc);
