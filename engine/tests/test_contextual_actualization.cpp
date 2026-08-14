/** FTD-0825 isolated contextual-actualization reference verifier. */

#include "ftd/eft/contextual_actualization.h"
#include "ftd/ontic/lemniscate.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

using namespace ftd::eft::contextual;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

bool close(double first, double second, double tolerance = 1e-12) {
  return std::abs(first - second) <= tolerance;
}

}  // namespace

int main() {
  const Region left({{0, 0, 0}});
  const Region right({{8, 0, 0}});
  const Region both({{8, 0, 0}, {0, 0, 0}, {0, 0, 0}});
  PotentialityNet net;
  check("region normalization removes duplicates", both.size() == 2);
  check("net isotony", net.isotonic(left, both) && net.isotonic(right, both));
  check("disjoint factors commute", net.spacelike_commute(left, right));
  const auto actual = net.actual_record_algebra(both);
  const auto potential = net.potential_algebra(both);
  check("actual record algebra is C^(3^N)",
      actual.commutative && actual.vector_space_dimension == 9);
  check("potential reference is selected M_(3^N)",
      !potential.commutative && potential.finite_type_i
          && potential.selected_reference && potential.hilbert_dimension == 9
          && potential.vector_space_dimension == 81);

  const auto prepared = PreparationMap::from_positive_weights({1, 2, 3, 2});
  check("preparation map normalizes adopted weights",
      prepared.valid() && close(prepared.total_weight(), 1.0));
  SelectorState deterministic(0.42);
  check("selector is deterministic",
      deterministic.select(prepared) == deterministic.select(prepared));
  std::vector<int> counts(4, 0);
  for (int index = 0; index < 8000; ++index) {
    SelectorState selector((static_cast<double>(index) + 0.5) / 8000.0);
    ++counts[selector.select(prepared)];
  }
  check("quantile pushforward is exact on frozen rational grid",
      counts == std::vector<int>({1000, 2000, 3000, 2000}));

  constexpr double pi = ftd::ontic::PI;
  const double chsh = chsh_value(0.0, 0.5 * pi, 0.25 * pi, -0.25 * pi);
  check("selected singlet reference reaches Tsirelson",
      close(std::abs(chsh), 2.0 * std::sqrt(2.0), 1e-12));
  check("selected reference satisfies operator ceiling",
      within_tsirelson_bound(chsh));
  check("PR-box control is rejected", !within_tsirelson_bound(4.0));
  double maximum_marginal_drift = 0.0;
  for (double remote : {-1.2, -0.2, 0.8, 1.4}) {
    const auto state = singlet_joint_state(0.3, remote);
    const auto left_probability = left_marginal(state);
    const auto right_probability = right_marginal(state);
    maximum_marginal_drift = std::max(maximum_marginal_drift,
        std::max(std::abs(left_probability[0] - 0.5),
                 std::abs(right_probability[0] - 0.5)));
  }
  check("remote-setting marginals are no-signalling",
      maximum_marginal_drift <= 1e-15);
  check("joint selector is not locally factorized",
      factorization_residual(singlet_joint_state(0.0, 0.0)) >= 0.25 - 1e-15);

  const double amplitude = 0.7;
  const double mass = 1.3;
  const double coupling = 0.9;
  const double period = critical_quartic_period(amplitude, mass, coupling);
  check("critical period carries canonical G* factor",
      close(period * amplitude / std::sqrt(mass / (2.0 * coupling)),
            std::sqrt(pi) * ftd::ontic::G_STAR, 2e-15));
  ClockController controller;
  controller.detuning_gain = 0.25;
  controller.amplitude_gain = 0.2;
  controller.detuning_tolerance = 1e-8;
  controller.relative_amplitude_tolerance = 1e-8;
  CriticalClockState clock;
  clock.phase = -0.1;
  clock.amplitude = 0.6;
  clock.target_amplitude = 1.0;
  clock.detuning = 0.3;
  for (int tick = 0; tick < 140; ++tick) {
    step_critical_clock(clock, 0.1, controller);
  }
  check("clock feedback reaches detuning compliance",
      std::abs(clock.detuning) <= controller.detuning_tolerance);
  check("clock feedback reaches amplitude compliance",
      std::abs(clock.amplitude - clock.target_amplitude)
          <= controller.relative_amplitude_tolerance);
  check("clock reference feedback accounting is recorded",
      clock.audit.controller_work > 0.0 && clock.audit.dissipated_energy > 0.0);
  check("eligible section crossing opens a gate", clock.gate_count >= 1);

  LocalInstrument alice{"alice", left, {"+", "-"}};
  LocalInstrument bob{"bob", right, {"+", "-"}};
  MeasurementContext context{
      "bell-a0-b0",
      {alice, bob},
      {{"+", "+"}, {"+", "-"}, {"-", "+"}, {"-", "-"}},
      singlet_joint_state(0.0, 0.0),
  };
  check("spacelike instruments compose order independently",
      context.spacelike_order_independent(net));
  ActualizationBatch batch{clock.global_tick, context,
      {{true, true, true, true}, {true, true, true, true}}};
  SelectorState batch_selector(0.3);
  const auto event = batch.actualize(batch_selector);
  check("one context-complete record is actualized",
      event.has_value() && event->records.size() == 2
          && event->context_id == context.id);

  std::cout << "FTD-0825 contextual actualization EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "CHSH=" << chsh << '\n';
  std::cout << "maximum_no_signalling_marginal_drift="
            << maximum_marginal_drift << '\n';
  std::cout << "scope=SELECTED_REFERENCE_NOT_SUBSTRATE_BORN_RECOVERY\n";
  return failures == 0 ? 0 : 1;
}
