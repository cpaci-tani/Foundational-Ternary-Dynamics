#include "ftd/eft/ten_source_shared_m_coherence.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
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

constexpr long double ORBIT_TOL = 5e-14L;
constexpr long double CHARACTER_TOL = 5e-13L;
constexpr long double SHELL_TOL = 5e-13L;
constexpr long double CROSS_TOL = 5e-12L;
constexpr std::array<int, 4> VOLUMES{{9, 17, 33, 65}};

class BigInt {
 public:
  BigInt() = default;
  BigInt(long long value) { assign(value); }

  bool is_zero() const { return sign_ == 0; }

  std::string str() const {
    if (is_zero()) return "0";
    std::ostringstream out;
    if (sign_ < 0) out << '-';
    out << limbs_.back();
    for (std::size_t i = limbs_.size() - 1; i-- > 0;) {
      out << std::setw(4) << std::setfill('0') << limbs_[i];
    }
    return out.str();
  }

  friend bool operator==(const BigInt& lhs, const BigInt& rhs) {
    return lhs.sign_ == rhs.sign_ && lhs.limbs_ == rhs.limbs_;
  }
  friend bool operator!=(const BigInt& lhs, const BigInt& rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const BigInt& lhs, const BigInt& rhs) {
    if (lhs.sign_ != rhs.sign_) return lhs.sign_ < rhs.sign_;
    if (lhs.sign_ == 0) return false;
    const int magnitude = compare_abs(lhs, rhs);
    return lhs.sign_ > 0 ? magnitude < 0 : magnitude > 0;
  }

  BigInt operator-() const {
    BigInt result = *this;
    result.sign_ = -result.sign_;
    return result;
  }

  BigInt& operator+=(const BigInt& other) {
    if (other.sign_ == 0) return *this;
    if (sign_ == 0) {
      *this = other;
      return *this;
    }
    if (sign_ == other.sign_) {
      add_abs(other);
      return *this;
    }
    const int comparison = compare_abs(*this, other);
    if (comparison == 0) {
      limbs_.clear();
      sign_ = 0;
    } else if (comparison > 0) {
      subtract_abs(other);
    } else {
      BigInt result = other;
      result.subtract_abs(*this);
      *this = std::move(result);
    }
    return *this;
  }

  BigInt& operator-=(const BigInt& other) {
    return *this += -other;
  }

  friend BigInt operator+(BigInt lhs, const BigInt& rhs) {
    lhs += rhs;
    return lhs;
  }
  friend BigInt operator-(BigInt lhs, const BigInt& rhs) {
    lhs -= rhs;
    return lhs;
  }
  friend BigInt operator*(const BigInt& lhs, const BigInt& rhs) {
    if (lhs.is_zero() || rhs.is_zero()) return BigInt{};
    BigInt result;
    result.sign_ = lhs.sign_ * rhs.sign_;
    result.limbs_.assign(lhs.limbs_.size() + rhs.limbs_.size(), 0u);
    for (std::size_t i = 0; i < lhs.limbs_.size(); ++i) {
      std::uint64_t carry = 0;
      for (std::size_t j = 0; j < rhs.limbs_.size() || carry != 0; ++j) {
        const std::uint64_t product = j < rhs.limbs_.size()
            ? static_cast<std::uint64_t>(lhs.limbs_[i]) * rhs.limbs_[j]
            : 0u;
        const std::uint64_t current =
            result.limbs_[i + j] + product + carry;
        result.limbs_[i + j] = static_cast<std::uint32_t>(current % BASE);
        carry = current / BASE;
      }
    }
    result.normalize();
    return result;
  }

 private:
  static constexpr std::uint32_t BASE = 10000u;
  int sign_ = 0;
  std::vector<std::uint32_t> limbs_;

  void assign(long long value) {
    limbs_.clear();
    if (value == 0) {
      sign_ = 0;
      return;
    }
    sign_ = value < 0 ? -1 : 1;
    std::uint64_t magnitude = value < 0
        ? static_cast<std::uint64_t>(-(value + 1)) + 1u
        : static_cast<std::uint64_t>(value);
    while (magnitude != 0u) {
      limbs_.push_back(static_cast<std::uint32_t>(magnitude % BASE));
      magnitude /= BASE;
    }
  }

  void normalize() {
    while (!limbs_.empty() && limbs_.back() == 0u) limbs_.pop_back();
    if (limbs_.empty()) sign_ = 0;
  }

  static int compare_abs(const BigInt& lhs, const BigInt& rhs) {
    if (lhs.limbs_.size() != rhs.limbs_.size()) {
      return lhs.limbs_.size() < rhs.limbs_.size() ? -1 : 1;
    }
    for (std::size_t i = lhs.limbs_.size(); i-- > 0;) {
      if (lhs.limbs_[i] != rhs.limbs_[i]) {
        return lhs.limbs_[i] < rhs.limbs_[i] ? -1 : 1;
      }
    }
    return 0;
  }

  void add_abs(const BigInt& other) {
    const std::size_t size = std::max(limbs_.size(), other.limbs_.size());
    limbs_.resize(size, 0u);
    std::uint32_t carry = 0u;
    for (std::size_t i = 0; i < size || carry != 0u; ++i) {
      if (i == limbs_.size()) limbs_.push_back(0u);
      const std::uint32_t rhs = i < other.limbs_.size()
          ? other.limbs_[i] : 0u;
      const std::uint32_t current = limbs_[i] + rhs + carry;
      limbs_[i] = current % BASE;
      carry = current / BASE;
    }
  }

  void subtract_abs(const BigInt& other) {
    std::int32_t borrow = 0;
    for (std::size_t i = 0; i < limbs_.size(); ++i) {
      const std::int32_t rhs = i < other.limbs_.size()
          ? static_cast<std::int32_t>(other.limbs_[i]) : 0;
      std::int32_t current = static_cast<std::int32_t>(limbs_[i])
          - rhs - borrow;
      if (current < 0) {
        current += static_cast<std::int32_t>(BASE);
        borrow = 1;
      } else {
        borrow = 0;
      }
      limbs_[i] = static_cast<std::uint32_t>(current);
    }
    normalize();
  }
};

using Polynomial = std::vector<BigInt>;
using EigenKey = std::vector<BigInt>;

void trim(Polynomial& value) {
  while (!value.empty() && value.back().is_zero()) value.pop_back();
}

Polynomial multiply(const Polynomial& lhs, const Polynomial& rhs) {
  if (lhs.empty() || rhs.empty()) return {};
  Polynomial result(lhs.size() + rhs.size() - 1, BigInt{});
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    for (std::size_t j = 0; j < rhs.size(); ++j) {
      result[i + j] += lhs[i] * rhs[j];
    }
  }
  trim(result);
  return result;
}

Polynomial divide_exact_monic(Polynomial numerator,
                              const Polynomial& denominator) {
  trim(numerator);
  if (denominator.empty() || denominator.back() != BigInt{1}
      || numerator.size() < denominator.size()) {
    throw std::runtime_error("invalid exact monic division");
  }
  Polynomial quotient(
      numerator.size() - denominator.size() + 1, BigInt{});
  while (numerator.size() >= denominator.size()) {
    const std::size_t shift = numerator.size() - denominator.size();
    const BigInt coefficient = numerator.back();
    quotient[shift] = coefficient;
    for (std::size_t j = 0; j < denominator.size(); ++j) {
      numerator[shift + j] -= coefficient * denominator[j];
    }
    trim(numerator);
  }
  if (!numerator.empty()) throw std::runtime_error("non-exact division");
  trim(quotient);
  return quotient;
}

std::vector<int> divisors(int n) {
  std::vector<int> result;
  for (int d = 1; d <= n; ++d) {
    if (n % d == 0) result.push_back(d);
  }
  return result;
}

const Polynomial& cyclotomic(int n, std::map<int, Polynomial>& cache) {
  const auto found = cache.find(n);
  if (found != cache.end()) return found->second;
  Polynomial value(static_cast<std::size_t>(n + 1), BigInt{});
  value[0] = BigInt{-1};
  value[static_cast<std::size_t>(n)] = BigInt{1};
  for (int d : divisors(n)) {
    if (d == n) continue;
    value = divide_exact_monic(value, cyclotomic(d, cache));
  }
  return cache.emplace(n, std::move(value)).first->second;
}

bool cyclotomic_identity(int L, std::map<int, Polynomial>& cache) {
  Polynomial product{BigInt{1}};
  for (int d : divisors(L)) product = multiply(product, cyclotomic(d, cache));
  Polynomial expected(static_cast<std::size_t>(L + 1), BigInt{});
  expected[0] = BigInt{-1};
  expected[static_cast<std::size_t>(L)] = BigInt{1};
  trim(product);
  trim(expected);
  return product == expected;
}

int modulo(int value, int L) {
  const int result = value % L;
  return result < 0 ? result + L : result;
}

EigenKey exact_symbol_key(int L, const std::array<int, 3>& mode,
                          const Polynomial& phi) {
  Polynomial value(static_cast<std::size_t>(L), BigInt{});
  auto add = [&](int exponent, long long coefficient) {
    value[static_cast<std::size_t>(modulo(exponent, L))] +=
        BigInt{coefficient};
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
    const BigInt coefficient = value[index];
    if (coefficient.is_zero()) continue;
    const std::size_t shift = index - degree;
    for (std::size_t j = 0; j < phi.size(); ++j) {
      value[shift + j] -= coefficient * phi[j];
    }
  }
  value.resize(degree);
  return value;
}

std::string serialize_key(const EigenKey& key) {
  std::ostringstream out;
  for (std::size_t i = 0; i < key.size(); ++i) {
    if (i != 0) out << ':';
    out << key[i].str();
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
  EigenKey exact_key;
  int shell_index = -1;
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
  return {
      4.0L - (2.0L / 3.0L) * (cx + cy + cz)
          - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz),
      sx * sx + sy * sy + sz * sz};
}

std::vector<ModeOrbit> build_orbits(
    int L, const Polynomial& phi, bool& exact_coverage,
    bool& exact_key_invariance, long double& invariance_residual) {
  const int half = L / 2;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  std::vector<ModeOrbit> orbits;
  std::vector<unsigned char> covered(
      static_cast<std::size_t>(L) * L * L, 0u);
  covered[0] = 1u;
  bool duplicate = false;
  int covered_nonzero = 0;
  exact_key_invariance = true;

  for (int a = 0; a <= half; ++a) {
    for (int b = a; b <= half; ++b) {
      for (int c = b; c <= half; ++c) {
        if (a == 0 && b == 0 && c == 0) continue;
        ModeOrbit orbit;
        orbit.representative = {a, b, c};
        orbit.magnitude_permutations = unique_permutations({a, b, c});
        orbit.members = orbit_members(L, {a, b, c});
        orbit.exact_key = exact_symbol_key(L, orbit.representative, phi);
        const auto [symbol, gradient2] = symbol_and_gradient(L, {a, b, c});
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
          exact_key_invariance = exact_key_invariance
              && exact_symbol_key(
                     L, {member.x, member.y, member.z}, phi)
                  == orbit.exact_key;
        }
        orbits.push_back(std::move(orbit));
      }
    }
  }
  exact_coverage = !duplicate && covered_nonzero == L * L * L - 1;
  for (unsigned char value : covered) exact_coverage = exact_coverage
      && value != 0u;
  return orbits;
}

long double orbit_character(
    const ModeOrbit& orbit, const Index3& displacement,
    const std::vector<std::vector<long double>>& signed_cosine) {
  CompensatedSum character;
  for (const auto& permutation : orbit.magnitude_permutations) {
    character.add(
        signed_cosine[static_cast<std::size_t>(permutation[0])]
                     [static_cast<std::size_t>(displacement.x)]
        * signed_cosine[static_cast<std::size_t>(permutation[1])]
                       [static_cast<std::size_t>(displacement.y)]
        * signed_cosine[static_cast<std::size_t>(permutation[2])]
                       [static_cast<std::size_t>(displacement.z)]);
  }
  return character.value;
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
    const long double phase = scale * static_cast<long double>(phase_index);
    real.add(std::cos(phase));
    imaginary.add(-std::sin(phase));
  }
  imaginary_residual = std::max(imaginary_residual,
                                std::abs(imaginary.value));
  return real.value;
}

TenSourceSharedMCoherenceVolume analyze_volume(
    int L, const RemovalTimeOrbitCoherenceVolume& parent) {
  TenSourceSharedMCoherenceVolume result;
  result.parent = parent;
  std::map<int, Polynomial> cyclotomic_cache;
  const Polynomial phi = cyclotomic(L, cyclotomic_cache);
  result.cyclotomic_degree = static_cast<int>(phi.size()) - 1;
  result.cyclotomic_identity_exact =
      cyclotomic_identity(L, cyclotomic_cache);

  bool exact_coverage = false;
  bool exact_key_invariance = false;
  long double invariance_residual = 0.0L;
  std::vector<ModeOrbit> orbits = build_orbits(
      L, phi, exact_coverage, exact_key_invariance, invariance_residual);
  result.exact_orbit_coverage = exact_coverage;
  result.exact_key_invariance = exact_key_invariance;
  result.maximum_orbit_invariance_residual =
      static_cast<double>(invariance_residual);

  std::map<EigenKey, std::vector<int>> groups;
  for (std::size_t i = 0; i < orbits.size(); ++i) {
    groups[orbits[i].exact_key].push_back(static_cast<int>(i));
  }
  result.shells.reserve(groups.size());
  int shell_index = 0;
  int shell_mode_count = 0;
  for (const auto& [key, indices] : groups) {
    SharedMEigenshellRecord record;
    record.exact_key = serialize_key(key);
    record.orbit_count = static_cast<int>(indices.size());
    for (int index : indices) {
      orbits[static_cast<std::size_t>(index)].shell_index = shell_index;
      record.mode_count += static_cast<int>(
          orbits[static_cast<std::size_t>(index)].members.size());
    }
    shell_mode_count += record.mode_count;
    if (record.orbit_count > 1) ++result.multi_orbit_shell_count;
    result.maximum_orbits_per_shell = std::max(
        result.maximum_orbits_per_shell, record.orbit_count);
    result.shells.push_back(std::move(record));
    ++shell_index;
  }
  result.eigenvalue_shell_count = static_cast<int>(result.shells.size());
  result.shell_mode_count = shell_mode_count;
  result.exact_shell_coverage = shell_mode_count == L * L * L - 1;

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

  long double maximum_shared_numerator = -1.0L;
  long double maximum_orbit_numerator = -1.0L;
  Index3 maximizing{};
  for (int dx = 0; dx <= half; ++dx) {
    for (int dy = dx; dy <= half; ++dy) {
      for (int dz = dy; dz <= half; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const Index3 displacement{dx, dy, dz};
        std::vector<CompensatedSum> shell_sums(result.shells.size());
        CompensatedSum orbit_numerator;
        for (const ModeOrbit& orbit : orbits) {
          const long double contribution = orbit.weight
              * orbit_character(orbit, displacement, signed_cosine);
          shell_sums[static_cast<std::size_t>(orbit.shell_index)].add(
              contribution);
          orbit_numerator.add(std::abs(contribution));
        }
        CompensatedSum shared_numerator;
        for (const CompensatedSum& shell : shell_sums) {
          shared_numerator.add(std::abs(shell.value));
        }
        maximum_orbit_numerator = std::max(
            maximum_orbit_numerator, orbit_numerator.value);
        if (shared_numerator.value > maximum_shared_numerator) {
          maximum_shared_numerator = shared_numerator.value;
          maximizing = displacement;
        }
      }
    }
  }
  result.maximizing_dx = maximizing.x;
  result.maximizing_dy = maximizing.y;
  result.maximizing_dz = maximizing.z;

  long double character_residual = 0.0L;
  long double imaginary_residual = 0.0L;
  std::vector<CompensatedSum> direct_shell_sums(result.shells.size());
  for (const ModeOrbit& orbit : orbits) {
    const long double formula =
        orbit_character(orbit, maximizing, signed_cosine);
    const long double direct =
        direct_orbit_character(L, orbit, maximizing, imaginary_residual);
    character_residual = std::max(character_residual,
                                  std::abs(formula - direct));
    direct_shell_sums[static_cast<std::size_t>(orbit.shell_index)].add(
        orbit.weight * direct);
  }
  CompensatedSum direct_shared_numerator;
  for (const CompensatedSum& shell : direct_shell_sums) {
    direct_shared_numerator.add(std::abs(shell.value));
  }
  const long double shell_residual =
      std::abs(maximum_shared_numerator - direct_shared_numerator.value)
      / weight_sum;
  character_residual = std::max(character_residual, imaginary_residual);

  const long double shared_coherence = maximum_shared_numerator / weight_sum;
  const long double orbit_coherence = maximum_orbit_numerator / weight_sum;
  const long double volume = static_cast<long double>(L) * L * L;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  const long double coupling = static_cast<long double>(G_C);
  const long double pulse_operator =
      coupling / (c2 * volume) * std::sqrt(pulse_sum * weight_sum);
  const long double common_step =
      coupling / c2 * std::sqrt(common_step_sum / volume);

  long double maximum_bound = -std::numeric_limits<long double>::infinity();
  int maximizing_removed = -1;
  bool all_finite = true;
  for (int removed = 0; removed <= 10; ++removed) {
    const long double remaining = common_step
        * std::sqrt(static_cast<long double>(10 - removed));
    const long double removed_factor = static_cast<long double>(removed)
        + shared_coherence * removed * (removed - 1);
    const long double pulse = pulse_operator
        * std::sqrt(std::max(0.0L, removed_factor));
    const long double bound = remaining + pulse;
    result.removal_partition_bounds[static_cast<std::size_t>(removed)] =
        static_cast<double>(bound);
    all_finite = all_finite && std::isfinite(static_cast<double>(bound));
    if (bound > maximum_bound) {
      maximum_bound = bound;
      maximizing_removed = removed;
    }
  }

  result.maximizing_removed_count = maximizing_removed;
  result.maximum_shared_m_coherence = static_cast<double>(shared_coherence);
  result.orbit_coherence_recomputed = static_cast<double>(orbit_coherence);
  result.coherence_improvement = static_cast<double>(
      orbit_coherence - shared_coherence);
  result.pulse_operator_coefficient = static_cast<double>(pulse_operator);
  result.common_step_coefficient = static_cast<double>(common_step);
  result.ten_source_shared_m_bound = static_cast<double>(maximum_bound);
  result.ten_source_margin = K_GENESIS - result.ten_source_shared_m_bound;
  result.maximum_character_residual = static_cast<double>(character_residual);
  result.shell_regrouping_residual = static_cast<double>(shell_residual);
  result.direct_character_verified = character_residual <= CHARACTER_TOL;
  result.shell_regrouping_verified = shell_residual <= SHELL_TOL;
  result.shared_m_no_weaker = shared_coherence
      <= orbit_coherence + SHELL_TOL;
  result.parent_scalars_reproduced =
      std::abs(orbit_coherence - parent.maximum_orbit_coherence) <= CROSS_TOL
      && std::abs(pulse_operator - parent.pulse_operator_coefficient)
          <= CROSS_TOL
      && std::abs(common_step - parent.common_step_coefficient) <= CROSS_TOL;
  result.all_partition_bounds_finite = all_finite;
  result.ten_source_closed = maximum_bound
      < static_cast<long double>(K_GENESIS);
  result.valid = result.cyclotomic_identity_exact
      && result.exact_key_invariance
      && result.exact_shell_coverage
      && result.exact_orbit_coverage
      && invariance_residual <= ORBIT_TOL
      && result.direct_character_verified
      && result.shell_regrouping_verified
      && result.shared_m_no_weaker
      && result.parent_scalars_reproduced
      && result.all_partition_bounds_finite
      && maximizing_removed >= 0 && maximizing_removed <= 10
      && std::isfinite(result.maximum_shared_m_coherence)
      && std::isfinite(result.ten_source_margin);
  return result;
}

}  // namespace

TenSourceSharedMCoherenceResult analyze_ten_source_shared_m_coherence() {
  TenSourceSharedMCoherenceResult result;
  const RemovalTimeOrbitCoherenceResult parent =
      analyze_removal_time_orbit_coherence();
  bool all_valid = parent.valid;
  bool all_identities = true;
  bool all_partitions = true;
  bool all_cross_checks = true;
  bool all_closed = true;
  result.volumes.reserve(VOLUMES.size());
  for (std::size_t i = 0; i < VOLUMES.size(); ++i) {
    TenSourceSharedMCoherenceVolume volume = analyze_volume(
        VOLUMES[i], parent.volumes[i]);
    all_valid = all_valid && volume.valid;
    all_identities = all_identities && volume.cyclotomic_identity_exact;
    all_partitions = all_partitions && volume.exact_key_invariance
        && volume.exact_shell_coverage && volume.exact_orbit_coverage;
    all_cross_checks = all_cross_checks && volume.direct_character_verified
        && volume.shell_regrouping_verified && volume.shared_m_no_weaker
        && volume.parent_scalars_reproduced;
    all_closed = all_closed && volume.ten_source_closed;
    result.volumes.push_back(std::move(volume));
    ++result.spectral_volume_count;
  }
  result.exact_shared_m_bound_derived = all_valid;
  result.all_cyclotomic_identities_exact = all_identities;
  result.all_shell_partitions_exact = all_partitions;
  result.all_cross_checks_pass = all_cross_checks;
  result.arbitrary_removal_n_le_ten_closed = all_valid && all_closed;
  result.ten_source_shared_m_bound_inconclusive = all_valid && !all_closed;
  result.valid = all_valid && result.spectral_volume_count == 4
      && (result.arbitrary_removal_n_le_ten_closed
          || result.ten_source_shared_m_bound_inconclusive);
  return result;
}

}  // namespace ftd::eft
