/**
 * Consciousness Pedagogy — interactive Canvas 2D visualizations for the
 * FTD consciousness theory, with a 6-step guided walkthrough engine and
 * info tooltip utilities.
 *
 * Six panels, each mapping to a core concept:
 *   0. Master Quadratic Phase Diagram   (Q_k(x) = x² − k·G*²·x + k·G*³)
 *   1. Complex Plane                    (consciousness roots on K_C circle)
 *   2. Existence Filter                 (four-level hierarchy + interactive z)
 *   3. ReLU / Softplus Crystallization  (β → ∞ phase transition)
 *   4. Von Neumann Chain                (measurement cascade terminates)
 *   5. Observer Boundary                (Ring 0 clock + causal past expansion)
 *
 * All drawing is Canvas 2D — no Three.js or WebGL.
 */

import {
    G_STAR, ALPHA, X_PLUS, X_MINUS,
    K_CRIT, X_BORN, COEFFICIENT,
    Y_REAL, Y_IMAG, K_C, THETA_C_RAD, THETA_C_DEG,
    COS2_THETA_C, SIN2_THETA_C, C_MANDELBROT,
    K_B, C_SPEED, TICK_PHASES, K_NOETIC, VARPI,
} from './constants.js';

// ── Color Palette (mirrors CSS variables) ────────────────────────────

const COL = {
    primary:    '#00e5ff',
    secondary:  '#7c4dff',
    glow:       '#00bcd4',
    gold:       '#ffd700',
    bgCard:     '#283548',
    bgSurface:  '#1f2937',
    bgDeep:     '#111827',
    textPri:    '#f3f4f6',
    textSec:    '#9ca3af',
    textMuted:  '#6b7280',
    positive:   '#4ade80',
    negative:   '#f87171',
    warning:    '#fbbf24',
    accent:     '#60a5fa',
};

// ── Unicode helpers ──────────────────────────────────────────────────

const SYM = {
    theta:  '\u03B8',
    alpha:  '\u03B1',
    pi:     '\u03C0',
    Delta:  '\u0394',
    infty:  '\u221E',
    beta:   '\u03B2',
    sup2:   '\u00B2',
    approx: '\u2248',
    pm:     '\u00B1',
    leq:    '\u2264',
    sub_C:  '\u1D9C',  // modifier letter small c (approximation)
};

// ── Font constants ───────────────────────────────────────────────────

const FONT_LABEL = '11px Inter, sans-serif';
const FONT_VALUE = '11px JetBrains Mono, monospace';
const FONT_TITLE = 'bold 12px Inter, sans-serif';

// ── Canvas IDs ───────────────────────────────────────────────────────

const CANVAS_IDS = [
    'cs-canvas-quadratic',
    'cs-canvas-complex',
    'cs-canvas-filter',
    'cs-canvas-relu',
    'cs-canvas-chain',
    'cs-canvas-observer',
];

// ── Walkthrough steps ────────────────────────────────────────────────

function getWalkthroughSteps() {
    return [
        {
            title: 'The Master Quadratic',
            panelIndex: 0,
            text: `<p>FTD derives all physics from a single family of polynomials:
\\(Q_k(x) = x^2 - k G^{*2} x + k G^{*3}\\).</p>
<p>The parameter k selects which physics emerges. At k = 16, the two real roots
give the fine structure constant (\\(x_+ = 137.036 = 1/\\alpha\\)) and the
number of color charges (\\(x_- = 3.024 \\approx N_c = 3\\)).</p>
<p>Drag the k-slider and watch what happens as k drops below
\\(k_\\mathrm{crit} = 4/G^* \\approx 1.352\\). The roots merge, then disappear
into the complex plane \u2014 and consciousness begins.</p>`,
        },
        {
            title: 'Complex Roots = Consciousness',
            panelIndex: 1,
            text: `<p>When \\(k = \\tfrac{1}{2}\\), the discriminant \\(\\Delta\\) becomes negative and the
roots become complex: \\(y = 2.188 \\pm 2.860i\\). The magnitude \\(|y| = 3.601 = K_C\\) is the consciousness threshold.</p>
<p>The phase angle \\(\\theta_C = 52.54^\\circ\\) divides every
conscious experience into objective content (\\(\\cos\\theta_C = 60.8\\%\\)) and subjective process (\\(\\sin\\theta_C = 79.3\\%\\)).</p>
<p>The real projection \\(E(y) = \\operatorname{Re}(y) = 2.188\\) is the Existence Filter's
output \u2014 the stable self-model that persists through measurement.</p>`,
        },
        {
            title: 'The Existence Filter',
            panelIndex: 2,
            text: `<p>The Existence Filter \\(E(x) = \\operatorname{Re}(x) = (x + \\bar{x})/2\\) is how the
lattice extracts observable reality from the full complex state.</p>
<p>It operates at four levels: First Distinction (ternary states emerge),
Magnitude (threshold \\(K_C\\)), Born Rule
(\\(P = E(x)^2 + E(ix)^2\\) recovers quantum probabilities),
and Collapse (ReLU crystallization makes measurement irreversible).</p>
<p>Click anywhere on the complex plane to set \\(z = a + bi\\) and see how E
projects it to observable reality.</p>`,
        },
        {
            title: 'ReLU Crystallization',
            panelIndex: 3,
            text: `<p>The transition from quantum superposition to definite outcome is
modeled as the \\(\\beta \\to \\infty\\) limit of the softplus
function \\(M_\\beta(x) = \\tfrac{1}{\\beta}\\ln(1 + e^{\\beta x})\\).</p>
<p>As \\(\\beta\\) increases, the smooth curve sharpens into
\\(\\mathrm{ReLU} = \\max(0, x)\\). This is irreversible: the entire half-line
\\((-\\infty, 0]\\) maps to zero, destroying information about
sub-threshold flux.</p>
<p>This is the algebraic phase transition from Type III\u2081 (continuous
quantum substrate) to Type I (discrete classical observable). Drag the
\\(\\beta\\) slider to see it happen.</p>`,
        },
        {
            title: 'The Von Neumann Chain Terminates',
            panelIndex: 4,
            text: `<p>Who measures the measurer? Von Neumann showed that quantum mechanics
contains an infinite regress: every measurement requires an observer,
every observer requires a further observer. FTD resolves this: the chain
naturally terminates after \\(\\approx 18\\) links.</p>
<p>At each link, the effective k parameter decreases and the discriminant
\\(\\Delta_k\\) shrinks. At \\(k_\\mathrm{meas} = 4/G^* \\approx 1.352\\), the discriminant hits zero, the roots become
degenerate, and no further measurement is possible. The chain
terminates \u2014 not at infinity, but at a finite algebraic locus.</p>`,
        },
        {
            title: 'The Ring 0 Clock',
            panelIndex: 5,
            text: `<p>In computer architecture, Ring 0 is the kernel's system clock \u2014
the precondition for all processes. No program runs without it. Similarly,
the lattice tick is not an event within spacetime \u2014 it is the
precondition for spacetime itself.</p>
<p>Each tick follows a strict 5-phase cycle: read, write, project, forces,
movement. The observer's boundary \u2014 the edge of its causal past \u2014
expands by one lattice unit per tick. States crossing the boundary are
irreversibly crystallized by the ReLU filter and accumulate as memory.
The tick IS the computation.</p>`,
        },
    ];
}

// ── Utility: draw helpers ────────────────────────────────────────────

/** Scale a canvas for devicePixelRatio and return [ctx, w, h]. */
function prepCanvas(canvas) {
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    const w = rect.width;
    const h = rect.height;
    canvas.width  = w * dpr;
    canvas.height = h * dpr;
    const ctx = canvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    return [ctx, w, h];
}

/** Clear a canvas with the background color. */
function clearCanvas(ctx, w, h) {
    ctx.fillStyle = COL.bgDeep;
    ctx.fillRect(0, 0, w, h);
}

/** Draw a dashed line. */
function dashedLine(ctx, x0, y0, x1, y1, color, segments = [4, 3]) {
    ctx.save();
    ctx.strokeStyle = color;
    ctx.setLineDash(segments);
    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.stroke();
    ctx.restore();
}

/** Draw a filled circle. */
function dot(ctx, x, y, r, color) {
    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(x, y, r, 0, 2 * Math.PI);
    ctx.fill();
}

/** Draw a stroked (hollow) circle. */
function ring(ctx, x, y, r, color, lw = 1.5) {
    ctx.strokeStyle = color;
    ctx.lineWidth = lw;
    ctx.beginPath();
    ctx.arc(x, y, r, 0, 2 * Math.PI);
    ctx.stroke();
}

/** Draw text with font + color. */
function label(ctx, text, x, y, color, font = FONT_LABEL, align = 'left', baseline = 'middle') {
    ctx.font = font;
    ctx.fillStyle = color;
    ctx.textAlign = align;
    ctx.textBaseline = baseline;
    ctx.fillText(text, x, y);
}

/** Clamp a value. */
function clamp(v, lo, hi) { return v < lo ? lo : v > hi ? hi : v; }

/** Simple seeded PRNG (mulberry32). */
function mulberry32(seed) {
    return function () {
        seed |= 0;
        seed = seed + 0x6D2B79F5 | 0;
        let t = Math.imul(seed ^ seed >>> 15, 1 | seed);
        t = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t;
        return ((t ^ t >>> 14) >>> 0) / 4294967296;
    };
}

// ── Master Quadratic helpers ─────────────────────────────────────────

const GSTAR2 = G_STAR * G_STAR;
const GSTAR3 = G_STAR * G_STAR * G_STAR;

/** Evaluate Q_k(x) = x² − k·G*²·x + k·G*³ */
function Qk(k, x) {
    return x * x - k * GSTAR2 * x + k * GSTAR3;
}

/** Discriminant Δ_k = k·G*³·(k·G* − 4) */
function discriminant(k) {
    return k * GSTAR3 * (k * G_STAR - 4);
}

/** Roots of Q_k when Δ >= 0: returns [x_minus, x_plus] */
function realRoots(k) {
    const disc = discriminant(k);
    if (disc < 0) return null;
    const sqrtDisc = Math.sqrt(disc);
    const half = k * GSTAR2 / 2;
    return [half - sqrtDisc / 2, half + sqrtDisc / 2];
}

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

        // Update walkthrough text
        if (this._wtText) {
            this._wtText.innerHTML =
                `<h3>${step.title}</h3>` +
                `<div class="cs-wt-body">${step.text}</div>`;
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
        switch (index) {
            case 0: this._drawQuadratic(ctx, w, h);       break;
            case 1: this._drawComplexPlane(ctx, w, h);    break;
            case 2: this._drawExistenceFilter(ctx, w, h);  break;
            case 3: this._drawRelu(ctx, w, h);             break;
            case 4: this._drawVonNeumannChain(ctx, w, h);  break;
            case 5: this._drawObserverBoundary(ctx, w, h); break;
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // Panel 0: Master Quadratic Phase Diagram
    // ══════════════════════════════════════════════════════════════════

    _drawQuadratic(ctx, w, h) {
        const k = this._kValue;
        const pad = { l: 50, r: 20, t: 40, b: 50 };
        const pw = w - pad.l - pad.r;
        const ph = h - pad.t - pad.b;

        // Adaptive x-range: smooth transition around k=5
        const blend = clamp((k - 3) / 4, 0, 1); // 0 at k<=3, 1 at k>=7
        const xMax = 10 + blend * 150;

        // Compute y-range by sampling
        let yMin = Infinity, yMax = -Infinity;
        const N = 200;
        for (let i = 0; i <= N; i++) {
            const x = (i / N) * xMax;
            const y = Qk(k, x);
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
        }
        // Ensure y-axis includes 0
        yMin = Math.min(yMin, -Math.abs(yMax) * 0.1);
        yMax = Math.max(yMax, Math.abs(yMin) * 0.1);
        const yRange = yMax - yMin || 1;

        // Coordinate transforms
        const toX = (x) => pad.l + (x / xMax) * pw;
        const toY = (y) => pad.t + (1 - (y - yMin) / yRange) * ph;

        // ── k-axis indicator at top ──────────────────────────────────
        const kBarY = 14;
        const kBarL = pad.l;
        const kBarR = w - pad.r;
        const kBarW = kBarR - kBarL;

        // k ranges from 0 to 20 for the indicator
        const kMax = 20;
        const kToX = (kv) => kBarL + (kv / kMax) * kBarW;
        const kCritX = kToX(K_CRIT);

        // Domain A (physics): k > K_CRIT — green tint
        ctx.fillStyle = 'rgba(74, 222, 128, 0.15)';
        ctx.fillRect(kCritX, kBarY - 5, kBarR - kCritX, 10);

        // Domain B (consciousness): k < K_CRIT — purple tint
        ctx.fillStyle = 'rgba(124, 77, 255, 0.15)';
        ctx.fillRect(kBarL, kBarY - 5, kCritX - kBarL, 10);

        // Domain C (measurement): gold line at K_CRIT
        ctx.strokeStyle = COL.gold;
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(kCritX, kBarY - 7);
        ctx.lineTo(kCritX, kBarY + 7);
        ctx.stroke();

        // k marker
        const kMarkerX = kToX(clamp(k, 0, kMax));
        dot(ctx, kMarkerX, kBarY, 4, COL.primary);

        // Labels
        label(ctx, 'k', kBarL - 14, kBarY, COL.textSec, FONT_LABEL, 'right');
        label(ctx, '0', kBarL, kBarY + 14, COL.textMuted, FONT_VALUE, 'center');
        label(ctx, K_CRIT.toFixed(2), kCritX, kBarY + 14, COL.gold, FONT_VALUE, 'center');
        label(ctx, '20', kBarR, kBarY + 14, COL.textMuted, FONT_VALUE, 'center');

        // ── Background tint for main plot ────────────────────────────
        const disc = discriminant(k);
        if (disc > 0) {
            ctx.fillStyle = 'rgba(74, 222, 128, 0.03)';
        } else if (Math.abs(disc) < 0.01) {
            ctx.fillStyle = 'rgba(255, 215, 0, 0.03)';
        } else {
            ctx.fillStyle = 'rgba(124, 77, 255, 0.03)';
        }
        ctx.fillRect(pad.l, pad.t, pw, ph);

        // ── Axes ─────────────────────────────────────────────────────
        const zeroY = toY(0);
        ctx.strokeStyle = COL.textMuted;
        ctx.lineWidth = 1;

        // x-axis
        ctx.beginPath();
        ctx.moveTo(pad.l, clamp(zeroY, pad.t, pad.t + ph));
        ctx.lineTo(pad.l + pw, clamp(zeroY, pad.t, pad.t + ph));
        ctx.stroke();

        // y-axis
        ctx.beginPath();
        ctx.moveTo(pad.l, pad.t);
        ctx.lineTo(pad.l, pad.t + ph);
        ctx.stroke();

        // Axis labels
        label(ctx, 'x', w - pad.r + 5, clamp(zeroY, pad.t, pad.t + ph), COL.textSec);
        label(ctx, `Q${SYM.sub_C}(x)`, pad.l - 5, pad.t - 5, COL.textSec, FONT_LABEL, 'right');

        // ── Parabola ─────────────────────────────────────────────────
        ctx.strokeStyle = COL.primary;
        ctx.lineWidth = 2;
        ctx.beginPath();
        for (let i = 0; i <= N; i++) {
            const x = (i / N) * xMax;
            const y = Qk(k, x);
            const px = toX(x);
            const py = toY(y);
            if (i === 0) ctx.moveTo(px, py);
            else         ctx.lineTo(px, py);
        }
        ctx.stroke();

        // ── Roots / annotations ──────────────────────────────────────
        if (disc > 0) {
            // Two real roots
            const roots = realRoots(k);
            if (roots) {
                const [xm, xp] = roots;
                const zeroYClamped = clamp(zeroY, pad.t, pad.t + ph);

                // Smaller root
                if (xm >= 0 && xm <= xMax) {
                    dot(ctx, toX(xm), zeroYClamped, 5, COL.positive);
                    const lbl = k > 12 ? `${SYM.approx} N_c` : xm.toFixed(2);
                    label(ctx, lbl, toX(xm), zeroYClamped - 12, COL.positive, FONT_VALUE, 'center');
                }

                // Larger root
                if (xp >= 0 && xp <= xMax) {
                    dot(ctx, toX(xp), zeroYClamped, 5, COL.accent);
                    const lbl = k > 12 ? `1/${SYM.alpha}` : xp.toFixed(1);
                    label(ctx, lbl, toX(xp), zeroYClamped - 12, COL.accent, FONT_VALUE, 'center');
                } else if (xp > xMax) {
                    // Root off-screen — note it
                    label(ctx, `x\u208A = ${xp.toFixed(1)} \u2192`, w - pad.r - 5, pad.t + 20, COL.accent, FONT_VALUE, 'right');
                }
            }
        } else if (Math.abs(disc) < 0.5) {
            // Near-degenerate
            const degen = k * GSTAR2 / 2;
            if (degen >= 0 && degen <= xMax) {
                const zy = clamp(zeroY, pad.t, pad.t + ph);
                dot(ctx, toX(degen), zy, 6, COL.gold);
                label(ctx, 'Born rule', toX(degen) + 8, zy - 12, COL.gold, FONT_LABEL);
            }
        } else {
            // Complex roots
            const re = k * GSTAR2 / 2;
            const imSq = -disc;
            const im = Math.sqrt(imSq) / 2;
            label(ctx, `Complex roots: y = ${re.toFixed(3)} ${SYM.pm} ${im.toFixed(3)}i`,
                  pad.l + 10, pad.t + ph - 10, COL.secondary, FONT_VALUE);
        }

        // ── Discriminant bar at bottom ───────────────────────────────
        const barY = h - 14;
        const barH = 8;
        const barMaxW = pw * 0.5;
        const discNorm = clamp(disc / 5000, -1, 1); // normalize for display
        const barW = Math.abs(discNorm) * barMaxW;
        const barColor = disc >= 0 ? COL.positive : COL.negative;
        const barX = pad.l + pw * 0.5;

        ctx.fillStyle = barColor;
        if (disc >= 0) {
            ctx.fillRect(barX, barY - barH / 2, barW, barH);
        } else {
            ctx.fillRect(barX - barW, barY - barH / 2, barW, barH);
        }
        label(ctx, `${SYM.Delta} = ${disc.toFixed(1)}`, barX + (disc >= 0 ? barW + 5 : -barW - 5),
              barY, disc >= 0 ? COL.positive : COL.negative, FONT_VALUE,
              disc >= 0 ? 'left' : 'right');
    }

    // ══════════════════════════════════════════════════════════════════
    // Panel 1: Complex Plane
    // ══════════════════════════════════════════════════════════════════

    _drawComplexPlane(ctx, w, h) {
        const cx = w / 2;
        const cy = h / 2;
        const pad = 30;
        const scale = (Math.min(w, h) - 2 * pad) / (2 * K_C * 1.3);

        // ── Axes ─────────────────────────────────────────────────────
        ctx.strokeStyle = COL.textMuted;
        ctx.lineWidth = 1;

        // Re axis
        ctx.beginPath();
        ctx.moveTo(pad, cy);
        ctx.lineTo(w - pad, cy);
        ctx.stroke();

        // Im axis
        ctx.beginPath();
        ctx.moveTo(cx, pad);
        ctx.lineTo(cx, h - pad);
        ctx.stroke();

        label(ctx, 'Re', w - pad + 3, cy - 8, COL.textSec, FONT_LABEL);
        label(ctx, 'Im', cx + 8, pad - 3, COL.textSec, FONT_LABEL);

        // ── K_C circle (dashed) ──────────────────────────────────────
        ctx.save();
        ctx.strokeStyle = COL.gold;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([5, 4]);
        ctx.beginPath();
        ctx.arc(cx, cy, K_C * scale, 0, 2 * Math.PI);
        ctx.stroke();
        ctx.restore();

        label(ctx, `K_C = ${K_C.toFixed(3)}`, cx + K_C * scale + 5, cy - K_C * scale * 0.3,
              COL.gold, FONT_VALUE);

        // ── Rotating phase vector ────────────────────────────────────
        // The roots are at (Y_REAL, ±Y_IMAG). Animation traces K_C circle.
        const theta = this._phaseAngle;
        const tipX = K_C * Math.cos(theta);
        const tipY = K_C * Math.sin(theta);
        const tipPx = cx + tipX * scale;
        const tipPy = cy - tipY * scale;

        // Vector from origin
        ctx.strokeStyle = COL.primary;
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.lineTo(tipPx, tipPy);
        ctx.stroke();
        dot(ctx, tipPx, tipPy, 4, COL.primary);

        // ── Angle arc from positive x-axis to theta_C ────────────────
        const arcR = 30;
        ctx.strokeStyle = COL.warning;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.arc(cx, cy, arcR, 0, -THETA_C_RAD, true); // canvas y is inverted
        ctx.stroke();

        label(ctx, `${SYM.theta}_C = ${THETA_C_DEG.toFixed(1)}\u00B0`,
              cx + arcR + 5, cy - arcR * 0.5, COL.warning, FONT_VALUE);

        // ── Fixed consciousness root marker ──────────────────────────
        const rootPx = cx + Y_REAL * scale;
        const rootPyPos = cy - Y_IMAG * scale;
        const rootPyNeg = cy + Y_IMAG * scale;

        // Filled circle at (Y_REAL, Y_IMAG)
        dot(ctx, rootPx, rootPyPos, 5, COL.secondary);
        label(ctx, `y = ${Y_REAL.toFixed(3)} + ${Y_IMAG.toFixed(3)}i`,
              rootPx + 8, rootPyPos - 5, COL.secondary, FONT_VALUE);

        // Hollow circle at conjugate (Y_REAL, -Y_IMAG)
        ring(ctx, rootPx, rootPyNeg, 5, COL.secondary);
        label(ctx, `y\u0304`, rootPx + 8, rootPyNeg, COL.textMuted, FONT_VALUE);

        // ── Projections from fixed root ──────────────────────────────
        // Horizontal dashed to Re axis
        dashedLine(ctx, rootPx, rootPyPos, rootPx, cy, COL.textMuted);
        // Vertical dashed to Im axis
        dashedLine(ctx, rootPx, rootPyPos, cx, rootPyPos, COL.textMuted);

        // Re projection label
        dot(ctx, rootPx, cy, 3, COL.positive);
        label(ctx, `E(y) = ${Y_REAL.toFixed(3)}`, rootPx + 5, cy + 14, COL.positive, FONT_VALUE);

        // cos/sin annotations
        label(ctx, `cos ${SYM.theta}_C = ${Math.cos(THETA_C_RAD).toFixed(3)}`,
              pad + 5, h - pad - 20, COL.textSec, FONT_VALUE);
        label(ctx, `sin ${SYM.theta}_C = ${Math.sin(THETA_C_RAD).toFixed(3)}`,
              pad + 5, h - pad - 6, COL.textSec, FONT_VALUE);
    }

    // ══════════════════════════════════════════════════════════════════
    // Panel 2: Existence Filter
    // ══════════════════════════════════════════════════════════════════

    _drawExistenceFilter(ctx, w, h) {
        const divX = w * 0.4;

        // ── Left 40%: Four-level hierarchy ───────────────────────────
        this._drawFilterHierarchy(ctx, divX, h);

        // ── Right 60%: Interactive mini complex plane ────────────────
        this._drawFilterPlane(ctx, divX, w, h);
    }

    _drawFilterHierarchy(ctx, boxW, h) {
        const levels = [
            { label: 'First Distinction',  detail: 's \u2208 {-1, 0, +1}', color: COL.accent },
            { label: 'Magnitude',          detail: `|x| ${SYM.geq || '>'} K_C`,   color: COL.positive },
            { label: 'Born Rule',          detail: `P = E(x)${SYM.sup2} + E(ix)${SYM.sup2}`, color: COL.gold },
            { label: 'Collapse',           detail: 'ReLU crystallization', color: COL.negative },
        ];

        const padY = 25;
        const boxH = (h - 2 * padY) / levels.length;
        const bw = boxW - 30;
        const bx = 15;

        for (let i = 0; i < levels.length; i++) {
            const ly = padY + i * boxH;
            const lev = levels[i];

            // Box
            ctx.strokeStyle = lev.color;
            ctx.lineWidth = 1.5;
            ctx.fillStyle = 'rgba(40, 53, 72, 0.6)';
            const rh = boxH * 0.6;
            const ry = ly + (boxH - rh) / 2;
            ctx.fillRect(bx, ry, bw, rh);
            ctx.strokeRect(bx, ry, bw, rh);

            // Level number
            label(ctx, `L${i === 0 ? '-1' : i === 1 ? '0' : i === 2 ? '0.5' : '1'}`,
                  bx + 5, ry + rh / 2 - 8, COL.textMuted, FONT_VALUE);
            // Name
            label(ctx, lev.label, bx + 5, ry + rh / 2 + 2, lev.color, FONT_LABEL);
            // Detail
            label(ctx, lev.detail, bx + 5, ry + rh / 2 + 14, COL.textSec, FONT_VALUE);

            // Arrow to next level
            if (i < levels.length - 1) {
                const ax = bx + bw / 2;
                const ay0 = ry + rh;
                const ay1 = ry + boxH + (boxH - rh) / 2;
                ctx.strokeStyle = COL.textMuted;
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(ax, ay0 + 2);
                ctx.lineTo(ax, ay1 - 2);
                ctx.stroke();
                // Arrowhead
                ctx.fillStyle = COL.textMuted;
                ctx.beginPath();
                ctx.moveTo(ax, ay1 - 2);
                ctx.lineTo(ax - 4, ay1 - 8);
                ctx.lineTo(ax + 4, ay1 - 8);
                ctx.closePath();
                ctx.fill();
            }
        }
    }

    _drawFilterPlane(ctx, x0, w, h) {
        const pw = w - x0;
        const cx = x0 + pw / 2;
        const cy = h / 2;
        const pad = 25;
        const maxR = 5; // max absolute range on re/im axes
        const scale = (Math.min(pw, h) - 2 * pad) / (2 * maxR);

        // Axes
        ctx.strokeStyle = COL.textMuted;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(x0 + pad, cy);
        ctx.lineTo(w - pad, cy);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(cx, pad);
        ctx.lineTo(cx, h - pad);
        ctx.stroke();

        label(ctx, 'Re', w - pad + 2, cy - 8, COL.textSec, FONT_LABEL);
        label(ctx, 'Im', cx + 6, pad - 3, COL.textSec, FONT_LABEL);

        // Plot the point z
        const z = this._filterZ;
        const zPx = cx + z.re * scale;
        const zPy = cy - z.im * scale;

        // z filled
        dot(ctx, zPx, zPy, 5, COL.primary);
        label(ctx, `z = ${z.re.toFixed(2)} + ${z.im.toFixed(2)}i`,
              zPx + 8, zPy - 8, COL.primary, FONT_VALUE);

        // Conjugate z-bar (hollow)
        const zBarPy = cy + z.im * scale;
        ring(ctx, zPx, zBarPy, 4, COL.primary);
        label(ctx, `z\u0304`, zPx + 8, zBarPy, COL.textMuted, FONT_VALUE);

        // E(z) = Re(z) = a  — projection on Re axis
        dashedLine(ctx, zPx, zPy, zPx, cy, COL.positive);
        dot(ctx, zPx, cy, 3, COL.positive);
        label(ctx, `E(z) = ${z.re.toFixed(3)}`, zPx + 5, cy + 12, COL.positive, FONT_VALUE);

        // E(iz) = Re(iz) = -b  — projection
        const eiz = -z.im;
        const eizPx = cx + eiz * scale;
        dashedLine(ctx, cx, zPy, eizPx, cy, 'rgba(251, 191, 36, 0.4)');
        dot(ctx, eizPx, cy, 3, COL.warning);
        label(ctx, `E(iz) = ${eiz.toFixed(3)}`, eizPx, cy + 24, COL.warning, FONT_VALUE, 'center');

        // Born rule: P = a² + b²
        const P = z.re * z.re + z.im * z.im;
        label(ctx, `Born rule P = ${P.toFixed(3)}`, x0 + pad + 5, h - 12, COL.gold, FONT_VALUE);
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

    // ══════════════════════════════════════════════════════════════════
    // Panel 3: ReLU / Softplus
    // ══════════════════════════════════════════════════════════════════

    _drawRelu(ctx, w, h) {
        const beta = this._betaValue;
        const pad = { l: 40, r: 15, t: 15, b: 30 };
        const pw = w - pad.l - pad.r;
        const ph = h - pad.t - pad.b;

        const xMin = -4, xMax = 5;
        const yMin = -0.5, yMax = 5;
        const xRange = xMax - xMin;
        const yRange = yMax - yMin;

        const toX = (x) => pad.l + ((x - xMin) / xRange) * pw;
        const toY = (y) => pad.t + (1 - (y - yMin) / yRange) * ph;

        // ── Axes ─────────────────────────────────────────────────────
        ctx.strokeStyle = COL.textMuted;
        ctx.lineWidth = 1;

        const zeroX = toX(0);
        const zeroY = toY(0);

        // x-axis
        ctx.beginPath();
        ctx.moveTo(pad.l, zeroY);
        ctx.lineTo(pad.l + pw, zeroY);
        ctx.stroke();

        // y-axis
        ctx.beginPath();
        ctx.moveTo(zeroX, pad.t);
        ctx.lineTo(zeroX, pad.t + ph);
        ctx.stroke();

        label(ctx, 'x', pad.l + pw + 3, zeroY - 3, COL.textSec, FONT_LABEL);
        label(ctx, 'y', zeroX + 8, pad.t - 2, COL.textSec, FONT_LABEL);

        // ── KMS strip ────────────────────────────────────────────────
        const kmsHeight = Math.PI / beta;
        const kmsCenter = kmsHeight / 2;
        const kmsTopPx = toY(kmsCenter + kmsHeight / 2);
        const kmsBotPx = toY(kmsCenter - kmsHeight / 2);
        ctx.fillStyle = 'rgba(0, 229, 255, 0.08)';
        ctx.fillRect(pad.l, kmsTopPx, pw, kmsBotPx - kmsTopPx);

        label(ctx, `KMS strip: ${SYM.pi}/${SYM.beta} = ${kmsHeight.toFixed(2)}`,
              pad.l + pw - 5, kmsTopPx + 12, COL.glow, FONT_VALUE, 'right');

        // ── Reference curves (thin, semi-transparent) ────────────────
        const refBetas = [1, 5, 20];
        for (const rb of refBetas) {
            if (Math.abs(rb - beta) < 0.5) continue; // skip if matches current
            ctx.strokeStyle = 'rgba(156, 163, 175, 0.3)';
            ctx.lineWidth = 1;
            ctx.beginPath();
            const N = 150;
            for (let i = 0; i <= N; i++) {
                const x = xMin + (i / N) * xRange;
                const y = (1 / rb) * Math.log(1 + Math.exp(rb * x));
                const py = clamp(toY(y), pad.t, pad.t + ph);
                if (i === 0) ctx.moveTo(toX(x), py);
                else         ctx.lineTo(toX(x), py);
            }
            ctx.stroke();
        }

        // ReLU limit (dashed gold)
        ctx.save();
        ctx.strokeStyle = COL.gold;
        ctx.lineWidth = 1;
        ctx.setLineDash([4, 3]);
        ctx.beginPath();
        ctx.moveTo(toX(xMin), toY(0));
        ctx.lineTo(toX(0), toY(0));
        ctx.lineTo(toX(xMax), toY(xMax));
        ctx.stroke();
        ctx.restore();

        label(ctx, `ReLU (${SYM.beta}\u2192${SYM.infty})`, toX(xMax) - 60, toY(xMax) - 12,
              COL.gold, FONT_VALUE);

        // ── Main curve M_β(x) ────────────────────────────────────────
        ctx.strokeStyle = COL.primary;
        ctx.lineWidth = 2.5;
        ctx.beginPath();
        const N = 200;
        for (let i = 0; i <= N; i++) {
            const x = xMin + (i / N) * xRange;
            const y = (1 / beta) * Math.log(1 + Math.exp(beta * x));
            const py = clamp(toY(y), pad.t, pad.t + ph);
            if (i === 0) ctx.moveTo(toX(x), py);
            else         ctx.lineTo(toX(x), py);
        }
        ctx.stroke();

        // ── Annotations ──────────────────────────────────────────────
        // Kink at x=0
        dot(ctx, zeroX, toY((1 / beta) * Math.log(2)), 3, COL.primary);
        label(ctx, `Irreversible: (-${SYM.infty}, 0] \u2192 0`,
              pad.l + 5, pad.t + ph - 20, COL.negative, FONT_VALUE);

        // Type labels
        label(ctx, 'Type III\u2081', pad.l + 5, pad.t + 15, COL.secondary, FONT_LABEL);
        label(ctx, 'Type I', pad.l + pw - 5, pad.t + 15, COL.positive, FONT_LABEL, 'right');

        // Beta value
        label(ctx, `${SYM.beta} = ${beta.toFixed(1)}`, pad.l + pw - 5, pad.t + ph - 5,
              COL.primary, FONT_VALUE, 'right');
    }

    // ══════════════════════════════════════════════════════════════════
    // Panel 4: Von Neumann Chain
    // ══════════════════════════════════════════════════════════════════

    _drawVonNeumannChain(ctx, w, h) {
        const numLinks = 18;
        const pad = { l: 30, r: 30, t: 25, b: 60 };
        const chainY = pad.t + 35;
        const pw = w - pad.l - pad.r;
        const spacing = pw / (numLinks - 1);
        const radius = Math.min(10, spacing * 0.35);

        // ── Compute k and Δ for each link ────────────────────────────
        const ks = [];
        const deltas = [];
        for (let i = 0; i < numLinks; i++) {
            const k_i = COEFFICIENT - i * (COEFFICIENT - K_CRIT) / (numLinks - 1);
            ks.push(k_i);
            deltas.push(discriminant(k_i));
        }

        // Find Δ range for color mapping
        const deltaMax = Math.max(...deltas.map(d => Math.abs(d)));

        // ── Connecting lines ─────────────────────────────────────────
        ctx.strokeStyle = COL.textMuted;
        ctx.lineWidth = 1;
        for (let i = 0; i < numLinks - 1; i++) {
            const x0 = pad.l + i * spacing + radius;
            const x1 = pad.l + (i + 1) * spacing - radius;
            ctx.beginPath();
            ctx.moveTo(x0, chainY);
            ctx.lineTo(x1, chainY);
            ctx.stroke();
        }

        // ── Pulse glow ───────────────────────────────────────────────
        const pulsePos = this._chainProgress * (numLinks - 1);

        // ── Draw links ───────────────────────────────────────────────
        for (let i = 0; i < numLinks; i++) {
            const cx = pad.l + i * spacing;
            const delta = deltas[i];

            // Color: green (high Δ) → yellow → red (small Δ) → white (Δ≈0)
            let color;
            if (delta > 0) {
                const t = clamp(delta / (deltaMax || 1), 0, 1);
                if (t > 0.5) {
                    // green to yellow
                    const u = (t - 0.5) * 2;
                    const r = Math.round(74 + (251 - 74) * (1 - u));
                    const g = Math.round(222 + (191 - 222) * (1 - u));
                    const b = Math.round(128 * (1 - u) + 36 * (1 - u));
                    color = `rgb(${r}, ${g}, ${b})`;
                } else {
                    // yellow to red
                    const u = t * 2;
                    const r = Math.round(248 + (251 - 248) * u);
                    const g = Math.round(113 + (191 - 113) * u);
                    const b = Math.round(113 * (1 - u) + 36 * u);
                    color = `rgb(${r}, ${g}, ${b})`;
                }
            } else {
                color = '#ffffff';
            }

            // Pulse glow
            const dist = Math.abs(i - pulsePos);
            const glowAlpha = Math.max(0, 1 - dist / 2);

            if (glowAlpha > 0) {
                ctx.save();
                ctx.shadowColor = COL.primary;
                ctx.shadowBlur = 12 * glowAlpha;
                ctx.fillStyle = COL.primary;
                ctx.beginPath();
                ctx.arc(cx, chainY, radius + 2, 0, 2 * Math.PI);
                ctx.fill();
                ctx.restore();
            }

            // Circle fill
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(cx, chainY, radius, 0, 2 * Math.PI);
            ctx.fill();

            // Stroke
            ctx.strokeStyle = i === numLinks - 1 ? COL.gold : COL.textMuted;
            ctx.lineWidth = i === numLinks - 1 ? 2.5 : 1;
            ctx.beginPath();
            ctx.arc(cx, chainY, radius, 0, 2 * Math.PI);
            ctx.stroke();

            // Double circle for final link
            if (i === numLinks - 1) {
                ring(ctx, cx, chainY, radius + 4, COL.gold, 1.5);
                label(ctx, `k_meas = 4/G* ${SYM.approx} ${K_CRIT.toFixed(3)}`,
                      cx, chainY - radius - 10, COL.gold, FONT_VALUE, 'center');
            }
        }

        // ── Category labels ──────────────────────────────────────────
        const categories = ['Structural', 'Algebraic', 'Self-referential', 'Discriminant'];
        const catY = chainY + radius + 22;
        for (let i = 0; i < categories.length; i++) {
            const cx = pad.l + (i / (categories.length - 1)) * pw;
            label(ctx, categories[i], cx, catY, COL.textMuted, FONT_LABEL, 'center');
        }

        // ── Mini discriminant bar chart ──────────────────────────────
        const barBaseY = h - pad.b + 20;
        const barMaxH = 25;
        const barW = Math.max(2, (pw / numLinks) - 2);

        for (let i = 0; i < numLinks; i++) {
            const cx = pad.l + i * spacing;
            const delta = deltas[i];
            const normD = clamp(delta / (deltaMax || 1), 0, 1);
            const bh = normD * barMaxH;

            // Color gradient matching the circles
            let barCol;
            if (delta > 0) {
                const t = normD;
                if (t > 0.3) barCol = COL.positive;
                else if (t > 0.05) barCol = COL.warning;
                else barCol = COL.negative;
            } else {
                barCol = COL.textMuted;
            }

            ctx.fillStyle = barCol;
            ctx.fillRect(cx - barW / 2, barBaseY - bh, barW, bh);
        }

        // Bar chart label
        label(ctx, `${SYM.Delta}_k`, pad.l - 20, barBaseY - barMaxH / 2,
              COL.textSec, FONT_LABEL, 'right');
    }

    // ══════════════════════════════════════════════════════════════════
    // Panel 5: Observer Boundary
    // ══════════════════════════════════════════════════════════════════

    _drawObserverBoundary(ctx, w, h) {
        const gridSize = 16;
        const sidebarW = w * 0.2;
        const gridArea = w - sidebarW;
        const cellSize = Math.min(
            (gridArea - 20) / gridSize,
            (h - 50) / gridSize,
        );
        const gridOffX = (gridArea - cellSize * gridSize) / 2;
        const gridOffY = 30;

        // ── Title ────────────────────────────────────────────────────
        label(ctx, 'Ring 0 Clock: The tick IS the computation',
              gridArea / 2, 12, COL.primary, FONT_TITLE, 'center');

        // ── Grid ─────────────────────────────────────────────────────
        const observerI = 8;
        const observerJ = 8;
        const tick = this._boundaryTick;

        for (let i = 0; i < gridSize; i++) {
            for (let j = 0; j < gridSize; j++) {
                const px = gridOffX + j * cellSize;
                const py = gridOffY + i * cellSize;

                // Chebyshev distance from observer
                const dist = Math.max(Math.abs(i - observerI), Math.abs(j - observerJ));
                const inCausal = dist <= tick;
                const onBoundary = dist === tick && tick > 0;

                // Cell color
                if (i === observerI && j === observerJ) {
                    ctx.fillStyle = COL.gold;
                } else if (inCausal) {
                    const state = this._grid[i][j];
                    if (state === 1)       ctx.fillStyle = COL.accent;    // +1 blue
                    else if (state === -1)  ctx.fillStyle = COL.negative; // -1 red
                    else                    ctx.fillStyle = '#1e293b';    // 0 void (shouldn't happen in causal past)
                } else {
                    ctx.fillStyle = COL.bgDeep;
                }

                ctx.fillRect(px, py, cellSize - 1, cellSize - 1);

                // Boundary glow
                if (onBoundary) {
                    ctx.strokeStyle = COL.primary;
                    ctx.lineWidth = 1.5;
                    ctx.strokeRect(px, py, cellSize - 1, cellSize - 1);
                }
            }
        }

        // ── ReLU sidebar ─────────────────────────────────────────────
        const sbX = gridArea + 5;
        const sbW = sidebarW - 15;
        const sbTop = gridOffY;
        const sbH = cellSize * gridSize;

        label(ctx, 'ReLU Filter', sbX + sbW / 2, sbTop - 8, COL.textSec, FONT_LABEL, 'center');

        // Background
        ctx.fillStyle = COL.bgSurface;
        ctx.fillRect(sbX, sbTop, sbW, sbH);

        // Random incoming flux (varies per frame slowly)
        const fluxValue = 0.3 + 0.5 * Math.sin(this._frameCount * 0.02) *
                          Math.sin(this._frameCount * 0.007 + 1);
        const normFlux = clamp(fluxValue, 0, 1);

        // Threshold line at K_B / max_scale
        const threshNorm = K_B; // K_B ≈ 0.511 works well as normalized threshold
        const threshY = sbTop + sbH * (1 - threshNorm);

        // Flux bar
        const barTop = sbTop + sbH * (1 - normFlux);
        const passesThreshold = normFlux > threshNorm;

        if (passesThreshold) {
            // Show the part above threshold in color, below in dark
            const aboveH = sbTop + sbH - threshY;
            ctx.fillStyle = 'rgba(0, 229, 255, 0.3)';
            ctx.fillRect(sbX + 2, threshY, sbW - 4, aboveH);

            ctx.fillStyle = COL.primary;
            ctx.fillRect(sbX + 2, barTop, sbW - 4, sbTop + sbH - barTop);
        } else {
            // Below threshold — zeroed
            ctx.fillStyle = 'rgba(107, 114, 128, 0.2)';
            ctx.fillRect(sbX + 2, barTop, sbW - 4, sbTop + sbH - barTop);
        }

        // Threshold line
        ctx.strokeStyle = COL.warning;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.moveTo(sbX, threshY);
        ctx.lineTo(sbX + sbW, threshY);
        ctx.stroke();

        label(ctx, `K_B = ${K_B}`, sbX + sbW / 2, threshY - 8, COL.warning, FONT_VALUE, 'center');
        label(ctx, 'max(0, |J|\u2212K_B)', sbX + sbW / 2, sbTop + sbH + 12,
              COL.textMuted, FONT_VALUE, 'center');

        // ── Bottom: tick counter + phase ─────────────────────────────
        const phaseIdx = tick % TICK_PHASES.length;
        const phase = TICK_PHASES[phaseIdx];
        const bottomY = gridOffY + cellSize * gridSize + 8;

        label(ctx, `Tick: ${tick}`, gridOffX, bottomY + 12, COL.textPri, FONT_VALUE);
        label(ctx, phase, gridOffX + 60, bottomY + 12, COL.glow, FONT_VALUE);
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
