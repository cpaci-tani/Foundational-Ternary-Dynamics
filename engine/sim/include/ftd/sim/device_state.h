#pragma once
/**
 * @file ftd/sim/device_state.h
 * @brief Backend-agnostic handle to a lattice simulation in flight.
 *
 * A Pipeline<Backend> owns exactly one DeviceState (via the Backend::DeviceState
 * template parameter). Observables take the DeviceState& in their measure()
 * method and emit either a device-side reduction kernel (GPU backend) or a
 * host-side loop (CPU backend). The DeviceState is the common vocabulary
 * between the orchestrator (pipeline.h) and the observers.
 *
 * We intentionally do NOT expose raw device pointers here — that is a
 * backend-specific concern. DeviceState exposes only:
 *   - what observables legitimately need (L, tick, field accessors)
 *   - small state-mutation hooks for the pipeline (inject_*, set_toggles)
 *   - an opaque "run N ticks" method
 *
 * CPU backend: `BackendCpu::DeviceState` wraps an owned RenderBridge.
 * GPU backend: `BackendGpu::DeviceState` wraps an owned gpu::GpuEngine +
 *              its embedded GpuBuffers + host-shadow voxels for inspection.
 */

#include <cstddef>
#include <cstdint>

#include "ftd/render_bridge.h"       // for RenderBridge and TermToggles

namespace ftd {
namespace sim {

// Forward declarations — backend-specific DeviceState implementations are
// in backend_cpu.h and backend_gpu.h respectively. The struct definitions
// sit in Backend::DeviceState nested types so Pipeline<Backend> can access
// them via template parameter.
struct BackendCpu;
#ifdef FTD_ENABLE_CUDA
struct BackendGpu;
#endif

}  // namespace sim
}  // namespace ftd
