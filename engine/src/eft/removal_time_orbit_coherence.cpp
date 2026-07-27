#include "ftd/eft/removal_time_orbit_coherence.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

namespace ftd::eft {
namespace {

constexpr long double ORBIT_TOL = 5e-14L;
constexpr long double CHARACTER_TOL = 5e-13L;
constexpr std::array<int, 4> VOLUMES{{9, 17, 33, 65}};

struct Index3 {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct ModeOrbit {
  std::array<int, 3> representative{};
  std::vector<std::array<int, 3>> magnitude_permutations;
  std::vector<Index3> members;
  long double symbol = 0.0L;
  long double gradient2 = 0.0L;
  long double weight = 0.0L;
  long double pulse_envelope = 0.0L;
  long double step_envelope = 0.0L;
};

struct CompensatedSum {
  long double value = 0.0L;
  long double correction = 0.0L;

  void add(long double term) {
    const long double adjusted = term - correction;
    const long double next = value + adjusted;
    correction = (next - value) - adjusted;
    value = next;
  }
};

int linear_index(int L, const Index3& value) {
  return value.x + L * (value.y + L * value.z);
}

int signed_index(int L, int magnitude, int sign) {
  if (magnitude == 0) return 0;
  return sign > 0 ? magnitude : L - magnitude;
}

std::vector<std::array<int, 3>> unique_permutations(
    std::array<int, 3> value) {
  std::sort(value.begin(), value.end());
  std::vector<std::array<int, 3>> result;
  do {
    result.push_back(value);
  } while (std::next_permutation(value.begin(), value.end()));
  return result;
}

std::vector<Index3> orbit_members(int L, std::array<int, 3> value) {
  std::set<std::tuple<int, int, int>> unique;
  for (const auto& permutation : unique_permutations(value)) {
    for (int sx : {-1, 1}) {
      if (permutation[0] == 0 && sx < 0) continue;
      for (int sy : {-1, 1}) {
        if (permutation[1] == 0 && sy < 0) continue;
        for (int sz : {-1, 1}) {
          if (permutation[2] == 0 && sz < 0) continue;
          unique.emplace(
              signed_index(L, permutation[0], sx),
              signed_index(L, permutation[1], sy),
              signed_index(L, permutation[2], sz));
        }
      }
    }
  }
  std::vector<Index3> result;
  result.reserve(unique.size());
  for (const auto& [x, y, z] : unique) result.push_back({x, y, z});
  return result;
}

std::pair<long double, long double> symbol_and_gradient(
    int L, const Index3& mode) {
  const long double pi = std::acos(-1.0L);
  const long double scale = 2.0L * pi / static_cast<long double>(L);
  const long double kx = scale * mode.x;
  const long double ky = scale * mode.y;
  const long double kz = scale * mode.z;
  const long double cx = std::cos(kx);
  const long double cy = std::cos(ky);
  const long double cz = std::cos(kz);
  const long double sx = std::sin(kx);
  const long double sy = std::sin(ky);
  const long double sz = std::sin(kz);
  const long double symbol =
      4.0L - (2.0L / 3.0L) * (cx + cy + cz)
      - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz);
  const long double gradient2 = sx * sx + sy * sy + sz * sz;
  return {symbol, gradient2};
}

std::vector<ModeOrbit> build_orbits(
    int L, bool& exact_coverage, long double& invariance_residual) {
  const int half = L / 2;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  std::vector<ModeOrbit> orbits;
  std::vector<unsigned char> covered(
      static_cast<std::size_t>(L) * L * L, 0u);
  covered[0] = 1u;
  bool duplicate = false;
  int covered_nonzero = 0;

  for (int a = 0; a <= half; ++a) {
    for (int b = a; b <= half; ++b) {
      for (int c = b; c <= half; ++c) {
        if (a == 0 && b == 0 && c == 0) continue;
        ModeOrbit orbit;
        orbit.representative = {a, b, c};
        orbit.magnitude_permutations = unique_permutations({a, b, c});
        orbit.members = orbit_members(L, {a, b, c});
        const auto [symbol, gradient2] =
            symbol_and_gradient(L, {a, b, c});
        orbit.symbol = symbol;
        orbit.gradient2 = gradient2;
        orbit.weight = gradient2 / symbol;
        const long double denominator = std::sqrt(1.0L - c2 * symbol / 4.0L);
        orbit.pulse_envelope = 2.0L / denominator;
        orbit.step_envelope = 1.0L + 1.0L / denominator;

        for (const Index3& member : orbit.members) {
          const int index = linear_index(L, member);
          if (covered[static_cast<std::size_t>(index)] != 0u) duplicate = true;
          covered[static_cast<std::size_t>(index)] = 1u;
          ++covered_nonzero;
          const auto [member_symbol, member_gradient2] =
              symbol_and_gradient(L, member);
          invariance_residual = std::max(
              invariance_residual,
              std::max({std::abs(member_symbol - symbol),
                        std::abs(member_gradient2 - gradient2),
                        std::abs(member_gradient2 / member_symbol
                                 - orbit.weight)}));
        }
        orbits.push_back(std::move(orbit));
      }
    }
  }

  exact_coverage = !duplicate && covered_nonzero == L * L * L - 1;
  for (unsigned char value : covered) exact_coverage = exact_coverage && value != 0u;
  return orbits;
}

long double orbit_character(
    const ModeOrbit& orbit, const Index3& displacement,
    const std::vector<std::vector<long double>>& signed_cosine) {
  long double character = 0.0L;
  for (const auto& permutation : orbit.magnitude_permutations) {
    character +=
        signed_cosine[static_cast<std::size_t>(permutation[0])]
                     [static_cast<std::size_t>(displacement.x)]
        * signed_cosine[static_cast<std::size_t>(permutation[1])]
                       [static_cast<std::size_t>(displacement.y)]
        * signed_cosine[static_cast<std::size_t>(permutation[2])]
                       [static_cast<std::size_t>(displacement.z)];
  }
  return character;
}

long double direct_orbit_character(
    int L, const ModeOrbit& orbit, const Index3& displacement,
    long double& imaginary_residual) {
  const long double pi = std::acos(-1.0L);
  const long double scale = 2.0L * pi / static_cast<long double>(L);
  CompensatedSum real;
  CompensatedSum imaginary;
  for (const Index3& mode : orbit.members) {
    const int phase_index = (
        mode.x * displacement.x + mode.y * displacement.y
        + mode.z * displacement.z) % L;
    const long double phase = scale * static_cast<long double>(phase_index);
    real.add(std::cos(phase));
    imaginary.add(-std::sin(phase));
  }
  imaginary_residual = std::max(
      imaginary_residual, std::abs(imaginary.value));
  return real.value;
}

RemovalTimeOrbitCoherenceVolume analyze_volume(int L) {
  RemovalTimeOrbitCoherenceVolume result;
  result.lattice_size = L;
  result.nonzero_mode_count = L * L * L - 1;

  bool exact_coverage = false;
  long double invariance_residual = 0.0L;
  const std::vector<ModeOrbit> orbits =
      build_orbits(L, exact_coverage, invariance_residual);
  result.mode_orbit_count = static_cast<int>(orbits.size());
  result.exact_orbit_coverage = exact_coverage;
  result.maximum_orbit_invariance_residual =
      static_cast<double>(invariance_residual);
  result.orbit_invariance_verified = invariance_residual <= ORBIT_TOL;

  CompensatedSum pulse_sum_accumulator;
  CompensatedSum weight_sum_accumulator;
  CompensatedSum common_step_sum_accumulator;
  for (const ModeOrbit& orbit : orbits) {
    const long double multiplicity =
        static_cast<long double>(orbit.members.size());
    pulse_sum_accumulator.add(multiplicity * orbit.pulse_envelope
                              * orbit.pulse_envelope / orbit.symbol);
    weight_sum_accumulator.add(multiplicity * orbit.weight);
    common_step_sum_accumulator.add(multiplicity * orbit.step_envelope
                                    * orbit.step_envelope / orbit.symbol);
  }
  const long double pulse_sum = pulse_sum_accumulator.value;
  const long double weight_sum = weight_sum_accumulator.value;
  const long double common_step_sum = common_step_sum_accumulator.value;

  const int half = L / 2;
  const long double pi = std::acos(-1.0L);
  std::vector<std::vector<long double>> signed_cosine(
      static_cast<std::size_t>(half + 1),
      std::vector<long double>(static_cast<std::size_t>(half + 1), 0.0L));
  for (int magnitude = 0; magnitude <= half; ++magnitude) {
    for (int displacement = 0; displacement <= half; ++displacement) {
      signed_cosine[static_cast<std::size_t>(magnitude)]
                   [static_cast<std::size_t>(displacement)] =
          magnitude == 0 ? 1.0L :
          2.0L * std::cos(2.0L * pi * magnitude * displacement / L);
    }
  }

  long double maximum_numerator = -1.0L;
  Index3 maximizing{};
  int displacement_count = 0;
  for (int dx = 0; dx <= half; ++dx) {
    for (int dy = dx; dy <= half; ++dy) {
      for (int dz = dy; dz <= half; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++displacement_count;
        const Index3 displacement{dx, dy, dz};
        CompensatedSum numerator;
        for (const ModeOrbit& orbit : orbits) {
          numerator.add(orbit.weight * std::abs(
              orbit_character(orbit, displacement, signed_cosine)));
        }
        if (numerator.value > maximum_numerator) {
          maximum_numerator = numerator.value;
          maximizing = displacement;
        }
      }
    }
  }
  result.displacement_orbit_count = displacement_count;
  result.maximizing_dx = maximizing.x;
  result.maximizing_dy = maximizing.y;
  result.maximizing_dz = maximizing.z;

  long double character_residual = 0.0L;
  long double imaginary_residual = 0.0L;
  CompensatedSum direct_numerator;
  for (const ModeOrbit& orbit : orbits) {
    const long double formula =
        orbit_character(orbit, maximizing, signed_cosine);
    const long double direct =
        direct_orbit_character(L, orbit, maximizing, imaginary_residual);
    character_residual = std::max(
        character_residual, std::abs(formula - direct));
    direct_numerator.add(orbit.weight * std::abs(direct));
  }
  character_residual = std::max(
      character_residual,
      std::abs(maximum_numerator - direct_numerator.value) / weight_sum);
  character_residual = std::max(character_residual, imaginary_residual);
  result.maximum_character_residual = static_cast<double>(character_residual);
  result.direct_character_verified = character_residual <= CHARACTER_TOL;

  const long double coherence = maximum_numerator / weight_sum;
  const long double volume = static_cast<long double>(L) * L * L;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  const long double coupling = static_cast<long double>(G_C);
  const long double pulse_operator =
      coupling / (c2 * volume) * std::sqrt(pulse_sum * weight_sum);
  const long double common_step =
      coupling / c2 * std::sqrt(common_step_sum / volume);

  long double maximum_bound = -std::numeric_limits<long double>::infinity();
  int maximizing_removed = -1;
  for (int removed = 0; removed <= 7; ++removed) {
    const long double remaining_term =
        common_step * std::sqrt(static_cast<long double>(7 - removed));
    const long double removed_factor = static_cast<long double>(removed)
        + coherence * removed * (removed - 1);
    const long double removed_term =
        pulse_operator * std::sqrt(std::max(0.0L, removed_factor));
    const long double bound = remaining_term + removed_term;
    if (bound > maximum_bound) {
      maximum_bound = bound;
      maximizing_removed = removed;
    }
  }

  result.pulse_cauchy_sum = static_cast<double>(pulse_sum);
  result.gradient_weight_sum = static_cast<double>(weight_sum);
  result.pulse_operator_coefficient = static_cast<double>(pulse_operator);
  result.maximum_orbit_coherence = static_cast<double>(coherence);
  result.common_step_coefficient = static_cast<double>(common_step);
  result.seven_source_orbit_bound = static_cast<double>(maximum_bound);
  result.seven_source_margin = K_GENESIS - result.seven_source_orbit_bound;
  result.maximizing_removed_count_at_seven = maximizing_removed;
  result.coherence_in_unit_interval =
      coherence >= -1e-14L && coherence <= 1.0L + 1e-14L;
  result.seven_source_closed = maximum_bound
      < static_cast<long double>(K_GENESIS);
  result.valid = result.exact_orbit_coverage
      && result.orbit_invariance_verified
      && result.direct_character_verified
      && result.coherence_in_unit_interval
      && std::isfinite(result.pulse_cauchy_sum)
      && std::isfinite(result.gradient_weight_sum)
      && std::isfinite(result.pulse_operator_coefficient)
      && std::isfinite(result.maximum_orbit_coherence)
      && std::isfinite(result.seven_source_orbit_bound);
  return result;
}

}  // namespace

RemovalTimeOrbitCoherenceResult analyze_removal_time_orbit_coherence() {
  RemovalTimeOrbitCoherenceResult result;
  result.volumes.reserve(VOLUMES.size());
  bool all_valid = true;
  bool all_coverage = true;
  bool all_character = true;
  bool all_closed = true;
  for (int L : VOLUMES) {
    RemovalTimeOrbitCoherenceVolume volume = analyze_volume(L);
    all_valid = all_valid && volume.valid;
    all_coverage = all_coverage && volume.exact_orbit_coverage;
    all_character = all_character && volume.direct_character_verified;
    all_closed = all_closed && volume.seven_source_closed;
    result.volumes.push_back(volume);
    ++result.spectral_volume_count;
  }
  result.cubic_orbit_bound_derived = all_valid;
  result.all_orbit_partitions_exact = all_coverage;
  result.all_direct_character_checks_pass = all_character;
  result.arbitrary_removal_n_le_seven_closed = all_valid && all_closed;
  result.seven_source_bound_inconclusive = all_valid && !all_closed;
  result.production_changed = false;
  result.valid = all_valid && result.spectral_volume_count == 4
      && (result.arbitrary_removal_n_le_seven_closed
          || result.seven_source_bound_inconclusive);
  return result;
}

}  // namespace ftd::eft
