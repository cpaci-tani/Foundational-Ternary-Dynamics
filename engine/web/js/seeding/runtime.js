import { FiniteRecordBridge } from '../bridge/finite-record-bridge.js';
import { getWebRecordCatalog } from '../scales/scale0/ui/controls/record-observation.js';
import { getScale0QualificationState, subscribeScale0Qualification } from '../scales/scale0/state/store.js';
import { cloneRecipe, recipesEqual, validateRecipe } from './recipe.js';
import { loadFiniteCatalog, findFiniteSchema } from './catalog.js';

/** True only when the recipe differs structurally from its registered v2 default —
 * every v2 default already carries enabled components, so `blank || some enabled`
 * (the v1 test) would call almost every registered preset "custom". */
export async function isEditedRecordRecipe(recipe) {
    const catalog = await loadFiniteCatalog();
    const registered = findFiniteSchema(catalog, recipe.scenarioId, recipe.size);
    return !registered || !recipesEqual(recipe, registered.recipe);
}

/** Prepare a detached, paused compiled owner. The live lattice is untouched. */
export async function prepareRecordRecipe(recipe, scenarios, signal) {
    const parent = validateRecipe(recipe, scenarios);
    if (parent.backend !== 'finite-records') throw new Error('Compiled preview is currently available for record preparations');
    const custom = await isEditedRecordRecipe(recipe);
    const spec = {...parent, sizes: [recipe.size], ...(custom ? {seedRecipe: cloneRecipe(recipe)} : {})};
    let failure = null;
    const owner = new FiniteRecordBridge(recipe.size, {onFailure: message => {failure = message;}});
    const abort = () => owner.dispose();
    signal?.addEventListener('abort', abort, {once: true});
    try {
        if (signal?.aborted) throw new Error('Preparation cancelled');
        await owner.setupScenario(spec, getWebRecordCatalog());
        if (!owner.ready || owner.disposed || failure) throw new Error(failure || 'Preparation cancelled');
        owner.quantity = parent.observation || 'field_tokens';
        owner.setRecordQuantity(owner.quantity);
        return owner;
    } catch (error) { owner.dispose(); throw error; }
    finally { signal?.removeEventListener('abort', abort); }
}

/** Resolve only the requested acknowledged load; never a later unrelated run. */
export function waitForSeedLoad(ctx, generation, signal) {
    return new Promise((resolve, reject) => {
        let off = () => {}, settled = false;
        const finish = error => {
            if (settled) return; settled = true; off(); clearTimeout(timer);
            signal?.removeEventListener('abort', cancel);
            if (error) reject(error); else resolve();
        };
        const cancel = () => finish(new Error('Seed request cancelled'));
        const timer = setTimeout(() => finish(new Error('Lattice did not acknowledge the seed')), 30000);
        const check = q => {
            if (signal?.aborted || ctx._loadGeneration !== generation || ctx.engineMode !== 'lattice') return cancel();
            if (q.authoritativeLoad && q.authoritativeLoad.status !== 'pending') return finish(new Error(q.authoritativeLoad.failureReason || 'Seed load failed'));
            if (!q.authoritativeLoad && q.anchor?.loadGeneration === generation) finish();
        };
        signal?.addEventListener('abort', cancel, {once: true});
        off = subscribeScale0Qualification(check);
        if (settled) off(); else check(getScale0QualificationState());
    });
}
