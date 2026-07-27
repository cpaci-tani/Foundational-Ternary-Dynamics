#include "ftd/eft/ten_source_distance_distribution_lp.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
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

constexpr long double KERNEL_TOL = 5e-12L;
constexpr long double CHARACTER_TOL = 5e-13L;
constexpr long double FOURIER_TOL = 1e-10L;
constexpr long double SOLVER_TOL = 1e-10L;
constexpr long double GAP_TOL = 1e-8L;
constexpr long double DUAL_SIGN_TOL = 1e-12L;
constexpr long double DUAL_PAD_FLOOR = 1e-12L;
constexpr std::array<int, 4> VOLUMES{{9, 17, 33, 65}};
constexpr std::array<int, 10> EDGE_CAPS{{0, 0, 1, 2, 4, 5, 7, 9, 12, 13}};

using Polynomial = std::vector<long long>;
using ExactKey = std::vector<long long>;

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

void trim(Polynomial& value) {
  while (!value.empty() && value.back() == 0) value.pop_back();
}

Polynomial divide_exact_monic(Polynomial numerator,
                              const Polynomial& denominator) {
  trim(numerator);
  Polynomial quotient(numerator.size() - denominator.size() + 1, 0);
  while (numerator.size() >= denominator.size()) {
    const std::size_t shift = numerator.size() - denominator.size();
    const long long coefficient = numerator.back();
    quotient[shift] = coefficient;
    for (std::size_t index = 0; index < denominator.size(); ++index) {
      numerator[shift + index] -= coefficient * denominator[index];
    }
    trim(numerator);
  }
  if (!numerator.empty()) throw std::runtime_error("non-exact division");
  trim(quotient);
  return quotient;
}

std::vector<int> divisors(int value) {
  std::vector<int> out;
  for (int candidate = 1; candidate <= value; ++candidate) {
    if (value % candidate == 0) out.push_back(candidate);
  }
  return out;
}

const Polynomial& cyclotomic(int value, std::map<int, Polynomial>& cache) {
  const auto found = cache.find(value);
  if (found != cache.end()) return found->second;
  Polynomial polynomial(static_cast<std::size_t>(value + 1), 0);
  polynomial[0] = -1;
  polynomial[static_cast<std::size_t>(value)] = 1;
  for (int divisor : divisors(value)) {
    if (divisor != value) {
      polynomial = divide_exact_monic(
          polynomial, cyclotomic(divisor, cache));
    }
  }
  return cache.emplace(value, std::move(polynomial)).first->second;
}

int modulo(int value, int lattice_size) {
  const int result = value % lattice_size;
  return result < 0 ? result + lattice_size : result;
}

ExactKey exact_key(int lattice_size, const std::array<int, 3>& mode,
                   const Polynomial& phi) {
  Polynomial value(static_cast<std::size_t>(lattice_size), 0);
  auto add = [&](int exponent, long long coefficient) {
    value[static_cast<std::size_t>(modulo(exponent, lattice_size))]
        += coefficient;
  };
  add(0, 24);
  for (int component : mode) {
    add(component, -2);
    add(-component, -2);
  }
  for (int left = 0; left < 3; ++left) {
    for (int right = left + 1; right < 3; ++right) {
      for (int left_sign : {-1, 1}) {
        for (int right_sign : {-1, 1}) {
          add(left_sign * mode[static_cast<std::size_t>(left)]
              + right_sign * mode[static_cast<std::size_t>(right)], -1);
        }
      }
    }
  }
  const std::size_t degree = phi.size() - 1;
  for (std::size_t index = value.size(); index-- > degree;) {
    const long long coefficient = value[index];
    if (coefficient == 0) continue;
    const std::size_t shift = index - degree;
    for (std::size_t phi_index = 0; phi_index < phi.size(); ++phi_index) {
      value[shift + phi_index] -= coefficient * phi[phi_index];
    }
  }
  value.resize(degree);
  return value;
}

struct Index3 {
  int x = 0;
  int y = 0;
  int z = 0;
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

std::vector<Index3> orbit_members(int lattice_size,
                                  std::array<int, 3> value) {
  std::set<std::tuple<int, int, int>> unique;
  for (const auto& permutation : unique_permutations(value)) {
    for (int sx : {-1, 1}) {
      if (permutation[0] == 0 && sx < 0) continue;
      for (int sy : {-1, 1}) {
        if (permutation[1] == 0 && sy < 0) continue;
        for (int sz : {-1, 1}) {
          if (permutation[2] == 0 && sz < 0) continue;
          unique.emplace(
              modulo(sx * permutation[0], lattice_size),
              modulo(sy * permutation[1], lattice_size),
              modulo(sz * permutation[2], lattice_size));
        }
      }
    }
  }
  std::vector<Index3> out;
  out.reserve(unique.size());
  for (const auto& [x, y, z] : unique) out.push_back({x, y, z});
  return out;
}

struct Orbit {
  std::array<int, 3> representative{};
  std::vector<std::array<int, 3>> permutations;
  std::vector<Index3> members;
  int shell_index = -1;
  long double symbol = 0.0L;
  long double weight = 0.0L;
  long double pulse_envelope = 0.0L;
  long double step_envelope = 0.0L;
};

std::pair<long double, long double> symbol_and_gradient(
    int lattice_size, const std::array<int, 3>& mode) {
  const long double pi = std::acos(-1.0L);
  const long double scale = 2.0L * pi / lattice_size;
  const long double kx = scale * mode[0];
  const long double ky = scale * mode[1];
  const long double kz = scale * mode[2];
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

struct Scheme {
  int lattice_size = 0;
  int shell_count = 0;
  std::vector<Orbit> orbits;
  std::vector<std::vector<long double>> signed_cosine;
  std::vector<long double> kernel;
  long double weight_sum = 0.0L;
  long double pulse_operator = 0.0L;
  long double common_step = 0.0L;
  bool exact_coverage = false;
  bool exact_shell_partition = false;
};

long double orbit_character(
    const Orbit& orbit, const std::array<int, 3>& displacement,
    const std::vector<std::vector<long double>>& signed_cosine) {
  CompensatedSum sum;
  for (const auto& permutation : orbit.permutations) {
    sum.add(
        signed_cosine[static_cast<std::size_t>(permutation[0])]
                     [static_cast<std::size_t>(displacement[0])]
        * signed_cosine[static_cast<std::size_t>(permutation[1])]
                       [static_cast<std::size_t>(displacement[1])]
        * signed_cosine[static_cast<std::size_t>(permutation[2])]
                       [static_cast<std::size_t>(displacement[2])]);
  }
  return sum.value;
}

long double character_value(const Scheme& scheme, int momentum_orbit,
                            int displacement_orbit) {
  const Orbit& displacement = scheme.orbits[static_cast<std::size_t>(
      displacement_orbit)];
  return orbit_character(
             displacement,
             scheme.orbits[static_cast<std::size_t>(momentum_orbit)]
                 .representative,
             scheme.signed_cosine)
      / static_cast<long double>(displacement.members.size());
}

Scheme build_scheme(int lattice_size) {
  Scheme scheme;
  scheme.lattice_size = lattice_size;
  const int half = lattice_size / 2;
  std::map<int, Polynomial> cache;
  const Polynomial phi = cyclotomic(lattice_size, cache);
  std::vector<ExactKey> keys;
  std::set<std::tuple<int, int, int>> covered;
  bool disjoint = true;
  bool invariant = true;
  for (int a = 0; a <= half; ++a) {
    for (int b = a; b <= half; ++b) {
      for (int c = b; c <= half; ++c) {
        if (a == 0 && b == 0 && c == 0) continue;
        Orbit orbit;
        orbit.representative = {a, b, c};
        orbit.permutations = unique_permutations({a, b, c});
        orbit.members = orbit_members(lattice_size, {a, b, c});
        const ExactKey key = exact_key(lattice_size, {a, b, c}, phi);
        for (const Index3& member : orbit.members) {
          const auto inserted = covered.emplace(member.x, member.y, member.z);
          disjoint = disjoint && inserted.second;
          invariant = invariant && exact_key(
              lattice_size, {member.x, member.y, member.z}, phi) == key;
        }
        const auto [symbol, gradient2] = symbol_and_gradient(
            lattice_size, orbit.representative);
        orbit.symbol = symbol;
        orbit.weight = gradient2 / symbol;
        const long double denominator = std::sqrt(
            1.0L - static_cast<long double>(C_WAVE) * C_WAVE
                * symbol / 4.0L);
        orbit.pulse_envelope = 2.0L / denominator;
        orbit.step_envelope = 1.0L + 1.0L / denominator;
        scheme.orbits.push_back(std::move(orbit));
        keys.push_back(key);
      }
    }
  }
  scheme.exact_coverage = disjoint
      && covered.size() == static_cast<std::size_t>(
             lattice_size * lattice_size * lattice_size - 1);
  std::vector<ExactKey> unique_keys = keys;
  std::sort(unique_keys.begin(), unique_keys.end());
  unique_keys.erase(
      std::unique(unique_keys.begin(), unique_keys.end()), unique_keys.end());
  scheme.shell_count = static_cast<int>(unique_keys.size());
  for (std::size_t index = 0; index < scheme.orbits.size(); ++index) {
    scheme.orbits[index].shell_index = static_cast<int>(
        std::lower_bound(unique_keys.begin(), unique_keys.end(), keys[index])
        - unique_keys.begin());
  }
  scheme.exact_shell_partition = invariant;

  const long double pi = std::acos(-1.0L);
  scheme.signed_cosine.assign(
      static_cast<std::size_t>(half + 1),
      std::vector<long double>(static_cast<std::size_t>(half + 1), 0.0L));
  for (int magnitude = 0; magnitude <= half; ++magnitude) {
    for (int displacement = 0; displacement <= half; ++displacement) {
      scheme.signed_cosine[static_cast<std::size_t>(magnitude)]
                            [static_cast<std::size_t>(displacement)] =
          magnitude == 0 ? 1.0L
                         : 2.0L * std::cos(
                               2.0L * pi * magnitude * displacement
                               / lattice_size);
    }
  }

  CompensatedSum weights;
  CompensatedSum pulses;
  CompensatedSum steps;
  for (const Orbit& orbit : scheme.orbits) {
    const long double multiplicity = orbit.members.size();
    weights.add(multiplicity * orbit.weight);
    pulses.add(multiplicity * orbit.pulse_envelope * orbit.pulse_envelope
               / orbit.symbol);
    steps.add(multiplicity * orbit.step_envelope * orbit.step_envelope
              / orbit.symbol);
  }
  scheme.weight_sum = weights.value;
  const long double volume = static_cast<long double>(lattice_size)
      * lattice_size * lattice_size;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  scheme.pulse_operator = static_cast<long double>(G_C) / (c2 * volume)
      * std::sqrt(pulses.value * weights.value);
  scheme.common_step = static_cast<long double>(G_C) / c2
      * std::sqrt(steps.value / volume);

  scheme.kernel.resize(scheme.orbits.size(), 0.0L);
  for (std::size_t displacement = 0;
       displacement < scheme.orbits.size(); ++displacement) {
    std::vector<CompensatedSum> shells(
        static_cast<std::size_t>(scheme.shell_count));
    for (const Orbit& orbit : scheme.orbits) {
      shells[static_cast<std::size_t>(orbit.shell_index)].add(
          orbit.weight * orbit_character(
              orbit, scheme.orbits[displacement].representative,
              scheme.signed_cosine));
    }
    CompensatedSum numerator;
    for (const CompensatedSum& shell : shells) {
      numerator.add(std::abs(shell.value));
    }
    scheme.kernel[displacement] = numerator.value / scheme.weight_sum;
  }
  return scheme;
}

struct SparseEntry {
  int index = -1;
  long double value = 0.0L;
};

struct PartitionCertificate {
  int removed = -1;
  long double bound = 0.0L;
  long double gram = 0.0L;
  long double primal = 0.0L;
  long double certified = 0.0L;
  long double lambda = 0.0L;
  long double epsilon = 0.0L;
  long double delta = 0.0L;
  long double minimum_fourier = 0.0L;
  long double minimum_dual_slack = 0.0L;
  std::vector<SparseEntry> y;
  std::vector<SparseEntry> z;
  std::vector<SparseEntry> a;
};

struct VolumeCertificate {
  int lattice_size = 0;
  int orbit_count = 0;
  int shell_count = 0;
  int maximizing_removed = -1;
  std::array<int, 3> maximum_displacement{};
  long double maximum_kernel = 0.0L;
  long double pulse_operator = 0.0L;
  long double common_step = 0.0L;
  long double bound = 0.0L;
  long double margin = 0.0L;
  bool valid = false;
  std::vector<long double> kernel;
  std::array<PartitionCertificate, 11> partitions{};
};

std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> fields;
  std::string field;
  std::istringstream stream(line);
  while (std::getline(stream, field, ',')) fields.push_back(field);
  while (fields.size() < 13) fields.emplace_back();
  return fields;
}

std::array<int, 3> parse_displacement(const std::string& value) {
  std::array<int, 3> result{};
  std::string token;
  std::istringstream stream(value);
  for (int axis = 0; axis < 3; ++axis) {
    if (!std::getline(stream, token, ':')) {
      throw std::runtime_error("invalid displacement in certificate");
    }
    result[static_cast<std::size_t>(axis)] = std::stoi(token);
  }
  return result;
}

std::map<int, VolumeCertificate> load_certificates(const std::string& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open FTD-0596 certificate");
  std::map<int, VolumeCertificate> volumes;
  std::string line;
  std::getline(stream, line);
  while (std::getline(stream, line)) {
    if (line.empty()) continue;
    const std::vector<std::string> fields = split_csv(line);
    const std::string& kind = fields[0];
    const int lattice_size = std::stoi(fields[1]);
    VolumeCertificate& volume = volumes[lattice_size];
    volume.lattice_size = lattice_size;
    if (kind == "volume") {
      volume.orbit_count = std::stoi(fields[3]);
      volume.maximum_kernel = std::stold(fields[4]);
      volume.shell_count = std::stoi(fields[5]);
      volume.pulse_operator = std::stold(fields[6]);
      volume.common_step = std::stold(fields[7]);
      volume.maximizing_removed = std::stoi(fields[8]);
      volume.bound = std::stold(fields[9]);
      volume.margin = std::stold(fields[10]);
      volume.maximum_displacement = parse_displacement(fields[11]);
      volume.valid = std::stoi(fields[12]) != 0;
    } else if (kind == "kappa") {
      const int index = std::stoi(fields[3]);
      if (volume.kernel.size() <= static_cast<std::size_t>(index)) {
        volume.kernel.resize(static_cast<std::size_t>(index + 1));
      }
      volume.kernel[static_cast<std::size_t>(index)] = std::stold(fields[4]);
    } else {
      const int removed = std::stoi(fields[2]);
      PartitionCertificate& partition = volume.partitions[
          static_cast<std::size_t>(removed)];
      partition.removed = removed;
      if (kind == "partition") {
        partition.bound = std::stold(fields[4]);
        partition.gram = std::stold(fields[5]);
        partition.primal = std::stold(fields[6]);
        partition.certified = std::stold(fields[7]);
        partition.lambda = std::stold(fields[8]);
        partition.epsilon = std::stold(fields[9]);
        partition.delta = std::stold(fields[10]);
        partition.minimum_fourier = std::stold(fields[11]);
        partition.minimum_dual_slack = std::stold(fields[12]);
      } else if (kind == "y" || kind == "z" || kind == "a") {
        const SparseEntry entry{std::stoi(fields[3]), std::stold(fields[4])};
        if (kind == "y") partition.y.push_back(entry);
        if (kind == "z") partition.z.push_back(entry);
        if (kind == "a") partition.a.push_back(entry);
      }
    }
  }
  return volumes;
}

long double direct_character_value(const Scheme& scheme, int momentum_orbit,
                                   int displacement_orbit,
                                   long double& imaginary_residual) {
  const long double pi = std::acos(-1.0L);
  const std::array<int, 3>& momentum = scheme.orbits[
      static_cast<std::size_t>(momentum_orbit)].representative;
  const Orbit& displacement = scheme.orbits[
      static_cast<std::size_t>(displacement_orbit)];
  CompensatedSum real;
  CompensatedSum imaginary;
  for (const Index3& member : displacement.members) {
    const int phase_index = modulo(
        momentum[0] * member.x + momentum[1] * member.y
            + momentum[2] * member.z,
        scheme.lattice_size);
    const long double phase = 2.0L * pi * phase_index / scheme.lattice_size;
    real.add(std::cos(phase));
    imaginary.add(std::sin(phase));
  }
  imaginary_residual = std::max(
      imaginary_residual,
      std::abs(imaginary.value) / displacement.members.size());
  return real.value / displacement.members.size();
}

TenSourceDistanceDistributionVolume verify_volume(
    const Scheme& scheme, const VolumeCertificate& certificate) {
  TenSourceDistanceDistributionVolume result;
  result.lattice_size = scheme.lattice_size;
  result.orbit_count = static_cast<int>(scheme.orbits.size());
  result.shell_count = scheme.shell_count;
  result.exact_orbit_coverage = scheme.exact_coverage;
  result.exact_shell_partition = scheme.exact_shell_partition;
  result.pulse_operator_coefficient = static_cast<double>(scheme.pulse_operator);
  result.common_step_coefficient = static_cast<double>(scheme.common_step);

  bool matches = certificate.valid
      && certificate.orbit_count == result.orbit_count
      && certificate.shell_count == result.shell_count
      && certificate.kernel.size() == scheme.kernel.size()
      && std::abs(certificate.pulse_operator - scheme.pulse_operator)
          <= KERNEL_TOL
      && std::abs(certificate.common_step - scheme.common_step)
          <= KERNEL_TOL;
  long double kernel_residual = 0.0L;
  for (std::size_t index = 0;
       index < std::min(certificate.kernel.size(), scheme.kernel.size());
       ++index) {
    kernel_residual = std::max(
        kernel_residual,
        std::abs(certificate.kernel[index] - scheme.kernel[index]));
  }
  result.kernel_table_residual = static_cast<double>(kernel_residual);
  matches = matches && kernel_residual <= KERNEL_TOL;
  const auto maximum = std::max_element(
      scheme.kernel.begin(), scheme.kernel.end());
  const int maximum_index = static_cast<int>(maximum - scheme.kernel.begin());
  result.maximum_kernel = static_cast<double>(*maximum);
  result.maximum_kernel_residual = static_cast<double>(
      std::abs(*maximum - certificate.maximum_kernel));
  result.maximum_kernel_displacement = scheme.orbits[
      static_cast<std::size_t>(maximum_index)].representative;
  matches = matches
      && result.maximum_kernel_displacement == certificate.maximum_displacement
      && result.maximum_kernel_residual <= KERNEL_TOL;

  std::map<int, std::vector<long double>> active_rows;
  std::map<int, long double> active_row_residuals;
  bool all_primal = true;
  bool all_dual = true;
  bool no_weaker = true;
  long double maximum_bound = -std::numeric_limits<long double>::infinity();
  int maximizing_removed = -1;
  const int axial_index = static_cast<int>(std::find_if(
      scheme.orbits.begin(), scheme.orbits.end(), [](const Orbit& orbit) {
        return orbit.representative == std::array<int, 3>{0, 0, 1};
      }) - scheme.orbits.begin());
  long double second_kernel = -1.0L;
  for (std::size_t index = 0; index < scheme.kernel.size(); ++index) {
    if (static_cast<int>(index) != axial_index) {
      second_kernel = std::max(second_kernel, scheme.kernel[index]);
    }
  }

  for (int removed = 0; removed <= 10; ++removed) {
    const PartitionCertificate& source = certificate.partitions[
        static_cast<std::size_t>(removed)];
    DistanceDistributionPartitionRecord& partition = result.partitions[
        static_cast<std::size_t>(removed)];
    partition.removed_count = removed;
    long double bound = 0.0L;
    long double gram = 0.0L;
    bool valid = source.removed == removed;
    if (removed == 0) {
      bound = scheme.common_step * std::sqrt(10.0L);
    } else if (removed == 1) {
      gram = 1.0L;
      bound = 3.0L * scheme.common_step + scheme.pulse_operator;
    } else if (removed == 10) {
      gram = 10.0L * (1.0L + 9.0L * *maximum);
      bound = scheme.pulse_operator * std::sqrt(gram);
    } else {
      const long double h = removed - 1.0L;
      std::vector<long double> upper(scheme.orbits.size(), h);
      for (std::size_t index = 0; index < upper.size(); ++index) {
        upper[index] = std::min<long double>(
            h, scheme.orbits[index].members.size());
      }
      upper[static_cast<std::size_t>(axial_index)] = std::min(
          upper[static_cast<std::size_t>(axial_index)],
          2.0L * EDGE_CAPS[static_cast<std::size_t>(removed)] / removed);
      std::vector<long double> a(scheme.orbits.size(), 0.0L);
      std::vector<long double> z(scheme.orbits.size(), 0.0L);
      CompensatedSum normalization;
      bool signs = true;
      for (const SparseEntry& entry : source.a) {
        valid = valid && entry.index >= 0
            && entry.index < static_cast<int>(a.size());
        if (entry.index < 0 || entry.index >= static_cast<int>(a.size())) {
          continue;
        }
        a[static_cast<std::size_t>(entry.index)] += entry.value;
        normalization.add(entry.value);
        signs = signs && entry.value >= -DUAL_SIGN_TOL;
      }
      for (const SparseEntry& entry : source.z) {
        valid = valid && entry.index >= 0
            && entry.index < static_cast<int>(z.size());
        if (entry.index < 0 || entry.index >= static_cast<int>(z.size())) {
          continue;
        }
        z[static_cast<std::size_t>(entry.index)] += entry.value;
        signs = signs && entry.value >= -DUAL_SIGN_TOL;
      }
      long double upper_residual = 0.0L;
      CompensatedSum primal;
      for (std::size_t index = 0; index < a.size(); ++index) {
        upper_residual = std::max(
            upper_residual, std::max(0.0L, a[index] - upper[index]));
        primal.add(a[index] * scheme.kernel[index]);
      }
      long double minimum_fourier = std::numeric_limits<long double>::infinity();
      for (std::size_t momentum = 0; momentum < scheme.orbits.size();
           ++momentum) {
        CompensatedSum fourier;
        fourier.add(1.0L);
        for (const SparseEntry& entry : source.a) {
          fourier.add(entry.value * character_value(
              scheme, static_cast<int>(momentum), entry.index));
        }
        minimum_fourier = std::min(minimum_fourier, fourier.value);
      }

      std::vector<long double> pressure(scheme.orbits.size(), 0.0L);
      CompensatedSum y_sum;
      long double character_residual = 0.0L;
      for (const SparseEntry& entry : source.y) {
        signs = signs && entry.value >= -DUAL_SIGN_TOL;
        auto row = active_rows.find(entry.index);
        if (row == active_rows.end()) {
          std::vector<long double> values(scheme.orbits.size(), 0.0L);
          long double direct_residual = 0.0L;
          long double imaginary_residual = 0.0L;
          for (std::size_t displacement = 0;
               displacement < scheme.orbits.size(); ++displacement) {
            values[displacement] = character_value(
                scheme, entry.index, static_cast<int>(displacement));
            const long double direct = direct_character_value(
                scheme, entry.index, static_cast<int>(displacement),
                imaginary_residual);
            direct_residual = std::max(
                direct_residual, std::abs(values[displacement] - direct));
          }
          direct_residual = std::max(direct_residual, imaginary_residual);
          active_row_residuals[entry.index] = direct_residual;
          row = active_rows.emplace(entry.index, std::move(values)).first;
        }
        character_residual = std::max(
            character_residual, active_row_residuals[entry.index]);
        y_sum.add(entry.value);
        for (std::size_t index = 0; index < pressure.size(); ++index) {
          pressure[index] += entry.value * row->second[index];
        }
      }
      long double epsilon = 0.0L;
      for (std::size_t index = 0; index < pressure.size(); ++index) {
        epsilon = std::max(
            epsilon,
            scheme.kernel[index] - source.lambda - z[index] + pressure[index]);
      }
      epsilon = std::max(0.0L, epsilon);
      const long double delta = KERNEL_TOL * (1.0L + y_sum.value)
          + DUAL_PAD_FLOOR;
      const long double padded_lambda = source.lambda + epsilon + delta;
      long double minimum_dual_slack =
          std::numeric_limits<long double>::infinity();
      CompensatedSum upper_work;
      for (std::size_t index = 0; index < pressure.size(); ++index) {
        minimum_dual_slack = std::min(
            minimum_dual_slack,
            padded_lambda + z[index] - pressure[index]
                - scheme.kernel[index]);
        upper_work.add(upper[index] * z[index]);
      }
      const long double certified = h * padded_lambda
          + y_sum.value + upper_work.value;
      gram = removed * (1.0L + certified);
      bound = scheme.common_step * std::sqrt(10.0L - removed)
          + scheme.pulse_operator * std::sqrt(std::max(0.0L, gram));
      partition.primal_support_count = static_cast<int>(source.a.size());
      partition.active_dual_count = static_cast<int>(source.y.size());
      partition.primal_objective = static_cast<double>(primal.value);
      partition.certified_objective = static_cast<double>(certified);
      partition.primal_dual_gap = static_cast<double>(certified - primal.value);
      partition.minimum_fourier_value = static_cast<double>(minimum_fourier);
      partition.minimum_dual_slack = static_cast<double>(minimum_dual_slack);
      partition.maximum_character_residual =
          static_cast<double>(character_residual);
      partition.normalization_residual = static_cast<double>(
          std::abs(normalization.value - h));
      partition.upper_bound_residual = static_cast<double>(upper_residual);
      partition.epsilon_residual = static_cast<double>(
          std::abs(epsilon - source.epsilon));
      partition.delta_residual = static_cast<double>(
          std::abs(delta - source.delta));
      partition.primal_feasible = signs
          && partition.normalization_residual <= SOLVER_TOL
          && upper_residual <= SOLVER_TOL
          && minimum_fourier >= -FOURIER_TOL
          && std::abs(primal.value - source.primal) <= KERNEL_TOL;
      partition.dual_certified = signs
          && minimum_dual_slack >= -DUAL_SIGN_TOL
          && character_residual <= CHARACTER_TOL
          && partition.epsilon_residual <= KERNEL_TOL
          && partition.delta_residual <= KERNEL_TOL
          && std::abs(certified - source.certified) <= KERNEL_TOL
          && certified + SOLVER_TOL >= primal.value
          && certified - primal.value <= GAP_TOL;
      valid = valid && partition.primal_feasible && partition.dual_certified;

      const long double pairs = removed * (removed - 1.0L) / 2.0L;
      const long double two_class_gram = removed + 2.0L * (
          EDGE_CAPS[static_cast<std::size_t>(removed)]
              * scheme.kernel[static_cast<std::size_t>(axial_index)]
          + (pairs - EDGE_CAPS[static_cast<std::size_t>(removed)])
              * second_kernel);
      const long double two_class_bound = scheme.common_step
          * std::sqrt(10.0L - removed)
          + scheme.pulse_operator * std::sqrt(two_class_gram);
      no_weaker = no_weaker && bound <= two_class_bound + GAP_TOL;
    }
    partition.gram_factor = static_cast<double>(gram);
    partition.partition_bound = static_cast<double>(bound);
    valid = valid
        && std::abs(bound - source.bound) <= KERNEL_TOL
        && std::abs(gram - source.gram) <= KERNEL_TOL;
    partition.valid = valid;
    all_primal = all_primal && (removed < 2 || removed > 9
        || partition.primal_feasible);
    all_dual = all_dual && (removed < 2 || removed > 9
        || partition.dual_certified);
    matches = matches && valid;
    if (bound > maximum_bound) {
      maximum_bound = bound;
      maximizing_removed = removed;
    }
  }

  result.maximizing_removed_count = maximizing_removed;
  result.distance_distribution_bound = static_cast<double>(maximum_bound);
  result.margin = K_GENESIS - result.distance_distribution_bound;
  result.certificate_matches = matches
      && maximizing_removed == certificate.maximizing_removed
      && std::abs(maximum_bound - certificate.bound) <= KERNEL_TOL
      && std::abs(static_cast<long double>(result.margin)
                  - certificate.margin) <= KERNEL_TOL;
  result.valid = result.exact_orbit_coverage
      && result.exact_shell_partition
      && result.kernel_table_residual <= KERNEL_TOL
      && result.maximum_kernel_residual <= KERNEL_TOL
      && all_primal && all_dual && no_weaker && result.certificate_matches;
  return result;
}

}  // namespace

TenSourceDistanceDistributionResult
analyze_ten_source_distance_distribution_lp(
    const std::string& certificate_csv_path) {
  TenSourceDistanceDistributionResult result;
  const std::map<int, VolumeCertificate> certificates =
      load_certificates(certificate_csv_path);
  bool all_valid = certificates.size() == VOLUMES.size();
  bool all_primal = true;
  bool all_dual = true;
  bool all_closed = true;
  for (int lattice_size : VOLUMES) {
    const auto found = certificates.find(lattice_size);
    if (found == certificates.end()) {
      all_valid = false;
      continue;
    }
    const Scheme scheme = build_scheme(lattice_size);
    TenSourceDistanceDistributionVolume volume = verify_volume(
        scheme, found->second);
    for (int removed = 2; removed <= 9; ++removed) {
      all_primal = all_primal && volume.partitions[
          static_cast<std::size_t>(removed)].primal_feasible;
      all_dual = all_dual && volume.partitions[
          static_cast<std::size_t>(removed)].dual_certified;
    }
    all_valid = all_valid && volume.valid;
    all_closed = all_closed && volume.distance_distribution_bound < K_GENESIS;
    result.volumes.push_back(std::move(volume));
    ++result.spectral_volume_count;
  }
  result.all_primal_feasible = all_primal;
  result.all_dual_certified = all_dual;
  result.arbitrary_removal_n_le_ten_closed = all_valid && all_closed;
  result.distance_distribution_lp_inconclusive = all_valid && !all_closed;
  result.valid = all_valid && result.spectral_volume_count == 4
      && (result.arbitrary_removal_n_le_ten_closed
          || result.distance_distribution_lp_inconclusive);
  return result;
}

}  // namespace ftd::eft
