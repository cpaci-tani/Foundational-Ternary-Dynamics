/**
 * Unit checks for the read-only FTD-0430 moving-source observer.
 */

#include "ftd/eft/native_retarded_polarity_response.h"

#include <cmath>
#include <complex>
#include <cstdint>
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
    constexpr int L = 16;
    ftd::RenderBridge moving(L);
    ftd::RenderBridge stationary(L);
    moving.force_cpu();
    stationary.force_cpu();
    moving.toggles.disable_all();
    stationary.toggles.disable_all();

    moving.set_state(5, 8, 8, 1);
    moving.set_state(11, 8, 8, -1);
    stationary.set_state(4, 8, 8, 1);
    stationary.set_state(10, 8, 8, -1);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = moving.lattice().index(x, y, z);
                moving.voxels()[static_cast<std::size_t>(index)].flux = {
                    0.1 * std::sin(2.0 * ftd::PI * x / L),
                    0.03 * std::cos(2.0 * ftd::PI * y / L), 0.0};
            }
        }
    }

    const auto modes = ftd::eft::measure_native_retarded_modes(
        moving, stationary);
    check("batch observer returns locked nine-mode basis", modes.size() == 9);
    check("sparse hop source is globally neutral",
          moving.charge_sum() == 0 && stationary.charge_sum() == 0);
    bool sources_nonzero = true;
    for (const auto& mode : modes)
        sources_nonzero = sources_nonzero && std::abs(mode.delta_source) > 1e-8;
    check("all locked sparse-hop modes are nonzero", sources_nonzero);

    const auto scalar_moving = ftd::eft::measure_native_polarity_mode(
        moving, 1, {1, 0, 0});
    const auto scalar_stationary = ftd::eft::measure_native_polarity_mode(
        stationary, 1, {1, 0, 0});
    check("batch source difference matches scalar observer",
          std::abs(modes[0].delta_source
              - (scalar_moving.source - scalar_stationary.source)) < 1e-14);
    check("batch divergence difference matches scalar observer",
          std::abs(modes[0].delta_divergence
              - (scalar_moving.divergence - scalar_stationary.divergence)) < 1e-14);

    const std::array<int, 4> changed{{
        stationary.lattice().index(4, 8, 8), moving.lattice().index(5, 8, 8),
        stationary.lattice().index(10, 8, 8), moving.lattice().index(11, 8, 8)}};
    const auto causal = ftd::eft::measure_native_causal_support(
        moving, stationary, changed, 1);
    check("synthetic divergence support is detected", causal.support_sites > 0);
    check("synthetic support radius is finite", causal.support_radius >= 0);

    const double omega = ftd::eft::native_discrete_pole({0.2, 0.0, 0.0});
    const std::complex<double> Z{0.256, 0.0};
    const std::complex<double> B = -Z;
    const std::complex<double> C = Z * std::tan(0.5 * omega);
    std::vector<ftd::eft::NativeResponseSample> samples;
    for (int tau = 0; tau <= 120; tau += 4) {
        samples.push_back({tau,
            Z + B * std::cos(omega * tau) + C * std::sin(omega * tau)});
    }
    const auto fit = ftd::eft::fit_native_response(samples, omega);
    check("step-response fit is valid", fit.valid);
    check("step-response fit recovers susceptibility",
          std::abs(fit.intercept - Z) < 1e-12);
    check("step residue follows the discrete pole identity",
          std::abs(ftd::eft::native_step_residue_ratio(fit)
                   - ftd::eft::native_exact_step_residue_ratio(omega)) < 1e-12);

    std::cout << "native_retarded_polarity_response failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
