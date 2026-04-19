/**
 * Shared template for library-style sidebars (KB, FAQ, any future ones).
 *
 * Accepts a config object so each consumer picks its own DOM ids, kicker
 * text, heading, page copy, and whether to show a search box. All other
 * shell behavior lives in SidebarLibraryComponent.
 */

function escapeHtml(s) {
    return String(s ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
}

export function getSidebarLibraryTemplate({
    idPrefix,
    kicker,
    title,
    pageCopy,
    showSearch = false,
    searchPlaceholder = 'Search…',
}) {
    const p = escapeHtml(idPrefix);
    const searchBlock = showSearch
        ? `
            <div class="sidelib-toolbar">
                <label class="sidelib-search-field" for="${p}-sidebar-search">
                    <span>Search the library</span>
                    <input id="${p}-sidebar-search" type="search" placeholder="${escapeHtml(searchPlaceholder)}" />
                </label>
                <div class="sidelib-page-meta">
                    <span class="sidelib-results-label" id="${p}-results-label">0 topics</span>
                </div>
            </div>
        `
        : '';
    return `
        <div id="${p}-backdrop" class="sidelib-backdrop" hidden></div>
        <section id="${p}-sidebar" class="sidelib-sidebar sidelib-sidebar--${p}"
            aria-label="${escapeHtml(kicker)}" aria-hidden="true">
            <div class="sidelib-shell-header">
                <div>
                    <div class="sidelib-kicker">${escapeHtml(kicker)}</div>
                    <h2 class="sidelib-title">${escapeHtml(title)}</h2>
                    <p class="sidelib-page-copy">${escapeHtml(pageCopy)}</p>
                </div>
                <button class="sidelib-close" id="btn-${p}-sidebar-close" aria-label="Close">&times;</button>
            </div>
            ${searchBlock}
            <div class="sidelib-section-filter" id="${p}-sidebar-sections" aria-label="Sections"></div>
            <div class="sidelib-body">
                <nav class="sidelib-entry-list" id="${p}-sidebar-list" aria-label="Entries"></nav>
                <article class="sidelib-entry-reader" id="${p}-sidebar-reader" aria-live="polite"></article>
            </div>
        </section>
    `;
}
