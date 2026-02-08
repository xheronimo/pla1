#pragma once
#include <Arduino.h>

enum WatchdogId {
    WDT_MAIN = 0,
    WDT_ALARM_RULES,
    WDT_ALARM_DISPATCHER,
    WDT_DISPLAY,
    WDT_RELOJ,
    WDT_SENSORS,
    WDT_SENSOR,
WDT_ALARM,
    WDT_NET,
    WDT_MQTT,
    WDT_ALARM_DISPATCH,
    WDT_RULES,
    WDT_MAX
    

};

void watchdogInit(uint32_t timeoutMs );

void watchdogRegister(WatchdogId id);
void watchdogKick(WatchdogId id);
