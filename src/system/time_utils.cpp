#include "time_utils.h"


uint32_t getTimestamp()
{
    // TODO: cuando haya RTC/NTP devolver epoch real
    return millis() / 1000;
}
