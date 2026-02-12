#include "signal_virtual.h"
#include "signal_manager.h"

bool signalDiff(const char* a, const char* b, float& out)
{
    float va, vb;
    if (!signalManagerGetValue(a, va)) return false;
    if (!signalManagerGetValue(b, vb)) return false;

    out = va - vb;
    return true;
}

bool signalSum(const char* a, const char* b, float& out)
{
    float va, vb;
    if (!signalManagerGetValue(a, va)) return false;
    if (!signalManagerGetValue(b, vb)) return false;

    out = va + vb;
    return true;
}

bool signalAnd(const char* a, const char* b, float& out)
{
    float va, vb;
    if (!signalManagerGetValue(a, va)) return false;
    if (!signalManagerGetValue(b, vb)) return false;

    out = (va != 0 && vb != 0) ? 1.0f : 0.0f;
    return true;
}
