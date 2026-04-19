/**
 * Knowledge Base — thin factory around SidebarLibraryComponent.
 * Configures the generic sidebar with KB's data and reader; nothing else.
 */

import { SidebarLibraryComponent } from '../sidebar-library/component.js';
import { getKnowledgeBaseSections, searchKnowledgeBase } from './data.js';
import { renderKbReader, renderKbEntryChip } from './reader.js';

export class KnowledgeBaseComponent {
    constructor({ app = null, getMutexPartners = () => [] } = {}) {
        this._sidebar = new SidebarLibraryComponent({
            app,
            idPrefix: 'kb',
            kicker: 'FTD Knowledge Base',
            title: 'Concepts, symbols, physics, and UI vocabulary',
            pageCopy: 'Read through the engine\'s terminology in one responsive library view with search, sections, and learner-friendly explanations.',
            sections: getKnowledgeBaseSections(),
            renderReader: renderKbReader,
            renderEntryChip: renderKbEntryChip,
            showSearch: true,
            searchPlaceholder: 'Try J, |J|, Born rule, TRAPPIST-1, sLoop, Planck units...',
            searchFn: searchKnowledgeBase,
            openButtonId: 'btn-knowledge-base',
            openClassName: 'knowledge-base-open',
            getMutexPartners,
        });
    }
    init() { this._sidebar.init(); return this; }
    open() { this._sidebar.open(); }
    close() { this._sidebar.close(); }
}
