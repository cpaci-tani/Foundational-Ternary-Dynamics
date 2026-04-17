export function getWorkspaceTabsTemplate(panelDefs) {
    const tabs = panelDefs.map((tab, index) => {
        const attrs = [
            `class="tab${index === 0 ? ' active' : ''}"`,
            'role="tab"',
            `tabindex="${index === 0 ? '0' : '-1'}"`,
            `aria-selected="${index === 0 ? 'true' : 'false'}"`,
            `data-panel="${tab.id}"`,
            `title="${tab.label}"`,
        ];
        if (tab.scales?.length) attrs.push(`data-scales="${tab.scales.join(',')}"`);
        const icon = tab.icon || '';
        return `<div ${attrs.join(' ')}><span class="tab-icon" aria-hidden="true">${icon}</span><span class="tab-label">${tab.label}</span></div>`;
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
