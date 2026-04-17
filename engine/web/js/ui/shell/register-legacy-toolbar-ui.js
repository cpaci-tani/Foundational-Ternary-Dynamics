const LEGACY_TOOLBAR_GROUPS = Object.freeze([]);

export function registerLegacyToolbarUi(toolbarRegistry) {
    for (const group of LEGACY_TOOLBAR_GROUPS) {
        toolbarRegistry.registerElement({
            id: `legacy-${group.id}`,
            elementId: group.id,
            scales: group.scales,
            slot: 'secondary',
            order: group.order,
        });
    }
}
