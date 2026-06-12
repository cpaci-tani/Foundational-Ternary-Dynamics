/**
 * S0Seed scenarios — s0-seed-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the bridge modularization pass documented in engine/web/docs/INDEX.md. This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('s0-seed-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupS0SeedScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { K_B, K_GENESIS, C_SPEED, G_N } from '../../constants.js';
import {
    TRIAD_ANGLES,
    injectRadialEnvelope,
    injectParticleFull,
    injectDressedParticle,
    injectTriad,
} from './_helpers.js';

function makeBridgeHarness(bridge) {
    return {
        bridge,
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        injectFlux: (x, y, z, fx, fy, fz) => bridge._injectFlux?.(x, y, z, fx, fy, fz),
        injectWaveVel: (x, y, z, vx, vy, vz) => bridge._injectWaveVel?.(x, y, z, vx, vy, vz),
        injectParticle: (x, y, z, state) => bridge.injectParticle?.(x, y, z, state),
    };
}

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness|{N:number, mid:number, midF:number}} harnessOrCtx
 * @param {{N:number, mid:number, midF:number}=} maybeCtx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupS0SeedScenario(name, harnessOrCtx, maybeCtx = null) {
    if (!name.startsWith('s0-seed-')) return false;
    const harness = maybeCtx ? harnessOrCtx : makeBridgeHarness(this);
    const ctx = maybeCtx ?? harnessOrCtx;
    const bridge = harness.bridge ?? this;
    const { N, midF } = ctx;
            bridge._initFluxGrid();
            const mc = Math.round(midF);

            switch (name) {
                // Audit-4 2026-04-28: s0-seed-{electron, muon, tau, photon} removed.
                // These were verbatim mirrors of the s0-vacuum-* counterparts,
                // which are now the canonical entry points (see SPEC_VACUUM_PARTICLE_SCENARIOS.md).
                // s0-seed-proton-candidate also removed earlier in audit-3.

                // ── Moore Seeds (geometric) ──────────────────────────
                // Mirror the C++ ftd::ctor:: constructors (constructors.h)
                // in JS so the dashboard can visualize them without a
                // WASM rebuild. Theory: THEOREM_MOORE_LAYER_DECOMPOSITION.md

                case 's0-seed-octahedron': {
                    // Shell 1: 6 face-neighbors at L2 distance 1 (SC sublattice)
                    harness.injectParticle(mc, mc, mc, -1);  // center anchor
                    const octOffsets = [
                        [1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]
                    ];
                    for (const [dx,dy,dz] of octOffsets) {
                        harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-cuboctahedron': {
                    // Shell 2: 12 edge-neighbors at L2 distance sqrt(2) (FCC sublattice)
                    harness.injectParticle(mc, mc, mc, -1);  // center anchor
                    const cubOffsets = [
                        [1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],
                        [1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
                        [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]
                    ];
                    for (const [dx,dy,dz] of cubOffsets) {
                        harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-stella-octangula': {
                    // Shell 3: 8 corner-neighbors at L2 distance sqrt(3) (BCC sublattice)
                    // Two interpenetrating tetrahedra
                    harness.injectParticle(mc, mc, mc, -1);  // center anchor
                    const stelOffsets = [
                        [1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],
                        [-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]
                    ];
                    for (const [dx,dy,dz] of stelOffsets) {
                        harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-moore-cell': {
                    // Full 26-site Moore neighborhood (union of all 3 shells).
                    // genesis=false (audit-2 2026-04-28): the 27-site geometric
                    // seed should *stay* a 27-site geometric seed. Genesis
                    // contamination drove it to ~30k particles by t=200,
                    // erasing the polyhedral structure.
                    harness.setToggle('genesis', false);
                    harness.injectParticle(mc, mc, mc, -1);  // center anchor
                    for (let dx = -1; dx <= 1; dx++)
                    for (let dy = -1; dy <= 1; dy++)
                    for (let dz = -1; dz <= 1; dz++) {
                        if (dx === 0 && dy === 0 && dz === 0) continue;
                        harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-emergent-ic1': {
                    // FTD-0102 ic1 (point injection). FTD-0107 confirmed
                    // L-invariant 25-voxel cluster at L ∈ {32, 64}, 5/5 seeds.
                    //
                    // Setup: inject 10·K_GENESIS flux at the lattice center,
                    // single voxel. Under (genesis + langevin + gauss-projection
                    // + wave-propagation) toggles, the dynamics produce an
                    // emergent bound state in the SC + FCC + face2 sub-stencils
                    // — predicted to be the L¹-ball-radius-2 octahedral fillout
                    // (centered octahedral number O(2) = 25).
                    //
                    // REQUIRED TOGGLES (set by scenario-registry.js at load):
                    //   genesis, langevin (T=0.005 γ=0.02), gauss-projection,
                    //   wave-propagation.
                    //
                    // What to watch for in the dashboard:
                    //   • Burn-in (~200 ticks): nothing manifested.
                    //   • After burn: 25 voxels manifest as state ±1, arranged
                    //     in a regular octahedral pattern centered on the
                    //     lattice midpoint.
                    //   • Voxels appear in 4 orbits under O_h symmetry:
                    //     1 center + 6 face1 (SC) + 12 edge (FCC) + 6 face2.
                    //   • The 8 BCC corner positions (±1,±1,±1) DO NOT manifest.
                    //
                    // See: docs/theory/08_structural/EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md
                    harness.injectFlux(mc, mc, mc, 10.0 * K_GENESIS, 0, 0);
                    break;
                }

                case 's0-seed-emergent-ic3-collision': {
                    // FTD-0102 ic3 (two-beam collision). FTD-0107 post-fix
                    // re-measurement: 5/5 seeds = 2 stable clusters of
                    // 2-3 voxels each at the two collision points.
                    // Toggles set by scenario-registry.js at load.
                    const q = Math.max(1, Math.floor(N / 4));
                    harness.injectFlux(mc - q, mc, mc, +5.0 * K_GENESIS, 0, 0);
                    harness.injectFlux(mc + q, mc, mc, -5.0 * K_GENESIS, 0, 0);
                    break;
                }

                case 's0-seed-emergent-ic4-subthreshold': {
                    // FTD-0102 ic4 (sub-threshold injection).
                    // 0.5·K_GENESIS at centre — below the gap. Pre-registered
                    // outcome: 0 manifested voxels (negative control).
                    // Toggles set by scenario-registry.js at load.
                    harness.injectFlux(mc, mc, mc, 0.5 * K_GENESIS, 0, 0);
                    break;
                }

                case 's0-seed-emergent-ic2-thermal-runaway': {
                    // FTD-0102 ic2 (thermal-driven runaway). NO flux
                    // injection — only elevated Langevin T = 0.05.
                    // Demonstrates the unstable-phase regime.
                    // Toggles + Langevin T set by scenario-registry.js at load.
                    break;
                }

                case 's0-seed-emergent-ic1-diagonal': {
                    // FTD-0110 D3g: body-diagonal injection.
                    // Same total flux magnitude as ic1 (10·K_GENESIS) but
                    // along (1,1,1)/√3 instead of +x. Tests Z_4 (face-axis,
                    // k=¼) vs Z_3 (body-diagonal, k=⅓) discrimination of
                    // the cluster-efficiency origin. If Z_4 reading correct,
                    // expect ~33-voxel cluster (vs 25 for axial).
                    const A_diag = 10.0 * K_GENESIS / Math.sqrt(3);
                    harness.injectFlux(mc, mc, mc, A_diag, A_diag, A_diag);
                    break;
                }

                case 's0-seed-emergent-ic1-isotropic': {
                    // FTD-0110 D3h: isotropic 6-axis injection.
                    // Distributes 10·K_GENESIS total magnitude across the
                    // 6 SC face-neighbours of the centre, each flux pointing
                    // outward. Tests whether the cluster is fully O_h-symmetric
                    // when the injection direction is symmetrised away.
                    const a_iso = 10.0 * K_GENESIS / Math.sqrt(6);
                    harness.injectFlux(mc + 1, mc, mc, +a_iso, 0, 0);
                    harness.injectFlux(mc - 1, mc, mc, -a_iso, 0, 0);
                    harness.injectFlux(mc, mc + 1, mc, 0, +a_iso, 0);
                    harness.injectFlux(mc, mc - 1, mc, 0, -a_iso, 0);
                    harness.injectFlux(mc, mc, mc + 1, 0, 0, +a_iso);
                    harness.injectFlux(mc, mc, mc - 1, 0, 0, -a_iso);
                    break;
                }

                case 's0-seed-emergent-ic1-viz': {
                    // Clean axial cluster (A=20, T=0). Higher amplitude
                    // compensates for CPU genesis-drain so cluster is
                    // visible in dashboard. Run ~200 ticks for clearest view.
                    harness.injectFlux(mc, mc, mc, 20.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-cluster-law': {
                    // FTD-0269: genesis-burst N(A) cluster-size law (interactive).
                    // A=10 default; the dashboard fire panel re-injects at a
                    // user-chosen A to sweep the broken-power law N(A).
                    // Toggles set by scenario-registry.js at load.
                    harness.injectFlux(mc, mc, mc, 10.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-cluster-law-subknee': {
                    // FTD-0269 answer-key: sub-knee (A=12, T=0). Compact
                    // 27-block cascade. Toggles set by registry at load.
                    harness.injectFlux(mc, mc, mc, 12.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-cluster-law-knee': {
                    // FTD-0269 answer-key: the knee (A=16, T=0) — 27-block escape.
                    harness.injectFlux(mc, mc, mc, 16.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-cluster-law-superknee': {
                    // FTD-0269 answer-key: super-knee (A=40, T=0) — bulk volume,
                    // N = k_eff*A^2.
                    harness.injectFlux(mc, mc, mc, 40.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-emergent-ic1-diagonal-viz': {
                    const A_dv = 20.0 * K_GENESIS / Math.sqrt(3);
                    harness.injectFlux(mc, mc, mc, A_dv, A_dv, A_dv);
                    break;
                }
                case 's0-seed-emergent-ic1-isotropic-viz': {
                    const a_iv = 20.0 * K_GENESIS / Math.sqrt(6);
                    harness.injectFlux(mc + 1, mc, mc, +a_iv, 0, 0);
                    harness.injectFlux(mc - 1, mc, mc, -a_iv, 0, 0);
                    harness.injectFlux(mc, mc + 1, mc, 0, +a_iv, 0);
                    harness.injectFlux(mc, mc - 1, mc, 0, -a_iv, 0);
                    harness.injectFlux(mc, mc, mc + 1, 0, 0, +a_iv);
                    harness.injectFlux(mc, mc, mc - 1, 0, 0, -a_iv);
                    break;
                }

                // s0-seed-symmetry-regression removed 2026-04-28: this was an
                // engine CI regression test (2026-04-27 voxel_uniform() RNG
                // determinism check), not a user-facing physics scenario.
                // If the regression check is still needed, fold it into a
                // ctest under engine/tests/.

                case 's0-seed-moore-decomposition': {
                    // All 3 shells with alternating states so they are
                    // visually distinguishable: shell 1 (+1), shell 2 (-1),
                    // shell 3 (+1), center (-1).
                    harness.injectParticle(mc, mc, mc, -1);  // center
                    // Shell 1 — octahedron (face) — state +1
                    for (const [dx,dy,dz] of [
                        [1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]
                    ]) harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    // Shell 2 — cuboctahedron (edge) — state -1
                    for (const [dx,dy,dz] of [
                        [1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],
                        [1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
                        [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]
                    ]) harness.injectParticle(mc+dx, mc+dy, mc+dz, -1);
                    // Shell 3 — stella octangula (corner) — state +1
                    for (const [dx,dy,dz] of [
                        [1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],
                        [-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]
                    ]) harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    break;
                }

                // ── Level 3-5: Particles, Composites, Atoms ──────────
                // Helper: inject a particle + radial flux dressing.
                // Audit-4 2026-04-28: removed s0-seed-{positron, pion, proton-l4, neutron}
                // — all superseded by s0-vacuum-* equivalents (canonical entries).
                // (Audit-3 also removed electron-l3, neutrino, quark, antiquark.)
                // Composite ATOMS stay because they have no vacuum-family equivalent yet.
                case 's0-seed-hydrogen':
                case 's0-seed-helium':
                case 's0-seed-h2-bond-formation': {
                    const dp  = (cx, cy, cz, st, sp, co, sig, amp, lock = false) =>
                        injectDressedParticle(harness, cx, cy, cz, st, sp, co, sig, amp, lock);
                    const tri = (cx, cy, cz, charges, colors, rad, lock = true) =>
                        injectTriad(harness, cx, cy, cz, charges, colors, rad, lock);
                    if (name === 's0-seed-hydrogen') {
                        const oR=Math.max(4,Math.floor(N/6)), bR=Math.max(2,Math.floor(N/12));
                        tri(mc,mc,mc,[+1,+1,-1],[1,2,3],bR); dp(mc,mc,mc+oR, -1,-1,0, 2,K_B);
                    }
                    else if (name === 's0-seed-helium') {
                        // ⁴He / α-particle: 2 protons + 2 neutrons at the
                        // 4 vertices of a tetrahedron, plus 2 electrons in
                        // the 1s² shell at ±z. Each nucleon is a 3-quark
                        // triad (proton charges [+1,+1,-1], neutron [+1,-1,-1])
                        // sharing the {1,2,3} color triple.
                        //
                        // Audit 2026-04-28 fix: previous body was a single
                        // dressed +1 particle posing as the nucleus —
                        // physically wrong. Now: 4 nucleons × 3 quarks +
                        // 2 electrons = 14 manifested particles.
                        const oR = Math.max(3, Math.floor(N/8));        // electron-shell radius
                        const nR = Math.max(2, Math.floor(N/12));       // nucleon-center offset
                        const bR = Math.max(1, Math.floor(N/16));       // intra-triad radius
                        const tet = [
                            [+nR, +nR, +nR],   // proton 1
                            [-nR, -nR, +nR],   // proton 2
                            [+nR, -nR, -nR],   // neutron 1
                            [-nR, +nR, -nR],   // neutron 2
                        ];
                        const pCharges = [+1, +1, -1];
                        const nCharges = [+1, -1, -1];
                        const colors   = [1, 2, 3];
                        for (let i = 0; i < 4; i++) {
                            const [dx, dy, dz] = tet[i];
                            const charges = (i < 2) ? pCharges : nCharges;
                            tri(mc + dx, mc + dy, mc + dz, charges, colors, bR);
                        }
                        // 1s² electron shell (2 electrons, opposite spins) along ±z.
                        dp(mc, mc, mc + oR, -1, +1, 0, 2, K_B * 0.8);
                        dp(mc, mc, mc - oR, -1, -1, 0, 2, K_B * 0.8);
                    }
                    else if (name === 's0-seed-h2-bond-formation') {
                        const bd = Math.max(4, Math.floor(N / 6));
                        const hf = Math.floor(bd / 2);
                        const bR = Math.max(1, Math.floor(N / 16));
                        // Place two hydrogen nuclei close together:
                        tri(mc - hf * 0.7, mc, mc, [+1, +1, -1], [1, 2, 3], bR);
                        tri(mc + hf * 0.7, mc, mc, [+1, +1, -1], [1, 2, 3], bR);
                        // Seed two shared electrons in the center with opposite spins:
                        dp(mc, mc, mc + 1, -1, -1, 0, 2, K_B * 0.8);
                        dp(mc, mc, mc - 1, -1, +1, 0, 2, K_B * 0.8);
                    }
                    break;
                }

                case 's0-seed-spark-of-life': {
                    // Demo only: a mineral-pore-like seed plus flux-fed
                    // threshold crossing, not a biochemical or replication
                    // claim.
                    const ringR = Math.max(5, Math.floor(N / 8));
                    const ringSites = 16;
                    for (let i = 0; i < ringSites; i++) {
                        const angle = 2 * Math.PI * i / ringSites;
                        const px = Math.round(mc + ringR * Math.cos(angle));
                        const py = Math.round(mc + ringR * Math.sin(angle));
                        const state = (i % 2 === 0) ? +1 : -1;
                        injectParticleFull(harness, px, py, mc, state, {
                            spin: state,
                            color: 0,
                            locked: true,
                        });
                    }

                    const precursorR = Math.max(ringR + 4, Math.floor(N / 4));
                    const precursorSpeed = 0.12 * C_SPEED;
                    for (let k = 0; k < 4; k++) {
                        const angle = 2 * Math.PI * k / 4;
                        const dirX = Math.cos(angle);
                        const dirY = Math.sin(angle);
                        const tanX = -dirY;
                        const tanY = dirX;
                        for (let j = 0; j < 2; j++) {
                            const side = (j === 0) ? -1 : +1;
                            const state = (j === 0) ? +1 : -1;
                            const px = Math.round(mc + precursorR * dirX + side * tanX);
                            const py = Math.round(mc + precursorR * dirY + side * tanY);
                            injectDressedParticle(harness, px, py, mc, state,
                                state, ((k + j) % 3) + 1, 1.6, K_B * 0.7, false);
                            const list = harness.bridge?._particles;
                            const p = list ? list[list.length - 1] : null;
                            if (p) {
                                p.vx = -dirX * precursorSpeed;
                                p.vy = -dirY * precursorSpeed;
                                p.vz = 0;
                            }
                        }
                    }

                    const triR = Math.max(2, Math.floor(N / 18));
                    injectTriad(harness, mc, mc, mc, [+1, -1, +1], [1, 2, 3], triR, false);

                    const spark = 6.0 * K_GENESIS / Math.sqrt(6.0);
                    harness.injectFlux(mc + 1, mc, mc, +spark, 0, 0);
                    harness.injectFlux(mc - 1, mc, mc, -spark, 0, 0);
                    harness.injectFlux(mc, mc + 1, mc, 0, +spark, 0);
                    harness.injectFlux(mc, mc - 1, mc, 0, -spark, 0);
                    harness.injectFlux(mc, mc, mc + 1, 0, 0, +spark);
                    harness.injectFlux(mc, mc, mc - 1, 0, 0, -spark);

                    const daughterPocket = (cx, cy, cz, sign) => {
                        injectRadialEnvelope(harness, cx, cy, cz, sign, 2.0, 0.75 * K_GENESIS, {
                            radius: 4,
                            minR2: 0,
                            minVal: 0.001,
                        });
                    };
                    const daughterR = Math.max(ringR + 2, Math.floor(N / 6));
                    daughterPocket(mc - daughterR, mc, mc + 2, +1);
                    daughterPocket(mc + daughterR, mc, mc - 2, -1);
                    break;
                }

                // ─────────────────────────────────────────────────────
                // LHC Standard Model scenarios (added 2026-04-17)
                // ─────────────────────────────────────────────────────
                // Individual quark flavours — colored particles with
                // generation-hierarchy amplitude scaling. Epistemically:
                // all quark masses are [OPEN] in FTD (see TRACKER §4.1),
                // so amplitudes here are visualisation cues, not physics.
                case 's0-seed-up-quark':
                case 's0-seed-down-quark':
                case 's0-seed-strange-quark':
                case 's0-seed-charm-quark':
                case 's0-seed-bottom-quark':
                case 's0-seed-top-quark': {
                    // Color assignment: R=1, G=2, B=3. Same-generation
                    // doublet (u/d, c/s, t/b) alternates colors to keep
                    // the catalog visually distinct.
                    let charge, color, ampBoost;
                    switch (name) {
                        case 's0-seed-up-quark':      charge=+1; color=1; ampBoost=0.5;  break;
                        case 's0-seed-down-quark':    charge=-1; color=2; ampBoost=0.5;  break;
                        case 's0-seed-strange-quark': charge=-1; color=3; ampBoost=0.7;  break;
                        case 's0-seed-charm-quark':   charge=+1; color=1; ampBoost=1.0;  break;
                        case 's0-seed-bottom-quark':  charge=-1; color=2; ampBoost=1.4;  break;
                        case 's0-seed-top-quark':     charge=+1; color=3; ampBoost=2.5;  break;
                    }
                    harness.injectParticle(mc, mc, mc, charge);
                    const lastQ = harness.bridge._particles[harness.bridge._particles.length - 1];
                    lastQ.color = color;
                    lastQ.spin = (charge > 0) ? +1 : -1;

                    // Narrow Gaussian envelope — smaller than a lepton's
                    // to suggest the "point-like" quark character. Amplitude
                    // stays below K_GENESIS so no spurious genesis.
                    const qSig = 1.5, qR = 4, qAmp = K_B * ampBoost;
                    for (let dz=-qR; dz<=qR; dz++)
                    for (let dy=-qR; dy<=qR; dy++)
                    for (let dx=-qR; dx<=qR; dx++) {
                        const r2 = dx*dx + dy*dy + dz*dz;
                        if (r2 === 0 || r2 > qR*qR) continue;
                        const r = Math.sqrt(r2);
                        const g = qAmp * Math.exp(-r2 / (2 * qSig * qSig));
                        if (g < 1e-3) continue;
                        const sign = (charge > 0) ? 1 : -1;
                        // Bias along the color axis so dominant-flux-axis
                        // labelling recovers `color` consistently.
                        const axisBias = [0, 0, 0];
                        axisBias[color - 1] = 0.5;
                        harness.injectFlux(mc+dx, mc+dy, mc+dz,
                            sign*g*(dx/r + axisBias[0]),
                            sign*g*(dy/r + axisBias[1]),
                            sign*g*(dz/r + axisBias[2]));
                    }
                    break;
                }

                // Audit-4 2026-04-28: s0-seed-higgs-boson removed (mirror of
                // s0-vacuum-higgs). s0-seed-higgs-field stays — distinct VEV
                // background scenario with no vacuum-family equivalent.
                case 's0-seed-higgs-field': {
                    // Uniform low-amplitude flux background representing
                    // the VEV, with small random perturbations (thermal-
                    // like fluctuations around equilibrium). Tunable by
                    // amplitude; here we use a conservative value so
                    // genesis threshold is never accidentally crossed.
                    const vevAmp = K_B * 0.3;        // VEV baseline
                    const noise  = K_B * 0.05;        // fluctuation scale
                    for (let z=0; z<N; z++)
                    for (let y=0; y<N; y++)
                    for (let x=0; x<N; x++) {
                        // Small deterministic perturbation so every tick
                        // looks identical (reproducible) yet non-uniform.
                        const sx = Math.sin(0.19*x + 0.23*y + 0.29*z);
                        const sy = Math.sin(0.37*x + 0.13*y + 0.17*z);
                        const sz = Math.sin(0.11*x + 0.31*y + 0.41*z);
                        harness.injectFlux(x, y, z,
                            vevAmp + noise*sx,
                            vevAmp + noise*sy,
                            vevAmp + noise*sz);
                    }
                    break;
                }

                // Audit-4 2026-04-28: s0-seed-{w-boson, z-boson} removed —
                // mirrors of s0-vacuum-{w-boson, z-boson}. Gluon stays — no
                // vacuum-family equivalent (color-charged scenarios are
                // research, not pedagogical).
                case 's0-seed-gluon': {
                    // Massless transverse wave like the photon, but with
                    // color charge encoded via axis dominance. Launched
                    // at x ≈ N/4, propagating +x, J_y polarized with
                    // color=G (y-axis dominant).
                    //
                    // genesis=false: same fix as photon (audit 2026-04-28) —
                    // a free gauge-boson wave should not pair-produce.
                    harness.setToggle('genesis', false);
                    const sigma = 3;
                    const gAmp = K_B * 2;
                    const startX = Math.max(4, Math.floor(N / 4));
                    const halfR = 8;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let dx = -halfR; dx <= halfR; dx++) {
                        const x = startX + dx;
                        if (x < 0 || x >= N) continue;
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx*dx + dy*dy + dz*dz;
                        const gg = gAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (gg < 1e-6) continue;
                        harness.injectFlux(x, y, z, 0, gg, 0);      // J_y polarised
                        harness.injectWaveVel(x, y, z, gg, 0, 0);    // propagate +x
                    }
                    break;
                }

                // Process demos.
                case 's0-seed-beta-decay': {
                    // Neutron triad (2 negative + 1 positive) on an
                    // equilateral triangle, with a pre-seeded electron
                    // and neutrino nearby as the leptonic output of a
                    // future weak transmutation event.
                    //
                    // Audit 2026-04-28: previously the comment said "enable
                    // weak_transmutation + dual_substrate toggles to see
                    // polarity flips" — but the user had to do it manually.
                    // Now the scenario auto-enables both toggles so the
                    // decay actually fires by default. Both are in the
                    // scenario-mutable whitelist (CONTRACTS.md §s0-seed).
                    harness.setToggle('weak_transmutation', true);
                    harness.setToggle('dual_substrate', true);
                    const bdR = Math.max(2, Math.floor(N/10));
                    // Three-vertex neutron-ish triangle.
                    for (let k = 0; k < 3; k++) {
                        const ang = TRIAD_ANGLES[k];
                        const bx = Math.round(mc + bdR * Math.cos(ang));
                        const by = Math.round(mc + bdR * Math.sin(ang));
                        const charge = (k === 0) ? +1 : -1;
                        harness.injectParticle(bx, by, mc, charge);
                    }
                    // Leptonic output preseeded, offset along +z so they
                    // can be visually associated with the decay direction.
                    const leptonR = Math.max(4, Math.floor(N/5));
                    harness.injectParticle(mc, mc, mc + leptonR, -1);      // electron
                    // Neutrino-like: no manifested state, soft L/R flux
                    const nuSig = 2, nuR = 4;
                    for (let dz2=-nuR; dz2<=nuR; dz2++)
                    for (let dy2=-nuR; dy2<=nuR; dy2++)
                    for (let dx2=-nuR; dx2<=nuR; dx2++) {
                        const r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                        if (r22 > nuR*nuR) continue;
                        const g = K_B * 0.3 * Math.exp(-r22/(2*nuSig*nuSig));
                        if (g < 1e-3) continue;
                        harness.injectFlux(mc+dx2, mc-leptonR+dy2, mc+dz2, g*0.55, g*0.45, 0);
                    }
                    // Toggle activation (weak_transmutation + dual_substrate)
                    // is handled via SCALE0_SCENARIO_OVERRIDES in
                    // engine/web/js/config/toggles.js. Do NOT mutate
                    // this._toggles directly here — applyToggleDefaults
                    // runs AFTER scenario setup and would clobber a
                    // direct mutation, and the validator would briefly
                    // see weak_transmutation=true with dual_substrate=
                    // false on the very next tick (C-arch-8).
                    break;
                }

                case 's0-seed-ee-annihilation': {
                    // Electron and positron on opposing faces of a
                    // central axis, moving toward each other. The
                    // collision resolution in phase_movement will
                    // recognise opposite-sign contact and annihilate
                    // the pair into a radial flux burst (two-photon-
                    // like final state).
                    const aSep = Math.max(6, Math.floor(N/3));
                    const half = Math.floor(aSep / 2);

                    // Electron on left, moving right.
                    harness.injectParticle(mc - half, mc, mc, -1);
                    const eP = harness.bridge._particles[harness.bridge._particles.length - 1];
                    eP.vx = +0.3 * C_SPEED;

                    // Positron on right, moving left.
                    harness.injectParticle(mc + half, mc, mc, +1);
                    const pP = harness.bridge._particles[harness.bridge._particles.length - 1];
                    pP.vx = -0.3 * C_SPEED;

                    // Dress each with a small flux envelope so they are
                    // visible as lepton-like lumps before collision.
                    injectRadialEnvelope(harness, mc - half, mc, mc, -1, 2, K_B, { radius: 4 });
                    injectRadialEnvelope(harness, mc + half, mc, mc, +1, 2, K_B, { radius: 4 });
                    break;
                }

                case 's0-seed-quark-gluon-plasma': {
                    // QGP: 8 quarks (alternating charges/colors) in a tight 4x4x4 cube around center.
                    const qOffset = 2;
                    let quarkIndex = 0;
                    for (const dx of [-qOffset, qOffset])
                    for (const dy of [-qOffset, qOffset])
                    for (const dz of [-qOffset, qOffset]) {
                        const charge = (quarkIndex % 2 === 0) ? +1 : -1;
                        const color = (quarkIndex % 3) + 1; // R=1, G=2, B=3
                        harness.injectParticle(mc + dx, mc + dy, mc + dz, charge);
                        const q = harness.bridge._particles[harness.bridge._particles.length - 1];
                        q.color = color;
                        q.spin = (charge > 0) ? +1 : -1;

                        // High thermal random velocity, speed = 0.5 * C_SPEED
                        const theta = Math.random() * Math.PI * 2;
                        const phi = Math.acos(Math.random() * 2 - 1);
                        const speed = 0.5 * C_SPEED;
                        q.vx = speed * Math.sin(phi) * Math.cos(theta);
                        q.vy = speed * Math.sin(phi) * Math.sin(theta);
                        q.vz = speed * Math.cos(phi);

                        quarkIndex++;
                    }

                    // Seed random high-energy gluon flux pulses in a central 8x8x8 region:
                    const pulseR = 4;
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > pulseR * pulseR) continue;

                        const amp = K_B * 3.0 * Math.random();
                        const theta = Math.random() * Math.PI * 2;
                        const phi = Math.acos(Math.random() * 2 - 1);

                        const jx = amp * Math.sin(phi) * Math.cos(theta);
                        const jy = amp * Math.sin(phi) * Math.sin(theta);
                        const jz = amp * Math.cos(phi);

                        const wx = amp * Math.sin(phi) * Math.cos(theta) * C_SPEED;
                        const wy = amp * Math.sin(phi) * Math.sin(theta) * C_SPEED;
                        const wz = amp * Math.cos(phi) * C_SPEED;

                        harness.injectFlux(mc + dx, mc + dy, mc + dz, jx, jy, jz);
                        harness.injectWaveVel(mc + dx, mc + dy, mc + dz, wx, wy, wz);
                    }
                    break;
                }

                case 's0-seed-gravitational-lensing': {
                    // Schwarzschild well at the center:
                    const sHalf = midF, rs = 3.0;
                    harness.injectParticle(mc, mc, mc, +1);
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
                        const r = Math.max(Math.sqrt(rx * rx + ry * ry + rz * rz), 0.5);
                        const mg = G_N * (K_B * rs) / (r * r);
                        if (mg < 1e-6) continue;
                        harness.injectFlux(x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
                    }

                    // Off-axis photon pulse launched at x0 = N/4, propagating in +x:
                    const x0 = Math.floor(N / 4);
                    const offset = Math.max(4, Math.floor(N / 6));
                    const y0 = mc + offset;
                    const z0 = mc;

                    const sigma = Math.max(2, Math.floor(N / 12));
                    const amp = K_B * 3; // high amplitude so it stands out
                    const lambdaEff = 4 * sigma;
                    const k = 2 * Math.PI / lambdaEff;
                    const cutR = 3.0 * sigma;
                    const cutR2 = cutR * cutR;

                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - x0, dy = y - y0, dz = z - z0;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > cutR2) continue;
                        const g = Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        const phase = k * dx;
                        const jz = amp * g * Math.sin(phase);
                        const wz = amp * g * Math.cos(phase) * C_SPEED;
                        harness.injectFlux(x, y, z, 0, 0, jz);
                        harness.injectWaveVel(x, y, z, wz, 0, 0);
                    }
                    break;
                }

                // ── Level 6: Gauge / Topological ─────────────────────
                case 's0-seed-wilson-loop': {
                    const R = Math.max(3, Math.floor(N/8)), wAmp = K_B;
                    for (let x=mc-R; x<=mc+R; x++) harness.injectFlux(x, mc-R, mc, wAmp, 0, 0);
                    for (let y=mc-R; y<=mc+R; y++) harness.injectFlux(mc+R, y, mc, 0, wAmp, 0);
                    for (let x=mc+R; x>=mc-R; x--) harness.injectFlux(x, mc+R, mc, -wAmp, 0, 0);
                    for (let y=mc+R; y>=mc-R; y--) harness.injectFlux(mc-R, y, mc, 0, -wAmp, 0);
                    harness.injectParticle(mc-R,mc-R,mc,+1); harness.injectParticle(mc+R,mc-R,mc,+1);
                    harness.injectParticle(mc+R,mc+R,mc,+1); harness.injectParticle(mc-R,mc+R,mc,+1);
                    break;
                }
                case 's0-seed-flux-tube': {
                    const ftSep=Math.max(6,Math.floor(N/4)), ftH=Math.floor(ftSep/2);
                    harness.injectParticle(mc-ftH,mc,mc,+1); harness.injectParticle(mc+ftH,mc,mc,-1);
                    const ftSig=1.5;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=mc-ftH;x<=mc+ftH;x++) {
                        const dy2=y-mc,dz2=z-mc,p2=dy2*dy2+dz2*dz2;
                        const g=K_B*Math.exp(-p2/(2*ftSig*ftSig)); if(g>0.001) harness.injectFlux(x,y,z,g,0,0);
                    }
                    break;
                }
                case 's0-seed-monopole': {
                    const mHalf=midF;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-mHalf,ry=y-mHalf,rz=z-mHalf;
                        const r=Math.max(Math.sqrt(rx*rx+ry*ry+rz*rz),1.0);
                        const mg=1.0/(4*Math.PI*r*r); if(mg<1e-6) continue;
                        const rxy=Math.sqrt(rx*rx+ry*ry);
                        if(rxy<0.5){harness.injectFlux(x,y,z,0,0,mg);continue;}
                        harness.injectFlux(x,y,z, -ry/rxy*mg, rx/rxy*mg, 0);
                    }
                    break;
                }
                case 's0-seed-instanton': {
                    const iSize=3.0, iHalf=midF;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-iHalf,ry=y-iHalf,rz=z-iHalf,r2=rx*rx+ry*ry+rz*rz,r=Math.sqrt(r2);
                        const mg=iSize/(r2+iSize*iSize); if(mg<1e-6||r<0.5) continue;
                        harness.injectFlux(x,y,z, mg*rx/r, mg*ry/r, mg*rz/r);
                    }
                    break;
                }
                case 's0-seed-schwarzschild': {
                    // Custom seed-bias inflow toward a central mass: a
                    // visualization aid, NOT engine gravity (does not gate
                    // on the gravity toggle and does not run through the
                    // engine's Newton solver). The G_N factor here is
                    // included so the seed bias scales with the FTD
                    // gravitational-coupling knob in the same direction
                    // as a physical Schwarzschild well.
                    const sHalf=midF, rs=3.0;
                    harness.injectParticle(mc,mc,mc,+1);
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-sHalf,ry=y-sHalf,rz=z-sHalf;
                        const r=Math.max(Math.sqrt(rx*rx+ry*ry+rz*rz),0.5);
                        const mg=G_N * (K_B*rs)/(r*r); if(mg<1e-6) continue;
                        harness.injectFlux(x,y,z, -mg*rx/r, -mg*ry/r, -mg*rz/r);
                    }
                    break;
                }
                case 's0-seed-massive-body': {
                    // A dense ball of LOCKED rest mass. Gravity from REAL manifested
                    // mass (rho = M_REST*|state|) via the latency-Poisson solver
                    // (latency_field on), not the |J|^2 field-energy proxy. Locked => static.
                    const R = Math.min(2, Math.max(1, Math.floor(N/16)));
                    const R2 = R*R;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-midF, ry=y-midF, rz=z-midF;
                        if (rx*rx+ry*ry+rz*rz > R2) continue;
                        harness.injectParticle(x, y, z, +1, { locked: true });
                    }
                    break;
                }
                case 's0-seed-gravitational-wave': {
                    const gwWl=Math.max(4,Math.floor(N/4)), gwK=2*Math.PI/gwWl, gwAmp=0.1;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const v=gwAmp*Math.sin(gwK*x); if(Math.abs(v)>1e-6) harness.injectFlux(x,y,z, 0,v,0);
                    }
                    break;
                }

                // ── Time-dilation scenarios (2026-06-07) ───────────────
                // Thin reuse wrappers for the Time Observatory panel. Each
                // delegates to an existing gravity seed so the latency well
                // (gravitational clock-slowdown) is real; no new physics.
                //
                // NOTE: the latency SAMPLER (getLatencySampled, both bridges)
                // builds the dτ/dt proxy from the |J|² flux field — so the Time
                // panel needs a FLUX-producing well, not the locked-rest-mass
                // body (whose flux is 0 and whose real Poisson latency is only
                // surfaced via the [C++] getGravityMetricAgg block). The
                // gravitational-wave / Schwarzschild seeds inject flux, giving a
                // measurable latency well on every bridge.
                //   gravity-well + twin-clocks → the gravitational-wave flux well
                //   horizon                    → the seed-bias Schwarzschild well
                case 's0-seed-time-gravity-well':
                case 's0-seed-time-twin-clocks': {
                    setupS0SeedScenario('s0-seed-gravitational-wave', harness, ctx);
                    break;
                }
                case 's0-seed-time-horizon': {
                    setupS0SeedScenario('s0-seed-schwarzschild', harness, ctx);
                    break;
                }

                // ── Level 8: Reference frame context / Observer ────────────────
                case 's0-seed-sloop': {
                    const slR=Math.max(3,Math.floor(N/8)), slN=12, slA=K_B;
                    for (let i=0;i<slN;i++) {
                        const a=2*Math.PI*i/slN;
                        const px=Math.round(mc+slR*Math.cos(a)), py=Math.round(mc+slR*Math.sin(a));
                        harness.injectParticle(px,py,mc,+1);
                        harness.injectFlux(px,py,mc, -Math.sin(a)*slA, Math.cos(a)*slA, 0);
                    }
                    break;
                }
                case 's0-seed-observer-cell': {
                    harness.injectParticle(mc,mc,mc,+1);
                    for (const [dx,dy,dz] of [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]])
                        harness.injectParticle(mc+dx,mc+dy,mc+dz,-1);
                    for (const [dx,dy,dz] of [[1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],[1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],[0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]])
                        harness.injectParticle(mc+dx,mc+dy,mc+dz,+1);
                    for (const [dx,dy,dz] of [[1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],[-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]])
                        harness.injectParticle(mc+dx,mc+dy,mc+dz,-1);
                    break;
                }
                case 's0-seed-de-broglie-clock': {
                    // FTD-0271: de Broglie internal clock. Central manifested
                    // 7^3 block carrying a uniform flux J0; the de-broglie-clock
                    // panel turns on the de_broglie_clock toggle so the KG mass
                    // term -omega0^2*J makes the block's flux oscillate at omega0
                    // (the rest-frame Compton clock). [CONDITIONAL] -- omega0 is
                    // IMPOSED; mirrors the C++ s0_seed.cpp branch for parity.
                    harness.setToggle('genesis', false);
                    harness.setToggle('damping', false);
                    const J0 = 0.08, half = 3;
                    for (let dx = -half; dx <= half; dx++)
                        for (let dy = -half; dy <= half; dy++)
                            for (let dz = -half; dz <= half; dz++) {
                                harness.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                                harness.injectFlux(mc+dx, mc+dy, mc+dz, J0, 0, 0);
                            }
                    break;
                }
            }
            return true;
}
