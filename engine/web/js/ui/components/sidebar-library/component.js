/**
 * SidebarLibraryComponent — reusable library-style sidebar.
 *
 * Consumers pass a config object:
 *   idPrefix           string, e.g. 'kb' or 'faq' — drives all DOM ids
 *   kicker             short uppercase label above the title
 *   title              main heading
 *   pageCopy           one-paragraph description beneath the title
 *   sections           [{id, title, entries: [{id, ...}]}]
 *   renderReader       (entry) => HTML for the right-column reader pane
 *   renderEntryChip    (entry, isActive) => HTML for an entry list item
 *                      (default: uses entry.shortTitle || entry.title || entry.shortQuestion || entry.question)
 *   showSearch         boolean, default false
 *   searchPlaceholder  string, used when showSearch is true
 *   searchFn           (query, sectionId) => entries[]   (required if showSearch)
 *   openButtonId       id of the button that opens the sidebar
 *   openClassName      class applied to <#app> when the sidebar is open
 *   getMutexPartners   () => SidebarLibraryComponent[]  (closes them on open)
 */

import { getSidebarLibraryTemplate } from './template.js';

function escapeHtml(s) {
    return String(s ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
}

function defaultChipLabel(entry) {
    return entry.shortTitle || entry.title || entry.shortQuestion || entry.question || entry.id;
}

function defaultRenderEntryChip(entry, isActive) {
    return `
        <button class="sidelib-entry-chip${isActive ? ' active' : ''}" type="button"
            data-sidelib-entry="${escapeHtml(entry.id)}">
            <span class="sidelib-entry-chip-title">${escapeHtml(defaultChipLabel(entry))}</span>
        </button>
    `;
}

function renderSectionFilter(sections, activeSectionId) {
    const all = `<button class="sidelib-section-pill${activeSectionId === 'all' ? ' active' : ''}" type="button" data-sidelib-section="all">All Topics</button>`;
    const rest = sections.map((s) => `
        <button class="sidelib-section-pill${activeSectionId === s.id ? ' active' : ''}" type="button"
            data-sidelib-section="${escapeHtml(s.id)}">${escapeHtml(s.title)}</button>
    `).join('');
    return all + rest;
}

export class SidebarLibraryComponent {
    constructor(config) {
        const {
            app = null,
            idPrefix,
            kicker,
            title,
            pageCopy,
            sections,
            renderReader,
            renderEntryChip = defaultRenderEntryChip,
            showSearch = false,
            searchPlaceholder = 'Search…',
            searchFn = null,
            openButtonId,
            openClassName,
            getMutexPartners = () => [],
        } = config;

        if (!idPrefix) throw new Error('SidebarLibraryComponent: idPrefix is required');
        if (typeof renderReader !== 'function') throw new Error('SidebarLibraryComponent: renderReader is required');
        if (showSearch && typeof searchFn !== 'function') throw new Error('SidebarLibraryComponent: searchFn is required when showSearch is true');

        this.app = app || document.getElementById('app');
        this.idPrefix = idPrefix;
        this.kicker = kicker;
        this.title = title;
        this.pageCopy = pageCopy;
        this.sections = sections;
        this.renderReader = renderReader;
        this.renderEntryChip = renderEntryChip;
        this.showSearch = showSearch;
        this.searchPlaceholder = searchPlaceholder;
        this.searchFn = searchFn;
        this.openButtonId = openButtonId;
        this.openClassName = openClassName;
        this.getMutexPartners = getMutexPartners;

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
        if (this.app.querySelector(`#${this.idPrefix}-sidebar`)) return;
        this.app.insertAdjacentHTML('beforeend', getSidebarLibraryTemplate({
            idPrefix: this.idPrefix,
            kicker: this.kicker,
            title: this.title,
            pageCopy: this.pageCopy,
            showSearch: this.showSearch,
            searchPlaceholder: this.searchPlaceholder,
        }));
    }

    _collectDom() {
        const p = this.idPrefix;
        this.dom = {
            backdrop: this.app.querySelector(`#${p}-backdrop`),
            sidebar: this.app.querySelector(`#${p}-sidebar`),
            sidebarSearch: this.app.querySelector(`#${p}-sidebar-search`),
            sidebarSections: this.app.querySelector(`#${p}-sidebar-sections`),
            sidebarList: this.app.querySelector(`#${p}-sidebar-list`),
            sidebarReader: this.app.querySelector(`#${p}-sidebar-reader`),
            resultsLabel: this.app.querySelector(`#${p}-results-label`),
            sidebarOpenButton: document.getElementById(this.openButtonId),
            sidebarCloseButton: this.app.querySelector(`#btn-${p}-sidebar-close`),
        };
    }

    _bindEvents() {
        this.dom.sidebarOpenButton?.addEventListener('click', () => this.open());
        this.dom.sidebarCloseButton?.addEventListener('click', () => this.close());
        this.dom.backdrop?.addEventListener('click', () => this.close());

        if (this.showSearch && this.dom.sidebarSearch) {
            this.dom.sidebarSearch.addEventListener('input', (e) => {
                this.query = e.target.value || '';
                this.render();
            });
        }

        this.dom.sidebarSections?.addEventListener('click', (e) => this._handleSectionClick(e));
        this.dom.sidebarList?.addEventListener('click', (e) => this._handleEntryClick(e));

        document.addEventListener('keydown', (e) => {
            if (this.isOpen && e.key === 'Escape') this.close();
        });
    }

    _handleSectionClick(event) {
        const btn = event.target instanceof Element ? event.target.closest('[data-sidelib-section]') : null;
        if (!(btn instanceof HTMLElement)) return;
        this.activeSectionId = btn.dataset.sidelibSection || 'all';
        this._ensureValidActiveEntry();
        this.render();
    }

    _handleEntryClick(event) {
        const btn = event.target instanceof Element ? event.target.closest('[data-sidelib-entry]') : null;
        if (!(btn instanceof HTMLElement)) return;
        this.activeEntryId = btn.dataset.sidelibEntry || this.activeEntryId;
        this.render();
    }

    _getFilteredEntries() {
        if (this.showSearch && this.searchFn) {
            return this.searchFn(this.query, this.activeSectionId);
        }
        if (this.activeSectionId === 'all') {
            return this.sections.flatMap((s) => s.entries);
        }
        const section = this.sections.find((s) => s.id === this.activeSectionId);
        return section ? section.entries : [];
    }

    _ensureValidActiveEntry() {
        const entries = this._getFilteredEntries();
        if (!entries.length) {
            this.activeEntryId = null;
            return;
        }
        if (!entries.some((e) => e.id === this.activeEntryId)) {
            this.activeEntryId = entries[0].id;
        }
    }

    _getActiveEntry() {
        const entries = this._getFilteredEntries();
        return entries.find((e) => e.id === this.activeEntryId) || entries[0] || null;
    }

    render() {
        this._ensureValidActiveEntry();
        const entries = this._getFilteredEntries();
        const activeEntry = this._getActiveEntry();

        if (this.dom.sidebarSections) {
            this.dom.sidebarSections.innerHTML = renderSectionFilter(this.sections, this.activeSectionId);
        }
        if (this.dom.sidebarList) {
            this.dom.sidebarList.innerHTML = entries.length
                ? entries.map((e) => this.renderEntryChip(e, e.id === activeEntry?.id)).join('')
                : '<div class="sidelib-empty-list">No topics matched this filter.</div>';
        }
        if (this.dom.sidebarReader) {
            this.dom.sidebarReader.innerHTML = this.renderReader(activeEntry);
        }
        if (this.dom.resultsLabel) {
            this.dom.resultsLabel.textContent = `${entries.length} topic${entries.length === 1 ? '' : 's'}`;
        }
        if (this.dom.sidebarSearch && this.dom.sidebarSearch.value !== this.query) {
            this.dom.sidebarSearch.value = this.query;
        }
    }

    open() {
        for (const partner of this.getMutexPartners()) {
            if (partner && typeof partner.close === 'function') partner.close();
        }
        this.isOpen = true;
        this._applyVisibility();
    }

    close() {
        this.isOpen = false;
        this._applyVisibility();
    }

    _applyVisibility() {
        if (this.openClassName) this.app.classList.toggle(this.openClassName, this.isOpen);
        if (this.dom.sidebar) this.dom.sidebar.setAttribute('aria-hidden', this.isOpen ? 'false' : 'true');
        if (this.dom.backdrop) this.dom.backdrop.hidden = !this.isOpen;
    }
}
