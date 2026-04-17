/**
 * Placeholder registry seam for viewport overlays.
 */

export function createOverlayRegistry(initialItems = []) {
    const items = [...initialItems];
    return {
        list() { return items.slice(); },
        register(definition) { items.push(definition); return definition; },
        clear() { items.length = 0; },
    };
}
