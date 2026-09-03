// @ts-check
/**
 * Opt-in Scale 2 overlay/load release gate.
 *
 * Run on hardware WebGL from engine/web/tests:
 *   FTD_HARDWARE_WEBGL=1 FTD_SCALE2_LOAD_GATE=1 npx playwright test scale2-overlay-load-release-gate.spec.js --workers=1
 *
 * This is deliberately separate from the fast functional suite. It holds the
 * complete AtomEngine visualization surface under two representative loads:
 * a 118-record empirical atlas and a 192-record live water population with
 * every applicable effective interaction enabled.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.use({ trace: 'off' });

const SAMPLE_MS = 11_000;

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

async function enableCompleteOverlaySurface(page) {
    await page.evaluate(() => {
        const checkboxIds = [
            'ae-show-clouds', 'ae-show-shells', 'ae-show-labels',
            'ae-show-shell-bounds', 'ae-show-lobes',
        ];
        const buttonIds = [
            'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond',
            'ae-force-hbond', 'ae-force-angle', 'ae-force-dipole', 'ae-force-net',
            'toggle-ae-field', 'toggle-ae-velocities', 'toggle-ae-dipoles',
            'toggle-ae-hbonds', 'toggle-ae-nuclear-events', 'toggle-ae-radiation',
            'toggle-ae-heat', 'toggle-ae-nuclear-boundary',
        ];
        for (const id of checkboxIds) {
            const input = document.getElementById(id);
            if (input instanceof HTMLInputElement && !input.checked) input.click();
        }
        for (const id of buttonIds) {
            const button = document.getElementById(id);
            if (button instanceof HTMLButtonElement && !button.classList.contains('active')) button.click();
        }
        const bondStyle = document.getElementById('bond-style-select');
        if (bondStyle instanceof HTMLSelectElement) {
            bondStyle.value = 'cylinders';
            bondStyle.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await page.waitForTimeout(1_000);
}

async function sample(page, running) {
    return page.evaluate(async ({ durationMs, shouldRun }) => {
        const probe = await import('/tests/scale0-ui-audit-probe.js');
        const bridge = window._ftdBridge;
        if (!bridge) throw new Error('Scale 2 bridge unavailable');
        const play = document.getElementById('btn-play');
        if (shouldRun && play?.getAttribute('data-paused') === 'true') play.click();
        if (!shouldRun && play?.getAttribute('data-paused') !== 'true') play.click();
        const startTick = Number(bridge.aeGetDiagnostics()?.tick) || 0;
        const started = performance.now();
        probe.startScale0UiAuditProbe({ rootSelector: '#viewport' });
        await new Promise(resolve => setTimeout(resolve, durationMs));
        const report = await probe.stopScale0UiAuditProbe();
        const elapsedSeconds = (performance.now() - started) / 1_000;
        const endTick = Number(bridge.aeGetDiagnostics()?.tick) || 0;
        if (shouldRun && play?.getAttribute('data-paused') !== 'true') play.click();
        const renderer = window.__ftdCtx?.viewport?._molRenderer;
        return {
            ...report,
            atomCount: bridge.aeAtomCount(),
            startTick,
            endTick,
            tickHz: (endTick - startTick) / Math.max(elapsedSeconds, 1e-9),
            drawCounts: {
                bonds: renderer?.bondLines?.geometry?.drawRange?.count || 0,
                hBonds: renderer?._hbondLines?.geometry?.drawRange?.count || 0,
                forceNet: renderer?._aeForceNet?.geometry?.drawRange?.count || 0,
            },
        };
    }, { durationMs: SAMPLE_MS, shouldRun: running });
}

function assertFrameGate(report, label) {
    expect(report.frames.count, `${label}: sampling adequacy`).toBeGreaterThanOrEqual(600);
    expect(report.frames.effectiveFps, `${label}: effective FPS`).toBeGreaterThanOrEqual(59.5);
    expect(report.frames.p95Ms, `${label}: p95 frame interval`).toBeLessThanOrEqual(17);
    expect(report.frames.p99Ms, `${label}: p99 frame interval`).toBeLessThanOrEqual(20);
    expect(report.frames.intervalsOver33_4ms, `${label}: long frame intervals`).toBe(0);
    expect(report.longTasks, `${label}: browser long tasks`).toEqual([]);
    expect(report.errors, `${label}: in-page errors`).toEqual([]);
}

test.describe('Scale 2 sustained overlay/load release gate', () => {
    test.skip(process.env.FTD_SCALE2_LOAD_GATE !== '1',
        'Set FTD_SCALE2_LOAD_GATE=1 to run the sustained Scale 2 probe.');

    test('complete overlay surface stays smooth at representative maximum loads', async ({ page }, testInfo) => {
        testInfo.setTimeout(240_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await switchMode(page, 'atoms');

        const webglRenderer = await rendererProvenance(page);
        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(webglRenderer, 'hardware release gate requires renderer provenance').not.toBe('');
            expect(webglRenderer, 'release gate does not certify software WebGL')
                .not.toMatch(/swiftshader|software|llvmpipe/i);
        }

        await page.selectOption('#ae-scenario-select', 'ae-periodic');
        await enableCompleteOverlaySurface(page);
        const atlas = await sample(page, false);
        expect(atlas.atomCount).toBe(118);
        assertFrameGate(atlas, '118-record atlas');

        const waterSetup = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            bridge.aeClear();
            bridge.aeSetDt(0.01);
            bridge.aeSetSoftening(0.1);
            bridge.aeSetDamping(true);
            bridge.aeSetBonding(false);
            bridge.aeSetIonic(false);
            bridge.aeSetVdw(true);
            bridge.aeSetBondsForce(true);
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
                bridge.aeCreateBond(oxygen, h1, 1);
                bridge.aeCreateBond(oxygen, h2, 1);
                molecules++;
            }
            bridge.aePreBond();
            return { atoms: bridge.aeAtomCount(), molecules };
        });
        expect(waterSetup).toEqual({ atoms: 192, molecules: 64 });
        await page.waitForTimeout(1_000);

        const water = await sample(page, true);
        expect(water.atomCount).toBe(192);
        expect(water.endTick).toBeGreaterThan(water.startTick);
        expect(water.tickHz, '192-record water population tick rate').toBeGreaterThanOrEqual(5);
        expect(water.drawCounts.forceNet, 'net-force arrows rendered').toBeGreaterThan(0);
        expect(water.drawCounts.hBonds, 'H-bond overlay rendered').toBeGreaterThan(0);
        assertFrameGate(water, '192-record live water population');

        await testInfo.attach('scale2-overlay-load-performance.json', {
            body: Buffer.from(JSON.stringify({ webglRenderer, atlas, water }, null, 2)),
            contentType: 'application/json',
        });
        console.log('scale2 overlay load performance', JSON.stringify({
            webglRenderer,
            atlas: {
                atomCount: atlas.atomCount,
                frames: atlas.frames,
                longTasks: atlas.longTasks.length,
            },
            water: {
                atomCount: water.atomCount,
                frames: water.frames,
                tickHz: water.tickHz,
                drawCounts: water.drawCounts,
                longTasks: water.longTasks.length,
            },
        }));
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
