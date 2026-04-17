export function getWorkspaceTabsTemplate(panelDefs) {
    const tabs = panelDefs.map((tab, index) => {
        const attrs = [
            `class="tab${index === 0 ? ' active' : ''}"`,
            'role="tab"',
            `tabindex="${index === 0 ? '0' : '-1'}"`,
            `aria-selected="${index === 0 ? 'true' : 'false'}"`,
            `data-panel="${tab.id}"`,
        ];
        if (tab.scales?.length) attrs.push(`data-scales="${tab.scales.join(',')}"`);
        return `<div ${attrs.join(' ')}>${tab.label}</div>`;
    }).join('');

    const options = panelDefs.map((tab) => `<option value="${tab.id}">${tab.label}</option>`).join('');

    return `
        <label class="workspace-tabs-mobile" for="tab-select-mobile">
            <span class="workspace-tabs-mobile-label">Panel</span>
            <select id="tab-select-mobile" class="workspace-tabs-select" aria-label="Select dashboard panel">
                ${options}
            </select>
        </label>
        <div class="workspace-tabs-strip">
            ${tabs}
            <button id="btn-panel-toggle" title="Collapse or expand panels"
                aria-label="Collapse or expand panels">&#9660;</button>
        </div>
    `;
}
