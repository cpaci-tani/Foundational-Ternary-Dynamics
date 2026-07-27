#include "ftd/eft/ten_source_temporal_product_capacity.h"

// Recompile the frozen FTD-0596 observer under a private symbol so this
// verifier can reuse its exact cyclotomic association-scheme reconstruction
// and LP certificate checker without changing the parent artifact.
#define analyze_ten_source_distance_distribution_lp \
  ftd0597_embedded_parent_analyze
#include "ten_source_distance_distribution_lp.cpp"
#undef analyze_ten_source_distance_distribution_lp

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace ftd::eft {
namespace {

struct TemporalVolumeCertificate {
  VolumeCertificate base;
  std::vector<long double> parent;
  std::vector<long double> positive;
  std::vector<long double> negative;
};

std::map<int, TemporalVolumeCertificate> load_temporal_certificates(
    const std::string& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open FTD-0597 certificate");
  std::map<int, TemporalVolumeCertificate> volumes;
  std::string line;
  std::getline(stream, line);
  while (std::getline(stream, line)) {
    if (line.empty()) continue;
    const std::vector<std::string> fields = split_csv(line);
    const std::string& kind = fields[0];
    const int lattice_size = std::stoi(fields[1]);
    TemporalVolumeCertificate& temporal = volumes[lattice_size];
    VolumeCertificate& volume = temporal.base;
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
    } else if (kind == "tau") {
      const int index = std::stoi(fields[3]);
      const std::size_t size = static_cast<std::size_t>(index + 1);
      if (volume.kernel.size() < size) volume.kernel.resize(size);
      if (temporal.parent.size() < size) temporal.parent.resize(size);
      if (temporal.positive.size() < size) temporal.positive.resize(size);
      if (temporal.negative.size() < size) temporal.negative.resize(size);
      volume.kernel[static_cast<std::size_t>(index)] = std::stold(fields[4]);
      temporal.parent[static_cast<std::size_t>(index)] =
          std::stold(fields[5]);
      temporal.positive[static_cast<std::size_t>(index)] =
          std::stold(fields[6]);
      temporal.negative[static_cast<std::size_t>(index)] =
          std::stold(fields[7]);
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

struct TemporalKernel {
  std::vector<long double> tau;
  std::vector<long double> parent;
  std::vector<long double> positive;
  std::vector<long double> negative;
  long double maximum_alternate_residual = 0.0L;
  long double maximum_parent_excess = 0.0L;
};

TemporalKernel build_temporal_kernel(const Scheme& scheme) {
  TemporalKernel result;
  const std::size_t count = scheme.orbits.size();
  result.tau.resize(count, 0.0L);
  result.parent.resize(count, 0.0L);
  result.positive.resize(count, 0.0L);
  result.negative.resize(count, 0.0L);
  for (std::size_t displacement = 0; displacement < count; ++displacement) {
    std::vector<CompensatedSum> shells(
        static_cast<std::size_t>(scheme.shell_count));
    for (const Orbit& orbit : scheme.orbits) {
      shells[static_cast<std::size_t>(orbit.shell_index)].add(
          orbit.weight * orbit_character(
              orbit, scheme.orbits[displacement].representative,
              scheme.signed_cosine));
    }
    CompensatedSum positive;
    CompensatedSum negative;
    for (const CompensatedSum& shell : shells) {
      if (shell.value >= 0.0L) positive.add(shell.value);
      else negative.add(-shell.value);
    }
    const long double p = positive.value / scheme.weight_sum;
    const long double n = negative.value / scheme.weight_sum;
    const long double parent = p + n;
    const long double tau = std::max(p + 0.25L * n, n + 0.25L * p);
    const long double alternate = 0.625L * parent
        + 0.375L * std::abs(p - n);
    result.positive[displacement] = p;
    result.negative[displacement] = n;
    result.parent[displacement] = parent;
    result.tau[displacement] = tau;
    result.maximum_alternate_residual = std::max(
        result.maximum_alternate_residual, std::abs(tau - alternate));
    result.maximum_parent_excess = std::max(
        result.maximum_parent_excess, tau - parent);
  }
  return result;
}

long double maximum_table_residual(const std::vector<long double>& left,
                                   const std::vector<long double>& right) {
  if (left.size() != right.size()) {
    return std::numeric_limits<long double>::infinity();
  }
  long double residual = 0.0L;
  for (std::size_t index = 0; index < left.size(); ++index) {
    residual = std::max(residual, std::abs(left[index] - right[index]));
  }
  return residual;
}

TenSourceTemporalProductVolume verify_temporal_volume(
    Scheme scheme, const TemporalVolumeCertificate& certificate,
    const TenSourceDistanceDistributionVolume& parent_volume) {
  TenSourceTemporalProductVolume result;
  const TemporalKernel temporal = build_temporal_kernel(scheme);
  result.lattice_size = scheme.lattice_size;
  result.orbit_count = static_cast<int>(scheme.orbits.size());
  result.shell_count = scheme.shell_count;
  result.exact_orbit_coverage = scheme.exact_coverage;
  result.exact_shell_partition = scheme.exact_shell_partition;
  result.pulse_operator_coefficient = static_cast<double>(scheme.pulse_operator);
  result.common_step_coefficient = static_cast<double>(scheme.common_step);
  result.maximum_parent_kernel = static_cast<double>(
      *std::max_element(temporal.parent.begin(), temporal.parent.end()));
  result.temporal_kernel_table_residual = static_cast<double>(
      maximum_table_residual(temporal.tau, certificate.base.kernel));
  result.parent_kernel_table_residual = static_cast<double>(
      maximum_table_residual(temporal.parent, certificate.parent));
  result.positive_mass_table_residual = static_cast<double>(
      maximum_table_residual(temporal.positive, certificate.positive));
  result.negative_mass_table_residual = static_cast<double>(
      maximum_table_residual(temporal.negative, certificate.negative));
  result.maximum_alternate_formula_residual = static_cast<double>(
      temporal.maximum_alternate_residual);
  result.maximum_parent_excess = static_cast<double>(
      temporal.maximum_parent_excess);
  result.product_interval_verified =
      temporal.maximum_alternate_residual <= KERNEL_TOL
      && temporal.maximum_parent_excess <= KERNEL_TOL;

  const auto maximum_tau = std::max_element(
      temporal.tau.begin(), temporal.tau.end());
  const int maximum_index = static_cast<int>(
      maximum_tau - temporal.tau.begin());
  result.maximum_temporal_kernel = static_cast<double>(*maximum_tau);
  result.maximum_temporal_kernel_displacement = scheme.orbits[
      static_cast<std::size_t>(maximum_index)].representative;

  // The inherited verifier expects its own kernel at r=10. Supply a synthetic
  // tau r=10 record for that internal check, then restore and independently
  // verify the preregistered unchanged FTD-0596 r=10 partition below.
  VolumeCertificate synthetic = certificate.base;
  const long double synthetic_r10_gram = 10.0L * (1.0L + 9.0L * *maximum_tau);
  synthetic.partitions[10].gram = synthetic_r10_gram;
  synthetic.partitions[10].bound =
      scheme.pulse_operator * std::sqrt(synthetic_r10_gram);
  scheme.kernel = temporal.tau;
  const TenSourceDistanceDistributionVolume checked =
      verify_volume(scheme, synthetic);
  result.partitions = checked.partitions;

  const PartitionCertificate& actual_r10 = certificate.base.partitions[10];
  const long double parent_maximum = *std::max_element(
      temporal.parent.begin(), temporal.parent.end());
  const long double parent_r10_gram = 10.0L * (1.0L + 9.0L * parent_maximum);
  const long double parent_r10_bound =
      scheme.pulse_operator * std::sqrt(parent_r10_gram);
  DistanceDistributionPartitionRecord& restored_r10 = result.partitions[10];
  restored_r10.removed_count = 10;
  restored_r10.gram_factor = static_cast<double>(parent_r10_gram);
  restored_r10.partition_bound = static_cast<double>(parent_r10_bound);
  restored_r10.valid = actual_r10.removed == 10
      && std::abs(actual_r10.gram - parent_r10_gram) <= KERNEL_TOL
      && std::abs(actual_r10.bound - parent_r10_bound) <= KERNEL_TOL;

  long double maximum_bound = -std::numeric_limits<long double>::infinity();
  int maximizing_removed = -1;
  bool partitions_valid = true;
  for (int removed = 0; removed <= 10; ++removed) {
    const auto& partition = result.partitions[
        static_cast<std::size_t>(removed)];
    partitions_valid = partitions_valid && partition.valid;
    if (partition.partition_bound > maximum_bound) {
      maximum_bound = partition.partition_bound;
      maximizing_removed = removed;
    }
  }
  result.maximizing_removed_count = maximizing_removed;
  result.temporal_product_bound = static_cast<double>(maximum_bound);
  result.parent_distance_distribution_bound =
      parent_volume.distance_distribution_bound;
  result.margin = K_GENESIS - result.temporal_product_bound;

  result.certificate_matches = certificate.base.valid
      && checked.certificate_matches
      && restored_r10.valid
      && result.temporal_kernel_table_residual <= KERNEL_TOL
      && result.parent_kernel_table_residual <= KERNEL_TOL
      && result.positive_mass_table_residual <= KERNEL_TOL
      && result.negative_mass_table_residual <= KERNEL_TOL
      && result.maximum_temporal_kernel_displacement
          == certificate.base.maximum_displacement
      && std::abs(*maximum_tau - certificate.base.maximum_kernel)
          <= KERNEL_TOL
      && maximizing_removed == certificate.base.maximizing_removed
      && std::abs(maximum_bound - certificate.base.bound) <= KERNEL_TOL
      && std::abs(static_cast<long double>(result.margin)
                  - certificate.base.margin) <= KERNEL_TOL;
  result.valid = checked.valid
      && parent_volume.valid
      && result.exact_orbit_coverage
      && result.exact_shell_partition
      && result.product_interval_verified
      && partitions_valid
      && result.temporal_product_bound
          <= result.parent_distance_distribution_bound + GAP_TOL
      && result.certificate_matches;
  return result;
}

}  // namespace

TenSourceTemporalProductResult
analyze_ten_source_temporal_product_capacity(
    const std::string& certificate_csv_path,
    const std::string& parent_certificate_csv_path) {
  TenSourceTemporalProductResult result;
  const std::map<int, TemporalVolumeCertificate> certificates =
      load_temporal_certificates(certificate_csv_path);
  const TenSourceDistanceDistributionResult parent =
      ftd0597_embedded_parent_analyze(parent_certificate_csv_path);
  std::map<int, TenSourceDistanceDistributionVolume> parent_by_lattice;
  for (const auto& volume : parent.volumes) {
    parent_by_lattice.emplace(volume.lattice_size, volume);
  }

  bool all_valid = certificates.size() == VOLUMES.size()
      && parent.valid && parent_by_lattice.size() == VOLUMES.size();
  bool all_primal = true;
  bool all_dual = true;
  bool all_closed = true;
  bool product_lemma = true;
  for (int lattice_size : VOLUMES) {
    const auto found = certificates.find(lattice_size);
    const auto parent_found = parent_by_lattice.find(lattice_size);
    if (found == certificates.end()
        || parent_found == parent_by_lattice.end()) {
      all_valid = false;
      continue;
    }
    TenSourceTemporalProductVolume volume = verify_temporal_volume(
        build_scheme(lattice_size), found->second, parent_found->second);
    for (int removed = 2; removed <= 9; ++removed) {
      const auto& partition = volume.partitions[
          static_cast<std::size_t>(removed)];
      all_primal = all_primal && partition.primal_feasible;
      all_dual = all_dual && partition.dual_certified;
    }
    product_lemma = product_lemma && volume.product_interval_verified;
    all_closed = all_closed && volume.temporal_product_bound < K_GENESIS;
    all_valid = all_valid && volume.valid;
    result.volumes.push_back(std::move(volume));
  }
  result.spectral_volume_count = static_cast<int>(result.volumes.size());
  result.exact_pulse_product_lemma = product_lemma;
  result.all_primal_feasible = all_primal;
  result.all_dual_certified = all_dual;
  result.arbitrary_removal_n_le_ten_closed = all_valid && all_closed;
  result.temporal_product_bound_inconclusive = all_valid && !all_closed;
  result.valid = all_valid && result.exact_pulse_product_lemma
      && result.all_primal_feasible && result.all_dual_certified
      && (result.arbitrary_removal_n_le_ten_closed
          || result.temporal_product_bound_inconclusive);
  return result;
}

}  // namespace ftd::eft
