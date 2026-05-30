import { readStoredBoolean, writeStoredBoolean } from './layout-state.js';
import { floatingWindowManager } from '../components/floating-window/component.js';

export class PanelDockController {
    constructor({
        app,
        tabBar,
        panelArea,
        toggleButton,
        resizeHandle,
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
        this.compactSelect = compactSelect;
        this.storageKey = storageKey;
        this.onTabActivated = typeof onTabActivated === 'function' ? onTabActivated : null;
        this.onViewportResize = typeof onViewportResize === 'function' ? onViewportResize : null;

        this._bound = false;
        this._compactMode = false;
        this._drag = { active: false, startY: 0, startHeight: 0 };
        this._handleMouseMove = this._handleMouseMove.bind(this);
        this._handleMouseUp = this._handleMouseUp.bind(this);
    }

    bind({ initialActiveTab = 'controls' } = {}) {
        if (this._bound) return;
        this._bound = true;

        this._bindTabs();
        this._bindCompactSelect();
        this._bindCollapseToggle();
        this._bindResizeHandle();
        this._restoreCollapsedState();
        this.activate(initialActiveTab, { emit: false, autoExpand: false });
    }

    activate(panelName, { emit = true, autoExpand = true } = {}) {
        if (!panelName) return;

        // If the panel is floated, focus its floating window and do not mount in dock
        if (floatingWindowManager.has(panelName)) {
            const win = floatingWindowManager.getWindow(panelName);
            win.focus();
            // Flash title bar to alert user
            if (win.header) {
                win.header.style.background = 'rgba(0, 229, 255, 0.2)';
                setTimeout(() => {
                    if (win.header) win.header.style.background = '';
                }, 180);
            }
            return;
        }

        if (autoExpand && this.app?.classList?.contains('panels-collapsed')) {
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
        if (!this.app) return;
        this.app.classList?.toggle('panels-collapsed', !!collapsed);
        if (this.app.dataset) {
            this.app.dataset.panelsCollapsed = collapsed ? 'true' : 'false';
        }
        
        const btn = document.getElementById('btn-panel-toggle');
        if (btn) {
            btn.innerHTML = collapsed ? '&#9650;' : '&#9660;';
            btn.title = collapsed ? 'Expand panels' : 'Collapse panels';
        }
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
            let startX = 0;
            let startY = 0;
            let hasDragged = false;
            let dragThresholdActive = false;

            const onPointerMove = (e) => {
                if (!dragThresholdActive) return;
                const dx = e.clientX - startX;
                const dy = e.clientY - startY;
                if (Math.sqrt(dx * dx + dy * dy) > 15) {
                    hasDragged = true;
                    dragThresholdActive = false;
                    
                    window.removeEventListener('pointermove', onPointerMove);
                    window.removeEventListener('pointerup', onPointerUp);

                    // Float the panel!
                    const panelId = tab.dataset.panel;
                    const win = this.floatPanel(panelId, e.clientX, e.clientY);
                    
                    // Seamlessly chain pointer drag on the new window title header
                    if (win) win.startDrag(e.clientX, e.clientY);
                }
            };

            const onPointerUp = (e) => {
                dragThresholdActive = false;
                window.removeEventListener('pointermove', onPointerMove);
                window.removeEventListener('pointerup', onPointerUp);
            };

            tab.addEventListener('pointerdown', (e) => {
                // Only left click / standard pointer touch
                if (e.button !== 0) return;
                startX = e.clientX;
                startY = e.clientY;
                hasDragged = false;
                dragThresholdActive = true;

                window.addEventListener('pointermove', onPointerMove);
                window.addEventListener('pointerup', onPointerUp);
            });

            tab.addEventListener('click', (e) => {
                if (!hasDragged) {
                    this.activate(tab.dataset.panel);
                }
            });
        });
    }

    floatPanel(panelId, x, y) {
        const tab = this.tabBar.querySelector(`.tab[data-panel="${panelId}"]`);
        const panelEl = this.panelArea.querySelector(`#panel-${panelId}`);
        if (!tab || !panelEl) return null;

        tab.classList.add('is-floated');

        // Extract label and icon from tab
        const label = tab.title || tab.querySelector('.tab-label')?.textContent || 'Panel';
        const icon = tab.querySelector('.tab-icon')?.textContent || '⚙️';

        // Auto-activate next visible docked tab so the dock is never blank
        const otherTabs = this._getTabs().filter(t => t.dataset.panel !== panelId && !t.classList.contains('is-floated') && t.style.display !== 'none');
        if (tab.classList.contains('active')) {
            if (otherTabs.length > 0) {
                this.activate(otherTabs[0].dataset.panel);
            } else {
                this.setCollapsed(true);
            }
        }

        // Create the floating window
        return floatingWindowManager.floatPanel(panelId, label, icon, panelEl, { x: x - 100, y: y - 10 }, (id, el) => {
            // Callback onDock back to dock
            tab.classList.remove('is-floated');
            
            // Put panel back into dock body
            const dockBody = this.panelArea.querySelector('[data-panel-dock-body]') || this.panelArea;
            dockBody.appendChild(el);

            this.setCollapsed(false);
            this.activate(id);
        });
    }

    _bindCompactSelect() {
        if (!this.compactSelect) return;
        this.compactSelect.addEventListener('change', () => this.activate(this.compactSelect.value));
        this._syncCompactOptions();
    }

    _bindCollapseToggle() {
        if (!this.app || typeof this.app.addEventListener !== 'function') return;
        this.app.addEventListener('click', (event) => {
            const btn = event.target.closest('#btn-panel-toggle');
            if (btn) {
                const collapsed = !this.app.classList?.contains('panels-collapsed');
                this.setCollapsed(collapsed);
            }
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

    _restoreCollapsedState() {
        const isCollapsed = readStoredBoolean(this.storageKey, false);
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
