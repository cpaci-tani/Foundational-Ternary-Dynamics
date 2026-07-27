/**
 * @file test_lorentz_bcc_time_floquet.cpp
 * @brief FTD-0411 exact and live-wiring gates for the BCC-time IR surrogate.
 */

#include "test_helpers.h"
#include "ftd/lorentz_bcc_time.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <string>

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;

    check_close("kick sum = 2/7",
                LORENTZ_BCC_TIME_KAPPA_EVEN
                    + LORENTZ_BCC_TIME_KAPPA_ODD,
                2.0 / 7.0, 1e-16, &c);
    check_close("kick product = -1/49",
                LORENTZ_BCC_TIME_KAPPA_EVEN
                    * LORENTZ_BCC_TIME_KAPPA_ODD,
                -1.0 / 49.0, 1e-16, &c);
    check_close("two-tick effective c^2 = 1/7",
                0.5 * (LORENTZ_BCC_TIME_KAPPA_EVEN
                     + LORENTZ_BCC_TIME_KAPPA_ODD),
                LORENTZ_BCC_TIME_EFFECTIVE_C2, 1e-16, &c);

    const double mmax = 16.0 / 3.0;
    check_close("full-band Floquet X(Mmax) = 400/441",
                lorentz_bcc_time_floquet_x(mmax), 400.0 / 441.0,
                2e-15, &c);
    check("full-band endpoint lies strictly inside stability interval",
          lorentz_bcc_time_floquet_x(mmax) > 0.0
          && lorentz_bcc_time_floquet_x(mmax) < 1.0, &c);

    TermToggles defaults;
    check("prototype defaults OFF", !defaults.lorentz_bcc_time_floquet, &c);

    TermToggles missing_wave;
    missing_wave.disable_all();
    missing_wave.lorentz_bcc_time_floquet = true;
    std::string err;
    check("prototype requires wave_propagation",
          !missing_wave.validate(&err)
          && err.find("requires wave_propagation") != std::string::npos, &c);

    TermToggles conflict;
    conflict.disable_all();
    conflict.wave_propagation = true;
    conflict.lorentz_bcc_time_floquet = true;
    conflict.lorentz_period2_floquet = true;
    check("BCC-time prototype rejects the FTD-0408 wave owner",
          !conflict.validate(&err)
          && err.find("lorentz_period2_floquet") != std::string::npos, &c);

    conflict.lorentz_period2_floquet = false;
    conflict.symplectic_leapfrog = true;
    check("BCC-time prototype rejects alternate leapfrog owner",
          !conflict.validate(&err)
          && err.find("mutually exclusive") != std::string::npos, &c);

    conflict.symplectic_leapfrog = false;
    conflict.verlet_wave_integrator = true;
    check("BCC-time prototype rejects alternate Verlet owner",
          !conflict.validate(&err)
          && err.find("verlet_wave_integrator") != std::string::npos, &c);

    // Exact axis mode wiring. Each microscopic tick still reads the existing
    // M18 nearest-Moore symbol; only the kick coefficient alternates.
    constexpr int L = 17;
    RenderBridge rb(L);
    prepare_bridge(rb, /*force_cpu=*/true);
    rb.toggles.wave_propagation = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.lorentz_bcc_time_floquet = true;
    rb.set_dt(0.25);
    check_close("prototype enforces its proved unit timestep", rb.dt(), 1.0,
                0.0, &c);

    const double q = 2.0 * PI / static_cast<double>(L);
    const double mode_symbol = 2.0 - 2.0 * std::cos(q);
    for (int x = 0; x < L; ++x) {
        const double value = std::cos(q * static_cast<double>(x));
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
                v.flux = {0.0, value, 0.0};
                v.wave_vel = {};
            }
        }
    }

    const int probe = rb.lattice().index(0, 0, 0);
    rb.prepare_delta_j();
    check_close("engine even-tick acceleration uses (1+sqrt(2))/7",
                rb.delta_j()[probe].y,
                -LORENTZ_BCC_TIME_KAPPA_EVEN * mode_symbol,
                3e-14, &c);

    rb.tick();
    const double j1 = rb.voxels()[probe].flux.y;
    rb.prepare_delta_j();
    check_close("engine odd-tick acceleration uses (1-sqrt(2))/7",
                rb.delta_j()[probe].y,
                -LORENTZ_BCC_TIME_KAPPA_ODD * mode_symbol * j1,
                3e-14, &c);

    rb.tick();
    const double expected_j2 = 1.0
        - (3.0 + LORENTZ_BCC_TIME_SQRT2) * mode_symbol / 7.0
        - mode_symbol * mode_symbol / 49.0;
    check_close("engine two-tick monodromy matches exact recurrence",
                rb.voxels()[probe].flux.y, expected_j2, 7e-14, &c);

    return report_and_exit_code(c, "FTD-0411 BCC-time Floquet surrogate");
}
