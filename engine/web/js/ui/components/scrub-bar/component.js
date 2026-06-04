/**
 * ScrubBarComponent — reads a TimelineBuffer and drives scrub-to-tick.
 *
 * Usage:
 *   const bar = new ScrubBarComponent(viewportEl, {
 *       getMemoryBuffer: () => memoryRecorder.buffer,
 *       getNowTick:      () => bridge.getDiagnostics().tick,
 *       onScrub:         (tick) => scale0Controller.hydrateToTick(tick),
 *       onScrubEnd:      () => scale0Controller.resumeLive(),
 *   });
 *   bar.mount();
 *   // Call bar.refresh() from the animate loop ~10 Hz to redraw zones.
 */

import { getScrubBarTemplate } from './template.js';

export class ScrubBarComponent {
    constructor(viewportEl, opts) {
        this.viewportEl = viewportEl;
        this.opts = opts;
        this.el = getScrubBarTemplate();
        this._dragging = false;
        this._refreshSkips = 0;
        this._lastScrubTick = null;
    }

    mount() {
        if (!this.viewportEl || this.el.parentElement) return this;
        this.viewportEl.appendChild(this.el);

        this.stripEl     = this.el.querySelector('.scrub-bar-strip');
        this.zonesEl     = this.el.querySelector('.scrub-bar-zones');
        this.playheadEl  = this.el.querySelector('.scrub-bar-playhead');
        this.timeEl      = this.el.querySelector('.scrub-bar-time');
        this.resetBtn    = this.el.querySelector('.scrub-bar-reset');
        this.settingsBtn = this.el.querySelector('.scrub-bar-settings');
        this.popoverEl   = this.el.querySelector('.scrub-bar-settings-popover');

        if (this.resetBtn) {
            this.resetBtn.addEventListener('click', () => {
                this.opts.onScrubEnd?.();
                this._updatePlayhead(1);
            });
        }

        if (this.stripEl) {
            this.stripEl.addEventListener('dblclick', () => {
                this.opts.onScrubEnd?.();
                this._updatePlayhead(1);
            });
            this.stripEl.addEventListener('pointerdown', (e) => this._beginDrag(e));
        }

        if (this.settingsBtn && this.popoverEl) {
            this.settingsBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                this._setPopoverOpen(this.popoverEl.hasAttribute('hidden'));
            });

            // Speed preset chips — snap the existing ticks-per-frame slider
            // to a multiplier of 1× (value 50 on the 0..100 range mapped to
            // 0.1..10×). The slider's existing wiring picks up the change,
            // so we don't need a separate speed callback — just dispatch
            // an `input` event after setting `.value`.
            for (const chip of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
                chip.addEventListener('click', (e) => {
                    e.stopPropagation();
                    const mult = parseFloat(chip.dataset.speedPreset);
                    if (!Number.isFinite(mult) || mult <= 0) return;
                    const slider = document.getElementById('ticks-per-frame');
                    if (slider) {
                        // ticks-per-frame: min=0, max=100, step=0.1, value=50 ≡ 1×.
                        // Log-ish mapping: slider 50 = 1×, 70 ≈ 2×, 90 ≈ 5×, 10 ≈ 0.1×.
                        // Use `value = 50 + 20·log10(mult)` so the chips line up
                        // with common multipliers on the slider.
                        const raw = 50 + 20 * Math.log10(mult);
                        slider.value = Math.max(0, Math.min(100, raw));
                        slider.dispatchEvent(new Event('input', { bubbles: true }));
                    }
                    for (const c of this.popoverEl.querySelectorAll('[data-speed-preset]')) {
                        const active = c === chip;
                        c.classList.toggle('is-active', active);
                        c.setAttribute('aria-checked', active ? 'true' : 'false');
                    }
                });
            }

            // Step-by-N chips — advance the simulation by N ticks without
            // starting continuous playback. Reuses the existing step action
            // (btn-step) which ticks exactly once, invoking it N times.
            //
            // SCRUB-2 audit pass 2: chain is generation-tagged. If the user
            // unmounts the scrub-bar, fires a new step-N, or reloads the
            // scenario mid-chain, the prior chain detects the generation
            // bump and aborts cleanly instead of clicking into the new sim.
            this._stepGen = this._stepGen | 0;
            for (const chip of this.popoverEl.querySelectorAll('[data-step-by]')) {
                chip.addEventListener('click', (e) => {
                    e.stopPropagation();
                    const n = parseInt(chip.dataset.stepBy, 10) || 1;
                    const stepBtn = document.getElementById('btn-step');
                    if (!stepBtn) return;
                    // Bump the generation so any prior in-flight chain sees
                    // its captured `gen` no longer match and bails.
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

            // Click-outside + Escape close the popover.
            // Idempotency guard (SCRUB-1 audit pass 2): if mount() runs
            // a second time (HMR / re-init) the prior listeners are
            // already attached. Detach them here before re-attaching so
            // we don't double-register.
            if (this._onDocClick) {
                document.removeEventListener('click', this._onDocClick);
            }
            if (this._onDocKey) {
                document.removeEventListener('keydown', this._onDocKey);
            }
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

    _beginDrag(e) {
        if (!this.stripEl) return;
        this._dragging = true;
        this.stripEl.setPointerCapture(e.pointerId);
        this._updateFromEvent(e);
        this.stripEl.addEventListener('pointermove', this._onMove);
        this.stripEl.addEventListener('pointerup',   this._onUp);
        this.stripEl.addEventListener('pointercancel', this._onUp);
    }
    _onMove = (e) => {
        if (!this._dragging || !this.stripEl) return;
        // Cache the latest pointer fraction and coalesce to one hydrate
        // per animation frame. High-rate pointers (240 Hz / trackpads) can
        // otherwise enqueue multiple heavy snapshot loads per frame.
        const rect = this.stripEl.getBoundingClientRect();
        this._pendingFrac = Math.max(0, Math.min(1, (e.clientX - rect.left) / rect.width));
        this._updatePlayhead(this._pendingFrac);
        if (this._rafId == null) {
            this._rafId = requestAnimationFrame(this._flushPending);
        }
    };
    _flushPending = () => {
        this._rafId = null;
        if (this._pendingFrac == null) return;
        const tick = this._fractionToTick(this._pendingFrac);
        this._pendingFrac = null;
        if (tick != null) {
            this._lastScrubTick = tick;
            this.opts.onScrub?.(tick);
        }
    };
    _onUp   = (e) => {
        this._dragging = false;
        if (this._rafId != null) { cancelAnimationFrame(this._rafId); this._rafId = null; }
        this._pendingFrac = null;
        if (this.stripEl) {
            try { this.stripEl.releasePointerCapture(e.pointerId); } catch {}
            this.stripEl.removeEventListener('pointermove', this._onMove);
            this.stripEl.removeEventListener('pointerup',   this._onUp);
            this.stripEl.removeEventListener('pointercancel', this._onUp);
        }
        this.opts.onScrubEnd?.();
    };

    _updateFromEvent(e) {
        if (!this.stripEl) return;
        const rect = this.stripEl.getBoundingClientRect();
        const t = Math.max(0, Math.min(1, (e.clientX - rect.left) / rect.width));
        this._updatePlayhead(t);
        const tick = this._fractionToTick(t);
        if (tick != null) {
            this._lastScrubTick = tick;
            this.opts.onScrub?.(tick);
        }
    }

    _fractionToTick(frac) {
        const mem = this.opts.getMemoryBuffer?.();
        const now = this.opts.getNowTick?.();
        if (now == null) return null;
        // Scrub through live "working memory" from oldest→now.
        // Defensive: if the buffer has stale snapshots from a previous
        // scenario (oldest > now), ignore it and pin to `now` so the scrub
        // never hydrates into wrong-scenario state. clearScale0Timeline()
        // on scenario load is the normal path that prevents this; this
        // guard catches the gap between reset and the first post-reset tick.
        const haveMem = mem && mem.size > 0 && mem.oldestTick <= now;
        const oldest = haveMem ? mem.oldestTick : now;
        return oldest + Math.round((now - oldest) * frac);
    }

    /** Snap the playhead back to "now" and clear the drag's last-scrub tick. */
    _resetPlayhead() {
        this._lastScrubTick = null;
        this._pendingFrac = null;
        if (this._rafId != null) { cancelAnimationFrame(this._rafId); this._rafId = null; }
        this._updatePlayhead(1);
        if (this.timeEl) this.timeEl.textContent = 'now';
    }

    _updatePlayhead(frac) {
        if (!this.playheadEl) return;
        this.playheadEl.style.left = `${(frac * 100).toFixed(2)}%`;
    }

    /** Re-render zones + time badge. Call ~10 Hz from the animate loop. */
    refresh() {
        // Throttle to ~10 Hz regardless of caller frequency.
        if ((this._refreshSkips++ % 6) !== 0) return;

        const mem = this.opts.getMemoryBuffer?.();
        const now = this.opts.getNowTick?.();
        if (!mem || now == null) return;

        // Skip zone drawing if the buffer has nothing for THIS scenario yet.
        // `now` is sim-tick (scenario); a stale buffer from a prior scenario
        // would have oldestTick > now, which would compute negative positions.
        if (this.zonesEl) {
            this.zonesEl.innerHTML = '';
            const haveMem = mem.size > 0 && mem.oldestTick <= now;
            if (haveMem) {
                const oldest = mem.oldestTick;
                const span = Math.max(1, now - oldest);
                for (const z of mem.asZones()) {
                    if (z.toTick > now) continue; // skip any post-"now" stragglers
                    const start = Math.max(0, (z.fromTick - oldest) / span);
                    const end   = Math.min(1, (z.toTick   - oldest) / span);
                    if (end <= start) continue;
                    const div = document.createElement('div');
                    div.className = 'scrub-bar-zone';
                    div.dataset.lod = String(z.lod);
                    div.style.left  = `${(start * 100).toFixed(2)}%`;
                    div.style.width = `${((end - start) * 100).toFixed(2)}%`;
                    this.zonesEl.appendChild(div);
                }
            }
        }

        if (!this._dragging) {
            if (this.timeEl) {
                this.timeEl.textContent = 'now';
            }
            this._updatePlayhead(1);
        } else if (this._lastScrubTick != null) {
            // Report age in SIM seconds (60 scenario ticks/sec), not wall-clock.
            // This stays accurate even when the scenario is locally paused:
            // `now` and `_lastScrubTick` both stop advancing, so the age
            // display freezes with them instead of drifting via global time.
            const ageSec = Math.max(0, (now - this._lastScrubTick) / 60).toFixed(1);
            if (this.timeEl) {
                this.timeEl.textContent = `t−${ageSec}s`;
            }
        }
    }

    unmount() {
        // Idempotent: each handler removed once, then nulled. Important
        // because the ScrubBar is a page-lifetime singleton today (see
        // scales/scale0/controller.js:274) but future SPA-style scale
        // remounts would otherwise re-register the popover handlers
        // each call without freeing the prior ones.
        // Cancel any in-flight step-by-N chain by bumping the generation
        // (SCRUB-2 audit pass 2 fix).
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
