/**
 * Meta Pedagogy — Interactive exploration of the 3³ Existential Unit.
 *
 * UX philosophy: the panel DRIVES the visualization.
 * Each section is a collapsible card with interactive buttons that
 * highlight the corresponding geometry in the 3D view.
 */

import { N_BASE } from './constants.js';

function selfConsistencyValue(n) {
    const lhs = n + N_BASE + (n + N_BASE) + (n * n + N_BASE);
    return { lhs, rhs: n * n * n, match: lhs === n * n * n };
}

// ── Main builder ────────────────────────────────────────────────────
export function buildMetaInfoPanel(container, metaUnit) {
    container.innerHTML = '';
    const root = document.createElement('div');
    root.className = 'meta-panel-root';
    root.innerHTML = `
    <!-- Hero -->
    <div class="meta-hero">
        <div class="meta-hero-title">The Existential Unit</div>
        <div class="meta-hero-number"><span>3</span><sup class="meta-sup">3</sup> = <span>27</span></div>
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
            <div class="meta-section-hint">Click a shell to isolate it in the 3D view</div>

            <div class="meta-shell-row active" data-shell="all">
                <div class="meta-shell-dot bg-all-gradient"></div>
                <div class="meta-shell-name">All Shells</div>
                <div class="meta-shell-count">27</div>
                <div class="meta-shell-desc">= N<sub>c</sub>&sup3;</div>
            </div>
            <div class="meta-shell-row" data-shell="center">
                <div class="meta-shell-dot bg-center"></div>
                <div class="meta-shell-name">Center</div>
                <div class="meta-shell-count color-center">1</div>
                <div class="meta-shell-desc">CM point <i>i</i></div>
            </div>
            <div class="meta-shell-row" data-shell="octahedron">
                <div class="meta-shell-dot bg-oct"></div>
                <div class="meta-shell-name">Octahedron</div>
                <div class="meta-shell-count color-oct">6</div>
                <div class="meta-shell-desc">SC &middot; d=1</div>
            </div>
            <div class="meta-shell-row" data-shell="cuboctahedron">
                <div class="meta-shell-dot bg-cuboct"></div>
                <div class="meta-shell-name">Cuboctahedron</div>
                <div class="meta-shell-count color-cuboct">12</div>
                <div class="meta-shell-desc">FCC &middot; d=&radic;2</div>
            </div>
            <div class="meta-shell-row" data-shell="cube">
                <div class="meta-shell-dot bg-cube"></div>
                <div class="meta-shell-name">Cube (2 tetra)</div>
                <div class="meta-shell-count color-cube">8</div>
                <div class="meta-shell-desc">BCC &middot; d=&radic;3</div>
            </div>

            <div class="meta-section-hint mt">
                <div class="meta-section-label">Wireframes</div>
                <button class="meta-btn" data-action="tetra-plus"><span class="swatch bg-tetra-plus"></span>Tetra T+</button>
                <button class="meta-btn" data-action="tetra-minus"><span class="swatch bg-tetra-minus"></span>Tetra T&minus;</button>
                <button class="meta-btn" data-action="connections"><span class="swatch bg-links"></span>Links</button>
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
                <span class="meta-section-hint">N<sub>c</sub></span>
                <input type="range" min="1" max="8" value="3" step="1" id="meta-nc-input">
                <div class="meta-slider-val" id="meta-nc-val">3</div>
            </div>
            <div class="meta-eq" id="meta-nc-result"></div>

            <div class="meta-section-hint centered mt">
                Factors as (N<sub>c</sub> &minus; 3)(N<sub>c</sub>&sup2; + 2N<sub>c</sub> + 4) = 0<br>
                Quadratic has no real roots &rarr; <span class="color-center">N<sub>c</sub> = 3 is unique</span>
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
            <div class="meta-section-hint">Toggle coloring modes to see how 27 decomposes</div>

            <button class="meta-btn full-width" data-action="bcc-fcc" title="Coord-sum parity coloring. This is the even/odd coordinate-sum partition, NOT the BCC/FCC sublattice distinction (the canonical shell&rarr;sublattice map is center/SC/FCC/BCC per Moore Layer Theorem &sect;4, shown in the site inspector). Audit P0-17 fix.">
                <span class="swatch bg-parity-even"></span>/<span class="swatch bg-parity-odd"></span>
                Coord-sum parity &mdash; 13 + 14
            </button>
            <div class="meta-section-hint centered">
                = N<sub>eff</sub> + 2&middot;b<sub>3</sub>
            </div>

            <button class="meta-btn full-width" data-action="gerade" title="Inversion fundamental domain: one representative per antipodal site-pair (orbit_rep) vs its antipode. NOT gerade/ungerade irrep parity — that labels irreps, not sites (Moore Layer Theorem §3/§8). Audit P1-7 fix.">
                <span class="swatch bg-gerade-even"></span>/<span class="swatch bg-gerade-odd"></span>
                Orbit rep / Antipode &mdash; 13 + 13
            </button>
            <div class="meta-section-hint centered">
                = N<sub>eff</sub> + N<sub>eff</sub> (inversion fundamental domain of the 26 Moore neighbors + center)
            </div>

            <button class="meta-btn full-width" data-action="reset-colors">
                Reset Colors
            </button>

            <div class="meta-section-hint mt">
                <div class="meta-section-label">Symmetry Elements</div>
                <button class="meta-btn" data-action="axes"><span class="swatch bg-axes"></span>Rotation Axes</button>
                <button class="meta-btn" data-action="mirrors"><span class="swatch bg-mirrors"></span>Mirror Planes</button>
            </div>

            <div class="meta-stat-grid mt">
                <div class="meta-stat" title="Order of the octahedral point group O_h (48 operations) — the stabilizer of the center site.">
                    <div class="meta-stat-val color-center">48</div><div class="meta-stat-label">|O<sub>h</sub>|</div></div>
                <div class="meta-stat" title="Order of the full symmetry group of the 3³ periodic torus, (Z/3Z)³ ⋊ O_h = 27 × 48 = 1296 = 6⁴ = |Aut(Eᵢ)|²·N_c⁴. [THEOREM] — DERIV_EXISTENTIAL_UNIT.md §8 (Theorem 8.1).">
                    <div class="meta-stat-val color-oct">1296</div><div class="meta-stat-label">(&#8484;/3)<sup>3</sup>&#8906;O<sub>h</sub> = 6<sup>4</sup></div></div>
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
                <div class="meta-stat"><div class="meta-stat-val color-oct">3</div><div class="meta-stat-label">N<sub>c</sub> (colors)</div></div>
                <div class="meta-stat"><div class="meta-stat-val color-cuboct">4</div><div class="meta-stat-label">N<sub>base</sub> (|Aut|)</div></div>
                <div class="meta-stat"><div class="meta-stat-val color-cube">7</div><div class="meta-stat-label">b<sub>3</sub> (Betti)</div></div>
                <div class="meta-stat"><div class="meta-stat-val color-dof">13</div><div class="meta-stat-label">N<sub>eff</sub> (DOF)</div></div>
            </div>

            <div class="meta-section-hint mt">Where they appear in O<sub>h</sub>:</div>
            <table class="meta-table">
                <tr><td>Scalar reps (A<sub>1g</sub>)</td><td class="meta-table-val color-cuboct">4</td><td class="meta-table-desc">= N<sub>base</sub></td></tr>
                <tr><td>Vector reps (T<sub>1u</sub>)</td><td class="meta-table-val color-oct">3</td><td class="meta-table-desc">= N<sub>c</sub></td></tr>
                <tr><td>Distinct irreps used</td><td class="meta-table-val color-cube">7</td><td class="meta-table-desc">= b<sub>3</sub></td></tr>
                <tr><td>Triplet dimensions</td><td class="meta-table-val color-text-primary">18</td><td class="meta-table-desc">= stencil</td></tr>
                <tr><td>Cuboct stabilizer</td><td class="meta-table-val color-cuboct">4</td><td class="meta-table-desc">= |Aut(E<sub>i</sub>)|</td></tr>
            </table>

            <div class="meta-section-hint mt">Vieta coefficients of P(x)=(x&minus;3)(x&minus;4)(x&minus;7)(x&minus;13):</div>
            <div class="meta-flex-row">
                <div class="meta-stat"><div class="meta-stat-val color-center lg">27</div><div class="meta-stat-label">e<sub>1</sub> = 3&sup3;</div></div>
                <div class="meta-stat"><div class="meta-stat-val color-center lg">243</div><div class="meta-stat-label">e<sub>2</sub> = 3<sup>5</sup></div></div>
                <div class="meta-stat"><div class="meta-stat-val lg">1092</div><div class="meta-stat-label">e<sub>4</sub> = product</div></div>
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
                    metaUnit.toggleInversionDomain(btn.classList.contains('active'));
                    _syncToolbarButton('meta-toggle-gerade', btn.classList.contains('active'));
                    const bfBtn = root.querySelector('[data-action="bcc-fcc"]');
                    if (bfBtn && btn.classList.contains('active')) { bfBtn.classList.remove('active'); }
                    break;
                case 'reset-colors':
                    metaUnit.toggleBCCFCC(false);
                    metaUnit.toggleInversionDomain(false);
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
                    <div class="meta-eq-main is-success">${n} + ${nBase} + ${b3} + ${nEff} = ${rhs}</div>
                    <div class="meta-eq-sub is-success">&#10003; ${lhs} = ${rhs}</div>
                `;
                resultBox.classList.add('is-success');
                resultBox.classList.remove('is-error');
            } else {
                resultBox.innerHTML = `
                    <div class="meta-eq-main is-error">${n} + ${nBase} + ${b3} + ${nEff} = ${lhs}</div>
                    <div class="meta-eq-sub is-error">&#10007; ${lhs} &ne; ${rhs} = ${n}&sup3;</div>
                `;
                resultBox.classList.add('is-error');
                resultBox.classList.remove('is-success');
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
        <div class="meta-inspect-header">
            <div class="meta-inspect-shell-dot" style="background:${c};"></div>
            <div class="meta-inspect-shell-name" style="color:${c};">${siteInfo.shell.charAt(0).toUpperCase() + siteInfo.shell.slice(1)}</div>
        </div>
        <div class="meta-stat-grid cols-3">
            <div class="meta-stat"><div class="meta-stat-val sm">(${siteInfo.position.map(v => v.toFixed(0)).join(',')})</div><div class="meta-stat-label">position</div></div>
            <div class="meta-stat"><div class="meta-stat-val sm">${siteInfo.distance}</div><div class="meta-stat-label">distance</div></div>
            <div class="meta-stat"><div class="meta-stat-val sm">${siteInfo.sublattice}</div><div class="meta-stat-label">sublattice</div></div>
        </div>
        <div class="meta-stat-grid mt">
            <div class="meta-stat"><div class="meta-stat-val xs">${siteInfo.stabilizer}</div><div class="meta-stat-label">stabilizer (|Stab|=${siteInfo.stabOrder || '?'})</div></div>
            <div class="meta-stat"><div class="meta-stat-val xs">${siteInfo.irrep}</div><div class="meta-stat-label">irrep sector</div></div>
        </div>
    `;
}
