// src/net/RedManager.h
#pragma once
#include "config/config_struct.h"

class RedManager
{
public:
    static bool hayRed();
    static const char* redActivaStr();
    static void init(const Configuracion& cfg);
    static void init();
};
