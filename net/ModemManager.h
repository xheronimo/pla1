#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config/config_struct.h"

// ===============================
// ESTADO DEL MODEM
// ===============================
struct ModemStatus
{
    bool listo = false;   // módem inicializado
    bool red   = false;   // registrado en red
    bool ppp   = false;   // conexión de datos
};

// ===============================
// MODEM MANAGER (STUB)
// ===============================
class ModemManager
{
public:
    static void inicializar();
    static bool isConnected();

    static ModemStatus getStatus();

    // SMS
    static bool sendSMS(const char* telefono, const char* msg);
    static void init();
    static void init(const Configuracion& cfg);
};
