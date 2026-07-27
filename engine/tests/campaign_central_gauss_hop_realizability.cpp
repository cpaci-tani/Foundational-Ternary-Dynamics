/**
 * @file campaign_central_gauss_hop_realizability.cpp
 * @brief FTD-0471 one-site Gauss transport under central versus face fields.
 */

#include "ftd/eft/central_gauss_hop_transport.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/moore_link_routes.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kGate = 1e-12;
constexpr double kPi = 3.141592653589793238462643383279502884;

int center_index(const ftd::Lattice& lattice) {
  const int center = lattice.size() / 2;
  return lattice.index(center, center, center);
}

int nonzero_face_support(const ftd::eft::MatchedFaceFlux& field) {
  int support = 0;
  for (std::size_t index = 0; index < field.x.size(); ++index) {
    if (field.x[index] != 0.0) ++support;
    if (field.y[index] != 0.0) ++support;
    if (field.z[index] != 0.0) ++support;
  }
  return support;
}

long double central_parity_null_test(int L, int axis) {
  ftd::RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double phase = 2.0 * kPi
        * static_cast<double>(coordinate.x + 3 * coordinate.y
                              + 5 * coordinate.z)
        / static_cast<double>(L);
    bridge.voxels()[static_cast<std::size_t>(index)].flux = {
        0.013 * std::sin(phase) + 0.007 * std::cos(2.0 * phase),
        -0.011 * std::cos(phase) + 0.005 * std::sin(3.0 * phase),
        0.017 * std::sin(2.0 * phase) - 0.003 * std::cos(phase)};
  }
  long double pairing = 0.0L;
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    pairing += static_cast<long double>(ftd::eft::parity_character(
        bridge.lattice().coord(index), axis))
        * static_cast<long double>(bridge.divergence_flux(index));
  }
  return pairing;
}

struct FaceResult {
  double continuity_residual = 0.0;
  double gauss_residual = 0.0;
  int current_support = 0;
  int field_support = 0;
  bool valid = false;
};

FaceResult run_matched_face(int L, int axis, int direction, int charge) {
  FaceResult result;
  ftd::eft::MatchedFaceFlux field(L);
  ftd::Lattice lattice(L);
  const int source = center_index(lattice);
  std::array<int, 3> delta{{0, 0, 0}};
  delta[static_cast<std::size_t>(axis)] = direction;
  const auto history = ftd::eft::route_single_moore_hop(
      L, source, delta, charge, {{0, 1, 2}});
  const auto update = ftd::eft::apply_conservative_current(field, history);
  std::vector<int> desired(static_cast<std::size_t>(L * L * L), 0);
  for (std::size_t index = 0; index < desired.size(); ++index)
    desired[index] = history.rho_after[index] - history.rho_before[index];
  result.continuity_residual = update.transport_residual;
  result.gauss_residual = ftd::eft::max_gauss_residual(field, desired);
  result.current_support = static_cast<int>(std::count_if(
      history.current_x.begin(), history.current_x.end(),
      [](double value) { return value != 0.0; }))
      + static_cast<int>(std::count_if(
          history.current_y.begin(), history.current_y.end(),
          [](double value) { return value != 0.0; }))
      + static_cast<int>(std::count_if(
          history.current_z.begin(), history.current_z.end(),
          [](double value) { return value != 0.0; }));
  result.field_support = nonzero_face_support(field);
  result.valid = update.valid;
  return result;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0471 central-Gauss hop realizability v1\n";

  bool protocol_valid = true;
  bool even_obstruction_pass = true;
  bool odd_construction_pass = true;
  bool face_local_pass = true;
  double worst_even_null_pairing = 0.0;
  double worst_odd_gauss = 0.0;
  double worst_face_gauss = 0.0;
  double worst_face_continuity = 0.0;
  int minimum_odd_support = std::numeric_limits<int>::max();
  int maximum_odd_support = 0;

  for (int L : {16, 32}) {
    ftd::Lattice lattice(L);
    const int source = center_index(lattice);
    for (int axis = 0; axis < 3; ++axis) {
      const long double null_pairing = central_parity_null_test(L, axis);
      worst_even_null_pairing = std::max(worst_even_null_pairing,
          std::abs(static_cast<double>(null_pairing)));
      for (int direction : {-1, +1}) {
        for (int charge : {-1, +1}) {
          const auto result = ftd::eft::construct_central_gauss_face_hop(
              L, source, axis, direction, charge);
          protocol_valid = protocol_valid && result.valid;
          even_obstruction_pass = even_obstruction_pass
              && !result.realizable
              && std::abs(std::abs(result.desired_parity_pairing) - 2.0)
                  <= kGate
              && std::abs(static_cast<double>(null_pairing)) <= kGate;
          std::cout << "central_even,L," << L
                    << ",axis," << axis
                    << ",direction," << direction
                    << ",charge," << charge
                    << ",desired_parity_pairing,"
                    << result.desired_parity_pairing
                    << ",arbitrary_field_null_pairing,"
                    << static_cast<double>(null_pairing)
                    << ",realizable," << (result.realizable ? "true" : "false")
                    << '\n';
        }
      }
    }
  }

  for (int L : {17, 33, 65}) {
    ftd::Lattice lattice(L);
    const int source = center_index(lattice);
    for (int axis = 0; axis < 3; ++axis) {
      for (int direction : {-1, +1}) {
        for (int charge : {-1, +1}) {
          const auto result = ftd::eft::construct_central_gauss_face_hop(
              L, source, axis, direction, charge);
          const int expected_support = (L - 1) / 2;
          protocol_valid = protocol_valid && result.valid;
          odd_construction_pass = odd_construction_pass
              && result.realizable
              && result.graph_steps == expected_support
              && result.support_sites == expected_support
              && result.gauss_residual <= kGate;
          worst_odd_gauss = std::max(worst_odd_gauss,
                                      result.gauss_residual);
          minimum_odd_support = std::min(minimum_odd_support,
                                          result.support_sites);
          maximum_odd_support = std::max(maximum_odd_support,
                                          result.support_sites);
          std::cout << "central_odd,L," << L
                    << ",axis," << axis
                    << ",direction," << direction
                    << ",charge," << charge
                    << ",graph_steps," << result.graph_steps
                    << ",support_sites," << result.support_sites
                    << ",gauss_residual," << result.gauss_residual
                    << ",realizable," << (result.realizable ? "true" : "false")
                    << '\n';
        }
      }
    }
  }

  for (int L : {16, 17, 32, 33, 65}) {
    for (int axis = 0; axis < 3; ++axis) {
      for (int direction : {-1, +1}) {
        for (int charge : {-1, +1}) {
          const auto result = run_matched_face(L, axis, direction, charge);
          protocol_valid = protocol_valid && result.valid;
          face_local_pass = face_local_pass
              && result.continuity_residual <= kGate
              && result.gauss_residual <= kGate
              && result.current_support == 1
              && result.field_support == 1;
          worst_face_gauss = std::max(worst_face_gauss,
                                       result.gauss_residual);
          worst_face_continuity = std::max(worst_face_continuity,
                                            result.continuity_residual);
          std::cout << "matched_face,L," << L
                    << ",axis," << axis
                    << ",direction," << direction
                    << ",charge," << charge
                    << ",continuity_residual," << result.continuity_residual
                    << ",gauss_residual," << result.gauss_residual
                    << ",current_support," << result.current_support
                    << ",field_support," << result.field_support
                    << '\n';
        }
      }
    }
  }

  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID";
  } else if (even_obstruction_pass && odd_construction_pass
             && face_local_pass) {
    verdict = "CENTRAL_GAUSS_HOP_EVEN_IMPOSSIBLE_ODD_NONLOCAL_FACE_LOCAL";
  } else {
    verdict = "CENTRAL_GAUSS_HOP_REALIZABILITY_CLAIM_FAILS";
  }

  std::cout << "summary,worst_even_null_pairing,"
            << worst_even_null_pairing
            << ",worst_odd_gauss," << worst_odd_gauss
            << ",minimum_odd_support," << minimum_odd_support
            << ",maximum_odd_support," << maximum_odd_support
            << ",worst_face_continuity," << worst_face_continuity
            << ",worst_face_gauss," << worst_face_gauss
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
