#include "config_global.h"
#include "config_struct.h"
#include <strings.h>

// ==================================================
// INSTANCIA GLOBAL
// ==================================================

Configuracion cfg;

SystemMode parseSystemMode(const char* s)
{
    if (!s)
        return SystemMode::NORMAL;

    // todo en minúsculas en el JSON
    if (strcasecmp(s, "safe") == 0)
        return SystemMode::SAFE;

    if (strcasecmp(s, "recovery") == 0)
        return SystemMode::RECOVERY;

    return SystemMode::NORMAL;
}
