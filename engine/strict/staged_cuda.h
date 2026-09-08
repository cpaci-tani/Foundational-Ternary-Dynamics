#pragma once
#include "staged_runtime.h"

namespace ftd::strict::gpu {
// Actual device kernels; failure to initialize CUDA is an error, never fallback.
StepResult step(const State& state);
std::vector<Events> advance(State& state, std::uint64_t ticks);
std::string device_json();
}
