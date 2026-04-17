import { getKnowledgeBaseEntry, getKnowledgeBaseSections, searchKnowledgeBase } from './data.js';
import { getKnowledgeBaseTemplate } from './template.js';

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

function buildEntryCard(entry, activeEntryId) {
    const label = entry.shortTitle || entry.title;
    const titleClass = isFormulaLikeLabel(label) ? 'kb-entry-chip-title kb-entry-chip-title-formula' : 'kb-entry-chip-title';
    return `
        <button class="kb-entry-chip${entry.id === activeEntryId ? ' active' : ''}" type="button"
            data-kb-entry="${escapeHtml(entry.id)}">
            <span class="${titleClass}">${escapeHtml(label)}</span>
        </button>
    `;
}

function buildSectionButton(section, activeSectionId) {
    return `
        <button class="kb-section-pill${section.id === activeSectionId ? ' active' : ''}" type="button"
            data-kb-section="${escapeHtml(section.id)}">
            ${escapeHtml(section.title)}
        </button>
    `;
}

function buildAllSectionsButton(activeSectionId) {
    return `
        <button class="kb-section-pill${activeSectionId === 'all' ? ' active' : ''}" type="button"
            data-kb-section="all">
            All Topics
        </button>
    `;
}

function buildEntryReader(entry) {
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

export class KnowledgeBaseComponent {
    constructor({ app = null } = {}) {
        this.app = app || document.getElementById('app');
        this.sections = getKnowledgeBaseSections();
        this.query = '';
        this.activeSectionId = 'all';
        this.activeEntryId = this.sections[0]?.entries[0]?.id || null;
        this.isOpen = false;
        this.dom = null;
    }

    init() {
        if (!this.app) return this;
        this._ensureMarkup();
        this._collectDom();
        this._bindEvents();
        this.render();
        return this;
    }

    _ensureMarkup() {
        if (this.app.querySelector('#knowledge-base-sidebar')) return;
        this.app.insertAdjacentHTML('beforeend', getKnowledgeBaseTemplate());
    }

    _collectDom() {
        this.dom = {
            backdrop: this.app.querySelector('#knowledge-base-backdrop'),
            sidebar: this.app.querySelector('#knowledge-base-sidebar'),
            sidebarSearch: this.app.querySelector('#kb-sidebar-search'),
            sidebarSections: this.app.querySelector('#kb-sidebar-sections'),
            sidebarList: this.app.querySelector('#kb-sidebar-list'),
            sidebarReader: this.app.querySelector('#kb-sidebar-reader'),
            resultsLabel: this.app.querySelector('#kb-results-label'),
            sidebarOpenButton: document.getElementById('btn-knowledge-base'),
            sidebarCloseButton: this.app.querySelector('#btn-kb-sidebar-close'),
        };
    }

    _bindEvents() {
        this.dom.sidebarOpenButton?.addEventListener('click', () => this.open());
        this.dom.sidebarCloseButton?.addEventListener('click', () => this.close());
        this.dom.backdrop?.addEventListener('click', () => this.close());

        const onSearch = (event) => {
            this.query = event.target.value || '';
            this.render();
        };
        this.dom.sidebarSearch?.addEventListener('input', onSearch);

        this.dom.sidebarSections?.addEventListener('click', (event) => this._handleSectionClick(event));
        this.dom.sidebarList?.addEventListener('click', (event) => this._handleEntryClick(event));

        document.addEventListener('keydown', (event) => {
            if (event.key === 'Escape') this.close();
        });
    }

    _handleSectionClick(event) {
        const button = event.target instanceof Element ? event.target.closest('[data-kb-section]') : null;
        if (!(button instanceof HTMLElement)) return;
        this.activeSectionId = button.dataset.kbSection || 'all';
        this._ensureValidActiveEntry();
        this.render();
    }

    _handleEntryClick(event) {
        const button = event.target instanceof Element ? event.target.closest('[data-kb-entry]') : null;
        if (!(button instanceof HTMLElement)) return;
        this.activeEntryId = button.dataset.kbEntry || this.activeEntryId;
        this.render();
    }

    _getFilteredEntries() {
        return searchKnowledgeBase(this.query, this.activeSectionId);
    }

    _ensureValidActiveEntry() {
        const entries = this._getFilteredEntries();
        if (!entries.length) {
            this.activeEntryId = null;
            return;
        }
        if (!entries.some((entry) => entry.id === this.activeEntryId)) {
            this.activeEntryId = entries[0].id;
        }
    }

    _getActiveEntry() {
        const entries = this._getFilteredEntries();
        return entries.find((entry) => entry.id === this.activeEntryId) || entries[0] || getKnowledgeBaseEntry(this.activeEntryId);
    }

    render() {
        this._ensureValidActiveEntry();
        const entries = this._getFilteredEntries();
        const activeEntry = this._getActiveEntry();

        const sectionButtons = [
            buildAllSectionsButton(this.activeSectionId),
            ...this.sections.map((section) => buildSectionButton(section, this.activeSectionId)),
        ].join('');

        const entryListMarkup = entries.length
            ? entries.map((entry) => buildEntryCard(entry, activeEntry?.id || null)).join('')
            : `<div class="kb-empty-list">No topics matched this filter.</div>`;

        const readerMarkup = buildEntryReader(activeEntry);

        if (this.dom.sidebarSections) this.dom.sidebarSections.innerHTML = sectionButtons;
        if (this.dom.sidebarList) this.dom.sidebarList.innerHTML = entryListMarkup;
        if (this.dom.sidebarReader) this.dom.sidebarReader.innerHTML = readerMarkup;
        if (this.dom.resultsLabel) {
            this.dom.resultsLabel.textContent = `${entries.length} topic${entries.length === 1 ? '' : 's'}`;
        }

        if (this.dom.sidebarSearch && this.dom.sidebarSearch.value !== this.query) this.dom.sidebarSearch.value = this.query;
    }

    open() {
        this.isOpen = true;
        this._applyVisibility();
    }

    close() {
        this.isOpen = false;
        this._applyVisibility();
    }

    _applyVisibility() {
        this.app.classList.toggle('knowledge-base-open', this.isOpen);
        if (this.dom.sidebar) this.dom.sidebar.setAttribute('aria-hidden', this.isOpen ? 'false' : 'true');
        if (this.dom.backdrop) this.dom.backdrop.hidden = !this.isOpen;
    }
}
