#pragma once

// Passive exact bookkeeping for the existing, unadopted Q4 candidate.
// No transition, retained physical state, or particle identification lives here.
#include "sparse_q4_kernel_v1.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace ftd::carrier_current {

template<class T, std::size_t Capacity> struct SparseRows {
    std::array<T, Capacity> rows{};
    std::size_t count = 0;
};

struct Context {
    const char* law_id = q4native_v1::LAW_ID;
    const char* boundary = "periodic";
    const char* number_units = "integer carrier count by physical polarity slot";
    const char* account_units = "carrier credits plus owned signed-edge label squared";
    std::uint16_t L = 0;
    std::uint8_t origin_code = 0, eta = 0;
    std::string start_tick_hex, end_tick_hex;
    bool mechanical_energy_available = false;
    bool mechanical_momentum_available = false;
    bool spin_available = false;
    bool rest_mass_available = false;
    bool particle_identification_available = false;
};

struct SiteBalance {
    std::uint32_t site = 0;
    std::array<std::int32_t, 2> before_number{}, after_number{};
    std::array<std::int32_t, 2> number_divergence{}, number_residual{};
    std::int32_t before_credit = 0, after_credit = 0;
    std::int32_t before_flux_account = 0, after_flux_account = 0;
    std::int32_t account_divergence = 0, account_residual = 0;
};

struct EdgeBalance {
    std::uint32_t owner = 0, head = 0;
    std::uint8_t axis = 0;
    std::int32_t before_flux = 0, after_flux = 0;
    std::array<std::int32_t, 2> number_current{};
    std::int32_t signed_charge_current = 0, account_current = 0;
    std::int32_t flux_residual = 0;
};

struct Observation {
    Context context;
    SparseRows<SiteBalance, 32> sites;
    SparseRows<EdgeBalance, 16> edges;
    // The observer certifies the number/credit/flux accounts of the supplied
    // transaction. It does not certify every heading/attempt update in Phi.
    bool complete_transition_law_validated = false;
};

// Rejects inconsistent accounts/events/context before publishing any result.
// Neither state nor the event bundle is changed. Throws std::invalid_argument.
Observation observe(const q4native_v1::State& before,
                    const q4native_v1::State& after,
                    const q4native_v1::Events& events);

struct BoundaryCurrent {
    std::uint32_t owner_site = 0, head_site = 0;
    std::uint32_t owner_block = 0, head_block = 0;
    std::uint8_t axis = 0;
    std::array<std::int32_t, 2> number_current{};
    std::int32_t signed_charge_current = 0, account_current = 0;
};

struct BlockObservation {
    Context context;
    unsigned block_width = 0;
    // SiteBalance::site is the x-major block index in this observation.
    SparseRows<SiteBalance, 32> blocks;
    SparseRows<BoundaryCurrent, 16> boundary_currents;
};

BlockObservation restrict_blocks(const Observation&, unsigned block_width);

struct AdvectiveMultiplier {
    // exp(2*pi*i*residue/denominator), for the convention exp(-i*k.x).
    unsigned residue = 0, denominator = 1;
    bool rest_mass_dispersion_available = false;
};
AdvectiveMultiplier advective_multiplier(unsigned L,
    const std::array<std::int64_t, 3>& integer_wavevector,
    const std::array<std::int64_t, 3>& translation);

} // namespace ftd::carrier_current
