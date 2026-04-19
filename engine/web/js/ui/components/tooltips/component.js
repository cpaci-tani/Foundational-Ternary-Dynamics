import { applyUiTooltipDefinitions } from './definitions.js';
import { renderMathInHtml } from '../../math-format/render.js';

function escapeHtml(s) {
    return String(s ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
}

function promoteNativeTitles(root) {
    if (!(root instanceof Document) && !(root instanceof HTMLElement) && !(root instanceof DocumentFragment)) return;
    if (root instanceof HTMLElement && root.hasAttribute('title') && !root.hasAttribute('data-ui-tooltip-skip')) {
        const title = root.getAttribute('title');
        if (title) {
            if (!root.dataset.uiTooltip || root.dataset.uiTooltipSource === 'title') {
                root.dataset.uiTooltip = title;
                root.dataset.uiTooltipSource = 'title';
            }
            root.removeAttribute('title');
        }
    }
    root.querySelectorAll?.('[title]:not([data-ui-tooltip-skip])').forEach((el) => {
        if (!(el instanceof HTMLElement)) return;
        const title = el.getAttribute('title');
        if (!title) return;
        if (!el.dataset.uiTooltip || el.dataset.uiTooltipSource === 'title') {
            el.dataset.uiTooltip = title;
            el.dataset.uiTooltipSource = 'title';
        }
        el.removeAttribute('title');
    });
}

export class TooltipComponent {
    constructor({ app = null } = {}) {
        this.app = app || document.getElementById('app');
        this.tooltipEl = null;
        this.activeTarget = null;
        this.activeMode = null;
        this.pointerX = 0;
        this.pointerY = 0;
        this._observer = null;
    }

    init() {
        this._ensureTooltipElement();
        this.annotate(this.app || document);
        this._bindEvents();
        this._watchMutations();
        return this;
    }

    annotate(root = this.app || document) {
        promoteNativeTitles(root);
        applyUiTooltipDefinitions(root);
    }

    _ensureTooltipElement() {
        if (this.tooltipEl) return;
        const existing = document.getElementById('ui-tooltip');
        if (existing) {
            this.tooltipEl = existing;
            return;
        }
        const el = document.createElement('div');
        el.id = 'ui-tooltip';
        el.className = 'ui-tooltip';
        el.setAttribute('role', 'tooltip');
        el.hidden = true;
        document.body.appendChild(el);
        this.tooltipEl = el;
    }

    _bindEvents() {
        document.addEventListener('pointerover', (event) => {
            const target = event.target instanceof Element ? event.target.closest('[data-ui-tooltip]') : null;
            if (!(target instanceof HTMLElement)) return;
            this.pointerX = event.clientX;
            this.pointerY = event.clientY;
            this.show(target, 'pointer');
        });

        document.addEventListener('pointermove', (event) => {
            if (this.activeMode !== 'pointer') return;
            this.pointerX = event.clientX;
            this.pointerY = event.clientY;
            this._position();
        });

        document.addEventListener('pointerout', (event) => {
            if (!this.activeTarget) return;
            const next = event.relatedTarget instanceof Node ? event.relatedTarget : null;
            if (next && this.activeTarget.contains(next)) return;
            if (event.target instanceof Node && this.activeTarget.contains(event.target)) this.hide();
        });

        document.addEventListener('focusin', (event) => {
            const target = event.target instanceof Element ? event.target.closest('[data-ui-tooltip]') : null;
            if (target instanceof HTMLElement) this.show(target, 'focus');
        });

        document.addEventListener('focusout', (event) => {
            if (!this.activeTarget) return;
            const next = event.relatedTarget instanceof Node ? event.relatedTarget : null;
            if (next && this.activeTarget.contains(next)) return;
            if (event.target instanceof Node && this.activeTarget.contains(event.target)) this.hide();
        });

        document.addEventListener('keydown', (event) => {
            if (event.key === 'Escape') this.hide();
        });

        window.addEventListener('scroll', () => this._position(), true);
        window.addEventListener('resize', () => this._position());
    }

    _watchMutations() {
        if (!this.app || this._observer) return;
        this._observer = new MutationObserver((mutations) => {
            for (const mutation of mutations) {
                mutation.addedNodes.forEach((node) => {
                    if (node instanceof HTMLElement || node instanceof DocumentFragment) {
                        this.annotate(node);
                    }
                });
            }
        });
        this._observer.observe(this.app, { childList: true, subtree: true });
    }

    show(target, mode = 'pointer') {
        if (document.documentElement.dataset.tooltips === 'off') return;
        const text = target?.dataset?.uiTooltip?.trim();
        if (!text || !this.tooltipEl) return;
        this.activeTarget = target;
        this.activeMode = mode;
        this.tooltipEl.innerHTML = renderMathInHtml(escapeHtml(text));
        this.tooltipEl.hidden = false;
        this.tooltipEl.dataset.visible = 'true';
        target.setAttribute('aria-describedby', 'ui-tooltip');
        this._position();
    }

    hide() {
        if (this.activeTarget) this.activeTarget.removeAttribute('aria-describedby');
        this.activeTarget = null;
        this.activeMode = null;
        if (!this.tooltipEl) return;
        this.tooltipEl.hidden = true;
        this.tooltipEl.dataset.visible = 'false';
    }

    _position() {
        if (!this.activeTarget || !this.tooltipEl || this.tooltipEl.hidden) return;
        const margin = 12;
        const offset = 10;
        const tooltipRect = this.tooltipEl.getBoundingClientRect();
        const rect = this.activeTarget.getBoundingClientRect();
        let left = rect.left + (rect.width / 2) - (tooltipRect.width / 2);
        const topCandidate = rect.top - tooltipRect.height - offset;
        const bottomCandidate = rect.bottom + offset;
        const anchorY = this.activeMode === 'pointer' ? this.pointerY : rect.top + (rect.height / 2);
        const preferBottom = anchorY < (window.innerHeight * 0.3);
        const fitsAbove = topCandidate >= margin;
        const fitsBelow = bottomCandidate + tooltipRect.height <= window.innerHeight - margin;
        let placement = 'top';
        let top = topCandidate;

        if (preferBottom && fitsBelow) {
            placement = 'bottom';
            top = bottomCandidate;
        } else if (!fitsAbove && fitsBelow) {
            placement = 'bottom';
            top = bottomCandidate;
        } else if (!fitsAbove) {
            placement = 'bottom';
            top = Math.min(bottomCandidate, window.innerHeight - tooltipRect.height - margin);
        }

        if (left + tooltipRect.width > window.innerWidth - margin) {
            left = window.innerWidth - tooltipRect.width - margin;
        }

        if (top < margin) top = margin;
        if (left < margin) left = margin;

        this.tooltipEl.dataset.placement = placement;
        this.tooltipEl.style.left = `${left}px`;
        this.tooltipEl.style.top = `${top}px`;
    }
}
