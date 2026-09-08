import { expect } from '@playwright/test';

// Preregistered UI preparation for the Damping display workload. The caller
// loads s0-seed-sloop through the ordinary scenario selector first. This is a
// modified, qualification-suspended profile, not an admitted scenario result.
export async function readDampingPerformanceSnapshot() {
    const [{ getScale0State, getScale0QualificationState, isScale0AuthoritativeGenerationReady },
        { SCALE0_ENGINE_TOGGLE_NAMES }] = await Promise.all([
        import('/js/scales/scale0/state/store.js'), import('/js/bridge/wasm-bridge-proxy.js'),
    ]);
    const state = getScale0State(), ctx = window.__ftdCtx;
    const owner = state.useFluxMock ? state.fluxMock : ctx?.bridge;
    const terms = Object.fromEntries(SCALE0_ENGINE_TOGGLE_NAMES.map(key => {
        const value = owner?.isWorker ? owner.getEngineTruthToggle?.(key) : owner?.getToggle?.(key);
        return [key, typeof value === 'boolean' ? value : null];
    }));
    const geometry = ctx?.viewport?._fieldRenderer?._dampingZones;
    const tick = owner?.currentTick?.();
    return {
        terms, qualification: getScale0QualificationState(), lifecycle: owner?.lifecycleDebug ?? null,
        ready: owner?.ready !== false && isScale0AuthoritativeGenerationReady(state)
            && state.qualificationAnchor?.loadGeneration === ctx?._loadGeneration,
        scenarioId: state.currentScenarioId, loadGeneration: ctx?._loadGeneration ?? null,
        isWasm: owner?.isWasm === true, isWorker: owner?.isWorker === true,
        size: owner?.latticeSize ?? null, settled: owner?.runningStateSettled !== false,
        paused: document.getElementById('btn-play')?.dataset.paused === 'true',
        tick: Number.isSafeInteger(tick) && tick >= 0 ? tick : null,
        particleCount: owner?.getParticleData?.()?.count ?? null,
        drawCount: geometry?.geometry?.drawRange?.count ?? null,
        visible: geometry?.visible === true,
        applicable: !!document.getElementById('toggle-damping-zones')
            && !document.getElementById('toggle-damping-zones').classList.contains('is-inapplicable'),
    };
}

export async function prepareDampingPerformanceProfile(page) {
    const read = () => page.evaluate(readDampingPerformanceSnapshot);
    await expect.poll(async () => {
        const s = await read();
        return s.ready && s.isWasm && s.scenarioId === 's0-seed-sloop'
            && (!s.lifecycle || s.lifecycle.configurationToken === s.lifecycle.appliedConfigurationToken)
            && Object.values(s.terms).every(value => typeof value === 'boolean');
    }, { timeout: 90000 }).toBe(true);
    await page.evaluate(() => window.__ftdCtx.pauseSimulation());
    await expect.poll(async () => { const s = await read(); return s.paused && s.settled; }, { timeout: 30000 }).toBe(true);
    const ownerHandle = await page.evaluateHandle(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const s = getScale0State(); return s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
    });
    const assertOwner = () => page.evaluate(async original => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const s = getScale0State(), current = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
        if (current !== original) throw new Error('Damping preparation replaced the physics owner');
    }, ownerHandle);
    try {
        const baseline = await read();
        expect(baseline.qualification.status).toBe('within-contract');
        expect(Object.keys(baseline.terms), 'complete registered boolean engine profile').toHaveLength(46);
        expect(Object.values(baseline.terms).every(value => value === false), 'registered static seed profile').toBe(true);
        expect(baseline.tick).not.toBeNull();
        await expect.poll(async () => (await read()).particleCount, { timeout: 30000 }).toBe(12);
        for (const [id, key] of [['t-damping', 'damping'], ['t-selective', 'selective_damping']]) {
            await assertOwner();
            await page.evaluate(id => {
                const input = document.getElementById(id);
                if (!input || input.disabled) throw new Error(`Damping preparation control unavailable: ${id}`);
                input.checked = true;
                input.dispatchEvent(new Event('change', { bubbles: true }));
            }, id);
            await expect.poll(async () => (await read()).terms[key], { timeout: 30000 }).toBe(true);
        }
        await assertOwner();
        await page.evaluate(async () => {
            const [{ syncScale0ToggleUiFromEngine }, { createScale0ViewportAdapter }] = await Promise.all([
                import('/js/scales/scale0/runtime/scenario-loader.js'),
                import('/js/scales/scale0/viewport-adapter.js'),
            ]);
            const ctx = window.__ftdCtx;
            if (!syncScale0ToggleUiFromEngine(ctx, createScale0ViewportAdapter(ctx.viewport), 's0-seed-sloop')) {
                throw new Error('Damping preparation engine profile synchronization unavailable');
            }
        });
        const modified = await read();
        expect(modified.terms).toEqual({ ...baseline.terms, damping: true, selective_damping: true });
        expect(modified.qualification.status).toBe('suspended');
        expect(modified.qualification.mutationEpoch).toBe(baseline.qualification.mutationEpoch + 2);
        expect(modified.qualification.anchor).toEqual(baseline.qualification.anchor);
        expect(modified.loadGeneration).toBe(baseline.loadGeneration);
        expect(modified.lifecycle?.configurationToken ?? null).toBe(baseline.lifecycle?.configurationToken ?? null);
        expect(modified.lifecycle?.appliedConfigurationToken ?? null).toBe(baseline.lifecycle?.appliedConfigurationToken ?? null);
        expect(modified.tick).toBe(baseline.tick);
        expect(modified.paused && modified.settled && modified.ready && modified.applicable).toBe(true);
        expect(modified.particleCount).toBe(12);
        const profileSnapshot = s => ({ terms: s.terms, qualification: s.qualification, lifecycle: s.lifecycle });
        return { baseline: profileSnapshot(baseline), modified: profileSnapshot(modified),
            particleCount: modified.particleCount, profile: 'damping-zones-modified-v1' };
    } finally { await ownerHandle.dispose(); }
}
