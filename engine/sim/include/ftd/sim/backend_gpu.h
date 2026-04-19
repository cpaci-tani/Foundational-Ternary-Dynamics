#pragma once
/**
 * @file ftd/sim/backend_gpu.h
 * @brief GPU backend specialisation for Pipeline<Backend>.
 *
 * Phase C baseline design (correctness-first):
 *
 *   BackendGpu::DeviceState wraps a gpu::GpuEngine and maintains a
 *   lazily-synced host-shadow of the voxel buffer. When an observable
 *   calls voxels(), we call GpuEngine::sync_to_host() first to refresh
 *   the shadow. The CPU observable implementation then runs over the
 *   shadow via the same code path as BackendCpu.
 *
 *   This baseline approach:
 *     - PROS: trivially correct (no new reduction kernels to write),
 *             observables don't need a GPU specialisation to work
 *     - CONS: one full-lattice PCIe download per measurement
 *
 *   In Phase D we add cub-based reduction kernels for the hot-path
 *   observables. The Phase-C baseline stays as the "reference
 *   implementation" — any future GPU observable can start by just
 *   forwarding to the CPU code, then optimise.
 *
 * The `#ifdef FTD_ENABLE_CUDA` guard makes this header safe to include
 * on Windows CPU builds: BackendGpu is declared-only when CUDA is off,
 * so user code that writes `Pipeline<BackendGpu>` fails at link time
 * rather than compile time (clearer error).
 */

#ifdef FTD_ENABLE_CUDA

#include <cstdint>
#include <memory>
#include <vector>

#include "ftd/gpu_engine.h"
#include "ftd/lattice.h"
#include "ftd/term_toggles.h"
#include "ftd/voxel.h"

namespace ftd {
namespace sim {

struct BackendGpu {
    struct DeviceState {
        explicit DeviceState(int L)
            : engine(L), lattice_(L), host_shadow_() {}

        DeviceState(const DeviceState&) = delete;
        DeviceState& operator=(const DeviceState&) = delete;

        gpu::GpuEngine engine;

        /// Accessors — matching BackendCpu::DeviceState surface.
        int L() const { return engine.lattice_size(); }
        int N() const { return engine.total_sites(); }
        int tick() const { return engine.current_tick(); }
        const Lattice& lattice() const { return lattice_; }

        /// Return the host-shadow voxel buffer, refreshing it from the
        /// device first. O(N) PCIe transfer per call — observables that
        /// call this repeatedly in a tight loop should cache the
        /// reference across measurements where possible.
        const std::vector<Voxel>& voxels() {
            refresh_host_shadow();
            return host_shadow_;
        }

        /// Configuration surface.
        void set_toggles(const TermToggles& t) { engine.toggles = t; }

        void inject_flux(int x, int y, int z, const Vec3& J) {
            engine.inject_flux(x, y, z, J);
            host_shadow_stale_ = true;
        }
        void inject_particle(int x, int y, int z, int8_t s, const Vec3& J) {
            engine.inject_particle(x, y, z, s, J);
            host_shadow_stale_ = true;
        }
        /// NB: GpuEngine does not expose per-voxel `locked` mutation
        /// directly; lock via a full host-round-trip is expensive.
        /// Observables that need locked voxels should use inject_particle
        /// with state != 0 (which the engine treats as immovable).
        void lock(int /*x*/, int /*y*/, int /*z*/) {
            // NOTE: Phase C baseline — locking not supported on GPU
            //       backend without a dedicated kernel. Placeholder for
            //       Phase D (when we'll add a per-voxel `locked` kernel).
        }

        /// Advance simulation.
        void tick_once() {
            engine.tick();
            host_shadow_stale_ = true;
        }
        void run(int n) {
            engine.run(n);
            host_shadow_stale_ = true;
        }

        EnergyAudit energy_audit() { return engine.energy_audit(); }

    private:
        Lattice lattice_;                     ///< cheap, non-GPU-backed, for index()/wrap()
        std::vector<Voxel> host_shadow_;      ///< filled on demand by refresh_host_shadow()
        bool host_shadow_stale_ = true;       ///< true iff device state is newer than shadow

        void refresh_host_shadow() {
            if (!host_shadow_stale_) return;
            engine.sync_to_host(host_shadow_);
            host_shadow_stale_ = false;
        }
    };

    static constexpr const char* name() { return "gpu"; }
};

}  // namespace sim
}  // namespace ftd

#endif  // FTD_ENABLE_CUDA
