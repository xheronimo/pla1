#include "api_signal_guard.h"
#include "signal/signal_manager.h"
#include "signal_parser.h"

// --------------------------------------------------
// VALIDAR ACTUALIZACIÓN
// --------------------------------------------------

SignalUpdateResult validateSignalUpdate(
    const Signal& current,
    JsonObjectConst update)
{
    // --------------------------------------------------
    // Señal reservada → no se puede tocar estructura
    // --------------------------------------------------
    if (current.systemReserved)
    {
        if (update.containsKey("bus")     ||
            update.containsKey("address") ||
            update.containsKey("channel") ||
            update.containsKey("chip")    ||
            update.containsKey("kind"))
        {
            return SignalUpdateResult::RESERVED;
        }
    }

    // --------------------------------------------------
    // Evitar cambiar tipo
    // --------------------------------------------------
    if (update.containsKey("kind"))
    {
        SignalKind newKind = parseSignalKind(update["kind"]);
        if (newKind != current.kind)
            return SignalUpdateResult::INVALID;
    }

    // --------------------------------------------------
    // Duplicado físico
    // --------------------------------------------------
    if (update.containsKey("bus") ||
        update.containsKey("address") ||
        update.containsKey("channel"))
    {
        Signal test = current;

        if (update.containsKey("bus"))
            test.bus = parseBusType(update["bus"]);

        if (update.containsKey("address"))
            test.address = update["address"];

        if (update.containsKey("channel"))
            test.channel = update["channel"];

        if (signalPhysicalExists(test, current.id))
            return SignalUpdateResult::DUPLICATE;
    }

    return SignalUpdateResult::OK;
}
