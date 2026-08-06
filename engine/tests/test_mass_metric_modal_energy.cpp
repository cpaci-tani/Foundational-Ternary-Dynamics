/** FTD-0675: canonical mass metric for connected tangent-mode energy. */

#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {
int failures = 0;
void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}
}  // namespace

int main() {
  constexpr double omega = 0.73;
  constexpr double amplitude = 0.019;
  const double exact = 0.5 * omega * omega * amplitude * amplitude;
  double canonical_min = INFINITY;
  double canonical_max = 0.0;
  double legacy_min = INFINITY;
  double legacy_max = 0.0;
  for (int sample = 0; sample <= 4096; ++sample) {
    const double phase = 2.0 * std::acos(-1.0) * sample / 4096.0;
    const double q = amplitude * std::sin(phase);
    const double momentum = omega * amplitude * std::cos(phase);
    const double canonical = 0.5 * (
        momentum * momentum + omega * omega * q * q);
    const double legacy_q = q / ftd::M_INERTIAL;
    const double legacy = 0.5 * (
        momentum * momentum + omega * omega * legacy_q * legacy_q);
    canonical_min = std::min(canonical_min, canonical);
    canonical_max = std::max(canonical_max, canonical);
    legacy_min = std::min(legacy_min, legacy);
    legacy_max = std::max(legacy_max, legacy);
  }
  const double expected_legacy_ratio =
      1.0 / (ftd::M_INERTIAL * ftd::M_INERTIAL);
  check("canonical harmonic invariant",
      std::abs(canonical_max - canonical_min) <= 1e-18);
  check("canonical expected energy",
      std::abs(canonical_min - exact) <= 1e-18);
  check("legacy false modulation", legacy_max - legacy_min >= 2.0 * exact);
  check("legacy modulation factor",
      std::abs(legacy_max / legacy_min - expected_legacy_ratio) <= 1e-12);

  std::cout.precision(17);
  std::cout << "mass=" << ftd::M_INERTIAL << '\n'
            << "canonical_variation=" << canonical_max - canonical_min << '\n'
            << "legacy_min=" << legacy_min << '\n'
            << "legacy_max=" << legacy_max << '\n'
            << "legacy_ratio=" << legacy_max / legacy_min << '\n'
            << "mass_metric_modal_energy failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
