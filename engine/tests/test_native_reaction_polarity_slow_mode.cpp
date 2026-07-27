/** Unit checks for the FTD-0431 reaction-mode observer. */

#include "ftd/eft/native_reaction_polarity_slow_mode.h"

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
    constexpr double survival = 0.9;
    const std::complex<double> initial{0.4, -0.3};
    std::vector<ftd::eft::NativeReactionModeMeasurement> samples;
    for (int tick = 0; tick <= 10; ++tick) {
        ftd::eft::NativeReactionModeMeasurement sample;
        sample.tick = tick;
        sample.source = initial * std::pow(survival, tick);
        samples.push_back(sample);
    }
    const auto fit = ftd::eft::fit_native_source_decay(samples, 6);
    check("analytic decay fit is valid", fit.valid);
    check("analytic decay rate recovered",
          std::abs(fit.gamma + std::log(survival)) < 1e-14);
    check("analytic decay residual is numerical zero",
          fit.normalized_rms < 1e-14);
    check("phase-referenced amplitude ignores common phase",
          std::abs(ftd::eft::native_phase_referenced_amplitude(
              initial * 0.75, initial) - 0.75) < 1e-14);

    ftd::RenderBridge bridge(16);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    auto& voxels = bridge.voxels();
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            for (int z = 0; z < 16; ++z) {
                const int index = bridge.lattice().index(x, y, z);
                bridge.set_state(index, static_cast<std::int8_t>(x < 8 ? 1 : -1));
                voxels[static_cast<std::size_t>(index)].flux = {};
                voxels[static_cast<std::size_t>(index)].wave_vel = {};
            }
        }
    }
    const auto measured = ftd::eft::measure_native_reaction_mode(
        bridge, 0, 1, {1, 0, 0});
    check("dense source is globally neutral", measured.signed_state == 0);
    check("dense source occupies full lattice", measured.occupancy == 16 * 16 * 16);
    check("dense source fundamental is nonzero", std::abs(measured.source) > 0.3);
    check("zero field has zero divergence", std::abs(measured.divergence) == 0.0);

    const std::array<double, 3> k{0.2, 0.1, 0.0};
    ftd::eft::NativeReactionModeMeasurement previous;
    ftd::eft::NativeReactionModeMeasurement current;
    ftd::eft::NativeReactionModeMeasurement next;
    previous.k = current.k = next.k = k;
    previous.divergence = {0.1, -0.03};
    current.divergence = {0.12, 0.04};
    current.source = {0.5, -0.2};
    double gradient_norm = 0.0;
    for (double component : k) gradient_norm += std::sin(component) * std::sin(component);
    next.divergence =
        (2.0 - ftd::C_WAVE * ftd::C_WAVE
                   * ftd::eft::native_wave_symbol_M(k)) * current.divergence
        - previous.divergence + ftd::G_C * gradient_norm * current.source;
    check("field recurrence residual closes",
          std::abs(ftd::eft::native_reaction_field_residual(
              previous, current, next)) < 1e-14);

    std::cout << "native_reaction_polarity_slow_mode failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
