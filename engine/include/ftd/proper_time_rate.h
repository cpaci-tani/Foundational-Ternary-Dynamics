#pragma once
// Compatibility include.  FTD-0402 moved the single CPU/CUDA definition of
// proper_time_rate() into causal_kinematics.h with explicit raw-speed/C_SPEED
// normalization.  Keep this filename so existing consumers do not fork the
// contract or need a source-level API migration.
#include "causal_kinematics.h"
