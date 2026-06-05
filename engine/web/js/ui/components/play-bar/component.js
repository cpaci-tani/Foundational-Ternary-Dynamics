/**
 * PlayBarComponent — the floating transport + speed bar at the bottom of the
 * viewport. Hosts the primary playback controls (play / step / reset — wired by
 * app.js), the speed control (nudge / presets / fine slider), the step-by-N
 * shortcuts, and a forward-only "T N" tick readout.
 *
 * The simulation is forward-only: the engine tick is the single time source,
 * and the render loop is the observer's external (always-flowing) clock. There
 * is no reverse / rewind timeline.
 *
 * Usage:
 *   const bar = new PlayBarComponent(viewportEl, { getNowTick: () => bridge…tick });
 *   bar.mount();
 *   // Call bar.refresh() from the animate loop; it throttles internally.
 */

import { getPlayBarTemplate } from './template.js';

export class PlayBarComponent {
    constructor(viewportEl, opts) {
        this.viewportEl = viewportEl;
        this.opts = opts;
        this.el = getPlayBarTemplate();
        this._refreshSkips = 0;
    }

    mount() {
        if (!this.viewportEl || this.el.parentElement) return this;
        const mountEl = document.getElementById('app') || this.viewportEl;
        mountEl.appendChild(this.el);

        this.timeEl      = this.el.querySelector('.play-bar-time');
        this.settingsBtn = this.el.querySelector('.play-bar-settings');
        this.popoverEl   = this.el.querySelector('.play-bar-settings-popover');
        this.speedNudgeBtns = this.el.querySelectorAll('[data-speed-nudge]');

        for (const btn of this.speedNudgeBtns) {
            btn.addEventListener('click', () => {
                this._nudgeSpeed(parseFloat(btn.dataset.speedNudge) || 0);
            });
        }

        if (this.settingsBtn && this.popoverEl) {
            this.settingsBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                this._setPopoverOpen(this.popoverEl.hasAttribute('hidden'));
            });

            // Speed preset chips snap the existing ticks-per-frame slider. The
            // slider's app.js wiring picks up the change, so we only dispatch the
            // same input event a direct slider drag emits.
            for (const chip of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
                chip.addEventListener('click', (e) => {
                    e.stopPropagation();
                    const mult = parseFloat(chip.dataset.speedPreset);
                    if (!Number.isFinite(mult) || mult <= 0) return;
                    const slider = document.getElementById('ticks-per-frame');
                    if (slider) {
                        // ticks-per-frame: min=0, max=100, step=0.1, value=50 ≡ 1×.
                        // `value = 50 + 20·log10(mult)` lines the chips up on the slider.
                        const raw = 50 + 20 * Math.log10(mult);
                        slider.value = Math.max(0, Math.min(100, raw));
                        slider.dispatchEvent(new Event('input', { bubbles: true }));
                    }
                    this._setActiveSpeedPreset(chip);
                });
            }

            // Step-by-N chips — advance the simulation by N ticks without starting
            // continuous playback. Reuses btn-step (one tick) N times. The chain is
            // generation-tagged: unmount / re-fire / scenario reload bumps the
            // generation so a prior in-flight chain aborts cleanly.
            this._stepGen = this._stepGen | 0;
            for (const chip of this.popoverEl.querySelectorAll('[data-step-by]')) {
                chip.addEventListener('click', (e) => {
                    e.stopPropagation();
                    const n = parseInt(chip.dataset.stepBy, 10) || 1;
                    const stepBtn = document.getElementById('btn-step');
                    if (!stepBtn) return;
                    this._stepGen += 1;
                    const gen = this._stepGen;
                    let i = 0;
                    const tickOne = () => {
                        if (gen !== this._stepGen) return;   // aborted
                        stepBtn.click();
                        i++;
                        if (i < n) setTimeout(tickOne, 0);
                    };
                    tickOne();
                });
            }

            // Click-outside + Escape close the popover. Idempotency guard: if
            // mount() runs again (HMR / re-init) detach the prior listeners first.
            if (this._onDocClick) document.removeEventListener('click', this._onDocClick);
            if (this._onDocKey) document.removeEventListener('keydown', this._onDocKey);
            this._onDocClick = (e) => {
                if (!this.popoverEl || this.popoverEl.hasAttribute('hidden')) return;
                if (this.popoverEl.contains(e.target) || this.settingsBtn.contains(e.target)) return;
                this._setPopoverOpen(false);
            };
            this._onDocKey = (e) => {
                if (e.key === 'Escape') this._setPopoverOpen(false);
            };
            document.addEventListener('click', this._onDocClick);
            document.addEventListener('keydown', this._onDocKey);
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
            chip.setAttribute('aria-checked', active ? 'true' : 'false');
        }
    }

    _syncSpeedPresetFromSlider(sliderValue) {
        if (!this.popoverEl) return;
        let matched = null;
        for (const chip of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
            const mult = parseFloat(chip.dataset.speedPreset);
            if (!Number.isFinite(mult) || mult <= 0) continue;
            const presetValue = 50 + 20 * Math.log10(mult);
            if (Math.abs(sliderValue - presetValue) < 0.25) matched = chip;
        }
        this._setActiveSpeedPreset(matched);
    }

    _setPopoverOpen(open) {
        if (!this.popoverEl || !this.settingsBtn) return;
        if (open) {
            this.popoverEl.removeAttribute('hidden');
            this.settingsBtn.setAttribute('aria-expanded', 'true');
            this.settingsBtn.classList.add('is-open');
        } else {
            this.popoverEl.setAttribute('hidden', '');
            this.settingsBtn.setAttribute('aria-expanded', 'false');
            this.settingsBtn.classList.remove('is-open');
        }
    }

    _formatTickLabel(tick) {
        return Number.isFinite(tick) ? `T ${Math.max(0, Math.round(tick))}` : 'T --';
    }

    /** Update the forward-only "T N" tick readout. Call from the animate loop; throttles internally. */
    refresh() {
        if ((this._refreshSkips++ % 6) !== 0) return;
        const now = this.opts.getNowTick?.();
        if (this.timeEl && now != null) {
            this.timeEl.textContent = this._formatTickLabel(now);
        }
    }

    unmount() {
        // Cancel any in-flight step-by-N chain (generation bump) + detach the
        // popover document listeners (idempotent; page-lifetime singleton today).
        this._stepGen = (this._stepGen | 0) + 1;
        if (this._onDocClick) {
            document.removeEventListener('click', this._onDocClick);
            this._onDocClick = null;
        }
        if (this._onDocKey) {
            document.removeEventListener('keydown', this._onDocKey);
            this._onDocKey = null;
        }
        if (this.el && this.el.parentElement) this.el.remove();
    }

    // Convention alias used elsewhere in the codebase.
    dispose() { this.unmount(); }
}
