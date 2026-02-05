// src/net/RedManager.cpp
#include "RedManager.h"
#include <WiFi.h>
#include <ETH.h>
#include "config/config_struct.h"

bool RedManager::hayRed()
{
    if (ETH.linkUp()) return true;
    if (WiFi.status() == WL_CONNECTED) return true;
    return false;
}

const char* RedManager::redActivaStr()
{
    if (ETH.linkUp()) return "ETH";
    if (WiFi.status() == WL_CONNECTED) return "WIFI";
    return "OFF";
}
void RedManager::init(const Configuracion& cfg)
{
   
}
void RedManager::init()
{
   
}
