// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.describe('Scale 0 scenario toolbar and epistemic disclosure audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    });

    test('menu exactly mirrors the admitted registry and repeat population does zero DOM work', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const registry = await import('/js/scales/scale0/scenario-registry.js');
            const select = document.getElementById('scenario-select');
            const expectedGroups = [];
            for (const scenario of registry.SCALE0_SCENARIOS) {
                let group = expectedGroups.find((entry) => entry.label === scenario.category);
                if (!group) {
                    group = { label: scenario.category, ids: [] };
                    expectedGroups.push(group);
                }
                group.ids.push(scenario.id);
            }
            const actualGroups = [...select.querySelectorAll('optgroup')].map((group) => ({
                label: group.label,
                ids: [...group.querySelectorAll('option')].map((option) => option.value),
            }));

            const records = [];
            const observer = new MutationObserver((batch) => records.push(...batch));
            observer.observe(select, { attributes: true, childList: true, subtree: true });
            registry.populateScale0ScenarioSelect(select, select.value);
            await Promise.resolve();
            observer.disconnect();

            const ids = [...select.options].map((option) => option.value);
            return {
                expectedGroups,
                actualGroups,
                ids,
                registryCount: registry.SCALE0_SCENARIOS.length,
                uniqueCount: new Set(ids).size,
                mutationRecords: records.length,
                validation: registry.validateScale0ScenarioRegistry(),
            };
        });

        expect(result.actualGroups).toEqual(result.expectedGroups);
        expect(result.ids).toHaveLength(result.registryCount);
        expect(result.uniqueCount).toBe(result.registryCount);
        expect(result.mutationRecords).toBe(0);
        expect(result.validation).toEqual({ ok: true, errors: [], count: result.registryCount });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('every admitted option renders its exact canonical epistemic status', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const defects = await page.evaluate(async () => {
            const registry = await import('/js/scales/scale0/scenario-registry.js');
            const { updateScenarioMetadata } = await import('/js/scales/scale0/ui/bindings.js');
            const failures = [];
            for (const scenario of registry.SCALE0_SCENARIOS) {
                updateScenarioMetadata(scenario.id);
                const wrap = document.getElementById('lat-scenario-desc');
                const text = document.getElementById('lat-scenario-desc-text')?.textContent || '';
                if (getComputedStyle(wrap).display === 'none') failures.push(`${scenario.id}: hidden`);
                if (wrap.open) failures.push(`${scenario.id}: unexpectedly open`);
                if (!text.includes('REGISTERED EPISTEMIC STATUS')) {
                    failures.push(`${scenario.id}: status heading missing`);
                }
                if (!text.includes(scenario.epistemicStatus)) {
                    failures.push(`${scenario.id}: exact registry status missing`);
                }
                if (!text.includes(`Evidence level: ${scenario.evidenceLevel}`)) {
                    failures.push(`${scenario.id}: evidence level missing`);
                }
            }

            const current = registry.getScale0Scenario(
                document.getElementById('scenario-select')?.value || 'flux-pulse',
            );
            updateScenarioMetadata(current.id, { profileModified: true });
            const modifiedText = document.getElementById('lat-scenario-desc-text')?.textContent || '';
            if (!modifiedText.includes('MODIFIED PHYSICS PROFILE — QUALIFICATION SUSPENDED')) {
                failures.push(`${current.id}: modified-profile warning missing`);
            }
            if (!modifiedText.includes(current.epistemicStatus)) {
                failures.push(`${current.id}: modified profile hid canonical status`);
            }
            return failures;
        });

        expect(defects).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('ten rapid selections dispatch once each, conserve workers, and commit only the latest scenario', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        test.skip(!(await page.evaluate(() => globalThis.crossOriginIsolated === true)),
            'requires the COOP/COEP Scale 0 worker path');

        await page.waitForFunction(async () => {
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const lifecycle = owner?.lifecycleDebug;
            const qualification = getScale0QualificationState();
            return owner?.ready === true
                && !!lifecycle?.workerRuntimeId
                && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                && qualification.status === 'within-contract';
        });

        const result = await page.evaluate(async () => {
            const registry = await import('/js/scales/scale0/scenario-registry.js');
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const select = document.getElementById('scenario-select');
            const ctx = window.__ftdCtx;
            const initialId = select.value;
            const candidates = registry.SCALE0_SCENARIOS
                .map((scenario) => scenario.id)
                .filter((id) => id !== initialId)
                .slice(0, 9);
            const ids = [...candidates, initialId];
            const baselineDeadline = performance.now() + 20_000;
            let baselineState;
            while (performance.now() < baselineDeadline) {
                baselineState = getScale0State();
                const lifecycle = baselineState.fluxMock?.lifecycleDebug;
                const qualification = getScale0QualificationState();
                if (baselineState.fluxMock?.ready === true
                    && !!lifecycle?.workerRuntimeId
                    && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                    && qualification.status === 'within-contract') break;
                await new Promise((resolve) => setTimeout(resolve, 50));
            }
            const beforeWorkers = window.__ftdWasmWorkers();
            const beforeLifecycle = baselineState?.fluxMock?.lifecycleDebug ?? null;
            const beforeGeneration = ctx._loadGeneration || 0;
            const originalPause = ctx.pauseSimulation;
            let pauseCalls = 0;
            ctx.pauseSimulation = function (...args) {
                pauseCalls += 1;
                return originalPause.apply(this, args);
            };
            try {
                for (const id of ids) {
                    select.value = id;
                    select.dispatchEvent(new Event('change', { bubbles: true }));
                }
            } finally {
                ctx.pauseSimulation = originalPause;
            }

            const finalId = ids.at(-1);
            const deadline = performance.now() + 20_000;
            while (performance.now() < deadline) {
                const state = getScale0State();
                const lifecycle = state.fluxMock?.lifecycleDebug;
                const qualification = getScale0QualificationState();
                if (state.currentScenarioId === finalId
                    && state.fluxMock?.ready === true
                    && state.fluxMock?._scenarioId === finalId
                    && lifecycle?.appliedConfigurationToken === lifecycle?.configurationToken
                    && qualification.status === 'within-contract'
                    && qualification.anchor?.scenarioId === finalId
                    && qualification.anchor?.loadGeneration === ctx._loadGeneration) break;
                await new Promise((resolve) => setTimeout(resolve, 50));
            }

            const state = getScale0State();
            const qualification = getScale0QualificationState();
            const finalScenario = registry.getScale0Scenario(finalId);
            return {
                inputCount: ids.length,
                pauseCalls,
                generationDelta: (ctx._loadGeneration || 0) - beforeGeneration,
                finalGeneration: ctx._loadGeneration || 0,
                workersBefore: beforeWorkers,
                workersAfter: window.__ftdWasmWorkers(),
                lifecycleBefore: beforeLifecycle,
                lifecycleAfter: state.fluxMock?.lifecycleDebug ?? null,
                qualification,
                finalId,
                selectedId: select.value,
                stateId: state.currentScenarioId,
                ownerId: state.fluxMock?._scenarioId,
                ownerReady: state.fluxMock?.ready === true,
                running: ctx.running,
                metadata: document.getElementById('lat-scenario-desc-text')?.textContent || '',
                expectedStatus: finalScenario.epistemicStatus,
            };
        });

        expect(result.pauseCalls).toBe(result.inputCount);
        expect(result.generationDelta).toBe(result.inputCount);
        expect(result.workersAfter.created - result.workersBefore.created).toBe(0);
        expect(result.workersAfter.terminated - result.workersBefore.terminated).toBe(0);
        expect(result.workersAfter.created).toBe(
            result.workersAfter.terminated + result.workersAfter.live,
        );
        expect(result.workersAfter.live).toBe(1);
        expect(result.lifecycleBefore.workerRuntimeId).toBeTruthy();
        expect(result.lifecycleAfter.workerRuntimeId).toBe(result.lifecycleBefore.workerRuntimeId);
        expect(result.lifecycleAfter.moduleInitCount).toBe(1);
        expect(result.lifecycleAfter.renderBridgeGeneration
            - result.lifecycleBefore.renderBridgeGeneration).toBe(1);
        expect(result.lifecycleAfter.appliedConfigurationToken)
            .toBe(result.lifecycleAfter.configurationToken);
        expect(result.qualification.status).toBe('within-contract');
        expect(result.qualification.anchor.scenarioId).toBe(result.finalId);
        expect(result.qualification.anchor.loadGeneration).toBe(result.finalGeneration);
        expect(result.qualification.anchor.source).toBe('worker-configuration-applied');
        expect(result.selectedId).toBe(result.finalId);
        expect(result.stateId).toBe(result.finalId);
        expect(result.ownerId).toBe(result.finalId);
        expect(result.ownerReady).toBe(true);
        expect(result.running).toBe(false);
        expect(result.metadata).toContain(result.expectedStatus);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('selection and disclosure survive a Scale 0 exit and re-entry without duplicate controls', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const target = 'flux-dipole';
        await page.evaluate((id) => {
            const select = document.getElementById('scenario-select');
            select.value = id;
            select.dispatchEvent(new Event('change', { bubbles: true }));
            document.getElementById('lat-scenario-desc').open = true;
        }, target);

        await switchMode(page, 'particles');
        await expect(page.locator('#lattice-controls')).toBeHidden();
        await switchMode(page, 'lattice');
        await expect(page.locator('#lattice-controls')).toBeVisible();

        const result = await page.evaluate(async (id) => {
            const registry = await import('/js/scales/scale0/scenario-registry.js');
            const status = registry.getScale0Scenario(id).epistemicStatus;
            return {
                selected: document.getElementById('scenario-select')?.value,
                controls: document.querySelectorAll('#lattice-controls').length,
                selects: document.querySelectorAll('#scenario-select').length,
                optionCount: document.querySelectorAll('#scenario-select option').length,
                registryCount: registry.SCALE0_SCENARIOS.length,
                metadata: document.getElementById('lat-scenario-desc-text')?.textContent || '',
                status,
            };
        }, target);

        expect(result.selected).toBe(target);
        expect(result.controls).toBe(1);
        expect(result.selects).toBe(1);
        expect(result.optionCount).toBe(result.registryCount);
        expect(result.metadata).toContain(result.status);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
