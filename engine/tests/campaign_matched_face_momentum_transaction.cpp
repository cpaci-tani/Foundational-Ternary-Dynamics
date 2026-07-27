/**
 * @file campaign_matched_face_momentum_transaction.cpp
 * @brief FTD-0473 matched local pseudomomentum and hop-recoil gate.
 */

#include "ftd/constants.h"
#include "ftd/eft/matched_face_momentum_transaction.h"
#include "ftd/eft/moore_link_routes.h"
#include "ftd/lattice.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace {

constexpr double kIdentityGate = 1e-10;
constexpr double kWaveRelativeGate = 1e-10;
constexpr double kElectrostaticRecoilCeiling = 1e-8;
constexpr double kElectrostaticWorkFloor = 0.1;
constexpr double kRouteDependenceGate = 1e-6;
constexpr int kWaveTicks = 256;
constexpr int kMode = 2;
constexpr double kWaveAmplitude = 0.02;

int factorial(int value) {
  return value <= 1 ? 1 : value * factorial(value - 1);
}

int nonzero_components(const std::array<int, 3>& delta) {
  return static_cast<int>(std::count_if(
      delta.begin(), delta.end(), [](int value) { return value != 0; }));
}

std::vector<std::array<int, 3>> unique_orders(
    int L, int source, const std::array<int, 3>& delta, int charge) {
  std::vector<std::array<int, 3>> orders;
  std::vector<ftd::eft::DualCellContinuity> histories;
  std::array<int, 3> order{{0, 1, 2}};
  do {
    const auto candidate = ftd::eft::route_single_moore_hop(
        L, source, delta, charge, order);
    bool duplicate = false;
    for (const auto& prior : histories) {
      if (ftd::eft::current_l2_distance(candidate, prior) == 0.0) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) {
      orders.push_back(order);
      histories.push_back(candidate);
    }
  } while (std::next_permutation(order.begin(), order.end()));
  return orders;
}

void add_stationary_countercharge(ftd::eft::DualCellContinuity& history,
                                  int sink, int charge) {
  history.rho_before[static_cast<std::size_t>(sink)] = -charge;
  history.rho_after[static_cast<std::size_t>(sink)] = -charge;
}

double vector_max_difference(const ftd::Vec3& a, const ftd::Vec3& b) {
  return std::max({std::abs(a.x - b.x), std::abs(a.y - b.y),
                   std::abs(a.z - b.z)});
}

void seed_directed_mode(ftd::eft::MatchedFaceFlux& electric,
                        ftd::eft::MatchedEdgeField& magnetic,
                        int propagation_axis) {
  const int L = electric.L;
  const double k = 2.0 * ftd::PI * static_cast<double>(kMode)
      / static_cast<double>(L);
  const int electric_axis = (propagation_axis + 1) % 3;
  const int magnetic_axis = (propagation_axis + 2) % 3;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int coordinate = propagation_axis == 0 ? x
            : (propagation_axis == 1 ? y : z);
        const int index = electric.index(x, y, z);
        const double e = kWaveAmplitude * std::cos(
            k * static_cast<double>(coordinate) - 0.5 * k);
        const double b = kWaveAmplitude * std::cos(
            k * static_cast<double>(coordinate));
        if (electric_axis == 0) electric.x[static_cast<std::size_t>(index)] = e;
        if (electric_axis == 1) electric.y[static_cast<std::size_t>(index)] = e;
        if (electric_axis == 2) electric.z[static_cast<std::size_t>(index)] = e;
        if (magnetic_axis == 0) magnetic.x[static_cast<std::size_t>(index)] = b;
        if (magnetic_axis == 1) magnetic.y[static_cast<std::size_t>(index)] = b;
        if (magnetic_axis == 2) magnetic.z[static_cast<std::size_t>(index)] = b;
      }
    }
  }
}

struct WaveControl {
  int L = 0;
  int axis = 0;
  double initial_magnitude = 0.0;
  double maximum_absolute_drift = 0.0;
  double maximum_relative_drift = 0.0;
  bool valid = false;
};

WaveControl run_wave_control(int L, int axis) {
  ftd::eft::MatchedFaceFlux electric(L);
  ftd::eft::MatchedEdgeField magnetic(L);
  seed_directed_mode(electric, magnetic, axis);
  WaveControl result;
  result.L = L;
  result.axis = axis;
  const auto initial = ftd::eft::matched_local_translation_momentum(
      electric, magnetic);
  result.initial_magnitude = initial.mag();
  for (int tick = 0; tick < kWaveTicks; ++tick) {
    const auto electric_curl = ftd::eft::matched_curl_adjoint(electric);
    for (std::size_t index = 0; index < magnetic.x.size(); ++index) {
      magnetic.x[index] -= ftd::C_SPEED * electric_curl.x[index];
      magnetic.y[index] -= ftd::C_SPEED * electric_curl.y[index];
      magnetic.z[index] -= ftd::C_SPEED * electric_curl.z[index];
    }
    const auto magnetic_curl = ftd::eft::matched_curl(magnetic);
    for (std::size_t index = 0; index < electric.x.size(); ++index) {
      electric.x[index] += ftd::C_SPEED * magnetic_curl.x[index];
      electric.y[index] += ftd::C_SPEED * magnetic_curl.y[index];
      electric.z[index] += ftd::C_SPEED * magnetic_curl.z[index];
    }
    const auto current = ftd::eft::matched_local_translation_momentum(
        electric, magnetic);
    const double drift = (current - initial).mag();
    result.maximum_absolute_drift = std::max(
        result.maximum_absolute_drift, drift);
    result.maximum_relative_drift = std::max(
        result.maximum_relative_drift,
        drift / std::max(1e-30, result.initial_magnitude));
  }
  result.valid = result.initial_magnitude > 1e-8
      && std::isfinite(result.maximum_relative_drift);
  return result;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0473 matched-face momentum transaction v1\n";

  bool protocol_valid = true;
  bool wave_control_pass = true;
  bool exchange_identity_pass = true;
  bool route_count_pass = true;
  double worst_wave_absolute = 0.0;
  double worst_wave_relative = 0.0;
  double worst_source_free_transaction = 0.0;
  double worst_exchange_formula = 0.0;
  double maximum_electrostatic_recoil = 0.0;
  double minimum_electrostatic_work = std::numeric_limits<double>::infinity();
  double maximum_transverse_route_impulse_span = 0.0;
  int transaction_rows = 0;
  int route_groups = 0;
  int transverse_route_dependent_groups = 0;

  for (int L : {16, 17}) {
    for (int axis = 0; axis < 3; ++axis) {
      const auto wave = run_wave_control(L, axis);
      protocol_valid = protocol_valid && wave.valid;
      wave_control_pass = wave_control_pass
          && wave.maximum_absolute_drift <= kIdentityGate
          && wave.maximum_relative_drift <= kWaveRelativeGate;
      worst_wave_absolute = std::max(worst_wave_absolute,
                                      wave.maximum_absolute_drift);
      worst_wave_relative = std::max(worst_wave_relative,
                                      wave.maximum_relative_drift);
      std::cout << "wave,L," << L << ",axis," << axis
                << ",initial," << wave.initial_magnitude
                << ",absolute_drift," << wave.maximum_absolute_drift
                << ",relative_drift," << wave.maximum_relative_drift
                << ",valid," << (wave.valid ? "true" : "false") << '\n';
    }

    ftd::Lattice lattice(L);
    const int source = lattice.index(L / 3, L / 2, L / 4);
    const int sink = lattice.index((L / 3 + L / 2) % L,
                                   (L / 2 + L / 3) % L,
                                   (L / 4 + L / 2 - 1) % L);
    for (int charge : {-1, +1}) {
      std::vector<int> rho(static_cast<std::size_t>(L * L * L), 0);
      rho[static_cast<std::size_t>(source)] = charge;
      rho[static_cast<std::size_t>(sink)] = -charge;
      ftd::eft::MatchedGaussDynamics initializer(L);
      const auto initialization = initializer.initialize_minimum_energy(
          rho, 1e-12, 12 * L);
      protocol_valid = protocol_valid && initialization.valid
          && initialization.converged;
      const ftd::eft::MatchedFaceFlux electrostatic = initializer.electric();
      ftd::eft::MatchedFaceFlux transverse = electrostatic;
      const auto electric_challenge = ftd::eft::make_transverse_challenge(
          L, 0.037);
      protocol_valid = protocol_valid
          && ftd::eft::apply_transverse_curl(
              transverse, electric_challenge) > 0.0;

      for (bool has_transverse : {false, true}) {
        const auto& electric = has_transverse ? transverse : electrostatic;
        const ftd::eft::MatchedEdgeField magnetic = has_transverse
            ? ftd::eft::make_transverse_challenge(L, 0.019)
            : ftd::eft::MatchedEdgeField(L);
        for (int dx = -1; dx <= 1; ++dx) {
          for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
              const std::array<int, 3> delta{{dx, dy, dz}};
              const int components = nonzero_components(delta);
              if (components == 0) continue;
              ++route_groups;
              const auto orders = unique_orders(
                  L, source, delta, charge);
              route_count_pass = route_count_pass
                  && static_cast<int>(orders.size()) == factorial(components);
              std::vector<ftd::Vec3> impulses;
              for (const auto& order : orders) {
                auto history = ftd::eft::route_single_moore_hop(
                    L, source, delta, charge, order);
                add_stationary_countercharge(history, sink, charge);
                const auto transaction =
                    ftd::eft::measure_matched_face_momentum_transaction(
                        electric, magnetic, history, ftd::C_SPEED, 1.0,
                        1e-12);
                ++transaction_rows;
                const double source_free =
                    transaction.source_free_residual.mag();
                worst_source_free_transaction = std::max(
                    worst_source_free_transaction, source_free);
                worst_exchange_formula = std::max(worst_exchange_formula,
                    transaction.formula_residual);
                exchange_identity_pass = exchange_identity_pass
                    && source_free <= kIdentityGate
                    && transaction.formula_residual <= kIdentityGate;
                protocol_valid = protocol_valid && transaction.valid;
                if (!has_transverse) {
                  maximum_electrostatic_recoil = std::max(
                      maximum_electrostatic_recoil,
                      transaction.field_momentum_change.mag());
                  minimum_electrostatic_work = std::min(
                      minimum_electrostatic_work,
                      std::abs(transaction.energy.midpoint_work));
                }
                impulses.push_back(transaction.required_matter_impulse);
                std::cout << "route,arm,"
                          << (has_transverse ? "transverse" : "electrostatic")
                          << ",L," << L << ",charge," << charge
                          << ",delta," << dx << ':' << dy << ':' << dz
                          << ",order," << order[0] << order[1] << order[2]
                          << ",work," << transaction.energy.midpoint_work
                          << ",field_delta,"
                          << transaction.field_momentum_change.x << ':'
                          << transaction.field_momentum_change.y << ':'
                          << transaction.field_momentum_change.z
                          << ",recoil," << transaction.field_momentum_change.mag()
                          << ",source_free," << source_free
                          << ",formula," << transaction.formula_residual
                          << ",valid,"
                          << (transaction.valid ? "true" : "false") << '\n';
              }
              double impulse_span = 0.0;
              for (std::size_t a = 0; a < impulses.size(); ++a) {
                for (std::size_t b = a + 1; b < impulses.size(); ++b) {
                  impulse_span = std::max(impulse_span,
                      vector_max_difference(impulses[a], impulses[b]));
                }
              }
              if (has_transverse && components > 1) {
                maximum_transverse_route_impulse_span = std::max(
                    maximum_transverse_route_impulse_span, impulse_span);
                if (impulse_span > kRouteDependenceGate)
                  ++transverse_route_dependent_groups;
              }
              std::cout << "group,arm,"
                        << (has_transverse ? "transverse" : "electrostatic")
                        << ",L," << L << ",charge," << charge
                        << ",delta," << dx << ':' << dy << ':' << dz
                        << ",components," << components
                        << ",routes," << orders.size()
                        << ",impulse_span," << impulse_span << '\n';
            }
          }
        }
      }
    }
  }

  const bool electrostatic_recoil_absent =
      maximum_electrostatic_recoil <= kElectrostaticRecoilCeiling
      && minimum_electrostatic_work > kElectrostaticWorkFloor;
  const bool transverse_route_dependence =
      transverse_route_dependent_groups > 0
      && maximum_transverse_route_impulse_span > kRouteDependenceGate;
  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID";
  } else if (wave_control_pass && exchange_identity_pass && route_count_pass
             && electrostatic_recoil_absent
             && transverse_route_dependence) {
    verdict = "LOCAL_PSEUDOMOMENTUM_EXACT_ELECTROSTATIC_HOP_RECOIL_ABSENT";
  } else {
    verdict = "MATCHED_FACE_MOMENTUM_TRANSACTION_CLAIM_FAILS";
  }

  std::cout << "summary,transaction_rows," << transaction_rows
            << ",route_groups," << route_groups
            << ",transverse_route_dependent_groups,"
            << transverse_route_dependent_groups
            << ",worst_wave_absolute," << worst_wave_absolute
            << ",worst_wave_relative," << worst_wave_relative
            << ",worst_source_free_transaction,"
            << worst_source_free_transaction
            << ",worst_exchange_formula," << worst_exchange_formula
            << ",maximum_electrostatic_recoil,"
            << maximum_electrostatic_recoil
            << ",minimum_electrostatic_work,"
            << minimum_electrostatic_work
            << ",maximum_transverse_route_impulse_span,"
            << maximum_transverse_route_impulse_span
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
