// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.describe.serial('Scale 0 scientific panel mutation contract', () => {
    /** @type {import('@playwright/test').BrowserContext|undefined} */
    let context;
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
    });

    test.afterAll(async () => {
        await context?.close();
    });

    test('Wave Lab coalesces one reseed and cancels queued work on applicability loss', async () => {
        await selectScale0Scenario(page, 's0-field-rf-lattice-wave');
        const result = await page.evaluate(async () => {
            const { WaveInfoComponent } = await import(
                '/js/scales/scale0/ui/overlays/wave-lab/wave-info.js'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            const owner = store.resolveActiveScale0BridgeFromWindow();
            const first = new WaveInfoComponent();
            first.bridgeRef = owner;
            first.scenarioId = 's0-field-rf-lattice-wave';
            const before = store.getScale0QualificationState().mutationEpoch;
            first._scheduleReseed();
            first._scheduleReseed();
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const afterAccepted = store.getScale0QualificationState();
            first.unmount();

            const cancelled = new WaveInfoComponent();
            cancelled.bridgeRef = owner;
            cancelled.scenarioId = 's0-field-rf-lattice-wave';
            cancelled._scheduleReseed();
            cancelled.update(owner, 'empty');
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const afterCancelled = store.getScale0QualificationState();
            cancelled.unmount();
            return {
                before,
                acceptedEpoch: afterAccepted.mutationEpoch,
                cancelledEpoch: afterCancelled.mutationEpoch,
                mutation: afterAccepted.lastMutation,
            };
        });

        expect(result.acceptedEpoch - result.before).toBe(1);
        expect(result.cancelledEpoch).toBe(result.acceptedEpoch);
        expect(result.mutation?.reason).toBe('wave-lab-reseed');
        expect(result.mutation?.source).toBe('panel.wave-lab');
    });

    test('Thermodynamics records an accepted parameter write and rejects a stale owner', async () => {
        await selectScale0Scenario(page, 'empty');
        const result = await page.evaluate(async () => {
            const { mountThermoPanel } = await import('/js/scales/scale0/ui/overlays/thermo-panel.js');
            const store = await import('/js/scales/scale0/state/store.js');
            const host = document.createElement('div');
            document.body.appendChild(host);
            const owner = store.resolveActiveScale0BridgeFromWindow();
            const api = mountThermoPanel(host, () => owner);
            const before = store.getScale0QualificationState().mutationEpoch;
            const accepted = api.setTemp(0.02);
            const afterAccepted = store.getScale0QualificationState();
            const originalSetTemp = owner.setLangevinTemp;
            const burstCalls = [];
            owner.setLangevinTemp = (value) => burstCalls.push(value);
            const burstBefore = afterAccepted.mutationEpoch;
            const slider = api.element.querySelector('#thermo-panel-slider');
            for (let i = 0; i < 25; i++) {
                slider.value = String(i / 1000);
                slider.dispatchEvent(new Event('input', { bubbles: true }));
            }
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const afterBurst = store.getScale0QualificationState();
            const supersedeBefore = afterBurst.mutationEpoch;
            slider.value = '0.199';
            slider.dispatchEvent(new Event('input', { bubbles: true }));
            api.element.querySelector('.tp-presets button[data-t="0.07"]').click();
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const afterSupersede = store.getScale0QualificationState();
            owner.setLangevinTemp = originalSetTemp;
            api.dispose();
            host.remove();

            let staleCalls = 0;
            const staleOwner = { setLangevinTemp() { staleCalls += 1; } };
            const staleHost = document.createElement('div');
            document.body.appendChild(staleHost);
            const staleApi = mountThermoPanel(staleHost, () => staleOwner);
            const rejected = staleApi.setTemp(0.03);
            const afterRejected = store.getScale0QualificationState();
            staleApi.dispose();
            staleHost.remove();
            return {
                accepted,
                rejected,
                staleCalls,
                before,
                acceptedEpoch: afterAccepted.mutationEpoch,
                burstEpoch: afterBurst.mutationEpoch,
                burstBefore,
                burstCalls,
                supersedeBefore,
                supersedeEpoch: afterSupersede.mutationEpoch,
                rejectedEpoch: afterRejected.mutationEpoch,
                mutation: afterAccepted.lastMutation,
            };
        });

        expect(result.accepted).toBe(true);
        expect(result.acceptedEpoch - result.before).toBe(1);
        expect(result.burstEpoch - result.burstBefore).toBe(1);
        expect(result.supersedeEpoch - result.supersedeBefore).toBe(1);
        expect(result.burstCalls).toEqual([0.025, 0.07]);
        expect(result.rejected).toBe(false);
        expect(result.staleCalls).toBe(0);
        expect(result.rejectedEpoch).toBe(result.supersedeEpoch);
        expect(result.mutation?.reason).toBe('parameter-change');
        expect(result.mutation?.source).toBe('panel.thermodynamics');
    });

    test('P1 direct toggle controls each route exactly once through the active owner', async () => {
        await selectScale0Scenario(page, 's0-field-thomson-scattering');
        const result = await page.evaluate(async () => {
            const { FineStructureComponent } = await import(
                '/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js'
            );
            const { ThomsonComponent } = await import(
                '/js/scales/scale0/ui/overlays/p1-observables/thomson.js'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            const owner = store.resolveActiveScale0BridgeFromWindow();

            const fine = new FineStructureComponent();
            fine.update(owner, 's0-field-thomson-scattering');
            const fineInput = fine.element.querySelector('[data-alpha-toggle="coupling"]');
            const before = store.getScale0QualificationState().mutationEpoch;
            fineInput.checked = !fineInput.checked;
            fineInput.dispatchEvent(new Event('change', { bubbles: true }));
            const afterFine = store.getScale0QualificationState();
            fine.unmount();

            const thomson = new ThomsonComponent();
            thomson.update(owner, 's0-field-thomson-scattering');
            const fluxInput = thomson.element.querySelector('[data-thomson-flux-unlocked]');
            fluxInput.checked = !fluxInput.checked;
            fluxInput.dispatchEvent(new Event('change', { bubbles: true }));
            const afterThomson = store.getScale0QualificationState();
            thomson.unmount();
            return {
                before,
                fineEpoch: afterFine.mutationEpoch,
                fineMutation: afterFine.lastMutation,
                thomsonEpoch: afterThomson.mutationEpoch,
                thomsonMutation: afterThomson.lastMutation,
            };
        });

        expect(result.fineEpoch - result.before).toBe(1);
        expect(result.fineMutation?.source).toBe('panel.p1.fine-structure');
        expect(result.thomsonEpoch - result.fineEpoch).toBe(1);
        expect(result.thomsonMutation?.source).toBe('panel.p1.thomson');
        expect(result.thomsonMutation?.reason).toBe('physics-toggle');
    });

    test('Genesis disables unacknowledged native experiments without recording a mutation', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        const result = await page.evaluate(async () => {
            const { getPhysicsHarness } = await import('/js/physics/index.js');
            const { mountGenesisBurstPanel } = await import(
                '/js/scales/scale0/ui/overlays/genesis-burst-panel.js?v=2'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            window.__ftdGenesisBurstPanel?.dispose?.();
            const owner = store.resolveActiveScale0BridgeFromWindow();
            const prior = Object.getOwnPropertyDescriptor(owner, 'isNativeGPU');
            Object.defineProperty(owner, 'isNativeGPU', {
                value: true,
                configurable: true,
                writable: true,
            });
            try {
                const api = mountGenesisBurstPanel(getPhysicsHarness(owner));
                const before = store.getScale0QualificationState().mutationEpoch;
                const value = await api.fire(16);
                const after = store.getScale0QualificationState().mutationEpoch;
                return {
                    value,
                    before,
                    after,
                    support: api.getSupportStatus(),
                    fireDisabled: api.element.querySelector('[ref="fire"]')?.disabled,
                    sweepDisabled: api.element.querySelector('[ref="sweep"]')?.disabled,
                    status: api.element.querySelector('[ref="status"]')?.textContent || '',
                    points: api.getPoints(),
                };
            } finally {
                window.__ftdGenesisBurstPanel?.dispose?.();
                if (prior) Object.defineProperty(owner, 'isNativeGPU', prior);
                else delete owner.isNativeGPU;
            }
        });

        expect(result.value).toBeNull();
        expect(result.after).toBe(result.before);
        expect(result.support).toBe('unavailable-native-unacknowledged');
        expect(result.fireDisabled).toBe(true);
        expect(result.sweepDisabled).toBe(true);
        expect(result.status).toContain('acknowledged transaction');
        expect(result.points).toEqual([]);
    });
});
