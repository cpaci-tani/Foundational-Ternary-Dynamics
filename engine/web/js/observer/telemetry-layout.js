// @ts-check
/** Fit telemetry text inside fixed presentation slots without changing readings. */
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';

/** @typedef {{base:number,min:number,family:string,weight:string,lineHeightRatio:number,letterSpacing:number,valueHeight:number}} ReadingStyle */
/** @typedef {{output:HTMLElement,value:HTMLElement,width:number,height:number,text:string,presentation:import('./telemetry.js').TelemetryPresentation,style:ReadingStyle|null,styleDirty:boolean,dirty:boolean,initialGeometry:boolean}} ReadingSlot */

/**
 * Geometry comes from ResizeObserver; live updates reuse cached font metrics.
 * Reads are batched before font writes, so fitting does not alternate forced
 * layout reads and writes for each changing measurement. Only value-span font
 * sizes change: numerical text, unit text and fixed row dimensions stay intact.
 * @param {{panel:HTMLElement,body:HTMLElement}} options
 */
export function createTelemetryLayout({ panel, body }) {
    const scope = new LifetimeScope();
    const canvas = document.createElement('canvas');
    const context = canvas.getContext('2d');
    /** @type {Map<HTMLElement,ReadingSlot>} */ const slots = new Map();
    /** @type {(()=>void)|null} */ let cancelFrame = null;
    /** @type {(()=>void)|null} */ let cancelVerification = null;
    let active = false;
    let disposed = false;
    let panelWidth = 0;
    let panelHeight = 0;

    function schedule() {
        if (!active || disposed || cancelFrame) return;
        cancelFrame = scope.frame(() => {
            cancelFrame = null;
            fitReadings();
        });
    }

    function invalidateStyles() {
        if (!active || disposed) return;
        for (const slot of slots.values()) {
            slot.styleDirty = true;
            slot.dirty = true;
        }
        schedule();
    }

    const resizeObserver = new ResizeObserver(entries => {
        if (!active || disposed) return;
        for (const entry of entries) {
            const { width, height } = entry.contentRect;
            if (entry.target === panel) {
                if (width !== panelWidth || height !== panelHeight) {
                    panelWidth = width;
                    panelHeight = height;
                    // Container-relative typography changes with panel width.
                    invalidateStyles();
                }
                continue;
            }
            const slot = slots.get(/** @type {HTMLElement} */ (entry.target));
            if (!slot || width === slot.width && height === slot.height) continue;
            slot.width = width;
            slot.height = height;
            slot.styleDirty = true;
            slot.dirty = true;
        }
        schedule();
    });
    scope.defer(() => resizeObserver.disconnect());
    scope.on(window, 'resize', invalidateStyles);
    scope.on(document.fonts, 'loadingdone', invalidateStyles);
    document.fonts.ready.then(() => {
        if (!disposed) invalidateStyles();
    });

    /** @param {ReadingSlot} slot @returns {ReadingStyle} */
    function readStyle(slot) {
        // The dd font is unaffected by a previous fit on its child value span.
        const parent = slot.output.closest('dd') ?? slot.output;
        const computed = window.getComputedStyle(parent);
        const base = Number.parseFloat(computed.fontSize) || 16;
        const minimum = Number.parseFloat(computed.getPropertyValue('--observer-reading-min-font'));
        const lineHeight = Number.parseFloat(computed.lineHeight) || base * 1.15;
        const valueHeight = Number.parseFloat(computed.getPropertyValue('--observer-reading-value-height'));
        return {
            base,
            min: Math.min(base, minimum > 0 ? minimum : slot.presentation === 'text' ? 11 : 15),
            family: computed.fontFamily,
            weight: computed.fontWeight,
            lineHeightRatio: lineHeight / base,
            letterSpacing: Number.parseFloat(computed.letterSpacing) || 0,
            valueHeight: valueHeight > 0 ? valueHeight : slot.presentation === 'number' ? Math.max(1, slot.height - 16) : slot.height,
        };
    }

    /** @param {string} text @param {ReadingStyle} style @returns {number} */
    function textWidth(text, style) {
        if (!context) return 0;
        return context.measureText(text).width + Math.max(0, Array.from(text).length - 1) * style.letterSpacing;
    }

    /**
     * CSS normal whitespace with overflow-wrap:anywhere: keep words together
     * when possible and split an otherwise unbreakable word by code point.
     * @param {string} text @param {number} width @param {ReadingStyle} style
     * @param {number} limit @returns {number}
     */
    function wrappedLines(text, width, style, limit) {
        let lines = 1;
        let line = '';
        for (const word of text.trim().split(/\s+/)) {
            const candidate = line ? `${line} ${word}` : word;
            if (textWidth(candidate, style) <= width) {
                line = candidate;
                continue;
            }
            if (line) {
                lines++;
                if (lines > limit) return lines;
                line = '';
            }
            if (textWidth(word, style) <= width) {
                line = word;
                continue;
            }
            for (const character of word) {
                if (line && textWidth(line + character, style) > width) {
                    lines++;
                    if (lines > limit) return lines;
                    line = character;
                } else {
                    line += character;
                }
                if (textWidth(line, style) > width) return limit + 1;
            }
        }
        return lines;
    }

    /** @param {ReadingSlot} slot @param {number} size @returns {boolean} */
    function fits(slot, size) {
        const style = slot.style;
        if (!context || !style) return true;
        context.font = `${style.weight} ${size}px ${style.family}`;
        const height = Math.min(slot.height, style.valueHeight);
        const lineHeight = size * style.lineHeightRatio;
        if (lineHeight > height + 0.25) return false;
        if (slot.presentation === 'number') return textWidth(slot.text, style) <= slot.width;
        const maxLines = Math.max(1, Math.floor((height + 0.25) / lineHeight));
        return wrappedLines(slot.text, slot.width, style, maxLines) <= maxLines;
    }

    /** @param {ReadingSlot} slot @returns {{size:number|null,overflow:boolean}} */
    function fittedSize(slot) {
        const style = slot.style;
        if (!style || !context || !slot.width || !slot.height || fits(slot, style.base)) {
            return { size: null, overflow: false };
        }
        if (!fits(slot, style.min)) return { size: style.min, overflow: true };
        let low = style.min;
        let high = style.base;
        for (let step = 0; step < 9; step++) {
            const middle = (low + high) / 2;
            if (fits(slot, middle)) low = middle;
            else high = middle;
        }
        return { size: Math.floor(low * 100) / 100, overflow: false };
    }

    function fitReadings() {
        if (!active || disposed) return;
        /** @type {ReadingSlot[]} */ const changed = [];
        // Read/cache everything first; later font writes cannot invalidate reads.
        for (const slot of slots.values()) {
            const text = slot.value.textContent ?? '';
            if (text !== slot.text) {
                slot.text = text;
                slot.dirty = true;
            }
            if (!slot.dirty) continue;
            if (slot.initialGeometry) {
                // Only newly registered slots need an initial synchronous size.
                // Subsequent live readings use ResizeObserver's cached geometry.
                const bounds = slot.output.getBoundingClientRect();
                slot.width = bounds.width;
                slot.height = bounds.height;
                slot.initialGeometry = false;
            }
            // Closed categories remain registered, but incur no live layout
            // reads. ResizeObserver wakes them when their slots become visible.
            if (!slot.width || !slot.height) { slot.dirty = false; continue; }
            if (slot.styleDirty || !slot.style) {
                slot.style = readStyle(slot);
                slot.styleDirty = false;
            }
            changed.push(slot);
        }
        for (const slot of changed) {
            const fitted = fittedSize(slot);
            if (fitted.size === null) slot.value.style.removeProperty('font-size');
            else slot.value.style.fontSize = `${fitted.size}px`;
            if (fitted.overflow) slot.output.dataset.readingFitOverflow = 'true';
            else delete slot.output.dataset.readingFitOverflow;
            slot.dirty = false;
        }
        // Canvas estimates wrapping without forcing layout for live numeric
        // values. Browser line-break rules for punctuation and Unicode are
        // authoritative for wrapped strings; certify them in a separate batch.
        const wrapped = changed.filter(slot => slot.presentation !== 'number');
        if (wrapped.length) {
            cancelVerification?.();
            cancelVerification = scope.frame(() => {
                cancelVerification = null;
                if (!active || disposed) return;
                const overflowing = wrapped.filter(slot => slot.output.isConnected && slot.width > 0 && slot.height > 0
                    && (slot.value.scrollHeight > (slot.style?.valueHeight ?? slot.height) + 1 || slot.value.scrollWidth > slot.width + 1));
                for (const slot of overflowing) slot.output.dataset.readingFitOverflow = 'true';
            });
        }
    }

    function close() {
        active = false;
        cancelFrame?.();
        cancelFrame = null;
        cancelVerification?.();
        cancelVerification = null;
        resizeObserver.disconnect();
        for (const slot of slots.values()) {
            slot.value.style.removeProperty('font-size');
            delete slot.output.dataset.readingFitOverflow;
        }
        slots.clear();
        panelWidth = 0;
        panelHeight = 0;
    }

    return {
        refresh() {
            if (disposed || panel.hidden || !panel.classList.contains('observer-panel-telemetry')) return;
            if (!active) {
                active = true;
                resizeObserver.observe(panel);
            }
            /** @type {Set<HTMLElement>} */ const current = new Set();
            for (const output of body.querySelectorAll('.observer-telemetry-reading output')) {
                if (!(output instanceof HTMLElement)) continue;
                const value = output.querySelector('.observer-telemetry-value');
                if (!(value instanceof HTMLElement)) continue;
                current.add(output);
                const presentation = output.closest('.observer-telemetry-reading')?.getAttribute('data-reading-presentation') ?? output.getAttribute('data-reading-presentation');
                const kind = presentation === 'text' || presentation === 'counter' ? presentation : 'number';
                const existing = slots.get(output);
                if (existing) {
                    if (existing.presentation !== kind) {
                        existing.presentation = kind;
                        existing.styleDirty = true;
                        existing.dirty = true;
                    }
                    continue;
                }
                slots.set(output, { output, value, width: 0, height: 0, text: '', presentation: kind, style: null, styleDirty: true, dirty: true, initialGeometry: true });
                resizeObserver.observe(output);
            }
            for (const [output, slot] of slots) {
                if (current.has(output)) continue;
                resizeObserver.unobserve(output);
                slot.value.style.removeProperty('font-size');
                delete output.dataset.readingFitOverflow;
                slots.delete(output);
            }
            schedule();
        },
        close,
        dispose() {
            if (disposed) return;
            close();
            disposed = true;
            scope.dispose();
        },
    };
}
