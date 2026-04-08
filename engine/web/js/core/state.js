/**
 * Centralized Application State — Single source of truth for runtime state.
 *
 * All per-session state lives here instead of scattered module-level variables.
 * Scale controllers read/write through this shared object (passed by reference
 * via the ctx pattern, so mutations are immediately visible to all consumers).
 *
 * Human coders: add new runtime state here, not as let/var in app.js.
 * This module has zero imports — it's pure data with no dependencies.
 */

export const state = {
    // Simulation control
    running: false,
    ticksPerFrame: 1,
    engineMode: 'lattice',   // 'lattice' | 'particles' | 'atoms' | 'molecules' | 'consciousness' | 'cosmic' | 'meta'

    // Frame tracking
    frameCount: 0,
    lastFpsTime: 0,
    fpsDisplay: 0,

    // Active UI
    activeTab: 'controls',

    // DOM cache (populated once during init)
    dom: {},
};
