#include "BootLog.h"
#include "Display/DisplaySSD1306.h"
#include <Arduino.h>

void bootLog(const char* msg)
{
    Serial.println(msg);

    if (displayActivo())
    {
        displayBootPrint(msg);
    }
}
