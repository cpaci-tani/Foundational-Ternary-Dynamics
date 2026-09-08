#pragma once
// Research candidate only: CUDA port of phi-hydro-staged-candidate-1
// (see hydro_runtime.h for the CPU reference this must match bit-for-bit).
#include "hydro_runtime.h"
#include <cstdint>
#include <string>

namespace ftd::hydro::gpu {
// Actual device kernels; failure to initialize CUDA is an error, never fallback.
// Advances `state` in place by `microticks` microticks and returns the per-category
// concatenation of every microtick's events, in tick order (same contract as
// ftd::hydro::advance).
Events advance(State& state, std::uint64_t microticks);
std::string device_json();
}  // namespace ftd::hydro::gpu
