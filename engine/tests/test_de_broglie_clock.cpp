// ============================================================================
// test_de_broglie_clock.cpp  (FTD-0271 Phase A, 2026-06-11)
// ----------------------------------------------------------------------------
// Verifies the [IMPOSED] de Broglie internal-clock mass term committed for
// FTD-0271. The de_broglie_clock toggle adds a Klein-Gordon rest-mass term
//
//     delta_j  -=  flux * omega0^2          (at manifested, state != 0 voxels)
//
// in phase_read (engine/src/render_bridge_phases/phase_read.cpp). Because
// delta_j is the flux ACCELERATION (the leapfrog in phase_write integrates
// wave_vel += delta_j; flux += wave_vel), the term turns the flux at a
// manifested voxel into a harmonic oscillator:  J'' = -omega0^2 J, i.e. the
// rest-frame Compton oscillation J(t) = J0*cos(omega0*t). FTD's native flux is
// massless (A0 audit: no restoring term), so the clock is IMPOSED, not forced.
//
// ── THE k=0 REST MODE (clean isolation) ─────────────────────────────────────
// To isolate the clock frequency from the spatial 18-pt Laplacian, the lattice
// is prepared in the k=0 rest mode: EVERY voxel manifested (state=+1) with a
// UNIFORM flux J0. The Laplacian of a uniform field is exactly 0 and stays 0
// (every voxel evolves identically, so the field remains uniform), and the
// state-flux coupling grad(s) is 0 for a uniform state. So delta_j = -omega0^2*J
// at every voxel and the whole field oscillates in phase at omega0 — the
// internal clock at rest. wave_propagation stays ON (its Laplacian is genuinely
// zero here), satisfying the toggle's `requires wave_propagation`.
//
// ── Discriminator ───────────────────────────────────────────────────────────
//   DB-1  clock OFF  -> flux stays constant at J0 (no oscillation without the
//                       clock; proves the oscillation is the clock, not waves).
//   DB-2  clock ON   -> flux oscillates: goes negative, >= 4 zero-crossings over
//                       ~4 periods, measured period within 20% of 2*pi/omega0.
//   DB-3  amplitude  -> the symplectic-Euler SHO conserves a bounded invariant;
//                       |flux.x| stays within [0.8, 1.3]*J0 (non-runaway).
//
// All CPU-only, deterministic, fast (L=8, ~90 ticks).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {
namespace test {

static constexpr double kPi = 3.14159265358979323846;

// Prepare the k=0 rest mode and tick `ticks` times, recording flux.x at the
// lattice centre each tick. clock_on selects the de_broglie_clock toggle.
static std::vector<double> run_rest_mode(int L, double J0, double omega0,
                                         int ticks, bool clock_on) {
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1);

    rb.toggles.disable_all();          // clears all bulk toggles to false
    rb.toggles.wave_propagation = true; // Laplacian is identically 0 (uniform)
    rb.toggles.de_broglie_clock = clock_on;
    rb.toggles.omega0           = omega0;
    // dual_substrate, coupling, damping, genesis, gauss_projection, forces,
    // movement, weak_transmutation all OFF after disable_all().

    // Uniform flux + manifested everywhere = the k=0 rest mode.
    const Vec3 J0v{J0, 0.0, 0.0};
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_particle(x, y, z, +1, J0v);

    const int c = L / 2;
    std::vector<double> trace;
    trace.reserve(static_cast<size_t>(ticks) + 1);
    trace.push_back(rb.voxel_at(c, c, c).flux.x);   // t = 0
    for (int t = 0; t < ticks; ++t) {
        rb.tick();
        trace.push_back(rb.voxel_at(c, c, c).flux.x);
    }
    return trace;
}

// Count sign changes of (s - mean) across a trace.
static int zero_crossings(const std::vector<double>& s) {
    double mean = 0.0;
    for (double v : s) mean += v;
    mean /= static_cast<double>(s.size());
    int crossings = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        const double a = s[i - 1] - mean, b = s[i] - mean;
        if ((a < 0.0 && b >= 0.0) || (a > 0.0 && b <= 0.0)) ++crossings;
    }
    return crossings;
}

static double max_abs(const std::vector<double>& s) {
    double m = 0.0;
    for (double v : s) m = std::max(m, std::abs(v));
    return m;
}
static double min_val(const std::vector<double>& s) {
    double m = s.empty() ? 0.0 : s[0];
    for (double v : s) m = std::min(m, v);
    return m;
}

void test_clock_off_is_static() {
    section("DB-1: clock OFF -> flux stays constant at J0 (no oscillation)");

    const double J0 = 0.1, omega0 = 0.3;
    const int L = 8, ticks = 90;
    const std::vector<double> off = run_rest_mode(L, J0, omega0, ticks, /*clock_on=*/false);

    // Non-vacuity: the initial flux really is J0 (inject_particle set it).
    std::printf("    [DB-1] flux.x(t=0) = %.6f  (expected J0 = %.6f)\n", off.front(), J0);
    check("DB-1: inject set flux.x to J0 at t=0 (test non-vacuous)",
          std::abs(off.front() - J0) < 1e-9,
          "inject_particle did not seed flux to J0; the oscillation test has no "
          "amplitude to work with.");

    double max_dev = 0.0;
    for (double v : off) max_dev = std::max(max_dev, std::abs(v - J0));
    std::printf("    [DB-1] max |flux.x - J0| over %d ticks = %.3e\n", ticks, max_dev);
    check("DB-1: clock OFF leaves the uniform flux exactly constant",
          max_dev < 1e-9,
          "With de_broglie_clock=false the uniform k=0 field drifted: either the "
          "Laplacian is not zero on a uniform field, or some other phase is "
          "perturbing the flux. The clock OFF run must be a flat control.");
}

void test_clock_on_oscillates() {
    section("DB-2: clock ON -> flux oscillates at omega0 (Compton rest-mode)");

    const double J0 = 0.1, omega0 = 0.3;
    const int L = 8, ticks = 90;
    const std::vector<double> on = run_rest_mode(L, J0, omega0, ticks, /*clock_on=*/true);

    const double T_expected = 2.0 * kPi / omega0;         // ~20.94 ticks
    const int crossings = zero_crossings(on);
    const double T_meas = (crossings > 0)
                          ? 2.0 * static_cast<double>(ticks) / crossings
                          : 0.0;
    const double mn = min_val(on);

    std::printf("    [DB-2] omega0=%.3f  T_expected=%.3f ticks  zero-crossings=%d  "
                "T_meas=%.3f  min(flux.x)=%.4f\n",
                omega0, T_expected, crossings, T_meas, mn);

    // (a) It actually swings negative — a real oscillation, not a slow drift.
    check("DB-2a: flux.x goes negative (genuine oscillation through zero)",
          mn < -0.4 * J0,
          "flux.x never went substantially negative; the clock is not producing "
          "the cos(omega0*t) rest-mode oscillation.");

    // (b) Enough crossings over ~4 periods.
    check("DB-2b: >= 4 zero-crossings over ~4 periods",
          crossings >= 4,
          "Too few zero-crossings: the flux is not oscillating at the expected "
          "clock cadence.");

    // (c) Measured period matches 2*pi/omega0 within 20%.
    const double rel = std::abs(T_meas - T_expected) / T_expected;
    std::printf("    [DB-2] |T_meas - T_expected| / T_expected = %.2f\n", rel);
    check("DB-2c: measured period == 2*pi/omega0 within 20%",
          rel < 0.20,
          "The oscillation period does not match the imposed clock frequency "
          "omega0 within 20%: the carrier frequency is set by something other "
          "than the -omega0^2*J mass term (e.g. a residual Laplacian).");
}

void test_amplitude_bounded() {
    section("DB-3: SHO invariant bounded -> |flux.x| stays near J0 (no runaway)");

    const double J0 = 0.1, omega0 = 0.3;
    const int L = 8, ticks = 90;
    const std::vector<double> on = run_rest_mode(L, J0, omega0, ticks, /*clock_on=*/true);

    const double amp = max_abs(on);
    std::printf("    [DB-3] max |flux.x| over %d ticks = %.4f  (J0 = %.4f)\n",
                ticks, amp, J0);
    check("DB-3: oscillation amplitude stays within [0.8, 1.3]*J0 (energy-bounded)",
          amp > 0.8 * J0 && amp < 1.3 * J0,
          "The symplectic-Euler SHO amplitude left the bounded window: either the "
          "clock is unstable (omega0*dt too large) or the integrator is leaking/"
          "pumping energy. With omega0=0.3 the invariant should be near-conserved.");
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_de_broglie_clock");
    ftd::test::test_clock_off_is_static();   // DB-1: flat control
    ftd::test::test_clock_on_oscillates();   // DB-2: oscillates at omega0
    ftd::test::test_amplitude_bounded();     // DB-3: bounded invariant
    return ftd::test::finalize();
}
