// ============================================================================
// test_render_bridge_golden.cpp
// ----------------------------------------------------------------------------
// Phase 4 PRE-FLIGHT REGRESSION GATE
//
// Before any further phase extraction is allowed out of render_bridge.cpp
// (Phase 4a/4b/4c — see plan `.claude/plans/i-want-to-try-crispy-charm.md`),
// every commit MUST reproduce the byte-hash captured here. If a commit
// changes the hash, it has changed engine physics — extraction commits
// MUST be bit-exact preserving.
//
// Pattern is the R1-R5 phase extraction precedent (poisson_solvers.cpp,
// transmutation_phases.cpp, injection.cpp; see ADR-0008).
//
// Setup (deterministic):
//   - L = 17 lattice (ODD — 2026-06-03: all lattices are odd so the flux
//     pulse at (8,8,8) lands on the true center voxel (N-1)/2 = 8)
//   - rb.force_cpu()                       (pin to CPU backend; bit-exactness)
//   - rb.seed_rng(42)                      (genesis Born-rule reproducible)
//   - 3 manifested particles at well-separated coordinates with known charges
//   - 1 flux pulse at lattice center
//   - Toggle profile (see set_toggle_profile() below): clean physics path
//
// Drive:
//   - rb.tick() x 100
//
// Hash (xor-fold of bit representations of every double we care about):
//   - voxels[*].state              (int8_t, cast to int64)
//   - voxels[*].flux               (Vec3)
//   - voxels[*].wave_vel           (Vec3)
//   - voxels[*].velocity           (Vec3)
//   - audit fields (22 doubles + 2 ints + Vec3 poynting)
//   - manifested particle state: (x, y, z, charge, vel) for each manifested site
//
// The xor-fold uses the FNV-1a 64-bit constant (0x100000001b3) as the mixer
// so each contribution is permuted before xoring — pure XOR is order-
// independent which would mask voxel-permutation bugs.
//
// Frozen golden hash:
//   GOLDEN_HASH = (computed on first run, then hardcoded — see below)
//
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Hash mixer — FNV-1a 64-bit prime. Each contribution is multiplied through
// the mixer before being xored into the running hash, so reordering inputs
// changes the hash (catches voxel-permutation regressions).
// ---------------------------------------------------------------------------
static constexpr std::uint64_t FNV_PRIME = 0x100000001b3ULL;
static constexpr std::uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;

static inline std::uint64_t mix_u64(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    h *= FNV_PRIME;
    return h;
}

static inline std::uint64_t mix_double(std::uint64_t h, double d) {
    // Bit-cast double -> uint64. NaNs collapse to a sentinel so a NaN bug
    // still gives a stable hash (failure is then visible in the audit, not
    // in the hash diff).
    if (std::isnan(d)) return mix_u64(h, 0x7ff8000000000000ULL);
    std::uint64_t u;
    std::memcpy(&u, &d, sizeof(u));
    return mix_u64(h, u);
}

static inline std::uint64_t mix_vec3(std::uint64_t h, const Vec3& v) {
    h = mix_double(h, v.x);
    h = mix_double(h, v.y);
    h = mix_double(h, v.z);
    return h;
}

static inline std::uint64_t mix_i64(std::uint64_t h, std::int64_t i) {
    return mix_u64(h, static_cast<std::uint64_t>(i));
}

// ---------------------------------------------------------------------------
// Toggle profile — exercises the cleanest physics path.
// ---------------------------------------------------------------------------
static void set_toggle_profile(RenderBridge& rb) {
    auto& t = rb.toggles;

    // ON
    t.wave_propagation  = true;
    t.coupling          = true;
    t.gauss_projection  = true;
    t.forces            = true;
    t.movement          = true;
    t.poisson_coulomb   = true;

    // OFF (damping path)
    t.damping           = false;
    t.selective_damping = false;  // must be off when damping is off (validate())
    t.larmor_radiation  = false;

    // OFF (extra force channels)
    t.gravity           = false;
    t.lorentz_force     = false;
    t.color_forces      = false;
    t.strong_force      = false;
    t.exchange_force    = false;
    t.confinement       = false;

    // OFF (substrate / extension toggles)
    t.dual_substrate    = false;
    t.weak_transmutation= false;  // requires dual_substrate when on (validate())
    t.triad_binding     = false;
    t.pair_production   = false;
    t.latency_field     = false;
    t.langevin          = false;
    t.exact_dual_gauss  = false;
    t.emergent_forces   = false;  // mutually exclusive with poisson_coulomb

    // genesis: leave default ON. Born-rule manifestation is RNG-driven; we
    // seed_rng(42) explicitly so it's reproducible.
    t.genesis           = true;
}

// ---------------------------------------------------------------------------
// Inject a deterministic initial state.
// ---------------------------------------------------------------------------
static void inject_initial_state(RenderBridge& rb) {
    // 3 manifested particles, well-separated, charges {+1, -1, +1}.
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});

    // Flux pulse at lattice centre.
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// ---------------------------------------------------------------------------
// Compute the byte-hash of the current engine state.
// ---------------------------------------------------------------------------
static std::uint64_t compute_state_hash(const RenderBridge& rb) {
    std::uint64_t h = FNV_OFFSET;

    // 1. Voxel fields — every site, in linear index order.
    const auto& voxels = rb.voxels();
    const int N = static_cast<int>(voxels.size());
    h = mix_i64(h, N);
    for (int idx = 0; idx < N; ++idx) {
        const auto& v = voxels[idx];
        h = mix_i64(h, static_cast<std::int64_t>(v.state));
        h = mix_vec3(h, v.flux);
        h = mix_vec3(h, v.wave_vel);
        h = mix_vec3(h, v.velocity);
    }

    // 2. Energy audit — 22 doubles + 2 ints + Vec3 poynting.
    auto a = rb.energy_audit();
    h = mix_double(h, a.field_energy);
    h = mix_double(h, a.wave_energy);
    h = mix_double(h, a.particle_ke);
    h = mix_double(h, a.total_energy);
    h = mix_double(h, a.gauss_violation);
    h = mix_double(h, a.max_gauss_error);
    h = mix_double(h, a.self_field_injection);
    h = mix_double(h, a.coulomb_pe);
    h = mix_double(h, a.E_field_energy);
    h = mix_double(h, a.B_field_energy);
    h = mix_i64(h, static_cast<std::int64_t>(a.charge_total));
    h = mix_i64(h, static_cast<std::int64_t>(a.manifested_count));
    h = mix_vec3(h, a.total_poynting);
    h = mix_double(h, a.E_L_total);
    h = mix_double(h, a.E_R_total);
    h = mix_double(h, a.wv_L_total);
    h = mix_double(h, a.wv_R_total);
    h = mix_double(h, a.chirality_total);
    h = mix_double(h, a.strong_energy);
    h = mix_double(h, a.weak_energy);

    // 3. Manifested-particle list — (idx, state, velocity) per manifested site.
    int n_manifested = 0;
    for (int idx = 0; idx < N; ++idx) {
        if (voxels[idx].state != 0) {
            h = mix_i64(h, idx);
            h = mix_i64(h, static_cast<std::int64_t>(voxels[idx].state));
            h = mix_vec3(h, voxels[idx].velocity);
            ++n_manifested;
        }
    }
    h = mix_i64(h, n_manifested);

    return h;
}

// ---------------------------------------------------------------------------
// FROZEN GOLDEN HASH.
//   - 2026-04-27: original capture on main @ HEAD at L=16 (0xcd957b601d47868a).
//   - 2026-06-03: RECAPTURED at L=17 — intentional config change (all lattice
//     sizes are now odd so phenomena/flux center on a true center voxel). This
//     is NOT a phase-extraction regression; the lattice changed 16→17, so the
//     byte-hash necessarily changed (different voxel count + center).
//
// If this changes WITHOUT a stated config/physics rationale, ENGINE PHYSICS
// CHANGED unexpectedly. To change it intentionally: (1) state the rationale in
// the commit, (2) update the constant below to the new captured value.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_HASH = 0xebaa6f314f66db3fULL;  // L=17 (aligned to origin/main baseline, 2026-06-10)

// ---------------------------------------------------------------------------
// Test driver
// ---------------------------------------------------------------------------
void test_golden_tick_hash() {
    section("100-tick byte-hash regression");

    RenderBridge rb(17);     // odd lattice — true center voxel at (17-1)/2 = 8
    rb.force_cpu();          // CPU-only — bit-exact reference
    rb.seed_rng(42);         // deterministic genesis Born-rule sampling
    set_toggle_profile(rb);
    inject_initial_state(rb);

    // Re-seed AFTER injection: inject_particle does not consume RNG, but
    // we want any future implementation that DOES consume RNG during
    // injection to leave the tick-loop RNG in a well-defined state.
    rb.seed_rng(42);

    // Drive 100 ticks.
    for (int t = 0; t < 100; ++t) {
        rb.tick();
    }

    const std::uint64_t hash = compute_state_hash(rb);

    // Always print the hash — useful when the test fails for diff'ing,
    // and useful when initially capturing the golden.
    std::printf("[golden] computed hash = 0x%016llx\n",
                static_cast<unsigned long long>(hash));
    std::printf("[golden] expected hash = 0x%016llx\n",
                static_cast<unsigned long long>(GOLDEN_HASH));

    check("hash matches frozen GOLDEN_HASH",
          hash == GOLDEN_HASH,
          "Engine physics has changed since the Phase 4 pre-flight golden "
          "was captured. If this is intentional (not a phase extraction), "
          "update GOLDEN_HASH and document the change.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_render_bridge_golden");
    ftd::test::test_golden_tick_hash();
    return ftd::test::finalize();
}
