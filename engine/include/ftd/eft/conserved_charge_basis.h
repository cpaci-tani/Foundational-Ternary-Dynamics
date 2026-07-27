#pragma once
/**
 * @file ftd/eft/conserved_charge_basis.h
 * @brief Exact additive-charge nullspace for the frozen native event catalog.
 *
 * The preregistered local feature vector is
 *
 *   (occupancy, signed state, chirality sign, state*chirality sign).
 *
 * Rows of the transition matrix are exact integer changes of the global
 * feature sums under allowed production events. No flux magnitude, fitted
 * function, physical constant, or target observable enters the calculation.
 */

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ftd {
namespace eft {

constexpr int NATIVE_CHARGE_FEATURES = 4;
using ChargeVector = std::array<std::int64_t, NATIVE_CHARGE_FEATURES>;

struct ChargeTransition {
    std::string name;
    ChargeVector delta{};
};

struct ConservedChargeBasis {
    int rank = 0;
    int nullity = 0;
    std::vector<ChargeVector> integer_basis;
};

std::vector<ChargeTransition> frozen_native_charge_transitions();
ConservedChargeBasis solve_conserved_charge_basis(
    const std::vector<ChargeTransition>& transitions);

}  // namespace eft
}  // namespace ftd
