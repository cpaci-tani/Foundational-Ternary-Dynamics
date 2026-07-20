#pragma once
// FTD matter-sector clock rate — THE single definition (2026-07-19).
//
// dτ/dt for a manifested voxel, exactly as the proper-time accumulator has
// always computed it (transmutation_phases.cpp accumulate_proper_time,
// FTD-0252 measured dτ/dt ∝ √(1−v²) at L=0; the 0.004 % gravitational
// dilation match and the FTD-0268 blind L=257 confirmation validate the
// latency part):
//
//     f = 1 − L²          (latency consumes the budget)
//     dτ/dt = √(f² − v²) / √f      with the c = 1 flux-velocity
//                                   normalization of the proper-time sector
//     frozen clock (f ≤ 0 or f² ≤ v²)  ⇒  dτ/dt = 0
//
// NOTE the c = 1 normalization is the proper-time sector's own convention,
// deliberately distinct from the transport cap |v| ≤ C_SPEED·√(1−L²) in
// phase_forces.cpp — see the do-not-couple note there. This header does not
// couple them; it makes the EXISTING clock the one clock every consumer
// reads.
//
// Consumers (keep this list current):
//   - accumulate_proper_time (transmutation_phases.cpp): v.tau += dτ, and the
//     FTD-0271 de Broglie phase advance dφ = ω₀·dτ.
//   - the evaporation hazard, CPU (phase_write.cpp) and GPU
//     (kernels_stencil_single.cu / kernels_stencil_dual.cu): per-tick decay
//     probability p = K_EVAP_RATE·exp(−E_local/K_MANIFEST²)·(dτ/dt) — the
//     2026-07-19 proper-time-hazard amendment. Provenance: the two-clock
//     consistency campaign (PREREG_TWO_CLOCK_CONSISTENCY_v1, Outcome A,
//     2026-07-18: 1,355 paired voxels, zero decay-tick differences across a
//     latency contrast of 0.62 — matter aged on tick time while the motion
//     sector ran on τ) and the owner's ruling on its decision point. With
//     this factor, decay statistics ARE proper-time clocks: a metastable
//     population in a well decays slower by √(1−L²) at rest, and a moving
//     population by the SR factor — the muon-storage-ring behaviour.
//   - Genesis is deliberately NOT a consumer: manifestation is a
//     field-sector nucleation process at void sites (no τ to integrate);
//     whether nucleation should dilate in a well is a separate [OPEN]
//     question, not silently decided here.
//
// Shared CPU/GPU per the voxel_rng.h idiom: bit-identical math on both
// backends (sqrt is correctly-rounded per IEEE-754 on host and device).

#ifndef __CUDACC__
#include <cmath>
#endif

#ifdef __CUDACC__
#define FTD_PTR_HD __host__ __device__ __forceinline__
#else
#define FTD_PTR_HD inline
#endif

namespace ftd {

FTD_PTR_HD double proper_time_rate(double latency, double speed2) {
    const double f = 1.0 - latency * latency;
    if (f <= 0.0) return 0.0;               // horizon: clock frozen
    const double arg = f * f - speed2;
    if (arg <= 0.0) return 0.0;             // c=1 kinematic freeze
#ifdef __CUDACC__
    return sqrt(arg) / sqrt(f);
#else
    return std::sqrt(arg) / std::sqrt(f);
#endif
}

}  // namespace ftd
