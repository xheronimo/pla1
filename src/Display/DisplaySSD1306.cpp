#include "Display/DisplaySSD1306.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "system/BootLog.h"
#include "system/WatchdogManager.h"
#include "net/RedManager.h"
#include "net/mqtt_manager.h"
#include "net/ModemManager.h"
#include "system/RelojSistema.h"


// --------------------------------------------------
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET     -1
#define OLED_ADDR      0x3C
// --------------------------------------------------

static Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

static bool activo = false;

// --------------------------------------------------
// buffer boot
// --------------------------------------------------
static char bootLines[6][22];
static uint8_t bootIndex = 0;

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
    if (!activo) return;
    display.clearDisplay();
    display.display();
}

// --------------------------------------------------
// BOOT PRINT
// --------------------------------------------------
void displayBootPrint(const char* msg)
{
    if (!activo) return;

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

// --------------------------------------------------
// LOOP NORMAL
// --------------------------------------------------
void displayLoop()
{
    if (!activo) return;

    static uint32_t last = 0;
    if (millis() - last < 1000)
        return;
    last = millis();

    display.clearDisplay();
    display.setCursor(0, 0);

    // Hora
    char hora[16];
    obtenerHoraHHMM(hora);
    display.print("Hora: ");
    display.println(hora);

    display.println("----------------");

    // Red
    display.print("Red: ");
    display.println(RedManager::redActivaStr());

    // MQTT
    display.print("MQTT: ");
    display.println(MQTTManager::isConnected() ? "OK" : "OFF");

    // Modem
    ModemStatus ms = ModemManager::getStatus();
    display.print("4G: ");
    if (!ms.listo) display.println("OFF");
    else if (!ms.red) display.println("NO NET");
    else if (!ms.ppp) display.println("NO PPP");
    else display.println("ONLINE");

    display.display();
}

// --------------------------------------------------
// TASK
// --------------------------------------------------


void taskDisplay(void*)
{
    watchdogRegister(WDT_DISPLAY);

    for (;;)
    {
        displayLoop();
        watchdogKick(WDT_DISPLAY);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
