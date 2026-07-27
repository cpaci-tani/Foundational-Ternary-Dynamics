/**
 * Unit checks for the read-only FTD-0429 Fourier observer.
 */

#include "ftd/eft/native_dynamic_polarity_response.h"

#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
    std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!pass) ++failures;
}

}  // namespace

int main() {
    const std::array<double, 3> k{0.01, 0.0, 0.0};
    const double M = ftd::eft::native_wave_symbol_M(k);
    check("18-point symbol has Laplacian IR limit",
          std::abs(M / (k[0] * k[0]) - 1.0) < 1e-4);
    const double response = ftd::eft::native_exact_static_response(k);
    check("native susceptibility has finite IR limit",
          std::abs(response / (3.0 * ftd::G_C) - 1.0) < 1e-4);

    const double omega = ftd::eft::native_discrete_pole({0.2, 0.1, 0.0});
    check("native pole is stable and nonzero", omega > 0.0 && omega < ftd::PI);

    const std::complex<double> z{0.25, -0.01};
    const std::complex<double> cosine{-0.20, 0.03};
    const std::complex<double> sine{0.04, 0.02};
    std::vector<ftd::eft::NativeResponseSample> samples;
    for (int tick = 0; tick <= 80; tick += 5) {
        samples.push_back({tick,
            z + cosine * std::cos(omega * tick)
              + sine * std::sin(omega * tick)});
    }
    const auto fit = ftd::eft::fit_native_response(samples, omega);
    check("complex response fit is valid", fit.valid);
    check("response fit recovers intercept", std::abs(fit.intercept - z) < 1e-12);
    check("response fit recovers cosine", std::abs(fit.cosine - cosine) < 1e-12);
    check("response fit recovers sine", std::abs(fit.sine - sine) < 1e-12);
    check("response fit residual is numerical zero", fit.normalized_residual < 1e-12);

    ftd::RenderBridge bridge(16);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.dual_substrate = false;
    const int L = bridge.lattice().size();
    for (int x = 0; x < L; ++x) {
        const int state = x < L / 2 ? 1 : -1;
        for (int y = 0; y < L; ++y)
            for (int zc = 0; zc < L; ++zc)
                bridge.set_state(x, y, zc, static_cast<std::int8_t>(state));
    }
    const auto fundamental = ftd::eft::measure_native_polarity_mode(
        bridge, 1, {1, 0, 0});
    const auto third = ftd::eft::measure_native_polarity_mode(
        bridge, 3, {1, 0, 0});
    check("square source is globally neutral", bridge.charge_sum() == 0);
    check("square source supplies fundamental", std::abs(fundamental.source) > 0.5);
    check("square source supplies third harmonic", std::abs(third.source) > 0.1);
    check("zero initial field has zero response",
          std::abs(fundamental.response) == 0.0);

    std::cout << "native_dynamic_polarity_response failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
