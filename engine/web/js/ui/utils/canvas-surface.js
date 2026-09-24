/**
 * DPR-aware canvas ownership for fixed and responsive panel charts.
 *
 * Drawing callbacks receive logical CSS-pixel dimensions while the backing
 * store is scaled to the current device-pixel ratio. CanvasSurface owns the
 * ResizeObserver and scheduled redraw, so panel controllers have one cleanup
 * boundary instead of open-coded resize listeners.
 */

import { LifetimeScope } from './lifetime-scope.js';

function finitePositive(value, fallback) {
    const n = Number(value);
    return Number.isFinite(n) && n > 0 ? n : fallback;
}

export function measureCanvasSurface(canvas, { maxDpr = 2 } = {}) {
    if (!canvas) return null;
    const rect = canvas.getBoundingClientRect?.() || {};
    const rectWidth = Number(rect.width) || 0;
    const rectHeight = Number(rect.height) || 0;
    const clientWidth = Number(canvas.clientWidth) || 0;
    const clientHeight = Number(canvas.clientHeight) || 0;
    if (canvas.ownerDocument && !(rectWidth > 0) && !(rectHeight > 0)
        && !(clientWidth > 0) && !(clientHeight > 0)) return null;
    const fallbackWidth = finitePositive(canvas.clientWidth, finitePositive(canvas.width, 1));
    const fallbackHeight = finitePositive(canvas.clientHeight, finitePositive(canvas.height, 1));
    const width = finitePositive(rect.width, fallbackWidth);
    const height = finitePositive(rect.height, fallbackHeight);
    const rawDpr = typeof window !== 'undefined' ? window.devicePixelRatio : globalThis.devicePixelRatio;
    const dpr = Math.max(1, Math.min(finitePositive(rawDpr, 1), finitePositive(maxDpr, 2)));
    return { width, height, dpr };
}

export function resizeCanvasSurface(canvas, options = {}) {
    const metrics = measureCanvasSurface(canvas, options);
    if (!metrics) return null;
    const backingWidth = Math.max(1, Math.round(metrics.width * metrics.dpr));
    const backingHeight = Math.max(1, Math.round(metrics.height * metrics.dpr));
    const changed = canvas.width !== backingWidth || canvas.height !== backingHeight;
    if (changed) {
        canvas.width = backingWidth;
        canvas.height = backingHeight;
    }
    const ctx = canvas.getContext?.('2d') || null;
    if (ctx?.setTransform) ctx.setTransform(metrics.dpr, 0, 0, metrics.dpr, 0, 0);
    return { canvas, ctx, ...metrics, backingWidth, backingHeight, changed };
}

export class CanvasSurface {
    constructor(canvas, draw, { maxDpr = 2, observe = true } = {}) {
        this.canvas = canvas;
        this.draw = typeof draw === 'function' ? draw : () => {};
        this.maxDpr = maxDpr;
        this.observe = observe;
        this._observer = null;
        this._cancelDraw = null;
        this._lifetime = null;
        this._mounted = false;
        this._disposed = false;
    }

    mount() {
        if (!this.canvas || this._disposed || this._mounted) return this;
        this._mounted = true;
        this._lifetime = new LifetimeScope();
        if (this.observe && typeof ResizeObserver !== 'undefined') {
            this._observer = new ResizeObserver(() => this.requestDraw());
            this._observer.observe(this.canvas);
            this._lifetime.defer(() => this._observer?.disconnect());
        }
        if (typeof window !== 'undefined' && window?.addEventListener) {
            this._lifetime.on(window, 'resize', () => this.requestDraw());
        }
        if (typeof matchMedia === 'function') {
            let releaseResolution = null;
            const armResolutionWatch = () => {
                releaseResolution?.();
                const dpr = finitePositive(
                    typeof window !== 'undefined' ? window.devicePixelRatio : globalThis.devicePixelRatio,
                    1,
                );
                const query = matchMedia(`(resolution: ${dpr}dppx)`);
                releaseResolution = this._lifetime.on(query, 'change', () => {
                    this.requestDraw();
                    armResolutionWatch();
                });
            };
            armResolutionWatch();
            this._lifetime.defer(() => releaseResolution?.());
        }
        this.requestDraw();
        return this;
    }

    requestDraw() {
        if (this._disposed || !this._mounted || !this._lifetime || this._cancelDraw) return;
        const callback = () => {
            this._cancelDraw = null;
            this.redrawNow();
        };
        this._cancelDraw = typeof requestAnimationFrame === 'function'
            ? this._lifetime.frame(callback)
            : this._lifetime.timeout(callback, 0);
    }

    redrawNow() {
        if (this._disposed) return null;
        const surface = resizeCanvasSurface(this.canvas, { maxDpr: this.maxDpr });
        if (!surface?.ctx) return surface;
        this.draw(surface);
        return surface;
    }

    dispose() {
        if (this._disposed) return;
        this._disposed = true;
        this._cancelDraw?.();
        this._cancelDraw = null;
        this._lifetime?.dispose();
        this._lifetime = null;
        this._observer = null;
        this._mounted = false;
    }
}

export function createCanvasSurface(canvas, draw, options) {
    return new CanvasSurface(canvas, draw, options).mount();
}
