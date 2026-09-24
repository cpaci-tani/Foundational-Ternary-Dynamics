export function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = String(markup || '').trim();
    const element = template.content.firstElementChild;
    if (!element) throw new Error('Toolbar template produced no root element');
    return element;
}

export function registerScaleToolbarFactories(toolbarRegistry, scale, definitions) {
    if (!toolbarRegistry?.registerFactory) {
        throw new Error('A toolbar registry with registerFactory() is required');
    }
    const scaleId = String(scale);
    for (const definition of definitions) {
        toolbarRegistry.registerFactory({
            slot: 'secondary',
            ...definition,
            scales: [scaleId],
        });
    }
}
