import { getTopbarInlineTemplate, getTopbarActionButtons, getAssistantSidebarTemplate } from './template.js';

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

function currentLatticeSize() {
    const raw = document.getElementById('lattice-size')?.value;
    const value = Number.parseInt(raw, 10);
    return Number.isFinite(value) && value >= 4 ? value : 64;
}

function buildVtkExportCommand() {
    const latticeSize = currentLatticeSize();
    const ticks = 200;
    const frameInterval = 20;
    const spatialStride = 1;
    return `.\\engine\\build\\Release\\ftd_sim.exe V ${latticeSize} ${ticks} .\\engine\\results\\vtk_research_ui ${frameInterval} ${spatialStride}`;
}

async function copyText(text) {
    if (navigator.clipboard?.writeText && window.isSecureContext) {
        await navigator.clipboard.writeText(text);
        return true;
    }
    const textarea = document.createElement('textarea');
    textarea.value = text;
    textarea.setAttribute('readonly', '');
    textarea.style.position = 'fixed';
    textarea.style.opacity = '0';
    document.body.appendChild(textarea);
    textarea.select();
    let ok = false;
    try {
        ok = document.execCommand('copy');
    } finally {
        textarea.remove();
    }
    return ok;
}

export class TopbarComponent {
    constructor({ app, toolbar, toolbarRegistry = null }) {
        this.app = app;
        this.toolbar = toolbar;
        this.toolbarRegistry = toolbarRegistry;
        this.detachedToolbarNodes = new Map();
        this.assistantSidebar = null;
        this.assistantBackdrop = null;
        this.toolbarMenuButton = null;
        this.assistantButton = null;
        this.vtkButton = null;
        this.onResize = null;
    }

    init() {
        if (!this.app || !this.toolbar) return this;
        if (!this.toolbar.querySelector('[data-topbar-slot="brand"]')) {
            this._rebuildToolbar();
        }
        this.toolbar.dataset.topbar = 'enhanced';
        this._ensureAssistantSidebar();
        this._bindInteractions();
        this._watchToolbarHeight();
        return this;
    }

    _watchToolbarHeight() {
        this._syncToolbarHeight();
        this._toolbarRO = new ResizeObserver(() => this._syncToolbarHeight());
        this._toolbarRO.observe(this.toolbar);
    }

    _syncToolbarHeight() {
        const h = this.toolbar.getBoundingClientRect().height;
        if (h > 0) {
            this.app.style.setProperty('--toolbar-h', `${h}px`);
            this.onResize?.(h);
        }
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

    _ensureAssistantSidebar() {
        let sidebar = this.app.querySelector('#assistant-sidebar');
        if (!sidebar) {
            this.app.insertAdjacentHTML('beforeend', getAssistantSidebarTemplate());
            sidebar = this.app.querySelector('#assistant-sidebar');
        }
        this.assistantSidebar = sidebar;
        this.assistantBackdrop = this.app.querySelector('#assistant-sidebar-backdrop');
    }

    _bindInteractions() {
        this.toolbarMenuButton = this.toolbar.querySelector('#btn-toolbar-menu');
        this.assistantButton = this.toolbar.querySelector('#btn-ftd-assistant');
        this.vtkButton = this.toolbar.querySelector('#btn-vtk-export');
        const assistantClose = this.app.querySelector('#btn-assistant-close');
        const assistantLaunch = this.app.querySelector('#btn-assistant-launch');
        const assistantDraft = this.app.querySelector('#assistant-draft');

        this.toolbarMenuButton?.addEventListener('click', () => {
            const open = this.toolbar.dataset.compactMenu === 'open';
            this.toolbar.dataset.compactMenu = open ? 'closed' : 'open';
            this.toolbarMenuButton.setAttribute('aria-expanded', open ? 'false' : 'true');
        });

        this.assistantButton?.addEventListener('click', () => this.toggleAssistant());
        this.vtkButton?.addEventListener('click', () => this.copyVtkCommand());
        assistantClose?.addEventListener('click', () => this.closeAssistant());
        this.assistantBackdrop?.addEventListener('click', () => this.closeAssistant());
        document.addEventListener('keydown', (event) => {
            if (event.key === 'Escape') this.closeAssistant();
        });

        this.app.querySelectorAll('[data-assistant-prompt]').forEach((button) => {
            button.addEventListener('click', () => {
                if (!assistantDraft) return;
                assistantDraft.value = button.dataset.assistantPrompt || '';
                assistantDraft.focus();
            });
        });

        assistantLaunch?.addEventListener('click', (event) => event.preventDefault());
    }

    async copyVtkCommand() {
        if (!this.vtkButton) return;
        const command = buildVtkExportCommand();
        const originalText = this.vtkButton.textContent;
        const originalTitle = this.vtkButton.title;
        this.vtkButton.dataset.state = 'busy';
        this.vtkButton.textContent = '...';
        try {
            const copied = await copyText(command);
            this.vtkButton.dataset.state = copied ? 'copied' : 'failed';
            this.vtkButton.textContent = copied ? 'OK' : 'CMD';
            this.vtkButton.title = copied ? `Copied: ${command}` : command;
        } catch (_err) {
            this.vtkButton.dataset.state = 'failed';
            this.vtkButton.textContent = 'CMD';
            this.vtkButton.title = command;
        }
        window.setTimeout(() => {
            if (!this.vtkButton) return;
            this.vtkButton.dataset.state = '';
            this.vtkButton.textContent = originalText || 'VTK';
            this.vtkButton.title = originalTitle;
        }, 2400);
    }

    toggleAssistant(force) {
        const shouldOpen = typeof force === 'boolean' ? force : !this.app.classList.contains('assistant-open');
        this.app.classList.toggle('assistant-open', shouldOpen);
        this.assistantSidebar?.setAttribute('aria-hidden', shouldOpen ? 'false' : 'true');
        if (this.assistantBackdrop) this.assistantBackdrop.hidden = !shouldOpen;
        this.assistantButton?.classList.toggle('active', shouldOpen);
    }

    closeAssistant() {
        this.toggleAssistant(false);
    }
}
