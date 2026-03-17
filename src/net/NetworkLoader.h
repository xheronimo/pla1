#ifndef NETWORK_LOADER_H
#define NETWORK_LOADER_H

#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>

namespace NetworkLoader {
    // Ahora loadAndSetup recibe la referencia al bus SPI que debe usar el Ethernet
    bool loadAndSetup(SPIClass &spiBus);
    
    bool setupEthernet(JsonObject config, SPIClass &spiBus);
    bool setupWiFiSTA(JsonObject config);
    void setupAP(JsonObject config);
    void setupRescueMode();
}

#endif