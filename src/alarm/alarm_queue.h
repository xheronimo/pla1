#pragma once
#include <cstddef>
#include "alarm_struct.h"

void alarm_queueInit();
void alarm_queuePush(const AlarmEvent& ev);
bool alarm_queuePop(AlarmEvent& ev);
size_t alarm_queueSize();
