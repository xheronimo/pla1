#pragma once


static constexpr uint32_t DEV_PCF_IN_1  = 101;
static constexpr uint32_t DEV_PCF_IN_2  = 102;
static constexpr uint32_t DEV_PCF_OUT_1 = 201;
static constexpr uint32_t DEV_PCF_OUT_2 = 202;
static constexpr uint32_t DEV_GPIO_ADC  = 301;
static constexpr uint32_t DEV_ONEWIRE   = 401;
static constexpr uint32_t DEV_ONEWIRE_1   = 402;
static constexpr uint32_t DEV_ONEWIRE_2   = 403;



#if defined(BOARD_V1)
    #include "board_v1.h"

#elif defined(BOARD_V3)
    #include "board_v3.h"

#else
    #error "Debes definir BOARD_V1 o BOARD_V3 en build_flags"
#endif
