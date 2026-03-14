#pragma once
#include <Arduino.h>

enum WatchdogId
{
    WDT_MAIN = 0,
    WDT_ALARM_RULES,
    WDT_ALARM_DISPATCHER,
    WDT_DISPLAY,
    WDT_RELOJ,
    WDT_SENSORS,
    WDT_ALARM,
    WDT_NET,
    WDT_MQTT,
    WDT_ALARM_DISPATCH,
    WDT_RULES,
    WDT_I2C,
    WDT_MODBUS,
    WDT_MAX,

};

void watchdogInit(uint32_t timeoutMs);

void watchdogRegister(WatchdogId id);
void watchdogKick(WatchdogId id);
void watchdogCheck();

uint32_t watchdogLastKickAgo(WatchdogId id);
bool watchdogIsRegistered(WatchdogId id);