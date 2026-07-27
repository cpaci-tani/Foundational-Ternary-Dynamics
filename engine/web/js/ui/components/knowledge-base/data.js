import { QUANTUM_SCENARIO_DESCRIPTIONS, S0_SEED_SCENARIO_METADATA } from '../../../config/scenarios.js';
import { getAllMolecules } from '../../../molecules.js';
import { SCALE0_SCENARIOS } from '../../../scales/scale0/scenario-registry.js';

const KNOWLEDGE_BASE_SECTIONS = Object.freeze([
    {
        id: 'foundations',
        title: 'Foundations',
        description: 'Core ontology, lattice objects, and the simulation cycle.',
        entries: [
            {
                id: 'lattice',
                title: 'Lattice',
                shortTitle: 'Lattice',
                summary: 'The discrete cubic space where every simulation site lives.',
                body: [
                    'In the web engine, the lattice is the 3D grid of voxel sites that carries the substrate state. Every update step reads from this discrete structure rather than from a continuous background manifold.',
                    'Scale 0 works most directly at this level. Other scales either sample from the lattice, aggregate it, or present higher-level derived behavior built on top of it.',
                ],
                bullets: [
                    'Domain: discrete 3D cubic grid.',
                    'Each lattice site can host ternary state information and flux-related derived values.',
                    'A single tick only propagates local effects through the allowed neighborhood rules.',
                ],
                notation: ['L', '\\(v \\in L \\subset \\mathbb{Z}^3\\)'],
                tags: ['space', 'scale0', 'ontology'],
            },
            {
                id: 'voxel',
                title: 'Voxel',
                shortTitle: 'Voxel',
                summary: 'A single site of the lattice, indexed by position.',
                body: [
                    'A voxel is one addressable lattice site. In the codebase it is the local unit that stores or samples state during Scale 0 simulation and rendering.',
                    'When the UI inspects a lattice position, it is usually reporting data associated with one voxel or a small neighborhood around it.',
                ],
                bullets: [
                    'Position variable: v.',
                    'Lives on the cubic lattice.',
                    'Can be read, updated, and projected during the tick cycle.',
                ],
                notation: ['v'],
                tags: ['space', 'state'],
            },
            {
                id: 'ternary-state',
                title: 'Ternary State s(v,t)',
                shortTitle: 's(v,t)',
                summary: 'The discrete manifestation field with values -1, 0, or +1.',
                body: [
                    'The state field records which of the three allowed substrate states a voxel occupies at a given tick. This is the manifest side of the two-layer ontology described in the project docs.',
                    'In practical UI terms, Scale 0 visuals and diagnostics often treat this state as the directly inspectable lattice configuration, while flux-derived quantities describe the dispositional field around it.',
                ],
                bullets: [
                    'Domain: {-1, 0, +1}.',
                    'Depends on voxel position and discrete tick.',
                    'Represents actual ternary occupancy rather than a continuous amplitude.',
                ],
                notation: ['s(v,t)'],
                tags: ['state', 'ontology'],
            },
            {
                id: 'flux-field-j',
                title: 'Flux Field J(v,t)',
                shortTitle: 'J(v,t)',
                summary: 'The continuous vector field used to represent dispositional structure and potential flow.',
                body: [
                    'J is the main vector-valued field used throughout the engine and theory references. In the UI, many overlays, field diagnostics, and symbolic explanations ultimately refer back to this quantity.',
                    'You can think of J as the local directional structure of the field: it has both magnitude and direction, so it can be sampled, sliced, diverged, curled, and compared across scales.',
                ],
                bullets: [
                    'Vector-valued field over the lattice.',
                    'Provides the source for derived scalar quantities like |J|.',
                    'Frequently sampled for overlays, diagnostics, and symbolic explanations.',
                ],
                notation: ['J(v,t)'],
                tags: ['field', 'flux', 'symbols'],
            },
            {
                id: 'tick-cycle',
                title: 'Tick Cycle',
                shortTitle: 'Tick',
                summary: 'One discrete update step of the engine.',
                body: [
                    'A tick is the atomic time step in the discrete engine. The current implementation advances through ordered phases, then increments the tick counter.',
                    'The browser UI displays tick and playback speed as direct observables, so “simulation speed” usually means how many ticks are processed for each rendered frame or interval.',
                ],
                bullets: [
                    'Time variable: t.',
                    'Discrete, deterministic update step.',
                    'Displayed in the status bar and used by playback controls.',
                ],
                notation: ['t'],
                tags: ['time', 'runtime'],
            },
            {
                id: 'moore-neighborhood',
                title: 'Moore Neighborhood',
                shortTitle: 'Moore 26',
                summary: 'The 26-neighbor local neighborhood around one lattice site.',
                body: [
                    'FTD uses a 3D Moore neighborhood, meaning a voxel can interact locally with the 26 surrounding sites in its immediate cube shell.',
                    'This matters because causality, update rules, and several structural arguments in the wider project all depend on what can happen inside one local neighborhood per tick.',
                ],
                bullets: [
                    'Includes face, edge, and corner neighbors.',
                    'Defines the local causality envelope.',
                    'Important for the project’s structural decomposition arguments.',
                ],
                notation: ['26-neighbor Moore neighborhood'],
                tags: ['locality', 'geometry'],
            },
        ],
    },
    {
        id: 'symbols',
        title: 'Symbols & Operators',
        description: 'What the mathematical symbols in the engine mean.',
        entries: [
            {
                id: 'nabla',
                title: '∇ (Nabla / Del)',
                shortTitle: '∇',
                summary: 'The upside-down triangle operator used for spatial derivatives.',
                body: [
                    'The upside-down triangle symbol is called nabla or del. It is the operator used to ask how a field changes in space.',
                    'On its own, ∇ is not a complete quantity. Its meaning depends on what it acts on: a scalar field gives a gradient, a vector field with a dot gives divergence, and a vector field with a cross gives curl.',
                ],
                bullets: [
                    'Read aloud as “nabla” or “del.”',
                    'Acts on fields to produce spatial derivatives.',
                    'Common combinations: ∇f, ∇·J, ∇×J, ∇²f.',
                ],
                notation: ['∇'],
                tags: ['operator', 'symbols'],
            },
            {
                id: 'flux-magnitude',
                title: '|J| (Flux Magnitude)',
                shortTitle: '|J|',
                summary: 'The size of the flux vector without regard to direction.',
                body: [
                    '|J| means the magnitude of the flux vector J. It tells you how strong the local flux is, ignoring which direction the vector points.',
                    'The reference glossary also identifies the scalar density ρ with |J| in the lattice-variable section. In UI language, this is often the strength-like scalar derived from the local field sample.',
                ],
                bullets: [
                    'Scalar derived from vector J.',
                    'Useful when you care about field strength rather than direction.',
                    'Often interpreted as a density-like measure in project references.',
                ],
                notation: ['|J|', 'ρ = |J|'],
                tags: ['field', 'magnitude', 'symbols'],
            },
            {
                id: 'gradient-of-flux-magnitude',
                title: '∇|J| (Gradient of Flux Magnitude)',
                shortTitle: '∇|J|',
                summary: 'How the strength of the flux field changes from place to place.',
                body: [
                    'If you were asking about the upside-down triangle applied to |J|, this is the quantity: ∇|J|. It is the gradient of the flux magnitude.',
                    'Intuitively, it points in the direction where flux strength increases fastest and its size tells you how steep that increase is. In the UI this kind of quantity is often used as a force-like cue or as part of an overlay explanation, not as a literal multiplication.',
                ],
                bullets: [
                    'Gradient of a scalar field.',
                    'Direction: where |J| rises fastest.',
                    'Magnitude: how steeply the field strength changes.',
                ],
                notation: ['∇|J|'],
                tags: ['operator', 'field', 'learning'],
            },
            {
                id: 'divergence-of-j',
                title: '∇·J (Divergence of J)',
                shortTitle: '∇·J',
                summary: 'Net outflow or inflow of the flux field at a point.',
                body: [
                    'The divergence of J measures whether flux is spreading outward from a location, collapsing inward, or balancing to zero locally.',
                    'When users read diagnostics or theory notes involving source-like or sink-like behavior, this is usually one of the core operators being referenced.',
                ],
                bullets: [
                    'Positive divergence: local outflow.',
                    'Negative divergence: local inflow.',
                    'Zero divergence: no net local source or sink.',
                ],
                notation: ['∇·J'],
                tags: ['operator', 'field', 'diagnostics'],
            },
            {
                id: 'curl-of-j',
                title: '∇×J (Curl of J)',
                shortTitle: '∇×J',
                summary: 'Local rotational tendency of the flux field.',
                body: [
                    'The curl of J measures how much the field circulates or twists around a point.',
                    'It is the vector-valued rotational operator associated with a vector field. When the learning surface talks about vorticity-like or rotational structure, curl is the relevant mathematical object.',
                ],
                bullets: [
                    'Vector-valued operator.',
                    'Captures local rotation rather than net outflow.',
                    'Different from divergence even though both act on J.',
                ],
                notation: ['∇×J'],
                tags: ['operator', 'field'],
            },
            {
                id: 'laplacian',
                title: '∇²f (Laplacian)',
                shortTitle: '∇²',
                summary: 'A second-derivative operator that measures local curvature of a field.',
                body: [
                    'The Laplacian combines second spatial derivatives. It is often used to express how a field bends relative to its surroundings.',
                    'For learners, a good intuition is that it compares the value at a point with the values around it. Large Laplacian values usually indicate strong local curvature or imbalance.',
                ],
                bullets: [
                    'Second-order differential operator.',
                    'Often appears in diffusion-, smoothing-, or curvature-like expressions.',
                    'Acts on scalar fields in the most familiar form.',
                ],
                notation: ['∇²f'],
                tags: ['operator', 'analysis'],
            },
            {
                id: 'rho-disambiguation',
                title: 'ρ (Rho)',
                shortTitle: 'ρ',
                summary: 'A context-sensitive symbol that can mean density, charge density, or a density matrix.',
                body: [
                    'Rho is one of the most overloaded symbols in physics and in FTD-adjacent writing. In the lattice glossary it appears as a scalar density ρ(v,t) = |J|, but in quantum contexts ρ often denotes a density matrix.',
                    'The safe way to read \\(\\rho\\) is to ask what kind of object the surrounding equation expects: a scalar field, a matrix, or a physical density ratio.',
                ],
                bullets: [
                    'Scalar \\(\\rho\\) often means density or magnitude-like content.',
                    'Matrix \\(\\rho\\) often means a statistical quantum state.',
                    'The surrounding notation should disambiguate the meaning.',
                ],
                notation: ['ρ(v,t)', 'ρ = |J|', 'ρ = |ψ⟩⟨ψ|'],
                tags: ['symbols', 'disambiguation', 'density'],
            },
            {
                id: 'bra-ket',
                title: '⟨·|·⟩ (Inner Product / Bra-Ket Notation)',
                shortTitle: '⟨·|·⟩',
                summary: 'The notation used for overlaps, amplitudes, and inner products in quantum-style expressions.',
                body: [
                    'Bra-ket notation is the compact language of quantum linear algebra. A ket |ψ⟩ represents a state vector, a bra ⟨ψ| is its dual, and ⟨ψ|φ⟩ is their inner product.',
                    'Even if the web engine is not presenting full symbolic derivations live, this notation appears in the project glossary and is important for learners moving between the UI and the theory documents.',
                ],
                bullets: [
                    '⟨ψ|φ⟩ is an overlap or inner product.',
                    '|ψ⟩ names a state vector.',
                    'Useful in wave-function and density-matrix discussions.',
                ],
                notation: ['|ψ⟩', '⟨ψ|', '⟨ψ|φ⟩'],
                tags: ['symbols', 'quantum', 'linear-algebra'],
            },
        ],
    },
    {
        id: 'constants',
        title: 'Constants',
        description: 'Frequently referenced constants and framework integers.',
        entries: [
            {
                id: 'g-star',
                title: 'G* (Lemniscatic Constant)',
                shortTitle: 'G*',
                summary: 'A central constant in the project’s theoretical numerology and coupling discussions.',
                body: [
                    'G* is the lemniscatic constant highlighted throughout the project documents. It appears as one of the central constants in the reference glossary and is tied to several selection and coupling stories in the wider FTD theory corpus.',
                    'In the web UI, you will mostly encounter it as a named theoretical constant rather than as a directly manipulated runtime variable.',
                ],
                bullets: [
                    'Named in the reference glossary.',
                    'Used across theoretical documents as a distinguished constant.',
                    'Important conceptually even when not shown in every live panel.',
                ],
                notation: ['G*'],
                tags: ['constants', 'theory'],
            },
            {
                id: 'alpha',
                title: 'α (Fine Structure Constant)',
                shortTitle: 'α',
                summary: 'The electromagnetic coupling constant used as a benchmark throughout the project.',
                body: [
                    'Alpha is the fine structure constant. In the project references it is one of the most frequently cited values because many theoretical comparisons are organized around it.',
                    'In the engine UI, \\(\\alpha\\) is more likely to appear in educational descriptions, derived constants, or explanatory material than as a live control.',
                ],
                bullets: [
                    'Electromagnetic coupling constant.',
                    'Central to many FTD theory references.',
                    'Often used as a comparison or scaling benchmark.',
                ],
                notation: ['α'],
                tags: ['constants', 'electromagnetism'],
            },
            {
                id: 'nc',
                title: '\\(N_c\\) (Color Charges)',
                shortTitle: 'N_c',
                summary: 'Framework integer associated with color structure in project references.',
                body: [
                    '\\(N_c\\) is the project’s symbol for the color-charge count and is listed as 3 in the reference glossary.',
                    'For learners moving between the engine and the theory notes, this is one of the key framework integers that keeps recurring in coupling, particle, and structural discussions.',
                ],
                bullets: [
                    'Framework integer.',
                    'Listed as 3 in the reference glossary.',
                    'Shows up in coupling and particle discussions.',
                ],
                notation: ['\\(N_c\\)'],
                tags: ['constants', 'particle-physics'],
            },
            {
                id: 'n-base',
                title: '\\(N_\\mathrm{base}\\) (Base Dimension)',
                shortTitle: 'N_base',
                summary: 'A framework integer used in the project’s structural and coupling arguments.',
                body: [
                    'The symbol \\(N_\\mathrm{base}\\) is listed in the symbol glossary as the base dimension integer with value 4.',
                    'You will usually see it in theory-facing expressions rather than in runtime telemetry, but it matters because it helps tie together several framework counts and derived integer combinations.',
                ],
                bullets: [
                    'Framework integer with value 4 in the glossary.',
                    'Often appears alongside \\(N_c\\) and b₃.',
                    'More structural than operational in the live UI.',
                ],
                notation: ['\\(N_\\mathrm{base} = 4\\)'],
                tags: ['constants', 'framework-integers'],
            },
            {
                id: 'b3',
                title: 'b₃ (QCD Beta Coefficient)',
                shortTitle: 'b₃',
                summary: 'The framework \\(\\beta\\) coefficient used in strong-coupling discussions.',
                body: [
                    'The reference glossary lists b₃ as the QCD \\(\\beta\\) coefficient with value 7. In the broader theory writing it participates in coupling formulas and angle selections.',
                    'For learners, the practical point is that b₃ is one of the named integer ingredients that connects structural counting to effective coupling stories.',
                ],
                bullets: [
                    'Listed as 7 in the glossary.',
                    'Shows up in strong-coupling and CKM-style expressions.',
                    'Often grouped with \\(N_c\\), \\(N_\\mathrm{base}\\), and \\(n_\\mathrm{eff}\\)',
                ],
                notation: ['b₃ = 7'],
                tags: ['constants', 'qcd', 'framework-integers'],
            },
            {
                id: 'n-eff',
                title: '\\(n_\\mathrm{eff}\\) (Effective Dimension)',
                shortTitle: 'n_eff',
                summary: 'The effective-dimension count used in several FTD coupling formulas.',
                body: [
                    'The glossary gives \\(n_\\mathrm{eff}\\) as 13. It is one of the recurring bookkeeping counts used in electroweak and strong-coupling expressions.',
                    'When the project refers to an effective dimension in a formula, it is usually talking about a model-specific counting quantity rather than a literal spatial dimension you can navigate inside the viewport.',
                ],
                bullets: [
                    'Listed as 13 in the glossary.',
                    'Appears in coupling formulas like \\(\\sin^2\\theta_W\\) and \\(\\alpha_s\\)',
                    'A framework count, not a camera-space coordinate axis.',
                ],
                notation: ['\\(n_\\mathrm{eff} = 13\\)'],
                tags: ['constants', 'dimension', 'framework-integers'],
            },
            {
                id: 'g-newton',
                title: '\\(G_N\\) (Newton’s Constant)',
                shortTitle: 'G_N',
                summary: 'The gravitational coupling constant in natural-unit notation.',
                body: [
                    '\\(G_N\\) is the usual symbol for Newton’s gravitational constant. In natural-unit conventions it is often written without SI dimensions, but conceptually it is still the coupling that sets gravitational strength.',
                    'This is a good example of why context matters: uppercase G* and \\(G_N\\) are very different objects even though both start with the same letter.',
                ],
                bullets: [
                    'Gravitational coupling constant.',
                    'Distinct from G*.',
                    'Important for Planck-unit definitions.',
                ],
                notation: ['\\(G_N\\)'],
                tags: ['constants', 'gravity', 'disambiguation'],
            },
            {
                id: 'manifestation-kb',
                title: '\\(K_B\\) (Manifestation Constant)',
                shortTitle: 'K_B',
                summary: 'Engine-side named constant currently exported as 0.511.',
                body: [
                    'In the web engine tests, \\(K_B\\) is verified as a named export with value 0.511. The project instructions describe this as the manifestation constant derived from the model’s mass story.',
                    'This is not the same thing as the lowercase \\(k_B\\) used for the Boltzmann constant in natural-units notation. The near-identical names make this one worth learning explicitly.',
                ],
                bullets: [
                    'Named export in the JS constants module.',
                    'Tested in the web suite as 0.511.',
                    'Different from the lowercase \\(k_B\\) of thermodynamics.',
                ],
                notation: ['\\(K_B = 0.511\\)', '\\(k_B = 1\\)'],
                tags: ['constants', 'engine', 'disambiguation'],
            },
            {
                id: 'c-speed',
                title: '\\(C_\\mathrm{SPEED}\\)',
                shortTitle: 'C_SPEED',
                summary: 'The engine’s named propagation-speed constant.',
                body: [
                    'The engine documentation lists \\(C_\\mathrm{SPEED}\\) as the propagation-speed constant used for cubic-lattice stability conventions.',
                    'Even when it is not surfaced directly in controls, it helps explain why the engine talks about local propagation limits, CFL-like stability, and discrete-time update pacing.',
                ],
                bullets: [
                    'Named engine constant.',
                    'Used in discrete propagation and stability explanations.',
                    'Connects runtime behavior to the lattice update model.',
                ],
                notation: ['\\(C_\\mathrm{SPEED}\\)'],
                tags: ['constants', 'runtime'],
            },
        ],
    },
    {
        id: 'dimensions-units',
        title: 'Dimensions & Units',
        description: 'How FTD and physics talk about dimension, scale, and measurement.',
        entries: [
            {
                id: 'dimension',
                title: 'Dimension',
                shortTitle: 'Dimension',
                summary: 'A count of independent directions or degrees of freedom needed to specify something.',
                body: [
                    'In geometry, a dimension counts independent directions such as x, y, and z. In physics, the word can also refer to dimensionality in state spaces, effective models, or scaling arguments.',
                    'Because this project uses phrases like base dimension, effective dimension, and scale, it helps to separate literal spatial dimensions from bookkeeping dimensions and from presentation scales in the UI.',
                ],
                bullets: [
                    'Spatial dimensions describe geometry.',
                    'Effective dimensions can describe a model or counting scheme.',
                    'UI scales are presentation layers, not extra spatial axes.',
                ],
                notation: ['D', 'dim'],
                tags: ['dimension', 'units', 'geometry'],
            },
            {
                id: 'd-equals-3',
                title: 'D = 3',
                shortTitle: 'D = 3',
                summary: 'The project’s physical space is three-dimensional.',
                body: [
                    'In the FTD engine, the substrate is a 3D cubic lattice. That means every voxel position lives in three spatial coordinates.',
                    'This does not mean every other quantity in the project has three components. Some are scalars, some are vectors, some are matrices, and some are abstract counts like \\(n_\\mathrm{eff}\\).',
                ],
                bullets: [
                    'Three spatial dimensions for the lattice.',
                    'Matches the cubic-grid visualization used in the engine.',
                    'Distinct from model-specific integer counts like \\(N_\\mathrm{base}\\) and \\(n_\\mathrm{eff}\\)',
                ],
                notation: ['D = 3', '\\(L \\subset \\mathbb{Z}^3\\)'],
                tags: ['dimension', 'space', 'geometry'],
            },
            {
                id: 'natural-units',
                title: 'Natural Units',
                shortTitle: 'Natural Units',
                summary: 'A unit convention where key constants are set to 1 so equations focus on relationships instead of conversion factors.',
                body: [
                    'The project glossary states that FTD uses natural units with c = 1, ℏ = 1, and \\(k_B\\) = 1. This is a standard physics move that simplifies formulas by absorbing conversion factors into the unit definitions.',
                    'When you see a formula in natural units, it does not mean those constants disappeared physically. It means the chosen unit system measures everything relative to them.',
                ],
                bullets: [
                    'Speed of light: c = 1.',
                    'Reduced Planck constant: ℏ = 1.',
                    'Boltzmann constant: \\(k_B\\) = 1.',
                ],
                notation: ['c = 1', 'ℏ = 1', '\\(k_B = 1\\)'],
                tags: ['units', 'natural-units', 'conventions'],
            },
            {
                id: 'planck-units',
                title: 'Planck Units',
                shortTitle: 'Planck Units',
                summary: 'The natural-unit scale built from c, ℏ, and \\(G_N\\)',
                body: [
                    'Planck units are the characteristic scales built from the speed of light, reduced Planck constant, and Newton’s constant. They provide a compact unit language for quantum gravity and lattice-scale reasoning.',
                    'The project glossary explicitly maps Planck length, time, and mass into the FTD unit story, which makes them especially useful as bridge concepts between theory documents and the engine.',
                ],
                bullets: [
                    'Length, time, and mass can all be expressed in Planck units.',
                    'Planck units are built from c, ℏ, and \\(G_N\\)',
                    'Helpful when comparing discrete engine units to theory-facing notation.',
                ],
                notation: ['\\(\\ell_P = \\sqrt{\\hbar G/c^3}\\)', '\\(t_P = \\sqrt{\\hbar G/c^5}\\)', '\\(m_P = \\sqrt{\\hbar c/G}\\)'],
                tags: ['units', 'planck', 'gravity'],
            },
            {
                id: 'voxel-length-unit',
                title: 'Voxel as Length Unit',
                shortTitle: 'Voxel Length',
                summary: 'One voxel is the engine’s basic spatial step in the lattice.',
                body: [
                    'Inside the web engine, positions advance in lattice-site increments. That makes the voxel the practical local length unit for direct substrate reasoning.',
                    'The glossary’s Planck-unit convention identifies one Planck-length-like step with one voxel, which is a useful learner shorthand when connecting symbolic formulas to the live lattice.',
                ],
                bullets: [
                    'Smallest explicit spatial step in the simulation.',
                    'Most direct length unit in Scale 0.',
                    'Useful for interpreting local neighborhoods and propagation limits.',
                ],
                notation: ['1 voxel', 'Δx'],
                tags: ['units', 'length', 'lattice'],
            },
            {
                id: 'tick-time-unit',
                title: 'Tick as Time Unit',
                shortTitle: 'Tick Time',
                summary: 'One tick is the engine’s basic discrete time step.',
                body: [
                    'Time in the engine advances in ticks, not in continuous fractions of a second. Playback can make ticks arrive faster or slower on screen, but the simulation’s intrinsic temporal unit is still the tick.',
                    'The glossary aligns one Planck-time-like step with one tick, which makes the tick the natural time unit for reading call stacks, dynamics, and propagation limits.',
                ],
                bullets: [
                    'The engine’s native time unit.',
                    'Independent from frame rate or browser refresh cadence.',
                    'Useful for reading logs, telemetry, and runtime stepping behavior.',
                ],
                notation: ['1 tick', 'Δt'],
                tags: ['units', 'time', 'runtime'],
            },
            {
                id: 'mass-energy',
                title: 'Mass and Energy',
                shortTitle: 'Mass / Energy',
                summary: 'Two closely related physical quantities that often share units in natural-unit systems.',
                body: [
                    'In ordinary SI language, mass and energy are measured differently. In natural units, they are often expressed in the same unit family because c is set to 1.',
                    'That is why physics texts often speak loosely about mass scales, energy scales, and mass-energy units in a nearly interchangeable way. In the engine UI, energy telemetry is live while many mass concepts appear in the explanatory layer.',
                ],
                bullets: [
                    'Natural units blur the practical difference between mass and energy units.',
                    'Mass stories in the docs often connect to energy scales in the UI.',
                    'A quantity can be conceptually different even if it shares a unit convention.',
                ],
                notation: ['E', 'm', '\\(E = mc^2\\)', 'c = 1'],
                tags: ['units', 'mass', 'energy'],
            },
            {
                id: 'density',
                title: 'Density',
                shortTitle: 'Density',
                summary: 'How much of something is present per unit volume, area, or state description.',
                body: [
                    'Density is a broad word in physics. It can mean mass density, charge density, probability density, energy density, or in the project glossary, flux density.',
                    'Whenever you see density in the KB or UI, ask what is being counted and with respect to what measure. That keeps scalar field magnitudes from being confused with matrix objects or population counts.',
                ],
                bullets: [
                    'Always ask: density of what?',
                    'Can be spatial, probabilistic, energetic, or informational.',
                    'In the glossary, ρ(v,t) is the flux density |J|.',
                ],
                notation: ['ρ', 'ρ(v,t)', '|J|'],
                tags: ['units', 'density', 'disambiguation'],
            },
            {
                id: 'coupling-constant',
                title: 'Coupling Constant',
                shortTitle: 'Coupling',
                summary: 'A parameter that tells you how strongly one field, force, or sector interacts with another.',
                body: [
                    'A coupling constant measures interaction strength. Fine structure \\(\\alpha\\), strong coupling \\(\\alpha_s\\), and state-flux couplings all belong to this family.',
                    'For learners, the main intuition is simple: a larger coupling usually means a stronger interaction, though the exact physical meaning depends on the theory and scale.',
                ],
                bullets: [
                    'Couplings measure interaction strength.',
                    'Can be dimensionless or unit-bearing depending on the theory.',
                    'Several project formulas are organized around specific coupling values.',
                ],
                notation: ['\\(\\alpha\\)', '\\(\\alpha_s\\)', '\\(g_c\\)'],
                tags: ['units', 'coupling', 'interactions'],
            },
            {
                id: 'speed-and-velocity',
                title: 'Speed and Velocity',
                shortTitle: 'Speed / Velocity',
                summary: 'Speed measures how fast; velocity measures both how fast and in which direction.',
                body: [
                    'Speed is a scalar magnitude. Velocity is a vector quantity that includes direction. This matters in a field-based engine because scalar magnitudes like |J| and vector quantities like J play different roles.',
                    'When the project talks about propagation speed limits or local directional flux, it is moving between these scalar and vector viewpoints.',
                ],
                bullets: [
                    'Speed is scalar; velocity is vector.',
                    'Propagation limits are about how fast information can move.',
                    'Vector fields encode direction as well as size.',
                ],
                notation: ['|v|', 'v⃗', 'J'],
                tags: ['units', 'kinematics', 'vectors'],
            },
        ],
    },
    {
        id: 'physics-terms',
        title: 'Physics Terms',
        description: 'Cross-domain ideas that appear in the theory corpus and learning surfaces.',
        entries: [
            {
                id: 'wave-function',
                title: 'ψ (Wave Function)',
                shortTitle: 'ψ',
                summary: 'A state-amplitude description used in quantum mechanics and in the project glossary’s complexified flux discussion.',
                body: [
                    'The symbol glossary defines ψ as a complexified flux combination Jx + iJy. More broadly, a wave function is the object used to encode amplitudes over a state space.',
                    'In learner terms, the wave function is not a directly visible classical object. It is a compact mathematical description whose derived probabilities and expectation values are usually what get compared to experiment.',
                ],
                bullets: [
                    'Often complex-valued.',
                    'Used to compute amplitudes and probabilities.',
                    'In the glossary, tied to a complexified flux interpretation.',
                ],
                notation: ['ψ', 'ψ = Jx + iJy'],
                tags: ['quantum', 'wave-function', 'symbols'],
            },
            {
                id: 'hilbert-space',
                title: '\\(H_\\mathrm{FTD}\\) (Hilbert Space)',
                shortTitle: 'Hilbert Space',
                summary: 'The vector-space setting where wave functions and inner products live.',
                body: [
                    'A Hilbert space is a complete inner-product space. In physics it is the standard home for quantum states, amplitudes, and operators.',
                    'The glossary names \\(H_\\mathrm{FTD}\\) as L² over the lattice with complex values. That is a formal way of saying the model’s quantum-style states are functions on the lattice with an inner-product structure.',
                ],
                bullets: [
                    'State space for quantum vectors.',
                    'Supports inner products, norms, and operators.',
                    'The glossary writes \\(H_\\mathrm{FTD}\\) as a lattice-based L² space.',
                ],
                notation: ['\\(H_\\mathrm{FTD}\\)', 'L²(Lattice, C)'],
                tags: ['quantum', 'hilbert-space', 'linear-algebra'],
            },
            {
                id: 'density-matrix',
                title: 'ρ (Density Matrix)',
                shortTitle: 'Density Matrix',
                summary: 'The matrix object used to represent pure states, mixed states, and statistical quantum ensembles.',
                body: [
                    'A density matrix packages state information in an operator rather than a single ket. It is especially useful when a system is probabilistic, mixed, or part of a larger entangled whole.',
                    'This is the matrix meaning of \\(\\rho\\), not the scalar density meaning. That distinction is worth learning early because both uses appear in the project’s reference material.',
                ],
                bullets: [
                    'Pure state example: |ψ⟩⟨ψ|.',
                    'Mixed state example: Σ pᵢ|ψᵢ⟩⟨ψᵢ|.',
                    'Central to entropy and statistical-state discussions.',
                ],
                notation: ['ρ = |ψ⟩⟨ψ|', 'ρ = Σ pᵢ|ψᵢ⟩⟨ψᵢ|'],
                tags: ['quantum', 'density-matrix', 'statistics'],
            },
            {
                id: 'inner-product',
                title: 'Inner Product',
                shortTitle: 'Inner Product',
                summary: 'The operation that measures overlap, norm, and angle-like structure in a vector space.',
                body: [
                    'An inner product generalizes the familiar dot product to broader spaces such as function spaces. In quantum mechanics it produces amplitudes and norms.',
                    'In the glossary, the inner product is written as a sum over lattice sites. That makes it directly relevant to wave functions defined on the lattice.',
                ],
                bullets: [
                    'Measures overlap between states.',
                    'Defines norms and orthogonality.',
                    'Written with bra-ket notation in many physics texts.',
                ],
                notation: ['⟨ψ|φ⟩', 'Σᵥ ψ*(v)φ(v)'],
                tags: ['quantum', 'linear-algebra', 'notation'],
            },
            {
                id: 'shannon-entropy',
                title: 'H (Shannon Entropy)',
                shortTitle: 'Shannon H',
                summary: 'The information-theoretic measure of uncertainty in a probability distribution.',
                body: [
                    'Shannon entropy measures how uncertain or spread out a classical probability distribution is. The more evenly distributed the possibilities are, the larger the entropy.',
                    'This is the right entropy when you are talking about classical information content rather than quantum-state operators.',
                ],
                bullets: [
                    'Used for classical distributions.',
                    'Higher values mean more uncertainty or spread.',
                    'Foundational to information theory and telemetry interpretation.',
                ],
                notation: ['H = -Σ pᵢ log pᵢ'],
                tags: ['information', 'entropy', 'statistics'],
            },
            {
                id: 'von-neumann-entropy',
                title: '\\(S_\\mathrm{vN}\\) (von Neumann Entropy)',
                shortTitle: 'S_vN',
                summary: 'The quantum analogue of entropy built from the density matrix.',
                body: [
                    'Von Neumann entropy measures the uncertainty or mixedness of a quantum state using the density matrix. It reduces to zero for a pure state and grows when the state becomes mixed.',
                    'It is the operator-based entropy you want when a system is described by ρ rather than by a plain classical probability table.',
                ],
                bullets: [
                    'Quantum-state entropy.',
                    'Built from the density matrix.',
                    'Useful for mixed states and entanglement discussions.',
                ],
                notation: ['\\(S_\\mathrm{vN} = -\\mathrm{Tr}(\\rho \\ln \\rho)\\)'],
                tags: ['information', 'entropy', 'quantum'],
            },
            {
                id: 'mutual-information',
                title: 'I (Mutual Information)',
                shortTitle: 'Mutual Info',
                summary: 'A measure of how much knowing one part tells you about another.',
                body: [
                    'Mutual information measures shared information between two systems or variables. If two pieces are independent, their mutual information is zero. If they are strongly correlated, it rises.',
                    'For learners, it is one of the cleanest ways to formalize “how linked are these two things?” without committing to a single mechanical interpretation.',
                ],
                bullets: [
                    'Measures shared information.',
                    'Zero means independence.',
                    'Useful in classical and quantum correlation discussions.',
                ],
                notation: ['I(A;B) = H(A) + H(B) - H(AB)'],
                tags: ['information', 'correlation', 'statistics'],
            },
            {
                id: 'weak-mixing-angle',
                title: 'sin²θ_W (Weak Mixing Angle)',
                shortTitle: 'sin²θ_W',
                summary: 'The electroweak mixing quantity that shows up in standard-model-style coupling discussions.',
                body: [
                    'The symbol glossary includes sin²θ_W as the weak mixing quantity and gives an FTD-side formula for it (\\(N_c/n_\\mathrm{eff} = 3/13 \\approx 0.231\\)). This is a [PARAMETRIC] insertion, not a derivation: it lands ~3.5% from the measured value and was demoted per FTD-0018 (2026-04-19). In mainstream particle-physics language, it measures how electroweak components combine.',
                    'In the knowledge base, the main point is recognition: this is a coupling-related parameter, not a literal geometric angle you rotate with the camera.',
                ],
                bullets: [
                    'Electroweak parameter.',
                    '[PARAMETRIC]: 3/13 ≈ 0.231, ~3.5% from measured (FTD-0018).',
                    'Appears in coupling formulas.',
                    'Not a viewport angle or scene transform.',
                ],
                notation: ['\\(\\sin^2\\theta_W = N_c/n_\\mathrm{eff}\\)'],
                tags: ['particle-physics', 'coupling', 'electroweak'],
            },
            {
                id: 'strong-coupling',
                title: 'α_s (Strong Coupling)',
                shortTitle: 'α_s',
                summary: 'The interaction-strength parameter associated with the strong sector.',
                body: [
                    '\\(\\alpha_s\\) is the strong coupling constant. It plays for strong interactions a role somewhat analogous to what \\(\\alpha\\) plays in electromagnetism, though the physical behavior is different.',
                    'The project glossary includes a model-side expression for α_s, so it is a useful bridge term between theory files and educational UI copy.',
                ],
                bullets: [
                    'Strong-interaction coupling.',
                    'Appears in coupling and mixing discussions.',
                    'Distinct from electromagnetic \\(\\alpha\\)',
                ],
                notation: ['α_s = b₃ / (b₃ + 4n_eff)'],
                tags: ['particle-physics', 'coupling', 'qcd'],
            },
            {
                id: 'density-parameter',
                title: 'Ω (Density Parameter)',
                shortTitle: 'Ω',
                summary: 'The ratio of a density to a reference critical density in cosmology.',
                body: [
                    'In cosmology, \\(\\Omega\\) often denotes a density parameter: how much of some component exists relative to a critical benchmark density.',
                    'This is another overloaded symbol. In other contexts \\(\\Omega\\) can mean a microstate count or a region of configuration space, so context matters.',
                ],
                bullets: [
                    'Common in cosmology.',
                    'Usually read as a normalized density ratio.',
                    'Should not be confused with every other meaning of Ω.',
                ],
                notation: ['Ω = ρ / ρ_crit'],
                tags: ['cosmology', 'density', 'disambiguation'],
            },
            {
                id: 'baryon-asymmetry',
                title: 'η (Baryon Asymmetry)',
                shortTitle: 'η',
                summary: 'A normalized measure of the matter-antimatter imbalance in cosmology.',
                body: [
                    'The baryon-asymmetry symbol η is used in cosmology to describe the excess of baryons over antibaryons relative to a photon-count benchmark.',
                    'Even if the web UI is not numerically exploring cosmology in detail on every screen, this symbol appears in the project’s broader glossary and is part of the physics vocabulary learners are likely to encounter.',
                ],
                bullets: [
                    'Matter-antimatter imbalance parameter.',
                    'Normalized to a photon reference count.',
                    'Belongs to the cosmology vocabulary rather than the lattice runtime vocabulary.',
                ],
                notation: ['\\(\\eta = (n_B - n_\\bar{B})/n_\\gamma\\)'],
                tags: ['cosmology', 'asymmetry', 'particle-physics'],
            },
        ],
    },
    {
        id: 'scales',
        title: 'Scale Guide',
        description: 'What each simulation scale is trying to show.',
        entries: [
            {
                id: 'scale0',
                title: 'Scale 0: Lattice',
                shortTitle: 'Scale 0',
                summary: 'Direct substrate simulation with field overlays, force styles, and lattice diagnostics.',
                body: [
                    'Scale 0 is the substrate-facing mode of the web engine. It is the place where you most directly see the lattice, ternary state behavior, and flux-derived overlays.',
                    'If you want to learn what the symbols mean in the most immediate runtime context, this is usually the best starting point.',
                ],
                bullets: [
                    'Primary place for lattice inspection.',
                    'Hosts field and force overlays.',
                    'Most direct view of substrate-level runtime behavior.',
                ],
                notation: ['Scale 0'],
                tags: ['scales', 'lattice'],
            },
            {
                id: 'scale1',
                title: 'Scale 1: Particles',
                shortTitle: 'Scale 1',
                summary: 'Particle-engine view and telemetry built on top of lower-level dynamics.',
                body: [
                    'Scale 1 presents particle-like behavior and exposes dedicated telemetry cards and scenario controls.',
                    'This is where learners usually move after the lattice if they want a more object-centered interpretation of the same simulation universe.',
                ],
                bullets: [
                    'Particle-oriented telemetry.',
                    'Scenario-driven educational entry point.',
                    'Higher-level interpretation than raw lattice view.',
                ],
                notation: ['Scale 1'],
                tags: ['scales', 'particles'],
            },
            {
                id: 'scale2',
                title: 'Scale 2: Atoms',
                shortTitle: 'Scale 2',
                summary: 'Atomic and bond-oriented view with derived structure displays.',
                body: [
                    'Scale 2 organizes the simulation into atom-like and bond-like structures for inspection and pedagogy.',
                    'For users trying to understand how structure emerges upward from the lattice and particle layers, this is the next abstraction tier.',
                    'Beyond the force-arrow channels (Coulomb, van der Waals, bond, net), three structure overlays expose quantities the engine computes per atom: velocity vectors (kinetic state), dipole-moment arrows (built from bond electronegativity differences), and dashed donor-H···acceptor lines that annotate geometrically eligible hydrogen bonds.',
                    'All Scale 2 energies, temperatures, and momenta are sim units (implicit k_B = 1) — panel labels say “(sim)”, never MeV or Kelvin. The temperature readout is the bare equipartition statistic 2·KE/(3N).',
                ],
                bullets: [
                    'Atom-focused view.',
                    'Includes derived molecular and bonding information.',
                    'Bridges particle-level behavior to chemistry-like structure.',
                    'Structure overlays: velocities, dipole arrows, H-bond lines (display gates only — the underlying forces have no hard cutoff).',
                    'Scenario presets light the overlays relevant to each demo (ionic → F_C + field; water → H-bond lines; thermal → velocities).',
                ],
                notation: ['Scale 2', 'T = 2·KE/(3N) (sim)', 'μ ∝ Σ r·Δχ'],
                tags: ['scales', 'atoms'],
            },
            {
                id: 'scale3',
                title: 'Scale 3: Molecules',
                shortTitle: 'Scale 3',
                summary: 'The chemistry-facing scale that organizes atom groups into molecular scenarios.',
                body: [
                    'Scale 3 presents molecule-level scenarios and is the natural continuation of the atom layer when the learner wants structure larger than a single atom.',
                    'It is a good reminder that the UI scale ladder is pedagogical and structural, not a claim that each scale is a separate universe with different base laws.',
                ],
                bullets: [
                    'Molecule-oriented scenarios.',
                    'Builds upward from Scale 2.',
                    'Helpful for chemistry-like structural interpretation.',
                ],
                notation: ['Scale 3'],
                tags: ['scales', 'molecules'],
            },
            {
                id: 'scale4',
                title: 'Scale 4: Planetary',
                shortTitle: 'Scale 4',
                summary: 'The planetary presentation layer for orbit-like or layered macro structure.',
                body: [
                    'Scale 4 reframes the engine’s presentation toward planetary or celestial structure and ships with its own scenario controls and panels.',
                    'This is one of the places where the shell becomes more about interpretive visualization than about direct lattice-state inspection.',
                ],
                bullets: [
                    'Planetary scenarios and panels.',
                    'Macro-scale educational framing.',
                    'More interpretive than substrate-facing.',
                ],
                notation: ['Scale 4'],
                tags: ['scales', 'planetary'],
            },
            {
                id: 'scale5',
                title: 'Scale 5: Cosmic',
                shortTitle: 'Scale 5',
                summary: 'Large-scale cosmic presentation with its own telemetry and camera controls.',
                body: [
                    'Scale 5 moves the presentation into the cosmic regime, with dedicated scenarios, cameras, and telemetry.',
                    'It is useful for learners who want to understand how the engine re-expresses simulation data at a far larger conceptual scale than the lattice or particle views.',
                ],
                bullets: [
                    'Dedicated cosmic scenarios and cameras.',
                    'Own telemetry surface.',
                    'Very different pedagogical framing from Scale 0.',
                ],
                notation: ['Scale 5'],
                tags: ['scales', 'cosmic'],
            },

        ],
    },
    {
        id: 'runtime',
        title: 'Runtime & UI',
        description: 'How to read overlays, telemetry, and engine outputs.',
        entries: [
            {
                id: 'field-overlays',
                title: 'Field Overlays',
                shortTitle: 'Overlays',
                summary: 'Visual layers that turn sampled field data into lines, slices, force cues, or volume views.',
                body: [
                    'Field overlays are the bridge between runtime data and visual explanation. They take sampled quantities such as flux, force-like derived values, or slices of a volume and draw them into the viewport.',
                    'In the modularized Scale 0 architecture, overlay updates are treated as their own runtime phase so they can be reasoned about independently from ticking and diagnostics.',
                ],
                bullets: [
                    'Translate data into visual explanation.',
                    'Can show vectors, magnitudes, slices, or derived force cues.',
                    'Especially important in Scale 0.',
                ],
                notation: ['overlay frame'],
                tags: ['ui', 'runtime', 'visualization'],
            },
            {
                id: 'telemetry',
                title: 'Telemetry',
                shortTitle: 'Telemetry',
                summary: 'Readouts that summarize the current runtime state of a scale.',
                body: [
                    'Telemetry surfaces turn raw engine state into quickly readable numerical summaries. Examples include energy, particle counts, scale-specific diagnostics, and status-bar summaries.',
                    'A good rule of thumb is that telemetry answers “what is happening right now?” while the knowledge base answers “what does this quantity mean?”',
                ],
                bullets: [
                    'Short-form numerical readouts.',
                    'Can be global or scale-specific.',
                    'Best read together with the underlying symbol definitions.',
                ],
                notation: ['energy', 'particles', 'fps'],
                tags: ['ui', 'telemetry'],
            },
            {
                id: 'status-bar',
                title: 'Status Bar',
                shortTitle: 'Status',
                summary: 'Global runtime strip for state, tick, energy, particle count, and performance.',
                body: [
                    'The status bar is the global snapshot strip at the bottom of the shell. It summarizes whether the engine is idle or running and reports high-level counts like tick, particles, energy, and frame rate.',
                    'It is not a substitute for the deeper scale panels, but it gives you the fastest cross-scale pulse of the simulation.',
                ],
                bullets: [
                    'Global rather than scale-specific.',
                    'Includes tick, particles, energy, and fps.',
                    'Useful for confirming whether the engine is advancing.',
                ],
                notation: ['tick', 'fps', 'energy'],
                tags: ['ui', 'status'],
            },
            {
                id: 'inspector',
                title: 'Inspector',
                shortTitle: 'Inspector',
                summary: 'Selection-driven details view for the currently inspected object or lattice location.',
                body: [
                    'The inspector is the learn-by-click feature of the UI. It lets users select a target in the viewport and then read the scale-specific information associated with that target.',
                    'Because the inspector is now modularized by scale, what you see there depends heavily on the active mode: lattice sites, particles, atoms, planetary objects, and cosmic entities all have different detail surfaces.',
                ],
                bullets: [
                    'Selection-based details surface.',
                    'Changes meaning across scales.',
                    'Best way to tie visual objects to textual information.',
                ],
                notation: ['selection', 'inspect'],
                tags: ['ui', 'learning'],
            },
            {
                id: 'gauss-projection',
                title: 'Gauss Projection',
                shortTitle: 'Gauss',
                summary: 'The projection phase that enforces a Gauss-like constraint in the tick cycle.',
                body: [
                    'The engine documentation lists gauss projection as one of the explicit phases in the tick cycle. Conceptually, this is where the runtime re-projects or regularizes the field to satisfy the relevant constraint before later phases continue.',
                    'For learners, the main value of this entry is recognizing that some runtime phases are not just visualization steps; they actively enforce structural conditions on the evolving field.',
                ],
                bullets: [
                    'Named tick-cycle phase.',
                    'Constraint-enforcement step.',
                    'Important for understanding the ordered call stack.',
                ],
                notation: ['gauss_project'],
                tags: ['runtime', 'call-stack'],
            },
        ],
    },
]);

function dedupe(values = []) {
    return Array.from(new Set(values.filter(Boolean)));
}

function makeScenarioEntry({
    id,
    title,
    shortTitle = title,
    scale,
    summary,
    body = [],
    bullets = [],
    notation = [],
    tags = [],
}) {
    return {
        id: `scenario-${id}`,
        title,
        shortTitle,
        summary,
        body,
        bullets: dedupe([`Scale: ${scale}.`, ...bullets]),
        notation: dedupe(notation),
        tags: dedupe(['scenario', scale.toLowerCase(), ...tags]),
    };
}

const SCALE0_CATEGORY_GUIDES = Object.freeze({
    Empty: {
        summary: 'A baseline reference state with minimal structure and no designed excitation.',
        body: [
            'This category is useful for establishing what the engine does in the absence of a deliberately prepared field pattern. It gives you a quiet reference state for comparing later scenarios.',
            'Mathematically, the empty case is the closest thing to a zero or vacuum-like initial condition in the live lattice UI. It is the right place to compare growth, decay, or spontaneous structure formation against a calm baseline.',
        ],
        notation: ['s(v,t) = 0', 'J(v,t) ≈ 0'],
        tags: ['baseline', 'reference'],
    },
    'Wave Dynamics': {
        summary: 'Prepared flux patterns that emphasize propagation, superposition, circulation, and localized wave structure.',
        body: [
            'Wave-dynamics scenarios are the cleanest place to study how the flux field J propagates, interferes, forms nodes, and organizes into coherent patterns.',
            'The main math language here is wave mechanics on the lattice: amplitudes, wavelengths, phase, gradients, standing modes, and sometimes vorticity through curl-like structure.',
        ],
        notation: ['J(v,t)', '|J|', 'ω', 'k', '∇×J'],
        tags: ['waves', 'interference', 'propagation'],
    },
    'Genesis & Manifestation': {
        summary: 'Threshold and emergence scenarios for appearance, disappearance, and noisy substrate activity.',
        body: [
            'Genesis scenarios focus on how manifestation turns on, cascades, randomizes, or cancels. They are about transitions between quiet substrate structure and visibly manifested events.',
            'The math emphasis is on thresholding, local amplification, creation-annihilation balances, and stochastic or pseudo-random seeding rather than on steady-state oscillation alone.',
        ],
        notation: ['|J|', '∇|J|', 'Δt', 'threshold'],
        tags: ['genesis', 'manifestation', 'threshold'],
    },
    Confinement: {
        summary: 'Scenarios that highlight string-like flux tubes, bound composites, and separation energy.',
        body: [
            'Confinement scenarios are about what happens when localized sources are connected by structured flux rather than freely separating without cost.',
            'The relevant mathematical intuition is tension, effective potential growth with separation, flux-tube energy, and Wilson-loop or area-law style reasoning rather than simple inverse-square falloff alone.',
        ],
        notation: ['V(r)', 'σ', 'Wilson loop', 'flux tube'],
        tags: ['confinement', 'bound-states'],
    },
    'Substrate Physics': {
        summary: 'Transport, screening, circulation, relaxation, and small-scale organization in the lattice substrate.',
        body: [
            'These scenarios treat the substrate as an active medium with transport, damping, orbital motion, and collective organization.',
            'The math language includes Lorentz-force-style motion, screened interactions, relaxation toward equilibrium, and small-N structure formation.',
        ],
        notation: ['q(v × B)', 'exp(-r/λ)', 'thermalization'],
        tags: ['substrate', 'transport', 'relaxation'],
    },
    'Light & EM': {
        summary: 'Optics-facing scenarios for radiation, color separation, path difference, and photon-scale propagation.',
        body: [
            'These are the scenarios to use when you want the lattice language to line up with familiar optics and electromagnetic demonstrations.',
            'The core math is interference, dipole radiation, transverse propagation, path-length phase difference, and dispersion-like color separation.',
        ],
        notation: ['E', 'B', '∇·J = 0', 'Δφ', 'I ∝ |A|²'],
        tags: ['optics', 'electromagnetism', 'light'],
    },
    'Quantum Lab': {
        summary: 'Quantitative tests where lattice outcomes are compared against standard quantum-style observables.',
        body: [
            'Quantum-lab scenarios turn the lattice into a measurement playground: distributions, fringes, barrier penetration, topological phase, and constrained vacuum effects.',
            'They are the most explicitly formula-driven scenarios in Scale 0 and are best read alongside probability amplitudes, transmission laws, spectral quantization, and correlation observables.',
        ],
        notation: ['\\(|J|^2\\)', 'T ∝ exp(-2κW)', '\\(f_n \\propto n^2\\)', '\\(S = 2\\sqrt{2}\\)'],
        tags: ['quantum', 'measurement', 'lab'],
    },
    'SM Seeds (epistemic-tagged)': {
        summary: 'Named seed configurations with explicit epistemic caution about what is derived versus what is imposed.',
        body: [
            'These seed scenarios are intentionally theory-facing. They are not just visual demos; they carry explicit claims about what is structurally motivated, what is selected, and what remains conjectural.',
            'The right way to read them is to separate configuration geometry from the physics quantity being compared. A mass formula can be derived without that proving a unique spatial realization.',
        ],
        notation: ['[THEOREM]', '[SELECTION]', '[CONJECTURE]', '[IMPOSED]'],
        tags: ['epistemic', 'seed', 'standard-model'],
    },
    'Elementary Particles': {
        summary: 'Single-particle-style seeds and dressed candidates at the lattice level.',
        body: [
            'These scenarios test whether a prepared localized pattern behaves like a plausible particle candidate under the substrate rules.',
            'The math focus is on conserved charge-like content, dressing envelopes, propagation, chirality, and whether the seed persists or disperses under local dynamics.',
        ],
        notation: ['s(v,t)', 'J(v,t)', 'charge', 'chirality'],
        tags: ['particles', 'seed'],
    },
    'Composite Particles': {
        summary: 'Multi-site or multi-constituent seeds that probe bound or composite structure.',
        body: [
            'Composite seeds ask whether several localized pieces can act like one persistent object rather than immediately dissolving.',
            'The mathematical language here is binding energy, effective potential, symmetry of the composite, and whether separation costs increase with distance.',
        ],
        notation: ['V(r)', 'binding energy', 'triad'],
        tags: ['composite', 'binding'],
    },
    'Atoms & Molecules': {
        summary: 'Lattice-level seeds for atom-like and molecule-like structure.',
        body: [
            'These are pedagogical bridge scenarios between raw lattice configurations and later atom-engine or molecule-engine views.',
            'They emphasize central potentials, shell-like organization, pair or bond formation, and whether the seeded pattern organizes into a recognizable bound aggregate.',
        ],
        notation: ['V(r)', 'bound state', 'bond length'],
        tags: ['atoms', 'molecules', 'bound-states'],
    },
    'Gauge / Topological': {
        summary: 'Loop, tube, monopole, and instanton-style configurations used to reason about topology and gauge structure.',
        body: [
            'These scenarios are about structure that matters globally or topologically, not just pointwise local amplitude.',
            'The mathematical focus is on loops, enclosed flux, topological winding, holonomy-style intuition, and whether a prepared field pattern carries global structure that cannot be erased by tiny local deformations.',
        ],
        notation: ['∮A·dl', 'flux tube', 'winding number', 'Wilson loop'],
        tags: ['gauge', 'topology'],
    },
    'Gravity / Cosmology': {
        summary: 'Seeds that frame the lattice in gravitational or cosmological language.',
        body: [
            'These scenarios use localized wells, expanding patches, or wave-like spacetime analogues to connect the lattice view to gravity-facing intuition.',
            'The key math is potential wells, horizon or patch scaling, cosmological expansion factors, and wave propagation on a large background.',
        ],
        notation: ['Φ', 'a(t)', 'h_μν', 'Schwarzschild-like well'],
        tags: ['gravity', 'cosmology'],
    },
    'Reference frame context / Observer': {
        summary: 'Observer-facing seed structures tied to self-reference and recursive closure themes.',
        body: [
            'These scenarios are conceptual seeds for observer-like or self-referential structure rather than ordinary particle or optics demos.',
            'Their math language is recursive closure, ring or loop structure, fixed-point behavior, and the extent to which a local pattern can sustain self-related organization.',
        ],
        notation: ['sLoop', 'fixed point', 'recursive closure'],
        tags: ['observer', 'reference frame structure'],
    },
    'Field Configurations': {
        summary: 'Canonical field patterns used as clean mathematical initial conditions.',
        body: [
            'This category is the closest thing to a field-theory workbook inside the lattice engine: plane waves, uniform fields, dipoles, and vortex lines.',
            'It is useful for matching visual intuition to textbook operators like gradients, divergence, curl, and boundary-conditioned wave behavior.',
        ],
        notation: ['∇·J', '∇×J', 'plane wave', 'dipole field'],
        tags: ['field-configurations', 'canonical-fields'],
    },
    'Moore Seeds (geometric)': {
        summary: 'Geometric shells of the 26-neighbor Moore neighborhood rendered as explicit seed configurations.',
        body: [
            'These scenarios visualize the discrete neighborhood geometry itself rather than a dynamical effect layered on top of it.',
            'The mathematical focus is polyhedral decomposition, shell counts, neighbor distance classes, and the gauge-structure interpretations built on that decomposition.',
        ],
        notation: ['6 + 12 + 8 = 26', 'octahedron', 'cuboctahedron', 'stella octangula'],
        tags: ['geometry', 'moore-neighborhood'],
    },
});

const SCALE0_SPECIFIC_GUIDES = Object.freeze({
    'empty': {
        summary: 'A quiet reference lattice with no deliberately injected excitation.',
        body: [
            'Use the empty lattice to calibrate your eye before reading more structured scenarios. It is the nearest thing the live engine has to a vacuum-like or zero-background initial state.',
            'The relevant math is not complicated here: you are watching the neighborhood rules act on something close to s(v,t)=0 and J(v,t)≈0, so any later structure can be compared against an intentionally quiet baseline.',
        ],
        notation: ['s(v,t) = 0', 'J(v,t) ≈ 0', 'baseline'],
    },
    'flux-pulse': {
        summary: 'A transverse native-wave packet for testing finite-box boundary algorithms.',
        body: [
            'This scenario isolates the production wave map: no matter creation, forces, damping, Gauss projection, or research extensions are active. The seed is a compact, divergence-free discrete curl traveling in +x.',
            'Periodic evolution conserves the exact kick-drift Hamiltonian. The copied ghost shell reverses flux momentum and conserves its interior modified Hamiltonian. The one-cell “dispersal” mode is only a lossy multiplier: in the fixed L=48 test it retained 52.9% of field norm at tick 90, so it is not an absorbing or radiation boundary.',
        ],
        notation: ['J = ∇_h×(ψ e_x)', 'H_h (kick–drift)', 'P_x', 'c = 1/√3'],
    },
    'flux-dipole': {
        summary: 'An antisymmetric pair of Gaussian vector-wave blobs used to test odd x-reflection parity.',
        body: [
            'Both J and W are exactly odd under x reflection at initialization, and the isolated periodic wave map preserves that parity for the qualified interval.',
            'No ternary source, electric or magnetic dipole moment, near-field law, or radiation pattern is represented.',
        ],
        notation: ['J(x) = -J(L-1-x)', 'W(x) = -W(L-1-x)', 'wave parity'],
    },
    'flux-standing': { summary: 'A reflection-even Gaussian wave pair with zero initial W. It preserves parity but is broadband, not a pure standing mode.', notation: ['J(x) = J(L-1-x)', 'W(t=0)=0', 'broadband'] },
    'flux-nested-standing': { summary: 'Two orthogonal reflection-even Gaussian pairs whose x/z parity is preserved; neither pair is a pure standing eigenmode.', notation: ['x/z reflection parity', 'orthogonal broadband pairs'] },
    'flux-soliton': { summary: 'A high-amplitude packet used to falsify soliton-like shape preservation: its width grows by a factor of 2.90 in 20 ticks.', notation: ['localized packet', 'width ratio = 2.90', 'not a soliton'] },
    'flux-interference': { summary: 'A four-lobe broadband wave field whose exact x/z reflection symmetries are tested. No detector fringe law is claimed.', notation: ['four Gaussian lobes', 'x/z reflection parity', 'no fringe gate'] },
    'flux-vortex': { summary: 'An exact inert three-plane helical-ring vector ansatz with imposed circulation and axial bias; it is not spin.', notation: ['imposed circulation', 'axial bias', 'no spin identity'] },
    'flux-dual-substrate': {
        summary: 'A mirror-polarized Gaussian wave pair. Despite the legacy ID, the dual-substrate operator is disabled.',
        body: [
            'J and W are x-even and y/z-odd under reflection between the two blobs, and the isolated native wave map preserves that mixed parity.',
            'There is only one vector field in this setup. It provides no evidence for coupled layers, handed sectors, or a dual ontology.',
        ],
        notation: ['Jx even', 'Jy/Jz odd', 'dual_substrate = OFF'],
    },
    'flux-cascade': {
        summary: 'A measured one-tick response of the selected local genesis law to a supercritical Gaussian.',
        body: [
            'With every other production term off, the canonical L=32 run creates 105 positive and 102 negative single-site events on the first tick. Independent setup and tick runs replay exactly.',
            'Every pair ID remains unset. The result validates only the selected genesis response; there is no branching, outward recruitment, cascade, or pair-production process.',
        ],
        notation: ['105 + / 102 -', 'pair IDs = 0', '[SELECTION] genesis law'],
    },
    'flux-random-genesis': {
        summary: 'A measured one-tick response of the selected local genesis law to eight fixed-seed random patches.',
        body: [
            'With every other production term off, the canonical L=32 run creates 179 positive and 168 negative single-site events on the first tick. Independent setup and tick runs replay exactly.',
            'Every pair ID remains unset. This is not vacuum pair production, an ongoing random bath, or evidence that noise self-organizes.',
        ],
        notation: ['179 + / 168 -', 'pair IDs = 0', 'fixed-seed initial patches'],
    },
    'flux-genesis-between-gates': {
        summary: 'A one-tick three-cohort discriminator for the selected local genesis threshold and hazard.',
        body: [
            'At the initial decision, three exact uniform-flux cohorts sit at |J| = 1.5160, 1.5250, and 1.5340 around the selected K_GENESIS = 3·W_SC ≈ 1.5164. Their local hazards are exactly 0, 0.0168973, and 0.034247 per site for the compiled rule.',
            'The L=24, seed-1 certification records 0, 49, and 120 genesis events on tick one. The experiment stops there conceptually: accepted genesis drains J and the master toggle also permits evaporation, so later ticks are neither frozen nor independent cohorts.',
        ],
        notation: ['K_GENESIS = 3·W_SC', 'hard gate', '1 − e^{−ΔJ/K_MANIFEST}', 'FTD-0388'],
    },
    'flux-pair-production': {
        summary: 'A one-tick cohort test of the engine’s selected adjacent polarity-pair transition.',
        body: [
            'The scenario isolates 343 +x flux sources at K_GENESIS + K_MANIFEST·ln(2), so each source has exactly p = 1/2 under the compiled rule. At L=24 and seed 1, 170 sources transition, within the preregistered six-sigma Bernoulli gate.',
            'Each accepted event makes an upstream −1 and downstream +1 state with a shared pair ID, distinct particle IDs, and cancelling signed state and vector flux. That certifies the discrete transition as implemented; it does not identify the states with physical particles or derive Schwinger pair production.',
        ],
        notation: ['p = 1/2', '−1/+1 adjacency', 'pair ID', '[SELECTION] transition law'],
    },
    'flux-annihilation': {
        summary: 'An exact two-tick test of the native rule for a moving state colliding with an opposite state.',
        body: [
            'A +1 state moving at C_SPEED crosses into an adjacent stationary −1 state on tick two. The movement branch removes both states and redistributes their pre-existing, cancelling flux over the two six-face neighbor shells.',
            'The measured field-norm ratio is exactly 1/6 and total vector flux stays zero. Wave momentum remains zero because the rule contains no rest-mass-to-field conversion and wave propagation is disabled. Read this as collision-removal bookkeeping, not a physical annihilation-radiation result.',
        ],
        notation: ['opposite-state collision', 'six-face redistribution', 'field norm × 1/6', 'no mass radiation'],
    },
    'flux-vacuum-foam': {
        summary: 'A finite fixed-seed random wave ball with exact source-free replay.',
        body: [
            'The random vectors are sampled once during setup. After that, only the isolated linear wave map runs: there is no ongoing noise, reaction, manifestation, or thermostat.',
            'Independent C++ dispatches and 12-tick evolutions agree bit for bit, and the periodic modified Hamiltonian is conserved below 1e-12. This is not a quantum-vacuum, virtual-particle, or spacetime-foam model.',
        ],
        notation: ['fixed-seed J,W', 'exact replay', 'source-free modified-H conservation'],
    },
    'flux-zero-point': {
        summary: 'A quiescent vacuum carrying an irreducible, sub-threshold fluctuation floor — energy that stays even when nothing is manifested.',
        body: [
            'Zero-Point Energy here is a pedagogical lattice illustration, not a derivation of the QFT ½ℏω vacuum energy: the flux field is seeded with uniform low-amplitude (~0.3·K_B) random fluctuations everywhere, roughly 20× below the genesis threshold, with genesis and damping both OFF.',
            'Because nothing damps it, the energy-conserving wave dynamics keep the field jittering — the energy-audit and Lagrangian-density overlays show a persistent non-zero floor that never relaxes to exactly zero, and (unlike Vacuum Fluctuations) it never crosses the manifestation threshold. Turn damping on to watch the floor decay; push the amplitude toward foam levels to see it start producing pairs.',
        ],
        notation: ['ground-state floor', 'sub-threshold fluctuation', 'no damping → persistent'],
    },
    'flux-meson': { summary: 'Two opposite ternary states with counter-directed velocities under movement-only dynamics. The field blobs are inert and no meson or confinement physics is present.', notation: ['vy = ±0.05', 'remainder transport', 'no confinement'] },
    'flux-string-breaking': { summary: 'A movement-only control: two opposite states separate outward with exact remainder bookkeeping. No string, tension, confinement, color, or pair-production mechanism exists in this profile.', notation: ['vx = -/+0.3', 'one face after 4 ticks', 'state count = 2'] },
    'flux-baryon': { summary: 'A movement-only threefold tangential transport seed plus one stationary opposite marker. It contains no binding, color, quark, or baryon dynamics.', notation: ['three tangential + markers', 'one stationary - marker', 'all unlocked'] },
    'flux-cyclotron': { summary: 'A controlled native-response test: an imposed vector potential with Bz=1 bends a mobile positive polarity through the selected alpha*s*(v cross curl J) term, while the no-Lorentz control stays straight. It does not derive magnetism.', notation: ['Bz = 1 (imposed)', 'vy(80) = -0.111379', 'speed drift = 1.224%', 'no-Lorentz control: y = constant'] },
    'flux-screening': { summary: 'An exact inert octahedral polarity shell: one central + state, six axis-aligned - states, and a separately imposed radial dressing. Its net state is -5, so it is not a screening or neutralization result.', notation: ['1 + 6 face orbit', 'Σs = -5', 'imposed radial J'] },
    'flux-thermalization': { summary: 'A fixed-seed compact random J/W patch spreading under the isolated linear wave map. This measures deterministic dephasing and support growth, not thermodynamic thermalization.', notation: ['outside-energy fraction at t=12: 0.0490', 'modified-H drift: 2.08e-14', 'no thermostat'] },
    'flux-triad': { summary: 'An exact inert threefold + polarity seed with imposed inward flux dressing. No binding term is active, so it demonstrates prepared geometry rather than triad formation or stability.', notation: ['three + states', '120-degree seed', 'binding off'] },
    'light-two-slit': { summary: 'Two classical transverse sources with exact native superposition. There is no barrier or particle path, and the fixed constructive-contrast gate fails.', notation: ['superposition residual 6.77e-16', 'constructive 0.0394 < 0.05', 'not quantum interference'] },
    'light-dipole': { summary: 'Radiation from a driven dipole-like source configuration.', notation: ['dipole radiation', 'far-field pattern'] },
    'light-rainbow': { summary: 'A color-separation or spectral-spread demonstration emphasizing wavelength dependence.', notation: ['λ', 'dispersion'] },
    'light-photon-race': { summary: 'A comparative propagation setup for pulse timing and path behavior.', notation: ['c = 1/√3', 'travel time'] },
    'quantum-born-rule': {
        summary: 'A fixed Gaussian J/W envelope passed once through the selected local genesis rule. It produces a deterministic finite cohort, not a Born-law measurement.',
        body: [
            'At L=32 with genesis as the only active term, one tick produces exactly 36 manifested sites and zero pair IDs.',
            'Two independent runs replay bit-exactly because both orientation and engine seed are fixed.',
            'There is no ensemble histogram, wave function, normalization rule, collapse operation, or comparison against P proportional to |J| squared.',
        ],
        notation: ['N_manifested(1) = 36', 'pair IDs = 0', 'genesis only'],
    },
    'quantum-double-slit': {
        summary: 'Two coherent classical lattice sources evolved under the isolated wave map. The field is exactly linear, but the fixed screen shows no destructive band, so the double-slit fringe interpretation fails.',
        body: [
            'At L=48 and tick 20, combined evolution agrees with the sum of separately evolved sources to relative residual 7.74e-16.',
            'The screen cross term is constructive with fraction 0.461954, while the destructive fraction is zero.',
            'There is no material barrier, slit boundary, single-particle impact distribution, measurement rule, or quantum-interference result.',
        ],
        notation: ['superposition residual = 7.74e-16', 'constructive = 0.461954', 'destructive = 0'],
    },
    'quantum-eraser': {
        summary: 'Two classical wave packets encounter one locked checkerboard state plane through the native coupling term. The grid strongly sources the field; it does not erase which-way information.',
        body: [
            'At L=32 and tick 28, downstream energy is 754.473 with the checkerboard and 49.2727 in the no-grid control, a ratio of 15.3122.',
            'The relative full-field difference is 4.58949, establishing a real native coupling response to the locked states.',
            'There is no which-way record, polarization basis, projection, delayed choice, measurement outcome, or erasure operation.',
        ],
        notation: ['E_downstream grid/control = 15.3122', 'field difference = 4.58949', '512 locked states'],
    },
    'quantum-tunnel': {
        summary: 'A transverse packet encountering three locked full state planes through the native coupling term. The planes amplify the field dramatically instead of acting as a tunneling barrier.',
        body: [
            'At L=32 and tick 28, right-side energy is 14091.6 with the wall and 0.909727 in the no-wall control, a ratio of 15489.9.',
            'The relative full-field difference is 81.6898. The locked states act as coupling sources, not a forbidden potential region.',
            'No barrier height, evanescent pole, exponential width law, incident/reflected flux normalization, or Schrödinger dynamics is implemented.',
        ],
        notation: ['E_right wall/control = 15489.9', 'field difference = 81.6898', 'three locked source planes'],
    },
    'quantum-well': {
        summary: 'A broadband superposition initialized between two locked state planes. The planes are inert markers and impose no wave boundary condition.',
        body: [
            'At L=32, a control with both marker planes removed evolves bit-identically to the marked scenario for eight ticks.',
            'By tick eight, 32.5541% of the instantaneous field energy lies outside the marked interval.',
            'The n=1 through 8 sine components are imposed initial data. No boundary spectrum, n-squared energy law, or particle-in-a-box mechanism is derived.',
        ],
        notation: ['marker effect = 0 exactly', 'outside energy(8) = 32.5541%', 'modes n = 1...8 imposed'],
    },
    'quantum-entangle': {
        summary: 'A correlation scenario for paired outcomes and distance-dependent joint statistics.',
        body: [
            'Entanglement Correlation is about relational structure between two outputs rather than a property assigned independently to each one.',
            'The main math is the correlation function and how it behaves with separation, basis choice, or decoherence length.',
        ],
        notation: ['C(d)', 'correlation', 'pair statistics'],
    },
    'quantum-aharonov-bohm': {
        summary: 'A central flux tube and two off-axis wave packets initialized in the same linear field. The tube and paths evolve independently and merely add.',
        body: [
            'The initial field decomposes exactly into a central tube and two path packets.',
            'After 12 ticks, the full evolution differs from the separately evolved components by relative residual 6.13e-16; normalized divergence is 9.06e-17.',
            'No link variable, gauge-invariant holonomy, enclosed-flux phase, recombination estimator, or path-tube interaction is implemented.',
        ],
        notation: ['superposition residual = 6.13e-16', 'divergence = 9.06e-17', 'phase observable absent'],
    },
    'quantum-casimir': {
        summary: 'A reproducible transverse lattice eigenmode crossing two locked marker planes. The plates have exactly zero effect on the field.',
        body: [
            'At L=32, the marked run and an otherwise identical no-plate control remain bit-identical after 12 ticks.',
            'The two locked 32 by 32 planes remain static and acquire no force or motion.',
            'There is no vacuum ensemble, inside/outside mode subtraction, separation sweep, boundary-condition operator, or Casimir-force estimator.',
        ],
        notation: ['plate effect(12) = 0 exactly', 'two inert 32x32 planes', 'force estimator absent'],
    },
    'quantum-zeno': {
        summary: 'A supercritical fixed J/W envelope passed once through the selected genesis rule. The engine contains no observation intervention, so this is not a quantum-Zeno test.',
        body: [
            'At L=32 with genesis as the only active term, one tick produces exactly 491 manifested sites and zero pair IDs.',
            'The result replays bit-exactly under the fixed seed.',
            'There is no measurement operator, measurement cadence, survival-probability comparison, or suppression arm.',
        ],
        notation: ['N_manifested(1) = 491', 'pair IDs = 0', 'measurement absent'],
    },
    // Audit 2026-04-28 removals: s0-seed-{electron-l3, neutrino, quark, antiquark}.
    // Audit-4 2026-04-28: KB entries for s0-seed-{positron, pion, proton-l4, neutron}
    // removed — these scenarios moved to s0-vacuum-* canonical entries.
    's0-seed-hydrogen': { summary: 'A prepared locked three-site source plus one mobile negative marker. The four sites persist for 64 ticks under Poisson force and movement, but no spectrum, binding energy, orbital pole, or emergent proton is demonstrated.', notation: ['3 locked + 1 mobile', 'N64 = 4', 'hydrogen identity absent'] },
    's0-seed-helium': { summary: 'A prepared 12-locked plus 2-mobile Coulomb cohort. Its signed state is -2 rather than neutral, so the helium interpretation fails even though all 14 sites persist for 64 ticks.', notation: ['12 locked + 2 mobile', 'Q = -2', 'neutral helium failed'] },
    's0-seed-h2-bond-formation': { summary: 'Two prepared locked three-site sources with two central mobile negative markers. The mobile pair is gone by tick 64, so the setup does not form an H2 bond.', notation: ['6 locked + 2 mobile', 'mobile N64 = 0', 'bond formation failed'] },
    's0-seed-wilson-loop': { summary: 'An exact inert oriented square in J. It computes no link holonomy, trace, Wilson observable, or area law.', notation: ['oriented square path', 'zero vector sum', 'no Wilson observable'] },
    's0-seed-flux-tube': { summary: 'An imposed Gaussian axial vector profile between two opposite ternary endpoint markers; confinement is not tested.', notation: ['Gaussian tube ansatz', 'neutral endpoints', 'no confinement law'] },
    's0-seed-monopole': { summary: 'An exact radial inverse-square vector profile. It is monopole-shaped initial data, not evidence for magnetic charge.', notation: ['radial J', 'r²|J| = 1/(4π)', 'imposed ansatz'] },
    's0-seed-instanton': { summary: 'A localized radial 3-vector profile whose instanton interpretation is rejected: it has no Euclidean time, non-Abelian connection, or topological charge.', notation: ['radial 3-vector', 'not an instanton', 'closed negative'] },
    's0-seed-schwarzschild': { summary: 'A gravity-facing well configuration for central-potential and horizon intuition.', notation: ['Φ(r)', '\\(r_s\\)'] },
    's0-seed-gravitational-wave': { summary: 'A wave-like spacetime-analogue configuration for strain and propagation intuition.', notation: ['h_μν', 'wave strain'] },
    's0-seed-sloop': { summary: 'An exact inert 12-site tangential ring with zero net vector flux; no self-reference or consciousness behavior is tested.', notation: ['12-site ring', '|J| = K_B', 'zero vector sum'] },
    's0-seed-observer-cell': { summary: 'An exact inert 1+6+12+8 Moore-shell pattern with imposed alternating ternary labels; it is not an observer model.', notation: ['3³ Moore cell', 'imposed labels', 'no observer claim'] },
    's0-field-plane-wave': { summary: 'A canonical plane-wave initial condition for reading wavelength, phase, and travel direction.', notation: ['A exp(i(kx-ωt))', 'k', 'ω'] },
    's0-field-standing-wave': { summary: 'A boundary-compatible field mode with fixed nodes and antinodes.', notation: ['sin(kx) cos(ωt)', 'nodes'] },
    's0-field-uniform-e': { summary: 'An exact inert uniform canonical-momentum W field used as an E proxy; it is not a sourced electromagnetic solution.', notation: ['uniform W', 'all dynamics off', 'E proxy'] },
    's0-field-uniform-b': { summary: 'An exact inert vector-potential ansatz whose interior discrete curl is uniform along z; finite-face behavior is not certified.', notation: ['curl J = (0,0,0.05)', 'interior only', 'B proxy'] },
    's0-field-photon-pulse': { summary: 'A transverse packet whose photon gate failed: its centroid is too slow and its width grows too much under the current seed.', notation: ['speed = 0.462', 'width ratio = 1.646', 'closed negative'] },
    's0-field-rf-lattice-wave': {
        summary: 'The selected n=1 transverse lattice mode, measured only in lattice units.',
        body: [
            'This scenario isolates the lowest selected spatial harmonic so its discrete-time pole can be checked directly.',
            'RF is only a legacy mnemonic. There is no mapping to SI radio frequency; the dashboard reports the exact kick-drift dispersion in lattice units.',
            'Matter, forces, genesis, damping, and Gauss projection are disabled so the wave readout stays clean and interpretable as a flux-mode instrument.',
        ],
        notation: ['mode n = 1', 'lambda = L', 'c = 1/sqrt(3)', 'transverse Jy/Wy'],
    },
    's0-field-light-lattice-wave': {
        summary: 'The selected n=6 transverse lattice mode, measured only in lattice units.',
        body: [
            'This scenario raises the spatial mode number so lattice dispersion can be compared against n=1.',
            'Light is only a legacy mnemonic. The setup has no SI optical-frequency, color, or photon calibration.',
        ],
        notation: ['mode n = 6', 'lambda = L/6', 'c = 1/sqrt(3)', 'transverse Jy/Wy'],
    },
    's0-field-sound-lattice-wave': {
        summary: 'A longitudinal n=4 seed that closes the proposed c/8 sound-speed interpretation for the frozen wave sector.',
        body: [
            'The initial W amplitude is seeded with a c/8 proxy, but the production operator re-propagates the mode at the same native pole as every vector component.',
            'The native recurrence fits to numerical precision while the c/8 recurrence has normalized residual 0.0801. No acoustic medium, density, or elastic modulus exists in this sector.',
            'The scenario therefore remains useful as a negative control, not as a sound simulation.',
        ],
        notation: ['mode n = 4', 'sound proxy v/c = 1/8', 'longitudinal Jx/Wx'],
    },
    's0-field-sound-collision': {
        summary: 'Two counter-seeded longitudinal packets overlap under the isolated native wave map. Their joint evolution is pointwise linear, so no acoustic collision or medium interaction is present.',
        body: [
            'The two declared lanes are reconstructed independently before evolution, avoiding any fitted post-hoc separation.',
            'At tick 20 their normalized spatial overlap is 0.804443, while the combined-minus-summed field residual is 6.02e-16.',
            'The setup is useful as a wave-linearity control. It is not a model of sound because the frozen sector has no density, pressure, elastic modulus, or separate acoustic pole.',
        ],
        notation: ['longitudinal Jx/Wx', 'overlap(20) = 0.804443', 'superposition residual = 6.02e-16'],
    },
    's0-field-spacetime-forcing-boundary': {
        summary: 'A production-wave point response whose exact support advances by no more than one lattice neighborhood per tick.',
        body: [
            'The scenario disables every non-wave phase and uses the periodic production kick-drift map. At tick eight its support reaches Chebyshev radius eight without appearing outside that cone.',
            'The exact modified Hamiltonian drifts by only 6e-15 in the native test. This qualifies locality and the selected wave integrator, not Lorentz invariance or a spacetime metric.',
            'The first-order diffusion comparison in the legacy demo is not an engine phase; it remains an explicitly counterfactual calculation.',
        ],
        notation: ['d2J/dt2 = c^2 nabla^2 J', 'dJ/dt = D nabla^2 J (counterfactual)', 'c = 1/sqrt(3)'],
    },
    's0-field-electric-dipole': { summary: 'An imposed softened opposite-source Coulomb-shaped flux profile with two opposite ternary markers; EM emergence is not tested.', notation: ['selected r/(r²+1)^(3/2)', 'neutral markers', 'imposed'] },
    's0-field-magnetic-dipole': { summary: 'An imposed softened dipole vector-potential ansatz A proportional to z-hat cross r; magnetism is not derived.', notation: ['selected vector potential', 'azimuthal A', 'imposed'] },
    's0-field-vortex-line': { summary: 'An exact inert azimuthal inverse-radius vector profile; no fluid, EM, or quantized-vortex identity is established.', notation: ['J tangential', 'r|J| constant', 'imposed'] },
    's0-field-thomson-scattering': { summary: 'A locked negative source plus an exact lattice plane wave. The four-arm campaign finds only linear superposition, no scattering residual and no recoil.', notation: ['max residual 3.86e-16', 'velocity = 0', 'closed negative'] },
    's0-field-thomson-unlocked-recoil': { summary: 'A mobile negative-polarity site responds deterministically to an exact lattice plane wave through the selected native flux-gradient force extension. This is not a physical-electron or Thomson-scattering result.', notation: ['beam-induced |Δx| = 0.1697027 at t=200', 'repeat residual = 0', 'legacy force response ≈ machine noise'] },
    's0-seed-thermal-ignition': {
        summary: 'An initially empty lattice driven by the selected fixed-seed Langevin bath at T=0.03 with genesis enabled. It produces field excitation but no manifested sites in the qualified finite run, so the legacy ignition/condensation name is rejected at this point.',
        body: [
            'At L=16, gamma=0.02, and 100 ticks, two independent CPU runs replay bit-exactly.',
            'The measured field excitation is 1000.82 while the manifested count remains 0/4096.',
            'This is one finite-volume, finite-time bath response. It does not establish a critical temperature, first-order transition, hysteresis, or thermodynamic condensation.',
        ],
        notation: ['T = 0.03 (imposed)', 'gamma = 0.02', 'N_manifested(100) = 0/4096'],
    },
    's0-seed-emergent-ic1': { summary: 'Axial A=10 genesis response: 3 manifested sites at ticks 100 and 120 on L=24, not the advertised 25-site octahedron.', notation: ['A=10', 'T=0.005', 'N100/N120=3/3'] },
    's0-seed-emergent-ic3-collision': { summary: 'Two separated opposite A=5 seeds produce exactly 2 manifested sites at ticks 100 and 120. Two multi-site collision products are not observed.', notation: ['two A=5 seeds', 'N100/N120=2/2', 'collision-product gate failed'] },
    's0-seed-emergent-ic4-subthreshold': { summary: 'A central 0.5*K_GENESIS injection in the T=0.005 bath remains at zero manifested sites through tick 120.', notation: ['A=0.5', 'N100/N120=0/0', 'negative control'] },
    's0-seed-emergent-ic2-thermal-runaway': { summary: 'An empty T=0.05 Langevin/genesis bath remains at zero manifested sites through tick 120 on L=24, closing runaway for this finite run.', notation: ['T=0.05', 'N100/N120=0/0', 'runaway gate failed'] },
    's0-seed-emergent-ic1-diagonal': { summary: 'A body-diagonal A=10 injection produces one manifested site at ticks 100 and 120 on L=24. No Z3 efficiency law is inferred.', notation: ['body diagonal', 'A=10', 'N100/N120=1/1'] },
    's0-seed-emergent-ic1-isotropic': { summary: 'A six-axis A=10 injection produces eight manifested sites at ticks 100 and 120 on L=24. No O_h bound-state identity is inferred.', notation: ['six-axis seed', 'A=10', 'N100/N120=8/8'] },
    's0-seed-emergent-ic1-viz': { summary: 'The axial A=20, T=0 response is deterministic but decays from 22 to 20 manifested sites between ticks 100 and 120.', notation: ['A=20', 'T=0', 'N100/N120=22/20'] },
    's0-seed-emergent-ic1-diagonal-viz': { summary: 'The body-diagonal A=20, T=0 response is deterministic but decays from 22 to 20 manifested sites between ticks 100 and 120.', notation: ['A=20 diagonal', 'T=0', 'N100/N120=22/20'] },
    's0-seed-emergent-ic1-isotropic-viz': { summary: 'The six-axis A=20, T=0 response is deterministic but decays from 20 to 18 manifested sites between ticks 100 and 120.', notation: ['A=20 six-axis', 'T=0', 'N100/N120=20/18'] },
    's0-seed-cluster-law': { summary: 'The interactive genesis probe is qualified only at its default A=10 point: 3 manifested sites at ticks 100 and 120 on L=24. Changing the amplitude starts a new, unqualified experiment.', notation: ['A=10 default', 'T=0.005', 'N100/N120=3/3'] },
    's0-vacuum-muon-neutrino': { summary: 'A neutral native wave packet imposed at exactly 1.3 times the base amplitude. It follows exactly the same linear trajectory, so the legacy muon-neutrino flavor interpretation is absent.', notation: ['amplitude = 1.3x', 'same centroid', 'flavor gate failed'] },
    's0-vacuum-tau-neutrino': { summary: 'A neutral native wave packet imposed at exactly 1.6 times the base amplitude. It follows exactly the same linear trajectory, so the legacy tau-neutrino flavor interpretation is absent.', notation: ['amplitude = 1.6x', 'same centroid', 'flavor gate failed'] },
    's0-seed-de-broglie-clock': { summary: 'A 7^3 manifested block exercising the optional Klein–Gordon restoring term at imposed omega0=0.30. The operator produces the exact -omega0^2 J kick; FTD does not derive the frequency or a pilot-wave guidance force.', notation: ['delta W = -omega0^2 J', 'omega0 = 0.30 imposed', 'guidance absent'] },
    's0-seed-gravitational-wave': { summary: 'An exact n=4 transverse eigenmode of the native wave map. It contains no metric, tensor polarization, mass source, or gravity-specific operator, so the gravitational-wave identity is rejected.', notation: ['n=4', 'A=0.1', 'plain wave only'] },
    's0-seed-time-gravity-well': { summary: 'A bit-identical alias of the exact n=4 wave. It has no latency well or proper-time observable; the legacy gravity-well interpretation is closed negative.', notation: ['exact alias', 'no latency', 'no clock'] },
    's0-seed-time-twin-clocks': { summary: 'A bit-identical alias of the exact n=4 wave. It contains no pair of clocks or worldlines and computes no proper-time difference.', notation: ['exact alias', 'no observers', 'no delta-tau'] },
    's0-seed-schwarzschild': { summary: 'An imposed inward inverse-square vector profile J=-3 G_N K_B r/r^3 with one central marker. It is inert and computes no metric, curvature, latency, or Schwarzschild solution.', notation: ['1/r^2 magnitude', 'all terms off', 'Schwarzschild identity failed'] },
    's0-seed-time-horizon': { summary: 'A bit-identical alias of the inert inverse-square ansatz. No horizon, latency field, clock, or proper-time observable is present.', notation: ['exact alias', 'no null surface', 'no clock'] },
    's0-seed-gravitational-lensing': { summary: 'A radial background and transverse packet evolved by the isolated linear wave map. The packet has a small intrinsic centroid drift, but exact superposition proves the radial background adds no deflection, closing native lensing for this setup.', notation: ['superposition residual < 1e-12', 'intrinsic y: 32.0000 to 31.8889', 'induced delta-y = 0'] },
    's0-seed-up-quark': { summary: 'The legacy up-quark entry is an imposed A=0.5 positive/red-labelled vector template. Its marker metadata do not couple to the isolated wave map, so no quark or fractional-charge identity is present.', notation: ['A=0.5', 'positive/red label', 'quark identity rejected'] },
    's0-seed-down-quark': { summary: 'The legacy down-quark entry is an imposed A=0.5 negative/green-labelled vector template. Its marker metadata do not couple to the isolated wave map.', notation: ['A=0.5', 'negative/green label', 'quark identity rejected'] },
    's0-seed-strange-quark': { summary: 'The legacy strange-quark entry is an imposed A=0.7 negative/blue-labelled vector template. It differs from the cohort only by selected metadata and amplitude.', notation: ['A=0.7', 'negative/blue label', 'flavor identity rejected'] },
    's0-seed-charm-quark': { summary: 'The legacy charm-quark entry is an imposed A=1.0 positive/red-labelled vector template. It contains no mass or generation operator.', notation: ['A=1.0', 'positive/red label', 'flavor identity rejected'] },
    's0-seed-bottom-quark': { summary: 'The legacy bottom-quark entry is an imposed A=1.4 negative/green-labelled vector template. Its larger amplitude is an input, not an emergent mass hierarchy.', notation: ['A=1.4', 'negative/green label', 'mass hierarchy absent'] },
    's0-seed-top-quark': { summary: 'The legacy top-quark entry is an imposed A=2.5 positive/blue-labelled vector template. Its large amplitude is selected and does not encode a top mass or Higgs coupling.', notation: ['A=2.5', 'positive/blue label', 'top identity rejected'] },
    's0-seed-higgs-field': { summary: 'A deterministic volume-filling three-vector background evolved by the source-free wave map. It has no scalar degree of freedom, Higgs potential, symmetry breaking, or VEV observable.', notation: ['three-vector J', 'modified-H conservation', 'scalar/VEV identity rejected'] },
    's0-seed-gluon': { summary: 'A selected mixed-polarization vector packet. With no color substrate, gauge connection, self-coupling, or color observable, the gluon identity is rejected.', notation: ['vector packet', 'wave only', 'gluon identity rejected'] },
    's0-vacuum-electron': { summary: 'One inert negative marker plus a selected radial vector wave. It has no charge coupling, mass pole, spinor, or electron-identifying observable.', notation: ['negative marker', 'radial J', 'electron identity rejected'] },
    's0-vacuum-muon': { summary: 'An exact 1.2-times amplitude copy of the electron-labelled radial vector template. Normalized evolution is identical, so no muon generation or mass distinction is present.', notation: ['amplitude = 1.2x', 'same normalized trajectory', 'muon identity rejected'] },
    's0-vacuum-tau': { summary: 'An exact 1.5-times amplitude copy of the electron-labelled radial vector template. Normalized evolution is identical, so no tau generation or mass distinction is present.', notation: ['amplitude = 1.5x', 'same normalized trajectory', 'tau identity rejected'] },
    's0-vacuum-w-boson': { summary: 'One inert positive marker plus an anisotropic vector wave. It has no weak charge, mass pole, polarization representation, or W-boson observable.', notation: ['positive marker', 'anisotropic J', 'W identity rejected'] },
    's0-vacuum-z-boson': { summary: 'An unmanifested inward radial vector wave. It has no neutral current, weak coupling, mass pole, or Z-boson observable.', notation: ['radial J', 'no state core', 'Z identity rejected'] },
    's0-vacuum-higgs': { summary: 'An unmanifested equal-component three-vector blob. It is not a scalar field and contains no Higgs potential, mass pole, symmetry breaking, or decay observable.', notation: ['Jx=Jy=Jz', 'three-vector', 'scalar Higgs identity rejected'] },
    's0-vacuum-proton': { summary: 'An unlocked three-site selected-color candidate. Under only static-dressing force, color force, and movement it has 3/1/0 sites at ticks 8/16/32, so the bound proton interpretation fails.', notation: ['N8/N16/N32 = 3/1/0', 'no locks', 'proton stability failed'] },
    's0-vacuum-neutron': { summary: 'The alternate-polarity three-site candidate falls to one site at tick 8 and zero by tick 64. No bound neutron mode survives.', notation: ['N8/N32/N64 = 1/1/0', 'selected color force', 'neutron stability failed'] },
    's0-vacuum-pion-charged': { summary: 'An unlocked opposite-polarity selected-color pair. Both sites are removed by tick 8, so it does not form a bound charged pion.', notation: ['N0/N8 = 2/0', 'collision removal', 'binding failed'] },
    's0-vacuum-pion-neutral': { summary: 'A bit-identical alias of the charged-pion-labelled pair, initially and through tick 16. No neutral-specific degree of freedom exists, and both sites are gone by tick 8.', notation: ['exact alias', 'N8 = 0', 'neutral distinction absent'] },
    's0-vacuum-kaon-charged': { summary: 'The same unlocked pair with an imposed 1.88 dressing boost. Both sites are still removed by tick 8, so the boost does not produce a bound kaon.', notation: ['dressing = 1.88x', 'N8 = 0', 'kaon binding failed'] },
    's0-seed-ee-annihilation': { summary: 'A long-baseline opposite-polarity collision using only native movement. At L=24 both states disappear exactly at tick 24, but no wave momentum or photon pair is created; only pre-existing dressing is redistributed.', notation: ['collision tick = 24', 'N: 2 to 0', '|W|² = 0'] },
    's0-seed-ew-phase-transition': { summary: 'An empty lattice under a nonnegative uniform additive +x drive with genesis enabled. It produces 2068 manifested sites at tick 64 on L=16, but the drive never reverses, so no down-sweep, hysteresis loop, or electroweak transition is tested.', notation: ['D(t)=(sin(0.01t)+1)*0.025', 'N64=2068', 'no down-sweep'] },
    's0-seed-beta-decay': { summary: 'A prepared weak-stress ramp, not a decay. The alleged electron marker and neutral packet are already present at tick zero; seven polarity flips occur from tick 54 through 64, but no site is created or emitted.', notation: ['products preseeded', 'first flip t=54', 'N: 4 to 4'] },
    's0-seed-quark-gluon-plasma': { summary: 'A fixed-seed T=0.02 Langevin vector bath carrying eight freely moving, inertly color-labelled markers. Color force is off and seven markers leave the open particle boundary by tick 64; no QGP or deconfinement observable is present.', notation: ['N8/N32/N64 = 8/8/1', '145 movement events', '0 annihilations'] },
    's0-seed-spark-of-life': { summary: 'A prepared patterned genesis-response cohort. It produces exactly six genesis events by tick 8 and then remains at 33 sites through tick 32, with no turnover or further growth. No chemistry, metabolism, heredity, replication, or autocatalysis is implemented.', notation: ['N1/N8/N32 = 27/33/33', '6 genesis events', 'life identity rejected'] },
});

const PARTICLE_SCENARIO_GUIDES = Object.freeze({
    'pe-hydrogen': 'Hydrogen is the baseline particle-engine atom: one attractive Coulomb channel, one reduced mass, and the cleanest route to comparing orbital size and binding scale.',
    'pe-hydrogen-fine': 'Hydrogen with magnetic dipole and spin-orbit forces enabled and tilted initial spin axes — spin arrows precess in the partner B-field (classical Larmor-style, not QM spinors).',
    'pe-helium': 'Helium is a Coulomb many-body demo: one composite Z=2 nucleus plus two mobile electrons at ±r. Orbit speeds are recomputed after both electrons are placed so e–e repulsion is included in the IC.',
    'pe-positronium': 'Positronium is valuable because the two constituents have equal mass. That changes the reduced mass and makes the center-of-mass problem unusually symmetric.',
    'pe-muonium': 'Muonium keeps the hydrogenic charge pattern but changes the mass hierarchy, which means orbit size, timescale, and spectral scale all shift through μ.',
    'pe-true-muonium': 'True muonium is a heavy equal-mass lepton pair, so the main lesson is how binding tightens as the constituent mass scale rises.',
    'pe-tauonium': 'Tauonium is so heavy and short-lived that the main math question is not just binding but whether the dynamical timescale competes with decay and relativistic effects.',
    'pe-tau-atom': 'Tauonic hydrogen (\\(\\tau^-\\)) shrinks the hydrogenic orbit dramatically because the orbiting lepton is much heavier than the electron.',
    'pe-pionic-hydrogen': 'Pionic hydrogen is shown here as a Coulomb reduced-mass baseline. Hadronic level shifts are not active unless a short-range force term is explicitly enabled.',
    'pe-kaonic-hydrogen': 'Kaonic hydrogen is shown here as a heavier Coulomb reduced-mass baseline. Short-range kaon-nucleon physics is not implied by the default scenario.',
    'pe-sigma-plus-atom': 'This is a \\(\\Sigma^+\\)-electron bound-state thought experiment where the core has a different charge/mass identity from a proton while the mathematics still starts from a central attractive potential.',
    'pe-antiprotonic-hydrogen': 'Protonium is a Coulomb matter-antimatter pair in this engine. The contact annihilation rule is geometric; detailed hadronic inelastic channels are not modeled.',
    'pe-pion-orbit': 'Pionium is a mesonic analogue of positronium in this engine: an opposite-charge pair with pion masses. It is a softened Coulomb baseline, not a decay-channel model.',
    'pe-kaon-pair': 'Kaonium is the heavier cousin of pionium, useful for comparing how bound-state scale changes with constituent mass.',
    'pe-delta-system': '\\(\\Delta^{++}\\) plus two electrons is a high-charge many-body balance problem where attraction to the center competes with strong electron-electron repulsion.',
    'pe-omega-scattering': '\\(\\Omega^-\\) scattering is primarily a kinematics lesson: impact parameter, momentum transfer, and deflection are more important than long-lived binding.',
    'pe-deuteron': 'Deuteron is a composition-and-inertia display at Scale 1: a locked proton-neutron core plus an electron. It is not a nuclear binding calculation by default.',
    'pe-tritium': 'Tritium adds neutral ballast to the locked core, so the visible lesson is isotope composition plus Coulomb electron motion, not beta decay or nuclear cohesion.',
    'pe-helion': 'Helion combines a locked 2p+n core with two electrons. Read it as an atomic presentation of nuclear composition, not an ab initio helium-3 nucleus.',
    'pe-w-pair': 'The W-pair scenario is a charged massive-particle pair with relativistic corrections enabled when the backend supports them; it is not a weak-interaction event generator.',
    'pe-scattering': 'This is the clean Rutherford-style comparison case: incoming particle, impact parameter, deflection angle, and momentum transfer.',
    'pe-three-body': 'Three-body particle dynamics is where intuition starts to break. Small changes in initial condition can radically change whether the system binds, scatters, or reconfigures.',
    'pe-meson-scattering': 'Meson scattering asks how a light hadronic projectile transfers momentum and bends off a proton target.',
    'pe-muon-scattering': 'Muon scattering is a nice comparison to electron scattering because the projectile is heavier and less easily deflected.',
    'pe-micro-bh': 'Micro black hole is a gravity-dominant Newtonian toy in the particle engine. The event horizon and emission cadence are visual/pedagogical choices, not a GR solver.',
    'pe-custom': 'Custom particle mode is a sandbox for testing your own mass, charge, and geometry assumptions against the same interaction rules.',
});

const ATOM_SCENARIO_GUIDES = Object.freeze({
    'ae-hydrogen-atom': 'Hydrogen is the central-potential baseline for the atom engine and the cleanest place to compare orbital intuition, cloud display, and one-center attraction.',
    'ae-rutherford-scattering': 'Rutherford scattering is about large-angle deflection from a compact charged center, so treat it as a geometry-and-impact-parameter problem.',
    'ae-he-cluster': 'Helium clustering is a weak-binding problem dominated by van der Waals attraction and excluded-volume repulsion, not strong covalent directionality.',
    'ae-ar-cluster': 'Argon makes the same noble-gas story visually stronger because dispersion attraction is deeper and the cluster compacts more readily.',
    'ae-noble-mix': 'The noble mix scenario is about species-dependent σ and ε values: same broad force law, different preferred spacing and clustering depth.',
    'ae-nacl-form': 'NaCl formation is the textbook ionic case: opposite charges attract, a preferred separation appears, and the bond is governed mainly by electrostatic balance.',
    'ae-nacl-lattice': 'NaCl lattice extends ionic bonding into periodic packing, so lattice energy and coordination become the right language.',
    'ae-mgf2': 'MgF₂ is a stoichiometry lesson as much as a force lesson: total charge balance determines the preferred assembly pattern.',
    'ae-h2-form': 'H₂ formation is the simplest covalent-bonding case, where bond length and spring-like stabilization are the main quantities to watch.',
    'ae-o2-form': 'O₂ formation pushes beyond the minimal H₂ picture and invites discussion of stronger bonding and molecular stability.',
    'ae-ch4-form': 'CH₄ is the tetrahedral geometry showcase, so symmetry and bond-angle stabilization matter as much as raw radial attraction.',
    'ae-water-dimer': 'The water dimer is the entry point for hydrogen bonding, dipole alignment, and directional intermolecular preference.',
    'ae-water-cluster': 'Water clusters quickly turn into network problems: local H-bond rules create global geometry.',
    'ae-vsepr-linear': 'The CO₂ case shows how repulsion geometry can favor a 180° arrangement even when the molecule is built from more than two atoms.',
    'ae-vsepr-tetrahedral': 'CH₄ tetrahedral is the classic 109.5° geometry lesson.',
    'ae-vsepr-bent': 'H₂O bent geometry is the standard “lone pairs change the angle” teaching case.',
    'ae-thermal-gas': 'Thermal gas is about ensemble behavior, temperature control, and whether kinetic agitation overwhelms short-range ordering.',
    'ae-collision': 'Head-on collision is the atom-engine momentum-conservation demo.',
    'ae-fe-bcc': 'Fe BCC is a packing-and-coordination scenario where geometry matters as much as pair potential.',
    'ae-cu-fcc': 'Cu FCC is the close-packed comparison case to BCC iron.',
    'ae-periodic': 'Periodic Table mode is a parameter atlas rather than one fixed simulation; the lesson is periodic trends, valence, and how element identity changes force-relevant quantities.',
    'ae-custom': 'Custom atom mode lets you test your own composition, force toggles, and geometry under the same atom-engine rules.',
});

const PLANETARY_SCENARIO_GUIDES = Object.freeze({
    'planetary-solar': 'This is the standard many-body orbital classroom: angular momentum, orbital period, eccentricity, and long-term stability around a dominant central mass.',
    'planetary-binary': 'Binary stars force you to reason in barycentric coordinates instead of pretending one body is fixed.',
    'planetary-threebody': 'The three-body problem is where perturbation, resonance, and sensitivity overtake closed-form ellipse intuition.',
    'exo-TRAPPIST-1': 'TRAPPIST-1 is especially good for resonance-chain intuition because the compact architecture makes period ratios and dynamical spacing visually meaningful.',
    'exo-Kepler-90': 'Kepler-90 is a packed multi-planet comparison case where architecture and orbital crowding matter.',
    'exo-Kepler-11': 'Kepler-11 emphasizes compact multi-planet stability and the challenge of fitting many worlds into a narrow orbital span.',
    'exo-HR 8799': 'HR 8799 is a wide, massive-system contrast case with larger separations and long-period companions.',
    'exo-Kepler-20': 'Kepler-20 is useful for comparing interior compact planets with farther companions in one observed-system frame.',
});

const COSMIC_SCENARIO_GUIDES = Object.freeze({
    'cosmic-galaxy': 'Spiral Galaxy is the rotation-curve and density-wave teaching case: watch orbital speed as a function of radius and how arms persist as patterns rather than rigid spokes.',
    'cosmic-cartwheel-collision': 'Cartwheel Collision is a ring-wave scenario in galactic form, where an impact launches an outward-moving density disturbance.',
    'cosmic-super-cluster': 'Supercluster Interaction is best read through large-scale potential wells and cluster-to-cluster influence rather than star-by-star detail.',
    'cosmic-merger': 'Galaxy Merger is about tidal stripping, angular-momentum redistribution, and the violent relaxation of two large gravitating systems.',
    'cosmic-binary-agn': 'Binary quasars combine orbital dynamics with active central engines, so accretion and compact-center motion both matter.',
    'cosmic-globular-cluster': 'Globular Cluster is the virial-balance and self-gravitating stellar-swarm case.',
    'cosmic-black-hole': 'Black Hole Close-up is a strong-gravity intuition builder: orbital speed rises, escape conditions sharpen, and near-horizon behavior dominates the camera framing.',
    'cosmic-ftd-collapse': 'FTD Collapse is the project’s collapse-to-compact-object narrative surface, so threshold and horizon language matter more than ordinary orbital talk.',
    'cosmic-stellar-lifecycle': 'Stellar Lifecycle is a staged-evolution scenario: fuel, equilibrium, transition, and remnant framing.',
    'cosmic-web': 'Cosmic Web is the large-scale-density-field view where filaments and nodes matter more than individual bound objects.',
    'cosmic-dark-matter-halo': 'Dark Matter Halo is about dynamical inference: the visible distribution alone does not explain the motion, so an extended halo profile is introduced.',
    'cosmic-gravitational-wave': 'Gravitational Wave is the binary-inspiral and strain case, where shrinking orbit and emitted wave signal are linked.',
    'cosmic-baryogenesis': 'Baryogenesis is the asymmetry story: how a tiny normalized excess can seed the later matter-dominated universe.',
});

function buildScale0ScenarioEntry(scenario) {
    const categoryGuide = SCALE0_CATEGORY_GUIDES[scenario.category] || {
        summary: `${scenario.title} is a Scale 0 lattice scenario.`,
        body: ['This scenario belongs to the Scale 0 lattice curriculum even if it does not yet have a more detailed scenario note.'],
        notation: ['J(v,t)', 's(v,t)'],
        tags: ['scale0'],
    };
    const specificGuide = SCALE0_SPECIFIC_GUIDES[scenario.id] || {};
    const seedMeta = S0_SEED_SCENARIO_METADATA[scenario.id] || null;
    const quantumDesc = QUANTUM_SCENARIO_DESCRIPTIONS[scenario.id] || '';

    const body = [];
    if (seedMeta?.desc) body.push(seedMeta.desc);
    else if (quantumDesc) body.push(quantumDesc);
    else body.push(`${scenario.title} belongs to the "${scenario.category}" family. ${specificGuide.summary || categoryGuide.summary}`);
    body.push(...(specificGuide.body || []));
    body.push(...(categoryGuide.body || []));

    const bullets = dedupe([
        `Category: ${scenario.category}.`,
        scenario.epistemicStatus && scenario.epistemicStatus !== '[OPEN]' ? `Epistemic status: ${scenario.epistemicStatus}.` : '',
        ...(seedMeta?.epistemic || []).map(([field, tag]) => `${field}: ${tag}.`),
        ...(scenario.tags || []).map((tag) => `Tag: ${tag}.`),
    ]);

    return makeScenarioEntry({
        id: scenario.id,
        title: `${scenario.title} [Scale 0]`,
        shortTitle: scenario.title,
        scale: 'Scale 0 / Lattice',
        summary: specificGuide.summary || categoryGuide.summary,
        body,
        bullets,
        notation: dedupe([...(specificGuide.notation || []), ...(categoryGuide.notation || [])]),
        tags: ['scale0', scenario.category.toLowerCase(), ...(scenario.tags || [])],
    });
}

function makeParticleScenario(id, title, summary, notation = [], focus = '') {
    const guide = PARTICLE_SCENARIO_GUIDES[id] || '';
    return makeScenarioEntry({
        id,
        title: `${title} [Scale 1]`,
        shortTitle: title,
        scale: 'Scale 1 / Particles',
        summary,
        body: [
            `${title} is a particle-engine scenario centered on ${summary.toLowerCase()}`,
            focus || guide || 'The main math here is pairwise interaction dynamics: Coulomb-like attraction/repulsion, reduced-mass effects, scattering kinematics, or relativistic energy depending on the participating species.',
        ],
        bullets: [
            'Primary Scale 1 math: inverse-square forces, reduced mass, orbital or scattering kinematics.',
            'Helpful controls: Coulomb, gravity, damping, time step, softening, and any advanced-force toggles that the active backend explicitly supports.',
        ],
        notation,
        tags: ['scale1', 'particle-engine'],
    });
}

const PARTICLE_SCENARIO_ENTRIES = Object.freeze([
    makeParticleScenario('pe-hydrogen', 'Hydrogen Atom (p + e−)', 'a Coulomb-bound two-body atom.', ['V(r) = -α/r', 'μ', '\\(E_n = -\\mu\\alpha^2/(2n^2)\\)', 'a₀ = 1/(μ α)']),
    makeParticleScenario('pe-hydrogen-fine', 'Hydrogen (spin + dipole demo)', 'hydrogen with μ = (q/m)S dipoles, spin-orbit, and evolving spin axes.', ['dS/dt = (q/m) S×B', 'F_dd ∝ 1/r⁴', 'L·S coupling']),
    makeParticleScenario('pe-helium', 'Helium Atom (He²⁺ + 2e−)', 'a three-body Coulomb problem with electron-electron repulsion.', ['ΣV_ij', 'screening', 'many-body bound state']),
    makeParticleScenario('pe-positronium', 'Positronium (e⁺e−)', 'an equal-mass bound state of matter and antimatter.', ['μ = \\(m_e\\)/2', '\\(E_n \\propto -\\mu\\alpha^2\\)']),
    makeParticleScenario('pe-muonium', 'Muonium (μ⁺e−)', 'a hydrogen-like exotic atom with a different reduced mass.', ['μ', 'Rydberg scaling']),
    makeParticleScenario('pe-true-muonium', 'True Muonium (μ⁺μ−)', 'a heavier equal-mass leptonic bound state.', ['μ = m_μ/2', 'compact bound state']),
    makeParticleScenario('pe-tauonium', 'Tauonium (τ⁺τ−)', 'an ultra-heavy leptonic atom with short lifetime scales.', ['μ = m_τ/2', 'E² = p² + m²']),
    makeParticleScenario('pe-tau-atom', 'Tauonic Hydrogen (τ− + p)', 'a hydrogenic atom where the orbiting lepton is much heavier than the electron.', ['μ', 'smaller Bohr radius']),
    makeParticleScenario('pe-pionic-hydrogen', 'Pionic Hydrogen (π− + p)', 'a hadron-facing Coulomb baseline.', ['Coulomb baseline', 'reduced mass']),
    makeParticleScenario('pe-kaonic-hydrogen', 'Kaonic Hydrogen (K− + p)', 'a heavier exotic Coulomb baseline.', ['μ', 'backend-supported short-range terms only']),
    makeParticleScenario('pe-sigma-plus-atom', 'Sigma⁺ Atom (Σ⁺ + e−)', 'a baryon-electron bound system with different core mass and charge structure.', ['V(r) = -α/r', 'μ']),
    makeParticleScenario('pe-antiprotonic-hydrogen', 'Protonium (p̄ + p)', 'a matter-antimatter proton pair with Coulomb attraction and geometric contact behavior.', ['contact rule', 'two-body Coulomb problem']),
    makeParticleScenario('pe-pion-orbit', 'Pionium (π⁺π−)', 'a mesonic bound state of opposite charges.', ['μ', 'two-body Coulomb problem']),
    makeParticleScenario('pe-kaon-pair', 'Kaonium (K⁺K−)', 'a heavier mesonic bound state used to compare mass scaling and binding.', ['μ', 'bound-state scaling']),
    makeParticleScenario('pe-delta-system', 'Delta++ System (Δ++ + 2e−)', 'a multi-charge bound scenario with strong repulsion/attraction balance.', ['many-body Coulomb', 'net charge']),
    makeParticleScenario('pe-omega-scattering', 'Omega− Scattering (Ω− + e⁺)', 'a scattering-focused heavy-hadron interaction setup.', ['impact parameter', 'deflection angle']),
    makeParticleScenario('pe-deuteron', 'Deuteron (p + n + e−)', 'a locked isotope-core composition demo with an electron.', ['neutral ballast', 'Coulomb electron motion']),
    makeParticleScenario('pe-tritium', 'Tritium (p + 2n + e−)', 'a heavier locked isotope-core presentation.', ['neutral ballast', 'Coulomb electron motion']),
    makeParticleScenario('pe-helion', 'Helion / He-3 (2p + n + 2e−)', 'a locked helium-3 core plus two-electron Coulomb demo.', ['Coulomb many-body', 'locked core']),
    makeParticleScenario('pe-w-pair', 'W⁺W− Pair', 'a charged massive-particle pair with backend-supported relativistic correction.', ['E² = p² + m_W²', 'pair kinematics']),
    makeParticleScenario('pe-scattering', 'Proton-Electron Scattering', 'a clean two-body scattering experiment.', ['dσ/dΩ', 'impact parameter', 'momentum transfer q']),
    makeParticleScenario('pe-three-body', 'Three-Body (p⁺ p⁺ e−)', 'a nontrivial few-body balance between attraction and repulsion.', ['few-body instability', 'Coulomb balance']),
    makeParticleScenario('pe-meson-scattering', 'π⁺ off Proton', 'a hadron-proton scattering test.', ['impact parameter', 'scattering angle']),
    makeParticleScenario('pe-muon-scattering', 'μ− off Proton', 'a lepton-proton scattering comparison case.', ['momentum transfer', 'Rutherford-like scattering']),
    makeParticleScenario('pe-micro-bh', 'Micro Black Hole (Accretion)', 'a Newtonian gravity toy with imposed horizon and emission visuals.', ['visual horizon', 'accretion', 'escape velocity']),
    makeParticleScenario('pe-custom', 'Custom (Manual)', 'user-prepared particle initial data and force settings.', ['user-defined initial conditions', 'force toggles']),
]);

function makeAtomScenario(id, title, summary, notation = [], focus = '') {
    const guide = ATOM_SCENARIO_GUIDES[id] || '';
    return makeScenarioEntry({
        id,
        title: `${title} [Scale 2]`,
        shortTitle: title,
        scale: 'Scale 2 / Atoms',
        summary,
        body: [
            `${title} is an atom-engine scenario centered on ${summary.toLowerCase()}`,
            focus || guide || 'The main math here is molecular-dynamics style: Coulomb attraction/repulsion, Lennard-Jones-style van der Waals terms, harmonic bond springs, angle constraints, and thermostat or damping terms where enabled.',
        ],
        bullets: [
            'Primary Scale 2 math: ionic forces, van der Waals potential, bond springs, bond-angle geometry, and damping/thermostat controls.',
            'Helpful controls: ionic, vdW, bond springs, auto-bonding, H-bonds, dipole, angle strain, time step, and softening.',
        ],
        notation,
        tags: ['scale2', 'atom-engine'],
    });
}

const ATOM_SCENARIO_ENTRIES = Object.freeze([
    makeAtomScenario('ae-hydrogen-atom', 'Hydrogen Atom (p + e−)', 'a one-electron central-force atom.', ['V(r) = -kQ_iQ_j/r²', 'central potential']),
    makeAtomScenario('ae-rutherford-scattering', 'Rutherford Scattering', 'a charged-particle scattering experiment off a compact center.', ['θ(b)', 'inverse-square scattering']),
    makeAtomScenario('ae-he-cluster', 'He Cluster (6 atoms, vdW)', 'a weakly bound noble-gas cluster dominated by dispersion forces.', ['Lennard-Jones 12-6']),
    makeAtomScenario('ae-ar-cluster', 'Ar Cluster (8 atoms, vdW)', 'a heavier noble-gas cluster with stronger dispersion attraction.', ['Lennard-Jones 12-6']),
    makeAtomScenario('ae-noble-mix', 'Noble Mix (He + Ne + Ar)', 'mixed-species van der Waals clustering and separation.', ['σ', 'ε', 'dispersion']),
    makeAtomScenario('ae-nacl-form', 'Na + Cl -> NaCl', 'ionic bond formation from opposite charges.', ['V(r) ∝ -1/r', 'ionic attraction']),
    makeAtomScenario('ae-nacl-lattice', 'NaCl 3x3 Lattice', 'periodic ionic packing and crystal-like order.', ['lattice energy', 'ionic crystal']),
    makeAtomScenario('ae-mgf2', 'Mg²⁺ + 2F− -> MgF₂', 'charge-balanced ionic assembly with stoichiometric attraction.', ['charge balance', 'ionic binding']),
    makeAtomScenario('ae-h2-form', 'H + H -> H₂', 'the simplest covalent bond-formation case.', ['bond spring', 'equilibrium bond length']),
    makeAtomScenario('ae-o2-form', 'O + O -> O₂', 'a covalent diatomic formation scenario with stronger bonding.', ['bond order', 'equilibrium separation']),
    makeAtomScenario('ae-ch4-form', 'C + 4H -> CH₄', 'tetrahedral covalent formation around a carbon center.', ['tetrahedral geometry', '109.5°']),
    makeAtomScenario('ae-water-dimer', 'Water Dimer (H-bond)', 'directional hydrogen bonding between two polar molecules.', ['H-bond', 'dipole alignment']),
    makeAtomScenario('ae-water-cluster', 'Water Pentamer', 'small-network hydrogen bonding and cluster geometry.', ['network bonding', 'H-bond geometry']),
    makeAtomScenario('ae-vsepr-linear', 'CO₂ -> Linear (180°)', 'VSEPR geometry favoring a linear molecular arrangement.', ['180°', 'VSEPR']),
    makeAtomScenario('ae-vsepr-tetrahedral', 'CH₄ -> Tetrahedral (109.5°)', 'VSEPR geometry favoring tetrahedral coordination.', ['109.5°', 'tetrahedral']),
    makeAtomScenario('ae-vsepr-bent', 'H₂O -> Bent (104.5°)', 'VSEPR geometry shifted by lone-pair repulsion.', ['104.5°', 'bent geometry']),
    makeAtomScenario('ae-thermal-gas', 'Ar Gas (12 atoms + thermostat)', 'thermalized gas-like motion under thermostat control.', ['temperature', 'Berendsen thermostat']),
    makeAtomScenario('ae-collision', 'Head-On Collision', 'direct impact dynamics, momentum exchange, and rebound.', ['momentum conservation', 'collision dynamics']),
    makeAtomScenario('ae-fe-bcc', 'Fe BCC Cluster (9 atoms)', 'body-centered-cubic metallic packing.', ['BCC packing', 'coordination number']),
    makeAtomScenario('ae-cu-fcc', 'Cu FCC Seed (7 atoms)', 'face-centered-cubic metallic packing.', ['FCC packing', 'coordination number']),
    makeAtomScenario('ae-periodic', 'Periodic Table (All 118)', 'a catalogue-driven survey across element parameters rather than one fixed small molecule.', ['Z', 'valence', 'periodic trends']),
    makeAtomScenario('ae-custom', 'Custom (Manual)', 'user-prepared atom-engine initial conditions.', ['user-defined initial data']),
]);

const MOLECULE_CATEGORY_MATH = Object.freeze({
    diatomic: ['bond length', 'two-body reduced mass', 'vibrational mode'],
    inorganic: ['VSEPR geometry', 'ionic vs covalent bonding', 'bond angle'],
    organic: ['hybridization', 'bond angle', 'ring or chain geometry'],
    biochemical: ['network bonding', 'crystal order', 'hydrogen bonding'],
});

const MOLECULE_SCENARIO_ENTRIES = Object.freeze(
    getAllMolecules().map((molecule) =>
        makeScenarioEntry({
            id: `mol-${molecule.id}`,
            title: `${molecule.formula.replace(/<[^>]+>/g, '')} ${molecule.name} [Scale 3]`,
            shortTitle: `${molecule.formula.replace(/<[^>]+>/g, '')} ${molecule.name}`,
            scale: 'Scale 3 / Molecules',
            summary: molecule.description,
            body: [
                `${molecule.name} is a molecule-library scenario loaded into the molecular view from the shared catalogue.`,
                `The math focus depends on its category: ${molecule.category}. Here that usually means ${MOLECULE_CATEGORY_MATH[molecule.category]?.join(', ') || 'bond geometry and interaction energy'}.`,
            ],
            bullets: [
                `Molecule category: ${molecule.category}.`,
                `Library description: ${molecule.description}`,
            ],
            notation: molecule.category === 'diatomic' ? ['\\(r_\\mathrm{eq}\\)', 'vibrational mode'] : MOLECULE_CATEGORY_MATH[molecule.category] || ['bond geometry'],
            tags: ['scale3', 'molecule', molecule.category],
        }),
    ).concat([
        makeScenarioEntry({
            id: 'mol-crystal',
            title: 'NaCl Crystal [Scale 3]',
            shortTitle: 'NaCl Crystal',
            scale: 'Scale 3 / Molecules',
            summary: 'A periodic ionic crystal scenario rather than a single isolated molecule.',
            body: [
                'This special molecular-view scenario emphasizes repeated ionic packing rather than one finite molecule.',
                'The governing math is lattice energy, periodic order, nearest-neighbor coordination, and long-range Coulomb balance.',
            ],
            bullets: ['Special Scale 3 scenario.', 'Best read as crystal structure rather than isolated chemistry.'],
            notation: ['lattice energy', 'periodic cell', 'coordination number'],
            tags: ['scale3', 'crystal', 'ionic'],
        }),
        makeScenarioEntry({
            id: 'mol-custom',
            title: 'Custom (Manual) [Scale 3]',
            shortTitle: 'Custom',
            scale: 'Scale 3 / Molecules',
            summary: 'A user-defined molecular starting point.',
            body: [
                'The custom molecular scenario lets you supply your own initial composition and geometry.',
                'The math you should watch depends entirely on what you build: bond lengths, angles, symmetry, packing, and whether the chosen force terms stabilize the structure.',
            ],
            bullets: ['User-authored molecular initial conditions.'],
            notation: ['user-defined geometry'],
            tags: ['scale3', 'custom'],
        }),
    ]),
);

const PLANETARY_SCENARIO_ENTRIES = Object.freeze([
    makeScenarioEntry({
        id: 'planetary-solar',
        title: 'Solar System [Scale 4]',
        shortTitle: 'Solar System',
        scale: 'Scale 4 / Planetary',
        summary: 'A canonical many-body orbital system with one dominant central mass.',
        body: [
            'This scenario is the standard planetary baseline: approximately Keplerian motion organized around a dominant central body.',
            PLANETARY_SCENARIO_GUIDES['planetary-solar'],
            'The math emphasis is orbital mechanics: inverse-square gravity, angular momentum conservation, semimajor axis, eccentricity, and orbital period scaling.',
        ],
        bullets: ['Best first stop for planetary mechanics.'],
        notation: ['F = G m₁m₂/r²', 'L', 'a', 'e', 'T² ∝ a³'],
        tags: ['scale4', 'orbits'],
    }),
    makeScenarioEntry({
        id: 'planetary-binary',
        title: 'Binary Star System [Scale 4]',
        shortTitle: 'Binary Stars',
        scale: 'Scale 4 / Planetary',
        summary: 'Two heavy bodies orbiting a shared barycenter.',
        body: [
            'Binary-star motion shifts the intuition from one-center orbits to mutual revolution about a barycenter.',
            PLANETARY_SCENARIO_GUIDES['planetary-binary'],
            'The important math is center-of-mass motion, orbital stability, and how secondary bodies respond to a moving gravitational frame.',
        ],
        bullets: ['Barycentric mechanics rather than fixed-center orbits.'],
        notation: ['barycenter', 'center of mass'],
        tags: ['scale4', 'binary'],
    }),
    makeScenarioEntry({
        id: 'planetary-threebody',
        title: 'Three-Body Problem [Scale 4]',
        shortTitle: 'Three-Body',
        scale: 'Scale 4 / Planetary',
        summary: 'A classic nonintegrable orbital-dynamics problem.',
        body: [
            'The three-body scenario emphasizes sensitivity, resonance, and complex orbital exchanges that do not reduce to a simple closed-form two-body solution.',
            PLANETARY_SCENARIO_GUIDES['planetary-threebody'],
            'This is where perturbation theory, resonance, and numerical integration matter more than a neat analytic ellipse.',
        ],
        bullets: ['Excellent for learning why many-body gravity becomes hard quickly.'],
        notation: ['N-body integration', 'resonance', 'chaos'],
        tags: ['scale4', 'three-body', 'chaos'],
    }),
    ...['TRAPPIST-1', 'Kepler-90', 'Kepler-11', 'HR 8799', 'Kepler-20'].map((name) =>
        makeScenarioEntry({
            id: `exo-${name}`,
            title: `${name} System [Scale 4]`,
            shortTitle: `${name}`,
            scale: 'Scale 4 / Planetary',
            summary: 'An exoplanetary system imported as a real-system orbital sandbox.',
            body: [
                `${name} is a data-driven planetary system scenario based on archived exoplanet data rather than a stylized toy model.`,
                PLANETARY_SCENARIO_GUIDES[`exo-${name}`],
                'The math focus is still gravitational N-body dynamics, but now interpreted through orbital architectures, resonant chains, spacing, stability windows, and observed-system comparison.',
            ],
            bullets: ['Real-system inspired exoplanet setup.'],
            notation: ['N-body gravity', 'resonance chain', 'orbital period'],
            tags: ['scale4', 'exoplanet'],
        }),
    ),
]);

const COSMIC_SCENARIO_ENTRIES = Object.freeze([
    ['cosmic-galaxy', 'Spiral Galaxy', 'galactic rotation, arm structure, and large-scale orbital motion.', ['rotation curve', 'orbital velocity', 'galactic potential']],
    ['cosmic-cartwheel-collision', 'Cartwheel Collision', 'ring-galaxy formation from a high-energy collision.', ['collision dynamics', 'density wave']],
    ['cosmic-super-cluster', 'Supercluster Interaction', 'very large-scale structure and mutual cluster influence.', ['cluster potential', 'large-scale gravity']],
    ['cosmic-merger', 'Galaxy Merger', 'nonlinear tidal interaction and structure mixing during coalescence.', ['tidal forces', 'merger dynamics']],
    ['cosmic-binary-agn', 'Binary Quasars', 'dual active galactic nuclei and compact massive central dynamics.', ['AGN accretion', 'binary orbit']],
    ['cosmic-globular-cluster', 'Globular Cluster', 'dense stellar packing and self-gravitating cluster motion.', ['virial balance', 'cluster binding']],
    ['cosmic-black-hole', 'Black Hole Close-up', 'strong central gravity and near-horizon motion.', ['escape velocity', 'Schwarzschild radius']],
    ['cosmic-ftd-collapse', 'FTD Collapse (Emergent BH)', 'gravitational collapse in the project’s cosmic presentation.', ['collapse threshold', 'horizon formation']],
    ['cosmic-stellar-lifecycle', 'Stellar Lifecycle', 'formation, evolution, and remnant-style transitions in stellar populations.', ['fusion-era scaling', 'lifecycle stages']],
    ['cosmic-web', 'Cosmic Web', 'filamentary large-scale matter structure.', ['filament network', 'large-scale density field']],
    ['cosmic-dark-matter-halo', 'Dark Matter Halo', 'mass distribution inferred from dynamics beyond luminous structure alone.', ['halo profile', 'rotation support']],
    ['cosmic-gravitational-wave', 'Gravitational Wave (Binary)', 'binary-driven wave emission and inspiral intuition.', ['h(t)', 'wave strain', 'inspiral']],
    ['cosmic-baryogenesis', 'Baryogenesis', 'matter-antimatter imbalance at cosmological scale.', ['\\(\\eta = (n_B - n_\\bar{B})/n_\\gamma\\)', 'asymmetry generation']],
].map(([id, title, summary, notation]) =>
    makeScenarioEntry({
        id,
        title: `${title} [Scale 5]`,
        shortTitle: title,
        scale: 'Scale 5 / Cosmic',
        summary: `A cosmic-scale scenario for ${summary}`,
        body: [
            `${title} is one of the high-level cosmic presentations used to reason about emergent astrophysical structure.`,
            COSMIC_SCENARIO_GUIDES[id],
            'The main math language is gravitating many-body dynamics, density evolution, orbital or collapse timescales, and whichever large-scale observable the scenario foregrounds.',
        ],
        bullets: ['Scale 5 scenarios auto-play through the cosmic renderer and telemetry surface.'],
        notation,
        tags: ['scale5', 'cosmic'],
    }),
));

const SCENARIO_KB_SECTIONS = Object.freeze([
    {
        id: 'scenario-lattice',
        title: 'Scenarios: Scale 0',
        description: 'Every Scale 0 lattice scenario, including what it demonstrates and the math it foregrounds.',
        entries: SCALE0_SCENARIOS.map(buildScale0ScenarioEntry),
    },
    {
        id: 'scenario-particles-atoms',
        title: 'Scenarios: Scales 1-2',
        description: 'Particle-engine and atom-engine scenarios with the main governing interaction laws.',
        entries: [...PARTICLE_SCENARIO_ENTRIES, ...ATOM_SCENARIO_ENTRIES],
    },
    {
        id: 'scenario-molecules',
        title: 'Scenarios: Scale 3',
        description: 'Molecule-library scenarios and the geometry or bonding math they emphasize.',
        entries: MOLECULE_SCENARIO_ENTRIES,
    },
    {
        id: 'scenario-worlds',
        title: 'Scenarios: Scales 4-5',
        description: 'Planetary and cosmic scenarios with their conceptual math framing.',
        entries: [...PLANETARY_SCENARIO_ENTRIES, ...COSMIC_SCENARIO_ENTRIES],
    },
]);

export const KNOWLEDGE_BASE = Object.freeze([...KNOWLEDGE_BASE_SECTIONS, ...SCENARIO_KB_SECTIONS]);

export const KNOWLEDGE_BASE_ENTRIES = Object.freeze(
    KNOWLEDGE_BASE.flatMap((section) => section.entries.map((entry) => ({ ...entry, sectionId: section.id, sectionTitle: section.title }))),
);

export function getKnowledgeBaseSections() {
    return KNOWLEDGE_BASE;
}

export function getKnowledgeBaseEntry(entryId) {
    return KNOWLEDGE_BASE_ENTRIES.find((entry) => entry.id === entryId) || null;
}

export function searchKnowledgeBase(query = '', sectionId = 'all') {
    const normalizedQuery = String(query || '').trim().toLowerCase();
    return KNOWLEDGE_BASE_ENTRIES.filter((entry) => {
        if (sectionId !== 'all' && entry.sectionId !== sectionId) return false;
        if (!normalizedQuery) return true;
        const haystack = [
            entry.title,
            entry.shortTitle,
            entry.summary,
            ...(entry.body || []),
            ...(entry.bullets || []),
            ...(entry.notation || []),
            ...(entry.tags || []),
            entry.sectionTitle,
        ]
            .filter(Boolean)
            .join(' ')
            .toLowerCase();
        return haystack.includes(normalizedQuery);
    });
}
