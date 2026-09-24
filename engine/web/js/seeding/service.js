/** Shared, headless seed installation. Preparations never tick the live owner. */
import { getActiveScale0Bridge } from '../scales/scale0/state/store.js';
import { cloneRecipe, isCustomNativeRecipe, validateRecipe } from './recipe.js';
import { prepareRecordRecipe, waitForSeedLoad } from './runtime.js';
import { prepareNativeRecipe } from './native-runtime.js';

export async function prepareSeed(ctx, recipe, scenarios, signal) {
    const scenario = validateRecipe(recipe, scenarios);
    if (signal?.aborted) throw new Error('Seed preparation cancelled');
    if (scenario.backend === 'finite-records' && ctx.bridge?.isNativeGPU)
        throw new Error('Record preparation requires the local WASM connection');
    return scenario.backend === 'finite-records'
        ? prepareRecordRecipe(recipe, scenarios, signal) : prepareNativeRecipe(ctx, recipe, signal);
}

/** Installation is a transaction boundary. Callers supply live activity checks,
 * not DOM events. Cancellation after a native commit never disposes its owner. */
export async function applySeed(ctx, {
    recipe: input, scenarios, owner: preparedOwner = null, loadScenario,
    signal, assertCurrent = () => {}, assertActive = () => {}, onInstalling = () => {},
    prepare = prepareSeed, waitForLoad = waitForSeedLoad,
}) {
    const recipe = cloneRecipe(input), scenario = validateRecipe(recipe, scenarios);
    const generation = ctx._loadGeneration;
    let owner = preparedOwner, adopted = false, nativeCommitted = false;
    const checkActivity = () => {
        if (signal?.aborted || ctx.engineMode !== 'lattice' || ctx.presentationSuspended)
            throw new Error('Seed request cancelled or workspace inactive');
        assertActive();
    };
    const checkSource = () => {
        checkActivity();
        if (!nativeCommitted) assertCurrent();
        if (ctx._loadGeneration !== generation) throw new Error('Seed superseded by another scenario');
    };
    try {
        checkSource();
        if (scenario.backend === 'finite-records' && ctx.bridge?.isNativeGPU)
            throw new Error('Record preparation requires the local WASM connection');
        owner ||= await prepare(ctx, recipe, scenarios, signal);
        checkSource();
        ctx.pauseSimulation?.(); onInstalling(true);
        if (owner.isNativeSeedPreview) {
            owner = await owner.commit(signal);
            adopted = owner === ctx.bridge;
            nativeCommitted = true;
            // The native owner is already committed. A cancellation here must
            // suppress UI adoption/run continuations without destroying it.
            checkSource();
        }
        const custom = scenario.backend === 'finite-records'
            ? !!owner.scenario?.seedRecipe : isCustomNativeRecipe(recipe);
        const params = scenario.backend === 'finite-records'
            ? { preparedRecordOwner: owner, seedRecipe: cloneRecipe(recipe), latticeSize: recipe.size }
            : { preparedNativeOwner: owner, seedRecipe: cloneRecipe(recipe), customSeed: custom, latticeSize: recipe.size };
        // Last synchronous cancellation/activity fence before replacing owner.
        checkSource();
        loadScenario(recipe.scenarioId, params);
        adopted = getActiveScale0Bridge(ctx) === owner;
        const installedGeneration = ctx._loadGeneration;
        onInstalling(false);
        await waitForLoad(ctx, installedGeneration, signal);
        if (signal?.aborted || ctx.presentationSuspended || ctx.engineMode !== 'lattice'
            || ctx._loadGeneration !== installedGeneration || getActiveScale0Bridge(ctx) !== owner)
            throw new Error('Seed acknowledgement superseded');
        ctx._appliedSeedRecipe = cloneRecipe(recipe);
        ctx._appliedSeedRecipeCustom = custom;
        return { owner, recipe, custom, installedGeneration };
    } finally {
        onInstalling(false);
        if (!adopted) owner?.dispose();
    }
}
