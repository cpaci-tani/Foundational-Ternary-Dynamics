#include "ftd/dag_engine.h"
#include <iostream>
#include <cassert>

// ══════════════════════════════════════════════════════════════════════
// STATUS BANNER — DAG Engine is a DEPRECATED SKELETON (ticket W6),
//                 NOT the production physics path.
// ══════════════════════════════════════════════════════════════════════
//
// The file name advertises a DAG-based six-phase engine. In reality:
//
//   phase_read    — implemented (Laplacian + coupling source)
//   phase_write   — implemented (leapfrog-style update + damping)
//   gauss_project — UNIMPLEMENTED STUB — [[deprecated]], asserts false
//   phase_forces  — UNIMPLEMENTED STUB — [[deprecated]], asserts false
//   phase_movement— UNIMPLEMENTED STUB — [[deprecated]], asserts false
//
// As of W6 (deprecate-clearly) the three stubs are no longer silent
// no-ops. tick() emits a one-time runtime warning and SKIPS them (it is
// wave-only), and each stub body asserts false so that any DIRECT call
// surfaces loudly in a debug build. This converts a "looks implemented
// but isn't" hazard into a loud, explicit incompleteness signal without
// breaking anything that currently compiles or links — see test_dag_engine
// (release build) which exercises only phase_read/phase_write via tick().
//
// The GOLDEN / production Scale-0 physics runs through src/render_bridge.cpp
// on a FLAT voxel array (golden hash 0xcd957b601d47868a). DagEngine is NOT
// in that tick path and has no WASM/JS binding. The SparseVoxelDAG here is
// in-progress infrastructure for a future migration, not a path the browser
// engine, tests, or benchmarks currently exercise.
//
// Do NOT cite results produced by this class as representative of FTD
// engine output. When this class is completed, delete this banner, the
// [[deprecated]] markers, the tick() warning, and the stub asserts.
//
// ══════════════════════════════════════════════════════════════════════
// INTEGRATION SCHEME NOTE
// ══════════════════════════════════════════════════════════════════════
// phase_read + phase_write below perform:
//
//     wave_vel += delta_J     (from Laplacian)
//     flux += wave_vel        (position-like update)
//
// This IS Störmer–Verlet leapfrog under the stagger interpretation where
// wave_vel = v(t + h/2) and flux = J(t). Verified empirically in
// RenderBridge by tests/test_leapfrog_integrator_audit.cpp (TRACKER §1.4
// closed 2026-04-17): 0.1% cumulative injection/dissipation balance over
// 5000 ticks with damping off. C_SPEED = 1/√D = 1/√3 is the correct
// leapfrog CFL limit.
//
// ══════════════════════════════════════════════════════════════════════
// LAPLACIAN ISOTROPY NOTE (verified 2026-04-17, TRACKER §1.8 CLOSED)
// ══════════════════════════════════════════════════════════════════════
// The 18-point Moore Laplacian below (face=1/3, edge=1/6, self=−4) is
// consistent (weights sum to 0) AND rotationally isotropic through
// O(h⁴). Taylor expansion gives:
//
//     ∇²_Moore f = ∇²f + (h²/12)(∇²)²f + O(h⁶)
//
// The 2:1 face:edge ratio is chosen precisely to cancel the anisotropic
// part of the O(h⁴) term. Residual anisotropy at finite h is lattice
// dispersion at k·h ~ 1 — a universal artefact of cubic-lattice FD
// schemes, quantified in tests/test_moore_laplacian_isotropy.cpp.
// ══════════════════════════════════════════════════════════════════════

namespace ftd {

DagEngine::DagEngine(int lattice_size) {
    dag_ = std::make_unique<SparseVoxelDAG>(lattice_size);
    
    // Core engine rules defaulting to ON
    toggles_.enable_all();
    
    // Disable extensions
    toggles_.larmor_radiation = false;
    toggles_.latency_field = false;
    toggles_.color_forces = false;
    toggles_.strong_force = false;
    toggles_.triad_binding = false;
    toggles_.pair_production = false;
    toggles_.exchange_force = false;

    // Buffer allocations (flat for now until we build a sparse delta list mapper)
    int total = lattice_size * lattice_size * lattice_size;
    delta_j_.resize(total, {0.0, 0.0, 0.0});
}

void DagEngine::clear() {
    int sz = dag_->size();
    dag_ = std::make_unique<SparseVoxelDAG>(sz);
    tick_ = 0;
}

void DagEngine::tick() {
    // ── W6 (deprecate-clearly) ────────────────────────────────────────────
    // DagEngine::tick() is WAVE-ONLY. Phases 1-2 (read/write) are real and
    // run. Phases 3-5 (gauss_project / phase_forces / phase_movement) are
    // UNIMPLEMENTED stubs; they used to be invoked here behind their toggles
    // and silently no-op'd, which made an incomplete engine look complete.
    //
    // They are now NOT invoked. Instead we emit a one-time loud runtime
    // warning so anyone who ticks this engine learns it is incomplete, and
    // we skip the stubs (each stub also asserts false if called directly).
    // This keeps the only live caller (test_dag_engine, release build, which
    // checks phase_read/phase_write wave propagation) passing while making
    // the incompleteness explicit. For real Gauss/force/movement physics use
    // RenderBridge (the golden production path).

    // 1. Compute delta_J (Laplacian wave equation, curl coupling source)
    if (toggles_.wave_propagation || toggles_.coupling) {
        phase_read();
    }

    // 2. Leapfrog wave update + damping (Störmer–Verlet)
    phase_write();

    // 3-5. UNIMPLEMENTED — gauss_project / phase_forces / phase_movement.
    //      Warn once and skip. Do NOT silently no-op, do NOT throw from the
    //      hot tick path (test_dag_engine ticks and expects no crash).
    if (toggles_.gauss_projection || toggles_.forces || toggles_.movement) {
        static bool warned = false;
        if (!warned) {
            warned = true;
            std::cerr
                << "[DagEngine][W6] WARNING: gauss_project / phase_forces / "
                   "phase_movement are UNIMPLEMENTED in this experimental, "
                   "deprecated engine. tick() is wave-only (read+write); no "
                   "charge projection, forces, or movement are applied. "
                   "Use RenderBridge for the production (golden) physics path.\n";
        }
    }

    tick_++;
}

// -----------------------------------------------------------------------------
// Discrete Physics Operators
// -----------------------------------------------------------------------------

Vec3 DagEngine::laplacian_flux(int x, int y, int z) const {
    Vec3 lap;
    // 6 faces (1/3 weight)
    lap += dag_->get_voxel(x+1, y, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x-1, y, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y+1, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y-1, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y, z+1).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y, z-1).flux * (1.0/3.0);

    // 12 edges (1/6 weight)
    lap += dag_->get_voxel(x+1, y+1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y-1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y+1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y-1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y+1, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y+1, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y-1, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y-1, z-1).flux * (1.0/6.0);

    lap -= dag_->get_voxel(x, y, z).flux * 4.0;
    return lap;
}

Vec3 DagEngine::gradient_state(int x, int y, int z) const {
    Vec3 grad;
    grad.x = (dag_->get_voxel(x+1, y, z).state - dag_->get_voxel(x-1, y, z).state) * 0.5;
    grad.y = (dag_->get_voxel(x, y+1, z).state - dag_->get_voxel(x, y-1, z).state) * 0.5;
    grad.z = (dag_->get_voxel(x, y, z+1).state - dag_->get_voxel(x, y, z-1).state) * 0.5;
    return grad;
}

Vec3 DagEngine::curl_state_velocity(int x, int y, int z) const {
    auto jcur = [&](int px, int py, int pz) -> Vec3 {
        auto v = dag_->get_voxel(px, py, pz);
        return v.velocity * static_cast<double>(v.state);
    };
    Vec3 curl;
    curl.x = (jcur(x, y + 1, z).z - jcur(x, y - 1, z).z) * 0.5 -
             (jcur(x, y, z + 1).y - jcur(x, y, z - 1).y) * 0.5;
    curl.y = (jcur(x, y, z + 1).x - jcur(x, y, z - 1).x) * 0.5 -
             (jcur(x + 1, y, z).z - jcur(x - 1, y, z).z) * 0.5;
    curl.z = (jcur(x + 1, y, z).y - jcur(x - 1, y, z).y) * 0.5 -
             (jcur(x, y + 1, z).x - jcur(x, y - 1, z).x) * 0.5;
    return curl;
}

// -----------------------------------------------------------------------------
// Recursive Tree Traversals
// -----------------------------------------------------------------------------

void DagEngine::recursive_read(int x, int y, int z, int current_size) {
    // Structural leaf mapping reached
    if (current_size == 1) {
        // Check local activity bounds (skip deep pure void optimizations here later)
        int idx = z*(dag_->size()*dag_->size()) + y*dag_->size() + x;
        delta_j_[idx] = {};
        
        if (toggles_.wave_propagation) {
            delta_j_[idx] = laplacian_flux(x, y, z) * (C_WAVE * C_WAVE);
        }
        if (toggles_.coupling) {
            delta_j_[idx] += gradient_state(x, y, z) * G_C;
            delta_j_[idx] += curl_state_velocity(x, y, z) * G_C;
        }
        return;
    }

    // Recurse heavily into 8 Octants 
    int half = current_size >> 1;
    recursive_read(x,          y,          z,          half);
    recursive_read(x,          y,          z + half,   half);
    recursive_read(x,          y + half,   z,          half);
    recursive_read(x,          y + half,   z + half,   half);
    recursive_read(x + half,   y,          z,          half);
    recursive_read(x + half,   y,          z + half,   half);
    recursive_read(x + half,   y + half,   z,          half);
    recursive_read(x + half,   y + half,   z + half,   half);
}

void DagEngine::recursive_write(int x, int y, int z, int current_size) {
    if (current_size == 1) {
        int idx = z*(dag_->size()*dag_->size()) + y*dag_->size() + x;
        Voxel v = dag_->get_voxel(x, y, z);
        Vec3 dj = delta_j_[idx];
        
        bool needs_update = (dj.mag2() > 0.0 || v.wave_vel.mag2() > 0.0 || v.flux.mag2() > 0.0 || v.state != 0);
        
        if (needs_update) {
            v.wave_vel += dj;
            v.flux += v.wave_vel;
            
            if (toggles_.damping) {
                double eff_damping = 1.0 - DAMPING;
                v.flux *= eff_damping;
                v.wave_vel *= eff_damping;
            }
            
            // Dynamic allocation through structural COW
            dag_->set_voxel(x, y, z, v);
        }
        return;
    }
    
    // Recurse into 8 Octants
    int half = current_size >> 1;
    recursive_write(x,          y,          z,          half);
    recursive_write(x,          y,          z + half,   half);
    recursive_write(x,          y + half,   z,          half);
    recursive_write(x,          y + half,   z + half,   half);
    recursive_write(x + half,   y,          z,          half);
    recursive_write(x + half,   y,          z + half,   half);
    recursive_write(x + half,   y + half,   z,          half);
    recursive_write(x + half,   y + half,   z + half,   half);
}

// -----------------------------------------------------------------------------
// Core Engine Phases
// -----------------------------------------------------------------------------

void DagEngine::phase_read() {
    recursive_read(0, 0, 0, dag_->size());
}

void DagEngine::phase_write() {
    recursive_write(0, 0, 0, dag_->size());
}

// ── UNIMPLEMENTED STUBS (W6 deprecate-clearly) ─────────────────────────────
// These three phases are NOT implemented. They are no longer called from
// tick() (which warns + skips). If invoked DIRECTLY they assert false — loud
// in a debug build, consistent with the engine's assert-based "this must not
// happen" convention (see field_operators.h). Under NDEBUG/Release the assert
// compiles out, so nothing that currently links is broken; the loud signal is
// the runtime warning in tick() plus the compile-time [[deprecated]] markers
// on the declarations. Implement on SparseVoxelDAG traversal (mirroring
// RenderBridge) to upgrade to production, then remove these asserts + markers.

void DagEngine::gauss_project() {
    // [OPEN] Implement recursive SOR solver skipping active manifested indices.
    assert(false && "DagEngine::gauss_project is not implemented (W6 "
                    "deprecate-clearly). Use RenderBridge::gauss_project.");
}

void DagEngine::phase_forces() {
    // [OPEN] Implement recursive Poisson and Lorentz force summations.
    assert(false && "DagEngine::phase_forces is not implemented (W6 "
                    "deprecate-clearly). Use RenderBridge::phase_forces.");
}

void DagEngine::phase_movement() {
    // [OPEN] Integrate fractional remainder accumulation handling for precise tracking.
    assert(false && "DagEngine::phase_movement is not implemented (W6 "
                    "deprecate-clearly). Use RenderBridge::phase_movement.");
}

// -----------------------------------------------------------------------------
// Test APIs & Infrastructure Implementations
// -----------------------------------------------------------------------------

void DagEngine::inject_flux(int x, int y, int z, double fx, double fy, double fz) {
    Voxel v = dag_->get_voxel(x, y, z);
    v.flux.x += fx;
    v.flux.y += fy;
    v.flux.z += fz;
    dag_->set_voxel(x, y, z, v);
}

bool DagEngine::get_toggle(const std::string& name) const { return false; }
void DagEngine::set_toggle(const std::string& name, bool value) {}
ScaleBaseDiagnostics DagEngine::base_diagnostics() const { return {}; }

} // namespace ftd
