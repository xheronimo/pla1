// alarm_actions.h
#pragma once
#include <stdint.h>

struct AlarmActions {
    bool mqtt;
    bool web;
    bool telegram;
    bool sms;
};
