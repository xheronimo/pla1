#pragma once

#if defined(BOARD_V1)
    #include "board_v1.h"

#elif defined(BOARD_V3)
    #include "board_v3.h"

#else
    #error "Debes definir BOARD_V1 o BOARD_V3 en build_flags"
#endif
