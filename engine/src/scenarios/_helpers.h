#pragma once
// ==========================================================================
//  engine/src/scenarios/_helpers.h
//
//  Private (non-installed) helper header shared by the split scenario
//  group files (flux.cpp, light.cpp, quantum.cpp, s0_seed.cpp, s0_field.cpp).
//
//  All symbols live in an anonymous namespace so each translation unit gets
//  its own internal-linkage copy — this keeps the split ABI-clean and lets
//  each .cpp evolve independently without touching a public header.
//
//  Origin: these helpers previously lived in engine/src/scenarios.cpp.
//  They were extracted verbatim as part of ticket S1 (scenarios.cpp split).
//
//  The shared RNG (urand / reset_rng / SCN_RNG_SEED) is DEFINED in
//  engine/src/scenarios.cpp next to dispatch_scenario() (which resets it
//  before each run) but DECLARED here in ftd::detail so the stochastic
//  scenarios (flux-random-genesis, flux-thermalization, flux-vacuum-foam,
//  flux-zero-point, quantum-born-rule, quantum-casimir) can call urand()
//  across TU boundaries.
// ==========================================================================

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <cmath>

namespace ftd {

// ── Shared stochastic RNG (defined in scenarios.cpp) ───────────────
// External linkage; lives in ftd::detail so it doesn't pollute the
// public ftd namespace while still being reachable from sibling TUs.
namespace detail {
double urand();
void   reset_scenario_rng();
}  // namespace detail

namespace {

// ── Injection helpers (match JS argument order) ────────────────────
inline void IF(RenderBridge& rb, int x, int y, int z, double fx, double fy, double fz) {
    // Scenario construction is one host-staged batch. Runtime injection uses
    // direct CUDA kernels, but routing this helper through that path would make
    // an IF()/IP()/LOCK() loop alternate device writes with full host syncs.
    // Keep every scenario primitive on the same canonical host shadow and let
    // the backend perform one lazy upload after construction.
    const Vec3 value(fx, fy, fz);
    Voxel& v = rb.voxel_at(x, y, z);
    v.flux += value;
    if (rb.toggles.dual_substrate) {
        const Vec3 half = value * 0.5;
        v.flux_L += half;
        v.flux_R += half;
    }
}
inline void IW(RenderBridge& rb, int x, int y, int z, double wx, double wy, double wz) {
    const Vec3 value(wx, wy, wz);
    Voxel& v = rb.voxel_at(x, y, z);
    v.wave_vel += value;
    if (rb.toggles.dual_substrate) {
        const Vec3 half = value * 0.5;
        v.wave_vel_L += half;
        v.wave_vel_R += half;
    }
}
inline void IP(RenderBridge& rb, int x, int y, int z, int state) {
    // Do not call the ordinary GPU-aware inject_particle() path for each
    // marker: a common IP()+LOCK()
    // pair would otherwise inject on the device, synchronize the entire
    // lattice back to the host for LOCK(), then upload it again before the
    // next marker.  Marker sheets contain O(L^2) sites, making that accidental
    // transfer loop dominate setup at native CUDA lattice sizes.
    //
    // Stage the same zero-flux particle record in the canonical host shadow.
    // voxel_at() marks that shadow dirty; the backend performs one lazy upload
    // when setup completes and the first visual read or tick is requested.
    Voxel& v = rb.voxel_at(x, y, z);
    v.state = static_cast<int8_t>(state);
    v.flux = Vec3(0, 0, 0);
    v.spin = 0;
    v.color = 0;
    v.particle_id = rb.injector().next_particle_id();
    v.pair_id = -1;
    if (rb.toggles.dual_substrate) {
        v.flux_L = Vec3(0, 0, 0);
        v.flux_R = Vec3(0, 0, 0);
    }
}
inline void IPF(RenderBridge& rb, int x, int y, int z, int state, int spin, int color) {
    Voxel& v = rb.voxel_at(x, y, z);
    v.state = static_cast<int8_t>(state);
    v.flux = Vec3(0, 0, 0);
    v.spin = static_cast<int8_t>(spin);
    v.color = static_cast<int8_t>(color);
    v.particle_id = rb.injector().next_particle_id();
    v.pair_id = -1;
    if (rb.toggles.dual_substrate) {
        v.flux_L = Vec3(0, 0, 0);
        v.flux_R = Vec3(0, 0, 0);
    }
}

// Mutate a just-injected particle at (x,y,z).
inline void SET_VEL(RenderBridge& rb, int x, int y, int z, double vx, double vy, double vz) {
    rb.voxel_at(x, y, z).velocity = Vec3(vx, vy, vz);
}
inline void LOCK(RenderBridge& rb, int x, int y, int z) {
    rb.voxel_at(x, y, z).locked = true;
}
inline void SET_SPIN(RenderBridge& rb, int x, int y, int z, int spin) {
    rb.voxel_at(x, y, z).spin = static_cast<int8_t>(spin);
}

// ── Math shims to keep the ported JS readable ──────────────────────
inline int    FLR(double d) { return static_cast<int>(std::floor(d)); }
inline int    CEL(double d) { return static_cast<int>(std::ceil(d)); }
inline int    RND(double d) { return static_cast<int>(std::round(d)); }

// Seed a finite, divergence-free transverse packet traveling along the x axis.
//
// The packet is built as the discrete curl of an x-directed scalar potential,
//
//     J = curl(psi e_x) = (0, D_z psi, -D_y psi),
//
// so the centered-difference divergence vanishes identically.  Its canonical
// momentum is then initialized with the one-way wave relation
//
//     wave_vel = -direction * C_SPEED * D_x J.
//
// This is the shared construction for every scenario that claims a localized
// traveling neutral packet.  Keeping it here prevents the historical J_z/W_x
// component mismatch from reappearing in individual scenario bodies.
inline void inject_transverse_packet_x(RenderBridge& rb,
                                        double x0, double y0, double z0,
                                        double sigma_x, double sigma_t,
                                        double amp, int direction = +1,
                                        double carrier_k = 0.0,
                                        double carrier_phase = 0.0) {
    const int N = rb.lattice().size();
    const double sx = std::max(1.0, sigma_x);
    const double st = std::max(1.0, sigma_t);
    const double psi_amp = amp * st;

    auto periodic_delta = [N](double a, double b) {
        double d = a - b;
        while (d >  0.5 * N) d -= N;
        while (d < -0.5 * N) d += N;
        return d;
    };

    auto psi = [&](double x, double y, double z) {
        const double dx = periodic_delta(x, x0);
        const double dy = periodic_delta(y, y0);
        const double dz = periodic_delta(z, z0);
        const double r2 = dx*dx/(sx*sx) + (dy*dy + dz*dz)/(st*st);
        if (r2 > 18.0) return 0.0;
        return psi_amp * std::exp(-0.5 * r2)
             * std::cos(carrier_k * dx + carrier_phase);
    };
    auto field = [&](double x, double y, double z) {
        const double jy =  0.5 * (psi(x, y, z + 1.0) - psi(x, y, z - 1.0));
        const double jz = -0.5 * (psi(x, y + 1.0, z) - psi(x, y - 1.0, z));
        return Vec3(0.0, jy, jz);
    };

    const double sign = (direction >= 0) ? 1.0 : -1.0;

    // Evaluate the complete torus. Clipping a broad potential at a face while
    // the lattice derivative wraps would break div(curl)=0 at that face.
    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        const Vec3 j = field(x, y, z);
        const Vec3 jp = field(x + 1.0, y, z);
        const Vec3 jm = field(x - 1.0, y, z);
        Vec3 face_sum;
        Vec3 edge_sum;
        const int face[6][3] = {
            {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
        };
        const int edge[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (const auto& o : face) face_sum += field(x + o[0], y + o[1], z + o[2]);
        for (const auto& o : edge) edge_sum += field(x + o[0], y + o[1], z + o[2]);
        const Vec3 lap = face_sum * (1.0 / 3.0)
                       + edge_sum * (1.0 / 6.0) - j * 4.0;
        // phase_read kicks W before phase_write drifts J.  The -c D_x term
        // supplies direction; -c^2 Lap(J)/2 places W on the kick-drift time
        // phase and suppresses the spurious counter-propagating branch.
        const Vec3 w = (jp - jm) * (-0.5 * sign * C_SPEED)
                     - lap * (0.5 * C_SPEED * C_SPEED);
        if (j.mag2() > 1e-20) IF(rb, x, y, z, j.x, j.y, j.z);
        if (w.mag2() > 1e-20) IW(rb, x, y, z, w.x, w.y, w.z);
    }
}

// Canonical isolated linear-wave profile. Native RenderBridge defaults enable
// several matter and interaction phases, so setting only wave/coupling/damping
// is not sufficient: a scenario would otherwise inherit gravity, Lorentz,
// weak, or dual-substrate dynamics depending on its caller.
inline void configure_free_wave_terms(RenderBridge& rb, bool gauss = true) {
    auto& t = rb.toggles;
    // Start from the complete registry OFF state. TermToggles::disable_all()
    // intentionally preserves non-bulk research controls, so it is insufficient
    // for an initial condition that promises an isolated wave map.
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.wave_propagation = true;
    t.gauss_projection = gauss;
}

// Canonical one-tick genesis-gate profile. The scenario probes the selected
// local hazard law, not the wave, force, motion, or auxiliary research terms.
// Reset typed campaign overrides as well as boolean toggles so a reused bridge
// cannot silently move either threshold or the probability scale.
inline void configure_genesis_gate_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.genesis = true;
    t.dual_substrate = false;
    t.langevin_seed = 1;
    t.kinetic_drain = 0.5;
    rb.genesis_threshold_override = -1.0;
    rb.manifest_scale_override = -1.0;
    rb.manifest_use_temperature = false;
}

// Canonical one-tick probe of the separate polarity-pair rule. Genesis is
// deliberately OFF: pair_production_cpu() is its own selected transition and
// must not be confused with the single-site genesis/evaporation phase.
inline void configure_pair_production_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.pair_production = true;
    t.dual_substrate = false;
    t.langevin_seed = 1;
    rb.genesis_threshold_override = -1.0;
    rb.manifest_scale_override = -1.0;
    rb.manifest_use_temperature = false;
}

// Canonical free transport profile. It exercises only the integer/remainder
// movement rule; fields remain frozen and no force, reaction, or transmutation
// phase is allowed to alter the seeded trajectories.
inline void configure_free_movement_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.movement = true;
    t.dual_substrate = false;
}

// Canonical collision-removal probe. The production movement phase is the
// only active bulk term. In particular wave propagation is OFF so the test
// cannot mistake redistribution of pre-existing flux for emitted radiation.
inline void configure_annihilation_terms(RenderBridge& rb) {
    configure_free_movement_terms(rb);
}

// Selected unlocked color-candidate profile.  Static dressing fields feed the
// legacy div(J) force and color labels feed the pairwise color force; only
// those forces and the production movement phase can alter the constituents.
// This is a candidate-stability experiment, not a QCD or hadron model.
inline void configure_unlocked_composite_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.forces = true;
    t.movement = true;
    t.color_forces = true;
    t.dual_substrate = false;
}

// Prepared electrostatic candidate: locked nuclear markers source the selected
// Poisson-Coulomb force while unlocked outer markers move. No wave, gravity,
// color, reaction, or damping phase can masquerade as atomic binding.
inline void configure_prepared_coulomb_candidate_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    // Substrate-mediated Coulomb (not the instantaneous Poisson shortcut): the
    // flux field J is the physical carrier of the interaction, so it must be the
    // field that both follows the charges AND moves them — otherwise a moving
    // charge drags a field that is frozen (visibly detached, non-physical).
    //   wave_propagation : J is dynamical (evolves each tick).
    //   coupling         : a charge sources J (-g_c·∇s), so J is emitted at the
    //                      charge's CURRENT position every tick.
    //   gauss_projection : enforces div(J) = s (FTD's Gauss law), re-solving each
    //                      tick, so J's monopole structure tracks the charges as
    //                      they move — the field follows its source.
    //   forces (poisson_coulomb OFF): the Coulomb force is taken from the flux
    //                      gradient, so the field you SEE is the field that acts.
    //   damping          : dissipates the radiated wake so it fades instead of
    //                      accumulating (a moving charge radiates; the field
    //                      trails then decays, as a retarded field should).
    t.wave_propagation = true;
    t.coupling = true;
    t.gauss_projection = true;
    t.damping = true;
    t.forces = true;
    t.movement = true;
    t.dual_substrate = false;
}

// Uniform positive-background drive plus the selected genesis stack. The
// drive increment is nonnegative at every tick; this can probe a driven
// threshold response but cannot execute a down-sweep or hysteresis loop.
inline void configure_uniform_genesis_drive_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.genesis = true;
    t.ew_background_sweep = true;
    t.dual_substrate = false;
    rb.genesis_threshold_override = -1.0;
    rb.manifest_scale_override = -1.0;
    rb.manifest_use_temperature = false;
}

// Prepared weak-transmutation cohort. All products are initial data; only the
// selected stress-triggered polarity-flip rule can change a state. This profile
// can test that rule, but it cannot by itself create a beta-decay final state.
//
// B1 (2026-07-27 pre-commit audit): this is the ONLY configure_*_terms in the
// file that turns on dual_substrate while leaving damping off. Every sibling
// profile either damps or stays single-substrate. The neutrino packet below
// seeds a nonzero wave_vel; with wave_propagation false, delta_j_L/R_ is never
// written (phase_read.cpp), so the dual-substrate leapfrog in phase_write.cpp
// integrates that seeded velocity as an UNDAMPED, UNBOUNDED ballistic drift:
// |wave_vel| stays exactly constant forever (measured: 0.502574, bit-identical
// at every probed tick from 8 to 300) while |flux| grows linearly and |flux|^2
// grows quadratically with no ceiling (measured: 145 -> 45534 from t=16 to
// t=300, no saturation). Because weak_transmutation_cpu gates its probability
// on stress = |flux_L| exceeding WEAK_THRESHOLD via p = 1-exp(-(stress-W)/K),
// this unbounded growth does not merely look wrong on a chart -- it drives p
// toward 1, and a manifested site re-triggers every tick once stress is far
// past threshold (measured: 383 events by t=300, vs. the 7 events the previous
// 64-tick-only test window observed; final signed_state oscillates instead of
// settling, -2 at t=100, -4 at t=200 and t=300). The 64-tick window the
// original test asserted exact values against was not a stable endpoint, only
// where the test stopped clocking. Damping restores the "prepared, bounded
// stress probe" reading consistent with every other profile in this file.
inline void configure_weak_transmutation_probe_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.dual_substrate = true;
    t.weak_transmutation = true;
    t.damping = true;
    t.langevin_seed = 1;
}

// Fixed-seed transport through a selected Langevin wave bath. Marker color
// labels are observers only because forces/color forces are deliberately off.
inline void configure_thermal_transport_terms(RenderBridge& rb,
                                              double temperature,
                                              double gamma) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.movement = true;
    t.langevin = true;
    t.langevin_T = temperature;
    t.langevin_gamma = gamma;
    t.langevin_seed = 1;
    t.dual_substrate = false;
}

// Patterned selected genesis-response profile. It combines the production
// wave/coupling/damping/Gauss stack, legacy field-gradient force, movement,
// and genesis. No biochemical, replication, or information-bearing operator
// exists; the profile measures only state/field response to prepared geometry.
inline void configure_patterned_genesis_response_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.wave_propagation = true;
    t.coupling = true;
    t.damping = true;
    t.genesis = true;
    t.gauss_projection = true;
    t.forces = true;
    t.movement = true;
    t.dual_substrate = false;
    rb.genesis_threshold_override = -1.0;
    rb.manifest_scale_override = -1.0;
    rb.manifest_use_temperature = false;
}

// Canonical inert initial-data profile. Geometry scenarios assert only the
// exact ternary arrangement they seed; no production phase is allowed to
// move, transmute, evaporate, or dress that arrangement during inspection.
inline void configure_static_seed_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.dual_substrate = false;
}

// Canonical selected genesis-response profile used by the N(A) cohort.
// Starting from the complete OFF state is essential: the measurement is the
// response of wave + Gauss + genesis (+ optional Langevin bath), not a mixture
// with gravity, motion, forces, pair production, or transmutation.
inline void configure_genesis_cluster_terms(RenderBridge& rb,
                                            double temperature,
                                            double gamma = 0.02) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.genesis = true;
    t.langevin = true;
    t.langevin_T = temperature;
    t.langevin_gamma = gamma;
    t.langevin_seed = 1;
    t.dual_substrate = false;
    rb.genesis_threshold_override = -1.0;
    rb.manifest_scale_override = -1.0;
    rb.manifest_use_temperature = false;
}

// Canonical locked-rest-mass latency probe. The body is a selected static
// source for the native Poisson latency solver. It does not claim Newtonian
// motion, field-energy gravity, genesis, or any other inherited phase.
inline void configure_mass_latency_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.gravity = true;  // declared dependency of latency_field; forces stays off
    t.latency_field = true;
    t.field_energy_gravity = false;
    t.dual_substrate = false;
}

// Locked matter marker plus the selected linear source-coupled wave sector.
// Forces and movement stay off, so this profile is a field-superposition null
// test and must never be described as mechanical recoil or scattering.
inline void configure_locked_coupled_field_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.wave_propagation = true;
    t.coupling = true;
    t.dual_substrate = false;
}

// Native mobile-source response profile.  This is deliberately narrower than
// the ordinary dashboard defaults: the field follows the linear wave/coupling
// map, while the manifested site can respond only through the selected native
// flux-gradient force and the production movement rule.  Imported qE/Lorentz,
// Poisson, reaction, gravity, damping, and auxiliary gauge terms remain OFF.
inline void configure_emergent_recoil_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.bcc_stencil = BccStencilMode::FULL;
    rb.set_dt(1.0);
    t.wave_propagation = true;
    t.coupling = true;
    t.forces = true;
    t.movement = true;
    t.emergent_forces = true;
    t.dual_substrate = false;
}

// Isolated imposed-B magnetic-response profile. J is held fixed and the only
// force allowed to act is the native velocity-dependent Lorentz term; the
// legacy electric contribution vanishes for the divergence-free linear vector
// potential used by the scenario.
inline void configure_lorentz_orbit_terms(RenderBridge& rb) {
    auto& t = rb.toggles;
    for (const auto& spec : TOGGLE_SPECS) t.*(spec.field) = false;
    t.flux_boundary = FluxBoundaryMode::Periodic;
    t.bcc_stencil = BccStencilMode::FULL;
    rb.set_dt(1.0);
    t.forces = true;
    t.poisson_coulomb = true;
    t.movement = true;
    t.lorentz_force = true;
    t.dual_substrate = false;
}

// Exact pole of the production kick-drift wave map for a yz-uniform harmonic
// with wave number k along x.  The 18-point Moore Laplacian reduces exactly to
// the one-dimensional second difference on this subspace:
//
//     Lap J = -4 sin^2(k/2) J,
//     sin(omega/2) = C_SPEED sin(k/2).
//
// This is deliberately not the small-k approximation 2c sin(k/2).
inline double lattice_harmonic_omega(double k) {
    return 2.0 * std::asin(C_SPEED * std::fabs(std::sin(0.5 * k)));
}

// Exact +x/-x eigenmode of the production kick-then-drift map.  At integer
// tick t the initialized field evolves as
//
//     J_z(x,t) = amp sin(kx - direction*omega*t).
//
// wave_vel is stored before the next kick, which accounts for the
// (1-cos(omega))*sin(kx) stagger term.
inline void inject_plane_harmonic_x(RenderBridge& rb, int mode_n, double amp,
                                    int direction = +1) {
    const int N = rb.lattice().size();
    const double k = 2.0 * PI * static_cast<double>(mode_n)
                   / static_cast<double>(N);
    const double omega = lattice_harmonic_omega(k);
    const double sign = direction >= 0 ? 1.0 : -1.0;
    for (int x = 0; x < N; ++x) {
        const double phase = k * x;
        const double jz = amp * std::sin(phase);
        const double wz = amp * ((1.0 - std::cos(omega)) * std::sin(phase)
                               - sign * std::sin(omega) * std::cos(phase));
        for (int y = 0; y < N; ++y)
        for (int z = 0; z < N; ++z) {
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0.0, 0.0, jz);
            if (std::fabs(wz) > 1e-12) IW(rb, x, y, z, 0.0, 0.0, wz);
        }
    }
}

// Exact standing eigenmode on the same kick-drift time staggering:
//
//     J_z(x,t) = amp sin(kx) cos(omega*t).
//
// W=0 would be a visually plausible but temporally mis-phased seed.  The
// stored pre-kick velocity required by this exact integer-tick solution is
// W=(1-cos(omega))*J.
inline void inject_standing_harmonic_x(RenderBridge& rb, int mode_n, double amp) {
    const int N = rb.lattice().size();
    const double k = 2.0 * PI * static_cast<double>(mode_n)
                   / static_cast<double>(N);
    const double omega = lattice_harmonic_omega(k);
    for (int x = 0; x < N; ++x) {
        const double jz = amp * std::sin(k * x);
        const double wz = (1.0 - std::cos(omega)) * jz;
        for (int y = 0; y < N; ++y)
        for (int z = 0; z < N; ++z) {
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0.0, 0.0, jz);
            if (std::fabs(wz) > 1e-12) IW(rb, x, y, z, 0.0, 0.0, wz);
        }
    }
}

// A divergence-free sheet packet: J_z varies in x/y and is uniform in z, so
// D_z J_z = 0.  This is useful for two-slit and side-by-side race scenarios,
// where a finite y profile is needed but a fully localized vector packet would
// obscure the comparison with transverse diffraction.
inline void inject_sheet_packet_x(RenderBridge& rb,
                                  double x0, double transverse0,
                                  double sigma_x, double sigma_t,
                                  double amp, int direction = +1,
                                  int polarization_axis = 2,
                                  double carrier_k = 0.0,
                                  double carrier_phase = 0.0) {
    const int N = rb.lattice().size();
    const double sx = std::max(1.0, sigma_x);
    const double st = std::max(1.0, sigma_t);
    auto scalar = [&](double x, double u) {
        const double dx = x - x0;
        const double du = u - transverse0;
        const double r2 = dx*dx/(sx*sx) + du*du/(st*st);
        return (r2 <= 18.0)
            ? amp * std::exp(-0.5 * r2) * std::cos(carrier_k * dx + carrier_phase)
            : 0.0;
    };
    const int xlo = std::max(0, FLR(x0 - 4.5 * sx));
    const int xhi = std::min(N - 1, CEL(x0 + 4.5 * sx));
    const int ulo = std::max(0, FLR(transverse0 - 4.5 * st));
    const int uhi = std::min(N - 1, CEL(transverse0 + 4.5 * st));
    const double sign = (direction >= 0) ? 1.0 : -1.0;
    for (int v = 0; v < N; ++v)
    for (int u = ulo; u <= uhi; ++u)
    for (int x = xlo; x <= xhi; ++x) {
        const double j = scalar(x, u);
        const double face_sum = scalar(x + 1.0, u) + scalar(x - 1.0, u)
                              + scalar(x, u + 1.0) + scalar(x, u - 1.0)
                              + 2.0 * j;
        const double edge_sum = scalar(x + 1.0, u + 1.0) + scalar(x + 1.0, u - 1.0)
                              + scalar(x - 1.0, u + 1.0) + scalar(x - 1.0, u - 1.0)
                              + 2.0 * (scalar(x + 1.0, u) + scalar(x - 1.0, u)
                                     + scalar(x, u + 1.0) + scalar(x, u - 1.0));
        const double lap = face_sum / 3.0 + edge_sum / 6.0 - 4.0 * j;
        const double w = -0.5 * sign * C_SPEED
                        * (scalar(x + 1.0, u) - scalar(x - 1.0, u))
                        - 0.5 * C_SPEED * C_SPEED * lap;
        // z-polarized: finite in y, uniform z. y-polarized: finite in z,
        // uniform y. The varying transverse coordinate is always orthogonal
        // to the field component, so both branches are divergence-free.
        const int y = polarization_axis == 1 ? v : u;
        const int z = polarization_axis == 1 ? u : v;
        if (std::fabs(j) > 1e-12) {
            if (polarization_axis == 1) IF(rb, x, y, z, 0.0, j, 0.0);
            else                        IF(rb, x, y, z, 0.0, 0.0, j);
        }
        if (std::fabs(w) > 1e-12) {
            if (polarization_axis == 1) IW(rb, x, y, z, 0.0, w, 0.0);
            else                        IW(rb, x, y, z, 0.0, 0.0, w);
        }
    }
}

// Exactly transverse 1D pulse: J_z=f(x), uniform on each yz plane.  All Fourier
// support is parallel to x, making this the clean propagation/calibration packet
// when transverse localization (and its unavoidable angular spread) is not the
// observable under test.
inline void inject_plane_packet_x(RenderBridge& rb, double x0, double sigma_x,
                                  double amp, int direction = +1,
                                  double carrier_k = 0.0) {
    const int N = rb.lattice().size();
    const double sx = std::max(1.0, sigma_x);
    auto scalar = [&](double x) {
        const double dx = x - x0;
        if (std::fabs(dx) > 4.5 * sx) return 0.0;
        return amp * std::exp(-0.5 * dx*dx/(sx*sx)) * std::cos(carrier_k * dx);
    };
    const double sign = direction >= 0 ? 1.0 : -1.0;
    for (int x = 0; x < N; ++x) {
        const double jz = scalar(x);
        const double lap = scalar(x + 1.0) + scalar(x - 1.0) - 2.0 * jz;
        const double wz = -0.5 * sign * C_SPEED * (scalar(x + 1.0) - scalar(x - 1.0))
                        - 0.5 * C_SPEED * C_SPEED * lap;
        for (int y = 0; y < N; ++y)
        for (int z = 0; z < N; ++z) {
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0.0, 0.0, jz);
            if (std::fabs(wz) > 1e-12) IW(rb, x, y, z, 0.0, 0.0, wz);
        }
    }
}

// ── k=0 wave-momentum projection ────────────────────────────────────
//
// P5 (2026-07-26). On the periodic lattice the 18-point stencil weights sum to
// zero and every shift is a bijection, so Sum_v wave_vel is EXACTLY conserved.
// A seed that leaves it nonzero therefore pins a permanent uniform ramp:
// J_x(t) = W_x(0)*t forever, i.e. a spatially uniform E field the scenario
// never declares. Measured drift/tick was 0.0817 / 0.0120 / 0.0039 at
// L = 17/33/48 -- a 21x span across the three lattice sizes in use -- and for
// s0-seed-gluon the parasitic x-channel outweighed the seeded y-channel 20:1
// in |J| within five ticks.
//
// This is NOT an instability: a uniform A(t) is an exact source-free solution
// and energy is conserved, so the structured field is exactly unperturbed. It
// is a silent, lattice-size-dependent contaminant that no existing test can
// see (test_scenario_behavior's modified-Hamiltonian check is structurally
// blind to k=0, which contributes a constant and exactly zero gradient energy).
//
// Helper-built packets get Sum W = 0 for free because W is assembled from
// telescoping lattice derivatives; this projection is for hand-written seeds.
// Subtracting the mean removes exactly the k=0 component and leaves every other
// mode untouched.
inline void remove_wave_mean(RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const std::size_t count = voxels.size();
    if (count == 0) return;
    Vec3 sum(0.0, 0.0, 0.0);
    for (const auto& v : voxels) sum = sum + v.wave_vel;
    const double inv = 1.0 / static_cast<double>(count);
    const Vec3 mean(sum.x * inv, sum.y * inv, sum.z * inv);
    if (mean.mag2() == 0.0) return;
    // Write through the public injection path (voxels_ is private, and the
    // add-accumulate helper is the same one every seed uses).
    const Vec3 corr(-mean.x, -mean.y, -mean.z);
    const int N = rb.lattice().size();
    for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x)
                rb.inject_wave_vel_add(x, y, z, corr);
}

// ── Common scenario injection harnesses ─────────────────────────────
//
// B4 (2026-07-27 pre-commit audit): dp() interleaves a destructive marker
// placement (IPF, which unconditionally sets flux to (0,0,0) at its own
// center) with an accumulative dressing halo (IF) around that marker. Calling
// dp() twice in a row for two nearby markers means the SECOND marker's IPF
// can land inside the FIRST marker's already-deposited dressing halo and wipe
// it back to (0,0,0) -- an order-dependent, silently asymmetric result
// presented as a symmetric configuration. Confirmed to 5 significant figures
// for s0-vacuum-pion-charged at N=17: the marker placed first ends with real
// dressing-contributed flux, the marker placed second ends with flux exactly
// (0,0,0), reflecting nothing but its own IPF.
//
// Fix: split placement from dressing so every caller with more than one
// marker can PLACE ALL markers first, then DRESS ALL markers second. Once
// every IPF has already run, no dressing pass can ever be overwritten by a
// later placement. dp()/tri() are kept as convenience wrappers for genuinely
// isolated single-marker use (nothing else in the scenario body can be
// overwritten when there is only one marker); every current composite-marker
// caller in this codebase has been rewritten to call the split forms instead.
inline void dp_place(RenderBridge& rb, int cx, int cy, int cz,
                     int st, int sp, int co, bool lock) {
    IPF(rb, cx, cy, cz, st, sp, (co >= 0) ? co : 0);
    if (lock) LOCK(rb, cx, cy, cz);
}

inline void dp_dress(RenderBridge& rb, int cx, int cy, int cz,
                     int st, double sig, double amp) {
    int sn = (st > 0) ? 1 : -1;
    int eR = CEL(3.0 * sig);
    for (int dz2 = -eR; dz2 <= eR; dz2++) for (int dy2 = -eR; dy2 <= eR; dy2++) for (int dx2 = -eR; dx2 <= eR; dx2++) {
        if (dx2 == 0 && dy2 == 0 && dz2 == 0) continue;
        double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
        double rr = std::sqrt(r22);
        if (rr > 3.0 * sig) continue;
        double gg = amp * std::exp(-r22 / (2.0 * sig * sig));
        if (gg < 0.001) continue;
        IF(rb, cx+dx2, cy+dy2, cz+dz2, sn*gg*dx2/rr, sn*gg*dy2/rr, sn*gg*dz2/rr);
    }
}

inline void dp(RenderBridge& rb, int cx, int cy, int cz,
               int st, int sp, int co, double sig, double amp, bool lock) {
    dp_place(rb, cx, cy, cz, st, sp, co, lock);
    dp_dress(rb, cx, cy, cz, st, sig, amp);
}

// Three (x,y) positions computed by tri_place, passed to tri_dress so the
// dressing pass places at exactly the same voxels the placement pass used.
struct TriPositions { int x[3]; int y[3]; };

inline TriPositions tri_place(RenderBridge& rb, int cx, int cy, int cz,
                              const int charges[3], const int colors[3],
                              int rad, bool lock) {
    TriPositions p;
    for (int k = 0; k < 3; k++) {
        double ang = (2.0 * PI * k) / 3.0;
        p.x[k] = RND(cx + rad * std::cos(ang));
        p.y[k] = RND(cy + rad * std::sin(ang));
        dp_place(rb, p.x[k], p.y[k], cz, charges[k], (k % 2 == 0) ? 1 : -1,
                 colors[k], lock);
    }
    return p;
}

inline void tri_dress(RenderBridge& rb, const TriPositions& p, int cz,
                      const int charges[3]) {
    for (int k = 0; k < 3; k++) {
        dp_dress(rb, p.x[k], p.y[k], cz, charges[k], 2, 0.511 * 0.5);
    }
}

inline void tri(RenderBridge& rb, int cx, int cy, int cz,
                const int charges[3], const int colors[3], int rad, bool lock) {
    const TriPositions p = tri_place(rb, cx, cy, cz, charges, colors, rad, lock);
    tri_dress(rb, p, cz, charges);
}
// π lives in ftd:: via `using ontic::PI;` in ftd/constants.h — every
// scenario .cpp already includes constants.h, so call sites use `PI`
// directly without re-defining a SCN_PI alias here.

// Vacuum environment — mirror of JS applyVacuumEnvironment(bridge, ctx).
// In v1 this is a no-op (RenderBridge::reset() is invoked by the caller);
// kept as the single extension point for a future absorbing_boundary toggle.
inline void apply_vacuum_environment(RenderBridge& rb) {
    (void)rb;
}

}  // namespace
}  // namespace ftd
