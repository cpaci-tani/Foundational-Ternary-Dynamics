// Measured FTD-0252 kinematic time-dilation data — OFFLINE campaign, NOT live.
// [M] measured (engine-derived), but pre-computed: commit 7ed9adf3,
// engine/results/time_dilation_v2_2026-06-07/. See LEDGER FTD-0252.
//
// A moving wave-clock (effective mass from transverse momentum, built on the
// massless lattice) dilates as sqrt(1-v^2). On the <100> axis the departure from
// exact gamma vanishes as L^-2 as the mode softens (k->0) -> gamma emerges in the IR.

export const FTD0252_PROVENANCE = 'FTD-0252 campaign (offline) — commit 7ed9adf3';

// Measured dilation (dtau/dt = omega_proper/omega_0) vs velocity v=v_g/c,
// <100> axis, L=129, n_perp=16. Representative subset.
export const DILATION_VS_V = [
  { v: 0.058, dilation: 0.99816 },
  { v: 0.170, dilation: 0.98385 },
  { v: 0.274, dilation: 0.95737 },
  { v: 0.366, dilation: 0.92235 },
  { v: 0.443, dilation: 0.88291 },
  { v: 0.506, dilation: 0.84287 },
  { v: 0.555, dilation: 0.80536 },
  { v: 0.639, dilation: 0.71701 },
];

// IR convergence: median |dtau/dt - sqrt(1-v^2)| (residual to exact gamma) vs lattice L,
// holding the mass fixed so k_perp -> 0. Falls ~ L^-2 -> gamma emerges in the IR.
export const IR_CONVERGENCE = [
  { L: 33, resid: 0.02459 },
  { L: 65, resid: 0.01847 },
  { L: 97, resid: 0.01379 },
  { L: 129, resid: 0.00984 },
  { L: 193, resid: 0.00458 },
];
