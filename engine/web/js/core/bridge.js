/**
 * UnifiedBridge — Scale-agnostic simulation interface.
 *
 * All scale controllers talk to the bridge through this single interface.
 * The bridge holds one active scale adapter and delegates all calls uniformly,
 * eliminating the per-scale method prefixes (peTick, aeTick, etc.) that
 * previously forced app.js to do mode-dependent dispatch.
 *
 * Usage:
 *   const bridge = new UnifiedBridge(wasmModule);
 *   bridge.switchScale(1);  // Switch to particle engine
 *   bridge.tick();           // Tick whatever scale is active
 *   bridge.getDiagnostics(); // Get diagnostics from active scale
 *
 * Human coders: when adding a new scale, create a new adapter class
 * implementing the ScaleAdapter interface and register it in _createAdapter().
 */

/**
 * @typedef {Object} ScaleAdapter
 * @property {function} tick - Advance one simulation step
 * @property {function} run - Advance N steps
 * @property {function} getDiagnostics - Get current diagnostic data
 * @property {function} getEntityData - Get positions/colors/sizes for rendering
 * @property {function} setToggle - Set a named toggle
 * @property {function} getToggle - Get a named toggle
 * @property {function} loadScenario - Load a named scenario
 * @property {function} clear - Reset to empty state
 * @property {function} dispose - Clean up resources
 * @property {number} scaleLevel - Which scale this adapter represents
 */

export class UnifiedBridge {
    constructor() {
        this._active = null;      // Currently active scale adapter
        this._scaleLevel = -1;    // Current scale level
        this._rawBridge = null;   // Reference to the underlying MockBridge/WasmBridge
    }

    /**
     * Set the underlying raw bridge (MockBridge or WasmBridge from the factory).
     * Called once during app init. The raw bridge handles Scale 0/1/2 directly;
     * Scale 5 uses CosmicMockBridge; Scales 4/6 are JS-only.
     */
    setRawBridge(bridge) {
        this._rawBridge = bridge;
    }

    /** Get the raw bridge for backward compatibility during migration. */
    get raw() { return this._rawBridge; }

    /** Get current scale level. */
    get scaleLevel() { return this._scaleLevel; }

    /**
     * Switch to a different scale. Tears down the old adapter (if any)
     * and constructs a new one for the requested scale.
     */
    switchScale(level) {
        if (this._active && this._active.dispose) {
            this._active.dispose();
        }
        this._scaleLevel = level;
        // Adapter creation deferred to scale controllers
        // (they know how to initialize their specific engine)
    }
}
