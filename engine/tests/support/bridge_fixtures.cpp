// ============================================================================
// tests/support/bridge_fixtures.cpp
// ----------------------------------------------------------------------------
// Implementation of the helpers declared in bridge_fixtures.h. See that
// header for the API contract. All bodies here are thin wrappers over
// existing RenderBridge methods — no new physics.
// ============================================================================

#include "bridge_fixtures.h"

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"  // EnergyAudit (total_energy)
#include "ftd/term_toggles.h"
#include "ftd/test_telemetry.h"             // ftd::test::check
#include "ftd/voxel.h"                       // Vec3

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ftd {
namespace test {

namespace {

// Apply the requested toggle profile on top of the RenderBridge default
// TermToggles. We mutate only the specific bits the profile names — every
// other toggle keeps whatever the engine constructor seeded it with.
void apply_profile(RenderBridge& rb, ToggleProfile profile) {
    auto& t = rb.toggles;
    switch (profile) {
    case ToggleProfile::Logic6: {
        // Default RenderBridge toggles already cover the canonical Logic-6
        // (wave_propagation, coupling, damping, genesis, gauss_projection,
        // forces). Keep extension toggles at their defaults for backward
        // compatibility with tests that adopt this profile but expect the
        // typical rb.toggles state.
        break;
    }
    case ToggleProfile::LogicOnly: {
        // Stencil-only: pure Laplacian + state-flux coupling, no genesis,
        // no projection, no forces, no movement.
        t.wave_propagation  = true;
        t.coupling          = true;
        t.damping           = false;
        t.genesis           = false;
        t.gauss_projection  = false;
        t.forces            = false;
        t.gravity           = false;
        t.poisson_coulomb   = false;
        t.movement          = false;
        t.lorentz_force     = false;
        t.selective_damping = false;
        t.larmor_radiation  = false;
        t.dual_substrate    = false;
        t.color_forces      = false;
        t.weak_transmutation= false;
        t.strong_force      = false;
        t.triad_binding     = false;
        t.pair_production   = false;
        t.exchange_force    = false;
        t.latency_field     = false;
        break;
    }
    case ToggleProfile::FullEM: {
        // Logic-6 (defaults) + lorentz_force ON. lorentz_force is already
        // ON by default, so this is functionally identical to Logic6 on
        // current toggle defaults — kept as a named profile for self-
        // documenting tests.
        t.lorentz_force = true;
        break;
    }
    case ToggleProfile::FullSM: {
        t.lorentz_force      = true;
        t.color_forces       = true;
        t.strong_force       = true;
        t.dual_substrate     = true;
        break;
    }
    case ToggleProfile::Custom: {
        // Caller will mutate rb.toggles after construction; we leave
        // everything at the engine default.
        break;
    }
    }
}

}  // namespace

std::unique_ptr<RenderBridge>
make_bridge(int L, ToggleProfile profile, unsigned seed, bool force_cpu) {
    auto rb = std::make_unique<RenderBridge>(L);
    if (force_cpu) {
        rb->force_cpu();
    }
    apply_profile(*rb, profile);
    rb->seed_rng(seed);
    return rb;
}

void run_for(RenderBridge& rb, int ticks) {
    for (int i = 0; i < ticks; ++i) {
        rb.tick();
    }
}

void inject_particle_at_center(RenderBridge& rb, int8_t state, Vec3 v) {
    if (state == 0) return;  // void → no-op
    const int L = rb.lattice().size();
    const int c = L / 2;
    rb.inject_particle(c, c, c, state,
                       Vec3{static_cast<double>(state), 0.0, 0.0},
                       /*spin=*/0, /*color=*/0);
    if (v.x != 0.0 || v.y != 0.0 || v.z != 0.0) {
        rb.voxel_at(c, c, c).velocity = v;
    }
}

bool assert_energy_conserved(RenderBridge& rb, int n_ticks, double eps_rel) {
    const double eps_abs = 1e-30;  // floor so divide-by-zero is impossible
    auto initial = rb.energy_audit();
    const double E0 = initial.total_energy;
    const double denom = std::max(std::fabs(E0), eps_abs);

    double max_drift = 0.0;
    for (int i = 0; i < n_ticks; ++i) {
        rb.tick();
        auto a = rb.energy_audit();
        const double drift = std::fabs(a.total_energy - E0) / denom;
        max_drift = std::max(max_drift, drift);
    }

    const bool ok = max_drift < eps_rel;
    char detail[160];
    std::snprintf(detail, sizeof(detail),
                  "max |dE/E| = %.6g over %d ticks (eps_rel = %.6g, E0 = %.6g)",
                  max_drift, n_ticks, eps_rel, E0);
    check("energy conservation", ok, detail);
    return ok;
}

}  // namespace test
}  // namespace ftd
