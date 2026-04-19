/**
 * Cosmic scale-5 scenario dispatcher.
 *
 * Each setup function is called via `.call(bridgeInstance, ctx)` where ctx
 * supplies pre-seeded RNG + the TYPE enum. Setup functions are pure
 * data-generation: they write bodies via `this.addBody(...)` and set the
 * five scenario parameters (_boxSize, _softening, _dt, _enableSubgrid,
 * _stellarEvolution, _hawkingEvaporation).
 *
 * Unknown scenario names fall through to the cosmic-web fallback,
 * matching the pre-refactor behavior.
 */

import {
    setupCosmicGalaxy,
    setupCartwheelCollision,
    setupBinaryAGN,
    setupGlobularCluster,
    setupCosmicWebScenario,
    setupBlackHoleScenario,
    setupMerger,
    setupSuperCluster
} from './galaxies.js';

import {
    setupStellarLifecycle,
    setupFtdCollapse,
    setupDarkMatterHalo,
    setupGravitationalWave,
    setupBaryogenesis,
    setupCosmicWebFallback
} from './exotic.js';

/**
 * Dispatch a scenario by name. Call via `runCosmicScenario.call(bridge, name)`.
 * Returns nothing — scenarios mutate `this` directly.
 */
export function runCosmicScenario(name, ctx) {
    switch (name) {
        case 'cosmic-galaxy':              return setupCosmicGalaxy.call(this, ctx);
        case 'cosmic-cartwheel-collision': return setupCartwheelCollision.call(this, ctx);
        case 'cosmic-binary-agn':          return setupBinaryAGN.call(this, ctx);
        case 'cosmic-globular-cluster':    return setupGlobularCluster.call(this, ctx);
        case 'cosmic-web':                 return setupCosmicWebScenario.call(this, ctx);
        case 'cosmic-black-hole':          return setupBlackHoleScenario.call(this, ctx);
        case 'cosmic-merger':              return setupMerger.call(this, ctx);
        case 'cosmic-super-cluster':       return setupSuperCluster.call(this, ctx);
        case 'cosmic-stellar-lifecycle':   return setupStellarLifecycle.call(this, ctx);
        case 'cosmic-ftd-collapse':        return setupFtdCollapse.call(this, ctx);
        case 'cosmic-dark-matter-halo':    return setupDarkMatterHalo.call(this, ctx);
        case 'cosmic-gravitational-wave':  return setupGravitationalWave.call(this, ctx);
        case 'cosmic-baryogenesis':        return setupBaryogenesis.call(this, ctx);
        default:                           return setupCosmicWebFallback.call(this, ctx);
    }
}
