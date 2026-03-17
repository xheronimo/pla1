#include "net/NetworkLoader.h"
#include <Ethernet.h> // Para W5500
#include <WiFi.h>
#include "storage/SDManager.h"
#include "config/pines.h"

namespace NetworkLoader {

    bool loadAndSetup(SPIClass &spiBus) {
        JsonDocument doc;

        if (!SDMgr::loadConfig("/config/network.json", doc)) {
            SDMgr::logEvent("ERROR", "NET: Sin config. MODO RESCATE.");
            setupRescueMode();
            return false;
        }

        // 1. ETHERNET (Pasamos el bus SPI)
        bool ethLink = false;
        if (doc["ethernet"]["enabled"] | false) {
            ethLink = setupEthernet(doc["ethernet"], spiBus);
        }

        // 2. WIFI STA
        bool wifiSTA = false;
        if (doc["wifi_sta"]["enabled"] | false) {
            wifiSTA = setupWiFiSTA(doc["wifi_sta"]);
        }

        // 3. LÓGICA DE AP (Tu modelo original)
        if (ethLink) {
            SDMgr::logEvent("INFO", "NET: Ethernet OK. AP Cerrado por seguridad.");
            WiFi.softAPdisconnect(true);
        } else if (!wifiSTA) {
            setupAP(doc["ap"]);
        } else {
            setupAP(doc["ap"]);
            // NetworkMgr::activateAPTimer(...) si lo tienes implementado
        }

        return true;
    }

    bool setupEthernet(JsonObject config, SPIClass &spiBus) {
        // --- LA MAGIA ESTÁ AQUÍ ---
        // Inicializamos el driver Ethernet con el pin CS de la V3 y el BUS spiEth
        Ethernet.init(W5500_CS, &spiBus);

        // Configuración de IP (DHCP o Estática)
        if (!(config["dhcp"] | true)) {
            IPAddress ip, gw, mask, dns;
            ip.fromString(config["ip"] | "192.168.1.100");
            gw.fromString(config["gw"] | "192.168.1.1");
            mask.fromString(config["mask"] | "255.255.255.0");
            dns.fromString(config["dns"] | "8.8.8.8");
            
            Ethernet.begin(ip, dns, gw, mask);
        } else {
            // Intento de DHCP (bloqueante unos segundos)
            if (Ethernet.begin() == 0) {
                SDMgr::logEvent("WARN", "NET: Fallo DHCP Ethernet.");
                return false;
            }
        }

        // Verificar link físico
        if (Ethernet.linkStatus() == LinkON) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "NET: Eth OK. IP: %s", Ethernet.localIP().toString().c_str());
            SDMgr::logEvent("INFO", buffer);
            return true;
        }
        return false;
    }

 
bool setupWiFiSTA(JsonObject config) {
    const char* ssid = config["ssid"] | "";
    const char* pass = config["pass"] | "";

    if (strlen(ssid) == 0) return false;

    WiFi.begin(ssid, pass);
    
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) delay(500);

    if (WiFi.status() == WL_CONNECTED) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "NET: WiFi STA OK. IP: %s", WiFi.localIP().toString().c_str());
        SDMgr::logEvent("INFO", buffer);
        return true;
    }
    return false;
}

void setupAP(JsonObject config) {
    const char* ssid = config["ssid"] | "PLC_AP";
    const char* pass = config["pass"] | "admin1234";
    
    WiFi.softAP(ssid, pass);
    SDMgr::logEvent("INFO", "NET: AP levantado en 192.168.4.1");
}

void setupRescueMode() {
    // Generamos un SSID de rescate único basado en la MAC del dispositivo
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char rescueSSID[32];
    snprintf(rescueSSID, sizeof(rescueSSID), "RESCUE_PLC_%02X%02X", mac[4], mac[5]);

    WiFi.softAP(rescueSSID, "admin1234");
    SDMgr::logEvent("CRITICAL", "NET: Modo Rescate Activo.");
}
}