// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectPEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('pe-scenario-select');
        if (!sel) throw new Error('pe-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function openPanel(page, id) {
    await page.locator(`#tab-bar .tab[data-panel="${id}"]`).click();
    await expect(page.locator(`#panel-${id}`)).toHaveClass(/active/);
}

test.describe('Scale 1 native-engine scenarios and overlays', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('Native Matter is the default read-only registered replay', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'native matter replay did not load' },
        ).toBe(2);

        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            const snapshot = b.peGetSnapshot('s1-native-m3-replay');
            const forces = b.peGetForces();
            const rejectedInjection = b.peAddParticle(
                'electron', -1, 5, 0, 0, 0, 0, 0, 0.511, 0.1);
            return {
                workspaceSelectorPresent: !!document.getElementById('pe-mode-select'),
                scenarioOptionCount: document.getElementById('pe-scenario-select')?.options.length,
                selectedScenario: document.getElementById('pe-scenario-select')?.value,
                core: snapshot?.core,
                objectCount: snapshot?.objects?.length || 0,
                identityAvailable: Array.from(snapshot?.objects || [])
                    .every(object => object.identityAvailable),
                centerObservers: Array.from(snapshot?.objects || [])
                    .map(object => [object.integerCenterAvailable, object.fractionalCenterAvailable]),
                massAvailable: Array.from(snapshot?.objects || []).map(o => o.massAvailable),
                kineticAvailable: Array.from(snapshot?.objects || []).map(o => o.kineticEnergyAvailable),
                conservation: snapshot?.conservation,
                unavailable: Array.from(snapshot?.unavailableReasons || []),
                outgoing: Array.from(snapshot?.fields || [])
                    .find(field => field.channel === 'outgoing'),
                forceCount: forces.count,
                rejectedInjection,
                countAfterRejectedInjection: b.peGetParticleData().count,
                registryRows: b.peGetPhysicsRegistry()?.physics?.length || 0,
            };
        });

        expect(state.workspaceSelectorPresent).toBe(false);
        expect(state.scenarioOptionCount).toBe(36);
        expect(state.selectedScenario).toBe('s1-native-m3-replay');
        expect(state.core.mode).toBe('native_matter');
        expect(state.core.workspace).toBe('particle_observatory');
        expect(state.core.scenarioClass).toBe('qualified_replay');
        expect(state.core.schemaVersion).toBe(3);
        expect(state.core.dynamicsOwner).toBe('native_matter_observer');
        expect(state.core.readOnly).toBe(true);
        expect(state.objectCount).toBe(2);
        expect(state.identityAvailable).toBe(true);
        expect(state.centerObservers).toEqual([[true, true], [true, true]]);
        expect(state.massAvailable).toEqual([false, false]);
        expect(state.kineticAvailable).toEqual([false, false]);
        expect(state.conservation.stateEnergyComplete).toBe(false);
        expect(state.conservation.driftEligible).toBe(false);
        expect(state.outgoing.available).toBe(false);
        expect(state.outgoing.unavailableReason).toMatch(/No certified persistent outgoing/i);
        expect(state.unavailable.some(reason => /Mass, conserved charge/i.test(reason))).toBe(true);
        expect(state.forceCount).toBe(0);
        expect(state.rejectedInjection).toBe(-1);
        expect(state.countAfterRejectedInjection).toBe(2);
        expect(state.registryRows).toBe(12);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('scenario behavior contract exposes M3 views and A/B controls', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        await expect(page.locator('#pe-scenario-behavior')).toHaveText('READ-ONLY REPLAY');
        await expect(page.locator('#pe-m3-view-group')).toBeVisible();
        await expect(page.locator('#pe-m3-view-select option')).toHaveCount(6);

        await openPanel(page, 'diagnostics');
        await page.locator('#pe-m3-view-select').selectOption('fields');
        await expect(page.locator('[data-section="pe-m3-fields"]')).toBeVisible();
        await expect(page.locator('[data-section="pe-m3-fields"]')).toContainText('actual');
        await expect(page.locator('[data-section="pe-m3-fields"]')).toContainText('selected bound');
        await expect(page.locator('[data-section="pe-m3-fields"]')).toContainText('outgoing');
        await expect(page.locator('[data-section="pe-m3-fields"]')).toContainText('unavailable');

        await selectPEScenario(page, 's1-qed-static-coulomb');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('STATIC FIELD');
        await expect(page.locator('#pe-m3-view-group')).toBeHidden();
        await expect(page.locator('#pe-scenario-contract')).toContainText(/Sources are intentionally locked/i);

        await selectPEScenario(page, 's1-quantum-exchange-spinless-control');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('ZERO EXPECTED');
        await expect(page.locator('#pe-paired-scenario')).toBeVisible();
        await expect(page.locator('#pe-scenario-contract')).toContainText(/Zero target-force response/i);
        await page.locator('#pe-paired-scenario').click();
        await expect(page.locator('#pe-scenario-select')).toHaveValue('s1-quantum-exchange-eligible');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('DYNAMIC');

        await selectPEScenario(page, 's1-empty-zoo');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('WAITING FOR INJECTION');
        await expect(page.locator('#pe-scenario-contract')).toContainText(/intentionally empty/i);

        await selectPEScenario(page, 's1-mass-ladder');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('STATIC REFERENCE');
        await expect(page.locator('#pe-scenario-contract')).toContainText(/No interaction is expected/i);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('scenario and reset UI changes preserve the selected Scale 1 panel', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await openPanel(page, 'particle-log');

        const expectParticleLogSelected = async () => {
            await expect(page.locator('#panel-particle-log')).toHaveClass(/active/);
            await expect(page.locator('#tab-bar .tab[data-panel="particle-log"]'))
                .toHaveClass(/active/);
            await expect(page.locator('#panel-diagnostics')).not.toHaveClass(/active/);
        };

        await selectPEScenario(page, 's1-qed-static-coulomb');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('STATIC FIELD');
        await expectParticleLogSelected();

        await selectPEScenario(page, 's1-empty-zoo');
        await expect(page.locator('#pe-scenario-behavior')).toHaveText('WAITING FOR INJECTION');
        await expectParticleLogSelected();

        await page.locator('#btn-reset').click();
        await expect(page.locator('#pe-scenario-select')).toHaveValue('s1-empty-zoo');
        await expectParticleLogSelected();

        await page.evaluate(() => document.getElementById('btn-pe-clear')?.click());
        await expect(page.locator('#pe-scenario-select')).toHaveValue('s1-empty-zoo');
        await expectParticleLogSelected();

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('battery discharge directs electrons under the all-applicable profile', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-open-terminal-battery');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count ?? -1),
            { message: 'open-terminal battery did not seed its electrodes and carrier' },
        ).toBe(18);

        const readBattery = () => page.evaluate(() => {
            const data = window._ftdBridge.peGetParticleData();
            const rows = Array.from({ length: data.count }, (_, index) => ({
                charge: Number(data.charges[index]),
                locked: !!data.locked[index],
                x: Number(data.positions[index * 3]),
            }));
            const mobile = rows.filter(row => !row.locked);
            const visual = window.__ftdCtx?.viewport?._particleRenderer?._peScenarioVisual;
            return {
                count: rows.length,
                locked: rows.filter(row => row.locked).length,
                mobile: mobile.length,
                netCharge: rows.reduce((sum, row) => sum + row.charge, 0),
                positiveProbeX: mobile.find(row => row.charge > 0)?.x ?? null,
                negativeProbeX: mobile.find(row => row.charge < 0)?.x ?? null,
                visualType: visual?.userData?.scenarioVisualType ?? null,
                positiveLabel: !!visual?.getObjectByName?.('positive-terminal-label'),
                negativeLabel: !!visual?.getObjectByName?.('negative-terminal-label'),
                presentationOnly: visual?.userData?.presentationOnly ?? false,
                physicalConstraint: visual?.userData?.physicalConstraint ?? null,
                portCount: visual?.userData?.portCount ?? 0,
                positiveDirection: visual?.getObjectByName?.('positive-terminal')
                    ?.userData?.electronDirection ?? null,
                negativeDirection: visual?.getObjectByName?.('negative-terminal')
                    ?.userData?.electronDirection ?? null,
            };
        });

        const before = await readBattery();
        expect(before).toMatchObject({
            count: 18,
            locked: 17,
            mobile: 1,
            netCharge: 0,
            visualType: 'open-terminal-battery',
            positiveLabel: true,
            negativeLabel: true,
            presentationOnly: true,
            physicalConstraint: 'native-perfect-insulator',
            portCount: 2,
            positiveDirection: 'in',
            negativeDirection: 'out',
        });
        expect(before.positiveProbeX).toBeNull();
        expect(before.negativeProbeX).toBeGreaterThan(0);

        await page.evaluate(() => document.getElementById('btn-pe-profile-applicable')?.click());
        expect(await page.evaluate(() => {
            const bridge = window._ftdBridge;
            return Array.from(bridge.peGetPhysicsRegistry().physics)
                .filter(spec => spec.available)
                .every(spec => bridge.peGetToggle(spec.toggle));
        })).toBe(true);

        await page.evaluate(() => {
            for (let tick = 0; tick < 160; tick++) window._ftdBridge.peTick();
        });
        const after = await readBattery();
        expect(after.positiveProbeX).toBeNull();
        expect(after.negativeProbeX).toBeGreaterThan(12);

        const containment = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const blockedId = bridge.peAddParticle(null, -1, 0, 2, 0, 0.5, 0, 0, 1000, 0.1);
            const outletId = bridge.peAddParticle(null, -1, 0, 0, 0, 0.5, 0, 0, 1000, 0.1);
            const positiveLeakId = bridge.peAddParticle(null, -1, 0, 1, 0, -0.5, 0, 0, 1000, 0.1);
            const inletId = bridge.peAddParticle(null, -1, -14, 0, 0, 0.5, 0, 0, 1, 0.1);
            for (let tick = 0; tick < 160; tick++) bridge.peTick();
            const data = bridge.peGetExtendedData();
            const row = id => {
                const index = Array.from(data.ids).indexOf(id);
                if (index < 0) return { removed: true, x: null, vx: null };
                return {
                    removed: false,
                    x: Number(data.positions[index * 3]),
                    vx: Number(data.velocities[index * 3]),
                };
            };
            return {
                blocked: row(blockedId),
                outlet: row(outletId),
                positiveLeak: row(positiveLeakId),
                inlet: row(inletId),
                diagnostics: bridge.peGetDiagnostics(),
            };
        });
        expect(containment.blocked.x).toBeLessThan(12);
        expect(containment.blocked.vx).toBeLessThan(0);
        expect(containment.outlet.x).toBeGreaterThan(12);
        expect(containment.outlet.vx).toBeGreaterThan(0);
        expect(containment.positiveLeak.x).toBeGreaterThan(-12);
        expect(containment.positiveLeak.vx).toBeGreaterThan(0);
        expect(containment.inlet.removed).toBe(true);
        expect(containment.diagnostics.insulatorCollisionCount).toBeGreaterThanOrEqual(2);
        expect(containment.diagnostics.insulatorPortCrossingCount).toBeGreaterThanOrEqual(2);
        expect(containment.diagnostics.contactEventCount).toBeGreaterThanOrEqual(1);

        await selectPEScenario(page, 's1-coulomb-orbit');
        expect(await page.evaluate(() =>
            window.__ftdCtx?.viewport?._particleRenderer?._peScenarioVisual ?? null,
        )).toBeNull();
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('Particle Zoo groups collapse persistently and remain contained when detached', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 2560, height: 1440 });
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await page.locator('#tab-bar .tab[data-panel="zoo"]').click();

        const categoryGroups = page.locator('#panel-zoo .zoo-group');
        await expect(categoryGroups.first()).toBeVisible();
        expect(await categoryGroups.count()).toBeGreaterThan(1);
        await expect(categoryGroups.first()).toHaveJSProperty('open', true);

        const dockedLayout = await page.evaluate(() => {
            const panel = document.getElementById('panel-zoo');
            const shell = panel?.querySelector('.panel-resource-shell');
            const table = panel?.querySelector('#zoo-table-container');
            const dockBody = panel?.parentElement;
            return {
                dockBodyHeight: dockBody?.getBoundingClientRect().height || 0,
                panelHeight: panel?.getBoundingClientRect().height || 0,
                shellHeight: shell?.getBoundingClientRect().height || 0,
                tableHeight: table?.getBoundingClientRect().height || 0,
            };
        });
        expect(Math.abs(dockedLayout.dockBodyHeight - dockedLayout.panelHeight)).toBeLessThanOrEqual(2);
        expect(Math.abs(dockedLayout.panelHeight - dockedLayout.shellHeight)).toBeLessThanOrEqual(2);
        expect(dockedLayout.tableHeight).toBeGreaterThan(300);

        const categoryKey = await categoryGroups.first().getAttribute('data-zoo-group');
        await categoryGroups.first().locator('summary').click();
        await expect(categoryGroups.first()).toHaveJSProperty('open', false);

        // Search triggers a full catalog re-render; the keyed collapse state
        // must survive it rather than snapping the section open again.
        await page.locator('#zoo-search').fill('electron');
        await page.locator('#zoo-search').fill('');
        await expect(page.locator(`[data-zoo-group="${categoryKey}"]`)).toHaveJSProperty('open', false);

        await page.locator('#zoo-group-by').selectOption('generation');
        const generationGroups = page.locator('#panel-zoo .zoo-group');
        await expect(generationGroups.first()).toBeVisible();
        const generationKey = await generationGroups.first().getAttribute('data-zoo-group');
        await generationGroups.first().locator('summary').click();
        await expect(generationGroups.first()).toHaveJSProperty('open', false);

        await page.locator('#zoo-group-by').selectOption('category');
        await expect(page.locator(`[data-zoo-group="${categoryKey}"]`)).toHaveJSProperty('open', false);
        await page.locator('#zoo-group-by').selectOption('generation');
        await expect(page.locator(`[data-zoo-group="${generationKey}"]`)).toHaveJSProperty('open', false);

        const detached = await page.evaluate(() => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            const win = dock?.floatPanel?.('zoo', 120, 80);
            if (!win?.el) return null;
            const body = win.el.querySelector('.floating-window-body');
            const panel = win.el.querySelector('#panel-zoo');
            const shell = panel?.querySelector('.panel-resource-shell');
            const toolbar = panel?.querySelector('.panel-resource-toolbar');
            const table = panel?.querySelector('#zoo-table-container');
            const bodyRect = body?.getBoundingClientRect();
            const panelRect = panel?.getBoundingClientRect();
            const shellRect = shell?.getBoundingClientRect();
            const toolbarRect = toolbar?.getBoundingClientRect();
            const tableRect = table?.getBoundingClientRect();
            const report = {
                inWindow: !!panel,
                bodyOverflow: body ? getComputedStyle(body).overflowY : '',
                tableOverflowY: table ? getComputedStyle(table).overflowY : '',
                tableOverflowX: table ? getComputedStyle(table).overflowX : '',
                tableFlexGrow: table ? getComputedStyle(table).flexGrow : '',
                panelHeight: panelRect?.height || 0,
                shellHeight: shellRect?.height || 0,
                toolbarHeight: toolbarRect?.height || 0,
                tableHeight: tableRect?.height || 0,
                unusedShellHeight: shellRect && toolbarRect && tableRect
                    ? shellRect.height - toolbarRect.height - tableRect.height : Infinity,
                containedVertically: !!(bodyRect && tableRect &&
                    tableRect.top >= bodyRect.top - 1 && tableRect.bottom <= bodyRect.bottom + 1),
                containedHorizontally: !!(bodyRect && tableRect &&
                    tableRect.left >= bodyRect.left - 1 && tableRect.right <= bodyRect.right + 1),
                cardOverflow: panel?.querySelector('.zoo-card')
                    ? getComputedStyle(panel.querySelector('.zoo-card')).overflow : '',
                groupCount: panel?.querySelectorAll('.zoo-group').length || 0,
            };
            win.dock();
            return report;
        });

        expect(detached).not.toBeNull();
        expect(detached?.inWindow).toBe(true);
        expect(['auto', 'scroll']).toContain(detached?.bodyOverflow);
        expect(['auto', 'scroll']).toContain(detached?.tableOverflowY);
        expect(['auto', 'scroll']).toContain(detached?.tableOverflowX);
        expect(detached?.tableFlexGrow).toBe('1');
        expect(Math.abs((detached?.panelHeight || 0) - (detached?.shellHeight || 0))).toBeLessThanOrEqual(2);
        expect(detached?.tableHeight).toBeGreaterThanOrEqual(160);
        expect(detached?.unusedShellHeight).toBeGreaterThanOrEqual(0);
        expect(detached?.unusedShellHeight).toBeLessThanOrEqual(20);
        expect(detached?.containedVertically).toBe(true);
        expect(detached?.containedHorizontally).toBe(true);
        expect(detached?.cardOverflow).toBe('visible');
        expect(detached?.groupCount).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('effective Coulomb orbit loads on the native engine with live data', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-coulomb-orbit');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'default scenario did not seed Scale 1 particles' },
        ).toBeGreaterThanOrEqual(2);

        const state = await page.evaluate(async () => {
            const reg = await import('./js/scales/scale1/scenario-registry.js');
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            const ext = b.peGetExtendedData();
            const forces = b.peGetForces();
            const diag = b.peGetDiagnostics();
            const caps = b.peGetBackendCapabilities();
            let maxSpeed = 0;
            for (let i = 0; i < data.count; i++) {
                maxSpeed = Math.max(maxSpeed, Math.hypot(
                    data.velocities[i * 3] || 0,
                    data.velocities[i * 3 + 1] || 0,
                    data.velocities[i * 3 + 2] || 0));
            }
            return {
                preset: reg.getScale1ScenarioPreset('s1-coulomb-orbit', b.peGetPhysicsRegistry()),
                backend: caps.backend,
                nativeForces: caps.nativeForces,
                count: data.count,
                velocityLength: data.velocities?.length || 0,
                massLength: data.masses?.length || 0,
                spinAxesLength: data.spinAxes?.length || 0,
                maxSpeed,
                extCount: ext?.count || 0,
                forceCount: forces.count,
                maxForce: forces.maxForce,
                coulombPE: diag.coulombPE,
                totalEnergy: diag.totalEnergy,
                descShown: !!document.getElementById('s1-scenario-desc-text')?.textContent?.length,
            };
        });

        expect(state.backend).toBe('wasm');
        expect(state.nativeForces).toBe(true);
        expect(state.preset.physics.coulomb).toBe(true);
        expect(state.count).toBeGreaterThanOrEqual(2);
        expect(state.velocityLength).toBe(state.count * 3);
        expect(state.massLength).toBe(state.count);
        expect(state.spinAxesLength).toBe(state.count * 3);
        expect(state.maxSpeed).toBeGreaterThan(0);          // orbit IC applied
        expect(state.extCount).toBe(state.count);
        expect(state.forceCount).toBe(state.count);
        expect(state.maxForce).toBeGreaterThan(0);
        expect(Math.abs(state.coulombPE)).toBeGreaterThan(0);
        expect(state.totalEnergy).toBeLessThan(0);          // bound orbit
        expect(state.descShown).toBe(true);                 // epistemic status rendered

        const decomp = await page.evaluate(() => {
            const d = window._ftdBridge?.peGetForceDecomposition?.();
            return d ? { count: d.count, maxCoulomb: d.maxCoulomb, maxNet: d.maxNet } : null;
        });
        expect(decomp?.count).toBe(state.count);
        expect(decomp?.maxCoulomb).toBeGreaterThan(0);
        expect(decomp?.maxNet).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('overlay audit keeps every force layer visible, bounded, and renderer-wired', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 1024, height: 768 });
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-quantum-exchange-eligible');
        await page.waitForTimeout(300);

        const audit = await page.evaluate(() => {
            const overlay = document.getElementById('pe-viewport-overlay');
            const body = overlay?.querySelector('.scale-overlay-body');
            const rect = overlay?.getBoundingClientRect();
            const viewportRect = document.getElementById('viewport')?.getBoundingClientRect();
            const ids = Array.from(overlay?.querySelectorAll('button.view-toggle') || [])
                .map(button => button.id);
            const buttons = Array.from(overlay?.querySelectorAll('button.view-toggle') || []);
            const horizontallyContained = buttons.every(button => {
                const buttonRect = button.getBoundingClientRect();
                return !rect || (buttonRect.left >= rect.left && buttonRect.right <= rect.right + 0.5);
            });
            const renderer = window.__ftdCtx?.viewport?._particleRenderer;
            return {
                buttonCount: ids.length,
                uniqueButtonCount: new Set(ids).size,
                advancedIds: [
                    'toggle-pe-force-lorentz', 'toggle-pe-force-exchange',
                    'toggle-pe-force-radiation', 'toggle-pe-force-magnetic-dipole',
                    'toggle-pe-force-spin-orbit',
                ].every(id => ids.includes(id)),
                exchangeActive: document.getElementById('toggle-pe-force-exchange')?.classList.contains('active'),
                exchangePressed: document.getElementById('toggle-pe-force-exchange')?.getAttribute('aria-pressed'),
                exchangeRendererVisible: renderer?._peForceExchange?.visible,
                coulombRendererVisible: renderer?._peForceCoulomb?.visible,
                summary: document.getElementById('pe-overlay-summary')?.textContent,
                overlayWithinViewport: !!rect && !!viewportRect
                    && rect.left >= viewportRect.left && rect.right <= viewportRect.right + 0.5
                    && rect.top >= viewportRect.top && rect.bottom <= viewportRect.bottom + 0.5,
                noHorizontalOverflow: !!body && body.scrollWidth <= body.clientWidth,
                verticallyBounded: !!body && body.clientHeight <= body.scrollHeight,
                horizontallyContained,
                sectionGroups: overlay?.querySelectorAll('.scale-overlay-section[role="group"]').length,
            };
        });

        expect(audit.buttonCount).toBe(17);
        expect(audit.uniqueButtonCount).toBe(17);
        expect(audit.advancedIds).toBe(true);
        expect(audit.exchangeActive).toBe(true);
        expect(audit.exchangePressed).toBe('true');
        expect(audit.exchangeRendererVisible).toBe(true);
        expect(audit.coulombRendererVisible).toBe(false);
        expect(audit.summary).toBe('4 active');
        expect(audit.overlayWithinViewport).toBe(true);
        expect(audit.noHorizontalOverflow).toBe(true);
        expect(audit.verticallyBounded).toBe(true);
        expect(audit.horizontallyContained).toBe(true);
        expect(audit.sectionGroups).toBe(4);

        await page.locator('#toggle-pe-force-exchange').click();
        const toggled = await page.evaluate(() => ({
            active: document.getElementById('toggle-pe-force-exchange')?.classList.contains('active'),
            pressed: document.getElementById('toggle-pe-force-exchange')?.getAttribute('aria-pressed'),
            rendererVisible: window.__ftdCtx?.viewport?._particleRenderer?._peForceExchange?.visible,
            summary: document.getElementById('pe-overlay-summary')?.textContent,
        }));
        expect(toggled.active).toBe(false);
        expect(toggled.pressed).toBe('false');
        expect(toggled.rendererVisible).toBe(false);
        expect(toggled.summary).toBe('3 active');

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('registry and conservation coverage share one native contract', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            const registry = b.peGetPhysicsRegistry();
            const retired = Array.from(registry.physics)
                .find(row => row.toggle === 'relativistic');
            const verified = Array.from(registry.physics)
                .filter(row => row.available && row.verifiedProfile)
                .map(row => row.toggle).sort();
            const physicsValidationComplete = Array.from(registry.physics)
                .every(row => !!row.validationState && !!row.validationEvidence
                    && !!row.validationCriterion);
            const physicsVerdicts = Array.from(registry.physics)
                .reduce((counts, row) => {
                    counts[row.validationState] = (counts[row.validationState] || 0) + 1;
                    return counts;
                }, {});
            const scenarioOptions = Array.from(document.getElementById('pe-scenario-select')?.options || []);

            b.peSetMode('effective_lab');
            b.peClear();
            const id = b.peAddParticle(null, 1, 0, 0, 0, 0.01, 0.02, 0.03, 1, 0.5);
            const effective = b.peGetSnapshot('browser-contract-fixture');
            b.peSetToggle('exchange', true);
            const incomplete = b.peGetSnapshot('browser-contract-fixture');
            return {
                workspaceSelectorPresent: !!document.getElementById('pe-mode-select'),
                scenarioCount: scenarioOptions.length,
                handoffButtonPresent: !!document.getElementById('btn-scale-up'),
                retired,
                verified,
                physicsValidationComplete,
                physicsVerdicts,
                retiredControlPresent: !!document.getElementById('pe-relativistic'),
                id,
                effectiveCore: effective.core,
                effectiveObject: effective.objects[0],
                completeBefore: effective.conservation.stateEnergyComplete,
                missingAfter: incomplete.conservation.missingMask,
                driftAfter: incomplete.conservation.driftEligible,
            };
        });

        expect(state.workspaceSelectorPresent).toBe(false);
        expect(state.scenarioCount).toBe(36);
        expect(state.handoffButtonPresent).toBe(false);
        expect(state.retired.available).toBe(false);
        expect(state.retired.tier).toBe('retired');
        expect(state.retiredControlPresent).toBe(false);
        expect(state.verified).toEqual(['coulomb', 'relativistic_verlet']);
        expect(state.physicsValidationComplete).toBe(true);
        expect(state.physicsVerdicts).toEqual({
            contract_qualified: 2,
            kernel_validated: 8,
            invalid_retired: 1,
            conditional_evidence: 1,
        });
        expect(state.id).toBeGreaterThanOrEqual(0);
        expect(state.effectiveCore.mode).toBe('effective_lab');
        expect(state.effectiveCore.dynamicsOwner).toBe('particle_engine');
        expect(state.effectiveObject.provenance.sourceScale).toBe(1);
        expect(state.effectiveObject.provenance.status).toBe('imposed');
        expect(state.completeBefore).toBe(true);
        expect(state.missingAfter).toBeGreaterThan(0);
        expect(state.driftAfter).toBe(false);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('canonical scenario manifest and migrated scenarios retain their owners', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        const cases = [
            ['s1-native-m3-replay', 'native_matter', 2],
            ['s1-charge-sign-matrix', 'effective_lab', 8],
            ['s1-coulomb-orbit', 'effective_lab', 2],
            ['s1-open-terminal-battery', 'effective_lab', 18],
            ['s1-finite-port-field-battery', 'effective_lab', 2],
            ['s1-cluster-pair', 'effective_lab', 2],
            ['s1-rutherford-scattering', 'effective_lab', 2],
            ['s1-force-decomposition', 'effective_lab', 2],
            ['s1-three-body', 'effective_lab', 3],
            ['s1-relativistic-integrator', 'effective_lab', 1],
            ['s1-damping-sink', 'effective_lab', 1],
            ['s1-contact-selection', 'effective_lab', 4],
            ['s1-advanced-force-isolation', 'effective_lab', 2],
            ['s1-incomplete-conservation', 'effective_lab', 2],
            ['s1-quantum-exchange-eligible', 'effective_lab', 2],
            ['s1-quantum-exchange-spinless-control', 'effective_lab', 2],
            ['s1-quantum-exchange-range', 'effective_lab', 4],
            ['s1-quantum-spin-orbit-parallel', 'effective_lab', 2],
            ['s1-quantum-spin-orbit-antiparallel', 'effective_lab', 2],
            ['s1-quantum-dipole-antiparallel', 'effective_lab', 2],
            ['s1-quantum-dipole-transverse', 'effective_lab', 2],
            ['s1-quantum-lorentz-charge-control', 'effective_lab', 3],
            ['s1-quantum-lorentz-velocity-control', 'effective_lab', 3],
            ['s1-quantum-radiation-scattering', 'effective_lab', 2],
            ['s1-quantum-relativistic-counterstream', 'effective_lab', 2],
            ['s1-quantum-color-triplet', 'effective_lab', 3],
            ['s1-qed-static-coulomb', 'effective_lab', 2],
            ['s1-qed-moller-reference', 'effective_lab', 2],
            ['s1-qed-bhabha-reference', 'effective_lab', 2],
            ['s1-qed-magnetic-dipole', 'effective_lab', 2],
            ['s1-qed-lorentz-dipole', 'effective_lab', 2],
            ['s1-qed-spin-orbit', 'effective_lab', 2],
            ['s1-qed-radiation-reaction', 'effective_lab', 2],
            ['s1-empty-zoo', 'catalog_reference', 0],
            ['s1-parametric-species', 'catalog_reference', 4],
            ['s1-mass-ladder', 'catalog_reference', 4],
        ];
        const manifest = await page.evaluate(() => {
            const rows = Array.from(window._ftdBridge?.peGetPhysicsRegistry?.()?.scenarios || []);
            return {
                count: rows.length,
                ids: rows.map(row => row.id),
                available: rows.filter(row => row.available).length,
                unavailableReasonsComplete: rows
                    .filter(row => !row.available)
                    .every(row => !!row.unavailableReason),
                validationRecordsComplete: rows.every(row =>
                    !!row.validationState && !!row.validationEvidence
                    && !!row.validationCriterion),
                unavailableProfilesInert: rows
                    .filter(row => !row.available)
                    .every(row => row.physicsMask === 0),
                behaviorComplete: rows.every(row => !!row.behavior),
            };
        });
        expect(manifest.count).toBe(36);
        expect(new Set(manifest.ids).size).toBe(36);
        expect(manifest.available).toBe(36);
        expect(manifest.unavailableReasonsComplete).toBe(true);
        expect(manifest.validationRecordsComplete).toBe(true);
        expect(manifest.unavailableProfilesInert).toBe(true);
        expect(manifest.behaviorComplete).toBe(true);
        expect(manifest.ids).not.toContain('s1-promoted-lattice');
        expect(manifest.ids).not.toContain('s1-voxel-debug');
        expect(manifest.ids).not.toContain('s1-scale2-handoff');
        for (const retiredSubview of [
            's1-constituent-graph', 's1-field-decomposition',
            's1-center-observers', 's1-identity-margins', 's1-coverage-ledger',
        ]) expect(manifest.ids).not.toContain(retiredSubview);
        for (const [id] of cases) expect(manifest.ids).toContain(id);

        const singleSelector = await page.evaluate(() => {
            const options = Array.from(document.getElementById('pe-scenario-select')?.options || []);
            const values = options.map(option => option.value);
            return {
                count: options.length,
                runnable: options.filter(option => !option.disabled).length,
                disabled: options.filter(option => option.disabled).length,
                quantumCount: values.filter(value => value.startsWith('s1-quantum-')).length,
                qedCount: values.filter(value => value.startsWith('s1-qed-')).length,
                groups: document.querySelectorAll('#pe-scenario-select optgroup').length,
                tooltipCoverage: options.every(option =>
                    option.title.includes('Observe:')
                    && option.title.includes('Boundary:')
                    && option.dataset.uiTooltip === option.title),
            };
        });
        expect(singleSelector).toEqual({
            count: 36,
            runnable: 36,
            disabled: 0,
            quantumCount: 12,
            qedCount: 7,
            groups: expect.any(Number),
            tooltipCoverage: true,
        });
        expect(singleSelector.groups).toBeGreaterThan(1);

        await selectPEScenario(page, 's1-qed-static-coulomb');
        await page.locator('#pe-scenario-select').hover();
        const scenarioTooltip = await page.evaluate(() => {
            const select = document.getElementById('pe-scenario-select');
            const row = Array.from(window._ftdBridge?.peGetPhysicsRegistry?.()?.scenarios || [])
                .find(spec => spec.id === select?.value);
            const overlay = document.getElementById('ui-tooltip');
            const text = select?.dataset.uiTooltip || '';
            return {
                visible: overlay?.hidden === false,
                overlayText: overlay?.textContent || '',
                selectedText: text,
                matchesRegistry: !!row
                    && text.includes(row.label)
                    && text.includes(row.summary)
                    && text.includes(row.owner)
                    && text.includes(row.expectedObservable)
                    && text.includes(row.prohibitedClaim)
                    && text.includes(row.validationState.replaceAll('_', ' ').toUpperCase()),
            };
        });
        expect(scenarioTooltip.visible).toBe(true);
        expect(scenarioTooltip.overlayText).toContain('Observe:');
        expect(scenarioTooltip.overlayText).toContain('Boundary:');
        expect(scenarioTooltip.overlayText).toBe(scenarioTooltip.selectedText);
        expect(scenarioTooltip.matchesRegistry).toBe(true);

        for (const [scenario, mode, count] of cases) {
            await selectPEScenario(page, scenario);
            await expect.poll(() => page.evaluate(() => ({
                mode: window._ftdBridge?.peGetSnapshot?.(
                    document.getElementById('pe-scenario-select')?.value)?.core?.mode,
                count: window._ftdBridge?.peGetParticleData?.()?.count ?? -1,
            }))).toEqual({ mode, count });

            const qualification = await page.evaluate((scenarioId) => {
                const bridge = window._ftdBridge;
                const registry = bridge.peGetPhysicsRegistry();
                const row = Array.from(registry.scenarios || [])
                    .find(spec => spec.id === scenarioId);
                const mismatches = [];
                if (row.mode !== 'native_matter') {
                    Array.from(registry.physics || []).forEach((spec, index) => {
                        const expected = !!(row.physicsMask & (1 << index)) && !!spec.available;
                        const actual = !!bridge.peGetToggle(spec.toggle);
                        if (actual !== expected) mismatches.push(`${spec.toggle}:${actual}/${expected}`);
                    });
                }
                const before = bridge.peGetDiagnostics();
                for (let i = 0; i < 4; i++) bridge.peTick();
                const after = bridge.peGetDiagnostics();
                const numeric = [after.totalEnergy, after.totalKE, after.totalPE,
                    after.particleCount, after.tick].filter(value => value !== undefined);
                return {
                    validationState: row.validationState,
                    evidence: row.validationEvidence,
                    criterion: row.validationCriterion,
                    mismatches,
                    finite: numeric.every(Number.isFinite),
                    immutableNative: row.mode !== 'native_matter'
                        || after.tick === before.tick,
                    advancesWhenInteractive: row.mode === 'native_matter'
                        || !row.interactive || after.tick >= before.tick + 4,
                };
            }, scenario);
            expect(qualification.validationState).not.toBe('open_blocked');
            expect(qualification.evidence).not.toBe('');
            expect(qualification.criterion).not.toBe('');
            expect(qualification.mismatches).toEqual([]);
            expect(qualification.finite).toBe(true);
            expect(qualification.immutableNative).toBe(true);
            expect(qualification.advancesWhenInteractive).toBe(true);
        }

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('quantum reference scenarios exercise every implemented control and null', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        const registry = await page.evaluate(() => {
            const rows = Array.from(window._ftdBridge?.peGetPhysicsRegistry?.()?.scenarios || [])
                .filter(row => row.workspace === 'quantum_reference');
            return {
                count: rows.length,
                available: rows.filter(row => row.available).length,
                imposed: rows.every(row => row.status === 'imposed'),
                boundaries: rows.every(row => !!row.prohibitedClaim),
            };
        });
        expect(registry).toEqual({ count: 12, available: 12, imposed: true, boundaries: true });

        await selectPEScenario(page, 's1-quantum-exchange-eligible');
        const exchangeEligible = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const force = bridge.peGetForceDecomposition();
            const data = bridge.peGetParticleData();
            const snapshot = bridge.peGetSnapshot('s1-quantum-exchange-eligible');
            return {
                max: force.maxExchange,
                spins: Array.from(data.spins),
                complete: snapshot.conservation.stateEnergyComplete,
                contract: document.getElementById('pe-scenario-contract')?.textContent || '',
            };
        });
        expect(exchangeEligible.max).toBeGreaterThan(0);
        expect(exchangeEligible.spins).toEqual([1, 1]);
        expect(exchangeEligible.complete).toBe(false);
        expect(exchangeEligible.contract).toMatch(/not Pauli exclusion/i);

        await selectPEScenario(page, 's1-quantum-exchange-spinless-control');
        const exchangeNull = await page.evaluate(() => ({
            max: window._ftdBridge.peGetForceDecomposition().maxExchange,
            spins: Array.from(window._ftdBridge.peGetParticleData().spins),
        }));
        expect(exchangeNull.max).toBe(0);
        expect(exchangeNull.spins).toEqual([0, 0]);

        await selectPEScenario(page, 's1-quantum-exchange-range');
        const exchangeRange = await page.evaluate(() => {
            const terms = window._ftdBridge.peGetForceDecomposition().exchange;
            const mag = index => Math.hypot(
                terms[index * 3], terms[index * 3 + 1], terms[index * 3 + 2],
            );
            return { near: Math.max(mag(0), mag(1)), far: Math.max(mag(2), mag(3)) };
        });
        expect(exchangeRange.near).toBeGreaterThan(exchangeRange.far * 5);

        await selectPEScenario(page, 's1-quantum-spin-orbit-parallel');
        const spinParallel = await page.evaluate(() => {
            const terms = window._ftdBridge.peGetForceDecomposition();
            return { x: terms.spin_orbit[3], max: terms.maxSpinOrbit };
        });
        await selectPEScenario(page, 's1-quantum-spin-orbit-antiparallel');
        const spinAntiparallel = await page.evaluate(() => {
            const terms = window._ftdBridge.peGetForceDecomposition();
            return { x: terms.spin_orbit[3], max: terms.maxSpinOrbit };
        });
        expect(spinParallel.max).toBeGreaterThan(0);
        expect(spinAntiparallel.max).toBeGreaterThan(0);
        expect(spinParallel.x * spinAntiparallel.x).toBeLessThan(0);

        for (const scenario of [
            's1-quantum-dipole-antiparallel',
            's1-quantum-dipole-transverse',
        ]) {
            await selectPEScenario(page, scenario);
            await expect.poll(() => page.evaluate(() =>
                window._ftdBridge.peGetForceDecomposition().maxMagneticDipole))
                .toBeGreaterThan(0);
        }

        for (const scenario of [
            's1-quantum-lorentz-charge-control',
            's1-quantum-lorentz-velocity-control',
        ]) {
            await selectPEScenario(page, scenario);
            const response = await page.evaluate(() => {
                const terms = window._ftdBridge.peGetForceDecomposition();
                const a = terms.lorentz.slice(3, 6);
                const b = terms.lorentz.slice(6, 9);
                return {
                    max: terms.maxLorentz,
                    dot: a[0] * b[0] + a[1] * b[1] + a[2] * b[2],
                };
            });
            expect(response.max).toBeGreaterThan(0);
            expect(response.dot).toBeLessThan(0);
        }

        await selectPEScenario(page, 's1-quantum-radiation-scattering');
        const radiation = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            for (let i = 0; i < 3; i++) bridge.peTick();
            const force = bridge.peGetForceDecomposition();
            const snapshot = bridge.peGetSnapshot('s1-quantum-radiation-scattering');
            return {
                max: force.maxRadiation,
                missing: snapshot.conservation.missingMask,
                drift: snapshot.conservation.driftEligible,
            };
        });
        expect(radiation.max).toBeGreaterThan(0);
        expect(radiation.missing).toBeGreaterThan(0);
        expect(radiation.drift).toBe(false);

        await selectPEScenario(page, 's1-quantum-relativistic-counterstream');
        const counterstream = await page.evaluate(() => ({
            velocities: Array.from(window._ftdBridge.peGetParticleData().velocities),
            maxNet: window._ftdBridge.peGetForceDecomposition().maxNet,
            relativistic: window._ftdBridge.peGetToggle('relativistic_verlet'),
        }));
        expect(counterstream.relativistic).toBe(true);
        expect(counterstream.velocities[0] * counterstream.velocities[3]).toBeLessThan(0);
        expect(counterstream.maxNet).toBe(0);

        await selectPEScenario(page, 's1-quantum-color-triplet');
        const color = await page.evaluate(() => ({
            colors: Array.from(window._ftdBridge.peGetParticleData().colorIds),
            max: window._ftdBridge.peGetForceDecomposition().maxStrong,
            contract: document.getElementById('pe-scenario-contract')?.textContent || '',
        }));
        expect(color.colors).toEqual([1, 2, 3]);
        expect(color.max).toBeGreaterThan(0);
        expect(color.contract).toMatch(/not QCD/i);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('QED scenario group contains only executable effective sectors', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        const registry = await page.evaluate(() => {
            const rows = Array.from(window._ftdBridge?.peGetPhysicsRegistry?.()?.scenarios || [])
                .filter(row => row.workspace === 'qed_reference');
            return {
                count: rows.length,
                runnable: rows.filter(row => row.available).length,
                disabled: rows.filter(row => !row.available).length,
            };
        });
        expect(registry.count).toBe(7);
        expect(registry.runnable).toBe(7);
        expect(registry.disabled).toBe(0);

        await selectPEScenario(page, 's1-qed-magnetic-dipole');
        await expect.poll(() => page.evaluate(() =>
            window._ftdBridge?.peGetForceDecomposition?.()?.maxMagneticDipole || 0))
            .toBeGreaterThan(0);
        const magnetic = await page.evaluate(() => ({
            spins: Array.from(window._ftdBridge.peGetParticleData().spinAxes),
            contract: document.getElementById('pe-scenario-contract')?.textContent || '',
            complete: window._ftdBridge.peGetSnapshot('s1-qed-magnetic-dipole')
                ?.conservation?.stateEnergyComplete,
        }));
        expect(magnetic.spins.some(value => Math.abs(value) > 0)).toBe(true);
        expect(magnetic.contract).toMatch(/Magnetic dipole sector/i);
        expect(magnetic.contract).toMatch(/not a derived Pauli\/QED magnetic moment/i);
        expect(magnetic.complete).toBe(false);

        await selectPEScenario(page, 's1-qed-lorentz-dipole');
        const lorentz = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const before = bridge.peGetForceDecomposition();
            bridge.peTick();
            return {
                coulomb: bridge.peGetToggle('coulomb'),
                lorentz: bridge.peGetToggle('lorentz'),
                maxNet: before.maxNet,
                complete: bridge.peGetSnapshot('s1-qed-lorentz-dipole')
                    .conservation.stateEnergyComplete,
            };
        });
        expect(lorentz).toMatchObject({ coulomb: false, lorentz: true, complete: false });
        expect(lorentz.maxNet).toBeGreaterThan(0);

        await selectPEScenario(page, 's1-qed-spin-orbit');
        await expect.poll(() => page.evaluate(() =>
            window._ftdBridge?.peGetForceDecomposition?.()?.maxSpinOrbit || 0))
            .toBeGreaterThan(0);

        await selectPEScenario(page, 's1-qed-radiation-reaction');
        const radiation = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            for (let i = 0; i < 3; i++) bridge.peTick();
            const snapshot = bridge.peGetSnapshot('s1-qed-radiation-reaction');
            return {
                radiation: bridge.peGetToggle('radiation'),
                missingMask: snapshot.conservation.missingMask,
                driftEligible: snapshot.conservation.driftEligible,
                count: snapshot.objects.length,
            };
        });
        expect(radiation).toMatchObject({
            radiation: true, driftEligible: false, count: 2,
        });
        expect(radiation.missingMask).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('cluster pair carries the N·K_B mass law and ±N charges', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-cluster-pair');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'cluster pair did not seed' },
        ).toBe(2);

        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            for (let i = 0; i < 30; i++) b.peTick();
            const diag = b.peGetDiagnostics();
            return {
                masses: Array.from(data.masses),
                charges: Array.from(data.charges),
                totalEnergy: diag.totalEnergy,
                tick: diag.tick,
            };
        });

        const K_B = 0.511;
        expect(state.masses[0]).toBeCloseTo(20 * K_B, 6);
        expect(state.masses[1]).toBeCloseTo(20 * K_B, 6);
        expect(state.charges.slice().sort((a, b) => a - b)).toEqual([-20, 20]);
        expect(state.totalEnergy).toBeLessThan(0);   // bound binary
        expect(state.tick).toBe(30);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('empty-zoo scenario is genuinely empty and Zoo injection works', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-empty-zoo');

        const result = await page.evaluate(() => {
            const b = window._ftdBridge;
            const before = b.peGetParticleData().count;
            const id = b.peAddParticle('electron', -1, 5, 0, 0, 0, 0, 0, 0.511, 0.1);
            const after = b.peGetParticleData().count;
            const types = b.peGetParticleTypes();
            return { before, after, id, taggedElectron: types.get(id) === 'electron' };
        });

        expect(result.before).toBe(0);
        expect(result.after).toBe(1);
        expect(result.id).toBeGreaterThanOrEqual(0);
        expect(result.taggedElectron).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('F_S force arrows use a dashed material (visible confirmation of the color-wheel fix)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-empty-zoo');
        await expect.poll(() => page.evaluate(() => !!window._ftdBridge)).toBe(true);

        await page.evaluate(() => {
            const b = window._ftdBridge;
            // Two same-color quarks so the native strong term (pairwise, requires
            // both colors nonzero) is nonzero.
            b.peAddParticle('up-quark', 2, 3, 0, 0, 0, 0, 0, 2.2, 0.3);
            b.peAddParticle('down-quark', -1, -3, 0, 0, 0, 0, 0, 4.7, 0.3);
            b.peSetStrong(true);
            document.getElementById('toggle-pe-force-strong')?.click();
        });
        await page.waitForTimeout(500);

        const isDashed = await page.evaluate(() => {
            const vp = window.__FTD_DEV__?.viewport;
            const mat = vp?._particleRenderer?._peForceStrong?.material;
            return mat?.isLineDashedMaterial === true;
        });
        expect(isDashed).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('integrator-only toggle buttons are removed from the overlay toolbar (folded into telemetry)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        const present = await page.evaluate(() => ({
            gravityBtn: !!document.getElementById('toggle-pe-gravity'),
            dampingBtn: !!document.getElementById('toggle-pe-damping'),
            gravityCheckbox: !!document.getElementById('pe-gravity'),
            dampingCheckbox: !!document.getElementById('pe-damping'),
        }));
        expect(present.gravityBtn).toBe(false);
        expect(present.dampingBtn).toBe(false);
        expect(present.gravityCheckbox).toBe(true);
        expect(present.dampingCheckbox).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
