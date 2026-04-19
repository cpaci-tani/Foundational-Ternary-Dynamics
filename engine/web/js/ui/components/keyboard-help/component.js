/**
 * Keyboard Help Overlay — press `?` to toggle a modal listing every
 * keyboard shortcut the dashboard supports.
 *
 * The list below is hand-maintained because shortcuts are bound across
 * half a dozen places (app_dag.js, scale-specific bindings, scrub-bar
 * component). Grouped by category so each section reads like a real
 * reference card.
 *
 * To add a shortcut: wire the key binding in the relevant module AND
 * add a row here. The mismatch is easy to spot because the row won't
 * fire — run the UI, press `?`, verify the row you added actually does
 * what the caption says.
 */

const SHORTCUTS = [
    { group: 'Playback', rows: [
        { keys: ['Space'],      label: 'Play / Pause (global)' },
        { keys: ['Shift', 'Space'], label: 'Play / Pause (local scenario)' },
        { keys: ['S'],          label: 'Step one tick' },
        { keys: ['R'],          label: 'Reset scenario' },
    ] },
    { group: 'Scale 0 overlays', rows: [
        { keys: ['1'], label: 'Toggle E field' },
        { keys: ['2'], label: 'Toggle B field' },
        { keys: ['3'], label: 'Toggle Poynting vector' },
        { keys: ['4'], label: 'Toggle \u2207\u00b7J (divergence)' },
        { keys: ['5'], label: 'Toggle flux streamlines' },
        { keys: ['6'], label: 'Toggle EM force' },
        { keys: ['7'], label: 'Toggle gravity force' },
        { keys: ['8'], label: 'Toggle strong force' },
        { keys: ['9'], label: 'Toggle weak force' },
    ] },
    { group: 'Help', rows: [
        { keys: ['?'],     label: 'Show this keyboard shortcuts list' },
        { keys: ['Esc'],   label: 'Close this overlay (or the settings popover)' },
    ] },
];

export class KeyboardHelpComponent {
    constructor() {
        this._overlayEl = null;
        this._onKey = null;
    }

    init() {
        // Build the overlay DOM up front but keep it hidden. Toggled via
        // `?` key; the cost of the hidden element is a handful of bytes.
        const overlay = document.createElement('div');
        overlay.id = 'keyboard-help-overlay';
        overlay.className = 'kbd-help-overlay';
        overlay.setAttribute('role', 'dialog');
        overlay.setAttribute('aria-modal', 'true');
        overlay.setAttribute('aria-label', 'Keyboard shortcuts');
        overlay.hidden = true;
        overlay.innerHTML = this._buildInnerHTML();
        document.body.appendChild(overlay);
        this._overlayEl = overlay;

        // Close on click-outside-panel or Escape.
        overlay.addEventListener('click', (e) => {
            if (e.target === overlay) this.hide();
        });
        const closeBtn = overlay.querySelector('.kbd-help-close');
        if (closeBtn) closeBtn.addEventListener('click', () => this.hide());

        this._onKey = (e) => {
            // Only listen when the user isn't typing into a form control.
            const tag = (e.target && e.target.tagName) || '';
            if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
            if (e.key === '?' && !e.ctrlKey && !e.metaKey) {
                e.preventDefault();
                this.toggle();
            } else if (e.key === 'Escape' && !overlay.hidden) {
                this.hide();
            }
        };
        document.addEventListener('keydown', this._onKey);

        return this;
    }

    toggle() {
        if (!this._overlayEl) return;
        if (this._overlayEl.hidden) this.show();
        else this.hide();
    }

    show() {
        if (!this._overlayEl) return;
        this._overlayEl.hidden = false;
        this._overlayEl.focus?.();
    }

    hide() {
        if (!this._overlayEl) return;
        this._overlayEl.hidden = true;
    }

    _buildInnerHTML() {
        const section = (g) => `
            <section class="kbd-help-section">
                <h3 class="kbd-help-section-title">${g.group}</h3>
                <dl class="kbd-help-list">
                    ${g.rows.map((r) => `
                        <div class="kbd-help-row">
                            <dt class="kbd-help-keys">
                                ${r.keys.map((k) => `<kbd>${k}</kbd>`).join('<span class="kbd-help-sep">+</span>')}
                            </dt>
                            <dd class="kbd-help-label">${r.label}</dd>
                        </div>
                    `).join('')}
                </dl>
            </section>
        `;
        return `
            <div class="kbd-help-panel" tabindex="-1">
                <header class="kbd-help-header">
                    <h2 class="kbd-help-title">Keyboard shortcuts</h2>
                    <button class="kbd-help-close" type="button" aria-label="Close shortcuts overlay">&#10005;</button>
                </header>
                <div class="kbd-help-body">
                    ${SHORTCUTS.map(section).join('')}
                </div>
                <footer class="kbd-help-footer">
                    Press <kbd>?</kbd> or <kbd>Esc</kbd> to close
                </footer>
            </div>
        `;
    }

    cleanup() {
        if (this._onKey) document.removeEventListener('keydown', this._onKey);
        if (this._overlayEl) this._overlayEl.remove();
        this._overlayEl = null;
        this._onKey = null;
    }
}
