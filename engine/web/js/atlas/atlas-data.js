// Static analytic illustrative fields for the Ontology Atlas.
// No THREE, no DOM, no engine — pure functions, node-testable.
// These are *teaching* fields (a couple of point charges, a point mass, a wave
// packet), computed once; the atlas does not run a live simulation.

const EPS = 0.06;
const sub = (a, b) => ({ x: a.x - b.x, y: a.y - b.y, z: a.z - b.z });
const len = (v) => Math.hypot(v.x, v.y, v.z);

// Coulomb/E-field of point charges at p: sum q * r / |r|^3 (softened by EPS).
export function fluxFromCharges(charges, p) {
  const E = { x: 0, y: 0, z: 0 };
  for (const c of charges) {
    const r = sub(p, c.pos), d = len(r) + EPS, inv = c.q / (d * d * d);
    E.x += inv * r.x; E.y += inv * r.y; E.z += inv * r.z;
  }
  return E;
}

// Signed source proxy for ∇·J: positive bump at +q, negative at −q.
export function divFlux(charges, p) {
  let d = 0;
  for (const c of charges) { const r = len(sub(p, c.pos)); d += c.q * Math.exp(-(r * r) / 0.08); }
  return d;
}

// Latency well L in [0,1): deeper (larger) closer to a mass. Poisson-like.
export function latencyWell(masses, p) {
  let prod = 1;
  for (const m of masses) { const r = len(sub(p, m.pos)) + EPS; prod *= (1 - Math.min(0.98, m.m / r)); }
  return 1 - prod;
}

// Transverse wave packet ψ = J_x + i·J_y value at p (Gaussian envelope × phase).
export function psiPacket(k, x0, p, t) {
  const r = sub(p, x0), kr = k.x * p.x + k.y * p.y + k.z * p.z;
  const env = Math.exp(-(r.x * r.x + r.y * r.y + r.z * r.z) / 0.5);
  const phase = kr - 0.0 * t;                 // t reserved for the animation pass
  return { re: env * Math.cos(phase), im: env * Math.sin(phase) };
}

// Manifestation: divergence → ternary state {−1, 0, +1}.
export function stateFromDiv(d, thresh = 0.15) { return d > thresh ? 1 : d < -thresh ? -1 : 0; }

// Sample fn over an n^3 grid centered in [-1, 1]^3.
export function sampleGrid(n, fn) {
  const out = []; const s = n > 1 ? 2 / (n - 1) : 0;
  for (let i = 0; i < n; i++) for (let j = 0; j < n; j++) for (let kk = 0; kk < n; kk++) {
    const p = { x: -1 + i * s, y: -1 + j * s, z: -1 + kk * s };
    out.push({ p, v: fn(p) });
  }
  return out;
}
