/**
 * Scale 0 visualization layers.
 *
 * One table drives the Visualization panel: its groups, its rows, the group
 * counters, the Rendering rows and the tests. Layers are grouped by the
 * quantity they draw, not by how they draw it, so two views of the same
 * quantity sit together.
 *
 * Per layer:
 *   id        button id; the click wiring in ui/bindings.js keys on it
 *   label     the quantity, named for what is computed
 *   swatch    suffix of its `.field-swatch-*` colour class
 *   equation  TeX, shown first in the tooltip (KaTeX, display mode)
 *   detail    what is drawn and what the layer's own controls do
 *   search    extra words the filter matches, including earlier names
 *   render    'scalar' or 'vector' when a Rendering row applies to it
 *   sheet     slice-height slider for layers drawn as a sheet
 *   sub       small switches shown under the layer while it is on
 *
 * TeX is HTML-escaped before KaTeX sees it, so equations use \lt and \gt and
 * avoid `&`.
 */

const tex = String.raw;

const sheet = (key, sliderId, value, aria) => Object.freeze({ key, sliderId, value, aria });

export const LAYER_GROUPS = Object.freeze([
    {
        id: 'flux',
        label: 'Flux J',
        layers: [
            {
                id: 'toggle-flux-volume',
                label: 'Flux volume',
                active: true,
                equation: tex`\begin{gathered} \varepsilon_i = \tfrac12|J_i|^2 + \big\langle \tfrac12|J_j|^2 \big\rangle_{j \in \mathrm{Moore}(i)} \\ {} + |s_i|\,E_{\mathrm{rest}} \end{gathered}`,
                detail: 'One point per voxel, sized and coloured by the amplitude √(2ε): blue through cyan and yellow to red as it grows. The threshold hides points below the chosen share of the frame maximum. Large lattices may be drawn from an evenly strided subset.',
                search: 'cloud activation points',
                sub: {
                    aria: 'Flux volume style',
                    items: [
                        { id: 'toggle-flux-organic', label: 'Organic', detail: 'Moves each point by up to half a cell so the grid pattern is less visible.' },
                        { id: 'toggle-flux-glow', label: 'Glow', active: true, detail: 'Additive glow on the points.' },
                    ],
                },
            },
            {
                id: 'toggle-flux-slice',
                label: 'Flux slice',
                equation: tex`|J(x)|`,
                detail: 'Flux magnitude on the three mid-planes of the lattice. Magnitude has no sign; ∇·J shows sources and sinks.',
                search: 'plane section',
                sub: {
                    aria: 'Flux slice planes',
                    items: [
                        { id: 'flux-slice-axis-xy', label: 'xy', active: true, detail: 'The xy mid-plane, at z = L/2.' },
                        { id: 'flux-slice-axis-xz', label: 'xz', active: true, detail: 'The xz mid-plane, at y = L/2.' },
                        { id: 'flux-slice-axis-yz', label: 'yz', active: true, detail: 'The yz mid-plane, at x = L/2.' },
                    ],
                },
            },
            {
                id: 'toggle-native-transport',
                label: 'Native transport',
                swatch: 'native-transport',
                equation: tex`\Delta e_x = -\sum_{y \in N_{18}(x)} I_{x \to y}`,
                detail: 'The energy current of the wave step on the 18 lattice links it moves energy along: 6 face links and 12 face-diagonal links. Nothing is interpolated; the brighter half of a link points at the receiving site. White markers are manifested matter clusters.',
                search: 'links current energy',
                nativeTransport: true,
            },
            {
                id: 'toggle-flux-lines',
                label: 'Streamlines',
                swatch: 'flux-lines',
                equation: tex`\frac{dx}{d\lambda} = J(x)`,
                detail: 'Traced with fourth-order Runge–Kutta through a reduced display copy of J that keeps the strongest site of each block, interpolated trilinearly. Fine structure differs from the full field; Native transport shows what the engine moves.',
                search: 'effective view lines',
            },
            {
                id: 'toggle-psi-squared',
                label: '|J|²',
                swatch: 'psi-squared',
                render: 'scalar',
                equation: tex`\frac{|J|^2}{\max |J|^2}`,
                detail: 'Flux amplitude squared, normalised to the frame maximum.',
                search: 'psi squared born density',
            },
            {
                id: 'toggle-dark-halo',
                label: 'Sub-threshold flux',
                swatch: 'dm-halo',
                equation: tex`0.003 \lt |J| \lt K_{\mathrm{genesis}}`,
                detail: 'Voxels that carry flux but are below the genesis threshold.',
                search: 'dm halo dark matter envelope',
            },
            {
                id: 'toggle-genesis-iso',
                label: 'Genesis',
                swatch: 'genesis',
                equation: tex`|J| \approx K_{\mathrm{genesis}}`,
                detail: 'The shell where the flux reaches the genesis threshold. Manifestation fires inside it, more often the further the flux exceeds the threshold.',
                search: 'frontier isosurface threshold shell',
            },
        ],
    },
    {
        id: 'sources',
        label: 'Sources and matter',
        layers: [
            {
                id: 'toggle-div-field',
                label: '∇·J',
                swatch: 'divj',
                equation: tex`\nabla \cdot J`,
                detail: 'Divergence of the flux, drawn as points. Charge ρ draws the same quantity as a sheet.',
                search: 'divergence div',
            },
            {
                id: 'toggle-charge-density',
                label: 'Charge ρ',
                swatch: 'charge',
                render: 'scalar',
                equation: tex`\rho = \nabla \cdot J`,
                detail: 'Drawn as a signed sheet. The y slider moves the sheet and samples a thin slab at that height.',
                search: 'rho divergence',
                sheet: sheet('chargeDensity', 'sheet-height-charge-density', 0.62, 'Charge density slice height'),
            },
            {
                id: 'toggle-gauss-residual',
                label: 'Gauss residual',
                swatch: 'gauss',
                render: 'scalar',
                equation: tex`r = \nabla \cdot J - s`,
                detail: 'Red where the residual is positive, blue where it is negative.',
                search: 'resid',
            },
            {
                id: 'toggle-state-field',
                label: 'State s',
                swatch: 'state',
                equation: tex`s \in \{-1,\, 0,\, +1\}`,
                detail: 'Manifested voxels as points: s = −1 blue, s = +1 red. Void voxels, s = 0, are not drawn.',
                search: 'ternary manifested matter',
            },
            {
                id: 'toggle-color-charge',
                label: 'Axis label',
                swatch: 'color-charge',
                equation: tex`\arg\max_{a \in \{x,y,z\}} |J_a|`,
                detail: 'Colours each particle red, green or blue by the axis of its largest flux component at genesis, instead of by charge sign.',
                search: 'color charge colour charge',
            },
            {
                id: 'toggle-damping-zones',
                label: 'Damping',
                swatch: 'damping',
                equation: tex`\{x_p\} \cup \{x_p \pm \hat e_i\}`,
                detail: 'A 3 × 3 × 3 box around each particle, enclosing its 7 damped cells: the centre and its 6 face neighbours. At most 100 boxes are drawn.',
                search: 'zones cells',
            },
        ],
    },
    {
        id: 'curl',
        label: 'Curl, E and B',
        layers: [
            {
                id: 'toggle-e-field',
                label: 'Radiative E',
                swatch: 'e-field',
                equation: tex`E = -\partial_t J`,
                detail: 'Drawn as streamlines. This is the time-varying part of the field, so a settled static charge shows almost nothing; the force on a test charge is the EM layer under Forces.',
                search: 'electric field',
            },
            {
                id: 'toggle-b-field',
                label: 'B field',
                swatch: 'b-field',
                equation: tex`B = \nabla \times J`,
                detail: 'Drawn as streamlines.',
                search: 'magnetic field',
            },
            {
                id: 'toggle-force-weak',
                label: '∇×J arrows',
                swatch: 'weak',
                render: 'vector',
                equation: tex`\delta\,(\nabla \times J), \quad \delta \approx 0.957`,
                detail: 'The curl of the flux as arrows, scaled by δ = DUAL_DELTA. Follows the Vector rendering choice.',
                search: 'pseudovector weak curl',
            },
            {
                id: 'toggle-vorticity',
                label: 'Flux curl |∇×J|',
                swatch: 'vorticity',
                render: 'scalar',
                equation: tex`|\nabla \times J|`,
                detail: 'Magnitude of the curl, normalised to the frame. In Surface mode the y slider picks the slab.',
                search: 'fluid vorticity curl',
                sheet: sheet('vorticity', 'sheet-height-vorticity', 0.68, 'Flux curl slice height'),
            },
        ],
    },
    {
        id: 'energy',
        label: 'Energy',
        layers: [
            {
                id: 'toggle-em-energy',
                label: 'Field energy',
                swatch: 'em-energy',
                render: 'scalar',
                equation: tex`u = \tfrac12|E|^2 + \tfrac{c^2}{2}|B|^2`,
                detail: 'With E = −∂J/∂t, B = ∇×J and c = C_SPEED. In Surface mode the y slider picks the slab.',
                search: 'fluid heatmap gas',
                sheet: sheet('emEnergy', 'sheet-height-em-energy', 0.56, 'Field energy slice height'),
            },
            {
                id: 'toggle-e-pressure',
                label: 'Electric energy',
                swatch: 'e-pressure',
                render: 'scalar',
                equation: tex`u_E = \tfrac12|E|^2`,
                detail: 'The electric part of the field energy, with E = −∂J/∂t. In Surface mode the y slider picks the slab.',
                search: 'fluid pressure energy',
                sheet: sheet('ePressure', 'sheet-height-e-pressure', 0.44, 'Electric energy slice height'),
            },
            {
                id: 'toggle-b-pressure',
                label: 'Magnetic energy',
                swatch: 'b-pressure',
                render: 'scalar',
                equation: tex`u_B = \tfrac{c^2}{2}|B|^2`,
                detail: 'The magnetic part of the field energy, with B = ∇×J and c = C_SPEED. In Surface mode the y slider picks the slab.',
                search: 'fluid pressure energy',
                sheet: sheet('bPressure', 'sheet-height-b-pressure', 0.38, 'Magnetic energy slice height'),
            },
            {
                id: 'toggle-poynting',
                label: 'Energy flow S',
                swatch: 'energy',
                equation: tex`S = c^2\,(E \times B)`,
                detail: 'Drawn as arrows, with E = −∂J/∂t, B = ∇×J and c = C_SPEED.',
                search: 'fluid poynting',
            },
            {
                id: 'toggle-lagrangian-density',
                label: '½|E|² − ½(∇·J)²',
                swatch: 'lagrangian',
                render: 'scalar',
                equation: tex`\tfrac12|E|^2 - \tfrac12(\nabla \cdot J)^2`,
                detail: 'The kinetic term minus the gradient term, with E = −∂J/∂t.',
                search: 'lagrangian density balance',
            },
            {
                id: 'toggle-entropy-density',
                label: 'Disorder 4p(1−p)',
                swatch: 'entropy',
                render: 'scalar',
                equation: tex`4p\,(1-p), \quad p = \frac{|J|}{\max |J|}`,
                detail: 'Largest where the flux is half the frame maximum, zero where it is zero or at the maximum.',
                search: 'entropy gini impurity',
            },
        ],
    },
    {
        id: 'forces',
        label: 'Forces',
        layers: [
            {
                id: 'toggle-force-em',
                label: 'EM',
                swatch: 'em',
                render: 'vector',
                equation: tex`F = \frac{\alpha}{4\pi} \sum_p s_p\, \frac{r - r_p}{\left(|r - r_p|^2 + 1\right)^{3/2}}`,
                detail: 'Force on a unit test charge, summed over manifested voxels with the periodic minimum image and one voxel of softening. It has no v × B term.',
                search: 'electromagnetic coulomb electrostatic',
            },
            {
                id: 'toggle-force-gravity',
                label: 'Gravity',
                swatch: 'gravity',
                render: 'vector',
                equation: tex`g = G_N\,\delta_2 |J|, \quad \delta_2 f = \frac{f(x+2) - f(x-2)}{4}`,
                detail: 'With Geometric Gravity selected the field is M c² L δ₂L instead. Sampled throughout the lattice; the tick applies it at manifested sites.',
                search: 'gravitational',
            },
            {
                id: 'toggle-force-strong',
                label: 'Flux-tube force',
                swatch: 'strong',
                render: 'vector',
                equation: tex`\begin{gathered} F(r) \propto r^{-2} \quad (r \lt 3) \\ F(r) \propto r \quad (r \ge 8) \end{gathered}`,
                detail: 'Pairwise between all manifested voxels, inside a Gaussian tube envelope, with a smooth blend between the two ranges for 3 ≤ r < 8 voxels.',
                search: 'strong colour color',
            },
            {
                id: 'toggle-confinement',
                label: 'Pair links',
                swatch: 'confinement',
                equation: tex`1 \lt r \lt \sqrt{120}`,
                detail: 'A line between every pair of particles whose separation r, in voxels, is in this range. Line colour shows the direction of the separation.',
                search: 'confinement string',
            },
        ],
    },
    {
        id: 'clocks',
        label: 'Gravity and clocks',
        layers: [
            {
                id: 'toggle-grav-potential',
                label: 'Φ potential',
                swatch: 'grav-potential',
                render: 'scalar',
                equation: tex`\Phi = -G_N\,|J|`,
                detail: 'With the latency Poisson term on it is Φ = −L² from the 18-point Poisson solve. The y slider picks a thin slab of the lattice.',
                search: 'phi gravity well',
                sheet: sheet('gravPotential', 'sheet-height-grav-potential', 0.5, 'Φ potential slice height'),
            },
            {
                id: 'toggle-latency',
                label: 'Latency L',
                swatch: 'latency',
                render: 'scalar',
                equation: tex`L = \sqrt{\frac{|J|^2}{\max |J|^2}}`,
                detail: 'Shown against a slowly decaying peak, so a scene that goes quiet fades instead of jumping.',
                search: 'flux density',
            },
            {
                id: 'toggle-horizon',
                label: 'Latency peak (L ≥ 0.95)',
                swatch: 'horizon',
                equation: tex`L \ge 0.95`,
                detail: 'Voxels whose latency is within 5% of its recent peak.',
                search: 'horizon',
            },
            {
                id: 'toggle-proper-time',
                label: 'Proper time τ',
                swatch: 'proper-time',
                equation: tex`\tau = \sum_t \sqrt{\max\!\left(1 - \frac{u^2}{c^2} - L^2,\ 0\right)}`,
                detail: 'Accumulated at each manifested voxel while the latency field or the de Broglie clock is on. Values already accumulated are kept when both are switched off.',
                search: 'tau clock',
            },
            {
                id: 'toggle-lapse',
                label: 'Lapse dτ/dt',
                swatch: 'lapse',
                equation: tex`\frac{d\tau}{dt} = \sqrt{\max\!\left(1 - \frac{u^2}{c^2} - L^2,\ 0\right)}`,
                detail: 'The current clock rate at each manifested voxel: green is full rate, red is zero.',
                search: 'clock rate',
            },
            {
                id: 'toggle-db-phase',
                label: 'dB phase φ',
                swatch: 'db-phase',
                equation: tex`d\varphi = \omega_0\, d\tau`,
                detail: 'The de Broglie clock phase, wrapped to [0, 2π) and drawn as a cyclic hue. It advances only while the de Broglie clock is on.',
                search: 'de broglie clock',
            },
        ],
    },
    {
        id: 'dual',
        label: 'Dual substrate',
        layers: [
            {
                id: 'toggle-dual-substrate',
                label: 'Amplitude split',
                swatch: 'dualj',
                equation: tex`J_L = \tfrac{1+\delta}{2}\,J, \qquad J_R = \tfrac{1-\delta}{2}\,J`,
                detail: 'The two parts of the flux, with δ = DUAL_DELTA. Both point along J.',
                search: 'dual j left right',
            },
            {
                id: 'toggle-chirality',
                label: 'Chiral amplitude',
                swatch: 'chirality',
                equation: tex`|J|\,\delta`,
                detail: 'A magnitude, so it is never negative.',
                search: 'chirality handedness',
            },
            {
                id: 'toggle-phase',
                label: 'arg(J_L + i J_R)',
                swatch: 'phase',
                equation: tex`\varphi = \arg\!\left(J_L + i\,J_R\right)`,
                detail: 'With the split above, J_L and J_R are multiples of the same J, so φ is the same constant everywhere.',
                search: 'phase phi complex',
            },
        ],
    },
    {
        id: 'reference',
        label: 'Reference',
        layers: [
            {
                id: 'toggle-sm-reference',
                label: 'Quantum numbers',
                swatch: 'sm-reference',
                equation: '',
                detail: 'A card in the view with the Standard Model quantum numbers of this scenario’s particle: spin, electric charge, chirality, generation and colour representation. Catalogue values, the same on every tick.',
                search: 'standard model sm particle',
                standardModel: true,
            },
        ],
    },
].map((group) => Object.freeze({ ...group, layers: Object.freeze(group.layers.map(Object.freeze)) })));

export const LAYERS = Object.freeze(LAYER_GROUPS.flatMap((group) => group.layers));

const layerIds = (test) => Object.freeze(LAYERS.filter(test).map((layer) => layer.id));

/** Group id → the button ids it holds; drives the counters and the clear buttons. */
export const GROUP_TO_TOGGLES = Object.freeze(Object.fromEntries(
    LAYER_GROUPS.map((group) => [group.id, Object.freeze(group.layers.map((layer) => layer.id))]),
));

/** Layers the Scalar rendering row applies to (surface, heat map, volume). */
export const SCALAR_RENDER_TOGGLES = layerIds((layer) => layer.render === 'scalar');

/** Layers the Vector rendering row applies to (arrows, heat, glyphs). */
export const VECTOR_STYLE_TOGGLES = layerIds((layer) => layer.render === 'vector');

/** Tooltip text: the equation in display math, then the detail. */
export function layerTooltip(layer) {
    return layer.equation ? `\\[${layer.equation}\\] ${layer.detail}` : layer.detail;
}
