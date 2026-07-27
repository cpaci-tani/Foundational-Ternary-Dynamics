/**
 * @file test_lorentz_ir_envelope.cpp
 * @brief FTD-0414 exact and finite-q gates for the selected IR envelope.
 */

#include "test_helpers.h"
#include "ftd/lorentz_bcc_time.h"
#include "ftd/lorentz_ir_envelope.h"

#include <array>
#include <cmath>

namespace {

using Vec = std::array<double, 3>;

double moore_symbol(const Vec& q) {
    const double cx = std::cos(q[0]);
    const double cy = std::cos(q[1]);
    const double cz = std::cos(q[2]);
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz)
         - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz);
}

double live_flux_phase(const Vec& q) {
    const double m = moore_symbol(q);
    return std::asin(std::sqrt(ftd::lorentz_bcc_time_floquet_x(m)));
}

double selected_matter_energy(const Vec& q) {
    constexpr double b = 1.0 / 3.0;
    constexpr double r2 = 4.0 / 3.0;
    double kinetic2 = 0.0;
    double wilson = 0.0;
    for (int mu = 0; mu < 3; ++mu) {
        const int nu = (mu + 1) % 3;
        const int rho = (mu + 2) % 3;
        const double transverse = (1.0 - 2.0 * b)
                                + b * (std::cos(q[nu]) + std::cos(q[rho]));
        const double k = std::sin(q[mu]) * transverse;
        kinetic2 += k * k;
        wilson += 1.0 - std::cos(q[mu]);
    }
    return std::sqrt(ftd::LORENTZ_IR_SELECTED_C2
                     * (kinetic2 + r2 * wilson * wilson));
}

double rk4_phase(double energy) {
    const double e2 = energy * energy;
    const double real = 1.0 - e2 / 2.0 + e2 * e2 / 24.0;
    const double imag = -energy + energy * e2 / 6.0;
    return -std::atan2(imag, real);
}

Vec scale(const Vec& n, double q) {
    return {q * n[0], q * n[1], q * n[2]};
}

double inferred_b(const Vec& n, double q, bool matter) {
    const Vec momentum = scale(n, q);
    const double phase = matter
        ? rk4_phase(selected_matter_energy(momentum))
        : live_flux_phase(momentum);
    const double q2 = q * q;
    return (phase * phase / ftd::LORENTZ_IR_SELECTED_C2 - q2)
         / (q2 * q2 * q2);
}

// Remove the leading O(q^2) contamination from the inferred q^6 coefficient.
double richardson_b(const Vec& n, double q, bool matter) {
    return (4.0 * inferred_b(n, q * 0.5, matter)
            - inferred_b(n, q, matter)) / 3.0;
}

}  // namespace

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;
    constexpr double inv_sqrt2 = 0.707106781186547524400844362104849039;
    constexpr double inv_sqrt3 = 0.577350269189625764509148780501957456;
    const Vec axis{1.0, 0.0, 0.0};
    const Vec face{inv_sqrt2, inv_sqrt2, 0.0};
    const Vec body{inv_sqrt3, inv_sqrt3, inv_sqrt3};

    check_close("axis matter coefficient is -101/8820",
                lorentz_matter_q6_coefficient(1.0, 0.0, 0.0),
                -101.0 / 8820.0, 2e-16, &c);
    check_close("body matter coefficient is 155/5292",
                lorentz_matter_q6_coefficient(1.0, 1.0, 1.0),
                155.0 / 5292.0, 2e-16, &c);
    check_close("axis flux coefficient is -1/245",
                lorentz_flux_q6_coefficient(1.0, 0.0, 0.0),
                -1.0 / 245.0, 2e-16, &c);
    check_close("body flux coefficient is -55/15876",
                lorentz_flux_q6_coefficient(1.0, 1.0, 1.0),
                -55.0 / 15876.0, 2e-16, &c);

    check_close("largest same-direction cone gap occurs on body diagonal",
                lorentz_common_cone_q6_gap(1.0, 1.0, 1.0),
                130.0 / 3969.0, 2e-16, &c);
    check_close("all-sector leading speed spread is 11/540",
                0.5 * (LORENTZ_IR_MATTER_BODY_B
                     - LORENTZ_IR_MATTER_AXIS_B),
                LORENTZ_IR_ALL_SPEED_SPREAD_COEFF, 2e-16, &c);
    check_close("same-direction leading speed-gap envelope is 65/3969",
                0.5 * (130.0 / 3969.0),
                LORENTZ_IR_COMMON_SPEED_GAP_COEFF, 2e-16, &c);

    // The helper must invert its own leading-order envelope exactly enough for
    // empirical tolerances to be supplied externally without hard-coding one.
    for (double tolerance : {1e-6, 1e-12, 1e-18}) {
        const double q_limit = lorentz_ir_q_limit(tolerance);
        check_close("q-limit inverts the requested speed tolerance",
                    lorentz_ir_leading_speed_spread(q_limit), tolerance,
                    2e-15, &c);
    }
    check_close("non-positive tolerance has no admitted q interval",
                lorentz_ir_q_limit(0.0), 0.0, 0.0, &c);

    // Compare the analytic coefficients with the exact live Floquet phase and
    // exact selected matter RK4 phase. Richardson removal isolates q^6 while
    // avoiding a fit or a search over parameters.
    constexpr double probe_q = 0.08;
    check_close("exact axis RK4 matter phase approaches analytic q6 coefficient",
                richardson_b(axis, probe_q, true),
                LORENTZ_IR_MATTER_AXIS_B, 2e-6, &c);
    check_close("exact body RK4 matter phase approaches analytic q6 coefficient",
                richardson_b(body, probe_q, true),
                LORENTZ_IR_MATTER_BODY_B, 2e-6, &c);
    check_close("exact axis live-flux phase approaches analytic q6 coefficient",
                richardson_b(axis, probe_q, false),
                LORENTZ_IR_FLUX_AXIS_B, 2e-6, &c);
    check_close("exact body live-flux phase approaches analytic q6 coefficient",
                richardson_b(body, probe_q, false),
                LORENTZ_IR_FLUX_BODY_B, 2e-6, &c);

    const auto face_moments = lorentz_direction_moments(
        face[0], face[1], face[2]);
    check_close("face direction has A4=1/2", face_moments.a4, 0.5,
                2e-16, &c);
    check_close("face direction has A6=1/4", face_moments.a6, 0.25,
                2e-16, &c);

    return report_and_exit_code(c,
        "FTD-0414 selected free-sector Lorentz IR envelope");
}
