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
const MOUNT_CLASSES = Object.freeze(USER_MOUNTS.map((mount) => `panel-mount-${mount}`));
const MOUNT_CLASS_TARGETS = Object.freeze([
    '#panel-area',
    '#panel-resizer',
    '#panel-rail-resizer',
    '#panel-side-resizer',
    '#tab-bar',
    '.play-bar',
    '.viewport-overlay-panel',
    '#viewport-overlay',
    '.viewport-overlay-bottom',
]);

/**
 * Mirror the canonical html[data-panel-mount] value onto only the shell nodes
 * whose layout actually depends on it. Keeping mount selectors off <html>
 * prevents every dock-position click from invalidating styles for the entire
 * 5k-node dashboard (including all hidden panel canvases).
 */
export function applyPanelMountClasses(mount) {
    const normalizedMount = USER_MOUNTS.includes(mount) ? mount : 'left';
    const nextClass = `panel-mount-${normalizedMount}`;
    document.querySelectorAll(MOUNT_CLASS_TARGETS.join(',')).forEach((element) => {
        if (!element.classList.contains(nextClass)) {
            element.classList.remove(...MOUNT_CLASSES);
            element.classList.add(nextClass);
        }
    });

    const activePanel = document.querySelector('#panel-area .panel.active');
    document.querySelectorAll(
        '#panel-area .panel.panel-mount-left,' +
        '#panel-area .panel.panel-mount-bottom,' +
        '#panel-area .panel.panel-mount-right',
    ).forEach((panel) => {
        if (panel !== activePanel || !panel.classList.contains(nextClass)) {
            panel.classList.remove(...MOUNT_CLASSES);
        }
    });
    if (activePanel && !activePanel.classList.contains(nextClass)) {
        activePanel.classList.add(nextClass);
    }

    const panelResizer = document.getElementById('panel-resizer');
    if (panelResizer) {
        const side = normalizedMount !== 'bottom';
        panelResizer.setAttribute('aria-orientation', side ? 'vertical' : 'horizontal');
        panelResizer.setAttribute('aria-label', side ? 'Resize side panel width' : 'Resize panel height');
        panelResizer.title = side ? 'Drag to resize side panel width' : 'Drag to resize panel height';
    }
    const sideResizer = document.getElementById('panel-side-resizer');
    if (sideResizer) {
        sideResizer.setAttribute('aria-orientation', 'vertical');
        sideResizer.setAttribute('aria-label', 'Resize side panel width');
    }
}

/**
 * Updates --viewport-safe-left / --viewport-safe-right on #viewport so overlay
 * consumers can inset themselves past the sidebar without invalidating the
 * unrelated panel/document trees. Called on every mount change and first init.
 */
export function updateSafeEdges(mount) {
    const root = document.getElementById('viewport') || document.documentElement;
    const shell = document.getElementById('app');
    // The resize controller mirrors both live numeric widths into data attrs.
    // Reading those values avoids getComputedStyle() in the mount hot path
    // (which previously produced 50–95 ms synchronous style tasks).
    const storedPanelWidth = Number(shell?.dataset.panelSideWidth);
    const storedRailWidth = Number(shell?.dataset.panelRailWidth);
    const panelWidth = Number.isFinite(storedPanelWidth) && storedPanelWidth > 0
        ? storedPanelWidth
        : Math.min(520, Math.max(380, window.innerWidth * 0.25));
    const railWidth = Number.isFinite(storedRailWidth) && storedRailWidth > 0
        ? storedRailWidth
        : 44;
    // 12px viewport gap + 6px rail-to-panel gap.
    const inset = `${Math.round(panelWidth + railWidth + 18)}px`;
    switch (mount) {
        case 'left':
            root.style.setProperty('--viewport-safe-left', inset);
            root.style.setProperty('--viewport-safe-right', '0px');
            break;
        case 'right':
            root.style.setProperty('--viewport-safe-left',  '0px');
            root.style.setProperty('--viewport-safe-right', inset);
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
        this._resizeRaf = null;
        this._syncedMount = null;
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
                    if (next !== this._syncedMount) this._syncMountState(next);
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
        this._syncMountState(effective);
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
        if (this._resizeRaf !== null) return;
        this._resizeRaf = window.requestAnimationFrame(() => {
            this._resizeRaf = null;
            const stored = readPanelMount();
            const effective = resolveEffectiveMount(stored, window.innerWidth);
            if (document.documentElement.dataset.panelMount !== effective) {
                document.documentElement.dataset.panelMount = effective;
            }
            this._syncMountState(effective);
        });
    }

    _apply(mount) {
        const next = writePanelMount(mount);
        this._syncMountState(next);
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

    _syncMountState(mount) {
        applyPanelMountClasses(mount);
        updateSafeEdges(mount);
        this._sync(mount);
        this._updateDisabledState();
        this._syncedMount = mount;
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
        if (this._resizeRaf !== null) {
            window.cancelAnimationFrame(this._resizeRaf);
            this._resizeRaf = null;
        }
        this._observer?.disconnect();
        this._observer = null;
        this._syncedMount = null;
        if (this.root?.dataset) delete this.root.dataset.panelMountToggle;
    }
}
