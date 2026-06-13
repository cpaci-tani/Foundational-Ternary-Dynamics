// ============================================================================
// test_db_clock_coulomb.cpp  (FTD-0281 hook smoke, 2026-06-13)
// ----------------------------------------------------------------------------
// Verifies the default-off live Coulomb clock diagnostic added for the atomic
// hardening program. This is not the FTD-0281 spectroscopy verdict. It checks
// only the v1 engine hook:
//
//   * the toggle validates only in the preregistered single-substrate profile;
//   * tick() pre-solves phi_coulomb before phase_read when the toggle is on;
//   * the all-site KG term reads V=-phi_coulomb with the attractive-well sign.
//
// The full FFT peak-vs-operator comparison belongs to the FTD-0281 locked
// campaign. This test is deliberately small and golden-neutral when the toggle
// stays off.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <string>

namespace ftd {
namespace test {

static void configure_valid_profile(RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(0x0281u);
    rb.set_sor_iterations(40);

    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.poisson_coulomb = true;
    rb.toggles.de_broglie_clock = true;
    rb.toggles.db_clock_coulomb = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.forces = false;
    rb.toggles.omega0 = 1.0;
}

static void seed_uniform_field_with_source(RenderBridge& rb, double j0) {
    const int L = rb.lattice().size();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                rb.inject_flux(x, y, z, Vec3{j0, 0.0, 0.0});
            }
        }
    }

    const int c = L / 2;
    rb.inject_particle(c, c, c, +1, Vec3{j0, 0.0, 0.0});
}

void test_validation_contract() {
    section("DBC-1: toggle validation contract");

    TermToggles missing;
    missing.disable_all();
    missing.db_clock_coulomb = true;
    std::string err;
    const bool missing_ok = missing.validate(&err);
    std::printf("    [DBC-1] missing deps valid=%d err=%s\n",
                missing_ok ? 1 : 0, err.c_str());
    check("DBC-1a: db_clock_coulomb rejects missing preregistered dependencies",
          !missing_ok && err.find("db_clock_coulomb requires") != std::string::npos);

    TermToggles valid;
    valid.disable_all();
    valid.wave_propagation = true;
    valid.poisson_coulomb = true;
    valid.de_broglie_clock = true;
    valid.db_clock_coulomb = true;
    valid.dual_substrate = false;
    valid.forces = false;
    err.clear();
    const bool valid_ok = valid.validate(&err);
    std::printf("    [DBC-1] valid profile valid=%d err=%s\n",
                valid_ok ? 1 : 0, err.c_str());
    check("DBC-1b: preregistered single-substrate no-force profile validates",
          valid_ok, err.c_str());
}

void test_live_potential_changes_clock_field() {
    section("DBC-2: live phi_coulomb changes the all-site clock field");

    constexpr int L = 9;
    constexpr double J0 = 0.01;
    RenderBridge rb(L);
    configure_valid_profile(rb);
    seed_uniform_field_with_source(rb, J0);

    const int c = L / 2;
    const int center = rb.lattice().index(c, c, c);
    const int far = rb.lattice().index(0, 0, 0);

    rb.tick();

    const auto& phi = rb.phi_coulomb();
    const double phi_center = phi[center];
    const double phi_far = phi[far];
    const double center_step = rb.flux_at(center).x - J0;
    const double far_step = rb.flux_at(far).x - J0;

    std::printf("    [DBC-2] phi_center=%+.8e phi_far=%+.8e\n",
                phi_center, phi_far);
    std::printf("    [DBC-2] center_step=%+.8e far_step=%+.8e\n",
                center_step, far_step);

    check("DBC-2a: pre-read Coulomb solve populated an attractive source potential",
          phi_center > phi_far,
          "For a +1 source the engine force convention should make phi_C larger "
          "near the source than far away.");

    check("DBC-2b: V=-phi_C makes the source-site clock step less negative",
          center_step > far_step,
          "The Coulomb-coupled KG term should reduce omega_eff^2 near a +1 "
          "source, so a uniform positive field steps down less at the source "
          "than far away.");

    check("DBC-2c: both clock steps are finite",
          std::isfinite(center_step) && std::isfinite(far_step));
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_db_clock_coulomb");
    ftd::test::test_validation_contract();
    ftd::test::test_live_potential_changes_clock_field();
    return ftd::test::finalize();
}
