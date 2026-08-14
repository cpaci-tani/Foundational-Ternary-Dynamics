#pragma once
/**
 * @file contextual_actualization.h
 * @brief FTD-0825 isolated contextual-actualization reference interfaces.
 *
 * These types are a selected EFT reference model. They are not production
 * Voxel state, do not derive the Born rule from substrate dynamics, and do not
 * alter the default engine tick. The actual record algebra is commutative;
 * the finite qutrit potential algebra is a separately selected Type-I witness.
 */

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ftd::eft::contextual {

struct LatticeSite {
  int x = 0;
  int y = 0;
  int z = 0;
  bool operator==(const LatticeSite& other) const;
  bool operator<(const LatticeSite& other) const;
};

class Region {
 public:
  explicit Region(std::vector<LatticeSite> sites = {});
  const std::vector<LatticeSite>& sites() const { return sites_; }
  std::size_t size() const { return sites_.size(); }
  bool empty() const { return sites_.empty(); }
  bool contains(const LatticeSite& site) const;
  bool contains(const Region& other) const;
  bool disjoint(const Region& other) const;

 private:
  std::vector<LatticeSite> sites_;
};

struct AlgebraDescriptor {
  std::size_t hilbert_dimension = 0;
  std::size_t vector_space_dimension = 0;
  bool commutative = false;
  bool finite_type_i = false;
  bool selected_reference = false;
};

class PotentialityNet {
 public:
  AlgebraDescriptor actual_record_algebra(const Region& region) const;
  AlgebraDescriptor potential_algebra(const Region& region) const;
  bool isotonic(const Region& subregion, const Region& region) const;
  bool spacelike_commute(const Region& first, const Region& second) const;
};

class PotentialState {
 public:
  PotentialState() = default;
  explicit PotentialState(std::vector<double> weights);
  bool valid() const { return valid_; }
  const std::vector<double>& weights() const { return weights_; }
  double total_weight() const;

 private:
  std::vector<double> weights_;
  bool valid_ = false;
};

/** Adopted bridge interface; physical substrate recovery remains OPEN. */
class PreparationMap {
 public:
  static PotentialState from_positive_weights(
      const std::vector<double>& weights);
};

struct LocalInstrument {
  std::string id;
  Region region;
  std::vector<std::string> outcomes;
  bool valid() const;
};

struct MeasurementContext {
  std::string id;
  std::vector<LocalInstrument> instruments;
  std::vector<std::vector<std::string>> joint_outcomes;
  PotentialState joint_state;
  bool valid() const;
  bool spacelike_order_independent(const PotentialityNet& net) const;
};

class SelectorState {
 public:
  explicit SelectorState(double unit_coordinate);
  bool valid() const { return valid_; }
  double unit_coordinate() const { return unit_coordinate_; }
  std::size_t select(const PotentialState& state) const;
  void advance();

 private:
  double unit_coordinate_ = 0.0;
  bool valid_ = false;
};

struct ClockCompliance {
  bool detuning_ok = false;
  bool amplitude_ok = false;
  bool phase_crossing = false;
  bool gate_open = false;
};

struct FeedbackAudit {
  // Legacy field names retained for API compatibility. These accumulate
  // dimensionless correction diagnostics, not physical work or energy.
  double controller_work = 0.0;
  double dissipated_energy = 0.0;
  double detuning_correction = 0.0;
  double amplitude_correction = 0.0;
};

struct CriticalClockState {
  std::uint64_t global_tick = 0;
  double phase = 0.0;
  // Accumulated phase parameter, not a calibrated physical duration.
  double local_duration = 0.0;
  std::uint64_t gate_count = 0;
  double amplitude = 1.0;
  double target_amplitude = 1.0;
  double detuning = 0.0;
  FeedbackAudit audit;
};

struct ClockController {
  double detuning_gain = 0.2;
  double amplitude_gain = 0.15;
  double detuning_tolerance = 1e-6;
  double relative_amplitude_tolerance = 1e-6;
  double section_phase = 0.0;
  bool valid() const;
};

struct ClockStep {
  ClockCompliance compliance;
  FeedbackAudit audit;
};

double critical_quartic_period(
    double amplitude, double mass, double coupling);
ClockStep step_critical_clock(
    CriticalClockState& state,
    double phase_increment,
    const ClockController& controller);

struct ActualizationEvent {
  std::uint64_t global_tick = 0;
  std::string context_id;
  std::size_t joint_outcome_index = 0;
  std::vector<std::string> records;
};

struct ActualizationBatch {
  std::uint64_t global_tick = 0;
  MeasurementContext context;
  std::vector<ClockCompliance> clock_compliance;
  std::optional<ActualizationEvent> actualize(SelectorState& selector) const;
};

PotentialState singlet_joint_state(double left_angle, double right_angle);
double dichotomic_correlation(const PotentialState& joint_state);
double chsh_value(double a0, double a1, double b0, double b1);
std::vector<double> left_marginal(const PotentialState& joint_state);
std::vector<double> right_marginal(const PotentialState& joint_state);
double factorization_residual(const PotentialState& joint_state);
bool within_tsirelson_bound(double value, double tolerance = 1e-12);

}  // namespace ftd::eft::contextual
