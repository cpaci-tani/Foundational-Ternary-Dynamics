#pragma once
/**
 * @file coupled_quartic_clock_field.h
 * @brief FTD-0770 selected coupled-clock EFT probe.
 *
 * This module is deliberately isolated from RenderBridge and Voxel.  Its
 * action-angle variables, compliance, stiffness, and edge connection are
 * imposed candidate types, not production FTD state.
 */

#include <cstddef>
#include <vector>

namespace ftd::eft {

struct PowerClockLaw {
  int exponent = 0;
  double action_coefficient = 0.0;
  double action_energy_exponent = 0.0;
  double energy_action_exponent = 0.0;
  double unit_shell_period = 0.0;
  bool valid = false;

  double action_from_energy(double energy) const;
  double energy_from_action(double action) const;
  double frequency(double action) const;
  double curvature(double action) const;
  double period(double energy) const;
};

PowerClockLaw make_even_power_clock_law(int exponent);

double linear_wave_cycle_ratio_squared(int exponent, double kappa_over_energy);
double axial_continuum_neighbor_factor();
double full_moore_continuum_neighbor_factor(int dimension);

struct ClockSite {
  double action = 0.0;
  double phase = 0.0;
  double compliance = 0.0;
};

struct ClockEdge {
  std::size_t tail = 0;
  std::size_t head = 0;
  double connection = 0.0;
};

struct ConnectionIntegrabilityResult {
  std::vector<double> gauge_offsets;
  int component_count = 0;
  double maximum_cycle_residual = 0.0;
  bool integrable = false;
  bool valid = false;
};

ConnectionIntegrabilityResult analyze_connection_integrability(
    std::size_t site_count,
    const std::vector<ClockEdge>& edges,
    double tolerance = 1e-12);

class CoupledClockField {
 public:
  CoupledClockField(PowerClockLaw law,
                    std::vector<ClockSite> sites,
                    std::vector<ClockEdge> edges,
                    double stiffness);

  bool valid() const { return valid_; }
  bool step(double step_size);

  double hamiltonian() const;
  double total_action() const;
  double phase_rate(std::size_t site) const;

  const PowerClockLaw& law() const { return law_; }
  const std::vector<ClockSite>& sites() const { return sites_; }
  const std::vector<ClockEdge>& edges() const { return edges_; }
  double stiffness() const { return stiffness_; }

 private:
  bool kick(double step_size);

  PowerClockLaw law_;
  std::vector<ClockSite> sites_;
  std::vector<ClockEdge> edges_;
  double stiffness_ = 0.0;
  bool valid_ = false;
};

}  // namespace ftd::eft
