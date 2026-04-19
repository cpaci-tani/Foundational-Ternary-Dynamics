/**
 * KB entry reader — extracted from component.js so the Knowledge Base can
 * share the generic SidebarLibraryComponent shell and only plug in its
 * own reader render function.
 */

function escapeHtml(value) {
    return String(value ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
}

function isFormulaLikeLabel(value) {
    return /[∇∂×·²³₀₁₂₃₄₅₆₇₈₉α-ωΑ-ΩψρσΩηθλμνξπφχτℏ⟨⟩|=+\-/*^_]/u.test(String(value ?? ''));
}

export function renderKbEntryChip(entry, isActive) {
    const label = entry.shortTitle || entry.title;
    const titleClass = isFormulaLikeLabel(label)
        ? 'sidelib-entry-chip-title kb-entry-chip-title-formula'
        : 'sidelib-entry-chip-title';
    return `
        <button class="sidelib-entry-chip${isActive ? ' active' : ''}" type="button"
            data-sidelib-entry="${escapeHtml(entry.id)}">
            <span class="${titleClass}">${escapeHtml(label)}</span>
        </button>
    `;
}

export function renderKbReader(entry) {
    if (!entry) {
        return `
            <div class="kb-empty-state">
                <h3>Nothing matched yet</h3>
                <p>Try a broader search or switch back to all topics.</p>
            </div>
        `;
    }

    const body = (entry.body || []).map((paragraph) => `<p>${escapeHtml(paragraph)}</p>`).join('');
    const bullets = (entry.bullets || []).length
        ? `<ul class="kb-bullet-list">${entry.bullets.map((item) => `<li>${escapeHtml(item)}</li>`).join('')}</ul>`
        : '';
    const notation = (entry.notation || []).length
        ? `
            <div class="kb-meta-block">
                <span class="kb-meta-label">Notation</span>
                <div class="kb-token-row">
                    ${entry.notation.map((item) => `<span class="kb-token kb-token-formula">${escapeHtml(item)}</span>`).join('')}
                </div>
            </div>
        `
        : '';
    const tags = (entry.tags || []).length
        ? `
            <div class="kb-meta-block">
                <span class="kb-meta-label">Tags</span>
                <div class="kb-token-row">
                    ${entry.tags.map((item) => `<span class="kb-token kb-token-muted">${escapeHtml(item)}</span>`).join('')}
                </div>
            </div>
        `
        : '';

    return `
        <header class="kb-reader-header">
            <div class="kb-reader-section">${escapeHtml(entry.sectionTitle)}</div>
            <h3 class="kb-reader-title">${escapeHtml(entry.title)}</h3>
            <p class="kb-reader-summary">${escapeHtml(entry.summary)}</p>
        </header>
        <div class="kb-reader-body">
            ${body}
            ${bullets}
            ${notation}
            ${tags}
        </div>
    `;
}
