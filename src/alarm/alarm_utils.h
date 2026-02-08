#pragma once
#include <stdint.h>
#include "alarm_struct.h"
#include "alarm/alarm_expr.h"
#include "alarm_struct.h"




const char* alarmSeverityToStr(AlarmSeverity s);
const char* alarmGroupToStr(AlarmGroup g);
AlarmGroup parseAlarmGroup(const char* s);
AlarmSeverity parseAlarmSeverity(const char* s);


void freeExpr(AlarmExpr* e);

const char* alarmEventKindToStr(AlarmEventKind k);
