import { applyUiTooltipDefinitions } from './definitions.js?v=5';
import { renderMathInHtml } from '../../math-format/render.js';
import { LifetimeScope } from '../../utils/lifetime-scope.js';

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
        this._scope = null;
        this._ownsTooltipElement = false;
        this._addedDescription = false;
    }

    init() {
        if (this._scope && !this._scope.disposed) return this;
        this._scope = new LifetimeScope();
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
        this._ownsTooltipElement = true;
    }

    _bindEvents() {
        this._scope.on(document, 'pointerover', (event) => {
            if (event.pointerType === 'touch') return;
            const target = event.target instanceof Element ? event.target.closest('[data-ui-tooltip]') : null;
            if (!(target instanceof HTMLElement)) return;
            this.pointerX = event.clientX;
            this.pointerY = event.clientY;
            if (this.activeMode === 'focus' && this.activeTarget === target) return;
            this.show(target, 'pointer');
        });

        this._scope.on(document, 'pointermove', (event) => {
            if (this.activeMode !== 'pointer') return;
            this.pointerX = event.clientX;
            this.pointerY = event.clientY;
            this._position();
        });

        this._scope.on(document, 'pointerout', (event) => {
            if (event.pointerType === 'touch' || this.activeMode !== 'pointer' || !this.activeTarget) return;
            const next = event.relatedTarget instanceof Node ? event.relatedTarget : null;
            if (next && this.activeTarget.contains(next)) return;
            if (event.target instanceof Node && this.activeTarget.contains(event.target)) {
                if (this.activeTarget.contains(document.activeElement)) this.show(this.activeTarget, 'focus');
                else this.hide();
            }
        });

        this._scope.on(document, 'pointerdown', (event) => {
            if (event.pointerType !== 'touch') return;
            const target = event.target instanceof Element ? event.target.closest('[data-ui-tooltip]') : null;
            if (target instanceof HTMLElement) this.show(target, 'touch');
            else if (this.activeMode === 'touch') this.hide();
        });

        this._scope.on(document, 'focusin', (event) => {
            const target = event.target instanceof Element ? event.target.closest('[data-ui-tooltip]') : null;
            if (target instanceof HTMLElement && !(this.activeMode === 'touch' && this.activeTarget === target)) {
                this.show(target, 'focus');
            }
        });

        this._scope.on(document, 'focusout', (event) => {
            if (this.activeMode !== 'focus' || !this.activeTarget) return;
            const next = event.relatedTarget instanceof Node ? event.relatedTarget : null;
            if (next && this.activeTarget.contains(next)) return;
            if (event.target instanceof Node && this.activeTarget.contains(event.target)) this.hide();
        });

        this._scope.on(document, 'keydown', (event) => {
            if (event.key === 'Escape') this.hide();
        });

        this._scope.on(window, 'scroll', () => this._position(), true);
        this._scope.on(window, 'resize', () => this._position());

        const preferenceObserver = new MutationObserver(() => {
            if (document.documentElement.dataset.tooltips === 'off') this.hide();
        });
        preferenceObserver.observe(document.documentElement, { attributes: true, attributeFilter: ['data-tooltips'] });
        this._scope.defer(() => preferenceObserver.disconnect());
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
            if (this.activeTarget && !this.activeTarget.isConnected) this.hide();
        });
        this._observer.observe(this.app, { childList: true, subtree: true });
        this._scope.defer(() => this._observer?.disconnect());
    }

    show(target, mode = 'pointer') {
        if (document.documentElement.dataset.tooltips === 'off') {
            this.hide();
            return;
        }
        const text = target?.dataset?.uiTooltip?.trim();
        if (!text || !this.tooltipEl) return;
        if (this.activeTarget && this.activeTarget !== target) this._removeDescription();
        this.activeTarget = target;
        this.activeMode = mode;
        this.tooltipEl.innerHTML = renderMathInHtml(escapeHtml(text));
        this.tooltipEl.hidden = false;
        this.tooltipEl.dataset.visible = 'true';
        const descriptionIds = (target.getAttribute('aria-describedby') || '').split(/\s+/).filter(Boolean);
        if (!descriptionIds.includes('ui-tooltip')) {
            target.setAttribute('aria-describedby', [...descriptionIds, 'ui-tooltip'].join(' '));
            this._addedDescription = true;
        }
        this._position();
    }

    _removeDescription() {
        if (!this.activeTarget || !this._addedDescription) return;
        const remaining = (this.activeTarget.getAttribute('aria-describedby') || '')
            .split(/\s+/).filter((id) => id && id !== 'ui-tooltip');
        if (remaining.length) this.activeTarget.setAttribute('aria-describedby', remaining.join(' '));
        else this.activeTarget.removeAttribute('aria-describedby');
        this._addedDescription = false;
    }

    hide() {
        this._removeDescription();
        this.activeTarget = null;
        this.activeMode = null;
        if (!this.tooltipEl) return;
        this.tooltipEl.hidden = true;
        this.tooltipEl.dataset.visible = 'false';
    }

    _position() {
        if (!this.activeTarget || !this.tooltipEl || this.tooltipEl.hidden) return;
        if (!this.activeTarget.isConnected) {
            this.hide();
            return;
        }
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

    destroy() {
        if (!this._scope || this._scope.disposed) return;
        this.hide();
        this._scope.dispose();
        this._observer = null;
        if (this._ownsTooltipElement) this.tooltipEl?.remove();
        this.tooltipEl = null;
        this._ownsTooltipElement = false;
    }

    cleanup() {
        this.destroy();
    }
}
