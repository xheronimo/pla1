#pragma once

struct ModemStatus {
    bool listo = false;
    bool red   = false;
    bool ppp   = false;
    int  rssi  = 0;
};

class ModemManager {
public:
    static ModemStatus getStatus() {
        return ModemStatus{};
    }
};
