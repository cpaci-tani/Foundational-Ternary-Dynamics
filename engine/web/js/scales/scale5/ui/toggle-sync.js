/**
 * Shared Scale-5 toggle-checkbox binding.
 *
 * Extracted from scales/scale5/ui/toolbar/component.js (Pass B, Ruling J2):
 * `sph_monaghan` became the first SCALE5_TOGGLES key with TWO UI surfaces —
 * the scenario toolbar's `#t-sph-monaghan` checkbox (Pass 0a) and the new
 * Gas controls-card `#cosmic-gas-sph` checkbox (Pass B) — and Pass D's
 * eleven remaining checkboxes will need the identical shape, so this is
 * built once here as shared foundation rather than re-solved per surface.
 *
 * Any UI surface (a toolbar group, a controls-panel card, …) registers its
 * DOM root via `bindScale5ToggleCheckboxes(root)`. Every checkbox under that
 * root that corresponds to a SCALE5_TOGGLES key — matched by the registry's
 * own `domId` (the toolbar's original convention) OR by a
 * `data-scale5-toggle="<key>"` attribute (for any surface that needs its
 * own distinct DOM id, since HTML ids must be unique per document) — gets a
 * 'change' listener that writes to whichever bridge is currently active
 * (`syncScale5Toggles`) and immediately reconciles the SAME key's checkbox
 * in every OTHER registered root, so two checkboxes for one key can never
 * drift apart.
 *
 * Rejected alternative (Ruling J2): a per-frame read-back guarded by
 * `document.activeElement`. That polls every frame with no precedent
 * anywhere else in the codebase — a signal it was the wrong shape. This
 * module instead reconciles synchronously on 'change': no polling, and
 * `_activeBridge` remains the one source of truth for which bridge a
 * checkbox drives (mirroring the toolbar's original `_activeBridge`
 * rationale: the DOM persists across scenario loads while the
 * CosmicMockBridge instance is recreated on every load, so
 * `syncScale5Toggles(bridge)` — called by scale5/controller.js right after
 * it builds a fresh bridge, and with `null` on destroy — both re-targets
 * every registered root's listeners and refreshes their checkboxes to that
 * bridge's current state).
 */
import { SCALE5_TOGGLES } from '../../../config/toggles.js';

let _activeBridge = null;
const _roots = new Set();

function findCheckbox(root, key, domId) {
    return root.querySelector(`[data-scale5-toggle="${key}"]`)
        || (domId ? root.querySelector('#' + domId) : null);
}

function reconcileOthers(key, domId, checked, exceptRoot) {
    for (const root of _roots) {
        if (root === exceptRoot) continue;
        const input = findCheckbox(root, key, domId);
        if (input) input.checked = checked;
    }
}

/**
 * Bind every SCALE5_TOGGLES checkbox present under `root` so a 'change'
 * event writes to the active bridge and reconciles the same key's checkbox
 * in every OTHER registered root. Registers `root` for future
 * `syncScale5Toggles()` calls. Idempotent per root — safe to call again
 * with the same root (e.g. a controls card re-mounted without a teardown).
 *
 * @param {Element|null} root
 */
export function bindScale5ToggleCheckboxes(root) {
    if (!root || _roots.has(root)) return;
    _roots.add(root);
    for (const [key, , domId] of SCALE5_TOGGLES) {
        const input = findCheckbox(root, key, domId);
        if (!input) continue;
        input.addEventListener('change', () => {
            _activeBridge?.setToggle?.(key, input.checked);
            reconcileOthers(key, domId, input.checked, root);
        });
    }
}

/**
 * Reflect `bridge`'s current toggle state onto every registered root's
 * checkboxes and make it the target of their 'change' listeners. Call once
 * a scenario load has produced a bridge (scale5/controller.js,
 * loadCosmicScenario), and with `null` on teardown (destroy) so a
 * destroyed bridge never keeps driving a stale checkbox.
 *
 * @param {{getToggle?: Function, setToggle?: Function}|null} bridge
 */
export function syncScale5Toggles(bridge) {
    _activeBridge = bridge || null;
    if (!bridge?.getToggle) return;
    for (const [key, , domId] of SCALE5_TOGGLES) {
        const checked = !!bridge.getToggle(key);
        for (const root of _roots) {
            const input = findCheckbox(root, key, domId);
            if (input) input.checked = checked;
        }
    }
}
