#include "DisplaySSD1306.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#include "net/RedManager.h"
#include "net/mqtt_manager.h"
#include "modbus/modbus_config.h"
#include "modbus/modbus_guard.h"
#include "system/RelojSistema.h"
#include "system/WatchdogManager.h"
#include "alarm/alarm_struct.h"
#include "alarm/alarm_runtime.h"
#include "system/i2c_health.h"
#include "system/system_fault.h"


// ===================================================
// EXTERNOS DE ESTADO GLOBAL
// ===================================================
extern bool g_modbusDegraded;
extern bool g_lastResetWdt;

static char bootLines[6][22];
static uint8_t bootIndex = 0;

// ===================================================
// DISPLAY
// ===================================================

// --------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
// --------------------------------------------------

static Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET);

static bool activo = true;


// ===================================================
// LOOP DISPLAY
// ===================================================
void displayLoop()
{
    if (!activo)
        return;

    static uint32_t lastDraw = 0;
    static uint32_t lastPageChange = 0;
    static uint8_t displayPage = PAGE_GENERAL;

    const uint32_t DRAW_MS = 1000;
    const uint32_t PAGE_TIME_MS = 4000;

    uint32_t now = millis();

    if (now - lastDraw < DRAW_MS)
        return;
    lastDraw = now;

    // =================================================
    // Selección de página
    // =================================================
    if (g_lastResetWdt)
    {
        displayPage = PAGE_FAULT; // ⚠️ prioridad absoluta
    }
    else if (now - lastPageChange > PAGE_TIME_MS)
    {
        displayPage++;
        if (displayPage >= PAGE_MAX_NORMAL)
            displayPage = PAGE_GENERAL;

        lastPageChange = now;
    }

    // =================================================
    // DIBUJO
    // =================================================
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // -------------------------------------------------
    // PÁGINA GENERAL
    // -------------------------------------------------
    if (displayPage == PAGE_GENERAL)
    {
        char hora[16];
        obtenerHoraHHMM(hora);

        display.print("Hora: ");
        display.println(hora);
        display.println("----------------");

        display.print("Red: ");
        display.println(RedManager::redActivaStr());

        display.print("MQTT: ");
        display.println(MQTTManager::isConnected() ? "OK" : "OFF");
    }

    // -------------------------------------------------
    // PÁGINA SISTEMA
    // -------------------------------------------------
    else if (displayPage == PAGE_SYSTEM)
    {
        uint32_t up = millis() / 1000;

        display.print("UP: ");
        display.print(up / 3600);
        display.print("h ");
        display.print((up % 3600) / 60);
        display.println("m");

        display.print("HEAP: ");
        display.print(ESP.getFreeHeap() / 1024);
        display.println("k");

        display.println("----------------");
        display.print("WDT: ");
display.println(systemHasCriticalFault() ? "FAULT" : "OK");

        display.println("----------------");

        if (g_lastI2CErrTs > 0)
        {
            display.print("I2C ERR: ");
            display.print("0x");
            display.println(g_lastI2CAddr, HEX);
        }
        else
        {
            display.println("I2C: OK");
        }
    }

    // -------------------------------------------------
    // PÁGINA ALARMAS
    // -------------------------------------------------
    else if (displayPage == PAGE_ALARMS)
    {
        display.println("ALARMAS");
        display.println("----------------");

        size_t ac;
        const AlarmRuntime *ars = alarmRuntimeAll(ac);

        if (ac == 0)
        {
            display.println("Sin alarmas");
        }
        else
        {
            uint8_t shown = 0;
            for (size_t i = 0; i < ac && shown < 4; i++)
            {
                const AlarmRuntime &a = ars[i];

                display.print("A");
                display.print(a.alarmId);
                display.print(": ");

                if (a.blocked)
                    display.println("BLOCK");
                else if (a.active)
                    display.println("ACTIVE");
                else
                    display.println("OK");

                shown++;
            }
        }
    }

    // -------------------------------------------------
    // PÁGINA MODBUS
    // -------------------------------------------------
    else if (displayPage == PAGE_MODBUS)
    {
        display.println("MODBUS");
        display.println("----------------");

        display.print("MODE: ");
        display.println(g_modbusDegraded ? "DEGRADED" : "NORMAL");

        display.print("MAX SLV: ");
        display.println(MODBUS_MAX_SLAVES);

    }

    // -------------------------------------------------
    // PÁGINA FALLO CRÍTICO
    // -------------------------------------------------
    else if (displayPage == PAGE_FAULT)
    {
        display.println("!! SYSTEM FAULT !!");
display.println("----------------");

if (g_lastResetWdt)
    display.println("CAUSE: WDT RESET");
else
    display.println("CAUSE: RUNTIME");

display.println("CHECK SYSTEM");
if (g_safeMode)
{
    display.clearDisplay();
    display.println("!! SAFE MODE !!");
    display.println("HW BLOQUEADO");
    display.println("----------------");
    display.println("Reles OFF");
    display.println("Modbus OFF");
    display.println("Web ACTIVE");
    display.display();
    return;
}
    }

    display.display();


}

// --------------------------------------------------
// TASK
// --------------------------------------------------



bool displayInit()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
        return false;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    activo = true;

    display.setCursor(0, 0);
    display.println("ESP32 SYSTEM");
    display.println("----------------");
    display.display();

    return true;
}
bool displayActivo()
{
    return activo;
}

void displayClear()
{
    if (!activo)
        return;
    display.clearDisplay();
    display.display();
}

// --------------------------------------------------
// BOOT PRINT
// --------------------------------------------------
void displayBootPrint(const char *msg)
{
    if (!activo)
        return;

    strncpy(bootLines[bootIndex % 6], msg, 21);
    bootLines[bootIndex % 6][21] = 0;
    bootIndex++;

    display.clearDisplay();
    display.setCursor(0, 0);

    uint8_t start = (bootIndex > 6) ? bootIndex - 6 : 0;

    for (uint8_t i = start; i < bootIndex; i++)
        display.println(bootLines[i % 6]);

    display.display();
}
