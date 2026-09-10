// engine/web/js/scales/scale3/liquid-transport.js
/**
 * Liquid-transport estimators for the Scale 3 molecular labs.
 * [IMPOSED effective classical dynamics; coefficients in engine units]
 * Pure functions on typed arrays plus a windowed tracker that publishes to the telemetry hub.
 * Units: AtomEngine reduced units (length in the AE lattice unit, time in ticks × dt, mass in proton masses).
 */
const MASS_OF_Z = { 1: 1, 6: 12, 7: 14, 8: 16, 10: 20, 18: 40 };
const massOf = (Z) => MASS_OF_Z[Z] ?? Z * 2;

export function moleculeCentroids(positions, atomicNums, bonds, count, bondCount) {
    const adj = Array.from({ length: count }, () => []);
    for (let b = 0; b < bondCount; b++) { const i = bonds[2 * b], j = bonds[2 * b + 1]; adj[i].push(j); adj[j].push(i); }
    const seen = new Uint8Array(count), molecules = [];
    for (let s = 0; s < count; s++) {
        if (seen[s]) continue;
        const comp = [], stack = [s]; seen[s] = 1;
        while (stack.length) { const a = stack.pop(); comp.push(a); for (const n of adj[a]) if (!seen[n]) { seen[n] = 1; stack.push(n); } }
        molecules.push(comp.sort((a, b) => a - b));
    }
    const centroids = new Float64Array(3 * molecules.length), masses = new Float64Array(molecules.length);
    molecules.forEach((comp, k) => {
        let m = 0, x = 0, y = 0, z = 0;
        for (const a of comp) { const w = massOf(atomicNums[a]); m += w; x += w * positions[3 * a]; y += w * positions[3 * a + 1]; z += w * positions[3 * a + 2]; }
        centroids[3 * k] = x / m; centroids[3 * k + 1] = y / m; centroids[3 * k + 2] = z / m; masses[k] = m;
    });
    const velocitiesOf = (velocities) => {
        const out = new Float64Array(3 * molecules.length);
        molecules.forEach((comp, k) => {
            let m = 0, vx = 0, vy = 0, vz = 0;
            for (const a of comp) { const w = massOf(atomicNums[a]); m += w; vx += w * velocities[3 * a]; vy += w * velocities[3 * a + 1]; vz += w * velocities[3 * a + 2]; }
            out[3 * k] = vx / m; out[3 * k + 1] = vy / m; out[3 * k + 2] = vz / m;
        });
        return out;
    };
    return { centroids, masses, molecules, velocitiesOf };
}

export function velocityProfile(coords, values, lo, hi, nBins) {
    const centers = new Float64Array(nBins), sum = new Float64Array(nBins), counts = new Int32Array(nBins);
    const width = (hi - lo) / nBins;
    for (let b = 0; b < nBins; b++) centers[b] = lo + (b + 0.5) * width;
    for (let i = 0; i < coords.length; i++) {
        const b = Math.floor((coords[i] - lo) / width);
        if (b < 0 || b >= nBins) continue;
        sum[b] += values[i]; counts[b]++;
    }
    const mean = new Float64Array(nBins);
    for (let b = 0; b < nBins; b++) mean[b] = counts[b] ? sum[b] / counts[b] : NaN;
    return { centers, mean, counts };
}

export function erf(x) {
    const s = Math.sign(x); x = Math.abs(x);
    const t = 1 / (1 + 0.3275911 * x);
    const y = 1 - (((((1.061405429 * t - 1.453152027) * t) + 1.421413741) * t - 0.284496736) * t + 0.254829592) * t * Math.exp(-x * x);
    return s * y;
}

/** Least squares of mean(y) = U erf((y - y0)/w) over (U, y0, w): U from the two outer quartiles, y0 the
 *  zero crossing of the linear interpolant, w by golden-section search on the residual. NaN bins skipped. */
export function fitErfWidth(centers, mean) {
    const pts = [];
    for (let i = 0; i < centers.length; i++) if (Number.isFinite(mean[i])) pts.push([centers[i], mean[i]]);
    if (pts.length < 4) return { U: NaN, y0: NaN, w: NaN, rms: NaN };
    const q = Math.max(1, Math.floor(pts.length / 4));
    const lowerU = pts.slice(0, q).reduce((s, p) => s + p[1], 0) / q, upperU = pts.slice(-q).reduce((s, p) => s + p[1], 0) / q;
    const U = (upperU - lowerU) / 2, offset = (upperU + lowerU) / 2;
    let y0 = pts[0][0];
    for (let i = 1; i < pts.length; i++) if ((pts[i - 1][1] - offset) * (pts[i][1] - offset) <= 0) {
        const a = pts[i - 1], b = pts[i]; y0 = a[0] + (b[0] - a[0]) * (offset - a[1]) / ((b[1] - a[1]) || 1e-12); break;
    }
    const residual = (w) => { let s = 0; for (const [y, v] of pts) { const d = v - offset - U * erf((y - y0) / w); s += d * d; } return s; };
    let lo = 1e-3, hi = Math.abs(pts[pts.length - 1][0] - pts[0][0]);
    const phi = (Math.sqrt(5) - 1) / 2;
    let c = hi - phi * (hi - lo), d = lo + phi * (hi - lo), fc = residual(c), fd = residual(d);
    for (let it = 0; it < 80; it++) {
        if (fc < fd) { hi = d; d = c; fd = fc; c = hi - phi * (hi - lo); fc = residual(c); }
        else { lo = c; c = d; fc = fd; d = lo + phi * (hi - lo); fd = residual(d); }
    }
    const w = (lo + hi) / 2;
    return { U, y0, w, rms: Math.sqrt(residual(w) / pts.length) };
}

/** a = (2/h) ∫ v cos(πy/h) dy estimated as a molecule average: (2/N) Σ v_i cos(π y_i / h) × (1/mean cos²) normalisation. */
export function firstModeAmplitude(y, vx, h) {
    let num = 0, den = 0;
    for (let i = 0; i < y.length; i++) { const c = Math.cos(Math.PI * y[i] / h); num += vx[i] * c; den += c * c; }
    return den ? num / den : NaN;
}

/**
 * First-mode amplitude that is orthogonal to a uniform drift. A pure
 * v(y) = U (no cosine content) projects onto cos(pi y/h) at 4/pi under the
 * bare firstModeAmplitude estimator above — that estimator is only valid
 * once the flow has no residual center-of-mass drift. This variant solves
 * the 2x2 least squares for vx ~= c0 + a1 cos(pi y / h) and returns a1, so a
 * constant offset is absorbed into c0 instead of leaking into the mode
 * amplitude.
 */
export function firstModeAmplitudeWithOffset(y, vx, h) {
    const n = y.length;
    let sumC = 0, sumCC = 0, sumV = 0, sumVC = 0;
    for (let i = 0; i < n; i++) {
        const c = Math.cos(Math.PI * y[i] / h);
        sumC += c; sumCC += c * c; sumV += vx[i]; sumVC += vx[i] * c;
    }
    const det = n * sumCC - sumC * sumC;
    return det ? (n * sumVC - sumC * sumV) / det : NaN;
}

export function meanSquareDisplacement(ref, now, driftRef, driftNow) {
    const m = ref.length / 3; let s = 0;
    for (let i = 0; i < m; i++) {
        const dx = (now[3 * i] - driftNow[0]) - (ref[3 * i] - driftRef[0]);
        const dy = (now[3 * i + 1] - driftNow[1]) - (ref[3 * i + 1] - driftRef[1]);
        const dz = (now[3 * i + 2] - driftNow[2]) - (ref[3 * i + 2] - driftRef[2]);
        s += dx * dx + dy * dy + dz * dz;
    }
    return s / m;
}

export function angularVelocitySplit(centroids, velocities, center, axis) {
    const m = centroids.length / 3, a1 = (axis + 1) % 3, a2 = (axis + 2) % 3;
    const rows = [];
    for (let i = 0; i < m; i++) {
        const p1 = centroids[3 * i + a1] - center[a1], p2 = centroids[3 * i + a2] - center[a2];
        const r2 = p1 * p1 + p2 * p2; if (r2 < 1e-12) continue;
        const omega = (p1 * velocities[3 * i + a2] - p2 * velocities[3 * i + a1]) / r2;
        rows.push([Math.sqrt(r2), omega]);
    }
    rows.sort((a, b) => a[0] - b[0]);
    const half = Math.floor(rows.length / 2);
    const avg = (xs) => xs.reduce((s, r) => s + r[1], 0) / (xs.length || 1);
    return { inner: avg(rows.slice(0, half)), outer: avg(rows.slice(half)) };
}

export function linearRegression(t, y) {
    const n = t.length; if (n < 3) return { slope: NaN, intercept: NaN, slopeSE: NaN, n };
    let st = 0, sy = 0; for (let i = 0; i < n; i++) { st += t[i]; sy += y[i]; }
    const mt = st / n, my = sy / n; let sxx = 0, sxy = 0;
    for (let i = 0; i < n; i++) { sxx += (t[i] - mt) ** 2; sxy += (t[i] - mt) * (y[i] - my); }
    const slope = sxy / sxx, intercept = my - slope * mt; let sse = 0;
    for (let i = 0; i < n; i++) sse += (y[i] - intercept - slope * t[i]) ** 2;
    return { slope, intercept, slopeSE: Math.sqrt(sse / (n - 2) / sxx), n };
}

/** Fits log(a) vs t up to (excluding) the first non-positive amplitude: past that
 *  point the signal has crossed zero into noise, and a bare a[i]>0 filter across the
 *  whole series would keep only the positive noise fluctuations beyond it, biasing
 *  the fitted rate. `samplesDropped` counts what was truncated away. */
export function logDecayRate(t, a) {
    const tt = [], la = [];
    for (let i = 0; i < t.length; i++) {
        if (a[i] <= 0) break;
        tt.push(t[i]); la.push(Math.log(a[i]));
    }
    const r = linearRegression(tt, la);
    return { gamma: -r.slope, gammaSE: r.slopeSE, n: r.n, samplesDropped: t.length - tt.length };
}

/** Add the declared flow to the current velocities (called at the measurement phase start). */
export function imposeLiquidFlow(bridge, scenario) {
    const L = scenario.liquid;
    if (L.kind === 'droplet') return; // this kind imposes no flow; free flight only.
    const data = bridge.aeGetAtomData();
    // Read every atom's current velocity in double precision through the O(N)
    // accessor (not the Float32 aeGetVelocities() renderer view, which
    // quantises the thermalised state, and not a per-atom aeInspectAtom()
    // loop, which recomputes all forces on every call and made this O(N^3)).
    const vel = bridge.aeGetVelocitiesF64().velocities;
    const g = { x: 0, y: 1, z: 2 }[L.gradient], f = { x: 0, y: 1, z: 2 }[L.axis];
    for (let i = 0; i < data.count; i++) {
        // Locked wall species never take a velocity: aeSetAtomVelocity rejects the
        // call for a locked atom, so skip it here rather than fire-and-ignore.
        if (Number.isFinite(L.wallZ) && data.atomicNums[i] === L.wallZ) continue;
        const p = [data.positions[3 * i], data.positions[3 * i + 1], data.positions[3 * i + 2]];
        const v = [vel[3 * i], vel[3 * i + 1], vel[3 * i + 2]];
        if (L.kind === 'shear-layer') v[f] += p[g] >= 0 ? L.U : -L.U;
        else if (L.kind === 'channel') v[f] += L.U * Math.max(0, 1 - (2 * p[g] / L.h) ** 2);
        else if (L.kind === 'spinning-droplet') {
            const r = Math.hypot(p[0], p[1]); const om = L.omega0 * Math.max(0, 1 - r / L.R);
            v[0] += -om * p[1]; v[1] += om * p[0];
        }
        bridge.aeSetAtomVelocity(data.ids[i], v[0], v[1], v[2]);
    }
}

export class LiquidTransportTracker {
    constructor(liquid, dt) {
        this.L = liquid; this.dt = dt; this.samples = []; this.ref = null;
        this.summaryCache = { status: 'waiting for the measurement window' };
        this._lastSampledTick = null;
    }
    sample(frame) {
        const { tick } = frame; const L = this.L;
        if (tick < L.window.start || tick > L.window.end) { this.summaryCache.status = tick < L.window.start ? 'thermalizing' : 'window closed'; return; }
        // A paused session still gets sampled every third rAF frame at the
        // same engine tick (collectScale2 gates on frame count, not on
        // `running`); without this guard, every extra frame at the same
        // tick pushes a duplicate row, inflating `samples` and shrinking
        // the reported standard error with no new information.
        if (tick === this._lastSampledTick) return;
        this._lastSampledTick = tick;
        const mol = moleculeCentroids(frame.positions, frame.atomicNums, frame.bonds, frame.count, frame.bondCount);
        const wallZ = L.wallZ, keep = mol.molecules.reduce((ks, comp, k) => { if (!Number.isFinite(wallZ) || comp.every((a) => frame.atomicNums[a] !== wallZ)) ks.push(k); return ks; }, []);
        const allCen = mol.centroids, allVel = mol.velocitiesOf(frame.velocities), m = keep.length;
        const cen = new Float64Array(3 * m), vel = new Float64Array(3 * m);
        keep.forEach((k, i) => { for (let c = 0; c < 3; c++) { cen[3 * i + c] = allCen[3 * k + c]; vel[3 * i + c] = allVel[3 * k + c]; } });
        const t = (tick - L.window.start) * this.dt;
        const g = { x: 0, y: 1, z: 2 }[L.gradient], f = { x: 0, y: 1, z: 2 }[L.axis];
        const coords = new Float64Array(m), values = new Float64Array(m);
        for (let k = 0; k < m; k++) { coords[k] = cen[3 * k + g]; values[k] = vel[3 * k + f]; }
        const drift = [0, 0, 0]; for (let k = 0; k < m; k++) for (let c = 0; c < 3; c++) drift[c] += cen[3 * k + c] / m;
        if (!this.ref) this.ref = { cen: Float64Array.from(cen), drift };
        const row = { t, tick };
        if (L.kind === 'shear-layer') { const p = velocityProfile(coords, values, -L.extent, L.extent, L.bins); const fit = fitErfWidth(p.centers, p.mean); row.w2 = fit.w * fit.w; row.U = fit.U; }
        else if (L.kind === 'channel') row.a = firstModeAmplitudeWithOffset(coords, values, L.h);
        else if (L.kind === 'droplet') {
            // meanSquareDisplacement indexes both arrays by the same molecule
            // count; a mismatch (the kept-molecule population changed between
            // the reference frame and this one) would otherwise read past the
            // shorter array and surface as a silent NaN. Skip the row instead.
            if (cen.length !== this.ref.cen.length) { this.summaryCache = { status: 'population changed' }; return; }
            row.msd = meanSquareDisplacement(this.ref.cen, cen, this.ref.drift, drift);
        }
        else if (L.kind === 'spinning-droplet') { const s = angularVelocitySplit(cen, vel, drift, 2); row.dOmega = s.inner - s.outer; }
        this.samples.push(row); this.summaryCache = this.summary();
    }
    summary() {
        const L = this.L, S = this.samples; if (S.length < 4) return { status: `collecting (${S.length} samples)` };
        const t = S.map((r) => r.t);
        if (L.kind === 'shear-layer') { const r = linearRegression(t, S.map((x) => x.w2)); return { status: 'measuring', quantity: 'nu', value: r.slope / 4, se: r.slopeSE / 4, samples: r.n, estimator: 'w^2 = 4 nu t (erf-profile width)' }; }
        if (L.kind === 'channel') {
            const d = logDecayRate(t, S.map((x) => x.a)); const k = L.h * L.h / (Math.PI * Math.PI);
            if (d.n < 4) return { status: `noise-limited (${d.n} usable samples, ${d.samplesDropped} dropped)`, samples: d.n, samplesDropped: d.samplesDropped };
            return { status: 'measuring', quantity: 'nu', value: d.gamma * k, se: d.gammaSE * k, samples: d.n, samplesDropped: d.samplesDropped, estimator: 'first-mode decay, nu = gamma h^2 / pi^2' };
        }
        if (L.kind === 'droplet') { const r = linearRegression(t, S.map((x) => x.msd)); return { status: 'measuring', quantity: 'D', value: r.slope / 6, se: r.slopeSE / 6, samples: r.n, estimator: 'MSD slope / 6 (drift removed)' }; }
        const d = logDecayRate(t, S.map((x) => Math.abs(x.dOmega)));
        if (d.n < 4) return { status: `noise-limited (${d.n} usable samples, ${d.samplesDropped} dropped)`, samples: d.n, samplesDropped: d.samplesDropped };
        return { status: 'measuring', quantity: 'tau', value: 1 / d.gamma, se: d.gammaSE / (d.gamma * d.gamma), samples: d.n, samplesDropped: d.samplesDropped, estimator: 'inner-outer angular velocity difference, exponential relaxation' };
    }
}
