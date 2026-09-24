/** Shared button/keyboard actions; ctx is the live application state owner. */
export function createPlaybackActions(ctx, controllers, loaders) {
    const selected = (id, fallback) => document.getElementById(id)?.value || fallback;
    return {
        ...loaders,
        step() {
            ctx.pauseSimulation();
            switch (ctx.engineMode) {
                case 'atoms': case 'molecules': ctx.bridge.aeTick(); break;
                case 'particles': ctx.bridge.peTick(); break;
                case 'planetary': controllers.planetary.step(); break;
                case 'cosmic': controllers.cosmic.step(ctx); break;
                case 'meta': break; // Step is disabled: this view has no tick law.
                default: controllers.lattice.step(ctx);
            }
        },
        reset() {
            ctx.pauseSimulation();
            switch (ctx.engineMode) {
                case 'particles': loaders.loadPEScenario(selected('pe-scenario-select')); break;
                case 'atoms': loaders.loadAEScenario(selected('ae-scenario-select')); break;
                case 'molecules': loaders.loadMoleculeScenario(selected('mol-scenario-select')); break;
                case 'planetary': controllers.planetary.loadScenario(ctx, selected('planetary-scenario-select', 'planetary-solar')); break;
                case 'cosmic': controllers.cosmic.loadCosmicScenario(ctx, selected('cosmic-scenario-select', 'cosmic-galaxy')); break;
                case 'meta': controllers.meta.loadScenario(ctx); break;
                default: controllers.lattice.reset(ctx);
            }
        },
    };
}
