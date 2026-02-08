#include "system_boot.h"
#include <Arduino.h>

// ================================
// CONFIGURACIÓN
// ================================
#define PIN_RECOVERY 33
#define PIN_SAFE     32

#define BOOT_LOOP_WINDOW_MS 60000
#define BOOT_LOOP_MAX       3

// ================================
// RTC PERSISTENTE
// ================================
RTC_DATA_ATTR uint8_t  bootFailCount = 0;
RTC_DATA_ATTR uint32_t lastBootTs     = 0;

// ================================
// PINES
// ================================
static bool isRecoveryPinActive()
{
    pinMode(PIN_RECOVERY, INPUT_PULLUP);
    delay(5);
    return digitalRead(PIN_RECOVERY) == LOW;
}

static bool isSafePinActive()
{
    pinMode(PIN_SAFE, INPUT_PULLUP);
    delay(5);
    return digitalRead(PIN_SAFE) == LOW;
}

// ================================
// BOOT LOOP
// ================================
static bool watchdogDetectedRebootLoop()
{
    uint32_t now = millis();

    if (now - lastBootTs > BOOT_LOOP_WINDOW_MS)
        bootFailCount = 0;

    lastBootTs = now;
    bootFailCount++;

    return bootFailCount >= BOOT_LOOP_MAX;
}

void markBootSuccess()
{
    bootFailCount = 0;
}

// ================================
// FLAGS PERSISTENTES (stub)
// ================================
void setSafeModeFlag(bool enable)
{
    // futuro: NVS / Preferences
    (void)enable;
}

bool systemFlagSafeMode()
{
    return false;
}

// ================================
// DETECCIÓN FINAL
// ================================
SystemMode detectSystemMode()
{
    // 1️⃣ RECOVERY físico
    if (isRecoveryPinActive())
        return SystemMode::RECOVERY;

    // 2️⃣ SAFE físico
    if (isSafePinActive())
        return SystemMode::SAFE;

    // 3️⃣ Reboot loop
    if (watchdogDetectedRebootLoop())
        return SystemMode::SAFE;

    // 4️⃣ Flag persistente
    if (systemFlagSafeMode())
        return SystemMode::SAFE;

    return SystemMode::NORMAL;
}
