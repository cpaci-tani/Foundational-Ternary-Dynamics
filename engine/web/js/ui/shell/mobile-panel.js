/**
 * MobilePanelController — touch swipe-to-dismiss and body scroll lock
 * for the bottom-sheet panel on mobile (≤767px).
 *
 * Works alongside PanelDockController: does NOT duplicate tab or collapse
 * logic, only adds touch and scroll-lock behavior that desktop doesn't need.
 *
 * Swipe-to-dismiss: drag the grip handle (or sheet header) downward ≥60px
 * to collapse; drag upward ≥40px while collapsed to expand.
 *
 * Body scroll lock: adds `.body-panel-open` to <body> when panel is open on
 * mobile, preventing background rubber-band scroll on iOS.
 */

const SWIPE_DISMISS_THRESHOLD = 60;   // px downward to collapse
const SWIPE_OPEN_THRESHOLD    = 40;   // px upward to expand
const MOBILE_MAX_WIDTH        = 767;

export class MobilePanelController {
    constructor({ app, panelArea, resizer, dockController }) {
        this.app            = app;
        this.panelArea      = panelArea;
        this.resizer        = resizer;   // grip-handle pill (#panel-resizer)
        this.dockController = dockController;

        this._touch  = { active: false, startY: 0, lastY: 0, startCollapsed: false };
        this._obs    = null;

        this._onTouchStart = this._onTouchStart.bind(this);
        this._onTouchMove  = this._onTouchMove.bind(this);
        this._onTouchEnd   = this._onTouchEnd.bind(this);
    }

    init() {
        if (!this.panelArea) return this;

        // Watch for panels-collapsed attribute changes to toggle body scroll lock.
        this._obs = new MutationObserver(() => this._syncScrollLock());
        this._obs.observe(this.app, { attributes: true, attributeFilter: ['data-panels-collapsed'] });
        this._syncScrollLock();

        // Attach touch listeners to the grip handle and the panel header area.
        const targets = [this.resizer, this.panelArea].filter(Boolean);
        targets.forEach((el) => {
            el.addEventListener('touchstart', this._onTouchStart, { passive: true });
            el.addEventListener('touchmove',  this._onTouchMove,  { passive: false });
            el.addEventListener('touchend',   this._onTouchEnd,   { passive: true });
        });

        return this;
    }

    destroy() {
        this._obs?.disconnect();
        const targets = [this.resizer, this.panelArea].filter(Boolean);
        targets.forEach((el) => {
            el.removeEventListener('touchstart', this._onTouchStart);
            el.removeEventListener('touchmove',  this._onTouchMove);
            el.removeEventListener('touchend',   this._onTouchEnd);
        });
        document.body.classList.remove('body-panel-open');
    }

    // ── Touch handlers ───────────────────────────────────────────────────────

    _onTouchStart(e) {
        if (!this._isMobile()) return;
        const t = e.touches[0];
        this._touch.active        = true;
        this._touch.startY        = t.clientY;
        this._touch.lastY         = t.clientY;
        this._touch.startCollapsed = this.app.classList.contains('panels-collapsed');
    }

    _onTouchMove(e) {
        if (!this._touch.active) return;
        const t  = e.touches[0];
        const dy = t.clientY - this._touch.startY;

        // Prevent page scroll while swiping the grip handle
        if (this._isOnResizer(e) || Math.abs(dy) > 10) {
            e.preventDefault();
        }

        this._touch.lastY = t.clientY;

        // Live feedback: nudge the panel with the finger (clamp to 0..100px downward)
        if (!this._touch.startCollapsed) {
            const nudge = Math.max(0, Math.min(dy, 100));
            if (this.panelArea) {
                this.panelArea.style.transform = nudge > 0
                    ? `translateY(${nudge}px)`
                    : 'translateY(0)';
            }
        }
    }

    _onTouchEnd() {
        if (!this._touch.active) return;
        this._touch.active = false;

        // Reset live transform before letting CSS take over
        if (this.panelArea) this.panelArea.style.transform = '';

        const dy = this._touch.lastY - this._touch.startY;

        if (!this._touch.startCollapsed && dy >= SWIPE_DISMISS_THRESHOLD) {
            this.dockController?.setCollapsed(true);
        } else if (this._touch.startCollapsed && dy <= -SWIPE_OPEN_THRESHOLD) {
            this.dockController?.setCollapsed(false);
        }
    }

    // ── Scroll lock ──────────────────────────────────────────────────────────

    _syncScrollLock() {
        if (!this._isMobile()) {
            document.body.classList.remove('body-panel-open');
            return;
        }
        const collapsed = this.app.dataset.panelsCollapsed === 'true';
        document.body.classList.toggle('body-panel-open', !collapsed);
    }

    // ── Helpers ──────────────────────────────────────────────────────────────

    _isMobile() {
        return window.innerWidth <= MOBILE_MAX_WIDTH;
    }

    _isOnResizer(e) {
        return this.resizer && this.resizer.contains(e.target);
    }
}
