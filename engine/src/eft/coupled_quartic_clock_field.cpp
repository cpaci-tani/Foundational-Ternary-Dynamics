#include "ftd/eft/coupled_quartic_clock_field.h"

#include "ftd/ontic/lemniscate.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <queue>
#include <utility>

namespace ftd::eft {
namespace {

double beta_function(double x, double y) {
  return std::tgamma(x) * std::tgamma(y) / std::tgamma(x + y);
}

double wrap_angle(double angle) {
  return std::remainder(angle, 2.0 * ftd::ontic::PI);
}

bool finite_site(const ClockSite& site) {
  return site.action > 0.0 && std::isfinite(site.action)
      && std::isfinite(site.phase) && std::isfinite(site.compliance);
}

}  // namespace

double PowerClockLaw::action_from_energy(double energy) const {
  if (!valid || !(energy > 0.0) || !std::isfinite(energy)) return 0.0;
  return action_coefficient * std::pow(
      2.0 * energy, action_energy_exponent);
}

double PowerClockLaw::energy_from_action(double action) const {
  if (!valid || !(action > 0.0) || !std::isfinite(action)) return 0.0;
  return 0.5 * std::pow(
      action / action_coefficient, energy_action_exponent);
}

double PowerClockLaw::frequency(double action) const {
  const double energy = energy_from_action(action);
  if (!(energy > 0.0)) return 0.0;
  return energy_action_exponent * energy / action;
}

double PowerClockLaw::curvature(double action) const {
  const double omega = frequency(action);
  if (!(omega > 0.0)) return 0.0;
  return (energy_action_exponent - 1.0) * omega / action;
}

double PowerClockLaw::period(double energy) const {
  if (!valid || !(energy > 0.0) || !std::isfinite(energy)) return 0.0;
  const double scaling_exponent = -static_cast<double>(exponent - 2)
      / (2.0 * static_cast<double>(exponent));
  return unit_shell_period * std::pow(2.0 * energy, scaling_exponent);
}

PowerClockLaw make_even_power_clock_law(int exponent) {
  PowerClockLaw law;
  if (exponent < 2 || exponent % 2 != 0) return law;
  const double m = static_cast<double>(exponent);
  law.exponent = exponent;
  law.action_energy_exponent = (m + 2.0) / (2.0 * m);
  law.energy_action_exponent = 1.0 / law.action_energy_exponent;
  law.action_coefficient = 2.0 * beta_function(1.0 / m, 1.5)
      / (ftd::ontic::PI * m);
  law.unit_shell_period = 4.0 * beta_function(1.0 / m, 0.5) / m;
  law.valid = std::isfinite(law.action_coefficient)
      && law.action_coefficient > 0.0
      && std::isfinite(law.unit_shell_period)
      && law.unit_shell_period > 0.0;
  return law;
}

double linear_wave_cycle_ratio_squared(
    int exponent, double kappa_over_energy) {
  if (exponent < 2 || exponent % 2 != 0
      || !(kappa_over_energy >= 0.0)
      || !std::isfinite(kappa_over_energy)) return -1.0;
  return kappa_over_energy * static_cast<double>(exponent - 2)
      / (2.0 * static_cast<double>(exponent));
}

double axial_continuum_neighbor_factor() {
  return 1.0;
}

double full_moore_continuum_neighbor_factor(int dimension) {
  if (dimension < 1) return 0.0;
  return std::pow(3.0, static_cast<double>(dimension - 1));
}

ConnectionIntegrabilityResult analyze_connection_integrability(
    std::size_t site_count,
    const std::vector<ClockEdge>& edges,
    double tolerance) {
  ConnectionIntegrabilityResult result;
  if (site_count == 0 || !(tolerance >= 0.0)
      || !std::isfinite(tolerance)) return result;

  using Neighbor = std::pair<std::size_t, double>;
  std::vector<std::vector<Neighbor>> adjacency(site_count);
  for (const ClockEdge& edge : edges) {
    if (edge.tail >= site_count || edge.head >= site_count
        || edge.tail == edge.head || !std::isfinite(edge.connection)) {
      return result;
    }
    adjacency[edge.tail].push_back({edge.head, edge.connection});
    adjacency[edge.head].push_back({edge.tail, -edge.connection});
  }

  result.gauge_offsets.assign(site_count, 0.0);
  std::vector<bool> assigned(site_count, false);
  for (std::size_t root = 0; root < site_count; ++root) {
    if (assigned[root]) continue;
    ++result.component_count;
    assigned[root] = true;
    std::queue<std::size_t> pending;
    pending.push(root);
    while (!pending.empty()) {
      const std::size_t site = pending.front();
      pending.pop();
      for (const Neighbor& neighbor : adjacency[site]) {
        const double candidate = result.gauge_offsets[site] + neighbor.second;
        if (!assigned[neighbor.first]) {
          assigned[neighbor.first] = true;
          result.gauge_offsets[neighbor.first] = candidate;
          pending.push(neighbor.first);
        } else {
          result.maximum_cycle_residual = std::max(
              result.maximum_cycle_residual,
              std::abs(wrap_angle(
                  result.gauge_offsets[neighbor.first] - candidate)));
        }
      }
    }
  }

  for (const ClockEdge& edge : edges) {
    result.maximum_cycle_residual = std::max(
        result.maximum_cycle_residual,
        std::abs(wrap_angle(edge.connection
            + result.gauge_offsets[edge.tail]
            - result.gauge_offsets[edge.head])));
  }
  result.valid = true;
  result.integrable = result.maximum_cycle_residual <= tolerance;
  return result;
}

CoupledClockField::CoupledClockField(
    PowerClockLaw law,
    std::vector<ClockSite> sites,
    std::vector<ClockEdge> edges,
    double stiffness)
    : law_(law),
      sites_(std::move(sites)),
      edges_(std::move(edges)),
      stiffness_(stiffness) {
  valid_ = law_.valid && !sites_.empty() && stiffness_ >= 0.0
      && std::isfinite(stiffness_);
  for (const ClockSite& site : sites_) valid_ = valid_ && finite_site(site);
  for (const ClockEdge& edge : edges_) {
    valid_ = valid_ && edge.tail < sites_.size() && edge.head < sites_.size()
        && edge.tail != edge.head && std::isfinite(edge.connection);
  }
}

double CoupledClockField::hamiltonian() const {
  if (!valid_) return 0.0;
  double total = 0.0;
  for (const ClockSite& site : sites_) {
    total += std::exp(-site.compliance)
        * law_.energy_from_action(site.action);
  }
  for (const ClockEdge& edge : edges_) {
    const double phase_difference = sites_[edge.tail].phase
        - sites_[edge.head].phase - edge.connection;
    total += stiffness_ * (1.0 - std::cos(phase_difference));
  }
  return total;
}

double CoupledClockField::total_action() const {
  if (!valid_) return 0.0;
  return std::accumulate(
      sites_.begin(), sites_.end(), 0.0,
      [](double total, const ClockSite& site) {
        return total + site.action;
      });
}

double CoupledClockField::phase_rate(std::size_t site) const {
  if (!valid_ || site >= sites_.size()) return 0.0;
  return std::exp(-sites_[site].compliance)
      * law_.frequency(sites_[site].action);
}

bool CoupledClockField::kick(double step_size) {
  std::vector<double> action_change(sites_.size(), 0.0);
  for (const ClockEdge& edge : edges_) {
    const double phase_difference = sites_[edge.tail].phase
        - sites_[edge.head].phase - edge.connection;
    const double transfer = step_size * stiffness_
        * std::sin(phase_difference);
    action_change[edge.tail] -= transfer;
    action_change[edge.head] += transfer;
  }
  for (std::size_t i = 0; i < sites_.size(); ++i) {
    const double candidate = sites_[i].action + action_change[i];
    if (!(candidate > 0.0) || !std::isfinite(candidate)) return false;
  }
  for (std::size_t i = 0; i < sites_.size(); ++i) {
    sites_[i].action += action_change[i];
  }
  return true;
}

bool CoupledClockField::step(double step_size) {
  if (!valid_ || !(step_size > 0.0) || !std::isfinite(step_size)) return false;
  const std::vector<ClockSite> original = sites_;
  if (!kick(0.5 * step_size)) return false;
  for (std::size_t i = 0; i < sites_.size(); ++i) {
    sites_[i].phase += step_size * phase_rate(i);
    if (!std::isfinite(sites_[i].phase)) {
      sites_ = original;
      return false;
    }
  }
  if (!kick(0.5 * step_size)) {
    sites_ = original;
    return false;
  }
  return true;
}

}  // namespace ftd::eft
