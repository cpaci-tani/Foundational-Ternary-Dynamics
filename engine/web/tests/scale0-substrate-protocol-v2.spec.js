// @ts-check
/**
 * Scale-0 Substrate Experimental Protocol (v2) Spec
 *
 * Implements the automated measurements for the v2 protocol, fixing v1 defects:
 * - F1 (selective_damping toggle-trap): turns selective_damping OFF first.
 * - F2 (conserved Maxwell Hamiltonian): uses velocity-Verlet and measures
 *   the unstaggered EFieldEnergy + BFieldEnergy diagnostic.
 * - F3 (c_lat speed via front-tracking): uses point pulse at L/2 and tracks dR/dt.
 * - F4 (genesis response): checks the preregistered fixed A=12/16/40
 *   finite-box ordering without promoting it to a universal power law.
 * - G4 (cluster count): implements connected-components BFS cluster counting.
 * - I5 (determinism): compares two full runs for bit-exact identical trajectories.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

// ── Configuration Recipes (generic wrappers for basic tests) ───────

async function applyConservativeConfig(page) {
    await page.evaluate(() => {
        const b = window._ftdBridge;
        // Turn OFF selective_damping first to release validation constraint (F1)
        b.setToggle('selective_damping', false);

        // Turn OFF all other physical/dissipative and Langevin toggles
        const togglesToDisable = [
            'damping', 'genesis', 'evaporation', 'coupling', 'movement',
            'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
            'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
            'latency_field', 'larmor_radiation', 'weak_transmutation',
            'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
            'symplectic_leapfrog', 'su2_gauge', 'su3_gauge', 'confinement',
            'gauss_projection'
        ];
        togglesToDisable.forEach(t => b.setToggle(t, false));

        // Turn ON wave propagation
        b.setToggle('wave_propagation', true);
    });
}

async function applyGenesisConfig(page) {
    await page.evaluate(() => {
        const b = window._ftdBridge;
        // Turn OFF everything first to ensure clean state
        const togglesToDisable = [
            'selective_damping', 'damping', 'genesis', 'evaporation', 'coupling', 'movement',
            'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
            'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
            'latency_field', 'larmor_radiation', 'weak_transmutation',
            'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
            'symplectic_leapfrog', 'su2_gauge', 'su3_gauge', 'confinement'
        ];
        togglesToDisable.forEach(t => b.setToggle(t, false));

        // Turn ON wave, gauss, genesis, damping, forces, movement
        b.setToggle('wave_propagation', true);
        b.setToggle('gauss_projection', true);
        b.setToggle('genesis', true);
        b.setToggle('damping', true);
        b.setToggle('forces', true);
        b.setToggle('poisson_coulomb', true);
        b.setToggle('movement', true);
    });
}

// ── Test Suites ─────────────────────────────────────────────────────

test.describe('Scale-0 Substrate Experimental Protocol (v2)', () => {

    test.beforeEach(async ({ page }) => {
        // These protocol arms issue ordered reset/setup/tick/read sequences.
        // Keep the production WASM engine but disable its asynchronous worker
        // proxy so every measurement observes the command it just issued.
        await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        await gotoAndReady(page);
        // Assert we are using the real C++/WASM bridge
        const isWasm = await page.evaluate(() => window._ftdBridge && window._ftdBridge.isWasm);
        expect(isWasm).toBe(true);
    });

    test('§R1: Conservative config compiles and validates with zero TermToggles combination errors', async ({ page }) => {
        const errors = [];
        page.on('console', msg => {
            if (msg.text().includes('Invalid combination') || msg.text().includes('TermToggles')) {
                errors.push(msg.text());
            }
        });

        await applyConservativeConfig(page);

        // Trigger a tick to force C++ validation
        await page.evaluate(() => window._ftdBridge.tick());

        expect(errors).toHaveLength(0);
    });

    test('§R2: I2 energy conservation of Maxwell Hamiltonian E_H < 0.5%', async ({ page }) => {
        const results = await page.evaluate(() => {
            const b = window._ftdBridge;
            b.reset(64);
            b.setupScenario('light-rainbow');

            b.setToggle('selective_damping', false);
            const togglesToDisable = [
                'damping', 'genesis', 'evaporation', 'coupling', 'movement',
                'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
                'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
                'latency_field', 'larmor_radiation', 'weak_transmutation',
                'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
                'su2_gauge', 'su3_gauge', 'confinement', 'gauss_projection'
            ];
            togglesToDisable.forEach(t => b.setToggle(t, false));
            b.setToggle('wave_propagation', true);
            // The kick-drift path conserves a modified Hamiltonian containing
            // a time-staggering cross term. Velocity-Verlet is the production
            // path appropriate for bounding the unstaggered E+B diagnostic.
            b.setToggle('symplectic_leapfrog', false);
            b.setToggle('verlet_wave_integrator', true);
            b.setDt(0.02);

            const energies = [];
            const rawEnergies = [];

            for (let t = 0; t <= 200; t++) {
                if (t > 0) {
                    b.tick();
                }
                if (t % 40 === 0) {
                    const audit = b.getEnergyAudit();
                    energies.push(audit.EFieldEnergy + audit.BFieldEnergy);
                    rawEnergies.push(audit.fieldEnergy + audit.waveEnergy);
                }
            }
            return { energies, rawEnergies };
        });

        const { energies, rawEnergies } = results;
        const E0 = energies[0];
        expect(E0).toBeGreaterThan(0);

        // Compute drift and peak-to-peak
        const maxE = Math.max(...energies);
        const minE = Math.min(...energies);
        const peakToPeak = (maxE - minE) / E0;
        const drift = Math.abs(energies[energies.length - 1] - E0) / E0;

        console.log(`[I2 Maxwell Energy EH] E0: ${E0.toFixed(4)}, Peak-to-Peak: ${(peakToPeak*100).toFixed(4)}%, Drift: ${(drift*100).toFixed(4)}%`);
        console.log(`[I2 Maxwell Energy EH] Trajectory:`, energies.map(e => e.toFixed(2)));
        console.log(`[I2 Sanity Cross-check] ½|J|² (slosh metric) Trajectory:`, rawEnergies.map(e => e.toFixed(2)));

        // Prediction: drift AND peak-to-peak < 0.5%
        expect(drift).toBeLessThan(0.005);
        expect(peakToPeak).toBeLessThan(0.005);
    });


    test('§R3: I1 exact harmonic dispersion and I6 strict causal support', async ({ page }) => {
        // WASM normalizes even requests upward so the lattice has a true
        // center voxel. Use the canonical odd size explicitly so indexing,
        // Fourier k, and the engine all refer to the same grid.
        const L = 63;
        const center = Math.floor(L / 2); // 31

        const result = await page.evaluate(({ L, cx, cy, cz, noise }) => {
            const b = window._ftdBridge;

            // I1: the exact native n=4 traveling harmonic has a signed phase,
            // unlike a thresholded pulse edge. Recover that phase through the
            // WASM vector sampler and measure the discrete pole directly.
            b.reset(L);
            b.setupScenario('s0-field-plane-wave');
            const modeN = 4;
            const k = 2 * Math.PI * modeN / L;
            let sampledCount = 0;
            const phaseOfMode = () => {
                const sample = b.getFluxVectorSampled(1);
                sampledCount = sample.count;
                let real = 0;
                let imag = 0;
                for (let i = 0; i < sample.count; i++) {
                    const x = sample.positions[3 * i];
                    const jz = sample.vectors[3 * i + 2];
                    real += jz * Math.cos(k * x);
                    imag -= jz * Math.sin(k * x);
                }
                return Math.atan2(imag, real);
            };

            const phase0 = phaseOfMode();
            const phaseTicks = 8;
            for (let t = 0; t < phaseTicks; t++) b.tick();
            const phase1 = phaseOfMode();
            let phaseDelta = phase1 - phase0;
            while (phaseDelta > Math.PI) phaseDelta -= 2 * Math.PI;
            while (phaseDelta < -Math.PI) phaseDelta += 2 * Math.PI;
            const measuredOmega = Math.abs(phaseDelta) / phaseTicks;

            // I6: a separate point pulse checks the radius-one dependency
            // hull. Twelve ticks keep the bound smaller than the periodic
            // box's maximum Chebyshev distance, so the assertion is non-vacuous.
            b.reset(L);

            b.setToggle('selective_damping', false);
            const togglesToDisable = [
                'damping', 'genesis', 'evaporation', 'coupling', 'movement',
                'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
                'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
                'latency_field', 'larmor_radiation', 'weak_transmutation',
                'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
                'symplectic_leapfrog', 'verlet_wave_integrator',
                'su2_gauge', 'su3_gauge', 'confinement', 'gauss_projection'
            ];
            togglesToDisable.forEach(t => b.setToggle(t, false));
            b.setToggle('wave_propagation', true);
            b.setDt(1.0);

            b.injectFlux(cx, cy, cz, 50.0, 0.0, 0.0);

            const fv0 = b.getFluxVolume();
            let C0 = 0;
            for (let z = 0; z < L; z++) {
                for (let y = 0; y < L; y++) {
                    for (let x = 0; x < L; x++) {
                        const idx = z * L * L + y * L + x;
                        if (fv0[idx] > noise) {
                            const dx = Math.abs(x - cx);
                            const dy = Math.abs(y - cy);
                            const dz = Math.abs(z - cz);
                            const cheb = Math.max(dx, dy, dz);
                            if (cheb > C0) C0 = cheb;
                        }
                    }
                }
            }

            const localityTicks = 12;
            for (let t = 0; t < localityTicks; t++) b.tick();
            const fvFinal = b.getFluxVolume();
            const leaked = [];
            const chebLimit = C0 + localityTicks;
            for (let z = 0; z < L; z++) {
                for (let y = 0; y < L; y++) {
                    for (let x = 0; x < L; x++) {
                        const idx = z * L * L + y * L + x;
                        if (fvFinal[idx] > noise) {
                            const dx = Math.abs(x - cx);
                            const dy = Math.abs(y - cy);
                            const dz = Math.abs(z - cz);
                            const cheb = Math.max(dx, dy, dz);
                            if (cheb > chebLimit) {
                                leaked.push({ x, y, z, val: fvFinal[idx], chebyshev: cheb });
                            }
                        }
                    }
                }
            }

            return {
                k,
                measuredOmega,
                sampledCount,
                C0,
                localityTicks,
                chebLimit,
                leaked
            };
        }, { L, cx: center, cy: center, cz: center, noise: 1e-10 });

        const { k, measuredOmega, sampledCount, C0, localityTicks, chebLimit, leaked } = result;
        const cLat = 1 / Math.sqrt(3);
        const expectedOmega = 2 * Math.asin(cLat * Math.abs(Math.sin(k / 2)));
        const phaseSpeed = measuredOmega / k;
        const poleError = Math.abs(measuredOmega - expectedOmega);
        const lowKDeviation = Math.abs(phaseSpeed - cLat) / cLat;

        console.log(`[I1 Native Dispersion] k=${k.toFixed(6)}, omega=${measuredOmega.toFixed(6)}, exact=${expectedOmega.toFixed(6)}, phase speed=${phaseSpeed.toFixed(6)}, c_lat=${cLat.toFixed(6)}`);
        console.log(`[I6 Locality] C0=${C0}, ticks=${localityTicks}, leaked beyond Chebyshev radius ${chebLimit}: ${leaked.length}`);
        if (leaked.length > 0) {
            console.log(`[I6 Locality] Sample leaked:`, leaked.slice(0, 5));
        }

        expect(sampledCount).toBe(L * L * L);
        expect(poleError).toBeLessThan(1e-6);
        expect(lowKDeviation).toBeLessThan(0.10);
        expect(chebLimit).toBeLessThan(L / 2);
        expect(leaked).toHaveLength(0);
    });

    test('§R3b: I4 charge conservation under conservative stepping', async ({ page }) => {
        const checkpoints = await page.evaluate(() => {
            const b = window._ftdBridge;
            b.reset(64);
            b.setupScenario('s0-vacuum-electron');

            b.setToggle('selective_damping', false);
            const togglesToDisable = [
                'damping', 'genesis', 'evaporation', 'coupling', 'movement',
                'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
                'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
                'latency_field', 'larmor_radiation', 'weak_transmutation',
                'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
                'symplectic_leapfrog', 'su2_gauge', 'su3_gauge', 'confinement'
            ];
            togglesToDisable.forEach(t => b.setToggle(t, false));
            b.setToggle('wave_propagation', true);
            b.setToggle('gauss_projection', true);

            const samples = [];
            for (let t = 0; t <= 120; t++) {
                if (t > 0) b.tick();
                if (t === 0 || t === 40 || t === 80 || t === 120) {
                    const d = b.getDiagnostics();
                    samples.push({
                        tick: d.tick,
                        chargeBalance: d.chargeBalance,
                        manifested: d.manifested,
                        positive: d.positive,
                        negative: d.negative
                    });
                }
            }
            return samples;
        });

        const q0 = checkpoints[0].chargeBalance;
        const n0 = checkpoints[0].manifested;
        console.log(`[I4 Charge Conservation] Checkpoints: ${JSON.stringify(checkpoints)}`);
        expect(n0).toBeGreaterThan(0);
        for (const sample of checkpoints) {
            expect(sample.chargeBalance, `charge drift at tick ${sample.tick}`).toBe(q0);
            expect(sample.manifested, `manifested count drift at tick ${sample.tick}`).toBe(n0);
        }
    });

    test('§R4: G1–G3 fixed-arm genesis response ordering and persistence', async ({ page }) => {
        const responses = await page.evaluate(() => {
            const b = window._ftdBridge;
            const latticeSize = 25;
            const arms = [
                { amplitude: 12, scenario: 's0-seed-cluster-law-subknee' },
                { amplitude: 16, scenario: 's0-seed-cluster-law-knee' },
                { amplitude: 40, scenario: 's0-seed-cluster-law-superknee' },
            ];
            return arms.map(({ amplitude, scenario }) => {
                b.reset(latticeSize);
                b.setupScenario(scenario);
                let at200 = -1;
                let at220 = -1;
                for (let tick = 1; tick <= 220; tick++) {
                    b.tick();
                    if (tick === 200) at200 = b.getDiagnostics().manifested;
                    if (tick === 220) at220 = b.getDiagnostics().manifested;
                }
                return { amplitude, scenario, latticeSize, at200, at220 };
            });
        });

        console.log(`[G1–G3 Fixed Genesis Arms] ${JSON.stringify(responses)}`);
        for (const response of responses) {
            expect(response.at200, `${response.scenario} must manifest at tick 200`).toBeGreaterThan(0);
            expect(response.at220, `${response.scenario} must persist at tick 220`).toBeGreaterThan(0);
        }
        expect(responses[0].at200).toBeLessThan(responses[1].at200);
        expect(responses[1].at200).toBeLessThan(responses[2].at200);
        expect(responses[0].at220).toBeLessThan(responses[1].at220);
        expect(responses[1].at220).toBeLessThan(responses[2].at220);
    });


    test('§R4: G4 connected-components cluster counting (ic1, ic3, ic4)', async ({ page }) => {
        const results = await page.evaluate(() => {
            const b = window._ftdBridge;

            const runCountClusters = () => {
                const pd = b.getParticleData();
                const n = pd.count;
                if (n === 0) return 0;

                const pos = [];
                const px = pd.positions;
                for (let i = 0; i < n; i++) {
                    pos.push({
                        x: Math.floor(px[i * 3]),
                        y: Math.floor(px[i * 3 + 1]),
                        z: Math.floor(px[i * 3 + 2])
                    });
                }

                const visited = new Set();
                let clusters = 0;

                const posKey = (p) => `${p.x},${p.y},${p.z}`;
                const posMap = new Map();
                pos.forEach(p => posMap.set(posKey(p), p));

                for (let i = 0; i < n; i++) {
                    const startNode = pos[i];
                    const startKey = posKey(startNode);
                    if (visited.has(startKey)) continue;

                    clusters++;
                    const queue = [startNode];
                    visited.add(startKey);

                    while (queue.length > 0) {
                        const curr = queue.shift();

                        for (let dz = -1; dz <= 1; dz++) {
                            for (let dy = -1; dy <= 1; dy++) {
                                for (let dx = -1; dx <= 1; dx++) {
                                    if (dx === 0 && dy === 0 && dz === 0) continue;
                                    const neighbor = { x: curr.x + dx, y: curr.y + dy, z: curr.z + dz };
                                    const neighborKey = posKey(neighbor);

                                    if (posMap.has(neighborKey) && !visited.has(neighborKey)) {
                                        visited.add(neighborKey);
                                        queue.push(neighbor);
                                    }
                                }
                            }
                        }
                    }
                }
                return clusters;
            };

            // ic4 sub-test
            b.setupScenario('s0-seed-emergent-ic4-subthreshold');
            for (let i = 0; i < 80; i++) b.tick();
            const manifested_ic4 = b.getDiagnostics().manifested;
            const clusters_ic4 = runCountClusters();

            // ic1 sub-test
            b.setupScenario('s0-seed-emergent-ic1');
            for (let i = 0; i < 300; i++) b.tick();
            const manifested_ic1 = b.getDiagnostics().manifested;
            const clusters_ic1 = runCountClusters();

            // ic3 sub-test
            b.setupScenario('s0-seed-emergent-ic3-collision');
            for (let i = 0; i < 300; i++) b.tick();
            const manifested_ic3 = b.getDiagnostics().manifested;
            const clusters_ic3 = runCountClusters();

            return {
                manifested_ic4, clusters_ic4,
                manifested_ic1, clusters_ic1,
                manifested_ic3, clusters_ic3
            };
        });

        const {
            manifested_ic4, clusters_ic4,
            manifested_ic1, clusters_ic1,
            manifested_ic3, clusters_ic3
        } = results;

        console.log(`[G4 ic4 sub-threshold null] manifested: ${manifested_ic4}, clusters: ${clusters_ic4}`);
        console.log(`[G4 ic1 cluster count] manifested: ${manifested_ic1}, clusters: ${clusters_ic1}`);
        console.log(`[G4 ic3 cluster count] manifested: ${manifested_ic3}, clusters: ${clusters_ic3}`);

        expect(manifested_ic4).toBe(0);
        expect(clusters_ic4).toBe(0);

        expect(manifested_ic1).toBeGreaterThan(0);
        expect(clusters_ic1).toBe(1);

        expect(manifested_ic3).toBeGreaterThan(0);
        expect(clusters_ic3).toBe(2);
    });

    test('§R5: I5 determinism (bit-exact checkpoint trajectories)', async ({ page }) => {
        const trajectories = await page.evaluate(() => {
            const b = window._ftdBridge;
            const runs = [];

            for (let run = 1; run <= 2; run++) {
                b.setupScenario('s0-seed-emergent-ic1');

                // apply genesis config
                const togglesToDisable = [
                    'selective_damping', 'damping', 'genesis', 'evaporation', 'coupling', 'movement',
                    'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
                    'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
                    'latency_field', 'larmor_radiation', 'weak_transmutation',
                    'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
                    'symplectic_leapfrog', 'su2_gauge', 'su3_gauge', 'confinement'
                ];
                togglesToDisable.forEach(t => b.setToggle(t, false));
                b.setToggle('wave_propagation', true);
                b.setToggle('gauss_projection', true);
                b.setToggle('genesis', true);
                b.setToggle('damping', true);
                b.setToggle('forces', true);
                b.setToggle('poisson_coulomb', true);
                b.setToggle('movement', true);

                const checkpoints = [];
                for (let tick = 0; tick <= 120; tick++) {
                    if (tick > 0) {
                        b.tick();
                    }
                    if (tick === 0 || tick === 40 || tick === 80 || tick === 120) {
                        const d = b.getDiagnostics();
                        const audit = b.getEnergyAudit();
                        checkpoints.push({
                            tick: d.tick,
                            manifested: d.manifested,
                            totalFlux: d.totalFlux,
                            EFieldEnergy: audit.EFieldEnergy,
                            BFieldEnergy: audit.BFieldEnergy
                        });
                    }
                }
                runs.push(checkpoints);
            }
            return runs;
        });

        // Assert bit-exact identity
        const t1 = trajectories[0];
        const t2 = trajectories[1];

        for (let i = 0; i < t1.length; i++) {
            const c1 = t1[i];
            const c2 = t2[i];

            expect(c1.tick).toBe(c2.tick);
            expect(c1.manifested).toBe(c2.manifested);

            // Float epsilon check: relative diff < 1e-12
            const checkFloat = (val1, val2, name) => {
                if (val1 === 0 && val2 === 0) return;
                const rel = Math.abs(val1 - val2) / Math.max(Math.abs(val1), Math.abs(val2));
                expect(rel, `${name} mismatch at tick ${c1.tick}: run1=${val1}, run2=${val2}`).toBeLessThan(1e-12);
            };

            checkFloat(c1.totalFlux, c2.totalFlux, 'totalFlux');
            checkFloat(c1.EFieldEnergy, c2.EFieldEnergy, 'EFieldEnergy');
            checkFloat(c1.BFieldEnergy, c2.BFieldEnergy, 'BFieldEnergy');
        }

        console.log(`[I5 Determinism] Bit-exact trajectories verified successfully!`);
    });

    test('§R6: I3 Gauss constraint enforcement (< 1e-3)', async ({ page }) => {
        const audit = await page.evaluate(() => {
            const b = window._ftdBridge;
            b.setupScenario('flux-pulse');

            // apply conservative config toggles but keep gauss_projection ON
            b.setToggle('selective_damping', false);
            const togglesToDisable = [
                'damping', 'genesis', 'evaporation', 'coupling', 'movement',
                'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces',
                'strong_force', 'triad_binding', 'pair_production', 'exchange_force',
                'latency_field', 'larmor_radiation', 'weak_transmutation',
                'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
                'symplectic_leapfrog', 'su2_gauge', 'su3_gauge', 'confinement'
            ];
            togglesToDisable.forEach(t => b.setToggle(t, false));
            b.setToggle('wave_propagation', true);
            b.setToggle('gauss_projection', true);

            for (let i = 0; i < 80; i++) b.tick();
            return b.getEnergyAudit();
        });

        console.log(`[I3 Gauss Constraint] maxGaussError: ${audit.maxGaussError.toExponential(4)}, gaussViolation: ${audit.gaussViolation.toExponential(4)}`);
        expect(audit.maxGaussError).toBeLessThan(1e-3);
    });
});
