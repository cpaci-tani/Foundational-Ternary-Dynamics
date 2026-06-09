// Pure FTD time-dilation math. No DOM, no Three.js — node-testable + browser ESM.
// Mirrors gravity-analysis.js conventions: lapse f = 1 - L^2, clock rate dtau/dt = sqrt(f).
// L is the dimensionless latency / gravity-well depth in [0, 1); v is velocity in units of c.

export const lapse = (L) => 1 - L * L;
export const clockRate = (L) => Math.sqrt(Math.max(0, lapse(L)));      // dtau/dt (gravitational)
export const slowdownPct = (L) => (1 - clockRate(L)) * 100;

// FTD generalized Lorentz factor gamma = sqrt(f) / sqrt(f^2 - v^2) (gravity-panel.js:138).
// Fuses gravitational lapse f and velocity v. Reduces to SR gamma when L=0 (f=1).
export function ftdGamma(L, v) {
  const f = lapse(L);
  const d = f * f - v * v;
  return d > 0 ? Math.sqrt(f) / Math.sqrt(d) : Infinity;
}

export const srDilation = (v) => Math.sqrt(Math.max(0, 1 - v * v)); // dtau/dt (kinematic) = sqrt(1-v^2)
export const srGamma = (v) => (v * v < 1 ? 1 / Math.sqrt(1 - v * v) : Infinity);

export const properTimeStep = (L, dt) => clockRate(L) * dt;          // accumulated proper time per tick

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
