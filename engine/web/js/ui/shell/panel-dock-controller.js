import { readStoredBoolean, writeStoredBoolean } from './layout-state.js';
import { readPanelMount } from './panel-mount-state.js';

/**
 * Owns tab activation, panel collapse state, and the existing resize handle.
 * This is the first step in moving panel-dock behavior out of app_dag.js.
 */
export class PanelDockController {
    constructor({
        app,
        tabBar,
        panelArea,
        toggleButton,
        resizeHandle,
        mountResizeHandle = null,
        compactSelect = null,
        storageKey = 'ftd-panels-collapsed',
        onTabActivated = null,
        onViewportResize = null,
    }) {
        this.app = app;
        this.tabBar = tabBar;
        this.panelArea = panelArea;
        this.toggleButton = toggleButton;
        this.resizeHandle = resizeHandle;
        this.mountResizeHandle = mountResizeHandle;
        this.compactSelect = compactSelect;
        this.storageKey = storageKey;
        this.onTabActivated = typeof onTabActivated === 'function' ? onTabActivated : null;
        this.onViewportResize = typeof onViewportResize === 'function' ? onViewportResize : null;

        this._bound = false;
        this._compactMode = false;
        this._drag = { active: false, startY: 0, startHeight: 0 };
        this._hdrag = { active: false, startX: 0, startWidth: 0, mount: 'left' };
        this._handleMouseMove = this._handleMouseMove.bind(this);
        this._handleMouseUp = this._handleMouseUp.bind(this);
        this._handleHMouseMove = this._handleHMouseMove.bind(this);
        this._handleHMouseUp = this._handleHMouseUp.bind(this);
    }

    bind({ initialActiveTab = 'controls' } = {}) {
        if (this._bound) return;
        this._bound = true;

        this._bindTabs();
        this._bindCompactSelect();
        this._bindCollapseToggle();
        this._bindResizeHandle();
        this._bindMountResizeHandle();
        this._restorePanelWidths();
        this._restoreCollapsedState();
        this.activate(initialActiveTab, { emit: false, autoExpand: false });
    }

    activate(panelName, { emit = true, autoExpand = true } = {}) {
        if (!panelName) return;
        if (autoExpand && this.app?.classList.contains('panels-collapsed')) {
            this.setCollapsed(false);
        }

        const tabs = this._getTabs();
        const panels = this._getPanels();
        const matchingTab = tabs.find((tab) => tab.dataset.panel === panelName && tab.style.display !== 'none');
        const nextTab = matchingTab || tabs.find((tab) => tab.dataset.panel === 'controls');
        if (!nextTab) return;

        tabs.forEach((tab) => {
            const isActive = tab === nextTab;
            tab.classList.toggle('active', isActive);
            tab.setAttribute('aria-selected', isActive ? 'true' : 'false');
            tab.setAttribute('tabindex', isActive ? '0' : '-1');
        });

        panels.forEach((panel) => {
            panel.classList.toggle('active', panel.id === `panel-${nextTab.dataset.panel}`);
        });

        this._syncCompactSelect(nextTab.dataset.panel);

        if (emit && this.onTabActivated) this.onTabActivated(nextTab.dataset.panel);
    }

    applyScaleFilter(scaleIndex, fallbackPanel = 'controls') {
        const tabs = this._getTabs();
        tabs.forEach((tab) => {
            if (!tab.dataset.scales) {
                tab.style.display = '';
                return;
            }
            tab.style.display = tab.dataset.scales.split(',').includes(scaleIndex) ? '' : 'none';
        });

        this._syncCompactOptions();

        const activeTab = tabs.find((tab) => tab.classList.contains('active'));
        if (activeTab && activeTab.style.display === 'none') {
            this.activate(fallbackPanel, { emit: true, autoExpand: false });
            return fallbackPanel;
        }
        return activeTab?.dataset.panel || fallbackPanel;
    }

    setCollapsed(collapsed) {
        if (!this.app || !this.toggleButton) return;
        this.app.classList.toggle('panels-collapsed', !!collapsed);
        this.app.dataset.panelsCollapsed = collapsed ? 'true' : 'false';
        this.toggleButton.innerHTML = collapsed ? '&#9650;' : '&#9660;';
        this.toggleButton.title = collapsed ? 'Expand panels' : 'Collapse panels';
        writeStoredBoolean(this.storageKey, !!collapsed);
        this._notifyViewportResize();
    }

    setCompactMode(isCompact) {
        this._compactMode = !!isCompact;
        if (this.resizeHandle) {
            this.resizeHandle.toggleAttribute('hidden', this._compactMode);
            this.resizeHandle.setAttribute('aria-hidden', this._compactMode ? 'true' : 'false');
        }
        if (this._compactMode && this.panelArea) {
            this.panelArea.style.removeProperty('height');
        }
    }

    _bindTabs() {
        this._getTabs().forEach((tab) => {
            tab.addEventListener('click', () => this.activate(tab.dataset.panel));
        });
    }

    _bindCompactSelect() {
        if (!this.compactSelect) return;
        this.compactSelect.addEventListener('change', () => this.activate(this.compactSelect.value));
        this._syncCompactOptions();
    }

    _bindCollapseToggle() {
        if (!this.toggleButton) return;
        this.toggleButton.addEventListener('click', () => {
            const collapsed = !this.app.classList.contains('panels-collapsed');
            this.setCollapsed(collapsed);
        });
    }

    _bindResizeHandle() {
        if (!this.resizeHandle || !this.panelArea) return;

        this.resizeHandle.addEventListener('mousedown', (event) => {
            if (this._compactMode) return;
            this._drag.active = true;
            this._drag.startY = event.clientY;
            this._drag.startHeight = this.panelArea.getBoundingClientRect().height;
            document.body.style.cursor = 'ns-resize';
            event.preventDefault();
        });

        document.addEventListener('mousemove', this._handleMouseMove);
        document.addEventListener('mouseup', this._handleMouseUp);
    }

    _handleMouseMove(event) {
        if (!this._drag.active || !this.panelArea || this._compactMode) return;
        const dy = this._drag.startY - event.clientY;
        const newHeight = Math.max(220, this._drag.startHeight + dy);
        this.panelArea.style.height = `${newHeight}px`;
    }

    _handleMouseUp() {
        if (!this._drag.active) return;
        this._drag.active = false;
        document.body.style.cursor = '';
        this._notifyViewportResize();
    }

    _bindMountResizeHandle() {
        if (!this.mountResizeHandle) return;
        this.mountResizeHandle.addEventListener('mousedown', (event) => {
            const mount = readPanelMount();
            if (mount !== 'left' && mount !== 'right') return;
            this._hdrag.active = true;
            this._hdrag.startX = event.clientX;
            this._hdrag.startWidth = parseFloat(
                getComputedStyle(document.documentElement).getPropertyValue(`--panel-width-${mount}`)
            ) || 380;
            this._hdrag.mount = mount;
            this.mountResizeHandle.classList.add('is-dragging');
            document.body.style.cursor = 'ew-resize';
            event.preventDefault();
        });
        document.addEventListener('mousemove', this._handleHMouseMove);
        document.addEventListener('mouseup', this._handleHMouseUp);
    }

    _handleHMouseMove(event) {
        if (!this._hdrag.active) return;
        const dx = event.clientX - this._hdrag.startX;
        const signed = this._hdrag.mount === 'left' ? dx : -dx;
        const next = Math.min(window.innerWidth * 0.5, Math.max(320, this._hdrag.startWidth + signed));
        document.documentElement.style.setProperty(`--panel-width-${this._hdrag.mount}`, `${Math.round(next)}px`);
    }

    _handleHMouseUp() {
        if (!this._hdrag.active) return;
        this._hdrag.active = false;
        this.mountResizeHandle.classList.remove('is-dragging');
        document.body.style.cursor = '';
        try {
            const mount = this._hdrag.mount;
            const value = getComputedStyle(document.documentElement)
                .getPropertyValue(`--panel-width-${mount}`).trim();
            localStorage.setItem(`ftd.panel.width.${mount}`, value);
        } catch (_err) { /* best-effort */ }
        this._notifyViewportResize();
    }

    _restorePanelWidths() {
        try {
            for (const mount of ['left', 'right']) {
                const stored = localStorage.getItem(`ftd.panel.width.${mount}`);
                if (stored && /^\d+(\.\d+)?px$/.test(stored.trim())) {
                    document.documentElement.style.setProperty(`--panel-width-${mount}`, stored.trim());
                }
            }
        } catch (_err) { /* best-effort */ }
    }

    _restoreCollapsedState() {
        const isCollapsed = readStoredBoolean(this.storageKey, true);
        this.setCollapsed(isCollapsed);
    }

    _notifyViewportResize() {
        if (this.onViewportResize) {
            window.setTimeout(() => this.onViewportResize(), 250);
        }
    }

    _getTabs() {
        return Array.from(this.tabBar?.querySelectorAll('.tab') || []);
    }

    _getPanels() {
        return Array.from(this.panelArea?.querySelectorAll('.panel') || []);
    }

    _syncCompactOptions(activePanel = null) {
        if (!this.compactSelect) return;
        const visiblePanels = new Set(
            this._getTabs()
                .filter((tab) => tab.style.display !== 'none')
                .map((tab) => tab.dataset.panel)
        );
        Array.from(this.compactSelect.options).forEach((option) => {
            const visible = visiblePanels.has(option.value);
            option.hidden = !visible;
            option.disabled = !visible;
        });
        if (activePanel) this.compactSelect.value = activePanel;
    }

    _syncCompactSelect(activePanel) {
        if (!this.compactSelect) return;
        this._syncCompactOptions(activePanel);
    }
}
