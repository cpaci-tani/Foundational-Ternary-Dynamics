#pragma once
// Live CUDA implementation class for every TOGGLE_SPECS row.
//
// The ToggleBackend bitmask is what the engine *claims*. This table is
// what the engine *does*. test_gpu_term_contract.cpp fails closed when
// the two disagree. Ports update the matching row in the same change as
// the kernel — never leave ANY on a CPU-only or host-mirror term.
//
// This is a completeness contract, not a derivation. Selected/imposed
// CUDA ports stay at their LEDGER tags.
//
// HONEST LIMITS OF "WHAT THE ENGINE DOES" (both added 2026-09-02; neither
// changes any row's classification below):
//
//   (1) NativeCuda does not mean "the same algorithm at the same precision
//       as CPU." The three Poisson-based rows below (gauss_projection,
//       poisson_coulomb, latency_field) run algorithmically DIFFERENT
//       solvers on the two backends: CPU is a warm-started 18-point SOR
//       sweep in DOUBLE precision (src/poisson_solvers.cpp); GPU is an
//       EXACT spectral FFT solve in SINGLE precision
//       (cuda/kernels_poisson.cu — the CUDA source's own comment calls
//       float's ~7 decimal digits "more than sufficient" for the
//       correction gradient). latency_field further differs in HOW the
//       periodic solvability condition is met: CPU explicitly
//       mean-subtracts the source/potential; GPU instead relies on the
//       precomputed Green's function zeroing the DC Fourier mode. The two
//       are equivalent in exact arithmetic, not bit-for-bit in float.
//       Parity between backends on these three terms is therefore
//       STATISTICAL BY DESIGN, not exact: the CPU/GPU parity test uses
//       2-5% family tolerances, with its own comment noting that FFT and
//       SOR will differ. Do not read a NativeCuda tag on these three rows
//       as a claim of bit-identical (or even algorithmically identical)
//       output to the CPU path.
//
//   (2) test_gpu_term_contract.cpp (the "oracle" above) does not exercise
//       any of this. It includes no CUDA header, launches no kernel, and
//       never compares CPU output against GPU output — it only checks this
//       table's rows against TOGGLE_SPECS and the toggle validator. A row
//       can therefore read NativeCuda while the kernel behind it is
//       missing, wrong, or (as in (1) above) a different algorithm at a
//       different precision, and this test still passes. Read this table
//       as an audited CLAIM ledger, not as evidence that any claim here was
//       checked against a running kernel.

#include "term_toggles.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ftd {

enum class GpuTermImpl : std::uint8_t {
    NativeCuda,          // device kernel; no host fallback required
    CpuOnly,             // forces CPU backend when enabled
    CpuFallbackSync,     // CUDA acknowledges the term by syncing to CPU
    HostMirrorHybrid,    // GPU tick then full AoS host mirror
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
    // See header note (1): CPU 18-pt SOR (double) vs. GPU spectral FFT
    // (single) — statistical parity only, not the same algorithm.
    {"gauss_projection", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"forces", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    {"gravity", GpuTermImpl::NativeCuda, ToggleBackend::ANY},
    // See header note (1): CPU 18-pt SOR (double) vs. GPU spectral FFT
    // (single) — statistical parity only, not the same algorithm.
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
    // See header note (1): CPU 18-pt SOR (double, explicit mean-subtract) vs.
    // GPU spectral FFT (single, DC-mode zeroed by the Green's function) —
    // equivalent in exact arithmetic only; statistical parity in float.
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
    // Flux-cell mechanisms (ftd/flux_cell.h): both edit / read the host
    // mirror (pump: every pump tick; port: every tick once open), then the
    // GPU tick uploads the dirty mirror. Same class as knot_tracking.
    {"flux_pump", GpuTermImpl::HostMirrorHybrid, ToggleBackend::ANY},
    {"flux_cell_port", GpuTermImpl::HostMirrorHybrid, ToggleBackend::ANY},
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
