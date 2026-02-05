#pragma once
#include <stddef.h>
#include "alarm_struct.h"

void alarm_queuePush(const AlarmEvent& ev);
bool alarm_queuePop(AlarmEvent& ev);
size_t alarm_queueSize();
