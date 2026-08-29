import { readStoredBoolean, writeStoredBoolean } from './layout-state.js';
import { floatingWindowManager } from '../components/floating-window/component.js';
import { applyPanelMountClasses, updateSafeEdges } from '../components/panel-dock/mount-toggle.js';

const PANEL_WIDTH_STORAGE_KEY = 'ftd.panel.side-width';
const RAIL_WIDTH_STORAGE_KEY = 'ftd.panel.rail-width';
const PANEL_MIN_WIDTH = 320;
const RAIL_MIN_WIDTH = 44;
const RAIL_EXPANDED_WIDTH = 220;
const RAIL_LABEL_THRESHOLD = 104;
const RESIZE_KEY_STEP = 16;

function clamp(value, min, max) {
    return Math.min(max, Math.max(min, value));
}

function readStoredNumber(key, fallback) {
    try {
        const raw = window.localStorage?.getItem(key);
        if (raw === null || raw === undefined || raw === '') return fallback;
        const value = Number(raw);
        return Number.isFinite(value) ? value : fallback;
    } catch {
        return fallback;
    }
}

function writeStoredNumber(key, value) {
    try {
        window.localStorage?.setItem(key, String(Math.round(value)));
    } catch {
        // Storage is optional. The live CSS state remains authoritative.
    }
}

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
        this._viewportResizeTimer = null;
        this._resizeRaf = null;
        this._windowResizeRaf = null;
        this._railResizeHandle = null;
        this._sideResizeHandle = null;
        this._drag = {
            active: false,
            kind: null,
            pointerId: null,
            handle: null,
            mount: 'left',
            startX: 0,
            startY: 0,
            startSize: 0,
            pendingSize: null,
        };
        this._handleResizePointerMove = this._handleResizePointerMove.bind(this);
        this._handleResizePointerUp = this._handleResizePointerUp.bind(this);
        this._handleOuterResizeKeydown = this._handleOuterResizeKeydown.bind(this);
        this._handleRailResizeKeydown = this._handleRailResizeKeydown.bind(this);
        this._handleWindowResize = this._handleWindowResize.bind(this);
    }

    bind({ initialActiveTab = 'controls' } = {}) {
        if (this._bound) return;
        this._bound = true;

        this._bindTabs();
        this._bindCompactSelect();
        this._bindCollapseToggle();
        this._ensureRailResizeHandle();
        this._restoreSizeState();
        this._bindResizeHandle();
        this._bindRailResizeHandle();
        window.addEventListener('resize', this._handleWindowResize, { passive: true });
        window.addEventListener('blur', this._handleResizePointerUp);
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
        applyPanelMountClasses(document.documentElement.dataset.panelMount || 'left');

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
            const action = collapsed ? 'Expand panel' : 'Collapse panel';
            const icon = btn.querySelector('.panel-toggle-icon');
            const label = btn.querySelector('.panel-toggle-label');
            if (icon) icon.innerHTML = collapsed ? '&#9650;' : '&#9660;';
            if (label) label.textContent = action;
            btn.title = action;
            btn.setAttribute('aria-label', action);
        }
        writeStoredBoolean(this.storageKey, !!collapsed);
        this._notifyViewportResize();
    }

    setCompactMode(isCompact) {
        if (isCompact && this._drag.active) this._handleResizePointerUp();
        this._compactMode = !!isCompact;
        if (this.resizeHandle) {
            this.resizeHandle.toggleAttribute('hidden', this._compactMode);
            this.resizeHandle.setAttribute('aria-hidden', this._compactMode ? 'true' : 'false');
        }
        if (this._railResizeHandle) {
            this._railResizeHandle.toggleAttribute('hidden', this._compactMode);
            this._railResizeHandle.setAttribute('aria-hidden', this._compactMode ? 'true' : 'false');
        }
        if (this._sideResizeHandle) {
            this._sideResizeHandle.toggleAttribute('hidden', this._compactMode);
            this._sideResizeHandle.setAttribute('aria-hidden', this._compactMode ? 'true' : 'false');
        }
        if (this._compactMode && this.panelArea) {
            this.panelArea.style.removeProperty('height');
        }
    }

    _bindTabs() {
        this._getTabs().forEach((tab) => {
            let startX = 0;
            let startY = 0;
            let startScrollTop = 0;
            let hasDragged = false;
            let dragThresholdActive = false;
            let railScrolling = false;

            const clearGesture = () => {
                dragThresholdActive = false;
                railScrolling = false;
                this.tabBar?.classList.remove('is-rail-scrolling');
                window.removeEventListener('pointermove', onPointerMove);
                window.removeEventListener('pointerup', onPointerUp);
                window.removeEventListener('pointercancel', onPointerUp);
                window.removeEventListener('blur', onPointerUp);
            };

            const onPointerMove = (e) => {
                if (!dragThresholdActive) return;
                const dx = e.clientX - startX;
                const dy = e.clientY - startY;
                const sideMount = this.tabBar?.classList.contains('panel-mount-left')
                    || this.tabBar?.classList.contains('panel-mount-right');

                // Vertical drags on the side rail are a scroll gesture. The old
                // direction-agnostic threshold floated the touched panel instead,
                // making the overflowing icon list effectively unscrollable by
                // mouse/stylus and race-prone on touch.
                if (railScrolling || (sideMount && Math.abs(dy) > 8 && Math.abs(dy) >= Math.abs(dx))) {
                    railScrolling = true;
                    hasDragged = true;
                    this.tabBar.classList.add('is-rail-scrolling');
                    // Touch keeps native momentum scrolling (`touch-action:
                    // pan-y`); mouse/stylus get the same grab-to-slide behavior
                    // by updating the rail directly.
                    if (e.pointerType !== 'touch') {
                        this.tabBar.scrollTop = startScrollTop - dy;
                        if (e.cancelable) e.preventDefault();
                    }
                    return;
                }
                if (Math.sqrt(dx * dx + dy * dy) > 15) {
                    hasDragged = true;
                    clearGesture();

                    // Float the panel!
                    const panelId = tab.dataset.panel;
                    const win = this.floatPanel(panelId, e.clientX, e.clientY);
                    
                    // Seamlessly chain pointer drag on the new window title header
                    if (win) win.startDrag(e.clientX, e.clientY);
                }
            };

            const onPointerUp = () => clearGesture();

            tab.addEventListener('pointerdown', (e) => {
                // Only left click / standard pointer touch
                if (e.button !== 0) return;
                startX = e.clientX;
                startY = e.clientY;
                startScrollTop = this.tabBar?.scrollTop || 0;
                hasDragged = false;
                dragThresholdActive = true;
                railScrolling = false;

                window.addEventListener('pointermove', onPointerMove);
                window.addEventListener('pointerup', onPointerUp);
                window.addEventListener('pointercancel', onPointerUp);
                window.addEventListener('blur', onPointerUp);
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
        this.resizeHandle.setAttribute('role', 'separator');
        this.resizeHandle.setAttribute('tabindex', '0');
        const bindHandle = (handle, sideOnly) => {
            if (!handle) return;
            handle.setAttribute('role', 'separator');
            handle.setAttribute('tabindex', '0');
            handle.addEventListener('pointerdown', (event) => {
                if (this._compactMode || event.button !== 0) return;
                const mount = this._getMount();
                if ((sideOnly && mount === 'bottom') || (!sideOnly && mount !== 'bottom')) return;
                const rect = this.panelArea.getBoundingClientRect();
                this._startResize('panel', event, mount === 'bottom' ? rect.height : rect.width, mount);
            });
            handle.addEventListener('pointermove', this._handleResizePointerMove);
            handle.addEventListener('pointerup', this._handleResizePointerUp);
            handle.addEventListener('pointercancel', this._handleResizePointerUp);
            handle.addEventListener('lostpointercapture', this._handleResizePointerUp);
            handle.addEventListener('keydown', this._handleOuterResizeKeydown);
        };
        bindHandle(this.resizeHandle, false);
        bindHandle(this._sideResizeHandle, true);
    }

    _ensureRailResizeHandle() {
        if (!this.app) return;
        let handle = document.getElementById('panel-rail-resizer');
        if (!handle) {
            handle = document.createElement('div');
            handle.id = 'panel-rail-resizer';
            handle.innerHTML = '<span aria-hidden="true"></span>';
            this.app.appendChild(handle);
        }
        handle.setAttribute('role', 'separator');
        handle.setAttribute('tabindex', '0');
        handle.setAttribute('aria-orientation', 'vertical');
        handle.setAttribute('aria-label', 'Resize panel navigation rail to show or hide panel titles');
        handle.title = 'Drag to resize panel navigation; double-click to show or hide titles';
        this._railResizeHandle = handle;

        let sideHandle = document.getElementById('panel-side-resizer');
        if (!sideHandle) {
            sideHandle = document.createElement('div');
            sideHandle.id = 'panel-side-resizer';
            sideHandle.innerHTML = '<span aria-hidden="true"></span>';
            this.app.appendChild(sideHandle);
        }
        sideHandle.setAttribute('role', 'separator');
        sideHandle.setAttribute('tabindex', '0');
        sideHandle.setAttribute('aria-orientation', 'vertical');
        sideHandle.setAttribute('aria-label', 'Resize side panel width');
        sideHandle.title = 'Drag to resize side panel width';
        this._sideResizeHandle = sideHandle;
        applyPanelMountClasses(this._getMount());
    }

    _bindRailResizeHandle() {
        const handle = this._railResizeHandle;
        if (!handle) return;
        handle.addEventListener('pointerdown', (event) => {
            if (this._compactMode || event.button !== 0 || !this._isSideMount()) return;
            this._startResize('rail', event, this._getRailWidth(), this._getMount());
        });
        handle.addEventListener('pointermove', this._handleResizePointerMove);
        handle.addEventListener('pointerup', this._handleResizePointerUp);
        handle.addEventListener('pointercancel', this._handleResizePointerUp);
        handle.addEventListener('keydown', this._handleRailResizeKeydown);
        handle.addEventListener('dblclick', () => this._toggleRailWidth());
    }

    _startResize(kind, event, startSize, mount) {
        this._drag.active = true;
        this._drag.kind = kind;
        this._drag.pointerId = event.pointerId;
        this._drag.handle = event.currentTarget;
        this._drag.mount = mount;
        this._drag.startX = event.clientX;
        this._drag.startY = event.clientY;
        this._drag.startSize = startSize;
        this._drag.pendingSize = startSize;
        this.app?.classList.add('is-panel-resizing');
        document.body.style.cursor = kind === 'rail' || mount !== 'bottom' ? 'ew-resize' : 'ns-resize';
        try { event.currentTarget.setPointerCapture(event.pointerId); } catch { /* synthetic event */ }
        event.preventDefault();
    }

    _handleResizePointerMove(event) {
        if (!this._drag.active || event.pointerId !== this._drag.pointerId || this._compactMode) return;
        const { kind, mount, startX, startY, startSize } = this._drag;
        let next;
        if (kind === 'rail') {
            const dx = event.clientX - startX;
            next = startSize + (mount === 'right' ? -dx : dx);
        } else if (mount === 'bottom') {
            next = Math.max(220, startSize + startY - event.clientY);
        } else {
            const dx = event.clientX - startX;
            next = startSize + (mount === 'right' ? -dx : dx);
        }
        this._queueResize(kind, next, mount);
        if (event.cancelable) event.preventDefault();
    }

    _queueResize(kind, size, mount) {
        this._drag.pendingSize = size;
        if (this._resizeRaf !== null) return;
        this._resizeRaf = window.requestAnimationFrame(() => {
            this._resizeRaf = null;
            if (!this._drag.active) return;
            this._applyResize(kind, this._drag.pendingSize, mount, false);
        });
    }

    _applyResize(kind, size, mount, persist) {
        if (kind === 'rail') {
            this._applyRailWidth(size, { persist });
        } else if (mount === 'bottom') {
            const maxHeight = Math.max(220, window.innerHeight - 96);
            const height = clamp(size, 220, maxHeight);
            this.panelArea.style.height = `${Math.round(height)}px`;
            this.resizeHandle?.setAttribute('aria-valuemin', '220');
            this.resizeHandle?.setAttribute('aria-valuemax', String(Math.round(maxHeight)));
            this.resizeHandle?.setAttribute('aria-valuenow', String(Math.round(height)));
        } else {
            this._applyPanelWidth(size, { persist });
        }
    }

    _handleResizePointerUp(event) {
        if (!this._drag.active || (event?.pointerId !== undefined && event.pointerId !== this._drag.pointerId)) return;
        if (this._resizeRaf !== null) {
            window.cancelAnimationFrame(this._resizeRaf);
            this._resizeRaf = null;
        }
        const { kind, mount, pendingSize, handle, pointerId } = this._drag;
        this._applyResize(kind, pendingSize, mount, true);
        // Clear first: releasePointerCapture synchronously emits
        // lostpointercapture in Chromium, which must observe an inactive drag.
        this._drag.active = false;
        try { handle?.releasePointerCapture(pointerId); } catch { /* capture may already be released */ }
        this._drag.kind = null;
        this._drag.handle = null;
        this._drag.pendingSize = null;
        this.app?.classList.remove('is-panel-resizing');
        document.body.style.cursor = '';
        this._notifyViewportResize();
    }

    _handleOuterResizeKeydown(event) {
        if (this._compactMode) return;
        const mount = this._getMount();
        const rect = this.panelArea.getBoundingClientRect();
        let next = mount === 'bottom' ? rect.height : this._getPanelWidth();
        if (event.key === 'Home') next = mount === 'bottom' ? 220 : PANEL_MIN_WIDTH;
        else if (event.key === 'End') next = mount === 'bottom' ? window.innerHeight - 96 : this._getPanelWidthBounds().max;
        else if (mount === 'bottom' && event.key === 'ArrowUp') next += RESIZE_KEY_STEP;
        else if (mount === 'bottom' && event.key === 'ArrowDown') next -= RESIZE_KEY_STEP;
        else if (mount === 'left' && event.key === 'ArrowRight') next += RESIZE_KEY_STEP;
        else if (mount === 'left' && event.key === 'ArrowLeft') next -= RESIZE_KEY_STEP;
        else if (mount === 'right' && event.key === 'ArrowLeft') next += RESIZE_KEY_STEP;
        else if (mount === 'right' && event.key === 'ArrowRight') next -= RESIZE_KEY_STEP;
        else return;
        event.preventDefault();
        this._applyResize('panel', next, mount, true);
        this._notifyViewportResize();
    }

    _handleRailResizeKeydown(event) {
        if (this._compactMode || !this._isSideMount()) return;
        if (event.key === 'Enter' || event.key === ' ') {
            event.preventDefault();
            this._toggleRailWidth();
            return;
        }
        const mount = this._getMount();
        let next = this._getRailWidth();
        if (event.key === 'Home') next = RAIL_MIN_WIDTH;
        else if (event.key === 'End') next = RAIL_EXPANDED_WIDTH;
        else if (mount === 'left' && event.key === 'ArrowRight') next += RESIZE_KEY_STEP;
        else if (mount === 'left' && event.key === 'ArrowLeft') next -= RESIZE_KEY_STEP;
        else if (mount === 'right' && event.key === 'ArrowLeft') next += RESIZE_KEY_STEP;
        else if (mount === 'right' && event.key === 'ArrowRight') next -= RESIZE_KEY_STEP;
        else return;
        event.preventDefault();
        this._applyRailWidth(next, { persist: true });
    }

    _toggleRailWidth() {
        const expanded = this._getRailWidth() >= RAIL_LABEL_THRESHOLD;
        this._applyRailWidth(expanded ? RAIL_MIN_WIDTH : RAIL_EXPANDED_WIDTH, { persist: true });
    }

    _restoreSizeState() {
        const defaultPanelWidth = Math.min(520, Math.max(380, window.innerWidth * 0.25));
        this._applyRailWidth(readStoredNumber(RAIL_WIDTH_STORAGE_KEY, RAIL_MIN_WIDTH));
        this._applyPanelWidth(readStoredNumber(PANEL_WIDTH_STORAGE_KEY, defaultPanelWidth));
    }

    _applyRailWidth(value, { persist = false } = {}) {
        const max = Math.max(RAIL_MIN_WIDTH, Math.min(240, window.innerWidth - PANEL_MIN_WIDTH - 48));
        const width = clamp(Number(value) || RAIL_MIN_WIDTH, RAIL_MIN_WIDTH, max);
        const rounded = Math.round(width);
        this.app?.style.setProperty('--panel-rail-width', `${rounded}px`);
        if (this.app?.dataset) this.app.dataset.panelRailWidth = String(rounded);
        if (this.tabBar?.dataset) this.tabBar.dataset.railExpanded = rounded >= RAIL_LABEL_THRESHOLD ? 'true' : 'false';
        this._railResizeHandle?.setAttribute('aria-valuemin', String(RAIL_MIN_WIDTH));
        this._railResizeHandle?.setAttribute('aria-valuemax', String(Math.round(max)));
        this._railResizeHandle?.setAttribute('aria-valuenow', String(rounded));
        this._railResizeHandle?.setAttribute('aria-valuetext', rounded >= RAIL_LABEL_THRESHOLD ? 'Panel titles shown' : 'Icons only');
        if (persist) writeStoredNumber(RAIL_WIDTH_STORAGE_KEY, rounded);
        this._applyPanelWidth(this._getPanelWidth(), { persist: false });
        updateSafeEdges(this._getMount());
        return rounded;
    }

    _applyPanelWidth(value, { persist = false } = {}) {
        const { min, max } = this._getPanelWidthBounds();
        const width = clamp(Number(value) || min, min, max);
        const rounded = Math.round(width);
        this.app?.style.setProperty('--panel-width-left', `${rounded}px`);
        this.app?.style.setProperty('--panel-width-right', `${rounded}px`);
        if (this.app?.dataset) this.app.dataset.panelSideWidth = String(rounded);
        this.resizeHandle?.setAttribute('aria-valuemin', String(Math.round(min)));
        this.resizeHandle?.setAttribute('aria-valuemax', String(Math.round(max)));
        this.resizeHandle?.setAttribute('aria-valuenow', String(rounded));
        this._sideResizeHandle?.setAttribute('aria-valuemin', String(Math.round(min)));
        this._sideResizeHandle?.setAttribute('aria-valuemax', String(Math.round(max)));
        this._sideResizeHandle?.setAttribute('aria-valuenow', String(rounded));
        if (persist) writeStoredNumber(PANEL_WIDTH_STORAGE_KEY, rounded);
        updateSafeEdges(this._getMount());
        return rounded;
    }

    _getPanelWidthBounds() {
        const rail = this._getRailWidth();
        return {
            min: Math.min(PANEL_MIN_WIDTH, Math.max(220, window.innerWidth - rail - 48)),
            max: Math.max(PANEL_MIN_WIDTH, Math.min(window.innerWidth * 0.72, window.innerWidth - rail - 48)),
        };
    }

    _getPanelWidth() {
        const value = Number(this.app?.dataset?.panelSideWidth);
        return Number.isFinite(value) && value > 0
            ? value
            : Math.min(520, Math.max(380, window.innerWidth * 0.25));
    }

    _getRailWidth() {
        const value = Number(this.app?.dataset?.panelRailWidth);
        return Number.isFinite(value) && value > 0 ? value : RAIL_MIN_WIDTH;
    }

    _getMount() {
        return document.documentElement.dataset.panelMount || 'left';
    }

    _isSideMount() {
        const mount = this._getMount();
        return mount === 'left' || mount === 'right';
    }

    _handleWindowResize() {
        if (this._windowResizeRaf !== null) return;
        this._windowResizeRaf = window.requestAnimationFrame(() => {
            this._windowResizeRaf = null;
            this._applyRailWidth(this._getRailWidth());
            this._applyPanelWidth(this._getPanelWidth());
        });
    }

    _restoreCollapsedState() {
        const isCollapsed = readStoredBoolean(this.storageKey, false);
        this.setCollapsed(isCollapsed);
    }

    _notifyViewportResize() {
        if (this.onViewportResize) {
            if (this._viewportResizeTimer !== null) {
                window.clearTimeout(this._viewportResizeTimer);
            }
            this._viewportResizeTimer = window.setTimeout(() => {
                this._viewportResizeTimer = null;
                this.onViewportResize();
            }, 250);
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
