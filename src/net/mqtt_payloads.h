#pragma once

#include "alarm/alarm_struct.h"
#include "signal/signal_struct.h"

// SIGNALS
void mqttPublishSignal(const Signal& s);

// ALARMS
void mqttPublishAlarm(const AlarmEvent& ev);

// STATUS
void mqttPublishStatus(const char* msg);

void mqttPublishAlarmState(uint32_t alarmId, bool active, bool acked);

void mqttPublishAlarmAck(uint32_t alarmId);

void mqttPublishAlarmEvent(const AlarmEvent& ev, const AlarmRule& rule);