#include "ftd/eft/ten_source_pair_distance_capacity.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ftd::eft {
namespace {

constexpr long double KERNEL_TOL = 5e-13L;
constexpr long double CROSS_TOL = 5e-12L;
constexpr std::array<int, 4> VOLUMES{{9, 17, 33, 65}};

using Polynomial = std::vector<long long>;
using ExactKey = std::vector<long long>;

void trim(Polynomial& value) {
  while (!value.empty() && value.back() == 0) value.pop_back();
}

long long checked_multiply(long long lhs, long long rhs) {
  if (lhs == 0 || rhs == 0) return 0;
  const long double magnitude = std::abs(static_cast<long double>(lhs))
      * std::abs(static_cast<long double>(rhs));
  if (magnitude > static_cast<long double>(
          std::numeric_limits<long long>::max())) {
    throw std::overflow_error("exact polynomial multiplication overflow");
  }
  return lhs * rhs;
}

long long checked_add(long long lhs, long long rhs) {
  if ((rhs > 0 && lhs > std::numeric_limits<long long>::max() - rhs)
      || (rhs < 0 && lhs < std::numeric_limits<long long>::min() - rhs)) {
    throw std::overflow_error("exact polynomial addition overflow");
  }
  return lhs + rhs;
}

Polynomial multiply(const Polynomial& lhs, const Polynomial& rhs) {
  if (lhs.empty() || rhs.empty()) return {};
  Polynomial out(lhs.size() + rhs.size() - 1, 0);
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    for (std::size_t j = 0; j < rhs.size(); ++j) {
      out[i + j] = checked_add(
          out[i + j], checked_multiply(lhs[i], rhs[j]));
    }
  }
  trim(out);
  return out;
}

Polynomial divide_exact_monic(Polynomial numerator,
                              const Polynomial& denominator) {
  trim(numerator);
  if (denominator.empty() || denominator.back() != 1
      || numerator.size() < denominator.size()) {
    throw std::runtime_error("invalid monic exact division");
  }
  Polynomial quotient(numerator.size() - denominator.size() + 1, 0);
  while (numerator.size() >= denominator.size()) {
    const std::size_t shift = numerator.size() - denominator.size();
    const long long coefficient = numerator.back();
    quotient[shift] = coefficient;
    for (std::size_t j = 0; j < denominator.size(); ++j) {
      numerator[shift + j] = checked_add(
          numerator[shift + j],
          -checked_multiply(coefficient, denominator[j]));
    }
    trim(numerator);
  }
  if (!numerator.empty()) throw std::runtime_error("non-exact division");
  trim(quotient);
  return quotient;
}

std::vector<int> divisors(int n) {
  std::vector<int> out;
  for (int d = 1; d <= n; ++d) if (n % d == 0) out.push_back(d);
  return out;
}

const Polynomial& cyclotomic(int n, std::map<int, Polynomial>& cache) {
  const auto found = cache.find(n);
  if (found != cache.end()) return found->second;
  Polynomial value(static_cast<std::size_t>(n + 1), 0);
  value[0] = -1;
  value[static_cast<std::size_t>(n)] = 1;
  for (int d : divisors(n)) {
    if (d != n) value = divide_exact_monic(value, cyclotomic(d, cache));
  }
  return cache.emplace(n, std::move(value)).first->second;
}

int modulo(int value, int L) {
  const int result = value % L;
  return result < 0 ? result + L : result;
}

ExactKey exact_symbol_key(int L, const std::array<int, 3>& mode,
                          const Polynomial& phi) {
  Polynomial value(static_cast<std::size_t>(L), 0);
  auto add = [&](int exponent, long long coefficient) {
    const std::size_t index = static_cast<std::size_t>(modulo(exponent, L));
    value[index] = checked_add(value[index], coefficient);
  };
  add(0, 24);
  for (int component : mode) {
    add(component, -2);
    add(-component, -2);
  }
  for (int i = 0; i < 3; ++i) {
    for (int j = i + 1; j < 3; ++j) {
      for (int si : {-1, 1}) {
        for (int sj : {-1, 1}) {
          add(si * mode[static_cast<std::size_t>(i)]
              + sj * mode[static_cast<std::size_t>(j)], -1);
        }
      }
    }
  }
  const std::size_t degree = phi.size() - 1;
  for (std::size_t index = value.size(); index-- > degree;) {
    const long long coefficient = value[index];
    if (coefficient == 0) continue;
    const std::size_t shift = index - degree;
    for (std::size_t j = 0; j < phi.size(); ++j) {
      value[shift + j] = checked_add(
          value[shift + j], -checked_multiply(coefficient, phi[j]));
    }
  }
  value.resize(degree);
  return value;
}

std::string serialize_key(const ExactKey& key) {
  std::ostringstream out;
  for (std::size_t i = 0; i < key.size(); ++i) {
    if (i != 0) out << ':';
    out << key[i];
  }
  return out.str();
}

struct Index3 {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct ModeOrbit {
  std::array<int, 3> representative{};
  std::vector<std::array<int, 3>> magnitude_permutations;
  std::vector<Index3> members;
  std::string exact_key;
  int shell_index = -1;
  long double symbol = 0.0L;
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

std::vector<std::array<int, 3>> unique_permutations(
    std::array<int, 3> value) {
  std::sort(value.begin(), value.end());
  std::vector<std::array<int, 3>> out;
  do {
    out.push_back(value);
  } while (std::next_permutation(value.begin(), value.end()));
  return out;
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
              modulo(sx * permutation[0], L),
              modulo(sy * permutation[1], L),
              modulo(sz * permutation[2], L));
        }
      }
    }
  }
  std::vector<Index3> out;
  out.reserve(unique.size());
  for (const auto& [x, y, z] : unique) out.push_back({x, y, z});
  return out;
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
  return {
      4.0L - (2.0L / 3.0L) * (cx + cy + cz)
          - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz),
      sx * sx + sy * sy + sz * sz};
}

std::vector<ModeOrbit> build_orbits(
    int L, const Polynomial& phi,
    const TenSourceSharedMCoherenceVolume& parent,
    bool& exact_parent_partition) {
  const int half = L / 2;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  std::map<std::string, int> parent_shell_index;
  std::map<std::string, std::pair<int, int>> expected_counts;
  for (std::size_t i = 0; i < parent.shells.size(); ++i) {
    parent_shell_index[parent.shells[i].exact_key] = static_cast<int>(i);
    expected_counts[parent.shells[i].exact_key] = {
        parent.shells[i].orbit_count, parent.shells[i].mode_count};
  }
  std::map<std::string, std::pair<int, int>> observed_counts;
  std::vector<ModeOrbit> orbits;
  exact_parent_partition = true;
  for (int a = 0; a <= half; ++a) {
    for (int b = a; b <= half; ++b) {
      for (int c = b; c <= half; ++c) {
        if (a == 0 && b == 0 && c == 0) continue;
        ModeOrbit orbit;
        orbit.representative = {a, b, c};
        orbit.magnitude_permutations = unique_permutations({a, b, c});
        orbit.members = orbit_members(L, {a, b, c});
        orbit.exact_key = serialize_key(exact_symbol_key(
            L, orbit.representative, phi));
        const auto shell = parent_shell_index.find(orbit.exact_key);
        if (shell == parent_shell_index.end()) {
          exact_parent_partition = false;
          orbit.shell_index = 0;
        } else {
          orbit.shell_index = shell->second;
        }
        const auto [symbol, gradient2] = symbol_and_gradient(L, {a, b, c});
        orbit.symbol = symbol;
        orbit.weight = gradient2 / symbol;
        const long double denominator = std::sqrt(1.0L - c2 * symbol / 4.0L);
        orbit.pulse_envelope = 2.0L / denominator;
        orbit.step_envelope = 1.0L + 1.0L / denominator;
        auto& count = observed_counts[orbit.exact_key];
        ++count.first;
        count.second += static_cast<int>(orbit.members.size());
        for (const Index3& member : orbit.members) {
          exact_parent_partition = exact_parent_partition
              && serialize_key(exact_symbol_key(
                     L, {member.x, member.y, member.z}, phi))
                  == orbit.exact_key;
        }
        orbits.push_back(std::move(orbit));
      }
    }
  }
  exact_parent_partition = exact_parent_partition
      && observed_counts == expected_counts;
  return orbits;
}

long double orbit_character(
    const ModeOrbit& orbit, const Index3& displacement,
    const std::vector<std::vector<long double>>& signed_cosine) {
  CompensatedSum sum;
  for (const auto& permutation : orbit.magnitude_permutations) {
    sum.add(
        signed_cosine[static_cast<std::size_t>(permutation[0])]
                     [static_cast<std::size_t>(displacement.x)]
        * signed_cosine[static_cast<std::size_t>(permutation[1])]
                       [static_cast<std::size_t>(displacement.y)]
        * signed_cosine[static_cast<std::size_t>(permutation[2])]
                       [static_cast<std::size_t>(displacement.z)]);
  }
  return sum.value;
}

long double direct_orbit_character(
    int L, const ModeOrbit& orbit, const Index3& displacement,
    long double& imaginary_residual) {
  const long double pi = std::acos(-1.0L);
  const long double scale = 2.0L * pi / static_cast<long double>(L);
  CompensatedSum real;
  CompensatedSum imaginary;
  for (const Index3& mode : orbit.members) {
    const int phase_index = modulo(
        mode.x * displacement.x + mode.y * displacement.y
        + mode.z * displacement.z, L);
    const long double phase = scale * phase_index;
    real.add(std::cos(phase));
    imaginary.add(-std::sin(phase));
  }
  imaginary_residual = std::max(imaginary_residual,
                                std::abs(imaginary.value));
  return real.value;
}

long double shell_kernel(
    const std::vector<ModeOrbit>& orbits, int shell_count,
    const Index3& displacement,
    const std::vector<std::vector<long double>>& signed_cosine,
    long double weight_sum) {
  std::vector<CompensatedSum> shells(static_cast<std::size_t>(shell_count));
  for (const ModeOrbit& orbit : orbits) {
    shells[static_cast<std::size_t>(orbit.shell_index)].add(
        orbit.weight * orbit_character(orbit, displacement, signed_cosine));
  }
  CompensatedSum numerator;
  for (const CompensatedSum& shell : shells) {
    numerator.add(std::abs(shell.value));
  }
  return numerator.value / weight_sum;
}

long double direct_shell_kernel(
    int L, const std::vector<ModeOrbit>& orbits, int shell_count,
    const Index3& displacement, long double weight_sum,
    long double& imaginary_residual) {
  std::vector<CompensatedSum> shells(static_cast<std::size_t>(shell_count));
  for (const ModeOrbit& orbit : orbits) {
    shells[static_cast<std::size_t>(orbit.shell_index)].add(
        orbit.weight * direct_orbit_character(
            L, orbit, displacement, imaginary_residual));
  }
  CompensatedSum numerator;
  for (const CompensatedSum& shell : shells) {
    numerator.add(std::abs(shell.value));
  }
  return numerator.value / weight_sum;
}

int encode(int L, const Index3& point) {
  return point.x + L * (point.y + L * point.z);
}

Index3 decode(int L, int value) {
  Index3 point;
  point.x = value % L;
  value /= L;
  point.y = value % L;
  point.z = value / L;
  return point;
}

const std::array<Index3, 6> AXIAL{{
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
    {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};

std::vector<int> canonicalize(int L, const std::vector<int>& encoded) {
  std::vector<Index3> points;
  points.reserve(encoded.size());
  for (int value : encoded) points.push_back(decode(L, value));
  const std::vector<std::array<int, 3>> coordinate_permutations =
      unique_permutations({0, 1, 2});
  std::vector<int> best;
  bool first = true;
  for (const auto& permutation : coordinate_permutations) {
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          std::vector<Index3> transformed;
          transformed.reserve(points.size());
          for (const Index3& point : points) {
            const std::array<int, 3> coordinate{{point.x, point.y, point.z}};
            transformed.push_back({
                modulo(sx * coordinate[static_cast<std::size_t>(
                    permutation[0])], L),
                modulo(sy * coordinate[static_cast<std::size_t>(
                    permutation[1])], L),
                modulo(sz * coordinate[static_cast<std::size_t>(
                    permutation[2])], L)});
          }
          for (const Index3& origin : transformed) {
            std::vector<int> candidate;
            candidate.reserve(transformed.size());
            for (const Index3& point : transformed) {
              candidate.push_back(encode(L, {
                  modulo(point.x - origin.x, L),
                  modulo(point.y - origin.y, L),
                  modulo(point.z - origin.z, L)}));
            }
            std::sort(candidate.begin(), candidate.end());
            if (first || candidate < best) {
              best = std::move(candidate);
              first = false;
            }
          }
        }
      }
    }
  }
  return best;
}

int axial_edge_count(int L, const std::vector<int>& animal) {
  int edges = 0;
  for (int value : animal) {
    const Index3 point = decode(L, value);
    for (const Index3& step : std::array<Index3, 3>{{
             {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}) {
      const int neighbor = encode(L, {
          modulo(point.x + step.x, L),
          modulo(point.y + step.y, L),
          modulo(point.z + step.z, L)});
      if (std::binary_search(animal.begin(), animal.end(), neighbor)) ++edges;
    }
  }
  return edges;
}

CubicAnimalCapacityRecord enumerate_animals(int L) {
  CubicAnimalCapacityRecord result;
  result.lattice_size = L;
  std::set<std::vector<int>> current;
  current.insert(std::vector<int>{0});
  result.canonical_animal_counts[1] = 1;
  result.maximum_axial_edges[1] = 0;
  for (int size = 1; size < 9; ++size) {
    std::set<std::vector<int>> next;
    for (const std::vector<int>& animal : current) {
      for (int value : animal) {
        const Index3 point = decode(L, value);
        for (const Index3& step : AXIAL) {
          const int neighbor = encode(L, {
              modulo(point.x + step.x, L),
              modulo(point.y + step.y, L),
              modulo(point.z + step.z, L)});
          if (std::binary_search(animal.begin(), animal.end(), neighbor)) {
            continue;
          }
          std::vector<int> grown = animal;
          grown.push_back(neighbor);
          std::sort(grown.begin(), grown.end());
          next.insert(canonicalize(L, grown));
        }
      }
    }
    current = std::move(next);
    result.canonical_animal_counts[static_cast<std::size_t>(size + 1)] =
        current.size();
    int maximum_edges = 0;
    for (const std::vector<int>& animal : current) {
      maximum_edges = std::max(maximum_edges, axial_edge_count(L, animal));
    }
    result.maximum_axial_edges[static_cast<std::size_t>(size + 1)] =
        maximum_edges;
  }
  result.connected_growth_complete = !current.empty();
  result.valid = result.connected_growth_complete
      && result.canonical_animal_counts[1] == 1;
  return result;
}

TenSourcePairDistanceCapacityVolume analyze_volume(
    int L, const TenSourceSharedMCoherenceVolume& parent,
    const CubicAnimalCapacityRecord& animals) {
  TenSourcePairDistanceCapacityVolume result;
  result.parent = parent;
  std::map<int, Polynomial> cache;
  const Polynomial phi = cyclotomic(L, cache);
  bool exact_parent_partition = false;
  const std::vector<ModeOrbit> orbits = build_orbits(
      L, phi, parent, exact_parent_partition);
  result.exact_parent_shell_partition = exact_parent_partition;

  CompensatedSum weight_accumulator;
  CompensatedSum pulse_accumulator;
  CompensatedSum step_accumulator;
  for (const ModeOrbit& orbit : orbits) {
    const long double multiplicity = orbit.members.size();
    weight_accumulator.add(multiplicity * orbit.weight);
    pulse_accumulator.add(multiplicity * orbit.pulse_envelope
                          * orbit.pulse_envelope / orbit.symbol);
    step_accumulator.add(multiplicity * orbit.step_envelope
                         * orbit.step_envelope / orbit.symbol);
  }
  const long double weight_sum = weight_accumulator.value;
  const long double volume = static_cast<long double>(L) * L * L;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  const long double pulse_operator = static_cast<long double>(G_C)
      / (c2 * volume) * std::sqrt(pulse_accumulator.value * weight_sum);
  const long double common_step = static_cast<long double>(G_C) / c2
      * std::sqrt(step_accumulator.value / volume);

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

  const long double axial = shell_kernel(
      orbits, parent.eigenvalue_shell_count, {0, 0, 1},
      signed_cosine, weight_sum);
  long double second = -1.0L;
  Index3 second_displacement{};
  int displacement_count = 0;
  for (int dx = 0; dx <= half; ++dx) {
    for (int dy = dx; dy <= half; ++dy) {
      for (int dz = dy; dz <= half; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++displacement_count;
        if (dx == 0 && dy == 0 && dz == 1) continue;
        const long double value = shell_kernel(
            orbits, parent.eigenvalue_shell_count, {dx, dy, dz},
            signed_cosine, weight_sum);
        if (value > second) {
          second = value;
          second_displacement = {dx, dy, dz};
        }
      }
    }
  }
  const int expected_displacements = static_cast<int>(orbits.size());
  result.exact_displacement_coverage = displacement_count
      == expected_displacements;
  result.second_kernel_dx = second_displacement.x;
  result.second_kernel_dy = second_displacement.y;
  result.second_kernel_dz = second_displacement.z;

  long double covariance_residual = 0.0L;
  long double covariance_imaginary_residual = 0.0L;
  for (const Index3& displacement : std::array<Index3, 6>{{
           {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
           {-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}}) {
    covariance_residual = std::max(
        covariance_residual,
        std::abs(direct_shell_kernel(
            L, orbits, parent.eigenvalue_shell_count, displacement,
            weight_sum, covariance_imaginary_residual) - axial));
  }
  covariance_residual = std::max(
      covariance_residual, covariance_imaginary_residual);

  long double direct_residual = 0.0L;
  for (const Index3& displacement :
       std::array<Index3, 2>{{{0, 0, 1}, second_displacement}}) {
    std::vector<CompensatedSum> direct_shells(parent.shells.size());
    long double imaginary_residual = 0.0L;
    for (const ModeOrbit& orbit : orbits) {
      direct_shells[static_cast<std::size_t>(orbit.shell_index)].add(
          orbit.weight * direct_orbit_character(
              L, orbit, displacement, imaginary_residual));
    }
    CompensatedSum direct_numerator;
    for (const CompensatedSum& shell : direct_shells) {
      direct_numerator.add(std::abs(shell.value));
    }
    const long double formula = shell_kernel(
        orbits, parent.eigenvalue_shell_count, displacement,
        signed_cosine, weight_sum);
    direct_residual = std::max(
        direct_residual,
        std::max(std::abs(formula - direct_numerator.value / weight_sum),
                 imaginary_residual / weight_sum));
  }

  result.axial_kernel = static_cast<double>(axial);
  result.second_kernel = static_cast<double>(second);
  result.axial_covariance_residual = static_cast<double>(covariance_residual);
  result.direct_kernel_residual = static_cast<double>(direct_residual);
  result.axial_kernel_is_maximal = axial >= second - KERNEL_TOL;
  result.cubic_covariance_verified = covariance_residual <= KERNEL_TOL;
  result.direct_kernel_verified = direct_residual <= KERNEL_TOL;

  for (int removed = 0; removed <= 9; ++removed) {
    const int edge_cap = animals.maximum_axial_edges[
        static_cast<std::size_t>(removed)];
    result.axial_edge_caps[static_cast<std::size_t>(removed)] = edge_cap;
    const long double pairs =
        static_cast<long double>(removed) * (removed - 1) / 2.0L;
    const long double gram = static_cast<long double>(removed)
        + 2.0L * (edge_cap * axial + (pairs - edge_cap) * second);
    result.pair_gram_factors[static_cast<std::size_t>(removed)] =
        static_cast<double>(gram);
    const long double bound = common_step
        * std::sqrt(static_cast<long double>(10 - removed))
        + pulse_operator * std::sqrt(std::max(0.0L, gram));
    result.pair_partition_bounds[static_cast<std::size_t>(removed)] =
        static_cast<double>(bound);
  }
  result.pair_partition_bounds[10] = parent.removal_partition_bounds[10];

  long double maximum_bound = -std::numeric_limits<long double>::infinity();
  int maximizing_removed = -1;
  bool all_finite = true;
  bool no_weaker = true;
  for (int removed = 0; removed <= 10; ++removed) {
    const long double bound = result.pair_partition_bounds[
        static_cast<std::size_t>(removed)];
    all_finite = all_finite && std::isfinite(static_cast<double>(bound));
    no_weaker = no_weaker && bound
        <= static_cast<long double>(parent.removal_partition_bounds[
               static_cast<std::size_t>(removed)]) + CROSS_TOL;
    if (bound > maximum_bound) {
      maximum_bound = bound;
      maximizing_removed = removed;
    }
  }
  result.maximizing_removed_count = maximizing_removed;
  result.pair_distance_bound = static_cast<double>(maximum_bound);
  result.pair_distance_margin = K_GENESIS - result.pair_distance_bound;
  result.pair_bound_no_weaker = no_weaker;
  result.all_partition_bounds_finite = all_finite;
  result.ten_source_closed = maximum_bound
      < static_cast<long double>(K_GENESIS);
  result.valid = result.exact_parent_shell_partition
      && result.exact_displacement_coverage
      && result.axial_kernel_is_maximal
      && result.cubic_covariance_verified
      && result.direct_kernel_verified
      && result.pair_bound_no_weaker
      && result.all_partition_bounds_finite
      && animals.valid
      && maximizing_removed >= 0 && maximizing_removed <= 10;
  return result;
}

}  // namespace

TenSourcePairDistanceCapacityResult
analyze_ten_source_pair_distance_capacity() {
  TenSourcePairDistanceCapacityResult result;
  const TenSourceSharedMCoherenceResult parent =
      analyze_ten_source_shared_m_coherence();
  result.animals_l9 = enumerate_animals(9);
  result.animals_l17 = enumerate_animals(17);
  result.exact_animal_enumeration_complete =
      result.animals_l9.valid && result.animals_l17.valid;

  bool all_valid = parent.valid && result.exact_animal_enumeration_complete;
  bool all_kernels = true;
  bool all_closed = true;
  result.volumes.reserve(VOLUMES.size());
  for (std::size_t i = 0; i < VOLUMES.size(); ++i) {
    const CubicAnimalCapacityRecord& animals =
        VOLUMES[i] == 9 ? result.animals_l9 : result.animals_l17;
    TenSourcePairDistanceCapacityVolume volume = analyze_volume(
        VOLUMES[i], parent.volumes[i], animals);
    all_valid = all_valid && volume.valid;
    all_kernels = all_kernels && volume.exact_parent_shell_partition
        && volume.exact_displacement_coverage
        && volume.axial_kernel_is_maximal
        && volume.cubic_covariance_verified
        && volume.direct_kernel_verified
        && volume.pair_bound_no_weaker;
    all_closed = all_closed && volume.ten_source_closed;
    result.volumes.push_back(std::move(volume));
    ++result.spectral_volume_count;
  }
  result.pair_distance_bound_derived = all_valid;
  result.all_kernel_checks_pass = all_kernels;
  result.arbitrary_removal_n_le_ten_closed = all_valid && all_closed;
  result.ten_source_pair_distance_bound_inconclusive =
      all_valid && !all_closed;
  result.valid = all_valid && result.spectral_volume_count == 4
      && (result.arbitrary_removal_n_le_ten_closed
          || result.ten_source_pair_distance_bound_inconclusive);
  return result;
}

}  // namespace ftd::eft
