// ============================================================================
// test_de_broglie_redshift.cpp  (FTD-0271 Phase A5, 2026-06-11)
// ----------------------------------------------------------------------------
// Verifies the [IMPOSED]->[SELECTION] half of the de Broglie clock: the clock's
// COVARIANT RATE is sourced by FTD's own proper-time, not imposed by hand.
//
// accumulate_proper_time (engine/src/transmutation_phases.cpp) integrates, per
// manifested voxel,
//
//     d(tau) = sqrt(f^2 - v^2) / sqrt(f),   f = 1 - L^2  (latency)
//
// and, when de_broglie_clock is ON, advances the clock phase
//
//     d(phi) = omega0 * d(tau).
//
// With the gravity/latency field OFF every particle has L = 0, so f = 1 and
//
//     d(tau) = sqrt(1 - v^2),     d(phi) = omega0 * sqrt(1 - v^2).
//
// So a MOVING clock advances SLOWER than a static one by exactly the special-
// relativistic factor sqrt(1 - v^2) — the de Broglie clock red-shift. FTD-0252
// independently MEASURED that this proper-time clock dilates as sqrt(1 - v^2);
// here we confirm the clock phase inherits it, so only the scalar omega0 is
// imposed (the covariant behaviour is FTD-native).
//
// ── Discriminator ───────────────────────────────────────────────────────────
//   RS-1  static clock ticks at omega0:  phi_static = omega0 * N  (within ~1%).
//   RS-2  moving clock red-shifts:        phi_moving / phi_static = sqrt(1-v^2).
//   RS-3  ordering:                       phi_moving < phi_static (slower clock).
//
// Two well-separated manifested particles in one bridge; movement OFF so the
// velocities are fixed inputs to the proper-time integrator (we measure the
// CLOCK RATE, not transport). CPU-only, deterministic, fast (L=12, 50 ticks).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>

namespace ftd {
namespace test {

void test_de_broglie_clock_redshift() {
    section("RS: de Broglie clock red-shifts as sqrt(1-v^2) (covariant rate from proper-time)");

    const int    L      = 12;
    const double omega0 = 0.3;
    const double vmove  = 0.3;     // < C_SPEED = 1/sqrt(3) ~ 0.577
    const int    N      = 50;

    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1);

    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;   // satisfies the toggle's requires-clause
    rb.toggles.de_broglie_clock = true;
    rb.toggles.omega0           = omega0;
    rb.toggles.latency_field    = false;  // => L = 0 => f = 1 => d(tau)=sqrt(1-v^2)
    // movement, forces, genesis, damping, gauss all OFF: velocities stay fixed.

    // Static particle at one corner, moving particle (fixed velocity) at another.
    const int sx = 3,  sy = 3,  sz = 3;
    const int mx = 8,  my = 8,  mz = 8;
    rb.inject_particle(sx, sy, sz, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(mx, my, mz, +1, Vec3{0.0, 0.0, 0.0});
    rb.voxel_at(sx, sy, sz).velocity = Vec3{0.0,   0.0, 0.0};
    rb.voxel_at(mx, my, mz).velocity = Vec3{vmove, 0.0, 0.0};

    for (int t = 0; t < N; ++t) rb.tick();

    const double phi_static = rb.voxel_at(sx, sy, sz).phase;
    const double phi_moving = rb.voxel_at(mx, my, mz).phase;
    const double v_read     = rb.voxel_at(mx, my, mz).speed();
    const double expected_ratio = std::sqrt(1.0 - v_read * v_read);

    std::printf("    [RS] omega0=%.3f  N=%d  v=%.3f\n", omega0, N, v_read);
    std::printf("    [RS] phi_static=%.5f (expect omega0*N=%.5f)  phi_moving=%.5f\n",
                phi_static, omega0 * N, phi_moving);
    std::printf("    [RS] ratio phi_moving/phi_static=%.5f   sqrt(1-v^2)=%.5f\n",
                (phi_static != 0.0 ? phi_moving / phi_static : 0.0), expected_ratio);

    // RS-1: the static clock ticks at omega0 per tick (d(tau)=1 at rest).
    const double rel_static = std::abs(phi_static - omega0 * N) / (omega0 * N);
    check("RS-1: static clock phase == omega0 * N within 1% (clock ticks at omega0)",
          rel_static < 0.01,
          "The static (v=0, L=0) clock did not advance at omega0 per tick: the "
          "phase wiring d(phi)=omega0*d(tau) is wrong, or accumulate_proper_time "
          "did not run with the clock on (latency_field is off).");

    // RS-2: the moving clock red-shifts by exactly sqrt(1-v^2).
    const double meas_ratio = (phi_static != 0.0) ? phi_moving / phi_static : 0.0;
    const double rel_ratio  = std::abs(meas_ratio - expected_ratio) / expected_ratio;
    check("RS-2: phi_moving/phi_static == sqrt(1-v^2) within 1% (covariant clock rate)",
          rel_ratio < 0.01,
          "The moving clock's phase ratio did not match the special-relativistic "
          "sqrt(1-v^2): the clock rate is NOT inheriting FTD's proper-time "
          "dilation (FTD-0252), so the covariant half is not FTD-sourced.");

    // RS-3: the moving clock is genuinely slower (non-vacuous ordering).
    check("RS-3: phi_moving < phi_static (the moving clock runs slow)",
          phi_moving < phi_static && phi_static > 0.0,
          "The moving clock did not run slower than the static one; there is no "
          "red-shift, so the test is vacuous or the velocity never registered.");
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_de_broglie_redshift");
    ftd::test::test_de_broglie_clock_redshift();
    return ftd::test::finalize();
}
