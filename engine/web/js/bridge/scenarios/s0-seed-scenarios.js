/**
 * S0Seed scenarios — s0-seed-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the large-file refactor (docs/SPEC_REFACTOR_LARGE_FILES.md §4). This
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

/**
 * @param {string} name - scenario identifier
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupS0SeedScenario(name, ctx) {
    if (!name.startsWith('s0-seed-')) return false;
    const { N, mid, midF } = ctx;
            this._initFluxGrid();
            const mc = Math.round(midF);

            switch (name) {
                case 's0-seed-electron': {
                    // Electron seed = unit negative charge + radial-inward
                    // flux envelope at scale K_B.
                    //
                    // Configuration : [SELECTION]  (DERIV_DARK_SECTOR §5.2)
                    // Name          : [IMPOSED]    (structural test absent)
                    // Mass m_e      : [THEOREM]    (m_P·√(2π)·(16/3)·α¹¹,
                    //                              but has NO spatial form)
                    this.injectParticle(mc, mc, mc, -1);
                    const envR = Math.max(3, Math.floor(N / 6));
                    injectRadialEnvelope(this, midF, midF, midF, -1, envR / 2, K_B * 1.5,
                        { radius: envR, minR2: 0.25 });
                    break;
                }

                case 's0-seed-muon':
                case 's0-seed-tau': {
                    // Heavy-lepton seeds (TRACKER §1.9, closed 2026-04-17).
                    //
                    // Same topology as the electron seed — unit s=−1 charge
                    // at centre + radial-inward flux envelope at scale K_B.
                    // The only visual difference is a small amplitude boost
                    // to convey a higher rest-mass energy:
                    //
                    //   electron : K_B · 1.5   (reference)
                    //   muon     : K_B · 1.8   (+20 %)
                    //   tau      : K_B · 2.25  (+50 %)
                    //
                    // Amplitudes are chosen to stay below K_GENESIS (= 3·K_B)
                    // so no spurious mass-genesis fires in neighbouring voxels.
                    //
                    // IMPORTANT — epistemic reality check:
                    //   FTD's mass ratios (μ/e = 207, τ/e = 3477) are
                    //   derived from framework integers and are [THEOREM].
                    //   The LAGRANGIAN mass term encodes the rest-mass
                    //   energy; it has no spatial form. The envelope you
                    //   see here is a visualization [SELECTION], not a
                    //   theory prescription. See S0_SEED_SCENARIO_METADATA
                    //   in engine/web/js/config/scenarios.js for the full
                    //   epistemic breakdown.
                    const boost = (name === 's0-seed-tau') ? 2.25 : 1.80;
                    this.injectParticle(mc, mc, mc, -1);
                    const envR = Math.max(3, Math.floor(N / 6));
                    injectRadialEnvelope(this, midF, midF, midF, -1, envR / 2, K_B * boost,
                        { radius: envR, minR2: 0.25 });
                    break;
                }

                case 's0-seed-photon': {
                    // Photon seed = J_z-polarized Gaussian pulse propagating +x.
                    //
                    // Propagation : [THEOREM]  (c = 1/√3 from cubic-lattice
                    //                           wave equation + CFL)
                    // Pol. (2)    : [THEOREM]  (Gauss constraint ∇·J = 0)
                    // Name        : [SELECTION] (structurally consistent
                    //                            with SM photon)
                    const sigma = 3;
                    const pAmp = K_B * 2;
                    const pStartX = Math.max(4, Math.floor(N / 4));
                    const halfR = 8;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let dx = -halfR; dx <= halfR; dx++) {
                        const x = pStartX + dx;
                        if (x < 0 || x >= N) continue;
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const g = pAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        this._injectFlux(x, y, z, 0, 0, g);      // J_z polarized
                        this._injectWaveVel(x, y, z, g, 0, 0);   // propagate +x
                    }
                    break;
                }

                case 's0-seed-proton-candidate': {
                    // Proton candidate = 3 s=+1 particles on an equilateral
                    // triangle + weak radial-outward flux dressing.
                    //
                    // Configuration : [SELECTION]  (consistent with baryon
                    //                               number 3; geometry not
                    //                               uniquely forced)
                    // Name          : [IMPOSED]    (no color/flavor encoded)
                    // m_p/m_e       : [THEOREM]    (1836.47, no spatial form)
                    //
                    // LANDMINE: do NOT label the three vertices u/u/d or
                    // map J-axes to color charges. The BCC→SU(3) link is a
                    // statement about the gluon propagator, not per-quark
                    // orientation. Any u/d/color assignment here would be
                    // post-hoc pattern matching.
                    const bR = Math.max(2, Math.floor(N / 8));
                    for (let k = 0; k < 3; k++) {
                        const ang = TRIAD_ANGLES[k];
                        const bx = Math.round(midF + bR * Math.cos(ang));
                        const bz = Math.round(midF + bR * Math.sin(ang));
                        this.injectParticle(bx, mc, bz, 1);
                    }
                    const envR = Math.max(3, Math.floor(N / 5));
                    injectRadialEnvelope(this, midF, midF, midF, +1, envR / 2, K_B,
                        { radius: envR, minR2: 0.25 });
                    break;
                }

                // ── Moore Seeds (geometric) ──────────────────────────
                // Mirror the C++ ftd::ctor:: constructors (constructors.h)
                // in JS so the dashboard can visualize them without a
                // WASM rebuild. Theory: THEOREM_MOORE_LAYER_DECOMPOSITION.md

                case 's0-seed-octahedron': {
                    // Shell 1: 6 face-neighbors at L2 distance 1 (SC sublattice)
                    this.injectParticle(mc, mc, mc, -1);  // center anchor
                    const octOffsets = [
                        [1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]
                    ];
                    for (const [dx,dy,dz] of octOffsets) {
                        this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-cuboctahedron': {
                    // Shell 2: 12 edge-neighbors at L2 distance sqrt(2) (FCC sublattice)
                    this.injectParticle(mc, mc, mc, -1);  // center anchor
                    const cubOffsets = [
                        [1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],
                        [1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
                        [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]
                    ];
                    for (const [dx,dy,dz] of cubOffsets) {
                        this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-stella-octangula': {
                    // Shell 3: 8 corner-neighbors at L2 distance sqrt(3) (BCC sublattice)
                    // Two interpenetrating tetrahedra
                    this.injectParticle(mc, mc, mc, -1);  // center anchor
                    const stelOffsets = [
                        [1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],
                        [-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]
                    ];
                    for (const [dx,dy,dz] of stelOffsets) {
                        this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    }
                    break;
                }

                case 's0-seed-moore-cell': {
                    // Full 26-site Moore neighborhood (union of all 3 shells)
                    this.injectParticle(mc, mc, mc, -1);  // center anchor
                    for (let dx = -1; dx <= 1; dx++)
                    for (let dy = -1; dy <= 1; dy++)
                    for (let dz = -1; dz <= 1; dz++) {
                        if (dx === 0 && dy === 0 && dz === 0) continue;
                        this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
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
                    this._injectFlux(mc, mc, mc, 10.0 * K_GENESIS, 0, 0);
                    break;
                }

                case 's0-seed-emergent-ic3-collision': {
                    // FTD-0102 ic3 (two-beam collision). FTD-0107 post-fix
                    // re-measurement: 5/5 seeds = 2 stable clusters of
                    // 2-3 voxels each at the two collision points.
                    // Toggles set by scenario-registry.js at load.
                    const q = Math.max(1, Math.floor(N / 4));
                    this._injectFlux(mc - q, mc, mc, +5.0 * K_GENESIS, 0, 0);
                    this._injectFlux(mc + q, mc, mc, -5.0 * K_GENESIS, 0, 0);
                    break;
                }

                case 's0-seed-emergent-ic4-subthreshold': {
                    // FTD-0102 ic4 (sub-threshold injection).
                    // 0.5·K_GENESIS at centre — below the gap. Pre-registered
                    // outcome: 0 manifested voxels (negative control).
                    // Toggles set by scenario-registry.js at load.
                    this._injectFlux(mc, mc, mc, 0.5 * K_GENESIS, 0, 0);
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
                    this._injectFlux(mc, mc, mc, A_diag, A_diag, A_diag);
                    break;
                }

                case 's0-seed-emergent-ic1-isotropic': {
                    // FTD-0110 D3h: isotropic 6-axis injection.
                    // Distributes 10·K_GENESIS total magnitude across the
                    // 6 SC face-neighbours of the centre, each flux pointing
                    // outward. Tests whether the cluster is fully O_h-symmetric
                    // when the injection direction is symmetrised away.
                    const a_iso = 10.0 * K_GENESIS / Math.sqrt(6);
                    this._injectFlux(mc + 1, mc, mc, +a_iso, 0, 0);
                    this._injectFlux(mc - 1, mc, mc, -a_iso, 0, 0);
                    this._injectFlux(mc, mc + 1, mc, 0, +a_iso, 0);
                    this._injectFlux(mc, mc - 1, mc, 0, -a_iso, 0);
                    this._injectFlux(mc, mc, mc + 1, 0, 0, +a_iso);
                    this._injectFlux(mc, mc, mc - 1, 0, 0, -a_iso);
                    break;
                }

                case 's0-seed-emergent-ic1-viz': {
                    // Clean axial cluster (A=20, T=0). Higher amplitude
                    // compensates for CPU genesis-drain so cluster is
                    // visible in dashboard. Run ~200 ticks for clearest view.
                    this._injectFlux(mc, mc, mc, 20.0 * K_GENESIS, 0, 0);
                    break;
                }
                case 's0-seed-emergent-ic1-diagonal-viz': {
                    const A_dv = 20.0 * K_GENESIS / Math.sqrt(3);
                    this._injectFlux(mc, mc, mc, A_dv, A_dv, A_dv);
                    break;
                }
                case 's0-seed-emergent-ic1-isotropic-viz': {
                    const a_iv = 20.0 * K_GENESIS / Math.sqrt(6);
                    this._injectFlux(mc + 1, mc, mc, +a_iv, 0, 0);
                    this._injectFlux(mc - 1, mc, mc, -a_iv, 0, 0);
                    this._injectFlux(mc, mc + 1, mc, 0, +a_iv, 0);
                    this._injectFlux(mc, mc - 1, mc, 0, -a_iv, 0);
                    this._injectFlux(mc, mc, mc + 1, 0, 0, +a_iv);
                    this._injectFlux(mc, mc, mc - 1, 0, 0, -a_iv);
                    break;
                }

                case 's0-seed-symmetry-regression': {
                    // Engine-fix regression test (2026-04-27).
                    // Inject 6-axis radial flux at centre. Post-fix the
                    // resulting cluster must be centro-symmetric within
                    // numerical noise (T=0 disables Langevin to isolate
                    // determinism). Pre-fix the serial-state RNG broke
                    // y/z reflection symmetry — see render_bridge.cpp
                    // voxel_uniform() docstring.
                    // Toggles set by scenario-registry.js at load.
                    const A = 5.0 * K_GENESIS;
                    this._injectFlux(mc + 1, mc, mc, +A, 0, 0);
                    this._injectFlux(mc - 1, mc, mc, -A, 0, 0);
                    this._injectFlux(mc, mc + 1, mc, 0, +A, 0);
                    this._injectFlux(mc, mc - 1, mc, 0, -A, 0);
                    this._injectFlux(mc, mc, mc + 1, 0, 0, +A);
                    this._injectFlux(mc, mc, mc - 1, 0, 0, -A);
                    break;
                }

                case 's0-seed-moore-decomposition': {
                    // All 3 shells with alternating states so they are
                    // visually distinguishable: shell 1 (+1), shell 2 (-1),
                    // shell 3 (+1), center (-1).
                    this.injectParticle(mc, mc, mc, -1);  // center
                    // Shell 1 — octahedron (face) — state +1
                    for (const [dx,dy,dz] of [
                        [1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]
                    ]) this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    // Shell 2 — cuboctahedron (edge) — state -1
                    for (const [dx,dy,dz] of [
                        [1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],
                        [1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
                        [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]
                    ]) this.injectParticle(mc+dx, mc+dy, mc+dz, -1);
                    // Shell 3 — stella octangula (corner) — state +1
                    for (const [dx,dy,dz] of [
                        [1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],
                        [-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]
                    ]) this.injectParticle(mc+dx, mc+dy, mc+dz, +1);
                    break;
                }

                // ── Level 3-5: Particles, Composites, Atoms ──────────
                // Helper: inject a particle + radial flux dressing
                case 's0-seed-electron-l3':
                case 's0-seed-positron':
                case 's0-seed-neutrino':
                case 's0-seed-quark':
                case 's0-seed-antiquark':
                case 's0-seed-pion':
                case 's0-seed-proton-l4':
                case 's0-seed-neutron':
                case 's0-seed-hydrogen':
                case 's0-seed-helium':
                case 's0-seed-h2-molecule': {
                    const dp  = (cx, cy, cz, st, sp, co, sig, amp, lock = false) =>
                        injectDressedParticle(this, cx, cy, cz, st, sp, co, sig, amp, lock);
                    const tri = (cx, cy, cz, charges, colors, rad, lock = true) =>
                        injectTriad(this, cx, cy, cz, charges, colors, rad, lock);
                    if (name === 's0-seed-electron-l3') dp(mc, mc, mc, -1, -1, 0, Math.max(3, Math.floor(N/10)), K_B*1.5);
                    else if (name === 's0-seed-positron') dp(mc, mc, mc, +1, +1, 0, Math.max(3, Math.floor(N/10)), K_B*1.5);
                    else if (name === 's0-seed-neutrino') {
                        // Soft chirality-biased flux blob (no manifested core).
                        const sig = 2, eR = 6;
                        for (let dz2=-eR; dz2<=eR; dz2++) for (let dy2=-eR; dy2<=eR; dy2++) for (let dx2=-eR; dx2<=eR; dx2++) {
                            const r22=dx2*dx2+dy2*dy2+dz2*dz2; if(r22>eR*eR)continue;
                            const gg=K_B*0.3*Math.exp(-r22/(2*sig*sig)); if(gg<0.001)continue;
                            this._injectFlux(mc+dx2,mc+dy2,mc+dz2, gg*0.55, gg*0.45, 0);
                        }
                    }
                    else if (name === 's0-seed-quark') dp(mc, mc, mc, +1, +1, 1, 2, K_B*0.5);
                    else if (name === 's0-seed-antiquark') dp(mc, mc, mc, -1, -1, 1, 2, K_B*0.5);
                    else if (name === 's0-seed-pion') {
                        const sp=Math.max(3,Math.floor(N/8)), hf=Math.floor(sp/2);
                        dp(mc+hf,mc,mc, +1,+1,1, 2,K_B*0.5, true); dp(mc-hf,mc,mc, -1,-1,1, 2,K_B*0.5, true);
                    }
                    else if (name === 's0-seed-proton-l4') { const bR=Math.max(2,Math.floor(N/8)); tri(mc,mc,mc,[+1,+1,-1],[1,2,3],bR); }
                    else if (name === 's0-seed-neutron') { const bR=Math.max(2,Math.floor(N/8)); tri(mc,mc,mc,[+1,-1,-1],[1,2,3],bR); }
                    else if (name === 's0-seed-hydrogen') {
                        const oR=Math.max(4,Math.floor(N/6)), bR=Math.max(2,Math.floor(N/12));
                        tri(mc,mc,mc,[+1,+1,-1],[1,2,3],bR); dp(mc,mc,mc+oR, -1,-1,0, 2,K_B);
                    }
                    else if (name === 's0-seed-helium') {
                        const oR=Math.max(3,Math.floor(N/8));
                        dp(mc,mc,mc, +1,0,0, 2,K_B*3, true); dp(mc,mc,mc+oR, -1,+1,0, 2,K_B*0.8); dp(mc,mc,mc-oR, -1,-1,0, 2,K_B*0.8);
                    }
                    else if (name === 's0-seed-h2-molecule') {
                        const bd=Math.max(4,Math.floor(N/6)), hf=Math.floor(bd/2), oR=Math.max(3,Math.floor(N/8)), bR=Math.max(1,Math.floor(N/16));
                        tri(mc-hf,mc,mc,[+1,+1,-1],[1,2,3],bR); dp(mc-hf,mc,mc+oR, -1,-1,0, 2,K_B*0.8);
                        tri(mc+hf,mc,mc,[+1,+1,-1],[1,2,3],bR); dp(mc+hf,mc,mc+oR, -1,+1,0, 2,K_B*0.8);
                    }
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
                    this.injectParticle(mc, mc, mc, charge);
                    const lastQ = this._particles[this._particles.length - 1];
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
                        this._injectFlux(mc+dx, mc+dy, mc+dz,
                            sign*g*(dx/r + axisBias[0]),
                            sign*g*(dy/r + axisBias[1]),
                            sign*g*(dz/r + axisBias[2]));
                    }
                    break;
                }

                // Electroweak gauge bosons + Higgs + gluon.
                case 's0-seed-higgs-boson': {
                    // Scalar (spin=0), neutral: void core + radially-
                    // symmetric isotropic flux envelope. Amplitude set
                    // by the FTD m_H/m_e = N_eff/α² ratio, scaled to K_B.
                    const hSig = 2.0, hR = 6, hAmp = K_B * 1.2;
                    // No injectParticle at centre — Higgs is the FIELD,
                    // not a state-manifested particle. Represented by a
                    // localised scalar flux lump.
                    for (let dz=-hR; dz<=hR; dz++)
                    for (let dy=-hR; dy<=hR; dy++)
                    for (let dx=-hR; dx<=hR; dx++) {
                        const r2 = dx*dx + dy*dy + dz*dz;
                        if (r2 === 0 || r2 > hR*hR) continue;
                        const g = hAmp * Math.exp(-r2 / (2 * hSig * hSig));
                        if (g < 1e-3) continue;
                        // Isotropic: equal Jx/Jy/Jz, no preferred axis.
                        const iso = g / Math.sqrt(3);
                        this._injectFlux(mc+dx, mc+dy, mc+dz, iso, iso, iso);
                    }
                    break;
                }

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
                        this._injectFlux(x, y, z,
                            vevAmp + noise*sx,
                            vevAmp + noise*sy,
                            vevAmp + noise*sz);
                    }
                    break;
                }

                case 's0-seed-w-boson': {
                    // Charged (s=+1) localised lump. Flux envelope
                    // chirality-biased via L-axis dominance (use Jx
                    // ahead of Jy/Jz to suggest left-handed coupling).
                    injectParticleFull(this, mc, mc, mc, +1, { spin: +1 });
                    // Chirality bias: +30% weight on Jx relative to transverse.
                    injectRadialEnvelope(this, mc, mc, mc, +1, 1.8, K_B * 1.6,
                        { radius: 5, axisBias: [1.3, 1, 1] });
                    break;
                }

                case 's0-seed-z-boson': {
                    // Neutral (no state-manifested core) localised lump.
                    // Balanced radial-inward envelope, no chirality bias.
                    injectRadialEnvelope(this, mc, mc, mc, -1, 2.0, K_B * 1.8, { radius: 6 });
                    break;
                }

                case 's0-seed-gluon': {
                    // Massless transverse wave like the photon, but with
                    // color charge encoded via axis dominance. Launched
                    // at x ≈ N/4, propagating +x, J_y polarized with
                    // color=G (y-axis dominant).
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
                        this._injectFlux(x, y, z, 0, gg, 0);      // J_y polarised
                        this._injectWaveVel(x, y, z, gg, 0, 0);    // propagate +x
                    }
                    break;
                }

                // Process demos.
                case 's0-seed-beta-decay': {
                    // Neutron triad (2 negative + 1 positive) on an
                    // equilateral triangle, with a pre-seeded electron
                    // and neutrino nearby as the leptonic output of a
                    // future weak transmutation event. Enable
                    // weak_transmutation + dual_substrate toggles to see
                    // polarity flips under stress.
                    const bdR = Math.max(2, Math.floor(N/10));
                    // Three-vertex neutron-ish triangle.
                    for (let k = 0; k < 3; k++) {
                        const ang = TRIAD_ANGLES[k];
                        const bx = Math.round(mc + bdR * Math.cos(ang));
                        const by = Math.round(mc + bdR * Math.sin(ang));
                        const charge = (k === 0) ? +1 : -1;
                        this.injectParticle(bx, by, mc, charge);
                    }
                    // Leptonic output preseeded, offset along +z so they
                    // can be visually associated with the decay direction.
                    const leptonR = Math.max(4, Math.floor(N/5));
                    this.injectParticle(mc, mc, mc + leptonR, -1);      // electron
                    // Neutrino-like: no manifested state, soft L/R flux
                    const nuSig = 2, nuR = 4;
                    for (let dz2=-nuR; dz2<=nuR; dz2++)
                    for (let dy2=-nuR; dy2<=nuR; dy2++)
                    for (let dx2=-nuR; dx2<=nuR; dx2++) {
                        const r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                        if (r22 > nuR*nuR) continue;
                        const g = K_B * 0.3 * Math.exp(-r22/(2*nuSig*nuSig));
                        if (g < 1e-3) continue;
                        this._injectFlux(mc+dx2, mc-leptonR+dy2, mc+dz2, g*0.55, g*0.45, 0);
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
                    this.injectParticle(mc - half, mc, mc, -1);
                    const eP = this._particles[this._particles.length - 1];
                    eP.vx = +0.3 * C_SPEED;

                    // Positron on right, moving left.
                    this.injectParticle(mc + half, mc, mc, +1);
                    const pP = this._particles[this._particles.length - 1];
                    pP.vx = -0.3 * C_SPEED;

                    // Dress each with a small flux envelope so they are
                    // visible as lepton-like lumps before collision.
                    injectRadialEnvelope(this, mc - half, mc, mc, -1, 2, K_B, { radius: 4 });
                    injectRadialEnvelope(this, mc + half, mc, mc, +1, 2, K_B, { radius: 4 });
                    break;
                }

                // ── Level 6: Gauge / Topological ─────────────────────
                case 's0-seed-wilson-loop': {
                    const R = Math.max(3, Math.floor(N/8)), wAmp = K_B;
                    for (let x=mc-R; x<=mc+R; x++) this._injectFlux(x, mc-R, mc, wAmp, 0, 0);
                    for (let y=mc-R; y<=mc+R; y++) this._injectFlux(mc+R, y, mc, 0, wAmp, 0);
                    for (let x=mc+R; x>=mc-R; x--) this._injectFlux(x, mc+R, mc, -wAmp, 0, 0);
                    for (let y=mc+R; y>=mc-R; y--) this._injectFlux(mc-R, y, mc, 0, -wAmp, 0);
                    this.injectParticle(mc-R,mc-R,mc,+1); this.injectParticle(mc+R,mc-R,mc,+1);
                    this.injectParticle(mc+R,mc+R,mc,+1); this.injectParticle(mc-R,mc+R,mc,+1);
                    break;
                }
                case 's0-seed-flux-tube': {
                    const ftSep=Math.max(6,Math.floor(N/4)), ftH=Math.floor(ftSep/2);
                    this.injectParticle(mc-ftH,mc,mc,+1); this.injectParticle(mc+ftH,mc,mc,-1);
                    const ftSig=1.5;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=mc-ftH;x<=mc+ftH;x++) {
                        const dy2=y-mc,dz2=z-mc,p2=dy2*dy2+dz2*dz2;
                        const g=K_B*Math.exp(-p2/(2*ftSig*ftSig)); if(g>0.001) this._injectFlux(x,y,z,g,0,0);
                    }
                    break;
                }
                case 's0-seed-monopole': {
                    const mHalf=(N-1)/2.0;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-mHalf,ry=y-mHalf,rz=z-mHalf;
                        const r=Math.max(Math.sqrt(rx*rx+ry*ry+rz*rz),1.0);
                        const mg=1.0/(4*Math.PI*r*r); if(mg<1e-6) continue;
                        const rxy=Math.sqrt(rx*rx+ry*ry);
                        if(rxy<0.5){this._injectFlux(x,y,z,0,0,mg);continue;}
                        this._injectFlux(x,y,z, -ry/rxy*mg, rx/rxy*mg, 0);
                    }
                    break;
                }
                case 's0-seed-instanton': {
                    const iSize=3.0, iHalf=(N-1)/2.0;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-iHalf,ry=y-iHalf,rz=z-iHalf,r2=rx*rx+ry*ry+rz*rz,r=Math.sqrt(r2);
                        const mg=iSize/(r2+iSize*iSize); if(mg<1e-6||r<0.5) continue;
                        this._injectFlux(x,y,z, mg*rx/r, mg*ry/r, mg*rz/r);
                    }
                    break;
                }

                // ── Level 7: Gravity / Cosmology ─────────────────────
                case 's0-seed-schwarzschild': {
                    // Custom seed-bias inflow toward a central mass: a
                    // visualization aid, NOT engine gravity (does not gate
                    // on the gravity toggle and does not run through the
                    // engine's Newton solver). The G_N factor here is
                    // included so the seed bias scales with the FTD
                    // gravitational-coupling knob in the same direction
                    // as a physical Schwarzschild well.
                    const sHalf=(N-1)/2.0, rs=3.0;
                    this.injectParticle(mc,mc,mc,+1);
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-sHalf,ry=y-sHalf,rz=z-sHalf;
                        const r=Math.max(Math.sqrt(rx*rx+ry*ry+rz*rz),0.5);
                        const mg=G_N * (K_B*rs)/(r*r); if(mg<1e-6) continue;
                        this._injectFlux(x,y,z, -mg*rx/r, -mg*ry/r, -mg*rz/r);
                    }
                    break;
                }
                case 's0-seed-frw-patch': {
                    const frwStride=Math.round(1.0/Math.cbrt(0.01)); let frwSign=1;
                    for (let z=0;z<N;z+=frwStride) for (let y=0;y<N;y+=frwStride) for (let x=0;x<N;x+=frwStride) {
                        this.injectParticle(x,y,z,frwSign); frwSign=-frwSign;
                    }
                    break;
                }
                case 's0-seed-gravitational-wave': {
                    const gwWl=Math.max(4,Math.floor(N/4)), gwK=2*Math.PI/gwWl, gwAmp=0.1;
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const v=gwAmp*Math.sin(gwK*x); if(Math.abs(v)>1e-6) this._injectFlux(x,y,z, 0,v,0);
                    }
                    break;
                }

                // ── Level 8: Consciousness / Observer ────────────────
                case 's0-seed-sloop': {
                    const slR=Math.max(3,Math.floor(N/8)), slN=12, slA=K_B;
                    for (let i=0;i<slN;i++) {
                        const a=2*Math.PI*i/slN;
                        const px=Math.round(mc+slR*Math.cos(a)), py=Math.round(mc+slR*Math.sin(a));
                        this.injectParticle(px,py,mc,+1);
                        this._injectFlux(px,py,mc, -Math.sin(a)*slA, Math.cos(a)*slA, 0);
                    }
                    break;
                }
                case 's0-seed-observer-cell': {
                    this.injectParticle(mc,mc,mc,+1);
                    for (const [dx,dy,dz] of [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]])
                        this.injectParticle(mc+dx,mc+dy,mc+dz,-1);
                    for (const [dx,dy,dz] of [[1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],[1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],[0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]])
                        this.injectParticle(mc+dx,mc+dy,mc+dz,+1);
                    for (const [dx,dy,dz] of [[1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],[-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1]])
                        this.injectParticle(mc+dx,mc+dy,mc+dz,-1);
                    break;
                }
            }
            return true;
}
