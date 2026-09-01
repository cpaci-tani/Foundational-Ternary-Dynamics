// Pure FTD causal-clock math. No DOM, no Three.js — node-testable + browser ESM.
// FTD-0402: stored velocity is raw nodes/tick and beta^2 = |u|^2/C_SPEED^2.
// The clock/bandwidth relation is a selected implementation axiom, not a
// substrate theorem of physical covariance.

import { C_SPEED } from '../../../constants.js';

export const lapse = (L) => 1 - L * L;
export const betaSquared = (rawSpeed) => (rawSpeed * rawSpeed) / (C_SPEED * C_SPEED);
export const causalBudget = (L, rawSpeed = 0) => betaSquared(rawSpeed) + L * L;
export const clockRate = (L, rawSpeed = 0) => Math.sqrt(Math.max(0, 1 - causalBudget(L, rawSpeed)));
export const slowdownPct = (L, rawSpeed = 0) => (1 - clockRate(L, rawSpeed)) * 100;

// Selected FTD transport factor gamma_FTD = 1/sqrt(1 - beta^2 - L^2).
export function ftdGamma(L, rawSpeed = 0) {
  const rate = clockRate(L, rawSpeed);
  return rate > 0 ? 1 / rate : Infinity;
}

export const srDilation = (rawSpeed) => Math.sqrt(Math.max(0, 1 - betaSquared(rawSpeed)));
export const srGamma = (rawSpeed) => {
  const rate = srDilation(rawSpeed);
  return rate > 0 ? 1 / rate : Infinity;
};

export const properTimeStep = (L, dt, rawSpeed = 0) => clockRate(L, rawSpeed) * dt;

// Bin sparse latency samples by radius from a center; return [{r, L, dtau_dt}] sorted by r.
// positions: Float32Array [x0,y0,z0, x1,...]; values: Float32Array of L per sample.
export function radialProfile(positions, values, center) {
  const out = [];
  for (let i = 0; i < values.length; i++) {
    const x = positions[i * 3], y = positions[i * 3 + 1], z = positions[i * 3 + 2];
    const r = Math.hypot(x - center.x, y - center.y, z - center.z);
    out.push({ r, L: values[i], dtau_dt: clockRate(values[i]) });
  }
  return out.sort((a, b) => a.r - b.r);
}

// Mean dtau/dt per radial bin (nBins over [0, rMax]); for the Card B sparkline.
export function radialBins(profile, nBins = 12) {
  if (!profile.length) return [];
  const rMax = profile[profile.length - 1].r || 1;
  const sum = new Array(nBins).fill(0), cnt = new Array(nBins).fill(0);
  for (const p of profile) {
    const b = Math.min(nBins - 1, Math.floor((p.r / rMax) * nBins));
    sum[b] += p.dtau_dt; cnt[b] += 1;
  }
  const bins = [];
  for (let b = 0; b < nBins; b++) {
    if (cnt[b]) bins.push({ r: (b + 0.5) / nBins * rMax, dtau_dt: sum[b] / cnt[b] });
  }
  return bins;
}

// Allocation-bounded live-panel reduction. Unlike radialProfile()+radialBins(),
// this never materializes or sorts one object per sampled voxel. Samples are
// reduced directly into radial bins while retaining the extrema needed by the
// twin-clock and lapse cards. `maxRadius` may be supplied from lattice geometry
// to keep the reduction one-pass; otherwise the sampled maximum is used.
export function radialSummary(positions, values, center, nBins = 12, maxRadius = null) {
  const count = Math.min(values?.length || 0, Math.floor((positions?.length || 0) / 3));
  if (!count || nBins < 1) {
    return { bins: [], lDeep: 0, lFar: 0, lMax: 0, rAtMax: 0, hasField: false };
  }
  let rMax = Number.isFinite(maxRadius) && maxRadius > 0 ? maxRadius : 0;
  if (!(rMax > 0)) {
    for (let i = 0; i < count; i++) {
      const dx = positions[i * 3] - center.x;
      const dy = positions[i * 3 + 1] - center.y;
      const dz = positions[i * 3 + 2] - center.z;
      rMax = Math.max(rMax, Math.sqrt(dx * dx + dy * dy + dz * dz));
    }
  }
  rMax ||= 1;
  const sum = new Float64Array(nBins);
  const binCount = new Uint32Array(nBins);
  let lDeep = -Infinity, lFar = Infinity, rAtMax = 0;
  for (let i = 0; i < count; i++) {
    const dx = positions[i * 3] - center.x;
    const dy = positions[i * 3 + 1] - center.y;
    const dz = positions[i * 3 + 2] - center.z;
    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
    const L = values[i];
    if (L > lDeep) { lDeep = L; rAtMax = r; }
    if (L < lFar) lFar = L;
    const b = Math.min(nBins - 1, Math.floor((r / rMax) * nBins));
    sum[b] += clockRate(L);
    binCount[b]++;
  }
  const bins = [];
  for (let b = 0; b < nBins; b++) {
    if (binCount[b]) bins.push({
      r: (b + 0.5) / nBins * rMax,
      dtau_dt: sum[b] / binCount[b],
    });
  }
  return { bins, lDeep, lFar, lMax: lDeep, rAtMax, hasField: true };
}
