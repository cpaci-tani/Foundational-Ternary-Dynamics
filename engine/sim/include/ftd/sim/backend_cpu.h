#pragma once
/**
 * @file ftd/sim/backend_cpu.h
 * @brief CPU backend specialisation for Pipeline<Backend>.
 *
 * BackendCpu::DeviceState wraps an owned RenderBridge. All pipeline
 * operations forward to RenderBridge methods directly — no layering,
 * no extra allocations beyond the one RenderBridge owns for its voxel
 * buffer.
 *
 * This header is the only "backend description" needed for the CPU
 * path. Observables that support CPU expose a measure() overload that
 * takes BackendCpu::DeviceState& and reads RenderBridge::voxels()
 * directly.
 */

#include <cstdint>
#include <memory>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/term_toggles.h"
#include "ftd/voxel.h"

namespace ftd {
namespace sim {

struct BackendCpu {
    /// CPU-backend DeviceState is a thin wrapper over RenderBridge.
    /// Pipeline creates one of these per simulation, then calls its
    /// methods through the `state_` member.
    ///
    /// IMPORTANT: we call `force_cpu()` on the bridge in the constructor
    /// so that on CUDA-enabled builds this backend actually runs on the
    /// CPU code path. Without this, RenderBridge auto-selects the GPU
    /// backend when CUDA is available, and Pipeline<BackendCpu> silently
    /// becomes GPU — which would make GPU/CPU parity tests meaningless.
    struct DeviceState {
        explicit DeviceState(int L) : bridge(L) { bridge.force_cpu(); }

        // Non-copyable, non-movable (RenderBridge manages a large heap
        // buffer and is expensive to relocate).
        DeviceState(const DeviceState&) = delete;
        DeviceState& operator=(const DeviceState&) = delete;

        RenderBridge bridge;

        // Convenience accessors — observables read these.
        int L() const { return bridge.lattice().size(); }
        int N() const { return bridge.lattice().total_sites(); }
        int tick() const { return bridge.current_tick(); }

        const std::vector<Voxel>& voxels() const { return bridge.voxels(); }
        const Lattice& lattice() const { return bridge.lattice(); }

        // Pipeline mutation surface.
        void set_toggles(const TermToggles& t) { bridge.toggles = t; }
        void inject_flux(int x, int y, int z, const Vec3& J) {
            bridge.inject_flux(x, y, z, J);
        }
        void inject_particle(int x, int y, int z, int8_t s, const Vec3& J) {
            bridge.inject_particle(x, y, z, s, J);
        }
        void lock(int x, int y, int z) {
            bridge.voxel_at(x, y, z).locked = true;
        }

        // Advance the simulation.
        void tick_once() { bridge.tick(); }
        void run(int n) { bridge.run(n); }

        // Snapshot energy for observables that hook into the engine's
        // own energy audit rather than reimplementing reductions.
        EnergyAudit energy_audit() { return bridge.energy_audit(); }
    };

    /// Backend identifier for debug/logging.
    static constexpr const char* name() { return "cpu"; }
};

}  // namespace sim
}  // namespace ftd
