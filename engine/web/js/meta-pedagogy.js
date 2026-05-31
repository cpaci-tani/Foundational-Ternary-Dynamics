/**
 * Meta Pedagogy — Interactive exploration of the 3³ Existential Unit.
 *
 * UX philosophy: the panel DRIVES the visualization.
 * Each section is a collapsible card with interactive buttons that
 * highlight the corresponding geometry in the 3D view.
 */

import { N_C, N_BASE, B_3, N_EFF } from './constants.js';

function selfConsistencyValue(n) {
    const lhs = n + N_BASE + (n + N_BASE) + (n * n + N_BASE);
    return { lhs, rhs: n * n * n, match: lhs === n * n * n };
}

// ── Main builder ────────────────────────────────────────────────────
export function buildMetaInfoPanel(container, metaUnit) {
    container.innerHTML = '';
    const root = document.createElement('div');
    root.className = 'meta-panel-root';
    root.innerHTML = `<style>
        .meta-panel-root {
            font-family: var(--font-mono, monospace);
            font-size: var(--fs-lg, 14px);
            color: var(--text-primary);
            padding: var(--sp-sm) var(--sp-md);
            user-select: none;
        }
        .meta-hero {
            text-align: center;
            padding: var(--sp-lg) 0 var(--sp-md);
            border-bottom: 1px solid var(--border);
            margin-bottom: var(--sp-md);
        }
        .meta-hero-title { font-size: var(--fs-md); letter-spacing: 2px; color: var(--text-muted); text-transform: uppercase; }
        .meta-hero-number { font-size: calc(38px * var(--ui-scale, 1)); font-weight: bold; color: var(--text-primary); line-height: 1.1; }
        .meta-hero-number span { color: var(--accent); }
        .meta-hero-sub { font-size: var(--fs-md); color: var(--text-muted); margin-top: 4px; }

        .meta-card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 6px;
            margin-bottom: 6px;
            overflow: hidden;
            transition: border-color 0.2s, background-color 0.2s;
        }
        .meta-card:hover { border-color: var(--border-light); }
        .meta-card-head {
            display: flex; align-items: center; gap: 8px;
            padding: 10px 12px;
            cursor: pointer;
            font-size: var(--fs-lg); font-weight: 600;
            color: var(--text-primary);
        }
        .meta-card-head .dot { width: 8px; height: 8px; border-radius: 50%; flex-shrink: 0; }
        .meta-card-head .arrow { margin-left: auto; font-size: var(--fs-base); color: var(--text-muted); transition: transform 0.2s; }
        .meta-card.open .meta-card-head .arrow { transform: rotate(90deg); }
        .meta-card-body { display: none; padding: 4px 10px 10px; }
        .meta-card.open .meta-card-body { display: block; }

        .meta-shell-row {
            display: flex; align-items: center; gap: 8px;
            padding: 7px 8px; margin: 2px 0;
            border-radius: 4px; cursor: pointer;
            transition: background 0.15s;
        }
        .meta-shell-row:hover { background: rgba(128,128,128,0.1); }
        .meta-shell-row.active { background: rgba(128,128,128,0.12); border: 1px solid var(--accent); }
        .meta-shell-dot { width: 10px; height: 10px; border-radius: 50%; flex-shrink: 0; }
        .meta-shell-desc { font-size: var(--fs-sm); color: var(--text-muted); }
        .meta-shell-name { flex: 1; font-size: var(--fs-lg); }
        .meta-shell-count { font-size: calc(18px * var(--ui-scale, 1)); font-weight: bold; min-width: 24px; text-align: right; }

        .meta-btn {
            display: inline-flex; align-items: center; gap: 6px;
            padding: 6px 12px; margin: 3px;
            border-radius: 4px; border: 1px solid var(--border);
            background: var(--bg-input);
            color: var(--text-secondary); font-size: var(--fs-md); font-family: inherit;
            cursor: pointer; transition: all 0.15s;
        }
        .meta-btn:hover { background: var(--bg-card); border-color: var(--border-light); color: var(--text-primary); }
        .meta-btn.active { background: rgba(128,128,128,0.15); border-color: var(--accent); color: var(--accent); }
        .meta-btn .swatch { width: 6px; height: 6px; border-radius: 2px; }

        .meta-eq {
            text-align: center; padding: var(--sp-md);
            background: var(--bg-input); border-radius: 4px;
            margin: 6px 0;
        }
        .meta-eq-main { font-size: calc(17px * var(--ui-scale, 1)); color: var(--text-primary); font-weight: bold; }
        .meta-eq-sub { font-size: var(--fs-base); color: var(--text-muted); margin-top: 2px; }
        .meta-eq-result { font-size: var(--fs-lg); margin-top: 4px; font-weight: bold; }

        .meta-slider-row {
            display: flex; align-items: center; gap: 8px; margin-top: 6px;
        }
        .meta-slider-row input[type=range] { flex: 1; accent-color: var(--accent); }
        .meta-slider-val {
            min-width: 30px; text-align: center;
            font-size: calc(22px * var(--ui-scale, 1)); font-weight: bold; color: var(--accent);
        }

        .meta-stat-grid {
            display: grid; grid-template-columns: 1fr 1fr; gap: 4px;
            margin: 6px 0;
        }
        .meta-stat {
            background: var(--bg-input); border-radius: 4px;
            padding: 8px 10px; text-align: center;
        }
        .meta-stat-val { font-size: calc(22px * var(--ui-scale, 1)); font-weight: bold; color: var(--text-primary); }
        .meta-stat-label { font-size: var(--fs-base); color: var(--text-muted); margin-top: 2px; }

        .meta-inspect {
            border-top: 1px solid var(--border);
            padding: 8px 0 4px;
            margin-top: 4px;
        }
        .meta-inspect-empty { color: var(--text-muted); font-size: var(--fs-base); font-style: italic; text-align: center; padding: 12px; }
    </style>

    <!-- Hero -->
    <div class="meta-hero">
        <div class="meta-hero-title">The Existential Unit</div>
        <div class="meta-hero-number"><span>3</span><sup style="font-size:18px;">3</sup> = <span>27</span></div>
        <div class="meta-hero-sub">minimal complete lattice</div>
    </div>

    <!-- 1. Explore Shells -->
    <div class="meta-card open" data-card="shells">
        <div class="meta-card-head">
            <div class="dot" style="background:#FFD700;"></div>
            Explore Shells
            <span class="arrow">&#9654;</span>
        </div>
        <div class="meta-card-body">
            <div style="font-size:10px;color:#6b7280;margin-bottom:6px;">Click a shell to isolate it in the 3D view</div>

            <div class="meta-shell-row active" data-shell="all">
                <div class="meta-shell-dot" style="background:linear-gradient(135deg,#FFD700,#00CED1,#FF00FF,#7FFF00);"></div>
                <div class="meta-shell-name">All Shells</div>
                <div class="meta-shell-count">27</div>
                <div class="meta-shell-desc">= N<sub>c</sub>&sup3;</div>
            </div>
            <div class="meta-shell-row" data-shell="center">
                <div class="meta-shell-dot" style="background:#FFD700;"></div>
                <div class="meta-shell-name">Center</div>
                <div class="meta-shell-count" style="color:#FFD700;">1</div>
                <div class="meta-shell-desc">CM point <i>i</i></div>
            </div>
            <div class="meta-shell-row" data-shell="octahedron">
                <div class="meta-shell-dot" style="background:#00CED1;"></div>
                <div class="meta-shell-name">Octahedron</div>
                <div class="meta-shell-count" style="color:#00CED1;">6</div>
                <div class="meta-shell-desc">SC &middot; d=1</div>
            </div>
            <div class="meta-shell-row" data-shell="cuboctahedron">
                <div class="meta-shell-dot" style="background:#FF00FF;"></div>
                <div class="meta-shell-name">Cuboctahedron</div>
                <div class="meta-shell-count" style="color:#FF00FF;">12</div>
                <div class="meta-shell-desc">FCC &middot; d=&radic;2</div>
            </div>
            <div class="meta-shell-row" data-shell="cube">
                <div class="meta-shell-dot" style="background:#7FFF00;"></div>
                <div class="meta-shell-name">Cube (2 tetra)</div>
                <div class="meta-shell-count" style="color:#7FFF00;">8</div>
                <div class="meta-shell-desc">BCC &middot; d=&radic;3</div>
            </div>

            <div style="margin-top:8px;">
                <div style="font-size:10px;color:#6b7280;margin-bottom:4px;">Wireframes</div>
                <button class="meta-btn" data-action="tetra-plus"><span class="swatch" style="background:#00FFAA;"></span>Tetra T+</button>
                <button class="meta-btn" data-action="tetra-minus"><span class="swatch" style="background:#FF5555;"></span>Tetra T&minus;</button>
                <button class="meta-btn" data-action="connections"><span class="swatch" style="background:#888;"></span>Links</button>
            </div>
        </div>
    </div>

    <!-- 2. Self-Consistency -->
    <div class="meta-card" data-card="consistency">
        <div class="meta-card-head">
            <div class="dot" style="background:#00CED1;"></div>
            Why N<sub>c</sub> = 3?
            <span class="arrow">&#9654;</span>
        </div>
        <div class="meta-card-body">
            <div class="meta-eq">
                <div class="meta-eq-main">N<sub>c</sub> + N<sub>base</sub> + b<sub>3</sub> + N<sub>eff</sub> = N<sub>c</sub>&sup3;</div>
                <div class="meta-eq-sub">The framework integers must fill the lattice</div>
            </div>

            <div class="meta-slider-row">
                <span style="font-size:10px;color:#6b7280;">N<sub>c</sub></span>
                <input type="range" min="1" max="8" value="3" step="1" id="meta-nc-input">
                <div class="meta-slider-val" id="meta-nc-val">3</div>
            </div>
            <div class="meta-eq" id="meta-nc-result" style="margin-top:6px;"></div>

            <div style="font-size:10px;color:#6b7280;margin-top:8px;text-align:center;">
                Factors as (N<sub>c</sub> &minus; 3)(N<sub>c</sub>&sup2; + 2N<sub>c</sub> + 4) = 0<br>
                Quadratic has no real roots &rarr; <span style="color:#FFD700;">N<sub>c</sub> = 3 is unique</span>
            </div>
        </div>
    </div>

    <!-- 3. Parity Modes -->
    <div class="meta-card" data-card="parity">
        <div class="meta-card-head">
            <div class="dot" style="background:#FF00FF;"></div>
            Parity &amp; Symmetry
            <span class="arrow">&#9654;</span>
        </div>
        <div class="meta-card-body">
            <div style="font-size:10px;color:#6b7280;margin-bottom:6px;">Toggle coloring modes to see how 27 decomposes</div>

            <button class="meta-btn" data-action="bcc-fcc" style="width:100%;justify-content:center;" title="Coord-sum parity coloring. This is the even/odd coordinate-sum partition, NOT the BCC/FCC sublattice distinction (the canonical shell&rarr;sublattice map is center/SC/FCC/BCC per Moore Layer Theorem &sect;4, shown in the site inspector). Audit P0-17 fix.">
                <span class="swatch" style="background:#4488FF;"></span>/<span class="swatch" style="background:#FF4444;"></span>
                Coord-sum parity &mdash; 13 + 14
            </button>
            <div style="font-size:10px;color:#6b7280;text-align:center;margin:2px 0 6px;">
                = N<sub>eff</sub> + 2&middot;b<sub>3</sub>
            </div>

            <button class="meta-btn" data-action="gerade" style="width:100%;justify-content:center;">
                <span class="swatch" style="background:#44CC44;"></span>/<span class="swatch" style="background:#FF8800;"></span>
                Gerade / Ungerade &mdash; 13 + 13
            </button>
            <div style="font-size:10px;color:#6b7280;text-align:center;margin:2px 0 6px;">
                = N<sub>eff</sub> + N<sub>eff</sub> (Moore neighborhood under inversion)
            </div>

            <button class="meta-btn" data-action="reset-colors" style="width:100%;justify-content:center;">
                Reset Colors
            </button>

            <div style="margin-top:10px;">
                <div style="font-size:10px;color:#6b7280;margin-bottom:4px;">Symmetry Elements</div>
                <button class="meta-btn" data-action="axes"><span class="swatch" style="background:#FFFF00;"></span>Rotation Axes</button>
                <button class="meta-btn" data-action="mirrors"><span class="swatch" style="background:#fff;opacity:0.5;"></span>Mirror Planes</button>
            </div>

            <div class="meta-stat-grid" style="margin-top:8px;">
                <div class="meta-stat"><div class="meta-stat-val" style="color:#FFD700;">48</div><div class="meta-stat-label">|O<sub>h</sub>|</div></div>
                <div class="meta-stat"><div class="meta-stat-val" style="color:#00CED1;">1296</div><div class="meta-stat-label">Full group = 6<sup>4</sup></div></div>
            </div>
        </div>
    </div>

    <!-- 4. Framework Integers -->
    <div class="meta-card" data-card="framework">
        <div class="meta-card-head">
            <div class="dot" style="background:#7FFF00;"></div>
            Framework Integers
            <span class="arrow">&#9654;</span>
        </div>
        <div class="meta-card-body">
            <div class="meta-stat-grid">
                <div class="meta-stat"><div class="meta-stat-val" style="color:#00CED1;">3</div><div class="meta-stat-label">N<sub>c</sub> (colors)</div></div>
                <div class="meta-stat"><div class="meta-stat-val" style="color:#FF00FF;">4</div><div class="meta-stat-label">N<sub>base</sub> (|Aut|)</div></div>
                <div class="meta-stat"><div class="meta-stat-val" style="color:#7FFF00;">7</div><div class="meta-stat-label">b<sub>3</sub> (Betti)</div></div>
                <div class="meta-stat"><div class="meta-stat-val" style="color:#FF6B6B;">13</div><div class="meta-stat-label">N<sub>eff</sub> (DOF)</div></div>
            </div>

            <div style="font-size:10px;color:#6b7280;margin-top:6px;">Where they appear in O<sub>h</sub>:</div>
            <table style="width:100%;font-size:10px;border-collapse:collapse;margin-top:4px;">
                <tr><td style="padding:2px 0;">Scalar reps (A<sub>1g</sub>)</td><td style="text-align:right;color:#FF00FF;font-weight:bold;">4</td><td style="color:#4b5563;padding-left:6px;">= N<sub>base</sub></td></tr>
                <tr><td style="padding:2px 0;">Vector reps (T<sub>1u</sub>)</td><td style="text-align:right;color:#00CED1;font-weight:bold;">3</td><td style="color:#4b5563;padding-left:6px;">= N<sub>c</sub></td></tr>
                <tr><td style="padding:2px 0;">Distinct irreps used</td><td style="text-align:right;color:#7FFF00;font-weight:bold;">7</td><td style="color:#4b5563;padding-left:6px;">= b<sub>3</sub></td></tr>
                <tr><td style="padding:2px 0;">Triplet dimensions</td><td style="text-align:right;font-weight:bold;">18</td><td style="color:#4b5563;padding-left:6px;">= stencil</td></tr>
                <tr><td style="padding:2px 0;">Cuboct stabilizer</td><td style="text-align:right;color:#FF00FF;font-weight:bold;">4</td><td style="color:#4b5563;padding-left:6px;">= |Aut(E<sub>i</sub>)|</td></tr>
            </table>

            <div style="margin-top:8px;font-size:10px;color:#6b7280;">Vieta coefficients of P(x)=(x&minus;3)(x&minus;4)(x&minus;7)(x&minus;13):</div>
            <div style="display:flex;gap:6px;margin-top:4px;">
                <div class="meta-stat" style="flex:1;"><div class="meta-stat-val" style="color:#FFD700;font-size:18px;">27</div><div class="meta-stat-label">e<sub>1</sub> = 3&sup3;</div></div>
                <div class="meta-stat" style="flex:1;"><div class="meta-stat-val" style="color:#FFD700;font-size:18px;">243</div><div class="meta-stat-label">e<sub>2</sub> = 3<sup>5</sup></div></div>
                <div class="meta-stat" style="flex:1;"><div class="meta-stat-val" style="font-size:18px;">1092</div><div class="meta-stat-label">e<sub>4</sub> = product</div></div>
            </div>
        </div>
    </div>

    <!-- 5. Inspect (populated on click) -->
    <div class="meta-inspect" id="meta-inspect-area">
        <div class="meta-inspect-empty">Click a site in the 3D view to inspect it</div>
    </div>
    `;

    container.appendChild(root);

    // ── Wire interactions ────────────────────────────────────────────

    // Collapsible cards
    root.querySelectorAll('.meta-card-head').forEach(head => {
        head.addEventListener('click', () => {
            head.parentElement.classList.toggle('open');
        });
    });

    // Shell isolation
    root.querySelectorAll('.meta-shell-row').forEach(row => {
        row.addEventListener('click', () => {
            if (!metaUnit) return;
            const shell = row.dataset.shell;
            // Deactivate all rows, activate this one
            root.querySelectorAll('.meta-shell-row').forEach(r => r.classList.remove('active'));
            row.classList.add('active');

            if (shell === 'all') {
                metaUnit.toggleCenter(true);
                metaUnit.toggleOctahedron(true);
                metaUnit.toggleCuboctahedron(true);
                metaUnit.toggleCube(true);
            } else {
                metaUnit.toggleCenter(shell === 'center');
                metaUnit.toggleOctahedron(shell === 'octahedron');
                metaUnit.toggleCuboctahedron(shell === 'cuboctahedron');
                metaUnit.toggleCube(shell === 'cube');
            }
            // Sync toolbar buttons
            _syncToolbarButton('meta-toggle-center', shell === 'all' || shell === 'center');
            _syncToolbarButton('meta-toggle-oct', shell === 'all' || shell === 'octahedron');
            _syncToolbarButton('meta-toggle-cuboct', shell === 'all' || shell === 'cuboctahedron');
            _syncToolbarButton('meta-toggle-cube', shell === 'all' || shell === 'cube');
        });
    });

    // Action buttons
    root.querySelectorAll('.meta-btn[data-action]').forEach(btn => {
        btn.addEventListener('click', () => {
            if (!metaUnit) return;
            const action = btn.dataset.action;
            switch (action) {
                case 'tetra-plus':
                    btn.classList.toggle('active');
                    metaUnit.toggleTetraPlus(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-tetra-plus', btn.classList.contains('active'));
                    break;
                case 'tetra-minus':
                    btn.classList.toggle('active');
                    metaUnit.toggleTetraMinus(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-tetra-minus', btn.classList.contains('active'));
                    break;
                case 'connections':
                    btn.classList.toggle('active');
                    metaUnit.toggleConnections(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-connections', btn.classList.contains('active'));
                    break;
                case 'bcc-fcc':
                    btn.classList.toggle('active');
                    metaUnit.toggleBCCFCC(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-bcc-fcc', btn.classList.contains('active'));
                    // Deactivate the other parity button
                    const guBtn = root.querySelector('[data-action="gerade"]');
                    if (guBtn && btn.classList.contains('active')) { guBtn.classList.remove('active'); }
                    break;
                case 'gerade':
                    btn.classList.toggle('active');
                    metaUnit.toggleGeradeUngerade(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-gerade', btn.classList.contains('active'));
                    const bfBtn = root.querySelector('[data-action="bcc-fcc"]');
                    if (bfBtn && btn.classList.contains('active')) { bfBtn.classList.remove('active'); }
                    break;
                case 'reset-colors':
                    metaUnit.toggleBCCFCC(false);
                    metaUnit.toggleGeradeUngerade(false);
                    root.querySelectorAll('[data-action="bcc-fcc"],[data-action="gerade"]').forEach(b => b.classList.remove('active'));
                    _syncToolbarButton('meta-toggle-bcc-fcc', false);
                    _syncToolbarButton('meta-toggle-gerade', false);
                    break;
                case 'axes':
                    btn.classList.toggle('active');
                    metaUnit.toggleRotationAxes(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-axes', btn.classList.contains('active'));
                    break;
                case 'mirrors':
                    btn.classList.toggle('active');
                    metaUnit.toggleMirrorPlanes(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-mirrors', btn.classList.contains('active'));
                    break;
            }
        });
    });

    // N_c slider
    const slider = root.querySelector('#meta-nc-input');
    const sliderVal = root.querySelector('#meta-nc-val');
    const resultBox = root.querySelector('#meta-nc-result');
    if (slider && resultBox) {
        function updateSlider() {
            const n = parseInt(slider.value);
            if (sliderVal) sliderVal.textContent = n;
            const { lhs, rhs, match } = selfConsistencyValue(n);
            const nBase = N_BASE;
            const b3 = n + nBase;
            const nEff = n * n + nBase;
            if (match) {
                resultBox.innerHTML = `
                    <div class="meta-eq-main" style="color:#4ade80;">${n} + ${nBase} + ${b3} + ${nEff} = ${rhs}</div>
                    <div class="meta-eq-sub" style="color:#4ade80;">&#10003; ${lhs} = ${rhs}</div>
                `;
                resultBox.style.borderColor = 'rgba(74,222,128,0.3)';
            } else {
                resultBox.innerHTML = `
                    <div class="meta-eq-main" style="color:#f87171;">${n} + ${nBase} + ${b3} + ${nEff} = ${lhs}</div>
                    <div class="meta-eq-sub" style="color:#f87171;">&#10007; ${lhs} &ne; ${rhs} = ${n}&sup3;</div>
                `;
                resultBox.style.borderColor = 'rgba(248,113,113,0.2)';
            }
        }
        slider.addEventListener('input', updateSlider);
        updateSlider();
    }
}

// ── Sync toolbar toggle buttons with panel state ────────────────────
function _syncToolbarButton(id, active) {
    const el = document.getElementById(id);
    if (!el) return;
    if (active) el.classList.add('active');
    else el.classList.remove('active');
}

// ── Site inspection (called from outside when a site is clicked) ────
export function buildSiteInspectPanel(container, siteInfo) {
    const area = container.querySelector('#meta-inspect-area') || container;
    if (!siteInfo) {
        area.innerHTML = '<div class="meta-inspect-empty">Click a site in the 3D view to inspect it</div>';
        return;
    }

    const colors = {
        'center': '#FFD700', 'octahedron': '#00CED1',
        'cuboctahedron': '#FF00FF', 'cube': '#7FFF00'
    };
    const c = colors[siteInfo.shell] || '#fff';

    area.innerHTML = `
        <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px;">
            <div style="width:12px;height:12px;border-radius:50%;background:${c};"></div>
            <div style="font-size:13px;font-weight:bold;color:${c};">${siteInfo.shell.charAt(0).toUpperCase() + siteInfo.shell.slice(1)}</div>
        </div>
        <div class="meta-stat-grid" style="grid-template-columns:1fr 1fr 1fr;">
            <div class="meta-stat"><div class="meta-stat-val" style="font-size:12px;">(${siteInfo.position.map(v => v.toFixed(0)).join(',')})</div><div class="meta-stat-label">position</div></div>
            <div class="meta-stat"><div class="meta-stat-val" style="font-size:12px;">${siteInfo.distance}</div><div class="meta-stat-label">distance</div></div>
            <div class="meta-stat"><div class="meta-stat-val" style="font-size:12px;">${siteInfo.sublattice}</div><div class="meta-stat-label">sublattice</div></div>
        </div>
        <div class="meta-stat-grid" style="margin-top:2px;">
            <div class="meta-stat"><div class="meta-stat-val" style="font-size:11px;">${siteInfo.stabilizer}</div><div class="meta-stat-label">stabilizer (|Stab|=${siteInfo.stabOrder || '?'})</div></div>
            <div class="meta-stat"><div class="meta-stat-val" style="font-size:11px;">${siteInfo.irrep}</div><div class="meta-stat-label">irrep sector</div></div>
        </div>
    `;
}
