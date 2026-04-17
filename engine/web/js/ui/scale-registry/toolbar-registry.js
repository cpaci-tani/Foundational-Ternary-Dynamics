/**
 * Registry for toolbar contributions.
 *
 * Contributions may point at existing legacy DOM nodes or provide a factory
 * that builds a node owned by a scale UI module.
 */

function normalizeItem(definition, index) {
    return {
        id: definition.id || `toolbar-item-${index}`,
        slot: definition.slot || 'secondary',
        order: Number.isFinite(definition.order) ? definition.order : index,
        type: definition.type || (definition.factory ? 'factory' : 'element'),
        elementId: definition.elementId || definition.id || null,
        factory: typeof definition.factory === 'function' ? definition.factory : null,
        scales: definition.scales || null,
    };
}

export function createToolbarRegistry(initialItems = []) {
    const items = initialItems.map((item, index) => normalizeItem(item, index));

    return {
        list({ slot = null } = {}) {
            const filtered = slot ? items.filter((item) => item.slot === slot) : items.slice();
            return filtered.slice().sort((a, b) => a.order - b.order);
        },
        register(definition) {
            const item = normalizeItem(definition, items.length);
            items.push(item);
            return item;
        },
        registerElement(definition) {
            return this.register({ ...definition, type: 'element' });
        },
        registerFactory(definition) {
            return this.register({ ...definition, type: 'factory' });
        },
        clear() { items.length = 0; },
    };
}
