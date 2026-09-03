// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectAEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('ae-scenario-select');
        if (!sel) throw new Error('ae-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

test.describe('Scale 2 atom scenarios and overlays', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('ionic preset lights force arrows and the decomposition works on the default bridge', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        // Deliberately the DEFAULT page (no ?dev=1): the production bridge is
        // WasmBridge, whose AE surface forwards to the JS fallback. Before
        // 2026-06-10 it never forwarded aeGetForceDecomposition, so every
        // force-arrow toggle threw a TypeError each compute frame (B10).
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-nacl-form');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-nacl-form did not seed atoms' },
        ).toBe(2);
        await expect.poll(() => page.evaluate(() => (
            window.__ftdCtx?.viewport?._molRenderer?._aeForceIonic
                ?.geometry?.drawRange?.count || 0
        )), { timeout: 10_000, message: 'ionic force glyphs were not rendered' }).toBeGreaterThan(0);

        const state = await page.evaluate(async () => {
            const presets = await import('./js/scales/scale2/scenarios.js');
            const ctrl = await import('./js/scales/scale2/controller.js');
            const b = window._ftdBridge;
            const decomp = b.aeGetForceDecomposition({ ionic: true });
            let ionicMag = 0;
            for (let i = 0; i < decomp.count * 3; i++) ionicMag += Math.abs(decomp.ionic[i]);
            const rt = b.aeGetRuntimeState();
            return {
                bridgeType: b.constructor?.name,
                preset: presets.getAEScenarioPreset('ae-nacl-form').visuals,
                visState: ctrl.getAEVisualState(),
                buttons: {
                    ionic: document.getElementById('ae-force-ionic')?.classList.contains('active') || false,
                    vdw: document.getElementById('ae-force-vdw')?.classList.contains('active') || false,
                    field: document.getElementById('toggle-ae-field')?.classList.contains('active') || false,
                },
                decompCount: decomp.count,
                ionicLen: decomp.ionic.length,
                ionicNonzero: ionicMag > 0,
                runtimeToggles: rt?.toggles ?? null,
                ionicGlyphVertices: window.__ftdCtx?.viewport?._molRenderer
                    ?._aeForceIonic?.geometry?.drawRange?.count || 0,
            };
        });

        // B10 regression: the production bridge must be WasmBridge AND the
        // decomposition must come back through its AE forwarding.
        expect(state.bridgeType).toBe('WasmBridge');
        expect(state.decompCount).toBe(2);
        expect(state.ionicLen).toBe(6);
        expect(state.ionicNonzero).toBe(true);

        // Preset → flags → DOM buttons, end to end.
        expect(state.preset.forceIonic).toBe(true);
        expect(state.preset.field).toBe(true);
        expect(state.visState.showAEForceIonic).toBe(true);
        expect(state.visState.showAEField).toBe(true);
        expect(state.buttons).toMatchObject({ ionic: true, vdw: false, field: true });

        // Engine truth from the runtime snapshot: NaCl disables auto-bonding.
        expect(state.runtimeToggles).not.toBeNull();
        expect(state.runtimeToggles.bonding).toBe(false);
        expect(state.runtimeToggles.ionic).toBe(true);
        expect(state.ionicGlyphVertices).toBeGreaterThan(0);
        expect(state.ionicGlyphVertices % 6).toBe(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('periodic table preset disables clouds; every visual control clicks cleanly', async ({ page }) => {
        // Heavy spec: the click audit exercises every per-frame overlay path.
        // 60s per the project's heavy-spec pattern (timeout-not-assert +
        // healthy page = load, not regression).
        test.setTimeout(60_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-periodic');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-periodic did not seed 118 atoms' },
        ).toBe(118);

        const preset = await page.evaluate(async () => {
            const ctrl = await import('./js/scales/scale2/controller.js');
            return {
                cloudsCheckbox: document.getElementById('ae-show-clouds')?.checked ?? null,
                visClouds: ctrl.getAEVisualState().showOrbitalClouds,
                visShells: ctrl.getAEVisualState().showNucleusShells,
            };
        });
        expect(preset.cloudsCheckbox).toBe(false);
        expect(preset.visClouds).toBe(false);
        expect(preset.visShells).toBe(false);

        // The click audit runs on the 6-atom water dimer, NOT the 118-atom
        // periodic table: with all force channels on, the O(N²)
        // decomposition every 2nd frame saturates the main thread at N=118
        // and page.evaluate round-trips blow the test budget. Wiring
        // correctness (button → flag → viewport) is atom-count-independent.
        await selectAEScenario(page, 'ae-water-dimer');
        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-water-dimer did not seed atoms' },
        ).toBe(6);

        // Play so per-frame update paths execute while we click.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        // The end-to-end button→flag→viewport audit: click EVERY Scale 2
        // visual control on and off; any wiring break throws to the console.
        const clickIds = [
            'ae-show-shells', 'ae-show-labels', 'ae-show-shell-bounds', 'ae-show-lobes',
            'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond',
            'ae-force-hbond', 'ae-force-angle', 'ae-force-dipole', 'ae-force-net',
            'toggle-ae-field', 'toggle-ae-velocities', 'toggle-ae-dipoles',
            'toggle-ae-hbonds', 'toggle-ae-nuclear-events', 'toggle-ae-radiation',
            'toggle-ae-heat', 'toggle-ae-nuclear-boundary', 'ae-show-clouds',
        ];
        const flagById = {
            'ae-show-shells': 'showNucleusShells',
            'ae-show-labels': 'showElementLabels',
            'ae-show-shell-bounds': 'showShellBounds',
            'ae-show-lobes': 'showOrbitalLobes',
            'ae-force-ionic': 'showAEForceIonic',
            'ae-force-vdw': 'showAEForceVdw',
            'ae-force-bond': 'showAEForceBond',
            'ae-force-hbond': 'showAEForceHBond',
            'ae-force-angle': 'showAEForceAngle',
            'ae-force-dipole': 'showAEForceDipole',
            'ae-force-net': 'showAEForceNet',
            'toggle-ae-field': 'showAEField',
            'toggle-ae-velocities': 'showAEVelocities',
            'toggle-ae-dipoles': 'showAEDipoles',
            'toggle-ae-hbonds': 'showAEHBondLines',
            'toggle-ae-nuclear-events': 'showAENuclearEvents',
            'toggle-ae-radiation': 'showAERadiation',
            'toggle-ae-heat': 'showAEHeat',
            'toggle-ae-nuclear-boundary': 'showAENuclearBoundary',
            'ae-show-clouds': 'showOrbitalClouds',
        };
        for (const id of clickIds) {
            const wired = await page.evaluate(async ({ elId, flag }) => {
                const el = document.getElementById(elId);
                if (!el) throw new Error(`missing control: ${elId}`);
                el.click();
                const ctrl = await import('./js/scales/scale2/controller.js');
                const controlValue = el instanceof HTMLInputElement
                    ? el.checked
                    : el.classList.contains('active');
                return {
                    controlValue,
                    flagValue: ctrl.getAEVisualState()[flag],
                    ariaPressed: el instanceof HTMLButtonElement
                        ? el.getAttribute('aria-pressed')
                        : null,
                };
            }, { elId: id, flag: flagById[id] });
            expect(wired.flagValue, `${id} did not update ${flagById[id]}`).toBe(wired.controlValue);
            if (wired.ariaPressed !== null) {
                expect(wired.ariaPressed).toBe(wired.controlValue ? 'true' : 'false');
            }
            await page.waitForTimeout(150);
        }
        // Cycle the bond style select through all options.
        for (const styleValue of ['lines', 'off', 'cylinders']) {
            await page.evaluate((v) => {
                const sel = document.getElementById('bond-style-select');
                if (!sel) throw new Error('missing bond-style-select');
                sel.value = v;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, styleValue);
            await page.waitForTimeout(150);
        }
        // Toggle everything back off.
        for (const id of clickIds) {
            await page.evaluate((elId) => document.getElementById(elId)?.click(), id);
            await page.waitForTimeout(100);
        }
        await page.waitForTimeout(500);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('water dimer preset arms dashed H-bond lines with real pairs', async ({ page }) => {
        test.setTimeout(90_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-water-dimer');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-water-dimer did not seed atoms' },
        ).toBe(6);

        const state = await page.evaluate(async () => {
            const ctrl = await import('./js/scales/scale2/controller.js');
            const b = window._ftdBridge;
            const hb = b.aeGetHBondPairs();
            const vel = b.aeGetVelocities();
            const dip = b.aeGetDipoles();
            return {
                btnActive: document.getElementById('toggle-ae-hbonds')?.classList.contains('active') || false,
                flagOn: ctrl.getAEVisualState().showAEHBondLines,
                pairs: hb.count,
                segmentsLen: hb.segments.length,
                velCount: vel.count,
                dipCount: dip.count,
            };
        });

        expect(state.btnActive).toBe(true);
        expect(state.flagOn).toBe(true);
        // Symmetric dimer: each donor-H sees the other molecule's O.
        expect(state.pairs).toBeGreaterThanOrEqual(1);
        expect(state.segmentsLen).toBe(state.pairs * 6);
        // The sibling getters expose full per-atom arrays.
        expect(state.velCount).toBe(6);
        expect(state.dipCount).toBe(6);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('thin bonds resolve stable atom IDs instead of mutable array slots', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const rendered = await page.evaluate(() => {
            const renderer = window.__ftdCtx.viewport._molRenderer;
            renderer.updateBondLines({
                count: 2,
                ids: new Uint32Array([41, 99]),
                positions: new Float32Array([1, 2, 3, 7, 8, 9]),
                colors: new Float32Array([1, 0, 0, 0, 0, 1]),
                bonds: new Uint32Array([41, 99]),
                bondCount: 1,
            });
            return {
                drawCount: renderer.bondLines.geometry.drawRange.count,
                positions: Array.from(renderer.bondLines.geometry.getAttribute('position').array.slice(0, 6)),
            };
        });

        expect(rendered.drawCount).toBe(2);
        expect(rendered.positions).toEqual([1, 2, 3, 7, 8, 9]);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('large overlay buffers do not silently truncate valid Scale 2 records', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const capacity = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const renderer = viewport._molRenderer;
            const pairCount = 300;
            const segments = new Float32Array(pairCount * 6);
            for (let i = 0; i < pairCount; i++) {
                segments[i * 6] = i * 0.01;
                segments[i * 6 + 3] = i * 0.01 + 0.5;
            }
            renderer.toggleHBondLines(true);
            renderer.updateHBondLines(segments, pairCount);
            viewport.toggleVelocityVectors(true);
            return {
                hbondCapacity: renderer._hbondCapacity,
                hbondVertices: renderer._hbondLines.geometry.drawRange.count,
                hbondVisible: renderer._hbondLines.visible,
                velocityCapacity: viewport._particleRenderer.velocityVectors
                    .geometry.getAttribute('position').array.length / 6,
            };
        });

        expect(capacity.hbondCapacity).toBeGreaterThanOrEqual(300);
        expect(capacity.hbondVertices).toBe(600);
        expect(capacity.hbondVisible).toBe(true);
        expect(capacity.velocityCapacity).toBeGreaterThanOrEqual(2048);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('paused electrostatic preset renders aligned potential and E samples', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-nacl-form');

        await expect.poll(() => page.evaluate(() => {
            const fr = window.__ftdCtx?.viewport?._fieldRenderer;
            return fr?._fieldVectors?.geometry?.drawRange?.count || 0;
        }), { timeout: 10_000, message: 'paused field vectors were never sampled' }).toBeGreaterThan(0);

        const alignment = await page.evaluate(() => {
            const fr = window.__ftdCtx.viewport._fieldRenderer;
            const vector = fr._fieldVectors.geometry.getAttribute('position').array;
            const heat = fr._fieldHeatmap.geometry.getAttribute('position').array;
            return {
                vectorVisible: fr._fieldVectors.visible,
                heatVisible: fr._fieldHeatmap.visible,
                vectorCount: fr._fieldVectors.geometry.drawRange.count,
                heatCount: fr._fieldHeatmap.geometry.drawRange.count,
                dx: vector[0] - heat[0],
                dy: vector[1] - heat[1],
                dz: vector[2] - heat[2],
            };
        });

        expect(alignment.vectorVisible).toBe(true);
        expect(alignment.heatVisible).toBe(true);
        expect(alignment.vectorCount).toBe(alignment.heatCount * 2);
        expect(alignment.dx).toBeCloseTo(0, 6);
        expect(alignment.dy).toBeCloseTo(0.3, 6);
        expect(alignment.dz).toBeCloseTo(0, 6);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('bond-cloud rendering preserves atom picking and bonded-component inspection', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-water-dimer');
        await expect.poll(
            () => page.evaluate(() => window.__ftdCtx?.inspector?._aeCloudCount || 0),
            { timeout: 10_000, message: 'orbital/bond cloud context was not published to the inspector' },
        ).toBeGreaterThan(0);

        const result = await page.evaluate(() => {
            const inspector = window.__ftdCtx.inspector;
            const atomData = window._ftdBridge.aeGetAtomData();
            const rect = inspector.viewport.renderer.domElement.getBoundingClientRect();
            const p = inspector._aePickProjection
                .set(atomData.positions[0], atomData.positions[1], atomData.positions[2])
                .project(inspector.viewport.camera);
            const clientX = rect.left + (p.x + 1) * rect.width / 2;
            const clientY = rect.top + (1 - p.y) * rect.height / 2;
            const picked = inspector.pickAEAtomAtClientPoint(clientX, clientY);
            const component = window._ftdBridge.aeInspectAtom(picked)?.component;
            const activeMap = Array.from(
                inspector._aeCloudAtomMap.subarray(0, inspector._aeCloudCount),
            );
            return {
                picked,
                expected: Number(atomData.ids[0]),
                component,
                cloudMode: inspector._aeCloudMode,
                hasDecorativeSamples: activeMap.some(atomIndex => atomIndex === -1),
                hasMappedSamples: activeMap.some(atomIndex => atomIndex >= 0),
            };
        });

        expect(result.cloudMode).toBe(true);
        expect(result.picked).toBe(result.expected);
        expect(result.component?.count).toBe(3);
        expect(result.component?.members).toContain(result.expected);
        expect(result.hasMappedSamples).toBe(true);
        expect(result.hasDecorativeSamples).toBe(true);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('viewport overlay groups controls by physical term on Scale 2', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const overlay = await page.evaluate(() => {
            const panel = document.getElementById('ae-viewport-overlay');
            const toolbarForces = document.getElementById('ae-force-controls');
            return {
                panelPresent: !!panel,
                sectionCount: panel?.querySelectorAll('.scale-overlay-section').length ?? 0,
                hasForceDecomp: !!panel?.querySelector('#ae-force-ionic'),
                hasCloudsInPanel: !!panel?.querySelector('#ae-show-clouds'),
                cloudsInToolbar: !!document.querySelector('#ae-controls #ae-show-clouds'),
                toolbarForcesVisible: toolbarForces
                    ? getComputedStyle(toolbarForces).display !== 'none'
                    : null,
                footnote: panel?.querySelector('.scale-overlay-footnote')?.textContent?.trim() ?? '',
                hasHeader: !!panel?.querySelector('.scale-overlay-header'),
                title: panel?.querySelector('.scale-overlay-title')?.textContent?.trim() ?? '',
                missingTooltips: Array.from(panel?.querySelectorAll(
                    '.scale-overlay-section button, .scale-overlay-section label, .scale-overlay-section select',
                ) || [])
                    .filter((element) => !element.getAttribute('title') && !element.dataset.uiTooltip)
                    .map((element) => element.id || element.textContent?.trim()),
                buttonsWithoutAria: Array.from(panel?.querySelectorAll('button.view-toggle') || [])
                    .filter((button) => button.getAttribute('aria-pressed') === null)
                    .map((button) => button.id),
                duplicateIds: Array.from(panel?.querySelectorAll('[id]') || [])
                    .map((element) => element.id)
                    .filter((id, index, ids) => ids.indexOf(id) !== index),
                duplicatedOutsidePanel: Array.from(panel?.querySelectorAll('[id]') || [])
                    .map((element) => element.id)
                    .filter((id) => document.querySelectorAll(`#${CSS.escape(id)}`).length !== 1),
            };
        });

        expect(overlay.panelPresent).toBe(true);
        expect(overlay.sectionCount).toBeGreaterThanOrEqual(6);
        expect(overlay.hasForceDecomp).toBe(true);
        expect(overlay.hasCloudsInPanel).toBe(true);
        expect(overlay.hasHeader).toBe(true);
        expect(overlay.cloudsInToolbar).toBe(false);
        expect(overlay.toolbarForcesVisible).toBeNull();
        expect(overlay.footnote).toContain('substrate-QM recovery');
        expect(overlay.title).toContain('Atom');
        expect(overlay.missingTooltips).toEqual([]);
        expect(overlay.buttonsWithoutAria).toEqual([]);
        expect(overlay.duplicateIds).toEqual([]);
        expect(overlay.duplicatedOutsidePanel).toEqual([]);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('overlay remains contained and scrollable in a constrained desktop viewport', async ({ page }) => {
        test.setTimeout(90_000);
        const errors = attachConsoleWatcher(page);
        // Below the desktop shell breakpoint the dashboard becomes a bottom
        // sheet. This gate targets the narrow desktop layout where the overlay
        // and side panel coexist.
        await page.setViewportSize({ width: 1280, height: 700 });
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const layout = await page.evaluate(() => {
            const panel = document.getElementById('ae-viewport-overlay');
            const body = panel?.querySelector('.scale-overlay-body');
            const panelRect = panel?.getBoundingClientRect();
            const bodyRect = body?.getBoundingClientRect();
            const statusRect = document.getElementById('status-bar')?.getBoundingClientRect();
            const controls = Array.from(panel?.querySelectorAll('button, label, select') || [])
                .filter((control) => control.getBoundingClientRect().width > 0);
            const style = body ? getComputedStyle(body) : null;
            return {
                visible: !!panel && getComputedStyle(panel).display !== 'none',
                panelLeft: panelRect?.left ?? -1,
                panelRight: panelRect?.right ?? Infinity,
                panelBottom: panelRect?.bottom ?? Infinity,
                viewportWidth: innerWidth,
                statusTop: statusRect?.top ?? innerHeight,
                overflowY: style?.overflowY || '',
                scrollHeight: body?.scrollHeight || 0,
                clientHeight: body?.clientHeight || 0,
                controlsFit: controls.every((control) => {
                    const rect = control.getBoundingClientRect();
                    return rect.left >= (bodyRect?.left ?? 0) - 1 &&
                        rect.right <= (bodyRect?.right ?? innerWidth) + 1;
                }),
            };
        });

        expect(layout.visible).toBe(true);
        expect(layout.panelLeft).toBeGreaterThanOrEqual(0);
        expect(layout.panelRight).toBeLessThanOrEqual(layout.viewportWidth + 1);
        expect(layout.panelBottom).toBeLessThanOrEqual(layout.statusTop + 1);
        expect(layout.overflowY).toBe('auto');
        expect(layout.scrollHeight).toBeGreaterThan(layout.clientHeight);
        expect(layout.controlsFit).toBe(true);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
