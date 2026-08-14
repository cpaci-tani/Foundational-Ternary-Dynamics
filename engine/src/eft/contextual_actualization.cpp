#include "ftd/eft/contextual_actualization.h"

#include "ftd/ontic/lemniscate.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>
#include <tuple>
#include <utility>

namespace ftd::eft::contextual {
namespace {

constexpr double normalization_tolerance = 1e-12;

std::size_t integer_power(std::size_t base, std::size_t exponent) {
  std::size_t result = 1;
  for (std::size_t i = 0; i < exponent; ++i) {
    if (result > std::numeric_limits<std::size_t>::max() / base) return 0;
    result *= base;
  }
  return result;
}

bool finite_nonnegative(double value) {
  return std::isfinite(value) && value >= 0.0;
}

}  // namespace

bool LatticeSite::operator==(const LatticeSite& other) const {
  return x == other.x && y == other.y && z == other.z;
}

bool LatticeSite::operator<(const LatticeSite& other) const {
  return std::tie(x, y, z) < std::tie(other.x, other.y, other.z);
}

Region::Region(std::vector<LatticeSite> sites) : sites_(std::move(sites)) {
  std::sort(sites_.begin(), sites_.end());
  sites_.erase(std::unique(sites_.begin(), sites_.end()), sites_.end());
}

bool Region::contains(const LatticeSite& site) const {
  return std::binary_search(sites_.begin(), sites_.end(), site);
}

bool Region::contains(const Region& other) const {
  return std::includes(
      sites_.begin(), sites_.end(), other.sites_.begin(), other.sites_.end());
}

bool Region::disjoint(const Region& other) const {
  std::vector<LatticeSite> overlap;
  std::set_intersection(
      sites_.begin(), sites_.end(), other.sites_.begin(), other.sites_.end(),
      std::back_inserter(overlap));
  return overlap.empty();
}

AlgebraDescriptor PotentialityNet::actual_record_algebra(
    const Region& region) const {
  const std::size_t dimension = integer_power(3, region.size());
  return {dimension, dimension, true, true, false};
}

AlgebraDescriptor PotentialityNet::potential_algebra(
    const Region& region) const {
  const std::size_t hilbert_dimension = integer_power(3, region.size());
  const std::size_t vector_dimension = hilbert_dimension == 0
      ? 0 : integer_power(hilbert_dimension, 2);
  return {
      hilbert_dimension,
      vector_dimension,
      hilbert_dimension <= 1,
      hilbert_dimension > 0,
      true,
  };
}

bool PotentialityNet::isotonic(
    const Region& subregion, const Region& region) const {
  return region.contains(subregion);
}

bool PotentialityNet::spacelike_commute(
    const Region& first, const Region& second) const {
  return first.disjoint(second);
}

PotentialState::PotentialState(std::vector<double> weights)
    : weights_(std::move(weights)) {
  if (weights_.empty()) return;
  if (!std::all_of(
          weights_.begin(), weights_.end(), finite_nonnegative)) return;
  const double total = total_weight();
  if (!(total > 0.0) || !std::isfinite(total)) return;
  for (double& weight : weights_) weight /= total;
  valid_ = std::abs(total_weight() - 1.0) <= normalization_tolerance;
}

double PotentialState::total_weight() const {
  return std::accumulate(weights_.begin(), weights_.end(), 0.0);
}

PotentialState PreparationMap::from_positive_weights(
    const std::vector<double>& weights) {
  return PotentialState(weights);
}

bool LocalInstrument::valid() const {
  if (id.empty() || region.empty() || outcomes.empty()) return false;
  std::set<std::string> unique;
  for (const std::string& outcome : outcomes) {
    if (outcome.empty() || !unique.insert(outcome).second) return false;
  }
  return true;
}

bool MeasurementContext::valid() const {
  if (id.empty() || instruments.empty() || !joint_state.valid()
      || joint_outcomes.size() != joint_state.weights().size()) return false;
  for (const LocalInstrument& instrument : instruments) {
    if (!instrument.valid()) return false;
  }
  for (const auto& record : joint_outcomes) {
    if (record.size() != instruments.size()) return false;
    for (std::size_t i = 0; i < record.size(); ++i) {
      const auto& allowed = instruments[i].outcomes;
      if (std::find(allowed.begin(), allowed.end(), record[i])
          == allowed.end()) return false;
    }
  }
  return true;
}

bool MeasurementContext::spacelike_order_independent(
    const PotentialityNet& net) const {
  if (!valid()) return false;
  for (std::size_t i = 0; i < instruments.size(); ++i) {
    for (std::size_t j = i + 1; j < instruments.size(); ++j) {
      if (!net.spacelike_commute(
              instruments[i].region, instruments[j].region)) return false;
    }
  }
  return true;
}

SelectorState::SelectorState(double unit_coordinate)
    : unit_coordinate_(unit_coordinate),
      valid_(std::isfinite(unit_coordinate)
          && unit_coordinate >= 0.0 && unit_coordinate < 1.0) {}

std::size_t SelectorState::select(const PotentialState& state) const {
  if (!valid_ || !state.valid()) return state.weights().size();
  double cumulative = 0.0;
  for (std::size_t i = 0; i < state.weights().size(); ++i) {
    cumulative += state.weights()[i];
    if (unit_coordinate_ < cumulative || i + 1 == state.weights().size()) {
      return i;
    }
  }
  return state.weights().size();
}

void SelectorState::advance() {
  if (!valid_) return;
  unit_coordinate_ = std::fmod(2.0 * unit_coordinate_, 1.0);
}

bool ClockController::valid() const {
  return detuning_gain > 0.0 && detuning_gain <= 1.0
      && amplitude_gain > 0.0 && amplitude_gain <= 1.0
      && finite_nonnegative(detuning_tolerance)
      && finite_nonnegative(relative_amplitude_tolerance)
      && std::isfinite(section_phase);
}

double critical_quartic_period(
    double amplitude, double mass, double coupling) {
  if (!(amplitude > 0.0) || !(mass > 0.0) || !(coupling > 0.0)
      || !std::isfinite(amplitude) || !std::isfinite(mass)
      || !std::isfinite(coupling)) return 0.0;
  return std::sqrt(ftd::ontic::PI) * ftd::ontic::G_STAR
      * std::sqrt(mass / (2.0 * coupling)) / amplitude;
}

ClockStep step_critical_clock(
    CriticalClockState& state,
    double phase_increment,
    const ClockController& controller) {
  ClockStep result;
  if (!controller.valid() || !(phase_increment > 0.0)
      || !std::isfinite(phase_increment) || !(state.amplitude > 0.0)
      || !(state.target_amplitude > 0.0)) return result;

  result.audit.detuning_correction =
      -controller.detuning_gain * state.detuning;
  result.audit.amplitude_correction = controller.amplitude_gain
      * (state.target_amplitude - state.amplitude);
  state.detuning += result.audit.detuning_correction;
  state.amplitude += result.audit.amplitude_correction;
  const double work = std::abs(result.audit.detuning_correction)
      + std::abs(result.audit.amplitude_correction);
  const double dissipation = 0.5 * (
      result.audit.detuning_correction * result.audit.detuning_correction
      + result.audit.amplitude_correction * result.audit.amplitude_correction);
  state.audit.controller_work += work;
  state.audit.dissipated_energy += dissipation;
  state.audit.detuning_correction = result.audit.detuning_correction;
  state.audit.amplitude_correction = result.audit.amplitude_correction;

  const double previous_phase = state.phase;
  state.phase += phase_increment;
  state.local_duration += phase_increment;
  ++state.global_tick;
  const double two_pi = 2.0 * ftd::ontic::PI;
  const auto previous_section = static_cast<long long>(
      std::floor((previous_phase - controller.section_phase) / two_pi));
  const auto current_section = static_cast<long long>(
      std::floor((state.phase - controller.section_phase) / two_pi));
  result.compliance.phase_crossing = current_section > previous_section;
  result.compliance.detuning_ok =
      std::abs(state.detuning) <= controller.detuning_tolerance;
  result.compliance.amplitude_ok = std::abs(
      state.amplitude / state.target_amplitude - 1.0)
      <= controller.relative_amplitude_tolerance;
  result.compliance.gate_open = result.compliance.phase_crossing
      && result.compliance.detuning_ok && result.compliance.amplitude_ok;
  if (result.compliance.gate_open) {
    state.gate_count += static_cast<std::uint64_t>(
        current_section - previous_section);
  }
  result.audit = state.audit;
  return result;
}

std::optional<ActualizationEvent> ActualizationBatch::actualize(
    SelectorState& selector) const {
  if (!context.valid() || !selector.valid() || clock_compliance.empty()) {
    return std::nullopt;
  }
  if (!std::all_of(
          clock_compliance.begin(), clock_compliance.end(),
          [](const ClockCompliance& compliance) {
            return compliance.gate_open;
          })) return std::nullopt;
  const std::size_t selected = selector.select(context.joint_state);
  if (selected >= context.joint_outcomes.size()) return std::nullopt;
  ActualizationEvent event;
  event.global_tick = global_tick;
  event.context_id = context.id;
  event.joint_outcome_index = selected;
  event.records = context.joint_outcomes[selected];
  selector.advance();
  return event;
}

PotentialState singlet_joint_state(double left_angle, double right_angle) {
  if (!std::isfinite(left_angle) || !std::isfinite(right_angle)) return {};
  const double cosine = std::cos(left_angle - right_angle);
  return PotentialState({
      0.25 * (1.0 - cosine),
      0.25 * (1.0 + cosine),
      0.25 * (1.0 + cosine),
      0.25 * (1.0 - cosine),
  });
}

double dichotomic_correlation(const PotentialState& joint_state) {
  if (!joint_state.valid() || joint_state.weights().size() != 4) return 0.0;
  const auto& weight = joint_state.weights();
  return weight[0] - weight[1] - weight[2] + weight[3];
}

double chsh_value(double a0, double a1, double b0, double b1) {
  return dichotomic_correlation(singlet_joint_state(a0, b0))
      + dichotomic_correlation(singlet_joint_state(a0, b1))
      + dichotomic_correlation(singlet_joint_state(a1, b0))
      - dichotomic_correlation(singlet_joint_state(a1, b1));
}

std::vector<double> left_marginal(const PotentialState& joint_state) {
  if (!joint_state.valid() || joint_state.weights().size() != 4) return {};
  const auto& weight = joint_state.weights();
  return {weight[0] + weight[1], weight[2] + weight[3]};
}

std::vector<double> right_marginal(const PotentialState& joint_state) {
  if (!joint_state.valid() || joint_state.weights().size() != 4) return {};
  const auto& weight = joint_state.weights();
  return {weight[0] + weight[2], weight[1] + weight[3]};
}

double factorization_residual(const PotentialState& joint_state) {
  if (!joint_state.valid() || joint_state.weights().size() != 4) return 0.0;
  const auto left = left_marginal(joint_state);
  const auto right = right_marginal(joint_state);
  double residual = 0.0;
  for (std::size_t i = 0; i < 2; ++i) {
    for (std::size_t j = 0; j < 2; ++j) {
      residual = std::max(residual, std::abs(
          joint_state.weights()[2 * i + j] - left[i] * right[j]));
    }
  }
  return residual;
}

bool within_tsirelson_bound(double value, double tolerance) {
  return finite_nonnegative(tolerance) && std::isfinite(value)
      && std::abs(value) <= 2.0 * std::sqrt(2.0) + tolerance;
}

}  // namespace ftd::eft::contextual
