// @ts-check
/**
 * Opt-in Scale 3 sustained dynamics and presentation release gate.
 *
 * Run from engine/web/tests with a hardware WebGL browser:
 *   FTD_HARDWARE_WEBGL=1 FTD_SCALE3_RELEASE_GATE=1 npx playwright test scale3-molecule-release-gate.spec.js --workers=1
 *
 * The canonical matrix exercises distinct molecular workloads. The final
 * 192-atom water population is an explicit stress fixture with every
 * applicable effective molecular term and overlay enabled.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.use({ trace: 'off' });

const SAMPLE_MS = 11_000;
const CANONICAL_MATRIX = Object.freeze([
    ['mol-h2-vibration', 2],
    ['mol-water-dimer-hbond', 6],
    ['mol-molecular-collision', 10],
    ['mol-water-thermal-cycle', 12],
    ['mol-caffeine', 24],
    ['mol-crystal', 27],
]);

async function rendererProvenance(page) {
    return page.evaluate(() => {
        const canvas = document.querySelector('#viewport canvas');
        const gl = canvas?.getContext?.('webgl2') || canvas?.getContext?.('webgl');
        const info = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
        return info
            ? String(gl.getParameter(info.UNMASKED_RENDERER_WEBGL) || '')
            : String(gl?.getParameter?.(gl.RENDERER) || '');
    });
}

async function selectScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const select = document.getElementById('mol-scenario-select');
        if (!(select instanceof HTMLSelectElement)) throw new Error('Scale 3 scenario selector unavailable');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await page.waitForTimeout(500);
}

async function enableCompleteMoleculeOverlaySurface(page) {
    await page.evaluate(() => {
        const labels = document.getElementById('ae-show-labels');
        if (labels instanceof HTMLInputElement && !labels.checked) labels.click();
        for (const id of [
            'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond',
            'ae-force-hbond', 'ae-force-angle', 'ae-force-dipole', 'ae-force-net',
            'toggle-ae-field', 'toggle-ae-velocities', 'toggle-ae-dipoles',
            'toggle-ae-hbonds',
        ]) {
            const button = document.getElementById(id);
            if (button instanceof HTMLButtonElement && !button.classList.contains('active')) button.click();
        }
        const style = document.getElementById('bond-style-select');
        if (style instanceof HTMLSelectElement) {
            style.value = 'cylinders';
            style.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await page.waitForTimeout(750);
}

async function sampleRunning(page) {
    return page.evaluate(async (durationMs) => {
        const probe = await import('/tests/scale0-ui-audit-probe.js');
        const bridge = window._ftdBridge;
        if (!bridge) throw new Error('Scale 3 bridge unavailable');
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
        const startTick = Number(bridge.aeGetDiagnostics()?.tick) || 0;
        const started = performance.now();
        probe.startScale0UiAuditProbe({ rootSelector: '#viewport' });
        await new Promise(resolve => setTimeout(resolve, durationMs));
        const report = await probe.stopScale0UiAuditProbe();
        const elapsedSeconds = (performance.now() - started) / 1_000;
        const diagnostics = bridge.aeGetDiagnostics();
        const molecule = bridge.aeGetMoleculeDiagnostics();
        if (play?.getAttribute('data-paused') !== 'true') play.click();
        return {
            ...report,
            diagnostics,
            molecule,
            tickHz: (Number(diagnostics.tick) - startTick) / Math.max(elapsedSeconds, 1e-9),
        };
    }, SAMPLE_MS);
}

function assertReleaseSample(report, label, expectedAtoms) {
    expect(report.frames.count, `${label}: sampling adequacy`).toBeGreaterThanOrEqual(600);
    expect(report.frames.effectiveFps, `${label}: effective FPS`).toBeGreaterThanOrEqual(59.5);
    expect(report.frames.p95Ms, `${label}: p95 frame interval`).toBeLessThanOrEqual(17);
    expect(report.frames.p99Ms, `${label}: p99 frame interval`).toBeLessThanOrEqual(20);
    expect(report.frames.intervalsOver33_4ms, `${label}: long frame intervals`).toBe(0);
    expect(report.longTasks, `${label}: browser long tasks`).toEqual([]);
    expect(report.errors, `${label}: in-page errors`).toEqual([]);
    expect(report.tickHz, `${label}: molecule tick rate`).toBeGreaterThanOrEqual(5);
    expect(report.diagnostics.atomCount, `${label}: live atom count`).toBe(expectedAtoms);
    expect(report.diagnostics.lastError, `${label}: finite-state status`).toBe('ok');
    expect([
        report.diagnostics.totalEnergy,
        report.molecule.translationalKE,
        report.molecule.rotationalKE,
        report.molecule.vibrationalKE,
        report.molecule.radiusOfGyration,
        report.molecule.bondRmsStrain,
    ].every(Number.isFinite), `${label}: finite diagnostics`).toBe(true);
}

test.describe('Scale 3 sustained molecular release gate', () => {
    test.skip(process.env.FTD_SCALE3_RELEASE_GATE !== '1',
        'Set FTD_SCALE3_RELEASE_GATE=1 to run the sustained Scale 3 probe.');

    test('canonical workloads and maximum effective surface sustain the frame budget', async ({ page }, testInfo) => {
        testInfo.setTimeout(240_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await switchMode(page, 'molecules');

        const webglRenderer = await rendererProvenance(page);
        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(webglRenderer, 'hardware release gate requires renderer provenance').not.toBe('');
            expect(webglRenderer, 'release gate does not certify software WebGL')
                .not.toMatch(/swiftshader|software|llvmpipe/i);
        }

        const canonical = {};
        for (const [id, atoms] of CANONICAL_MATRIX) {
            await selectScenario(page, id);
            const report = await sampleRunning(page);
            assertReleaseSample(report, id, atoms);
            canonical[id] = report;
        }

        const stressSeed = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            bridge.aeClear();
            bridge.aeSetDt(0.01);
            bridge.aeSetSoftening(0.1);
            bridge.aeSetIonic(true);
            bridge.aeSetVdw(true);
            bridge.aeSetBondsForce(true);
            bridge.aeSetBonding(false);
            bridge.aeSetDamping(false);
            bridge.aeSetSpeedLimit(true);
            bridge.aeSetHBonds(true);
            bridge.aeSetAngleStrain(true);
            bridge.aeSetDipoleDipole(true);
            bridge.aeSetThermostat(true);
            bridge.aeSetThermostatTemp(0.25);
            bridge.aeSetElectronegativity(true);
            const spacing = 4.2;
            let molecules = 0;
            for (let ix = 0; ix < 4; ix++) for (let iy = 0; iy < 4; iy++) for (let iz = 0; iz < 4; iz++) {
                const x = (ix - 1.5) * spacing;
                const y = (iy - 1.5) * spacing;
                const z = (iz - 1.5) * spacing;
                const oxygen = bridge.aeAddAtom(8, x, y, z);
                const h1 = bridge.aeAddAtom(1, x + 0.96, y, z);
                const h2 = bridge.aeAddAtom(1, x - 0.24, y + 0.93, z);
                bridge.aeCreateBond(oxygen, h1, 1, 0.96);
                bridge.aeCreateBond(oxygen, h2, 1, 0.96);
                molecules++;
            }
            bridge.aeSetMoleculeReference?.('release-water-grid');
            return { atoms: bridge.aeAtomCount(), molecules };
        });
        expect(stressSeed).toEqual({ atoms: 192, molecules: 64 });
        await enableCompleteMoleculeOverlaySurface(page);
        const stress = await sampleRunning(page);
        assertReleaseSample(stress, '192-atom complete molecular surface', 192);

        await testInfo.attach('scale3-molecule-release-performance.json', {
            body: Buffer.from(JSON.stringify({ webglRenderer, canonical, stress }, null, 2)),
            contentType: 'application/json',
        });
        console.log('scale3 molecular release performance', JSON.stringify({
            webglRenderer,
            canonical: Object.fromEntries(Object.entries(canonical).map(([id, report]) => [id, {
                frames: report.frames,
                tickHz: report.tickHz,
            }])),
            stress: { frames: stress.frames, tickHz: stress.tickHz },
        }));
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
