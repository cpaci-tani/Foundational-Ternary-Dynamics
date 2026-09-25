export function getTopbarInlineTemplate() {
    return `
        <div class="topbar-slot topbar-slot-brand" data-topbar-slot="brand"></div>
        <div class="topbar-slot topbar-slot-sim" data-topbar-slot="sim"></div>
        <div class="topbar-slot topbar-slot-session" data-topbar-slot="session"></div>
        <div class="topbar-slot topbar-slot-context-meta" data-topbar-slot="context-meta">
            <span class="topbar-context-kicker">Context</span>
            <span class="topbar-context-copy">Scale-specific controls</span>
        </div>
        <div class="topbar-slot topbar-slot-context" id="toolbar-secondary-panel" data-topbar-slot="secondary"></div>
        <div class="topbar-slot topbar-slot-actions" data-topbar-slot="actions"></div>
    `;
}

export function getTopbarActionButtons() {
    return `
        <button class="tb-btn tb-btn-mobile" id="btn-toolbar-menu" title="Show mode controls"
            aria-label="Show mode controls" aria-expanded="false" aria-controls="toolbar-secondary-panel">Menu</button>
    `;
}
