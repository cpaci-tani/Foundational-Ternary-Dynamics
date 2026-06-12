/**
 * FloatingWindow — high-performance draggable/resizable glassmorphic panel wrapper.
 * Detaches any DOM panel and mounts it inside a viewport-floating window.
 */

let _activeWindows = [];
let _maxZIndex = 1000;

export class FloatingWindow {
    constructor(panelId, opts = {}) {
        this.panelId = panelId;
        this.title = opts.title || 'Panel';
        this.icon = opts.icon || '⚙️';
        this.panelEl = opts.panelEl;
        this.initialPos = opts.initialPos || { x: 100, y: 100 };
        this.onDock = typeof opts.onDock === 'function' ? opts.onDock : null;

        this.el = null;
        this.header = null;
        this.body = null;
        this.isCollapsed = false;

        this._drag = { active: false, offsetX: 0, offsetY: 0 };
        this._onPointerDown = this._onPointerDown.bind(this);
        this._onPointerMove = this._onPointerMove.bind(this);
        this._onPointerUp = this._onPointerUp.bind(this);
        this._ro = null;
    }

    init() {
        if (!this.panelEl) return this;

        // 1. Create window DOM element
        this.el = document.createElement('div');
        this.el.className = 'floating-window';
        this.el.dataset.panelId = this.panelId;
        this.el.style.left = `${this.initialPos.x}px`;
        this.el.style.top = `${this.initialPos.y}px`;
        this.focus();

        // 2. Build header and title
        this.el.innerHTML = `
            <header class="floating-window-header">
                <div class="floating-window-title">
                    <span class="floating-window-icon" aria-hidden="true">${this.icon}</span>
                    <span>${this.title}</span>
                </div>
                <div class="floating-window-controls">
                    <button class="floating-window-btn btn-collapse" title="Minimize/Restore">—</button>
                    <button class="floating-window-btn btn-close" title="Dock Panel">✕</button>
                </div>
            </header>
            <div class="floating-window-body"></div>
        `;

        this.header = this.el.querySelector('.floating-window-header');
        this.body = this.el.querySelector('.floating-window-body');

        // 3. Move panel element into the floating body
        this.body.appendChild(this.panelEl);

        // 4. Attach event listeners
        this.header.addEventListener('pointerdown', this._onPointerDown);
        this.el.addEventListener('pointerdown', () => this.focus());

        // Minimize / Expand buttons
        this.el.querySelector('.btn-collapse').addEventListener('click', (e) => {
            e.stopPropagation();
            this.toggleCollapse();
        });
        this.header.addEventListener('dblclick', () => this.toggleCollapse());

        // Close/Dock button
        this.el.querySelector('.btn-close').addEventListener('click', (e) => {
            e.stopPropagation();
            this.dock();
        });

        // 5. Setup Resize Observer to notify embedded uPlot instances
        this._ro = new ResizeObserver(() => this.triggerChartResize());
        this._ro.observe(this.el);

        // Append to application root
        const app = document.getElementById('app') || document.body;
        app.appendChild(this.el);

        _activeWindows.push(this);

        return this;
    }

    focus() {
        if (this.el.style.zIndex != _maxZIndex) {
            _maxZIndex += 1;
            this.el.style.zIndex = _maxZIndex;
            _activeWindows.forEach(win => win.el.classList.remove('active-focus'));
            this.el.classList.add('active-focus');
        }
    }

    toggleCollapse() {
        this.isCollapsed = !this.isCollapsed;
        this.el.classList.toggle('is-collapsed', this.isCollapsed);
        const btn = this.el.querySelector('.btn-collapse');
        if (btn) {
            btn.textContent = this.isCollapsed ? '⛶' : '—';
            btn.title = this.isCollapsed ? 'Restore' : 'Minimize';
        }
        this.triggerChartResize();
    }

    dock() {
        if (this.onDock) {
            this.onDock(this.panelId, this.panelEl);
        }
        this.destroy();
    }

    destroy() {
        if (this._ro) {
            this._ro.disconnect();
            this._ro = null;
        }
        if (this.el) {
            this.el.remove();
        }
        _activeWindows = _activeWindows.filter(win => win !== this);
    }

    triggerChartResize() {
        if (!this.body) return;
        // Trigger ResizeObserver callbacks for nested elements with custom resize functions (like uPlot charts)
        const resizableEls = this.body.querySelectorAll('*');
        resizableEls.forEach(el => {
            if (typeof el._ftdResize === 'function') {
                el._ftdResize();
            }
        });
    }

    // ── Drag mechanics ───────────────────────────────────────────────────────

    startDrag(clientX, clientY) {
        this.focus();
        const rect = this.el.getBoundingClientRect();
        this._drag.active = true;
        this._drag.offsetX = clientX - rect.left;
        this._drag.offsetY = clientY - rect.top;
        this.el.classList.add('is-dragging');

        window.addEventListener('pointermove', this._onPointerMove);
        window.addEventListener('pointerup', this._onPointerUp);
    }

    _onPointerDown(e) {
        if (e.target.closest('.floating-window-btn')) return;
        this.startDrag(e.clientX, e.clientY);
        e.preventDefault();
    }

    _onPointerMove(e) {
        if (!this._drag.active) return;
        let x = e.clientX - this._drag.offsetX;
        let y = e.clientY - this._drag.offsetY;

        // Bound checking to keep header visible on screen
        const padding = 20;
        x = Math.max(padding - this.el.offsetWidth, Math.min(window.innerWidth - padding, x));
        y = Math.max(0, Math.min(window.innerHeight - 38, y));

        this.el.style.left = `${x}px`;
        this.el.style.top = `${y}px`;
    }

    _onPointerUp() {
        if (!this._drag.active) return;
        this._drag.active = false;
        this.el.classList.remove('is-dragging');
        window.removeEventListener('pointermove', this._onPointerMove);
        window.removeEventListener('pointerup', this._onPointerUp);
        this.triggerChartResize();
    }
}

export class FloatingWindowManager {
    constructor() {
        this.windows = new Map(); // panelId -> FloatingWindow
    }

    floatPanel(panelId, title, icon, panelEl, initialPos, onDock) {
        if (this.windows.has(panelId)) {
            const win = this.windows.get(panelId);
            win.focus();
            return win;
        }

        const win = new FloatingWindow(panelId, {
            title,
            icon,
            panelEl,
            initialPos,
            onDock: (id, el) => {
                this.windows.delete(id);
                if (onDock) onDock(id, el);
            }
        }).init();

        this.windows.set(panelId, win);
        return win;
    }

    dockAll() {
        for (const [_id, win] of this.windows) {
            win.dock();
        }
    }

    getWindow(panelId) {
        return this.windows.get(panelId) || null;
    }

    has(panelId) {
        return this.windows.has(panelId);
    }
}

export const floatingWindowManager = new FloatingWindowManager();
