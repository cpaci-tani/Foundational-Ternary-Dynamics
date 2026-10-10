/**
 * PlayBarComponent — the floating transport + speed bar at the bottom of the
 * viewport. Hosts the primary playback controls (play / step / reset — wired by
 * app.js), the speed control (nudge / presets / fine slider), and the step-by-N
 * shortcuts.
 *
 * Usage:
 *   const bar = new PlayBarComponent(viewportEl);
 *   bar.mount();
 *   // Call bar.refresh() from the animate loop; it throttles internally.
 */

import { getPlayBarTemplate } from './template.js';
import { appRegistry } from '../../../core/registry.js';
import { applyPanelMountClasses } from '../panel-dock/mount-toggle.js';
import { BaseLifecycleController } from '../../../lifecycle.js';
import { speedToSliderValue } from './speed-scale.js';

export class PlayBarComponent extends BaseLifecycleController {
    constructor(viewportEl, opts) {
        super();
        this.viewportEl = viewportEl;
        this.opts = opts;
        this.el = getPlayBarTemplate();
        this._refreshSkips = 0;
        this._mounted = false;
        this._stepTimer = null;
    }

    mount() {
        if (!this.viewportEl || this._mounted) return this;
        this._mounted = true;
        const mountEl = document.getElementById('app') || this.viewportEl;
        if (!this.el.parentElement) mountEl.appendChild(this.el);
        applyPanelMountClasses(document.documentElement.dataset.panelMount || 'left');

        this.zoomBtn = this.el.querySelector('#play-bar-zoom-btn');
        this.zoomMenu = this.el.querySelector('#play-bar-zoom-menu');
        this.speedNudgeBtns = this.el.querySelectorAll('[data-speed-nudge]');

        for (const btn of this.speedNudgeBtns) {
            this.bindEvent(btn, 'click', () => {
                this._nudgeSpeed(parseFloat(btn.dataset.speedNudge) || 0);
            });
        }

        this._stepGen = this._stepGen | 0;
        for (const chip of this.el.querySelectorAll('[data-step-by]')) {
            this.bindEvent(chip, 'click', (e) => {
                e.stopPropagation();
                const n = parseInt(chip.dataset.stepBy, 10) || 1;
                const stepBtn = document.getElementById('btn-step');
                if (!stepBtn) return;
                this.cancelPendingSteps();
                const gen = this._stepGen;
                let i = 0;
                const tickOne = () => {
                    if (gen !== this._stepGen) return;
                    stepBtn.click();
                    i++;
                    if (i < n) this._stepTimer = setTimeout(tickOne, 0);
                    else this._stepTimer = null;
                };
                tickOne();
            });
        }
        this.bindEvent(document, 'change', () => this.cancelPendingSteps());
        this.bindEvent(document.getElementById('btn-reset'), 'click', () => this.cancelPendingSteps());

        if (this.zoomBtn && this.zoomMenu) {
            this.bindEvent(this.zoomBtn, 'click', (e) => {
                e.stopPropagation();
                const open = this.zoomMenu.hasAttribute('hidden');
                this._setZoomMenu(open);
            });
            for (const item of this.zoomMenu.querySelectorAll('[data-framed-view]')) {
                this.bindEvent(item, 'click', (e) => {
                    e.stopPropagation();
                    // Stops beyond the lattice are travelled to, so the empty range is seen.
                    appRegistry.get('viewport')?.setFramedView?.(
                        item.dataset.framedView, { animate: item.hasAttribute('data-travel') },
                    );
                    this._setZoomMenu(false);
                });
            }
            this._onDocClick = (e) => {
                if (!this.zoomMenu || this.zoomMenu.hasAttribute('hidden')) return;
                if (this.zoomMenu.contains(e.target) || this.zoomBtn.contains(e.target)) return;
                this._setZoomMenu(false);
            };
            this._onDocKey = (e) => {
                if (e.key === 'Escape') this._setZoomMenu(false);
            };
            this.bindEvent(document, 'click', this._onDocClick);
            this.bindEvent(document, 'keydown', this._onDocKey);
        }

        return this;
    }

    _nudgeSpeed(rawStep) {
        const slider = document.getElementById('ticks-per-frame');
        if (!slider || rawStep === 0) return;
        const min = Number.parseFloat(slider.min || '0');
        const max = Number.parseFloat(slider.max || '100');
        const current = Number.parseFloat(slider.value || '50');
        const next = Math.max(min, Math.min(max, current + rawStep));
        slider.value = String(Number(next.toFixed(1)));
        slider.dispatchEvent(new Event('input', { bubbles: true }));
        this._syncSpeedPresetFromSlider(next);
    }

    _setActiveSpeedPreset(activeChip) {
        if (!this.popoverEl) return;
        for (const chip of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
            const active = chip === activeChip;
            chip.classList.toggle('is-active', active);
            const ariaChecked = active ? 'true' : 'false';
            if (chip.getAttribute('aria-checked') !== ariaChecked) {
                chip.setAttribute('aria-checked', ariaChecked);
            }
        }
    }

    _syncSpeedPresetFromSlider(sliderValue) {
        if (!this.popoverEl) return;
        let matched = null;
        for (const chip of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
            const mult = parseFloat(chip.dataset.speedPreset);
            if (!Number.isFinite(mult) || mult <= 0) continue;
            const presetValue = speedToSliderValue(mult);
            if (Math.abs(sliderValue - presetValue) < 0.25) matched = chip;
        }
        this._setActiveSpeedPreset(matched);
    }

    _setActiveZoomPreset(activeChip) {
        if (!this.popoverEl) return;
        for (const chip of this.popoverEl.querySelectorAll('[data-zoom-preset]')) {
            const active = chip === activeChip;
            chip.classList.toggle('is-active', active);
            const ariaChecked = active ? 'true' : 'false';
            if (chip.getAttribute('aria-checked') !== ariaChecked) {
                chip.setAttribute('aria-checked', ariaChecked);
            }
        }
    }

    _syncZoomPresetFromCamera() {
        if (!this.popoverEl) return;
        const viewport = appRegistry.get('viewport');
        if (!viewport || !viewport.camera || !viewport.controls) return;

        const currentDist = viewport.camera.position.distanceTo(viewport.controls.target);
        const refDist = viewport.getReferenceDistance ? viewport.getReferenceDistance() : null;
        if (!refDist) return;

        const currentZoom = refDist / currentDist;

        let bestChip = null;
        let minDiff = Infinity;

        const chips = this.popoverEl.querySelectorAll('[data-zoom-preset]');
        for (const chip of chips) {
            const factor = parseFloat(chip.dataset.zoomPreset);
            if (!Number.isFinite(factor) || factor <= 0) continue;
            const diff = Math.abs(Math.log(currentZoom / factor));
            if (diff < minDiff) {
                minDiff = diff;
                bestChip = chip;
            }
        }

        const threshold = 0.25;
        this._setActiveZoomPreset(minDiff < threshold ? bestChip : null);
    }

    _setZoomMenu(open) {
        if (!this.zoomMenu || !this.zoomBtn) return;
        if (open) this.zoomMenu.removeAttribute('hidden');
        else this.zoomMenu.setAttribute('hidden', '');
        this.zoomBtn.setAttribute('aria-expanded', open ? 'true' : 'false');
        this.zoomBtn.classList.toggle('is-open', open);
    }

    _setPopoverOpen(open) {
        if (!this.popoverEl || !this.settingsBtn) return;
        if (open) {
            this.popoverEl.removeAttribute('hidden');
            this.settingsBtn.setAttribute('aria-expanded', 'true');
            this.settingsBtn.classList.add('is-open');
            const slider = document.getElementById('ticks-per-frame');
            if (slider) this._syncSpeedPresetFromSlider(Number.parseFloat(slider.value));
            this._syncZoomPresetFromCamera();
        } else {
            this.popoverEl.setAttribute('hidden', '');
            this.settingsBtn.setAttribute('aria-expanded', 'false');
            this.settingsBtn.classList.remove('is-open');
        }
    }

    /** Sync zoom preset chips from camera state. Call from the animate loop; throttles internally. */
    refresh() {
        if (!this.popoverEl || this.popoverEl.hasAttribute('hidden')) return;
        if ((this._refreshSkips++ % 6) !== 0) return;
        this._syncZoomPresetFromCamera();
    }

    cancelPendingSteps() {
        this._stepGen = (this._stepGen | 0) + 1;
        if (this._stepTimer !== null) {
            clearTimeout(this._stepTimer);
            this._stepTimer = null;
        }
    }

    unmount() {
        // Cancel any in-flight step-by-N chain (generation bump) + detach the
        // popover document listeners (idempotent; page-lifetime singleton today).
        this.cancelPendingSteps();
        super.destroy();
        this._onDocClick = null;
        this._onDocKey = null;
        if (this.el && this.el.parentElement) this.el.remove();
        this._mounted = false;
    }

    // Convention alias used elsewhere in the codebase.
    dispose() { this.unmount(); }
}
