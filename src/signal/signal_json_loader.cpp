#include "signal_json_loader.h"
#include "signal_manager.h"
#include "signal_parser.h"
#include "modbus/modbus_utils.h"
#include <ArduinoJson.h>
#include <string.h>
#include <stdlib.h>

extern Signal signalTable[];
extern size_t signalCount;
// ======================================================
// FORWARD DECLARATIONS
// ======================================================

static bool buildSignalFromJson(JsonObjectConst o, Signal& s);
static void applyEditableFields(Signal& s, JsonObjectConst o);
static bool isPhysicalDuplicate(const Signal& s, const char* ignoreId = nullptr);
static bool validateSignalConfig(const Signal& s);

// ======================================================
// BUSCAR POR ID
// ======================================================

Signal* signalManagerFindById(const char* id)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        if (strcmp(signalTable[i].id, id) == 0)
            return &signalTable[i];
    }
    return nullptr;
}

// ======================================================
// DETECTAR DUPLICADO FÍSICO (IGNORA MISMO ID)
// ======================================================

static bool isPhysicalDuplicate(const Signal& s, const char* ignoreId)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        const Signal& other = signalTable[i];

        if (ignoreId && strcmp(other.id, ignoreId) == 0)
            continue;

        if (other.bus     == s.bus &&
            other.address == s.address &&
            other.channel == s.channel)
        {
            return true;
        }
    }
    return false;
}

// ======================================================
// LOADER PRINCIPAL
// ======================================================

bool loadSignalsFromJson(const JsonArrayConst &arr)
{
    for (JsonObjectConst o : arr)
    {
        if (!o.containsKey("id"))
            continue;

        const char* id = o["id"] | "";

        Signal* existing = signalManagerFindById(id);

        // --------------------------------------------------
        // YA EXISTE
        // --------------------------------------------------
        if (existing)
        {
            // 🔒 Señal reservada → solo editable parcial
            if (existing->systemReserved)
            {
                applyEditableFields(*existing, o);
                continue;
            }

            // Señal normal → reconstruir completa
            Signal s{};
            if (!buildSignalFromJson(o, s))
                return false;

            if (isPhysicalDuplicate(s, id))
                return false;

            // Preservar flags internos
            bool sys     = existing->systemReserved;
            bool locked  = existing->lockedConfig;

            *existing = s;

            existing->systemReserved = sys;
            existing->lockedConfig   = locked;

            continue;
        }

        // --------------------------------------------------
        // NUEVA SEÑAL
        // --------------------------------------------------
        Signal s{};
        if (!buildSignalFromJson(o, s))
            return false;

        if (isPhysicalDuplicate(s))
            return false;

        if (!signalManagerAdd(s))
            return false;
    }

    return true;
}

// ======================================================
// BUILD COMPLETO DESDE JSON
// ======================================================

static bool buildSignalFromJson(JsonObjectConst o, Signal& s)
{
    memset(&s, 0, sizeof(Signal));

    // ⚠ IMPORTANTE: duplicamos ID para no usar memoria JSON temporal
    const char* id = o["id"] | "";
    if (!id || strlen(id) == 0)
        return false;

    s.id = strdup(id);

    s.kind    = parseSignalKind(o["kind"] | "digital");
    s.bus     = parseBusType(o["bus"] | "gpio");

    s.address = o["address"] | 0;
    s.channel = o["channel"] | 0;
    s.options = o["options"] | 0;

    s.invertido  = o["invert"] | false;
    s.debounceMs = o["debounce"] | 0;
    s.writable   = o["writable"] | false;

    s.systemReserved = o["systemReserved"] | false;
    s.lockedConfig   = o["lockedConfig"]   | false;

    s.initialized = false;
    s.error = false;
    s.errorCount = 0;
    s.lastOkTs = 0;

    // -------------------------------
    // I2C / PCF
    // -------------------------------
    if (s.bus == BusType::BUS_I2C ||
        s.bus == BusType::BUS_PCF)
    {
        s.chip = parseI2CDevice(o["chip"] | "none");
    }

    // -------------------------------
    // MODBUS
    // -------------------------------
    if (s.bus == BusType::BUS_MODBUS)
    {
        s.wordCount   = o["wordCount"] | 1;
        s.signedValue = o["signed"] | false;
        s.dataType    = parseModbusDataType(o["dataType"] | "uint16");
        s.wordOrder   = parseModbusWordOrder(o["wordOrder"] | "ab");
        s.modbusType  = parseModbusRegType(o["regType"] | "holding");
    }

    // -------------------------------
    // CALIBRACIÓN
    // -------------------------------
    if (s.kind == SignalKind::SENSOR_ANALOG &&
        o.containsKey("calib"))
    {
        JsonObjectConst c = o["calib"];

        s.calib.rawMin  = c["rawMin"]  | 0.0f;
        s.calib.rawMax  = c["rawMax"]  | 4095.0f;
        s.calib.realMin = c["realMin"] | 0.0f;
        s.calib.realMax = c["realMax"] | 100.0f;
        s.calib.clamp   = c["clamp"]   | true;
    }

    return validateSignalConfig(s);
}

// ======================================================
// CAMPOS EDITABLES (SEÑAL RESERVADA)
// ======================================================

static void applyEditableFields(Signal& s, JsonObjectConst o)
{
    // 🔒 Bloqueo absoluto
    if (s.lockedConfig)
        return;

    if (o.containsKey("invert"))
        s.invertido = o["invert"];

    if (o.containsKey("debounce"))
        s.debounceMs = o["debounce"];

    if (o.containsKey("options"))
        s.options = o["options"];

    if (o.containsKey("writable"))
        s.writable = o["writable"];

    if (s.kind == SignalKind::SENSOR_ANALOG &&
        o.containsKey("calib"))
    {
        JsonObjectConst c = o["calib"];

        s.calib.rawMin  = c["rawMin"]  | s.calib.rawMin;
        s.calib.rawMax  = c["rawMax"]  | s.calib.rawMax;
        s.calib.realMin = c["realMin"] | s.calib.realMin;
        s.calib.realMax = c["realMax"] | s.calib.realMax;
        s.calib.clamp   = c["clamp"]   | s.calib.clamp;
    }
}

// ======================================================
// VALIDACIÓN
// ======================================================

static bool validateSignalConfig(const Signal& s)
{
    if (!s.id || strlen(s.id) == 0)
        return false;

    switch (s.bus)
    {
        case BusType::BUS_GPIO:
            if (s.channel > 48) return false;
            break;

        case BusType::BUS_PCF:
            if (s.channel > 15) return false;
            break;

        case BusType::BUS_I2C:
            if (s.address == 0) return false;
            break;

        case BusType::BUS_MODBUS:
            if (s.address == 0) return false;
            break;

        default:
            return false;
    }

    return true;
}
