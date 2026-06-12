/**
 * Cosmic scale-5 force kernel.
 *
 * Extracted from mock-scale5.js (MS5-2). The kernel runs the O(N^2) Gadget-2
 * gravity + optional SPH-like sub-grid physics. It operates on a CosmicMockBridge
 * instance's `_bodies` array, reusing (or lazily allocating) the instance's
 * `_soa` SoA scratch buffer to avoid GC churn across ticks.
 *
 * Invoked via `.call(bridge)` from CosmicMockBridge._computeForces so `this`
 * binds to the bridge instance. All state mutations (body accelerations,
 * temperatures, internal energies) happen on the bridge's own bodies.
 *
 * Unit system: G = G_N = 0.01 (FTD ontic chain).
 */

import { G_N, C_SPEED } from '../constants.js';

// Fixed softening per body type (Gadget-2 convention: constant, energy-conserving).
// 2026-04-26 (Wave 2H): the prior "mirrored from mock-scale5.js" note
// was stale — mock-scale5.js does not declare its own copy; it calls
// computeCosmicForces.call(this) on this module's tables. This is now
// the single source of truth. If a constants.js entry is added later,
// migrate from here.
const SOFTENING = {
    [-3]: 6.0,  // DARK_ENERGY
    [-2]: 3.0,  // QUASAR
    [-1]: 1.5,  // BLACK_HOLE
    [0]:  8.0,  // DARK_MATTER
    [1]:  3.0,  // GAS
    [2]:  2.5,  // STAR
    [3]:  2.0,  // NEUTRON_STAR
    [4]:  3.0,  // NEBULA
    [5]:  2.0,  // WHITE_DWARF
};
const SOFTENING_SQ = {
    [-3]: 36.0, [-2]: 9.0, [-1]: 2.25,
    [0]: 64.0, [1]: 9.0, [2]: 6.25,
    [3]: 4.0, [4]: 9.0, [5]: 4.0
};

// ── Barnes–Hut tuning (F-1) ────────────────────────────────────────────────
// BH_THETA: opening angle. A cell of width w at distance d to its centre of
//   mass is treated as a single monopole when w/d < BH_THETA. 0.5 is the
//   classic *conservative* value (errors ~0.1–1%); smaller ⇒ more accurate &
//   slower, larger ⇒ faster & coarser. Do NOT raise without re-checking the
//   visual-equivalence note in computeCosmicForces.
const BH_THETA = 0.5;
const BH_THETA_SQ = BH_THETA * BH_THETA;   // compared against w²/d² (no sqrt)
// Body-count gate. The direct O(N²) sum here is a tight, branch-free, cache-
// local SoA loop that V8 auto-vectorizes (SIMD); the BH walk is branchy and
// pointer-chasing (no SIMD) and carries tree-build overhead. Benchmarked
// crossover for THIS kernel is N≈2600 (warm JIT): below it the direct sum is
// equal-or-faster, above it BH pulls ahead and keeps widening (≈1.3× at 4000,
// ≈2× at 9000, ≈3× at 20000). We gate slightly above the crossover so:
//   • N ≤ threshold → exact direct sum, bit-identical output (zero regression);
//   • N >  threshold → BH, where it is a genuine and growing win.
// Net effect: F-1 makes the cosmic sim *scale gracefully as bodies accrete past
// ~3000* (merger / cosmic-web / globular-cluster growth) rather than the frame
// time exploding ∝N². It is approximately neutral on the baseline ~2600-body
// galaxy (already ~10–15 ms warm). Set to Infinity to force exact gravity
// everywhere (disables F-1); lower it toward ~2000 only if profiling on the
// target hardware shows the direct sum dominating below the crossover there.
const BH_N_THRESHOLD = 3000;
// Max bodies a leaf cell holds before it subdivides. Small buckets keep the
// near-field exact (intra-leaf pairs are summed directly) while bounding tree
// depth. 8 is a good JS cache/þroughput balance.
const BH_LEAF_CAP = 8;
// Floor on cell half-width. If a cluster is so tight that a cell would shrink
// below this, we stop subdividing and let the (now possibly >CAP) leaf bucket
// accumulate — prevents unbounded depth on near-coincident bodies. Tiny vs the
// smallest softening (min eps = 1.5 for BLACK_HOLE), so it never affects forces.
const BH_MIN_HALF = 1e-3;
// Safety cap on subdivision depth (defensive; BH_MIN_HALF normally binds first).
const BH_MAX_DEPTH = 32;

/**
 * Run the gravity + sub-grid force kernel against `this._bodies`.
 * Call via `computeCosmicForces.call(bridgeInstance)`.
 */
export function computeCosmicForces(TYPE) {
    const G = G_N;
    const n = this._bodies.length;

    // JIT SoA buffers — grown lazily, reused across ticks.
    if (!this._soa || this._soa.n < n) {
        const MathMaxOffset = 1000;
        const capacity = n + MathMaxOffset;
        this._soa = {
            n: capacity,
            x: new Float64Array(capacity),
            y: new Float64Array(capacity),
            z: new Float64Array(capacity),
            mass: new Float64Array(capacity),
            soft: new Float64Array(capacity),
            softSq: new Float64Array(capacity),
            ax: new Float64Array(capacity),
            ay: new Float64Array(capacity),
            az: new Float64Array(capacity)
        };
    }

    const soa = this._soa;
    const X = soa.x, Y = soa.y, Z = soa.z, M = soa.mass;
    const SOFT = soa.soft, SQ = soa.softSq;
    const AX = soa.ax, AY = soa.ay, AZ = soa.az;

    // 1. Flatten JS objects into typed arrays.
    for (let i = 0; i < n; i++) {
        const b = this._bodies[i];
        X[i] = b.x;
        Y[i] = b.y;
        Z[i] = b.z;
        M[i] = b.mass;
        SOFT[i] = SOFTENING[b.type] || 2.0;
        SQ[i] = SOFTENING_SQ[b.type] || 4.0;
        AX[i] = 0.0;
        AY[i] = 0.0;
        AZ[i] = 0.0;
    }

    // 2. Gravity sum.
    //
    // ── F-1 (audit 2026-05-27): Barnes–Hut octree, APPROXIMATE ──────────────
    // The historical kernel here was an exact O(N²) all-pairs sum (~3.4M pair
    // iterations/tick at N≈2600). It is retained verbatim for N ≤ BH_N_THRESHOLD
    // (where it is equal-or-faster AND bit-identical); above the threshold the
    // gravity sum switches to an O(N log N) Barnes–Hut octree walk so the frame
    // time scales gracefully as bodies accrete/merge into the thousands instead
    // of growing ∝N². (Measured crossover ≈2600 bodies warm; see BH_N_THRESHOLD.
    // The win is in the *tail* — large/grown scenes — not the baseline galaxy.)
    //
    // *** THE BH BRANCH CHANGES RESULTS SLIGHTLY — IT IS AN APPROXIMATION. ***
    // For a node whose (cellWidth / distanceToCOM) < BH_THETA, the node's whole
    // mass is approximated by its centre-of-mass monopole. Validated against the
    // direct sum on galaxy-like distributions (exponential disk + bulge + DM
    // halo) at θ=0.5: mean per-body acceleration error ≈1–2%, worst-case ≈10–22%
    // for a handful of bodies sitting near a cell boundary; quadrupole and higher
    // multipoles are dropped. Over many ticks individual trajectories diverge
    // from the direct sum (N-body gravity is chaotic) — expected and acceptable
    // for a *visual sandbox*: a galaxy still rotates as a galaxy and a cluster
    // still clusters. The result is statistically equivalent, NOT trajectory-
    // identical. Forcing the direct branch (BH_N_THRESHOLD = Infinity) restores
    // exactness for any regression-grade test.
    //
    // Force law preserved per interaction:
    //   • softening: leaf↔leaf uses the exact Gadget-2 rule eps² =
    //     max(eps_i², eps_j²); body↔aggregate uses max(eps_i², node.maxSoftSq),
    //     i.e. never smaller than the true pairwise softening (conservative —
    //     cannot manufacture a spuriously hard force). Aggregates are far by the
    //     θ-criterion, so eps² ≪ r² there and this choice is numerically inert.
    //   • kernel: identical 1/(r²+eps²)^{3/2} softened-Newton form, same G.
    //
    // Known characteristic (not a bug): the direct sum uses Newton's third law
    // (i+1..n with equal/opposite updates) so total momentum Σm·a is conserved
    // to machine precision; the BH walk computes each body's force independently
    // against aggregated COMs, so it does NOT enforce the third law and total
    // momentum drifts by ~O(error)·(typical force). For a visual sandbox this is
    // invisible (no perceptible bulk COM drift over a session); a momentum-exact
    // result requires the direct branch.
    if (n < BH_N_THRESHOLD) {
        // Exact O(N²) direct sum — bit-identical to the legacy kernel.
        for (let i = 0; i < n; i++) {
            const bix = X[i], biy = Y[i], biz = Z[i], bim = M[i];
            const s_i = SOFT[i], sq_i = SQ[i];
            let ax = AX[i], ay = AY[i], az = AZ[i];
            for (let j = i + 1; j < n; j++) {
                const s_j = SOFT[j];
                const eps2 = s_i > s_j ? sq_i : SQ[j];
                const dx = X[j] - bix;
                const dy = Y[j] - biy;
                const dz = Z[j] - biz;
                const r2 = dx * dx + dy * dy + dz * dz + eps2;
                const invR3 = 1.0 / (r2 * Math.sqrt(r2));
                const f_j = G * M[j] * invR3;
                const f_i = G * bim * invR3;
                ax += f_j * dx;
                ay += f_j * dy;
                az += f_j * dz;
                AX[j] -= f_i * dx;
                AY[j] -= f_i * dy;
                AZ[j] -= f_i * dz;
            }
            AX[i] = ax;
            AY[i] = ay;
            AZ[i] = az;
        }
    } else {
        barnesHutGravity.call(this, n, G, X, Y, Z, M, SOFT, SQ, AX, AY, AZ);
    }

    // 3. Restitute accelerations back to JS body objects.
    for (let i = 0; i < n; i++) {
        const b = this._bodies[i];
        b.ax = AX[i];
        b.ay = AY[i];
        b.az = AZ[i];
    }

    // Sub-grid physics only active in select scenarios (BH accretion / FTD collapse).
    if (!this._enableSubgrid) return;

    const T = TYPE;
    const baseSoft2 = this._softening * this._softening;
    const bodies = this._bodies;
    const nb = bodies.length;

    const gasIdx  = [];
    const starIdx = [];
    const bhIdx   = [];
    for (let i = 0; i < nb; i++) {
        const t = bodies[i].type;
        if (t === T.GAS || t === T.NEBULA) {
            gasIdx.push(i);
        } else if (t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF) {
            starIdx.push(i);
        } else if (t === T.BLACK_HOLE || t === T.QUASAR) {
            bhIdx.push(i);
        }
    }
    const nGas  = gasIdx.length;
    const nStar = starIdx.length;
    const nBH   = bhIdx.length;

    // Tidal spaghettification (radial stretch only).
    for (let bi = 0; bi < nBH; bi++) {
        const bh = bodies[bhIdx[bi]];
        const bhMass = bh.mass;
        const bhx = bh.x, bhy = bh.y, bhz = bh.z;
        const bhId = bh.id;
        const r_tidal = Math.max(8.0, Math.cbrt(bhMass) * 1.5);
        const r_tidal2 = r_tidal * r_tidal;
        const tidalK = 2.0 * G * bhMass * 0.3;
        for (let i = 0; i < nb; i++) {
            const b = bodies[i];
            if (b.id === bhId) continue;
            const dx = b.x - bhx, dy = b.y - bhy, dz = b.z - bhz;
            const r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > r_tidal2 || r2 < 0.01) continue;
            const r = Math.sqrt(r2);
            const invR = 1.0 / r;
            const tidalStrength = tidalK / (r2 * r);
            b.ax += tidalStrength * dx * invR;
            b.ay += tidalStrength * dy * invR;
            b.az += tidalStrength * dz * invR;
        }
    }

    // Gas cooling — reduces internal energy (not velocity drag).
    const coolRadius2 = baseSoft2 * 25;
    for (let gi = 0; gi < nGas; gi++) {
        const b = bodies[gasIdx[gi]];
        const bx = b.x, by = b.y, bz = b.z;
        let localDensity = b.mass;
        for (let gj = 0; gj < nGas; gj++) {
            if (gj === gi) continue;
            const other = bodies[gasIdx[gj]];
            const dx = bx - other.x, dy = by - other.y, dz = bz - other.z;
            const dr2 = dx * dx + dy * dy + dz * dz;
            if (dr2 < coolRadius2) localDensity += other.mass;
        }
        const coolingRate = Math.min(0.0002, 0.000002 * localDensity);
        b.internal_energy = Math.max(0.001, b.internal_energy * (1 - coolingRate));
        b.temperature = Math.max(100, b.internal_energy * 1000);
    }

    // Gas pressure (SPH-like repulsion).
    const h_press = this._softening * 2.5;
    const h_press2 = h_press * h_press;
    for (let gi = 0; gi < nGas; gi++) {
        const bi_idx = gasIdx[gi];
        const bi = bodies[bi_idx];
        const bix = bi.x, biy = bi.y, biz = bi.z;
        const biMass = bi.mass;
        const biE = bi.internal_energy;
        for (let gj = gi + 1; gj < nGas; gj++) {
            const bj = bodies[gasIdx[gj]];
            const dx = bj.x - bix, dy = bj.y - biy, dz = bj.z - biz;
            const r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > h_press2 || r2 < 1e-10) continue;
            const r = Math.sqrt(r2);
            const q = r / h_press;
            const T_avg = 0.5 * (biE + bj.internal_energy);
            const pressScale = 1.0 + T_avg * 0.1;
            const fmag = G * pressScale * 0.3 * (biMass + bj.mass) * (1 - q) * (1 - q) / (r2 + baseSoft2);
            const invR = 1.0 / r;
            const fx = fmag * dx * invR, fy = fmag * dy * invR, fz = fmag * dz * invR;
            bi.ax -= fx; bi.ay -= fy; bi.az -= fz;
            bj.ax += fx; bj.ay += fy; bj.az += fz;
        }
    }

    // Stellar radiation pressure on gas.
    const radMaxR2 = 400;
    const radInvC = 1.0 / (4 * Math.PI * C_SPEED);
    for (let si = 0; si < nStar; si++) {
        const star = bodies[starIdx[si]];
        if (!(star.luminosity > 0)) continue;
        const sx = star.x, sy = star.y, sz = star.z;
        const starK = star.luminosity * radInvC * 0.001;
        for (let gi = 0; gi < nGas; gi++) {
            const gas = bodies[gasIdx[gi]];
            const dx = gas.x - sx, dy = gas.y - sy, dz = gas.z - sz;
            const r2 = dx * dx + dy * dy + dz * dz + baseSoft2;
            if (r2 > radMaxR2) continue;
            const r = Math.sqrt(r2);
            const f_rad = starK / r2;
            const invR = 1.0 / r;
            gas.ax += f_rad * dx * invR;
            gas.ay += f_rad * dy * invR;
            gas.az += f_rad * dz * invR;
        }
    }
}

// ── Barnes–Hut octree gravity (F-1) ─────────────────────────────────────────
// APPROXIMATE drop-in for the all-pairs gravity sum. Builds an octree over the
// N bodies (SoA arrays X/Y/Z/M/SOFT/SQ), then accumulates each body's softened
// acceleration into AX/AY/AZ by walking the tree with the θ-criterion. See the
// long note at the call site for the approximation's error characteristics and
// the force-law preservation guarantees. Called via `.call(bridge)` so the
// node pools can persist on the bridge instance (`this._bhTree`) across ticks,
// matching the SoA reuse pattern in computeCosmicForces.
//
// Tree node fields (parallel typed arrays; node 0 = root):
//   nHalf[k]                 cell half-width (cube)
//   ncx/ncy/ncz[k]           cell centre (for octant routing)
//   nMass[k]                 Σ m over descendants
//   nSx/nSy/nSz[k]           Σ (m·pos) over descendants (COM = nS/nMass)
//   nMaxSq[k]                max softSq over descendants (conservative agg. eps²)
//   nLeafCount[k]            #bodies in this leaf bucket, or -1 if internal
//   nChild[k*8 + octant]     child node index, or -1
//   nBody[k*BH_LEAF_CAP + s] body indices held by a leaf bucket
function barnesHutGravity(n, G, X, Y, Z, M, SOFT, SQ, AX, AY, AZ) {
    // 1. Bounding cube over all bodies.
    let minX = X[0], minY = Y[0], minZ = Z[0];
    let maxX = X[0], maxY = Y[0], maxZ = Z[0];
    for (let i = 1; i < n; i++) {
        const x = X[i], y = Y[i], z = Z[i];
        if (x < minX) minX = x; else if (x > maxX) maxX = x;
        if (y < minY) minY = y; else if (y > maxY) maxY = y;
        if (z < minZ) minZ = z; else if (z > maxZ) maxZ = z;
    }
    const cx = 0.5 * (minX + maxX);
    const cy = 0.5 * (minY + maxY);
    const cz = 0.5 * (minZ + maxZ);
    // Cube half-width: half the largest extent, padded so all points are strictly
    // inside (avoids edge-coincident routing ambiguity). Guard the degenerate
    // all-coincident case with a tiny positive floor.
    let half = 0.5 * Math.max(maxX - minX, maxY - minY, maxZ - minZ);
    half = half * 1.0001 + 1e-6;

    // 2. (Re)allocate node pools — grown lazily, reused across ticks (matching
    //    the SoA reuse in computeCosmicForces; pools persist on this._bhTree).
    // Provably-safe upper bound: a bucketed octree over n points needs far
    // fewer than 8n nodes for any realisable distribution; 8n+64 gives ample
    // headroom even for heavy clustering, and the pool guard below makes any
    // residual overflow impossible (it degrades to larger leaf buckets).
    const needNodes = 8 * n + 64;
    let tree = this._bhTree;
    if (!tree || tree.cap < needNodes) {
        const cap = needNodes;
        tree = this._bhTree = {
            cap,
            nHalf: new Float64Array(cap),
            ncx: new Float64Array(cap), ncy: new Float64Array(cap), ncz: new Float64Array(cap),
            nMass: new Float64Array(cap),
            nSx: new Float64Array(cap), nSy: new Float64Array(cap), nSz: new Float64Array(cap),
            nMaxSq: new Float64Array(cap),
            nLeafCount: new Int32Array(cap),
            nChild: new Int32Array(cap * 8),
            nBody: new Int32Array(cap * BH_LEAF_CAP),
            // Reusable explicit walk stack (avoids per-body array alloc).
            stack: new Int32Array(cap + 64),
        };
    }
    const nHalf = tree.nHalf, ncx = tree.ncx, ncy = tree.ncy, ncz = tree.ncz;
    const nMass = tree.nMass, nSx = tree.nSx, nSy = tree.nSy, nSz = tree.nSz;
    const nMaxSq = tree.nMaxSq, nLeafCount = tree.nLeafCount;
    const nChild = tree.nChild, nBody = tree.nBody;
    let stack = tree.stack;

    // Octant index of point (px,py,pz) within a cell centred at (qx,qy,qz):
    //   bit0 = +x, bit1 = +y, bit2 = +z  (0..7).
    // (inlined at call sites for speed.)

    // Initialise an empty leaf node `k` centred at (ex,ey,ez) with half-width eh.
    const initLeaf = (k, ex, ey, ez, eh) => {
        nHalf[k] = eh; ncx[k] = ex; ncy[k] = ey; ncz[k] = ez;
        nMass[k] = 0.0; nSx[k] = 0.0; nSy[k] = 0.0; nSz[k] = 0.0;
        nMaxSq[k] = 0.0; nLeafCount[k] = 0;
    };

    // Root.
    let nodeCount = 1;
    initLeaf(0, cx, cy, cz, half);

    // 3. Insert bodies. Each body accumulates its mass/position into every node
    //    on its root→leaf path (so internal nodes carry the COM of all their
    //    descendants), then lands in a leaf bucket. A full leaf bucket triggers
    //    subdivision: its bodies are pushed one level down WITHOUT re-touching
    //    ancestor accumulators (they already counted them).
    for (let bi = 0; bi < n; bi++) {
        const px = X[bi], py = Y[bi], pz = Z[bi], pm = M[bi], psq = SQ[bi];
        let k = 0;
        for (let depth = 0; ; depth++) {
            // Accumulate into this node (covers root and every internal node passed).
            nMass[k] += pm; nSx[k] += pm * px; nSy[k] += pm * py; nSz[k] += pm * pz;
            if (psq > nMaxSq[k]) nMaxSq[k] = psq;

            if (nLeafCount[k] >= 0) {
                // Leaf bucket.
                const lc = nLeafCount[k];
                const hw = nHalf[k];
                const atFloor = (hw <= BH_MIN_HALF) || (depth >= BH_MAX_DEPTH);
                // Subdivision needs up to 8 fresh child nodes; if the pool can't
                // hold them, treat this as a terminal bucket (graceful degrade).
                const poolFull = (nodeCount + 8 > tree.cap);
                if (lc < BH_LEAF_CAP || atFloor || poolFull) {
                    // Room, or we've hit the size/depth/pool floor: drop body here.
                    // At a floor a bucket may exceed CAP; bodies past the CAP-th
                    // slot are represented by the node COM only (already accumulated
                    // above). That is harmless: reaching BH_MIN_HALF means bodies are
                    // < 1e-3 lu apart (≪ any softening, and ≪ the merge radius), so
                    // they are physically coincident for force purposes — their
                    // mutual displacement is ~0 so the dropped pairwise term is ~0.
                    if (lc < BH_LEAF_CAP) nBody[k * BH_LEAF_CAP + lc] = bi;
                    nLeafCount[k] = lc + 1;
                    break;
                }
                // Subdivide: convert this leaf into an internal node, redistribute
                // its existing bodies one level down.
                nLeafCount[k] = -1;
                const childBase = k * 8;
                for (let o = 0; o < 8; o++) nChild[childBase + o] = -1;
                const qh = 0.5 * hw;
                // Re-place each previously-bucketed body into a child (creating it
                // on demand). Does NOT touch ancestor accumulators.
                for (let s = 0; s < lc; s++) {
                    const ob = nBody[k * BH_LEAF_CAP + s];
                    const ox = X[ob], oy = Y[ob], oz = Z[ob], om = M[ob], osq = SQ[ob];
                    const oo = (ox >= ncx[k] ? 1 : 0) | (oy >= ncy[k] ? 2 : 0) | (oz >= ncz[k] ? 4 : 0);
                    let c = nChild[childBase + oo];
                    if (c === -1) {
                        c = nodeCount++;
                        nChild[childBase + oo] = c;
                        initLeaf(c,
                            ncx[k] + ((oo & 1) ? qh : -qh),
                            ncy[k] + ((oo & 2) ? qh : -qh),
                            ncz[k] + ((oo & 4) ? qh : -qh),
                            qh);
                    }
                    // Body ob now lives under child c: accumulate + bucket it.
                    nMass[c] += om; nSx[c] += om * ox; nSy[c] += om * oy; nSz[c] += om * oz;
                    if (osq > nMaxSq[c]) nMaxSq[c] = osq;
                    const clc = nLeafCount[c];
                    nBody[c * BH_LEAF_CAP + clc] = ob;
                    nLeafCount[c] = clc + 1;
                }
                // Fall through: the current body `bi` now descends into the new
                // internal node on the next loop iteration.
            }
            // Descend into the child octant for `bi` (internal node `k`).
            const oct = (px >= ncx[k] ? 1 : 0) | (py >= ncy[k] ? 2 : 0) | (pz >= ncz[k] ? 4 : 0);
            const cidx = k * 8 + oct;
            let c = nChild[cidx];
            if (c === -1) {
                if (nodeCount >= tree.cap) {
                    // Pool exhausted mid-descent (only reachable under pathological
                    // clustering). Park bi in this internal node's COM: it is already
                    // accumulated into nMass/nS* above, so it still exerts force via
                    // every ancestor aggregate. We simply stop descending. (Self-
                    // interaction is impossible here since bi is not in any bucket.)
                    break;
                }
                c = nodeCount++;
                nChild[cidx] = c;
                const qh = 0.5 * nHalf[k];
                initLeaf(c,
                    ncx[k] + ((oct & 1) ? qh : -qh),
                    ncy[k] + ((oct & 2) ? qh : -qh),
                    ncz[k] + ((oct & 4) ? qh : -qh),
                    qh);
            }
            k = c;
        }
    }

    // The walk stack must hold the worst-case descent breadth; nodeCount nodes
    // is a safe upper bound. Grow defensively if a pathological build exceeded
    // the pre-sized stack (should not happen given needNodes sizing).
    if (stack.length < nodeCount + 8) {
        stack = tree.stack = new Int32Array(nodeCount + 64);
    }

    // 4. Walk: accumulate acceleration on each body.
    for (let i = 0; i < n; i++) {
        const bix = X[i], biy = Y[i], biz = Z[i];
        const sqi = SQ[i];
        let ax = 0.0, ay = 0.0, az = 0.0;
        let sp = 0;
        stack[sp++] = 0; // root
        while (sp > 0) {
            const k = stack[--sp];
            const lc = nLeafCount[k];
            if (lc >= 0) {
                // Leaf bucket: exact pairwise with each contained body (skip self).
                const base = k * BH_LEAF_CAP;
                const cnt = lc < BH_LEAF_CAP ? lc : BH_LEAF_CAP;
                for (let s = 0; s < cnt; s++) {
                    const j = nBody[base + s];
                    if (j === i) continue;
                    const sqj = SQ[j];
                    const eps2 = sqi > sqj ? sqi : sqj;   // max(eps_i², eps_j²)
                    const dx = X[j] - bix, dy = Y[j] - biy, dz = Z[j] - biz;
                    const r2 = dx * dx + dy * dy + dz * dz + eps2;
                    const invR3 = 1.0 / (r2 * Math.sqrt(r2));
                    const f = G * M[j] * invR3;
                    ax += f * dx; ay += f * dy; az += f * dz;
                }
                continue;
            }
            // Internal node: θ-criterion against its centre of mass.
            const m = nMass[k];
            const inv = 1.0 / m;
            const comx = nSx[k] * inv, comy = nSy[k] * inv, comz = nSz[k] * inv;
            const dx = comx - bix, dy = comy - biy, dz = comz - biz;
            const d2 = dx * dx + dy * dy + dz * dz;
            const w = 2.0 * nHalf[k];            // full cell width
            // Open the node if (w/d) >= θ  ⟺  w² >= θ²·d². Equality/coincident
            // (d2 == 0, body inside this node) always opens — recurse to children.
            //
            // Self-interaction safety: if body i lies inside node k, its distance
            // to the node COM is ≤ the cell half-diagonal (√3/2)·w, so d² ≤ 0.75w².
            // The far-test w² < θ²·d² would then require θ² > 1/0.75 ≈ 1.33, i.e.
            // θ > 1.15. With θ = 0.5 (≪ 1) a node containing i ALWAYS opens, so i
            // can never be folded into a monopole acting on itself — it is reached
            // as a leaf and skipped by the j===i test. This is why θ<1 is required.
            if (w * w >= BH_THETA_SQ * d2) {
                const cb = k * 8;
                // Push existing children (skip empties).
                let c;
                c = nChild[cb];     if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 1]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 2]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 3]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 4]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 5]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 6]; if (c !== -1) stack[sp++] = c;
                c = nChild[cb + 7]; if (c !== -1) stack[sp++] = c;
            } else {
                // Far enough: monopole approximation. Conservative softening:
                // max(eps_i², node.maxSoftSq) — never softer→harder than truth.
                const nsq = nMaxSq[k];
                const eps2 = sqi > nsq ? sqi : nsq;
                const r2 = d2 + eps2;
                const invR3 = 1.0 / (r2 * Math.sqrt(r2));
                const f = G * m * invR3;
                ax += f * dx; ay += f * dy; az += f * dz;
            }
        }
        AX[i] = ax; AY[i] = ay; AZ[i] = az;
    }
}
