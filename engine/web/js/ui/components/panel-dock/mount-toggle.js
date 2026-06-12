import {
    readPanelMount,
    writePanelMount,
    resolveEffectiveMount,
    getSideMountMinWidth,
} from '../../shell/panel-mount-state.js';
import { BaseLifecycleController } from '../../../lifecycle.js';

const GLYPHS = Object.freeze({
    left:   '\u25EA',
    bottom: '\u25A2',
    right:  '\u25E9',
});

const LABELS = Object.freeze({
    left:   'Dock to left (Ctrl+Shift+Left)',
    bottom: 'Dock to bottom (Ctrl+Shift+Down)',
    right:  'Dock to right (Ctrl+Shift+Right)',
});

const SHORTCUT_MAP = Object.freeze({
    ArrowLeft:  'left',
    ArrowDown:  'bottom',
    ArrowRight: 'right',
});

const USER_MOUNTS = Object.freeze(['left', 'bottom', 'right']);

/**
 * Updates --viewport-safe-left / --viewport-safe-right on <html> so that any
 * overlay consumers can inset themselves past the sidebar without hardcoding
 * the sidebar width.  Called on every mount change and on first init.
 */
export function updateSafeEdges(mount) {
    const root = document.documentElement;
    const leftW  = parseFloat(root.style.getPropertyValue('--panel-width-left'))  || 
                   parseFloat(getComputedStyle(root).getPropertyValue('--panel-width-left')) || 380;
    const rightW = parseFloat(root.style.getPropertyValue('--panel-width-right')) || 
                   parseFloat(getComputedStyle(root).getPropertyValue('--panel-width-right')) || 380;
    const gap    = 12;
    const tabW   = 50; // icon-rail width + gap
    switch (mount) {
        case 'left':
            root.style.setProperty('--viewport-safe-left',  `${leftW + tabW + gap}px`);
            root.style.setProperty('--viewport-safe-right', '0px');
            break;
        case 'right':
            root.style.setProperty('--viewport-safe-left',  '0px');
            root.style.setProperty('--viewport-safe-right', `${rightW + tabW + gap}px`);
            break;
        default:
            root.style.setProperty('--viewport-safe-left',  '0px');
            root.style.setProperty('--viewport-safe-right', '0px');
    }
}

export class MountToggleComponent extends BaseLifecycleController {
    constructor(root) {
        super();
        this.root = root;
        this._observer = null;
        this._keydown = this._keydown.bind(this);
        this._click = this._click.bind(this);
        this._onResize = this._onResize.bind(this);
    }

    init() {
        if (!this.root || this.root.dataset.panelMountToggle === 'true') return this;
        this.root.dataset.panelMountToggle = 'true';
        this.root.setAttribute('role', 'group');
        this.root.setAttribute('aria-label', 'Panel dock position');
        this.root.innerHTML = USER_MOUNTS.map((mount) => (
            `<button type="button" class="mount-toggle-btn" data-mount="${mount}" ` +
            `title="${LABELS[mount]}" aria-label="${LABELS[mount]}" aria-pressed="false">` +
            `<span aria-hidden="true">${GLYPHS[mount]}</span>` +
            `</button>`
        )).join('');

        this.bindEvent(this.root, 'click', this._click);
        this.bindEvent(window, 'keydown', this._keydown);
        this.bindEvent(window, 'resize', this._onResize);

        // Watch html[data-panel-mount] for programmatic updates (e.g., from integration tests)
        this._observer = new MutationObserver((mutations) => {
            for (const mutation of mutations) {
                if (mutation.attributeName === 'data-panel-mount') {
                    const next = document.documentElement.dataset.panelMount;
                    updateSafeEdges(next);
                    this._sync(next);
                    this._updateDisabledState();
                }
            }
        });
        this._observer.observe(document.documentElement, {
            attributes: true,
            attributeFilter: ['data-panel-mount'],
        });
        const current = readPanelMount();
        const effective = resolveEffectiveMount(current, window.innerWidth);
        if (effective !== current) {
            document.documentElement.dataset.panelMount = effective;
        }
        updateSafeEdges(effective);
        this._sync(effective);
        this._updateDisabledState();
        return this;
    }

    _click(event) {
        const btn = event.target.closest('button[data-mount]');
        if (!btn || btn.getAttribute('aria-disabled') === 'true') return;
        this._apply(btn.dataset.mount);
    }

    _keydown(event) {
        if (!event.ctrlKey || !event.shiftKey) return;
        const mount = SHORTCUT_MAP[event.key];
        if (!mount) return;
        const effective = resolveEffectiveMount(mount, window.innerWidth);
        event.preventDefault();
        this._apply(effective);
    }

    _onResize() {
        const stored = readPanelMount();
        const effective = resolveEffectiveMount(stored, window.innerWidth);
        document.documentElement.dataset.panelMount = effective;
        updateSafeEdges(effective);
        this._sync(effective);
        this._updateDisabledState();
    }

    _apply(mount) {
        const next = writePanelMount(mount);
        updateSafeEdges(next);
        this._sync(next);
        this._updateDisabledState();
        this.root.dispatchEvent(new CustomEvent('mountchange', {
            detail: { mount: next },
            bubbles: true,
        }));
    }

    _sync(active) {
        this.root.querySelectorAll('button[data-mount]').forEach((btn) => {
            const pressed = btn.dataset.mount === active;
            btn.setAttribute('aria-pressed', pressed ? 'true' : 'false');
            btn.classList.toggle('is-active', pressed);
        });
    }

    _updateDisabledState() {
        const tooNarrow = window.innerWidth < getSideMountMinWidth();
        this.root.querySelectorAll('button[data-mount]').forEach((btn) => {
            const isSide = btn.dataset.mount === 'left' || btn.dataset.mount === 'right';
            if (isSide && tooNarrow) {
                btn.setAttribute('aria-disabled', 'true');
                btn.setAttribute('tabindex', '-1');
            } else {
                btn.removeAttribute('aria-disabled');
                btn.setAttribute('tabindex', '0');
            }
        });
    }

    /**
     * Tear down all resources acquired in init().
     *
     * super.destroy() removes the three tracked listeners (root 'click',
     * window 'keydown', window 'resize'). The MutationObserver is NOT tracked
     * by the base class, so it is disconnected explicitly here.
     *
     * Safe to call multiple times: super.destroy() empties its listener list,
     * and disconnect() on an already-disconnected observer is a no-op.
     */
    destroy(ctx) {
        super.destroy(ctx);
        this._observer?.disconnect();
        this._observer = null;
    }
}
