#pragma once
#include "signal_struct.h"



float applyCalibration(float raw,
                       Calibration& calib,
                       bool& stable,
                       bool& clamped);
                       
                       