/**
 * @file engine/web/js/bridge/capabilities/install.js
 * @purpose Installs the lazy `bridge.capabilities` getter on MockBridge
 *          and WasmBridge prototypes so consumers see one symmetric
 *          surface (CONTRACTS.md §2). Called once at module load by
 *          wasm-bridge-dag.js.
 * @consumers wasm-bridge-dag.js (the re-export shim).
 * @contract CONTRACTS.md §2 (Capability Factory Contract).
 * @related ./scale0.js, ./scale1.js, ./scale2.js (the three factories
 *          this composes).
 *
 * Phase 2c of the refactor sweep extracted installCapabilityGetter
 * from wasm-bridge-dag.js. Body unchanged — defines a getter that
 * lazily instantiates the per-scale capability objects on first read
 * and caches them on the bridge instance.
 */

import { createScale0Capabilities } from './scale0.js';
import { createScale1Capabilities } from './scale1.js';
import { createScale2Capabilities } from './scale2.js';

export function installCapabilityGetter(proto) {
    Object.defineProperty(proto, 'capabilities', {
        configurable: true,
        get() {
            if (!this._capabilities) {
                this._capabilities = {
                    scale0: createScale0Capabilities(this),
                    scale1: createScale1Capabilities(this),
                    scale2: createScale2Capabilities(this),
                };
            }
            return this._capabilities;
        },
    });
}
