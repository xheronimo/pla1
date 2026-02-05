#include "ModemManager.h"
#include "config/config_struct.h"

// ===============================
// STUB – sin implementación real
// ===============================

static ModemStatus status;

void ModemManager::inicializar()
{
    // Stub: todo apagado
    status.listo = false;
    status.red   = false;
    status.ppp   = false;
}

bool ModemManager::isConnected()
{
    return status.ppp;
}

ModemStatus ModemManager::getStatus()
{
    return status;
}

bool ModemManager::sendSMS(const char*, const char*)
{
    // Stub: SMS no disponible
    return false;
}

void ModemManager::init(){}

void ModemManager::init(const Configuracion& cfg)
{
   
}