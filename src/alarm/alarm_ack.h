#pragma once
#include <stdint.h>

bool alarmIsAcked(const char* id);
void alarmAck(const char* id);
void alarmClearAck(const char* id);
