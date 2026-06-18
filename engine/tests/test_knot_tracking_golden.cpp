// ============================================================================
// test_knot_tracking_golden.cpp
// ----------------------------------------------------------------------------
// Proves the knot_tracking toggle is OBSERVATION-ONLY: ticking the engine with
// toggles.knot_tracking == true produces a BIT-IDENTICAL engine state to
// ticking with it false. The only new behavior introduced by Task 4 is a gated
// `KnotTracker::record(*this)` at tick-end that READS settled state — it must
// never perturb voxels, RNG, audit, or control flow.
//
// Load-bearing invariant: hash(on) == hash(off). The same fields are hashed in
// both runs, so equality is the proof. (The canonical absolute golden value is
// pinned separately in test_render_bridge_golden.cpp = 0xb604d81a3d79366e; with
// knot_tracking default-off, THAT test is the strongest cross-check that the
// canonical hash is unchanged.)
//
// Voxel.flux is a Vec3 (three doubles): hashed component-wise (.x/.y/.z), per
// the plan's Vec3 note. We additionally fold wave_vel + velocity + state so the
// equality check covers the full per-voxel dynamical state, not flux alone.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

namespace {

// FNV-1a-style mixer (same spirit as the canonical golden test: each input is
// permuted through the prime so reordering changes the hash).
inline std::uint64_t mix_u64(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    h *= 0x100000001b3ULL;
    return h;
}
inline std::uint64_t mix_double(std::uint64_t h, double d) {
    if (std::isnan(d)) return mix_u64(h, 0x7ff8000000000000ULL);
    std::uint64_t u;
    std::memcpy(&u, &d, sizeof(u));
    return mix_u64(h, u);
}
inline std::uint64_t mix_vec3(std::uint64_t h, const ftd::Vec3& v) {
    h = mix_double(h, v.x);
    h = mix_double(h, v.y);
    h = mix_double(h, v.z);
    return h;
}

std::uint64_t hash_state(const ftd::RenderBridge& rb) {
    std::uint64_t h = 0xcbf29ce484222325ULL;  // FNV offset basis
    const auto& vox = rb.voxels();
    const std::size_t n = vox.size();
    h = mix_u64(h, static_cast<std::uint64_t>(n));
    for (std::size_t i = 0; i < n; ++i) {
        const auto& v = vox[i];
        h = mix_u64(h, static_cast<std::uint64_t>(static_cast<int>(v.state) + 2));
        h = mix_vec3(h, v.flux);       // Vec3 — hashed component-wise (.x/.y/.z)
        h = mix_vec3(h, v.wave_vel);
        h = mix_vec3(h, v.velocity);
    }
    h = mix_u64(h, static_cast<std::uint64_t>(rb.current_tick()));
    return h;
}

// Run the deterministic golden-style scenario, flipping ONLY knot_tracking.
std::uint64_t run(bool tracking) {
    ftd::RenderBridge rb(17);   // odd lattice — true center voxel at (17-1)/2 = 8
    rb.force_cpu();             // pin to CPU backend for bit-exactness
    rb.seed_rng(42);

    // Clean physics path (mirrors test_render_bridge_golden's intent: the
    // cleanest wave + Gauss + forces + movement path, damping off).
    auto& t = rb.toggles;
    t.wave_propagation  = true;
    t.coupling          = true;
    t.gauss_projection  = true;
    t.forces            = true;
    t.movement          = true;
    t.poisson_coulomb   = true;
    t.genesis           = true;
    t.damping           = false;
    t.selective_damping = false;
    t.larmor_radiation  = false;
    t.gravity           = false;
    t.lorentz_force     = false;
    t.color_forces      = false;
    t.dual_substrate    = false;
    t.weak_transmutation= false;
    t.latency_field     = false;
    t.langevin          = false;
    t.emergent_forces   = false;

    // The variable under test.
    t.knot_tracking     = tracking;

    // Deterministic initial state: 3 manifested particles + 1 flux pulse.
    rb.inject_particle( 3,  3,  3, +1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, ftd::Vec3{1.0, 0.0, 0.0});

    rb.seed_rng(42);            // re-seed after injection (genesis reproducibility)

    for (int tk = 0; tk < 60; ++tk) rb.tick();
    return hash_state(rb);
}

}  // namespace

int main() {
    ftd::test::init("test_knot_tracking_golden");

    const std::uint64_t off = run(false);
    const std::uint64_t on  = run(true);

    std::printf("[knot-golden] off=0x%016llx on=0x%016llx\n",
                static_cast<unsigned long long>(off),
                static_cast<unsigned long long>(on));

    ftd::test::check(
        "knot_tracking is observation-only (state hash unchanged on==off)",
        off == on,
        "knot_tracking changed engine state — it must be observation-only "
        "(record() may only READ settled voxels()/lattice()/current_tick()).");

    return ftd::test::finalize();
}
