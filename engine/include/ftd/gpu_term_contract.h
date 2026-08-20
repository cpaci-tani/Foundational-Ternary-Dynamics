#pragma once
// Live CUDA implementation class for every TOGGLE_SPECS row.
//
// The ToggleBackend bitmask is what the engine *claims*. This table is
// what the engine *does*. test_gpu_term_contract.cpp fails closed when
// the two disagree. Ports update the matching row in the same change as
// the kernel — never leave ANY on a CPU-only, intent-flag, or no-op term.
//
// This is a completeness contract, not a derivation. Selected/imposed
// CUDA ports stay at their LEDGER tags.

#include "term_toggles.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ftd {

enum class GpuTermImpl : std::uint8_t {
    NativeCuda,          // device kernel; no host fallback required
    GpuOnlyNoCpu,        // CUDA kernel; CPU is an advertised no-op
    CpuOnly,             // forces CPU backend when enabled
    CpuFallbackSync,     // CUDA acknowledges the term by syncing to CPU
    HostMirrorHybrid,    // GPU tick then full AoS host mirror
    IntentFlag,          // no physics branch (unused; confinement is now NativeCuda)
    ControlOnly,         // validator / diagnostics, not a field term
};

struct GpuTermContract {
    const char* name;
    GpuTermImpl impl;
    std::uint8_t declared_backends;
};

// Must stay 1:1 with TOGGLE_SPECS, same order, same names.
inline constexpr GpuTermContract GPU_TERM_CONTRACT[] = {
    {"wave_propagation", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"coupling", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"damping", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"genesis", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"evaporation", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"gauss_projection", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"forces", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"gravity", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"poisson_coulomb", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"movement", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"lorentz_force", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"selective_damping", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"larmor_radiation", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"dual_substrate", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"color_forces", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"strong_stress_energy", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"weak_transmutation", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"strong_force", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"triad_binding", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"pair_production", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"exchange_force", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"latency_field", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"exact_dual_gauss", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"matched_gauss_dynamics", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"emergent_forces", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"langevin", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"symplectic_leapfrog", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"verlet_wave_integrator", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"lorentz_period2_floquet", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"lorentz_bcc_time_floquet", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"su2_gauge", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"su3_gauge", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"symmetric_movement_order", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"absorbing_boundary", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"reflective_boundary", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"field_energy_gravity", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"cluster_inertia", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"geometric_gravity", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"de_broglie_clock", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"db_clock_coulomb", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"confinement", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"knot_tracking", GpuTermImpl::HostMirrorHybrid, ToggleBackend::ANY},
    {"strict_validation", GpuTermImpl::ControlOnly, ToggleBackend::ANY},
    {"ew_background_sweep", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
};

static_assert(
    sizeof(GPU_TERM_CONTRACT) / sizeof(GPU_TERM_CONTRACT[0])
        == sizeof(TOGGLE_SPECS) / sizeof(TOGGLE_SPECS[0]),
    "GPU_TERM_CONTRACT must stay 1:1 with TOGGLE_SPECS");

inline constexpr std::size_t toggle_spec_count() {
    return sizeof(TOGGLE_SPECS) / sizeof(TOGGLE_SPECS[0]);
}

inline constexpr std::size_t gpu_term_contract_count() {
    return sizeof(GPU_TERM_CONTRACT) / sizeof(GPU_TERM_CONTRACT[0]);
}

inline const GpuTermContract* find_gpu_term_contract(std::string_view name) {
    for (const auto& row : GPU_TERM_CONTRACT) {
        if (name == row.name) return &row;
    }
    return nullptr;
}

}  // namespace ftd
