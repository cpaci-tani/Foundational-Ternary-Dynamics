/**
 * @file test_lorentz_period2_floquet.cpp
 * @brief FTD-0408 exact and engine-wiring gates for the P4 period-two wave map.
 */

#include "test_helpers.h"
#include "ftd/lorentz_period2.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <string>

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;

    check_close("even kick = 3/13", LORENTZ_PERIOD2_KAPPA_EVEN,
                3.0 / 13.0, 0.0, &c);
    check_close("odd kick = -1/13", LORENTZ_PERIOD2_KAPPA_ODD,
                -1.0 / 13.0, 0.0, &c);
    check_close("two-tick effective c^2 = 1/13",
                0.5 * (LORENTZ_PERIOD2_KAPPA_EVEN
                     + LORENTZ_PERIOD2_KAPPA_ODD),
                LORENTZ_PERIOD2_EFFECTIVE_C2, 1e-16, &c);

    const double mmax = 16.0 / 3.0;
    check_close("full-band Floquet X(Mmax) = 272/507",
                lorentz_period2_floquet_x(mmax), 272.0 / 507.0,
                1e-15, &c);
    check("full-band endpoint lies strictly inside stability interval",
          lorentz_period2_floquet_x(mmax) > 0.0
          && lorentz_period2_floquet_x(mmax) < 1.0, &c);

    TermToggles defaults;
    check("prototype defaults OFF", !defaults.lorentz_period2_floquet, &c);

    TermToggles missing_wave;
    missing_wave.disable_all();
    missing_wave.lorentz_period2_floquet = true;
    std::string err;
    check("prototype requires wave_propagation",
          !missing_wave.validate(&err)
          && err.find("requires wave_propagation") != std::string::npos, &c);

    TermToggles conflict;
    conflict.disable_all();
    conflict.wave_propagation = true;
    conflict.lorentz_period2_floquet = true;
    conflict.symplectic_leapfrog = true;
    check("prototype rejects alternate leapfrog owner",
          !conflict.validate(&err)
          && err.find("mutually exclusive") != std::string::npos, &c);

    // Engine wiring: an exact axis Fourier mode is an eigenmode of the full
    // 18-point stencil with M=2-2cos(q).  prepare_delta_j() exposes the kick
    // without mutating the field, so the ratio delta/J directly identifies
    // the even-tick coefficient.  One tick then advances the clock parity;
    // the same ratio identifies the odd-tick anti-kick on the evolved mode.
    constexpr int L = 17;
    RenderBridge rb(L);
    prepare_bridge(rb, /*force_cpu=*/true);
    rb.toggles.wave_propagation = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.lorentz_period2_floquet = true;
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
    check_close("engine even-tick acceleration uses +3/13",
                rb.delta_j()[probe].y,
                -LORENTZ_PERIOD2_KAPPA_EVEN * mode_symbol,
                2e-14, &c);

    rb.tick();
    const double j1 = rb.voxels()[probe].flux.y;
    rb.prepare_delta_j();
    check_close("engine odd-tick acceleration uses -1/13",
                rb.delta_j()[probe].y,
                -LORENTZ_PERIOD2_KAPPA_ODD * mode_symbol * j1,
                2e-14, &c);

    rb.tick();
    const double expected_j2 = 1.0
        - 5.0 * mode_symbol / 13.0
        - 3.0 * mode_symbol * mode_symbol / 169.0;
    check_close("engine two-tick monodromy matches exact recurrence",
                rb.voxels()[probe].flux.y, expected_j2, 5e-14, &c);

    return report_and_exit_code(c, "FTD-0408 period-two Lorentz prototype");
}
