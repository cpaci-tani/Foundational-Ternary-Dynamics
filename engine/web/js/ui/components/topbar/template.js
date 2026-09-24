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
        <button class="tb-btn tb-btn-knowledge" id="btn-knowledge-base" title="Open the FTD knowledge base"
            aria-label="Open the FTD knowledge base">KB</button>
        <button class="tb-btn tb-btn-faq" id="btn-faq" title="Open the FTD FAQ — hard problems, framed"
            aria-label="Open the FTD FAQ">FAQ</button>
        <button class="tb-btn tb-btn-fluid" id="btn-fluid" type="button"
            title="Open the Fluid panel for the main lattice" aria-label="Open the Fluid panel"
            aria-controls="panel-fluid">Fluid</button>
        <button class="tb-btn tb-btn-assistant" id="btn-ftd-assistant" title="Open the JEV console"
            aria-label="Open the JEV console" aria-expanded="false">JEV</button>
        <button class="tb-btn tb-btn-mobile" id="btn-toolbar-menu" title="Show mode controls"
            aria-label="Show mode controls" aria-expanded="false" aria-controls="toolbar-secondary-panel">Menu</button>
    `;
}
