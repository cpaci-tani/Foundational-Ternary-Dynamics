// @ts-check
/**
 * Manual helium lattice-spectrum campaign recipe.
 *
 * This is intentionally not a default *.spec.js file. Run it only after the
 * preregistration has been locked and tagged:
 *
 *   npx playwright test tests/manual/helium-lattice-spectrum.manual.js
 */
import { test } from '@playwright/test';
import { gotoAndReady } from '../_helpers.js';

const SCENARIOS = [
    's0-seed-helium',
    'empty',
    's0-seed-hydrogen',
    's0-seed-h2-bond-formation',
    's0-seed-moore-cell',
    's0-seed-octahedron',
];
const AXES = [
    { name: 'x', v: [1, 0, 0] },
    { name: 'y', v: [0, 1, 0] },
    { name: 'z', v: [0, 0, 1] },
];

test.setTimeout(900_000);

test('manual helium lattice-spectrum campaign', async ({ page }) => {
    await gotoAndReady(page, { timeout: 30_000 });
    const result = await page.evaluate(async ({ scenarios, axes }) => {
        const {
            denseVectorGridFromSamples,
            energySpectrum,
        } = await import('/js/scales/scale0/analysis/lattice-spectrum.js');
        const {
            chargeDipoleFromParticles,
            dominantPeaks,
            latticeCenter,
            peakBinAgreement,
            secondDifferenceVectorSeries,
            spectrumDistance,
            spectrumFingerprint,
            timeSeriesPowerSpectrum,
        } = await import('/js/scales/scale0/analysis/helium-spectrum-protocol.js');

        const b = window._ftdBridge;
        const L = 64;
        const checkpoints = [0, 20, 80, 160];
        const snapshots = [];
        const spatialRepeats = 2;

        function readSnapshot(scenario, tick) {
            const samples = b.getFluxVectorSampled(1);
            const grid = denseVectorGridFromSamples(samples, L, 1);
            const spec = energySpectrum(grid, grid.srcN, 64, L);
            const particles = b.getScale0ParticleList?.() ?? [];
            const dipole = chargeDipoleFromParticles(particles, latticeCenter(L));
            const fp = spectrumFingerprint(spec, {
                parsevalRatio: spec.sumReal > 0 ? spec.totalE / spec.sumReal : 0,
                chargeDipoleMagnitude: dipole.magnitude,
            });
            return {
                scenario,
                tick,
                diagnostics: b.getDiagnostics(),
                audit: b.getEnergyAudit(),
                parsevalRatio: spec.sumReal > 0 ? spec.totalE / spec.sumReal : 0,
                fingerprint: fp,
                spectrum: { k: spec.k, E: spec.E },
            };
        }

        for (let repeat = 0; repeat < spatialRepeats; repeat++) {
            for (const scenario of scenarios) {
                b.reset(L);
                b.setupScenario(scenario);
                let currentTick = 0;
                for (const target of checkpoints) {
                    while (currentTick < target) {
                        b.tick();
                        currentTick++;
                    }
                    const snap = readSnapshot(scenario, target);
                    snap.repeat = repeat;
                    snapshots.push(snap);
                }
            }
        }

        const byScenarioTick80 = new Map(
            snapshots.filter((s) => s.tick === 80 && s.repeat === 0).map((s) => [s.scenario, s])
        );
        const helium = byScenarioTick80.get('s0-seed-helium');
        const distances = [];
        if (helium) {
            for (const scenario of scenarios) {
                if (scenario === 's0-seed-helium') continue;
                const other = byScenarioTick80.get(scenario);
                if (!other) continue;
                distances.push({
                    scenario,
                    ...spectrumDistance(helium.spectrum, other.spectrum),
                });
            }
        }

        const duplicateDistances = [];
        for (const scenario of scenarios) {
            const a = snapshots.find((s) => s.scenario === scenario && s.tick === 80 && s.repeat === 0);
            const c = snapshots.find((s) => s.scenario === scenario && s.tick === 80 && s.repeat === 1);
            if (a && c) duplicateDistances.push({ scenario, ...spectrumDistance(a.spectrum, c.spectrum) });
        }

        const constants = b.getConstants?.() ?? {};
        const epsilon = 0.05 * (constants.K_B ?? 0.511);
        const lineCandidates = [];
        for (const axis of axes) {
            const axisRuns = [];
            for (let repeat = 0; repeat < 2; repeat++) {
                b.reset(L);
                b.setupScenario('s0-seed-helium');
                for (let t = 0; t < 80; t++) b.tick();

                const c = latticeCenter(L);
                const dx = axis.v[0], dy = axis.v[1], dz = axis.v[2];
                b.injectFlux(c.x + 2 * dx, c.y + 2 * dy, c.z + 2 * dz,
                    epsilon * dx, epsilon * dy, epsilon * dz);
                b.injectFlux(c.x - 2 * dx, c.y - 2 * dy, c.z - 2 * dz,
                    -epsilon * dx, -epsilon * dy, -epsilon * dz);

                const dipoles = [];
                for (let t = 0; t < 512; t++) {
                    const particles = b.getScale0ParticleList?.() ?? [];
                    dipoles.push(chargeDipoleFromParticles(particles, latticeCenter(L)));
                    b.tick();
                }
                const accelMag = secondDifferenceVectorSeries(dipoles).map((v) => v.magnitude);
                const ts = timeSeriesPowerSpectrum(accelMag, { dt: 1 });
                const peaks = dominantPeaks(ts.frequency, ts.power, { limit: 5, minBin: 2 });
                axisRuns.push({ repeat, peaks, totalPower: ts.totalPower });
            }
            lineCandidates.push({
                axis: axis.name,
                runs: axisRuns,
                agreement: peakBinAgreement(axisRuns[0].peaks, axisRuns[1].peaks, {
                    toleranceBins: 1,
                    limit: 5,
                }),
            });
        }

        return { L, checkpoints, snapshots, distances, duplicateDistances, lineCandidates };
    }, { scenarios: SCENARIOS, axes: AXES });

    console.log(JSON.stringify(result, null, 2));
});
