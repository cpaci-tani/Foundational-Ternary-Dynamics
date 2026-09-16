import { FiniteRecordBridge } from '../../../bridge/finite-record-bridge.js';
import { setFluxMock, getActiveScale0Bridge, completeScale0AuthoritativeLoad, failScale0AuthoritativeLoad } from '../state/store.js';
import { getWebRecordCatalog, syncRecordControls, updateRecordReadout } from '../ui/controls/record-observation.js';
import { applyScale0OverlayApplicability } from '../ui/overlays/applicability.js';

export function loadRecordScenario(ctx, state, viewportAdapter, spec, loadGeneration, size = null, visual = {}) {
    ctx.pauseSimulation?.();
    const previous = getActiveScale0Bridge(ctx, state);
    previous?.setRunning?.(false); ctx.bridge?.cancelQueuedTicks?.();
    const desired = Number(size ?? document.getElementById('lattice-size').value);
    const L = spec.sizes.includes(desired) ? desired : spec.sizes[spec.sizes.length - 1];
    const current = () => ctx._loadGeneration === loadGeneration && state.fluxMock === owner;
    let quantity = null;
    const owner = new FiniteRecordBridge(L, {
        onFrame: () => { if (!current()) return; if (quantity !== owner.quantity) { ctx.viewport.resetFluxNormalization?.(); quantity = owner.quantity; } state.latticeNeedsUpload = true; state.fieldNeedsUpdate = true; state.fieldDataVersion++; },
        onFailure: message => {
            if (!current()) return;
            ctx.pauseSimulation?.(); failScale0AuthoritativeLoad({scenarioId: spec.id, loadGeneration, reason: message});
            ctx.scale0Validity?.runtimeFailure(message); window.showToast?.(message, 'error'); updateRecordReadout(owner);
        },
    });
    owner.quantity = previous?.isFiniteRecord && previous.scenario?.id === spec.id
        ? previous.quantity : spec.observation || 'tokens';
    // Same authoritative worker slot as every other web-lattice scenario.
    // Replacement terminates the previous browser worker, including its loop.
    setFluxMock(owner, true); ctx.fluxMock = owner; ctx.useFluxMock = true;
    ctx.resetAllVisualState(); viewportAdapter.clearScaleVisuals();
    visual.restore?.({...visual.preferences, fluxSlice: false,
        overlays: {'toggle-state-field': visual.preferences?.overlays?.['toggle-state-field']}}, state, viewportAdapter);
    applyScale0OverlayApplicability(spec.id, viewportAdapter, {});
    syncRecordControls(ctx, spec); ctx.syncScale0AuthoritativeLatticeSize?.(L);
    ctx.viewport.setBoundaryShape('cube'); ctx.viewport.setBoundaryDynamics?.(0, 3);
    ctx.viewport.resetFluxNormalization?.();
    viewportAdapter.setFluxSliceVisible(false);
    ctx.viewport.camera.position.set(L * 1.5, L * 1.1, L * 1.5);
    ctx.viewport.controls.target.set(L / 2, L / 2, L / 2);
    ctx.inspectorRuntime?.setBridge(owner);
    state.latticeNeedsUpload = true; updateRecordReadout(owner);
    if (ctx.bridge?.isNativeGPU) {
        owner.fail(new Error('This registered scenario requires the local WASM connection.'));
        return owner;
    }
    void (async () => {
        const deadline = performance.now() + 10000;
        while (previous?.isWorker && previous.disposed !== true) {
            if (!current()) return;
            if (performance.now() > deadline) throw new Error('Previous lattice worker did not finish disposal');
            await new Promise(resolve => setTimeout(resolve, 20));
        }
        if (!current()) return;
        await owner.setupScenario(spec, getWebRecordCatalog());
        if (!current() || !owner.ready) return;
        ctx.pauseSimulation?.();
        completeScale0AuthoritativeLoad({scenarioId: spec.id, loadGeneration, tick: owner.currentTick(), source: 'finite-record-worker'});
        updateRecordReadout(owner);
    })().catch(error => owner.fail(error));
    return owner;
}
