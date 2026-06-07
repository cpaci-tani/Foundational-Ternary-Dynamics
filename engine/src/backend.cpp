/**
 * @file backend.cpp
 * @brief Backend implementations — CpuBackend + GpuBackend.
 *
 * ARCH-2 Phases B/C (CHECKLIST_ENGINE.md): the wrappers delegate to the
 * existing RenderBridge phase methods (CPU) and GpuEngine entry points (GPU).
 * No behavior change — this is the abstraction layer the next migration
 * phases will dispatch through.
 */

#include "ftd/backend.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"   // phase_forces_integrate_clusters (GPU cluster-inertia mirror)
#include <cstdio>
#include <cmath>
#include <cstdlib>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

// ─── CpuBackend ───────────────────────────────────────────────────────────

CpuBackend::CpuBackend(RenderBridge& bridge) : bridge_(bridge) {}

void CpuBackend::tick() {
    // Currently the CPU tick body lives in RenderBridge::tick() and is gated
    // by the `use_gpu_` flag. Phase D (next) migrates that body here. For
    // now this method is unreachable — RenderBridge::tick() does the dispatch
    // via the existing if-ladder.
    //
    // Once Phase D lands: this calls bridge_.phase_read(); phase_write();
    // gauss_project(); phase_forces(); phase_movement(); etc.
}

// ─── GpuBackend ───────────────────────────────────────────────────────────

#ifdef FTD_ENABLE_CUDA
GpuBackend::GpuBackend(RenderBridge& bridge, gpu::GpuEngine* engine)
    : bridge_(bridge), engine_(engine) {}

void GpuBackend::tick() {
    // ARCH-2-D: full GPU tick body. Replaces the if-use_gpu_ block in
    // RenderBridge::tick(). Caller (RenderBridge::tick) is responsible for
    // toggle validation, post-tick proper-time accumulation, and ledger
    // update — those are backend-agnostic.
    if (!engine_) return;

    // Wave 5.2 (2026-04-14): flush direct host writes back to device.
    flush_host_mutations();
    // Sync toggle state to the GPU engine each tick.
    engine_->toggles = bridge_.toggles;
    engine_->tick();
    bridge_.gpu_dirty_ = true;
    bridge_.physical_time_ += bridge_.dt_;
    ++bridge_.tick_;
    // Download device buffers so RenderBridge::accumulate_proper_time() and
    // update_energy_ledger() see fresh state.
    sync_to_host();

    // ── Unified-mass Phase 2: rigid-body cluster inertia (GPU mirror) ───────
    // The cluster pass has NO GPU kernel; it is a HOST-SIDE reduction by design.
    // A device-side segmented/atomic reduction would sum F_cluster and Σvelocity
    // in nondeterministic float order and break the bit-exact golden / gpu_parity
    // gates. Instead we reuse the CPU free function verbatim on the synced host
    // RenderBridge, so the GPU result is bit-exact-by-construction identical to
    // the CPU path (same host arithmetic, same traversal order).
    //
    // PLACEMENT / PARITY: the CPU path runs this between phase_forces and
    // phase_movement (render_bridge.cpp:469). Here it runs AFTER the whole GPU
    // tick (post-movement, post-download). That is observably IDENTICAL because
    // the pass only writes `velocity` on LOCKED voxels, and movement — on BOTH
    // backends — skips locked voxels entirely (CPU phase_movement_main_loop:63
    // `if (v.state==0 || v.locked ...) continue;`; GPU phase_movement_kernel:421
    // `if (state[i]==0 || locked[i]) return;`). So the locked-velocity write is
    // never consumed by movement; computing it before vs. after movement yields
    // the same final state. Running post-sync also lets us reuse the per-tick
    // download (state/velocity/locked/latency + force_diag scatter) that
    // sync_to_host() already performed for the energy ledger.
    //
    // sync_to_host() above has populated bridge_.voxels_ (state, velocity,
    // locked, latency) and bridge_.force_diag_ (f_coulomb/f_gravity/f_strong/
    // f_magnetic) — exactly the inputs phase_forces_integrate_clusters reads,
    // and bridge_.lattice_ is backend-independent. Default-OFF: when the toggle
    // is clear this whole block is skipped, so GPU runs are unchanged.
    if (bridge_.toggles.cluster_inertia) {
        // Reuse the CPU integrator verbatim — do NOT reimplement the flood-fill.
        phase_forces_integrate_clusters(bridge_);
        // Upload the rewritten host velocity back to the device so subsequent
        // ticks / diagnostics see the cluster's COM velocity. push_to_device()
        // re-uploads the full host voxel array (only locked velocities changed),
        // keeping device == host; gpu_dirty_ stays false (state is consistent).
        push_to_device();
    }
}

void GpuBackend::set_dt(double dt) {
    if (engine_) engine_->set_dt(dt);
}

void GpuBackend::sync_to_host() {
    // ARCH-2-A: download device buffers when host shadow is stale. Friend
    // access lets us touch RenderBridge's private sync state directly during
    // the migration phase.
    if (engine_ && bridge_.gpu_dirty_) {
        engine_->sync_to_host(bridge_.voxels_);
        bridge_.phi_          = engine_->phi();
        bridge_.phi_coulomb_  = engine_->phi_coulomb();
        bridge_.phi_latency_  = engine_->phi_latency();
        // Per-site force diagnostics — without this, GPU runs returned zero
        // from RenderBridge::force_diag_at() because the array was never
        // populated. The four tests test_emergent_measurements,
        // test_asymptotic_freedom, test_confinement, and test_triad_confinement
        // depend on this scatter to read f_coulomb / f_strong after tick().
        const auto& fd = engine_->force_diag();
        const int n = static_cast<int>(bridge_.force_diag_.size());
        for (int i = 0; i < n; ++i) {
            auto& d = bridge_.force_diag_[i];
            d.f_coulomb  = { fd.coulomb_x[i],  fd.coulomb_y[i],  fd.coulomb_z[i]  };
            d.f_strong   = { fd.strong_x[i],   fd.strong_y[i],   fd.strong_z[i]   };
            d.f_magnetic = { fd.magnetic_x[i], fd.magnetic_y[i], fd.magnetic_z[i] };
            d.f_gravity  = { fd.gravity_x[i],  fd.gravity_y[i],  fd.gravity_z[i]  };
            d.f_exchange = { fd.exchange_x[i], fd.exchange_y[i], fd.exchange_z[i] };
        }
        bridge_.sync_ternary_from_voxels();
        bridge_.sync_fields_from_voxels();
        bridge_.gpu_dirty_    = false;
    }
}

void GpuBackend::mark_host_dirty() {
    // ARCH-2-B: mark the host shadow as needing upload before next tick.
    bridge_.host_mutated_ = true;
}

void GpuBackend::push_to_device() {
    // ARCH-2-B: unconditional upload host→device (used by inject_*_cpu when
    // the GPU is active and the host has been edited directly).
    if (engine_) {
        engine_->upload_from_host(bridge_.voxels_);
        bridge_.gpu_dirty_ = false;
    }
}

void GpuBackend::flush_host_mutations() {
    // ARCH-2-C: conditional upload — only when host_mutated_ was set since
    // the last flush. Wave 5.2 (2026-04-14): tick() calls this at the start
    // so direct host writes via voxels()[idx].field = ... reach the device.
    // Bug-fix 2026-04-21: also called from RenderBridge::run() GPU fast-path.
    if (engine_ && bridge_.host_mutated_) {
        engine_->upload_from_host(bridge_.voxels_);
        bridge_.gpu_dirty_   = false;
        bridge_.host_mutated_ = false;
    }
}

void GpuBackend::mirror_phi_latency() {
    // ARCH-2-F: lazy fetch of d_phi_latency into RenderBridge::phi_latency_
    // so external callers (phi_latency() accessor) get a stable host ref.
    if (engine_) {
        bridge_.phi_latency_ = engine_->phi_latency();
    }
}

void GpuBackend::mark_gpu_dirty() {
    // ARCH-2-I: indicate the device has newer state than the host shadow.
    // The next access through voxels()/voxel_at() will trigger sync_to_host.
    // Used by inject_*_cpu free functions after a GPU-side inject call.
    bridge_.gpu_dirty_ = true;
}
#endif

}  // namespace ftd
