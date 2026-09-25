import { getTopbarInlineTemplate, getTopbarActionButtons } from './template.js';
import { createValidityIndicator } from '../validity-status.js';

function htmlToFragment(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content;
}

function isSimulationToolbarNode(node) {
    if (!node || !(node instanceof HTMLElement)) return false;
    if (node.querySelector('#btn-play')) return true;
    return false;
}

function isSessionToolbarNode(node) {
    if (!node || !(node instanceof HTMLElement)) return false;
    if (node.querySelector('#engine-mode')) return true;
    if (node.querySelector('#ticks-per-frame')) return true;
    return false;
}

export class TopbarComponent {
    constructor({ app, toolbar, toolbarRegistry = null }) {
        this.app = app;
        this.toolbar = toolbar;
        this.toolbarRegistry = toolbarRegistry;
        this.detachedToolbarNodes = new Map();
        this.toolbarMenuButton = null;
        this.vtkButton = null;
        this.validity = null;
    }

    init() {
        if (!this.app || !this.toolbar) return this;
        if (!this.toolbar.querySelector('[data-topbar-slot="brand"]')) {
            this._rebuildToolbar();
        }
        this.validity ??= createValidityIndicator(this.toolbar.querySelector('[data-topbar-slot="actions"]'),
            { id: 'scale0-validity-status' });
        this.toolbar.dataset.topbar = 'enhanced';
        this._bindInteractions();
        return this;
    }

    _rebuildToolbar() {
        const existingChildren = Array.from(this.toolbar.children).filter((node) => !(node.classList?.contains('separator')));
        this.detachedToolbarNodes = new Map(
            existingChildren
                .filter((node) => node instanceof HTMLElement && node.id)
                .map((node) => [node.id, node]),
        );
        const inlineLayout = htmlToFragment(getTopbarInlineTemplate());
        this.toolbar.replaceChildren(inlineLayout);

        const brandSlot = this.toolbar.querySelector('[data-topbar-slot="brand"]');
        const simSlot = this.toolbar.querySelector('[data-topbar-slot="sim"]');
        const sessionSlot = this.toolbar.querySelector('[data-topbar-slot="session"]');
        const actionsSlot = this.toolbar.querySelector('[data-topbar-slot="actions"]');
        const secondarySlot = this.toolbar.querySelector('[data-topbar-slot="secondary"]');

        existingChildren.forEach((node) => {
            if (!(node instanceof HTMLElement)) return;
            if (node.style?.marginLeft === 'auto') return;
            if (node.classList.contains('brand')) {
                brandSlot.appendChild(node);
                return;
            }
            if (node.id === 'btn-settings' || node.id === 'btn-toggle-ui') {
                actionsSlot.appendChild(node);
                return;
            }
            if (isSimulationToolbarNode(node)) {
                simSlot.appendChild(node);
                return;
            }
            if (isSessionToolbarNode(node)) {
                sessionSlot.appendChild(node);
                return;
            }
            // Remaining groups are scale-context controls and belong in the contextual row.
            secondarySlot.appendChild(node);
        });

        this._mountRegistryItems(secondarySlot);
        actionsSlot.prepend(htmlToFragment(getTopbarActionButtons()));
        this.validity = createValidityIndicator(actionsSlot, { id: 'scale0-validity-status' });
        this.toolbar.dataset.compactMenu = 'closed';
    }

    _mountRegistryItems(slot) {
        if (!this.toolbarRegistry) return;
        for (const item of this.toolbarRegistry.list({ slot: 'secondary' })) {
            let node = null;
            if (item.type === 'factory' && item.factory) {
                node = item.factory();
            } else if (item.type === 'element' && item.elementId) {
                node = this.detachedToolbarNodes.get(item.elementId) || document.getElementById(item.elementId);
            }
            if (!node) continue;
            if (node instanceof DocumentFragment) {
                slot.appendChild(node);
                continue;
            }
            node.dataset.toolbarContribution = item.id;
            slot.appendChild(node);
        }
    }

    _bindInteractions() {
        this.toolbarMenuButton = this.toolbar.querySelector('#btn-toolbar-menu');
        this.toolbarMenuButton?.addEventListener('click', () => {
            const open = this.toolbar.dataset.compactMenu === 'open';
            this.toolbar.dataset.compactMenu = open ? 'closed' : 'open';
            this.toolbarMenuButton.setAttribute('aria-expanded', open ? 'false' : 'true');
        });

    }
}
