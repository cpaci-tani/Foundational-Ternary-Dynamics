/**
 * Walkthrough step content — HTML strings for the 6-step guided tour.
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-2). Pure data.
 */

export function getWalkthroughSteps() {
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
