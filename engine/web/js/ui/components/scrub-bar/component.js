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
        this.renderBtn   = this.el.querySelector('.scrub-bar-render-btn'); // may be null until Task 15

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
            this.renderBtn.addEventListener('click', () => this.opts.onRender?.(30));
        }

        return this;
    }

    _beginDrag(e) {
        this._dragging = true;
        this.stripEl.setPointerCapture(e.pointerId);
        this._updateFromEvent(e);
        this.stripEl.addEventListener('pointermove', this._onMove);
        this.stripEl.addEventListener('pointerup',   this._onUp);
        this.stripEl.addEventListener('pointercancel', this._onUp);
    }
    _onMove = (e) => { if (this._dragging) this._updateFromEvent(e); };
    _onUp   = (e) => {
        this._dragging = false;
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
        if (render && frac > 0.5) {
            const futureFrac = (frac - 0.5) * 2;
            const renderSpan = render.latestTick - render.oldestTick;
            return render.oldestTick + Math.round(renderSpan * futureFrac);
        }
        const pastFrac = render ? frac * 2 : frac;
        const oldest = mem?.oldestTick ?? now;
        return oldest + Math.round((now - oldest) * pastFrac);
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

        this.zonesEl.innerHTML = '';
        const oldest = mem.oldestTick;
        const span = Math.max(1, now - oldest);
        for (const z of mem.asZones()) {
            const start = (z.fromTick - oldest) / span;
            const end   = (z.toTick   - oldest) / span;
            const div = document.createElement('div');
            div.className = 'scrub-bar-zone';
            div.dataset.lod = String(z.lod);
            div.style.left  = `${(start * 100).toFixed(2)}%`;
            div.style.width = `${((end - start) * 100).toFixed(2)}%`;
            this.zonesEl.appendChild(div);
        }

        const hasRender = !!(render && render.size > 0);
        this.renderEl.dataset.active = hasRender ? 'true' : 'false';
        if (this.renderBtn) this.renderBtn.dataset.rendering = hasRender ? 'true' : 'false';

        if (!this._dragging) {
            this.timeEl.textContent = 'now';
            this._updatePlayhead(1);
        } else if (this._lastScrubTick != null) {
            const ageSec = Math.max(0, (now - this._lastScrubTick) / 60).toFixed(1);
            this.timeEl.textContent = `t−${ageSec}s`;
        }
    }

    unmount() { this.el.remove(); }
}
