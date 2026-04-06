/**
 * Meta Pedagogy — Info panels and walkthroughs for the Existential Unit.
 * Displays framework integer correspondences, self-consistency equation,
 * representation theory, Vieta structure, and parity decompositions.
 */

// ── Framework Constants ─────────────────────────────────────────────
const N_C = 3, N_BASE = 4, B_3 = 7, N_EFF = 13;
const D_CONSTRAINT = N_C * N_BASE * N_BASE - 1; // 47

// ── Self-consistency slider data ────────────────────────────────────
function selfConsistencyValue(n) {
    // n + 4 + (n+4) + (n²+4) vs n³
    const lhs = n + N_BASE + (n + N_BASE) + (n * n + N_BASE);
    const rhs = n * n * n;
    return { lhs, rhs, match: lhs === rhs };
}

// ── Build the info panel HTML ───────────────────────────────────────
export function buildMetaInfoPanel(container) {
    container.innerHTML = `
    <div class="meta-info" style="font-family:var(--font-mono);font-size:12px;color:#e0e0e0;padding:8px;">

        <div style="text-align:center;margin-bottom:12px;">
            <div style="font-size:16px;font-weight:bold;color:#FFD700;">THE EXISTENTIAL UNIT</div>
            <div style="font-size:22px;font-weight:bold;color:#fff;">3<sup>3</sup> = 27</div>
            <div style="font-size:11px;color:#9ca3af;">Minimal Complete Lattice</div>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Shell Decomposition</div>
            <table style="width:100%;font-size:11px;border-collapse:collapse;">
                <tr><td style="color:#FFD700;">Center</td><td>1</td><td style="color:#808080;">CM point i</td></tr>
                <tr><td style="color:#00CED1;">Octahedron</td><td>6</td><td style="color:#808080;">SC (d=1)</td></tr>
                <tr><td style="color:#FF00FF;">Cuboctahedron</td><td>12</td><td style="color:#808080;">FCC (d=&radic;2)</td></tr>
                <tr><td style="color:#7FFF00;">Cube</td><td>8</td><td style="color:#808080;">BCC (d=&radic;3)</td></tr>
                <tr style="border-top:1px solid #404060;font-weight:bold;">
                    <td>Total</td><td>27</td><td style="color:#FFD700;">= N<sub>c</sub>&sup3;</td>
                </tr>
            </table>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Self-Consistency</div>
            <div style="text-align:center;font-size:13px;color:#fff;">
                N<sub>c</sub> + N<sub>base</sub> + b<sub>3</sub> + N<sub>eff</sub> = N<sub>c</sub>&sup3;
            </div>
            <div style="text-align:center;font-size:13px;color:#00CED1;">
                3 + 4 + 7 + 13 = 27
            </div>
            <div style="text-align:center;font-size:10px;color:#9ca3af;margin-top:2px;">
                (N<sub>c</sub> &minus; 3)(N<sub>c</sub>&sup2; + 2N<sub>c</sub> + 4) = 0
            </div>
            <div id="meta-nc-slider" style="margin-top:6px;">
                <input type="range" min="1" max="8" value="3" step="1"
                    style="width:100%;accent-color:#FFD700;" id="meta-nc-input">
                <div id="meta-nc-result" style="text-align:center;font-size:11px;margin-top:2px;"></div>
            </div>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Representation Theory (O<sub>h</sub>)</div>
            <div style="font-size:10px;color:#9ca3af;margin-bottom:4px;">
                27 = 4&middot;A<sub>1g</sub> + A<sub>2u</sub> + 2&middot;E<sub>g</sub> + T<sub>1g</sub> + T<sub>2g</sub> + 3&middot;T<sub>1u</sub> + T<sub>2u</sub>
            </div>
            <table style="width:100%;font-size:10px;border-collapse:collapse;">
                <tr style="color:#9ca3af;"><th style="text-align:left;">Quantity</th><th>Value</th><th>= </th></tr>
                <tr><td>Scalar (A<sub>1g</sub>) reps</td><td style="color:#FF00FF;font-weight:bold;">4</td><td style="color:#808080;">N<sub>base</sub></td></tr>
                <tr><td>Vector (T<sub>1u</sub>) reps</td><td style="color:#00CED1;font-weight:bold;">3</td><td style="color:#808080;">N<sub>c</sub></td></tr>
                <tr><td>Distinct irreps used</td><td style="color:#7FFF00;font-weight:bold;">7</td><td style="color:#808080;">b<sub>3</sub></td></tr>
                <tr><td>Triplet dimensions</td><td style="font-weight:bold;">18</td><td style="color:#808080;">18-pt stencil</td></tr>
            </table>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Parity Decompositions</div>
            <table style="width:100%;font-size:10px;border-collapse:collapse;">
                <tr><td>Inversion (g/u)</td><td style="color:#4ade80;">13g</td><td>+</td><td style="color:#fb923c;">13u</td><td style="color:#808080;">= N<sub>eff</sub>+N<sub>eff</sub></td></tr>
                <tr><td>Translation (BCC/FCC)</td><td style="color:#4488FF;">13</td><td>+</td><td style="color:#FF4444;">14</td><td style="color:#808080;">= N<sub>eff</sub>+2b<sub>3</sub></td></tr>
            </table>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Vieta Coefficients</div>
            <div style="font-size:10px;">
                P(x) = (x&minus;3)(x&minus;4)(x&minus;7)(x&minus;13)
            </div>
            <table style="width:100%;font-size:10px;border-collapse:collapse;margin-top:4px;">
                <tr><td>e<sub>1</sub> = sum</td><td style="color:#FFD700;font-weight:bold;">27 = 3&sup3;</td></tr>
                <tr><td>e<sub>2</sub> = pairs</td><td style="color:#FFD700;font-weight:bold;">243 = 3<sup>5</sup></td></tr>
                <tr><td>e<sub>4</sub> = product</td><td>1092 = 3&middot;4&middot;7&middot;13</td></tr>
            </table>
            <div style="font-size:10px;color:#9ca3af;margin-top:4px;">
                P(x) mod 27 = (x&minus;1)(x&minus;3)(x&sup2;+4x+13)
            </div>
        </div>

        <div class="meta-section" style="margin-bottom:10px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Symmetry Group</div>
            <div style="font-size:11px;">
                (Z/3Z)&sup3; &rtimes; O<sub>h</sub>
            </div>
            <div style="font-size:11px;color:#00CED1;">
                |G| = 1296 = |Aut|&sup2; &middot; N<sub>c</sub><sup>4</sup> = 6<sup>4</sup>
            </div>
        </div>

        <div class="meta-section" style="margin-bottom:6px;">
            <div style="color:#FFD700;font-weight:bold;margin-bottom:4px;">Stabilizers</div>
            <table style="width:100%;font-size:10px;border-collapse:collapse;">
                <tr style="color:#9ca3af;"><th style="text-align:left;">Shell</th><th>|Stab|</th><th>Group</th></tr>
                <tr><td style="color:#FFD700;">Center</td><td>48</td><td style="color:#808080;">O<sub>h</sub></td></tr>
                <tr><td style="color:#00CED1;">Octahedron</td><td>8</td><td style="color:#808080;">C<sub>4v</sub></td></tr>
                <tr><td style="color:#FF00FF;">Cuboctahedron</td><td style="color:#FFD700;font-weight:bold;">4</td><td style="color:#808080;">C<sub>2v</sub> = |Aut(E<sub>i</sub>)|</td></tr>
                <tr><td style="color:#7FFF00;">Cube</td><td>6</td><td style="color:#808080;">C<sub>3v</sub></td></tr>
            </table>
        </div>

    </div>`;

    // Wire up the N_c slider
    const slider = container.querySelector('#meta-nc-input');
    const result = container.querySelector('#meta-nc-result');
    if (slider && result) {
        function updateSlider() {
            const n = parseInt(slider.value);
            const { lhs, rhs, match } = selfConsistencyValue(n);
            if (match) {
                result.innerHTML = `<span style="color:#4ade80;">N<sub>c</sub>=${n}: ${lhs} = ${rhs} &#10003; UNIQUE SOLUTION</span>`;
            } else {
                result.innerHTML = `<span style="color:#f87171;">N<sub>c</sub>=${n}: ${lhs} &ne; ${rhs}</span>`;
            }
        }
        slider.addEventListener('input', updateSlider);
        updateSlider();
    }
}

// ── Site inspection panel ───────────────────────────────────────────
export function buildSiteInspectPanel(container, siteInfo) {
    if (!siteInfo) {
        container.innerHTML = '<div style="padding:8px;color:#9ca3af;font-size:11px;">Click a site to inspect</div>';
        return;
    }

    const shellColors = {
        'Center': '#FFD700',
        'Octahedron': '#00CED1',
        'Cuboctahedron': '#FF00FF',
        'Cube': '#7FFF00'
    };
    const color = shellColors[siteInfo.shell] || '#ffffff';

    container.innerHTML = `
    <div style="padding:8px;font-family:var(--font-mono);font-size:11px;">
        <div style="color:${color};font-weight:bold;font-size:13px;margin-bottom:6px;">
            ${siteInfo.shell}
        </div>
        <table style="width:100%;font-size:11px;border-collapse:collapse;">
            <tr><td style="color:#9ca3af;">Position</td><td>(${siteInfo.position.map(v => v.toFixed(0)).join(', ')})</td></tr>
            <tr><td style="color:#9ca3af;">Distance</td><td>${siteInfo.distance}</td></tr>
            <tr><td style="color:#9ca3af;">Sublattice</td><td>${siteInfo.sublattice}</td></tr>
            <tr><td style="color:#9ca3af;">Stabilizer</td><td>${siteInfo.stabilizer} (|Stab| = ${siteInfo.stabOrder})</td></tr>
            <tr><td style="color:#9ca3af;">Irrep sector</td><td>${siteInfo.irrep}</td></tr>
            <tr><td style="color:#9ca3af;">Parity (x+y+z)</td><td>${siteInfo.parity}</td></tr>
            <tr><td style="color:#9ca3af;">Inversion</td><td>${siteInfo.inversion}</td></tr>
        </table>
    </div>`;
}
