/**
 * Consciousness Pedagogy — 6-panel Canvas 2D visualization for FTD
 * consciousness theory. This module is the dispatcher: the six draw methods
 * have been extracted to `consciousness/pedagogy-panels/*.js` (ticket CP-3),
 * shared primitives to `consciousness/canvas-primitives.js` (CP-1), and
 * walkthrough content to `consciousness/walkthrough-steps.js` (CP-2).
 *
 * Panels:
 *   0. Master Quadratic Phase Diagram   (phase.js)
 *   1. Complex Plane                    (complex-plane.js)
 *   2. Existence Filter                 (filter.js)
 *   3. ReLU / Softplus Crystallization  (relu.js)
 *   4. Von Neumann Chain                (chain.js)
 *   5. Observer Boundary                (observer.js)
 */

import {
    COEFFICIENT, Y_REAL, Y_IMAG, THETA_C_RAD,
} from './constants.js';
import { prepCanvas, clearCanvas, mulberry32, clamp } from './consciousness/canvas-primitives.js';
import { getWalkthroughSteps } from './consciousness/walkthrough-steps.js';
import { renderMathInHtml } from './ui/math-format/render.js';
import { drawQuadratic }         from './consciousness/pedagogy-panels/phase.js';
import { drawComplexPlane }      from './consciousness/pedagogy-panels/complex-plane.js';
import { drawExistenceFilter }   from './consciousness/pedagogy-panels/filter.js';
import { drawRelu }              from './consciousness/pedagogy-panels/relu.js';
import { drawVonNeumannChain }   from './consciousness/pedagogy-panels/chain.js';
import { drawObserverBoundary }  from './consciousness/pedagogy-panels/observer.js';

// ── Canvas IDs ───────────────────────────────────────────────────────

const CANVAS_IDS = [
    'cs-canvas-quadratic',
    'cs-canvas-complex',
    'cs-canvas-filter',
    'cs-canvas-relu',
    'cs-canvas-chain',
    'cs-canvas-observer',
];

// Panel dispatch table — index matches CANVAS_IDS
const PANEL_DRAWERS = [
    drawQuadratic,
    drawComplexPlane,
    drawExistenceFilter,
    drawRelu,
    drawVonNeumannChain,
    drawObserverBoundary,
];

// ══════════════════════════════════════════════════════════════════════
// Main Class
// ══════════════════════════════════════════════════════════════════════

export class ConsciousnessPedagogy {
    constructor() {
        // Locate canvases
        this._canvases = CANVAS_IDS.map(id => document.getElementById(id));
        this._ctxs = this._canvases.map(c => c ? c.getContext('2d') : null);

        // Locate sliders
        this._kSlider    = document.getElementById('cs-k-slider');
        this._kDisplay   = document.getElementById('cs-k-value');
        this._betaSlider = document.getElementById('cs-beta-slider');
        this._betaDisplay = document.getElementById('cs-beta-value');

        // State
        this._kValue        = COEFFICIENT;  // 16
        this._betaValue     = 1;
        this._filterZ       = { re: Y_REAL, im: Y_IMAG };
        this._phaseAngle    = 0;
        this._chainProgress = 0;
        this._boundaryTick  = 0;
        this._walkthroughStep = -1;
        this._visible       = false;
        this._dirty         = true;
        this._frameId       = null;
        this._frameCount    = 0;

        // Grid state for observer boundary (16x16)
        this._grid = Array.from({ length: 16 }, () => new Int8Array(16));
        this._gridRng = mulberry32(42);
        this._gridMaxRing = 0;

        // Engine data (updated externally)
        this._fluxRatio       = 0;
        this._effTheta        = THETA_C_RAD;
        this._consciousnessI  = 0;

        // Bind slider handlers
        if (this._kSlider) {
            this._kSlider.addEventListener('input', () => {
                this._kValue = parseFloat(this._kSlider.value);
                if (this._kDisplay) this._kDisplay.textContent = this._kValue.toFixed(2);
                this._dirty = true;
            });
        }
        if (this._betaSlider) {
            this._betaSlider.addEventListener('input', () => {
                this._betaValue = parseFloat(this._betaSlider.value);
                if (this._betaDisplay) this._betaDisplay.textContent = this._betaValue.toFixed(1);
                this._dirty = true;
            });
        }

        // Click handler for Existence Filter canvas
        const filterCanvas = this._canvases[2];
        if (filterCanvas) {
            filterCanvas.addEventListener('click', (e) => {
                this._handleFilterClick(e, filterCanvas);
            });
        }

        // Walkthrough elements
        this._wtCanvas = document.getElementById('cs-walk-canvas');
        this._wtText   = document.getElementById('cs-walk-text');

        // Bound loop
        this._loop = this._loop.bind(this);
    }

    // ── Public API ───────────────────────────────────────────────────

    /** Called per frame with live engine data. */
    update(engineData) {
        if (engineData) {
            if (engineData.fluxRatio !== undefined)      this._fluxRatio      = engineData.fluxRatio;
            if (engineData.effTheta !== undefined)        this._effTheta       = engineData.effTheta;
            if (engineData.consciousnessI !== undefined)  this._consciousnessI = engineData.consciousnessI;
        }
        this._dirty = true;
    }

    /** Handle responsive resizing. */
    resize() {
        this._dirty = true;
    }

    /** Theory tab became visible — start animation loop. */
    show() {
        this._visible = true;
        this._dirty = true;
        if (!this._frameId) {
            this._frameId = requestAnimationFrame(this._loop);
        }
    }

    /** Theory tab hidden — stop loop. */
    hide() {
        this._visible = false;
        if (this._frameId) {
            cancelAnimationFrame(this._frameId);
            this._frameId = null;
        }
    }

    /** Cleanup resources. */
    dispose() {
        this.hide();
        this._canvases = [];
        this._ctxs = [];
    }

    // ── Walkthrough ──────────────────────────────────────────────────

    startWalkthrough() {
        this.setWalkthroughStep(0);
    }

    setWalkthroughStep(n) {
        const steps = getWalkthroughSteps();
        if (n < 0) n = 0;
        if (n >= steps.length) n = steps.length - 1;
        this._walkthroughStep = n;
        const step = steps[n];

        // Update walkthrough text. step.text is authored HTML (with LaTeX
        // \(...\) spans); renderMathInHtml locates and renders those via KaTeX.
        if (this._wtText) {
            this._wtText.innerHTML =
                `<h3>${step.title}</h3>` +
                `<div class="cs-wt-body">${renderMathInHtml(step.text)}</div>`;
        }

        // Update title and indicator elements
        const titleEl = document.getElementById('cs-walk-title');
        if (titleEl) titleEl.textContent = `Step ${n + 1}: ${step.title}`;
        const indEl = document.getElementById('cs-walk-indicator');
        if (indEl) indEl.textContent = `${n + 1} / ${steps.length}`;

        this._dirty = true;
    }

    // ── Animation Loop ───────────────────────────────────────────────

    _loop() {
        if (!this._visible) {
            this._frameId = null;
            return;
        }

        // Advance animated state
        this._phaseAngle += 0.005;
        if (this._phaseAngle > 2 * Math.PI) this._phaseAngle -= 2 * Math.PI;

        this._chainProgress += 0.003;
        if (this._chainProgress > 1) this._chainProgress -= 1;

        this._frameCount++;

        // Observer boundary: advance every ~30 frames
        if (this._frameCount % 30 === 0) {
            this._boundaryTick++;
            this._expandCausalPast();
            if (this._boundaryTick > 8) {
                this._resetGrid();
            }
        }

        this._drawAll();
        this._dirty = false;

        this._frameId = requestAnimationFrame(this._loop);
    }

    /** Draw all six canvases (or just the walkthrough target). */
    _drawAll() {
        // If walkthrough is active, draw the relevant panel into the
        // walkthrough canvas, otherwise draw all six.
        if (this._walkthroughStep >= 0 && this._wtCanvas) {
            const steps = getWalkthroughSteps();
            const step = steps[this._walkthroughStep];
            const [ctx, w, h] = prepCanvas(this._wtCanvas);
            clearCanvas(ctx, w, h);
            this._drawPanel(step.panelIndex, ctx, w, h);
        }

        // Always draw the inline panels (they may be visible alongside)
        for (let i = 0; i < 6; i++) {
            const canvas = this._canvases[i];
            if (!canvas) continue;
            // Skip if canvas is not in viewport (simple check)
            if (canvas.clientWidth === 0 || canvas.clientHeight === 0) continue;
            const [ctx, w, h] = prepCanvas(canvas);
            clearCanvas(ctx, w, h);
            this._drawPanel(i, ctx, w, h);
        }
    }

    /** Dispatch to the correct panel renderer. */
    _drawPanel(index, ctx, w, h) {
        const drawer = PANEL_DRAWERS[index];
        if (drawer) drawer(ctx, w, h, this);
    }

    /** Handle click on the filter canvas to set z. */
    _handleFilterClick(e, canvas) {
        const rect = canvas.getBoundingClientRect();
        const clickX = e.clientX - rect.left;
        const clickY = e.clientY - rect.top;

        // Only respond to clicks in the right 60% (the mini complex plane)
        const divX = rect.width * 0.4;
        if (clickX < divX) return;

        const pw = rect.width - divX;
        const cx = divX + pw / 2;
        const cy = rect.height / 2;
        const pad = 25;
        const maxR = 5;
        const scale = (Math.min(pw, rect.height) - 2 * pad) / (2 * maxR);

        const re = (clickX - cx) / scale;
        const im = -(clickY - cy) / scale;

        this._filterZ = { re: clamp(re, -maxR, maxR), im: clamp(im, -maxR, maxR) };
        this._dirty = true;
    }

    /** Expand the causal past by one ring. */
    _expandCausalPast() {
        const tick = this._boundaryTick;
        const cx = 8, cy = 8;
        for (let i = 0; i < 16; i++) {
            for (let j = 0; j < 16; j++) {
                const dist = Math.max(Math.abs(i - cy), Math.abs(j - cx));
                if (dist === tick && this._grid[i][j] === 0) {
                    // Crystallize: assign +1 or -1
                    this._grid[i][j] = this._gridRng() > 0.5 ? 1 : -1;
                }
            }
        }
    }

    /** Reset the grid for a fresh cycle. */
    _resetGrid() {
        this._boundaryTick = 0;
        this._gridMaxRing = 0;
        for (let i = 0; i < 16; i++) {
            this._grid[i].fill(0);
        }
        this._gridRng = mulberry32(42 + this._frameCount);
    }
}

// ══════════════════════════════════════════════════════════════════════
// Info Tooltip Utility
// ══════════════════════════════════════════════════════════════════════

/**
 * Adds "?" info buttons and tooltip divs to all cards in
 * #panel-consciousness that have a `title` attribute.
 *
 * Click the "?" to toggle the tooltip; click outside to dismiss all.
 */
export function addInfoTooltips() {
    const panel = document.getElementById('panel-consciousness');
    if (!panel) return;

    const cards = panel.querySelectorAll('[title]');
    const tooltips = [];

    cards.forEach(card => {
        const tipText = card.getAttribute('title');
        if (!tipText) return;
        card.removeAttribute('title');

        // Create "?" button
        const btn = document.createElement('button');
        btn.className = 'cs-info-btn';
        btn.textContent = '?';
        btn.setAttribute('aria-label', 'Show info');

        // Create tooltip div
        const tip = document.createElement('div');
        tip.className = 'cs-info-tooltip';
        tip.textContent = tipText;
        tip.style.display = 'none';
        tooltips.push(tip);

        // Position the button relative to the card
        const wrapper = card.style.position === 'relative' ? card : card;
        if (getComputedStyle(wrapper).position === 'static') {
            wrapper.style.position = 'relative';
        }

        btn.addEventListener('click', (e) => {
            e.stopPropagation();
            const showing = tip.style.display !== 'none';
            // Close all first
            tooltips.forEach(t => { t.style.display = 'none'; });
            // Toggle this one
            if (!showing) tip.style.display = 'block';
        });

        card.appendChild(btn);
        card.appendChild(tip);
    });

    // Click outside closes all tooltips
    document.addEventListener('click', () => {
        tooltips.forEach(t => { t.style.display = 'none'; });
    });
}
