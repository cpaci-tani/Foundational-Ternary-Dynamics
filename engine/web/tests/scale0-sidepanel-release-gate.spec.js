// @ts-check
/**
 * Formal hardware release probe for one Scale-0 sidepanel at a time.
 *
 * Invoke with both environment variables set, for example:
 *   FTD_HARDWARE_WEBGL=1 FTD_SCALE0_SIDEPANEL_GATE=charts npx playwright test ...
 *
 * Scientific/interaction behavior remains covered by the focused panel specs;
 * this file gives every gate the same sustained foreground frame, lifecycle,
 * and renderer-provenance record at the largest supported browser lattice.
 */
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

test.use({ trace: 'off' });

const GATES = Object.freeze({
    charts: {
        panelId: 'charts', root: '#panel-charts', api: 'ctx:chartsPanel',
        methods: ['update'], subscriberPrefixes: [],
    },
    lagrangian: {
        panelId: 'lagrangian', root: '#panel-lagrangian', api: 'ctx:lagrangianPanel',
        methods: ['update'], subscriberPrefixes: [],
    },
    inspector: {
        panelId: 'inspector', root: '#panel-inspector', api: 'ctx:inspector',
        methods: ['update'], subscriberPrefixes: [],
    },
    scene: {
        panelId: 'scene', root: '#panel-scene', api: '',
        methods: [], subscriberPrefixes: [],
    },
    'flux-slice': {
        panelId: 'flux-slice', root: '#panel-flux-slice', api: 'window:__ftdFluxSlicePanel',
        methods: ['update', '_buildFrameSampleCache', '_paintSlice'],
        subscriberPrefixes: ['flux-slice-panel'],
    },
    'wave-lab': {
        panelId: 'wave-lab', root: '#panel-wave-lab', api: 'window:__ftdWaveLabPanel',
        methods: ['update'], subscriberPrefixes: ['wave-lab-panel'],
        scenario: 's0-field-rf-lattice-wave',
    },
    'p1-observables': {
        panelId: 'p1-observables', root: '#panel-p1-observables', api: 'window:__ftdP1Panel',
        methods: ['update'], subscriberPrefixes: ['p1-observables-panel'],
    },
    spectrum: {
        panelId: 'spectrum', root: '#panel-spectrum', api: 'window:__ftdSpectrumPanel',
        methods: ['update'], subscriberPrefixes: ['spectrum-panel'],
    },
    dispersion: {
        panelId: 'dispersion', root: '#panel-dispersion', api: 'window:__ftdDispersionPanel',
        methods: ['update'], subscriberPrefixes: ['dispersion-panel'],
    },
    knots: {
        panelId: 'knots', root: '#panel-knots', api: 'window:__ftdKnotsPanel',
        methods: ['update'], subscriberPrefixes: ['knots-panel'],
    },
    time: {
        panelId: 'time', root: '#panel-time', api: 'window:__ftdTimePanel',
        methods: ['update'], subscriberPrefixes: ['time-panel'], scenario: 's0-seed-time-twin-clocks',
    },
    thermo: {
        panelId: 'thermo', root: '#panel-thermo', api: 'window:__ftdThermoPanel',
        methods: ['update'], subscriberPrefixes: ['thermo-panel'], scenario: 'flux-thermalization',
    },
    'scale-context': {
        panelId: 'scale-context', root: '#panel-scale-context', api: 'window:__ftdScaleContextPanel',
        methods: ['update'], subscriberPrefixes: ['scale-context-panel'],
    },
});

const gateName = process.env.FTD_SCALE0_SIDEPANEL_GATE || '';
const gate = GATES[gateName];

test.describe('Scale 0 one-sidepanel release gate', () => {
    test.skip(!gate, `Set FTD_SCALE0_SIDEPANEL_GATE to one of: ${Object.keys(GATES).join(', ')}`);

    test(`${gateName || 'unselected'} sustains the formal hardware frame budget`, async ({ page }, testInfo) => {
        testInfo.setTimeout(150_000);
        const consoleErrors = attachConsoleWatcher(page);
        const scenario = gate.scenario || 'flux-pulse';
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await selectScale0Scenario(page, scenario, { settleMs: 0 });

        const sizeEnabled = await page.evaluate(() => {
            const option = [...document.querySelectorAll('#lattice-size option')]
                .find((candidate) => candidate.value === '97');
            return !!option && !option.disabled;
        });
        expect(sizeEnabled, 'L=97 is available on the worker-backed browser path').toBe(true);
        await page.selectOption('#lattice-size', '97');
        await expect.poll(async () => page.evaluate(async (scenarioId) => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            const owner = state.fluxMock;
            return state.currentScenarioId === scenarioId
                && state.useFluxMock === true
                && owner?.isWorker === true
                && owner.ready === true
                && Number(owner.latticeSize) === 97;
        }, scenario), { timeout: 90_000 }).toBe(true);

        await page.evaluate((panelId) => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            dock?.setCollapsed(false);
            dock?.activate(panelId);
            const play = document.getElementById('btn-play');
            if (play?.getAttribute('data-paused') === 'true') play.click();
            if (panelId === 'knots') {
                const tracking = document.getElementById('kp-toggle-tracking');
                if (tracking && !tracking.checked) {
                    tracking.checked = true;
                    tracking.dispatchEvent(new Event('change', { bubbles: true }));
                }
            }
        }, gate.panelId);
        await expect(page.locator(gate.root)).toHaveClass(/active/);
        await page.waitForTimeout(3_000);

        const report = await page.evaluate(async (config) => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const gl = window.__ftdCtx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';
            const resolveApi = () => {
                if (!config.api) return null;
                const [scope, name] = config.api.split(':');
                return scope === 'ctx' ? window.__ftdCtx?.[name] : window[name];
            };

            probe.startScale0UiAuditProbe({
                rootSelector: config.root,
                subscriberPrefixes: config.subscriberPrefixes,
            });
            const api = resolveApi();
            if (api && config.methods.length) {
                probe.trackScale0UiMethods(config.panelId, api, config.methods);
            }
            await new Promise((resolve) => setTimeout(resolve, 12_000));
            return { ...await probe.stopScale0UiAuditProbe(), webglRenderer };
        }, gate);

        await testInfo.attach(`scale0-${gateName}-release-performance.json`, {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        console.log(`scale0 ${gateName} release performance`, JSON.stringify(report));

        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        for (const [name, timing] of Object.entries(report.callbacks)) {
            expect(timing.count, `${name} was exercised`).toBeGreaterThan(0);
            expect(timing.p95Ms, `${name} p95 update cost`).toBeLessThanOrEqual(2);
            expect(timing.maxMs, `${name} maximum update cost`).toBeLessThanOrEqual(8);
        }
        expect(report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.resourceDelta.domNodes, 'panel retains no additional DOM nodes')
            .toBeLessThanOrEqual(0);
        expect(report.resourceDelta.canvases).toBe(0);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
