#pragma once
#include "signal/signal_struct.h"
#include <ArduinoJson.h>

enum class SignalUpdateResult {
    OK,
    NOT_FOUND,
    RESERVED,
    DUPLICATE,
    INVALID,
};

SignalUpdateResult validateSignalUpdate(
    const Signal& current,
    JsonObjectConst update
);
