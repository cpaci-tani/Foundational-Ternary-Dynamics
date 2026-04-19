/**
 * FAQ — thin factory around SidebarLibraryComponent.
 */

import { SidebarLibraryComponent } from '../sidebar-library/component.js';
import { getFaqSections } from './data.js';
import { renderFaqReader } from './reader.js';

export class FaqComponent {
    constructor({ app = null, getMutexPartners = () => [] } = {}) {
        this._sidebar = new SidebarLibraryComponent({
            app,
            idPrefix: 'faq',
            kicker: 'FTD FAQ',
            title: 'Hard problems, framed',
            pageCopy: 'Sixteen canonical hard problems of physics and foundational science, framed through the FTD lens with explicit epistemic tags. Every entry ends with what FTD does not resolve.',
            sections: getFaqSections(),
            renderReader: renderFaqReader,
            showSearch: false,
            openButtonId: 'btn-faq',
            openClassName: 'faq-open',
            getMutexPartners,
        });
    }
    init() { this._sidebar.init(); return this; }
    open() { this._sidebar.open(); }
    close() { this._sidebar.close(); }
}
