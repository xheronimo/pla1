#include "web_api_reset_wdt.h"

#include <WebServer.h>
#include <Preferences.h>
#include "system/LogSystem.h"

extern WebServer server;
extern uint32_t g_wdtResetCount;

void handleApiResetWdt()
{
    Preferences prefs;
    prefs.begin("boot", false);

    g_wdtResetCount = 0;
    prefs.putUInt("wdtCount", 0);

    prefs.end();

    escribirLog("API: Contador WDT reseteado");

    server.send(200, "application/json",
        "{\"ok\":true,\"message\":\"WDT counter cleared\"}");
}

void registerSystemResetApi()
{
    server.on("/api/system/reset_wdt", HTTP_POST, handleApiResetWdt);
}
