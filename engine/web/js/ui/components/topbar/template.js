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
        <button class="tb-btn tb-btn-assistant" id="btn-ftd-assistant" title="Open the FTD assistant sidebar"
            aria-label="Open the FTD assistant sidebar">FTD</button>
        <button class="tb-btn tb-btn-mobile" id="btn-toolbar-menu" title="Show mode controls"
            aria-label="Show mode controls" aria-expanded="false" aria-controls="toolbar-secondary-panel">Menu</button>
    `;
}

export function getAssistantSidebarTemplate() {
    return `
        <div id="assistant-sidebar-backdrop" hidden></div>
        <aside id="assistant-sidebar" aria-label="FTD assistant" aria-hidden="true">
            <div class="assistant-sidebar-header">
                <div>
                    <div class="assistant-kicker">FTD Copilot</div>
                    <h2 class="assistant-title">Research Sidebar</h2>
                </div>
                <button class="assistant-close" id="btn-assistant-close" aria-label="Close assistant">&times;</button>
            </div>
            <div class="assistant-status">
                <span class="assistant-status-dot"></span>
                Local-model slot reserved for an eventual FTD-tuned assistant
            </div>
            <p class="assistant-copy">
                This drawer is the future home for a compact language model that can explain scales, scenarios,
                theorems, and UI state directly inside the engine.
            </p>
            <div class="assistant-prompts">
                <button class="assistant-chip" data-assistant-prompt="Summarize the current scale and active scenario.">Summarize scale</button>
                <button class="assistant-chip" data-assistant-prompt="Explain the active panel and what it measures.">Explain panel</button>
                <button class="assistant-chip" data-assistant-prompt="List open questions or conjectures relevant to this mode.">Open questions</button>
            </div>
            <label class="assistant-field" for="assistant-draft">
                <span>Prompt draft</span>
                <textarea id="assistant-draft" rows="7"
                    placeholder="Eventually ask things like: explain the lattice call stack, summarize Scale 0 flux overlays, or compare two scenarios."></textarea>
            </label>
            <div class="assistant-actions">
                <button class="assistant-primary" id="btn-assistant-launch" disabled>Local model coming soon</button>
            </div>
        </aside>
    `;
}
