export function getKnowledgeBaseTemplate() {
    return `
        <div id="knowledge-base-backdrop" hidden></div>

        <section id="knowledge-base-sidebar" aria-label="FTD knowledge base" aria-hidden="true">
            <div class="kb-shell-header">
                <div>
                    <div class="kb-kicker">FTD Knowledge Base</div>
                    <h2 class="kb-title">Concepts, symbols, physics, and UI vocabulary</h2>
                    <p class="kb-page-copy">
                        Read through the engine's terminology in one responsive library view with search, sections,
                        and learner-friendly explanations.
                    </p>
                </div>
                <button class="kb-close" id="btn-kb-sidebar-close" aria-label="Close knowledge base">&times;</button>
            </div>

            <div class="kb-toolbar">
                <label class="kb-search-field" for="kb-sidebar-search">
                    <span>Search the library</span>
                    <input id="kb-sidebar-search" type="search"
                        placeholder="Try J, |J|, Born rule, TRAPPIST-1, sLoop, Planck units..." />
                </label>
                <div class="kb-page-meta">
                    <span class="kb-results-label" id="kb-results-label">0 topics</span>
                </div>
            </div>

            <div class="kb-section-filter" id="kb-sidebar-sections" aria-label="Knowledge base sections"></div>

            <div class="kb-sidebar-body">
                <nav class="kb-entry-list" id="kb-sidebar-list" aria-label="Knowledge base topics"></nav>
                <article class="kb-entry-reader" id="kb-sidebar-reader" aria-live="polite"></article>
            </div>
        </section>
    `;
}
