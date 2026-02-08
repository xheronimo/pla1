#pragma once
#include <stddef.h>
#include "alarm/alarm_struct.h"
#include "alarm_expr.h"

void alarmEvaluate();


static bool evalCond(const AlarmExpr* e, bool wasActive);
