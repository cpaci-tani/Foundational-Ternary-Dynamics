/** Keyboard Help Overlay — renders the shared dashboard shortcut contract. */
import { DASHBOARD_SHORTCUTS, isEditableTarget } from '../../shortcuts.js';

export class KeyboardHelpComponent {
    constructor() {
        this._overlayEl = null;
        this._onKey = null;
        this._onOverlayClick = null;
        this._onCloseClick = null;
        this._returnFocus = null;
    }

    init() {
        if (this._overlayEl) return this;
        const existing = document.getElementById('keyboard-help-overlay');
        if (existing) {
            this._overlayEl = existing;
        } else {
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
        }
        const overlay = this._overlayEl;
        this._onOverlayClick = (event) => {
            if (event.target === overlay) this.hide();
        };
        overlay.addEventListener('click', this._onOverlayClick);
        const closeBtn = overlay.querySelector('.kbd-help-close');
        this._onCloseClick = () => this.hide();
        closeBtn?.addEventListener('click', this._onCloseClick);
        this._onKey = (event) => {
            if (isEditableTarget(event.target)) return;
            if (event.key === '?' && !event.ctrlKey && !event.metaKey) {
                event.preventDefault();
                this.toggle();
            } else if (event.key === 'Escape' && !overlay.hidden) {
                this.hide();
            }
        };
        document.addEventListener('keydown', this._onKey);
        return this;
    }

    toggle() { if (this._overlayEl?.hidden) this.show(); else this.hide(); }
    show() {
        if (!this._overlayEl || !this._overlayEl.hidden) return;
        this._returnFocus = document.activeElement instanceof HTMLElement ? document.activeElement : null;
        this._overlayEl.hidden = false;
        this._overlayEl.querySelector('.kbd-help-close')?.focus();
    }

    hide() {
        if (!this._overlayEl?.hidden) this._overlayEl.hidden = true;
        const returnFocus = this._returnFocus;
        this._returnFocus = null;
        if (returnFocus?.isConnected && !returnFocus.closest('[hidden]')) returnFocus.focus();
    }

    _buildInnerHTML() {
        const section = (group) => `<section class="kbd-help-section"><h3 class="kbd-help-section-title">${group.group}</h3><dl class="kbd-help-list">${group.rows.map((row) => `<div class="kbd-help-row"><dt class="kbd-help-keys">${row.keys.map((key) => `<kbd>${key}</kbd>`).join('<span class="kbd-help-sep">+</span>')}</dt><dd class="kbd-help-label">${row.label}</dd></div>`).join('')}</dl></section>`;
        return `<div class="kbd-help-panel" tabindex="-1"><header class="kbd-help-header"><h2 class="kbd-help-title">Keyboard shortcuts</h2><button class="kbd-help-close" type="button" aria-label="Close shortcuts overlay">&#10005;</button></header><div class="kbd-help-body">${DASHBOARD_SHORTCUTS.map(section).join('')}</div><footer class="kbd-help-footer">Press <kbd>?</kbd> or <kbd>Esc</kbd> to close</footer></div>`;
    }

    cleanup() {
        document.removeEventListener('keydown', this._onKey);
        this._overlayEl?.removeEventListener('click', this._onOverlayClick);
        this._overlayEl?.querySelector('.kbd-help-close')?.removeEventListener('click', this._onCloseClick);
        this._overlayEl?.remove();
        this._overlayEl = null;
        this._onKey = null;
        this._onOverlayClick = null;
        this._onCloseClick = null;
        this._returnFocus = null;
    }

    destroy() { this.cleanup(); }
}
