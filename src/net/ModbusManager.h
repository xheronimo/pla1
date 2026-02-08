#pragma once

class ModbusManager {
public:
    static bool leerRegistro(int index, float& outValue);
};
