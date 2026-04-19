/**
 * ScrubBarComponent — reads a TimelineBuffer and drives scrub-to-tick.
 *
 * Usage:
 *   const bar = new ScrubBarComponent(viewportEl, {
 *       getMemoryBuffer: () => memoryRecorder.buffer,
 *       getRenderBuffer: () => renderController?.buffer ?? null,
 *       getNowTick:      () => bridge.getDiagnostics().tick,
 *       onScrub:         (tick) => scale0Controller.hydrateToTick(tick),
 *       onScrubEnd:      () => scale0Controller.resumeLive(),
 *       onRender:        (seconds) => scale0Controller.startRender(seconds),
 *   });
 *   bar.mount();
 *   // Call bar.refresh() from the animate loop ~10 Hz to redraw zones.
 */

import { createScrubBarTemplate } from './template.js';

export class ScrubBarComponent {
    constructor(viewportEl, opts) {
        this.viewportEl = viewportEl;
        this.opts = opts;
        this.el = createScrubBarTemplate();
        this._dragging = false;
        this._refreshSkips = 0;
        this._lastScrubTick = null;
        this._renderSeconds = 30; // current Render-button duration (user-selectable via gear)
    }

    mount() {
        if (!this.viewportEl || this.el.parentElement) return this;
        this.viewportEl.appendChild(this.el);

        this.stripEl     = this.el.querySelector('.scrub-bar-strip');
        this.zonesEl     = this.el.querySelector('.scrub-bar-zones');
        this.renderEl    = this.el.querySelector('.scrub-bar-render');
        this.playheadEl  = this.el.querySelector('.scrub-bar-playhead');
        this.timeEl      = this.el.querySelector('.scrub-bar-time');
        this.resetBtn    = this.el.querySelector('.scrub-bar-reset');
        this.renderBtn   = this.el.querySelector('.scrub-bar-render-btn');
        this.settingsBtn = this.el.querySelector('.scrub-bar-settings');
        this.popoverEl   = this.el.querySelector('.scrub-bar-settings-popover');

        this.resetBtn.addEventListener('click', () => {
            this.opts.onScrubEnd?.();
            this._updatePlayhead(1);
        });

        this.stripEl.addEventListener('dblclick', () => {
            this.opts.onScrubEnd?.();
            this._updatePlayhead(1);
        });

        this.stripEl.addEventListener('pointerdown', (e) => this._beginDrag(e));

        if (this.renderBtn) {
            this.renderBtn.addEventListener('click', () => {
                // Always close the settings popover before starting a render so
                // it doesn't cover the viewport during the progress chip animation.
                this._setPopoverOpen(false);
                const secs = this._renderSeconds || 30;
                this.renderBtn.title = `Render next ${secs} seconds into a scrubbable clip`;
                this.opts.onRender?.(secs);
            });
        }

        if (this.settingsBtn && this.popoverEl) {
            this.settingsBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                this._setPopoverOpen(this.popoverEl.hasAttribute('hidden'));
            });

            // Duration chip clicks update the stored seconds + active-class.
            for (const chip of this.popoverEl.querySelectorAll('[data-render-secs]')) {
                chip.addEventListener('click', (e) => {
                    e.stopPropagation();
                    const secs = Number(chip.dataset.renderSecs) || 30;
                    this._renderSeconds = secs;
                    for (const c of this.popoverEl.querySelectorAll('[data-render-secs]')) {
                        const active = c === chip;
                        c.classList.toggle('is-active', active);
                        c.setAttribute('aria-checked', active ? 'true' : 'false');
                    }
                    if (this.renderBtn) {
                        this.renderBtn.title = `Render next ${secs} seconds into a scrubbable clip`;
                    }
                });
            }

            // Click-outside + Escape close the popover.
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
        this._dragging = true;
        this.stripEl.setPointerCapture(e.pointerId);
        this._updateFromEvent(e);
        this.stripEl.addEventListener('pointermove', this._onMove);
        this.stripEl.addEventListener('pointerup',   this._onUp);
        this.stripEl.addEventListener('pointercancel', this._onUp);
    }
    _onMove = (e) => {
        if (!this._dragging) return;
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
        try { this.stripEl.releasePointerCapture(e.pointerId); } catch {}
        this.stripEl.removeEventListener('pointermove', this._onMove);
        this.stripEl.removeEventListener('pointerup',   this._onUp);
        this.stripEl.removeEventListener('pointercancel', this._onUp);
        this.opts.onScrubEnd?.();
    };

    _updateFromEvent(e) {
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
        const render = this.opts.getRenderBuffer?.();
        const now = this.opts.getNowTick?.();
        if (now == null) return null;
        // Rendered clip wins when present — it's the dense, high-fidelity
        // buffer the user just asked to render. The full scrub bar maps
        // oldest→newest of the clip.
        if (render && render.size > 0 && render.latestTick >= render.oldestTick) {
            const span = Math.max(1, render.latestTick - render.oldestTick);
            return render.oldestTick + Math.round(span * frac);
        }
        // Otherwise scrub through live "working memory" from oldest→now.
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
        const render = this.opts.getRenderBuffer?.();
        const now = this.opts.getNowTick?.();
        if (!mem || now == null) return;

        // Skip zone drawing if the buffer has nothing for THIS scenario yet.
        // `now` is sim-tick (scenario); a stale buffer from a prior scenario
        // would have oldestTick > now, which would compute negative positions.
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

        const hasRender = !!(render && render.size > 0);
        this.renderEl.dataset.active = hasRender ? 'true' : 'false';
        if (this.renderBtn) this.renderBtn.dataset.rendering = hasRender ? 'true' : 'false';

        if (!this._dragging) {
            this.timeEl.textContent = 'now';
            this._updatePlayhead(1);
        } else if (this._lastScrubTick != null) {
            // Report age in SIM seconds (60 scenario ticks/sec), not wall-clock.
            // This stays accurate even when the scenario is locally paused:
            // `now` and `_lastScrubTick` both stop advancing, so the age
            // display freezes with them instead of drifting via global time.
            const ageSec = Math.max(0, (now - this._lastScrubTick) / 60).toFixed(1);
            this.timeEl.textContent = `t−${ageSec}s`;
        }
    }

    unmount() {
        if (this._onDocClick) document.removeEventListener('click', this._onDocClick);
        if (this._onDocKey)   document.removeEventListener('keydown', this._onDocKey);
        this.el.remove();
    }
}
