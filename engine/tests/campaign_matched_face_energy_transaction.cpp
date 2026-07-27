/**
 * @file campaign_matched_face_energy_transaction.cpp
 * @brief FTD-0472 exact face-current energy and Moore-route ambiguity.
 */

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/moore_link_routes.h"
#include "ftd/lattice.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kIdentityGate = 1e-10;
constexpr double kGaussGate = 1e-9;
constexpr double kInverseGate = 1e-12;
constexpr double kPathClosureGate = 1e-9;
constexpr double kPathDependenceGate = 1e-6;
constexpr double kNaiveFailureGate = 1e-3;

struct RouteRecord {
  std::array<int, 3> order{};
  ftd::eft::DualCellContinuity history;
  ftd::eft::MatchedFaceEnergyTransaction transaction;
};

int factorial(int value) {
  return value <= 1 ? 1 : value * factorial(value - 1);
}

int nonzero_components(const std::array<int, 3>& delta) {
  return static_cast<int>(std::count_if(
      delta.begin(), delta.end(), [](int value) { return value != 0; }));
}

std::vector<std::array<int, 3>> unique_orders(
    int L, int source, const std::array<int, 3>& delta, int charge) {
  std::vector<std::array<int, 3>> result;
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
      result.push_back(order);
      histories.push_back(candidate);
    }
  } while (std::next_permutation(order.begin(), order.end()));
  return result;
}

void add_stationary_countercharge(ftd::eft::DualCellContinuity& history,
                                  int sink, int charge) {
  history.rho_before[static_cast<std::size_t>(sink)] = -charge;
  history.rho_after[static_cast<std::size_t>(sink)] = -charge;
}

const char* arm_name(bool transverse) {
  return transverse ? "transverse" : "electrostatic";
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0472 matched-face energy transaction v1\n";

  bool protocol_valid = true;
  bool identity_pass = true;
  bool inverse_pass = true;
  bool gauss_pass = true;
  bool route_count_pass = true;
  bool electrostatic_path_closure_pass = true;
  bool face_unique_pass = true;
  double worst_balance = 0.0;
  double worst_current_balance = 0.0;
  double worst_source_free = 0.0;
  double worst_naive_formula = 0.0;
  double worst_inverse = 0.0;
  double worst_gauss = 0.0;
  double maximum_naive_failure = 0.0;
  double maximum_transverse_route_span = 0.0;
  double maximum_multiroute_field_difference = 0.0;
  int transaction_rows = 0;
  int route_groups = 0;
  int multiroute_groups = 0;
  int transverse_path_dependent_groups = 0;

  for (int L : {16, 17}) {
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
      const double injected = ftd::eft::apply_transverse_curl(
          transverse, electric_challenge);
      protocol_valid = protocol_valid && injected > 0.0;

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
              if (components > 1) ++multiroute_groups;
              const auto orders = unique_orders(
                  L, source, delta, charge);
              const int expected_routes = factorial(components);
              route_count_pass = route_count_pass
                  && static_cast<int>(orders.size()) == expected_routes;

              std::vector<RouteRecord> records;
              double minimum_work = std::numeric_limits<double>::infinity();
              double maximum_work = -std::numeric_limits<double>::infinity();
              for (const auto& order : orders) {
                auto history = ftd::eft::route_single_moore_hop(
                    L, source, delta, charge, order);
                add_stationary_countercharge(history, sink, charge);
                auto transaction =
                    ftd::eft::measure_matched_face_energy_transaction(
                        electric, magnetic, history, ftd::C_SPEED, 1.0,
                        1e-12);
                ++transaction_rows;
                minimum_work = std::min(minimum_work,
                                         transaction.midpoint_work);
                maximum_work = std::max(maximum_work,
                                         transaction.midpoint_work);
                worst_balance = std::max(worst_balance,
                    std::abs(transaction.balance_residual));
                worst_current_balance = std::max(worst_current_balance,
                    std::abs(transaction.current_balance_residual));
                worst_source_free = std::max(worst_source_free,
                    std::abs(transaction.source_free_residual));
                worst_naive_formula = std::max(worst_naive_formula,
                    std::abs(transaction.naive_formula_residual));
                worst_inverse = std::max(worst_inverse,
                    transaction.inverse_residual);
                worst_gauss = std::max({worst_gauss,
                    transaction.gauss_before, transaction.gauss_after,
                    transaction.continuity_residual});
                maximum_naive_failure = std::max(maximum_naive_failure,
                    std::abs(transaction.naive_balance_residual));
                protocol_valid = protocol_valid && transaction.valid;
                identity_pass = identity_pass
                    && std::abs(transaction.balance_residual) <= kIdentityGate
                    && std::abs(transaction.current_balance_residual)
                        <= kIdentityGate
                    && std::abs(transaction.source_free_residual)
                        <= kIdentityGate
                    && std::abs(transaction.naive_formula_residual)
                        <= kIdentityGate;
                inverse_pass = inverse_pass
                    && transaction.inverse_residual <= kInverseGate;
                gauss_pass = gauss_pass
                    && transaction.gauss_before <= kGaussGate
                    && transaction.gauss_after <= kGaussGate
                    && transaction.continuity_residual <= kIdentityGate;
                std::cout << "route,arm," << arm_name(has_transverse)
                          << ",L," << L << ",charge," << charge
                          << ",delta," << dx << ':' << dy << ':' << dz
                          << ",order," << order[0] << order[1] << order[2]
                          << ",routes," << orders.size()
                          << ",support," << transaction.current_support
                          << ",work," << transaction.midpoint_work
                          << ",balance," << transaction.balance_residual
                          << ",current_balance,"
                          << transaction.current_balance_residual
                          << ",source_free,"
                          << transaction.source_free_residual
                          << ",naive_balance,"
                          << transaction.naive_balance_residual
                          << ",naive_formula,"
                          << transaction.naive_formula_residual
                          << ",inverse," << transaction.inverse_residual
                          << ",gauss_before," << transaction.gauss_before
                          << ",gauss_after," << transaction.gauss_after
                          << ",valid,"
                          << (transaction.valid ? "true" : "false") << '\n';
                records.push_back({order, std::move(history),
                                   std::move(transaction)});
              }

              const double work_span = maximum_work - minimum_work;
              double field_difference = 0.0;
              for (std::size_t index = 1; index < records.size(); ++index) {
                field_difference = std::max(field_difference,
                    ftd::eft::matched_face_max_difference(
                        records[0].transaction.electric_after,
                        records[index].transaction.electric_after));
              }
              if (components == 1)
                face_unique_pass = face_unique_pass
                    && orders.size() == 1 && work_span <= kIdentityGate
                    && field_difference <= kIdentityGate;
              if (!has_transverse)
                electrostatic_path_closure_pass =
                    electrostatic_path_closure_pass
                    && work_span <= kPathClosureGate;
              if (has_transverse && components > 1) {
                maximum_transverse_route_span = std::max(
                    maximum_transverse_route_span, work_span);
                if (work_span > kPathDependenceGate)
                  ++transverse_path_dependent_groups;
              }
              if (components > 1)
                maximum_multiroute_field_difference = std::max(
                    maximum_multiroute_field_difference, field_difference);
              std::cout << "group,arm," << arm_name(has_transverse)
                        << ",L," << L << ",charge," << charge
                        << ",delta," << dx << ':' << dy << ':' << dz
                        << ",components," << components
                        << ",routes," << orders.size()
                        << ",work_span," << work_span
                        << ",field_difference," << field_difference << '\n';
            }
          }
        }
      }
    }
  }

  const bool path_selection_exposed = transverse_path_dependent_groups > 0
      && maximum_transverse_route_span > kPathDependenceGate
      && maximum_multiroute_field_difference > kPathDependenceGate;
  const bool naive_rule_fails = maximum_naive_failure > kNaiveFailureGate;
  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID";
  } else if (identity_pass && inverse_pass && gauss_pass && route_count_pass
             && electrostatic_path_closure_pass && face_unique_pass
             && path_selection_exposed && naive_rule_fails) {
    verdict = "MIDPOINT_WORK_EXACT_MOORE_ROUTE_TYPE_STILL_REQUIRED";
  } else {
    verdict = "MATCHED_FACE_ENERGY_TRANSACTION_CLAIM_FAILS";
  }

  std::cout << "summary,transaction_rows," << transaction_rows
            << ",route_groups," << route_groups
            << ",multiroute_groups," << multiroute_groups
            << ",transverse_path_dependent_groups,"
            << transverse_path_dependent_groups
            << ",worst_balance," << worst_balance
            << ",worst_current_balance," << worst_current_balance
            << ",worst_source_free," << worst_source_free
            << ",worst_naive_formula," << worst_naive_formula
            << ",worst_inverse," << worst_inverse
            << ",worst_gauss," << worst_gauss
            << ",maximum_naive_failure," << maximum_naive_failure
            << ",maximum_transverse_route_span,"
            << maximum_transverse_route_span
            << ",maximum_multiroute_field_difference,"
            << maximum_multiroute_field_difference
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
