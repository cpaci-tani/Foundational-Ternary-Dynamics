// @ts-check
import { test } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const NAMES = [
    // Vacuum (15)
    's0-vacuum-electron','s0-vacuum-muon','s0-vacuum-tau',
    's0-vacuum-electron-neutrino','s0-vacuum-muon-neutrino','s0-vacuum-tau-neutrino',
    's0-vacuum-photon','s0-vacuum-w-boson','s0-vacuum-z-boson','s0-vacuum-higgs',
    's0-vacuum-proton','s0-vacuum-neutron',
    's0-vacuum-pion-charged','s0-vacuum-pion-neutral','s0-vacuum-kaon-charged',
    // Quark flavors (6)
    's0-seed-up-quark','s0-seed-down-quark','s0-seed-strange-quark',
    's0-seed-charm-quark','s0-seed-bottom-quark','s0-seed-top-quark',
    // Bosons + processes (4)
    's0-seed-gluon','s0-seed-higgs-field','s0-seed-beta-decay','s0-seed-ee-annihilation',
    // Composites + atoms (3)
    's0-seed-hydrogen','s0-seed-helium','s0-seed-2-hydrogen-atoms',
    // Geometric / topology (15)
    's0-seed-octahedron','s0-seed-cuboctahedron','s0-seed-stella-octangula',
    's0-seed-moore-cell','s0-seed-moore-decomposition','s0-seed-observer-cell',
    's0-seed-monopole','s0-seed-instanton','s0-seed-sloop',
    's0-seed-schwarzschild','s0-seed-gravitational-wave',
    's0-seed-wilson-loop','s0-seed-flux-tube',
    // Emergent (9)
    's0-seed-emergent-ic1','s0-seed-emergent-ic1-viz',
    's0-seed-emergent-ic1-diagonal','s0-seed-emergent-ic1-diagonal-viz',
    's0-seed-emergent-ic1-isotropic','s0-seed-emergent-ic1-isotropic-viz',
    's0-seed-emergent-ic2-thermal-runaway','s0-seed-emergent-ic3-collision',
    's0-seed-emergent-ic4-subthreshold',
    // Flux dynamics (20)
    'flux-pulse','flux-dipole','flux-standing','flux-nested-standing',
    'flux-soliton','flux-interference','flux-vortex','flux-dual-substrate',
    'flux-cascade','flux-pair-production','flux-annihilation','flux-vacuum-foam',
    'flux-meson','flux-string-breaking','flux-baryon','flux-screening',
    'flux-triad','flux-cyclotron','flux-random-genesis','flux-thermalization',
    // Light (4)
    'light-rainbow','light-dipole','light-two-slit','light-photon-race',
    // Quantum (8)
    'quantum-born-rule','quantum-double-slit','quantum-tunnel','quantum-well',
    'quantum-entangle','quantum-aharonov-bohm','quantum-casimir','quantum-zeno',
    // S0 field (8)
    's0-field-plane-wave','s0-field-standing-wave','s0-field-photon-pulse',
    's0-field-electric-dipole','s0-field-magnetic-dipole',
    's0-field-uniform-e','s0-field-uniform-b','s0-field-vortex-line',
];

test.setTimeout(900_000);
test('per-scenario position audit', async ({ page }) => {
    page.on('pageerror', (e) => console.error('PAGEERROR:', e.message));
    await gotoAndReady(page);
    const data = await page.evaluate(async (names) => {
        const b = window._ftdBridge;
        const N = b.latticeSize ?? 32;
        const midF = (N - 1) / 2;
        const out = [];
        for (const n of names) {
            try {
                b.setupScenario(n);
                // Particle CoM at t=0
                const pd0 = b.getParticleData();
                let pCoM0 = null, pCount0 = pd0?.count|0;
                if (pCount0 > 0) {
                    let sx=0,sy=0,sz=0;
                    for (let i=0;i<pCount0;i++) { sx+=pd0.positions[i*3]; sy+=pd0.positions[i*3+1]; sz+=pd0.positions[i*3+2]; }
                    pCoM0 = [sx/pCount0, sy/pCount0, sz/pCount0];
                }
                // Flux CoM at t=0
                const fv = b.getFluxVolume();
                let fCom0 = null, fMag0 = 0, fBBox = null;
                if (fv && fv.length === N*N*N) {
                    let sx=0,sy=0,sz=0,m=0,xmn=N,xmx=-1,ymn=N,ymx=-1,zmn=N,zmx=-1;
                    for (let z=0; z<N; z++) for (let y=0; y<N; y++) for (let x=0; x<N; x++) {
                        const v = Math.abs(fv[z*N*N + y*N + x]);
                        if (v > 1e-6) {
                            sx+=x*v; sy+=y*v; sz+=z*v; m+=v;
                            if (x<xmn) xmn=x; if (x>xmx) xmx=x;
                            if (y<ymn) ymn=y; if (y>ymx) ymx=y;
                            if (z<zmn) zmn=z; if (z>zmx) zmx=z;
                        }
                    }
                    if (m>0) { fCom0 = [sx/m, sy/m, sz/m]; fMag0 = m; fBBox = {x:[xmn,xmx],y:[ymn,ymx],z:[zmn,zmx]}; }
                }
                // Tick 200, capture again
                for (let t=0;t<200;t++) b.tick();
                const d200 = b.getDiagnostics();
                out.push({
                    n, pCount0, pCoM0,
                    fCoM0: fCom0, fMag0,
                    fBBox,
                    pCount200: d200.manifested|0,
                    fMag200: +d200.totalFlux.toFixed(2),
                });
            } catch (e) {
                out.push({ n, err: String(e).slice(0,150) });
            }
        }
        return { N, midF, results: out };
    }, NAMES);

    // Print structured table
    const TOL = 1.5; // voxels
    const off = (com, midF) => com ? Math.max(Math.abs(com[0]-midF), Math.abs(com[1]-midF), Math.abs(com[2]-midF)) : 0;
    const f3 = (a) => a ? `[${a[0].toFixed(1).padStart(5)}, ${a[1].toFixed(1).padStart(5)}, ${a[2].toFixed(1).padStart(5)}]` : '(none)             ';

    const groups = { perfect: [], asym: [], explosion: [], nofx: [] };
    for (const r of data.results) {
        if (r.err) continue;
        const pOff = off(r.pCoM0, data.midF);
        const fOff = off(r.fCoM0, data.midF);
        const pPop = r.pCount200 - r.pCount0;
        const exploded = pPop > 100;
        const offCenter = (r.pCoM0 && pOff > TOL) || (r.fCoM0 && fOff > TOL);
        if (exploded) groups.explosion.push(r);
        else if (offCenter) groups.asym.push(r);
        else if (!r.fCoM0 && r.pCount0 === 0) groups.nofx.push(r);
        else groups.perfect.push(r);
    }

    console.log(`\n=== PER-SCENARIO POSITION AUDIT (L=${data.N}, midF=${data.midF}, tol=${TOL} voxels) ===\n`);
    const printRow = (r) => {
        const pOff = off(r.pCoM0, data.midF);
        const fOff = off(r.fCoM0, data.midF);
        const pop = `${r.pCount0}→${r.pCount200}`;
        console.log(`  ${r.n.padEnd(36)} | p=${f3(r.pCoM0)} (off=${pOff.toFixed(1)}, n=${String(r.pCount0).padStart(4)}) | f=${f3(r.fCoM0)} (off=${fOff.toFixed(1)}, mag=${(r.fMag0||0).toFixed(0).padStart(6)}) | t200=${pop.padStart(10)}`);
    };

    console.log(`\n✓ PERFECT (centroid at midF, no explosion) — ${groups.perfect.length} scenarios:\n`);
    groups.perfect.forEach(printRow);
    console.log(`\n⚠ ASYMMETRIC BY DESIGN (off-center but intentional) — ${groups.asym.length} scenarios:\n`);
    groups.asym.forEach(printRow);
    console.log(`\n✗ POPULATION EXPLOSION (>100 manifested at t=200) — ${groups.explosion.length} scenarios:\n`);
    groups.explosion.forEach(printRow);
    console.log(`\n— EMPTY at t=0 (no flux, no particles): ${groups.nofx.length} scenarios:\n`);
    groups.nofx.forEach(printRow);
});
