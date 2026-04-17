import { CS_SCENARIO_DESCRIPTIONS, QUANTUM_SCENARIO_DESCRIPTIONS, S0_SEED_SCENARIO_METADATA } from '../../../config/scenarios.js';
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
                notation: ['L', 'v ∈ L ⊂ Z^3'],
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
                    'The safe way to read rho is to ask what kind of object the surrounding equation expects: a scalar field, a matrix, or a physical density ratio.',
                ],
                bullets: [
                    'Scalar rho often means density or magnitude-like content.',
                    'Matrix rho often means a statistical quantum state.',
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
                    'In the engine UI, alpha is more likely to appear in educational descriptions, derived constants, or explanatory material than as a live control.',
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
                title: 'N_c (Color Charges)',
                shortTitle: 'N_c',
                summary: 'Framework integer associated with color structure in project references.',
                body: [
                    'N_c is the project’s symbol for the color-charge count and is listed as 3 in the reference glossary.',
                    'For learners moving between the engine and the theory notes, this is one of the key framework integers that keeps recurring in coupling, particle, and structural discussions.',
                ],
                bullets: [
                    'Framework integer.',
                    'Listed as 3 in the reference glossary.',
                    'Shows up in coupling and particle discussions.',
                ],
                notation: ['N_c'],
                tags: ['constants', 'particle-physics'],
            },
            {
                id: 'n-base',
                title: 'N_base (Base Dimension)',
                shortTitle: 'N_base',
                summary: 'A framework integer used in the project’s structural and coupling arguments.',
                body: [
                    'The symbol N_base is listed in the symbol glossary as the base dimension integer with value 4.',
                    'You will usually see it in theory-facing expressions rather than in runtime telemetry, but it matters because it helps tie together several framework counts and derived integer combinations.',
                ],
                bullets: [
                    'Framework integer with value 4 in the glossary.',
                    'Often appears alongside N_c and b₃.',
                    'More structural than operational in the live UI.',
                ],
                notation: ['N_base = 4'],
                tags: ['constants', 'framework-integers'],
            },
            {
                id: 'b3',
                title: 'b₃ (QCD Beta Coefficient)',
                shortTitle: 'b₃',
                summary: 'The framework beta coefficient used in strong-coupling discussions.',
                body: [
                    'The reference glossary lists b₃ as the QCD beta coefficient with value 7. In the broader theory writing it participates in coupling formulas and angle selections.',
                    'For learners, the practical point is that b₃ is one of the named integer ingredients that connects structural counting to effective coupling stories.',
                ],
                bullets: [
                    'Listed as 7 in the glossary.',
                    'Shows up in strong-coupling and CKM-style expressions.',
                    'Often grouped with N_c, N_base, and n_eff.',
                ],
                notation: ['b₃ = 7'],
                tags: ['constants', 'qcd', 'framework-integers'],
            },
            {
                id: 'n-eff',
                title: 'n_eff (Effective Dimension)',
                shortTitle: 'n_eff',
                summary: 'The effective-dimension count used in several FTD coupling formulas.',
                body: [
                    'The glossary gives n_eff as 13. It is one of the recurring bookkeeping counts used in electroweak and strong-coupling expressions.',
                    'When the project refers to an effective dimension in a formula, it is usually talking about a model-specific counting quantity rather than a literal spatial dimension you can navigate inside the viewport.',
                ],
                bullets: [
                    'Listed as 13 in the glossary.',
                    'Appears in coupling formulas like sin²θ_W and α_s.',
                    'A framework count, not a camera-space coordinate axis.',
                ],
                notation: ['n_eff = 13'],
                tags: ['constants', 'dimension', 'framework-integers'],
            },
            {
                id: 'g-newton',
                title: 'G_N (Newton’s Constant)',
                shortTitle: 'G_N',
                summary: 'The gravitational coupling constant in natural-unit notation.',
                body: [
                    'G_N is the usual symbol for Newton’s gravitational constant. In natural-unit conventions it is often written without SI dimensions, but conceptually it is still the coupling that sets gravitational strength.',
                    'This is a good example of why context matters: uppercase G* and G_N are very different objects even though both start with the same letter.',
                ],
                bullets: [
                    'Gravitational coupling constant.',
                    'Distinct from G*.',
                    'Important for Planck-unit definitions.',
                ],
                notation: ['G_N'],
                tags: ['constants', 'gravity', 'disambiguation'],
            },
            {
                id: 'manifestation-kb',
                title: 'K_B (Manifestation Constant)',
                shortTitle: 'K_B',
                summary: 'Engine-side named constant currently exported as 0.511.',
                body: [
                    'In the web engine tests, K_B is verified as a named export with value 0.511. The project instructions describe this as the manifestation constant derived from the model’s mass story.',
                    'This is not the same thing as the lowercase k_B used for the Boltzmann constant in natural-units notation. The near-identical names make this one worth learning explicitly.',
                ],
                bullets: [
                    'Named export in the JS constants module.',
                    'Tested in the web suite as 0.511.',
                    'Different from the lowercase k_B of thermodynamics.',
                ],
                notation: ['K_B = 0.511', 'k_B = 1'],
                tags: ['constants', 'engine', 'disambiguation'],
            },
            {
                id: 'c-speed',
                title: 'C_SPEED',
                shortTitle: 'C_SPEED',
                summary: 'The engine’s named propagation-speed constant.',
                body: [
                    'The engine documentation lists C_SPEED as the propagation-speed constant used for cubic-lattice stability conventions.',
                    'Even when it is not surfaced directly in controls, it helps explain why the engine talks about local propagation limits, CFL-like stability, and discrete-time update pacing.',
                ],
                bullets: [
                    'Named engine constant.',
                    'Used in discrete propagation and stability explanations.',
                    'Connects runtime behavior to the lattice update model.',
                ],
                notation: ['C_SPEED'],
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
                    'This does not mean every other quantity in the project has three components. Some are scalars, some are vectors, some are matrices, and some are abstract counts like n_eff.',
                ],
                bullets: [
                    'Three spatial dimensions for the lattice.',
                    'Matches the cubic-grid visualization used in the engine.',
                    'Distinct from model-specific integer counts like N_base and n_eff.',
                ],
                notation: ['D = 3', 'L ⊂ Z^3'],
                tags: ['dimension', 'space', 'geometry'],
            },
            {
                id: 'natural-units',
                title: 'Natural Units',
                shortTitle: 'Natural Units',
                summary: 'A unit convention where key constants are set to 1 so equations focus on relationships instead of conversion factors.',
                body: [
                    'The project glossary states that FTD uses natural units with c = 1, ℏ = 1, and k_B = 1. This is a standard physics move that simplifies formulas by absorbing conversion factors into the unit definitions.',
                    'When you see a formula in natural units, it does not mean those constants disappeared physically. It means the chosen unit system measures everything relative to them.',
                ],
                bullets: [
                    'Speed of light: c = 1.',
                    'Reduced Planck constant: ℏ = 1.',
                    'Boltzmann constant: k_B = 1.',
                ],
                notation: ['c = 1', 'ℏ = 1', 'k_B = 1'],
                tags: ['units', 'natural-units', 'conventions'],
            },
            {
                id: 'planck-units',
                title: 'Planck Units',
                shortTitle: 'Planck Units',
                summary: 'The natural-unit scale built from c, ℏ, and G_N.',
                body: [
                    'Planck units are the characteristic scales built from the speed of light, reduced Planck constant, and Newton’s constant. They provide a compact unit language for quantum gravity and lattice-scale reasoning.',
                    'The project glossary explicitly maps Planck length, time, and mass into the FTD unit story, which makes them especially useful as bridge concepts between theory documents and the engine.',
                ],
                bullets: [
                    'Length, time, and mass can all be expressed in Planck units.',
                    'Planck units are built from c, ℏ, and G_N.',
                    'Helpful when comparing discrete engine units to theory-facing notation.',
                ],
                notation: ['ℓ_P = √(ℏG/c^3)', 't_P = √(ℏG/c^5)', 'm_P = √(ℏc/G)'],
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
                notation: ['E', 'm', 'E = mc^2', 'c = 1'],
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
                    'A coupling constant measures interaction strength. Fine structure alpha, strong coupling α_s, and state-flux couplings all belong to this family.',
                    'For learners, the main intuition is simple: a larger coupling usually means a stronger interaction, though the exact physical meaning depends on the theory and scale.',
                ],
                bullets: [
                    'Couplings measure interaction strength.',
                    'Can be dimensionless or unit-bearing depending on the theory.',
                    'Several project formulas are organized around specific coupling values.',
                ],
                notation: ['α', 'α_s', 'g_c'],
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
                title: 'H_FTD (Hilbert Space)',
                shortTitle: 'Hilbert Space',
                summary: 'The vector-space setting where wave functions and inner products live.',
                body: [
                    'A Hilbert space is a complete inner-product space. In physics it is the standard home for quantum states, amplitudes, and operators.',
                    'The glossary names H_FTD as L² over the lattice with complex values. That is a formal way of saying the model’s quantum-style states are functions on the lattice with an inner-product structure.',
                ],
                bullets: [
                    'State space for quantum vectors.',
                    'Supports inner products, norms, and operators.',
                    'The glossary writes H_FTD as a lattice-based L² space.',
                ],
                notation: ['H_FTD', 'L²(Lattice, C)'],
                tags: ['quantum', 'hilbert-space', 'linear-algebra'],
            },
            {
                id: 'density-matrix',
                title: 'ρ (Density Matrix)',
                shortTitle: 'Density Matrix',
                summary: 'The matrix object used to represent pure states, mixed states, and statistical quantum ensembles.',
                body: [
                    'A density matrix packages state information in an operator rather than a single ket. It is especially useful when a system is probabilistic, mixed, or part of a larger entangled whole.',
                    'This is the matrix meaning of rho, not the scalar density meaning. That distinction is worth learning early because both uses appear in the project’s reference material.',
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
                title: 'S_vN (von Neumann Entropy)',
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
                notation: ['S_vN = -Tr(ρ ln ρ)'],
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
                    'The symbol glossary includes sin²θ_W as the weak mixing quantity and gives an FTD-side formula for it. In mainstream particle-physics language, it measures how electroweak components combine.',
                    'In the knowledge base, the main point is recognition: this is a coupling-related parameter, not a literal geometric angle you rotate with the camera.',
                ],
                bullets: [
                    'Electroweak parameter.',
                    'Appears in coupling formulas.',
                    'Not a viewport angle or scene transform.',
                ],
                notation: ['sin²θ_W = N_c / n_eff'],
                tags: ['particle-physics', 'coupling', 'electroweak'],
            },
            {
                id: 'strong-coupling',
                title: 'α_s (Strong Coupling)',
                shortTitle: 'α_s',
                summary: 'The interaction-strength parameter associated with the strong sector.',
                body: [
                    'Alpha_s is the strong coupling constant. It plays for strong interactions a role somewhat analogous to what alpha plays in electromagnetism, though the physical behavior is different.',
                    'The project glossary includes a model-side expression for α_s, so it is a useful bridge term between theory files and educational UI copy.',
                ],
                bullets: [
                    'Strong-interaction coupling.',
                    'Appears in coupling and mixing discussions.',
                    'Distinct from electromagnetic alpha.',
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
                    'In cosmology, Omega often denotes a density parameter: how much of some component exists relative to a critical benchmark density.',
                    'This is another overloaded symbol. In other contexts Omega can mean a microstate count or a region of configuration space, so context matters.',
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
                notation: ['η = (n_B - n_B̄) / n_γ'],
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
                ],
                bullets: [
                    'Atom-focused view.',
                    'Includes derived molecular and bonding information.',
                    'Bridges particle-level behavior to chemistry-like structure.',
                ],
                notation: ['Scale 2'],
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
            {
                id: 'scale11',
                title: 'Scale 11: Consciousness',
                shortTitle: 'Scale 11',
                summary: 'Pedagogical consciousness-mode presentation with figure and scenario controls.',
                body: [
                    'Scale 11 is the consciousness-oriented teaching surface in the web engine. It has its own figures, scenarios, and explanatory framing.',
                    'Even if a learner never works through the full theory stack, this scale is useful for understanding how the engine packages conceptual material into guided visual surfaces.',
                ],
                bullets: [
                    'Own scenario and figure controls.',
                    'Pedagogy-heavy presentation layer.',
                    'Focused on explanation as much as simulation.',
                ],
                notation: ['Scale 11'],
                tags: ['scales', 'consciousness'],
            },
            {
                id: 'meta-unit',
                title: 'Meta / Existential Unit',
                shortTitle: 'Meta',
                summary: 'The meta-scale learning surface for existential-unit framing and inspection.',
                body: [
                    'The meta mode is a separate conceptual presentation layer that exposes existential-unit controls and information panels.',
                    'In practical UI terms, it acts as a high-level interpretive layer rather than a direct substrate or particle simulation view.',
                ],
                bullets: [
                    'Meta-scale educational framing.',
                    'Own control surface and information panel.',
                    'Interpretive layer above the ordinary scale ladder.',
                ],
                notation: ['Meta'],
                tags: ['scales', 'meta'],
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
        notation: ['|J|²', 'T ∝ exp(-2κW)', 'f_n ∝ n²', 'S = 2√2'],
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
    'Consciousness / Observer': {
        summary: 'Observer-facing seed structures tied to self-reference and recursive closure themes.',
        body: [
            'These scenarios are conceptual seeds for observer-like or self-referential structure rather than ordinary particle or optics demos.',
            'Their math language is recursive closure, ring or loop structure, fixed-point behavior, and the extent to which a local pattern can sustain self-related organization.',
        ],
        notation: ['sLoop', 'fixed point', 'recursive closure'],
        tags: ['observer', 'consciousness'],
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
        summary: 'A traveling localized pulse used to study propagation speed, packet broadening, and transport.',
        body: [
            'Flux Pulse is the cleanest “launch something and watch it move” scenario in Scale 0. It is ideal for seeing how a localized packet advances through the lattice and whether it preserves or sheds shape.',
            'Mathematically, this is a packet-propagation problem: group motion, amplitude falloff, packet width, and how local gradients move under the tick cycle.',
        ],
        notation: ['J(x,t)', 'packet width', 'group velocity', 'c = 1/√3'],
    },
    'flux-dipole': {
        summary: 'A two-source field pattern that emphasizes near-field structure and directional symmetry breaking.',
        body: [
            'Flux Dipole is the first place to connect the lattice visuals to familiar dipole intuition: two opposed sources producing a structured field between and around them.',
            'The math lens is dipole geometry, gradient alignment, and the difference between near-field directional structure and far-field decay.',
        ],
        notation: ['dipole moment', '∇|J|', 'near field', 'far field'],
    },
    'flux-standing': { summary: 'A standing-wave configuration with fixed nodes and antinodes in the flux field.', notation: ['J(x,t) = A sin(kx) cos(ωt)', 'nodes', 'antinodes'] },
    'flux-nested-standing': { summary: 'Several standing modes layered together so node structure appears across more than one spatial scale.', notation: ['mode superposition', 'nested nodes', 'harmonics'] },
    'flux-soliton': { summary: 'A localized packet intended to retain shape better than an ordinary dispersive pulse.', notation: ['localized packet', 'dispersion vs nonlinearity'] },
    'flux-interference': { summary: 'A many-source interference setup for reading fringe structure and phase cancellation.', notation: ['A_total = Σ A_i', 'I ∝ |A_total|²', 'fringes'] },
    'flux-vortex': { summary: 'A circulating flux pattern emphasizing angular structure and spin-like vorticity.', notation: ['∇×J', 'circulation', 'vorticity'] },
    'flux-dual-substrate': {
        summary: 'A dual-background field arrangement used to compare coupled substrate layers or handed sectors.',
        body: [
            'Dual Substrate is useful when you want to reason about two interlocked field sectors rather than one isolated wave pattern.',
            'The math to watch is symmetry and asymmetry between the coupled layers: phase offsets, handedness, and whether excitations lock, oppose, or exchange energy.',
        ],
        notation: ['coupled sectors', 'phase offset', 'chirality', 'Δφ'],
    },
    'flux-cascade': {
        summary: 'A branching manifestation setup where one activation can seed a wider growth pattern.',
        body: [
            'Genesis Cascade is about amplification: a local trigger causes subsequent sites or packets to activate in a visibly spreading way.',
            'The key math is threshold behavior and branching growth, not steady oscillation. Watch for local flux magnitude crossing a criterion and then recruiting neighboring structure.',
        ],
        notation: ['threshold', 'branching', '|J|', 'local amplification'],
    },
    'flux-random-genesis': {
        summary: 'A noisy or pseudo-random manifestation field used to study stochastic-looking emergence.',
        body: [
            'Random Genesis asks what structured observables still appear when the initial condition is noisy rather than cleanly prepared.',
            'The math focus is statistics over local events: distributions, hotspot frequency, and whether noise self-organizes into coherent patches.',
        ],
        notation: ['distribution', 'variance', 'noise floor', 'emergence'],
    },
    'flux-pair-production': { summary: 'A creation event where localized opposite manifestations emerge from a prepared high-flux configuration.', notation: ['pair creation', 'threshold crossing'] },
    'flux-annihilation': { summary: 'The inverse of pair production: opposite localized structures cancel and redistribute energy into the field.', notation: ['annihilation', 'energy redistribution'] },
    'flux-vacuum-foam': {
        summary: 'A fluctuation-heavy background used to illustrate vacuum-like transient structure.',
        body: [
            'Vacuum Fluctuations is not a claim that the engine literally reproduces continuum QFT vacuum exactly; it is a pedagogical fluctuation scenario inside the lattice substrate.',
            'Read it statistically: short-lived local excitations, fluctuation density, and how often transient structures cross the display threshold before decaying again.',
        ],
        notation: ['fluctuation density', 'transient excitation', 'threshold'],
    },
    'flux-meson': { summary: 'A two-endpoint confinement picture where the connective flux behaves like a string-like tube.', notation: ['flux tube', 'V(r) ≈ σr'] },
    'flux-string-breaking': { summary: 'A confinement setup where increasing separation can trigger string breaking rather than unlimited tension growth.', notation: ['σr', 'breaking threshold'] },
    'flux-baryon': { summary: 'A three-endpoint confinement picture with triadic connective structure.', notation: ['triad', 'three-body confinement'] },
    'flux-cyclotron': { summary: 'A curved-orbit scenario showing how an effective magnetic environment bends motion.', notation: ['q(v × B)', 'cyclotron radius'] },
    'flux-screening': { summary: 'A screening setup where an interaction is damped by medium response or nearby counter-structure.', notation: ['exp(-r/λ)', 'screened potential'] },
    'flux-thermalization': { summary: 'A relaxation scenario where initially structured energy spreads toward a more mixed state.', notation: ['equilibration', 'relaxation'] },
    'flux-triad': { summary: 'A three-body or three-site organization scenario for small-N stable structure.', notation: ['triad', 'three-body stability'] },
    'light-two-slit': { summary: 'A two-path optical interference setup rendered in lattice language.', notation: ['Δφ = 2πΔL/λ', 'interference fringes'] },
    'light-dipole': { summary: 'Radiation from a driven dipole-like source configuration.', notation: ['dipole radiation', 'far-field pattern'] },
    'light-rainbow': { summary: 'A color-separation or spectral-spread demonstration emphasizing wavelength dependence.', notation: ['λ', 'dispersion'] },
    'light-photon-race': { summary: 'A comparative propagation setup for pulse timing and path behavior.', notation: ['c = 1/√3', 'travel time'] },
    'quantum-born-rule': {
        summary: 'A measurement-statistics scenario testing whether accumulated outcomes track |J|².',
        body: [
            'Born Rule Test is one of the most formula-explicit Scale 0 scenarios. It is about repeated trials and histogram convergence, not about one visually impressive single shot.',
            'The quantity to watch is whether manifestation frequency approaches a probability law proportional to |J|² across many realizations.',
        ],
        notation: ['P(x) ∝ |J(x)|²', 'histogram', 'convergence'],
    },
    'quantum-double-slit': {
        summary: 'A quantitative interference setup where path difference is translated into fringe observables.',
        body: [
            'This scenario is the measurement-grade version of two-slit optics: not just “you can see fringes,” but “you can compare visibility and spacing to a wave model.”',
            'The math lens is phase difference, fringe visibility, and the detector intensity pattern built from coherent path superposition.',
        ],
        notation: ['V = (I_max-I_min)/(I_max+I_min)', 'Δφ', 'fringe spacing'],
    },
    'quantum-tunnel': {
        summary: 'Barrier penetration and transmission decay under a lattice analogue of a forbidden region.',
        body: [
            'Quantum Tunneling is about the part of the packet that gets through despite a barrier that would classically block it.',
            'What you should compare is transmission against barrier width or strength. The reference intuition is exponential suppression, not a hard zero-until-threshold cutoff.',
        ],
        notation: ['T ∝ exp(-2κW)', 'barrier width W', 'evanescent tail'],
    },
    'quantum-well': {
        summary: 'A boundary-conditioned standing-mode scenario for discrete spectral peaks.',
        body: [
            'Particle in a Box turns the lattice into a spectrum-reading exercise. You are not mainly looking for a pretty pattern; you are looking for discrete frequencies consistent with confinement.',
            'The relevant math is mode quantization: allowed standing modes, FFT peaks, and the n²-like scaling of confined energy levels.',
        ],
        notation: ['f_n ∝ n²', 'standing modes', 'FFT spectrum'],
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
        summary: 'A topological-phase scenario where enclosed flux matters even away from a local force region.',
        body: [
            'Aharonov-Bohm is the cleanest “global structure matters” lesson in the quantum-lab set.',
            'The thing to track is phase shift from enclosed flux and how that shift reappears when two paths recombine.',
        ],
        notation: ['Δφ ∝ Φ_enclosed', 'topological phase', 'path recombination'],
    },
    'quantum-casimir': {
        summary: 'A boundary-modified vacuum-energy scenario with plate separation as the control variable.',
        body: [
            'Casimir Effect turns geometry itself into an observable: change the boundary separation and the available fluctuation spectrum changes with it.',
            'The math lens is mode restriction between boundaries and a force law that scales sharply with distance.',
        ],
        notation: ['F ∝ 1/d⁴', 'boundary modes', 'plate separation d'],
    },
    'quantum-zeno': {
        summary: 'A repeated-measurement scenario where frequent probing can inhibit ordinary decay or transition.',
        body: [
            'Quantum Zeno is not about stronger forcing; it is about more frequent intervention.',
            'The math to watch is survival probability under repeated sampling and whether shortening the measurement interval suppresses change.',
        ],
        notation: ['survival probability', 'measurement interval', 'Zeno suppression'],
    },
    's0-seed-electron-l3': { summary: 'A dressed electron-style seed used to study whether a localized negative excitation persists under lattice dynamics.', notation: ['charge-like seed', 'envelope scale', 'stability'] },
    's0-seed-positron': { summary: 'A sign-flipped partner of the electron-style seed.', notation: ['opposite charge sign', 'dressing envelope'] },
    's0-seed-neutrino': { summary: 'A chiral, weakly manifest seed for minimal localized structure without strong charge-like dressing.', notation: ['chirality', 'weakly coupled seed'] },
    's0-seed-quark': { summary: 'A quark-labeled seed used pedagogically to discuss confinement-style behavior rather than free asymptotic propagation.', notation: ['confinement', 'color-sector caution', '[CONJECTURE]'] },
    's0-seed-antiquark': { summary: 'The anti-labeled partner of the quark seed, intended for pairing and confinement demonstrations.', notation: ['pairing', 'confinement', '[CONJECTURE]'] },
    's0-seed-pion': { summary: 'A two-constituent composite seed motivated by meson-style pairing.', notation: ['q-q̄ pair', 'bound composite'] },
    's0-seed-proton-l4': { summary: 'A three-constituent composite seed for proton-like triadic structure.', notation: ['three-body seed', 'triad', '[CONJECTURE]'] },
    's0-seed-neutron': { summary: 'A neutron-labeled triadic seed emphasizing composite persistence rather than electric charge display.', notation: ['neutral composite', 'triad'] },
    's0-seed-hydrogen': { summary: 'A lattice-level atom seed placing one light negative constituent around a heavy positive center.', notation: ['central potential', 'bound pair'] },
    's0-seed-helium': { summary: 'A multi-particle atom seed for compact central binding with more than one light constituent.', notation: ['many-body atom seed', 'screening'] },
    's0-seed-h2-molecule': { summary: 'A molecule-style seed that asks whether two atom-like centers can form a shared bound structure.', notation: ['bond formation', 'equilibrium separation'] },
    's0-seed-wilson-loop': { summary: 'A loop configuration intended to foreground holonomy and confinement-style area intuition.', notation: ['Wilson loop', 'area law', 'holonomy'] },
    's0-seed-flux-tube': { summary: 'A tube-like field bridge between separated endpoints.', notation: ['flux tube', 'σr'] },
    's0-seed-monopole': { summary: 'A monopole-style pedagogical seed for source-like field geometry.', notation: ['radial field', 'topological charge?'] },
    's0-seed-instanton': { summary: 'A localized topological event seed emphasizing global sector changes rather than static shape alone.', notation: ['topological event', 'instanton'] },
    's0-seed-schwarzschild': { summary: 'A gravity-facing well configuration for central-potential and horizon intuition.', notation: ['Φ(r)', 'r_s'] },
    's0-seed-frw-patch': { summary: 'A cosmological patch seed that frames the lattice with scale-factor language.', notation: ['a(t)', 'expansion patch'] },
    's0-seed-gravitational-wave': { summary: 'A wave-like spacetime-analogue configuration for strain and propagation intuition.', notation: ['h_μν', 'wave strain'] },
    's0-seed-sloop': { summary: 'A self-referential ring seed connected to the project’s sLoop language.', notation: ['sLoop', 'fixed point', 'recursive closure'] },
    's0-seed-observer-cell': { summary: 'A compact 3³ observer-style seed for local recursive organization.', notation: ['3³ cell', 'observer geometry'] },
    's0-field-plane-wave': { summary: 'A canonical plane-wave initial condition for reading wavelength, phase, and travel direction.', notation: ['A exp(i(kx-ωt))', 'k', 'ω'] },
    's0-field-standing-wave': { summary: 'A boundary-compatible field mode with fixed nodes and antinodes.', notation: ['sin(kx) cos(ωt)', 'nodes'] },
    's0-field-uniform-e': { summary: 'A uniform electric-like field background for testing force and drift intuition.', notation: ['uniform field', '∇·J ≈ const'] },
    's0-field-uniform-b': { summary: 'A uniform magnetic-like field background for curvature and circulation intuition.', notation: ['uniform curl structure', 'v × B'] },
    's0-field-photon-pulse': { summary: 'A clean propagating pulse used as the field-configuration counterpart of a light packet.', notation: ['transverse pulse', 'c = 1/√3'] },
    's0-field-electric-dipole': { summary: 'A canonical electric-dipole field configuration with clear symmetry axes.', notation: ['dipole moment', 'field lines'] },
    's0-field-magnetic-dipole': { summary: 'A canonical magnetic-dipole field configuration emphasizing circulation and loop structure.', notation: ['circulation', 'dipole loop'] },
    's0-field-vortex-line': { summary: 'A line-like vorticity configuration for curl-dominated structure.', notation: ['∇×J', 'vortex core'] },
});

const PARTICLE_SCENARIO_GUIDES = Object.freeze({
    'pe-hydrogen': 'Hydrogen is the baseline particle-engine atom: one attractive Coulomb channel, one reduced mass, and the cleanest route to comparing orbital size and binding scale.',
    'pe-helium': 'Helium adds electron-electron repulsion on top of nuclear attraction, so it is the first place where screening and genuine many-body balance matter more than the textbook two-body picture.',
    'pe-positronium': 'Positronium is valuable because the two constituents have equal mass. That changes the reduced mass and makes the center-of-mass problem unusually symmetric.',
    'pe-muonium': 'Muonium keeps the hydrogenic charge pattern but changes the mass hierarchy, which means orbit size, timescale, and spectral scale all shift through μ.',
    'pe-true-muonium': 'True muonium is a heavy equal-mass lepton pair, so the main lesson is how binding tightens as the constituent mass scale rises.',
    'pe-tauonium': 'Tauonium is so heavy and short-lived that the main math question is not just binding but whether the dynamical timescale competes with decay and relativistic effects.',
    'pe-tau-atom': 'Tauonic hydrogen shrinks the hydrogenic orbit dramatically because the orbiting lepton is much heavier than the electron.',
    'pe-pionic-hydrogen': 'Pionic hydrogen is a Coulomb-plus-hadronic system, so you should think in terms of an electromagnetic baseline modified by a short-range correction.',
    'pe-kaonic-hydrogen': 'Kaonic hydrogen pushes farther into the regime where reduced mass and short-range structure both matter, making it a useful comparison to pionic hydrogen.',
    'pe-sigma-plus-atom': 'This is a baryon-electron bound-state thought experiment where the core has a different charge/mass identity from a proton while the mathematics still starts from a central attractive potential.',
    'pe-antiprotonic-hydrogen': 'Protonium sits at the intersection of binding and annihilation. The interesting question is not only whether a bound orbit forms, but how long it survives before inelastic channels matter.',
    'pe-pion-orbit': 'Pionium is a mesonic analogue of positronium: a charged equal-sign-opposite pair where reduced mass and annihilation-style channels shape the dynamics.',
    'pe-kaon-pair': 'Kaonium is the heavier cousin of pionium, useful for comparing how bound-state scale changes with constituent mass.',
    'pe-delta-system': 'Delta++ plus two electrons is a high-charge many-body balance problem where attraction to the center competes with strong electron-electron repulsion.',
    'pe-omega-scattering': 'Omega scattering is primarily a kinematics lesson: impact parameter, momentum transfer, and deflection are more important than long-lived binding.',
    'pe-deuteron': 'Deuteron is the first compact nuclear composite in the particle engine, so the educational focus shifts from atomic orbit language to few-body binding and effective nuclear cohesion.',
    'pe-tritium': 'Tritium adds one more neutral constituent, which changes inertia and binding balance without simply scaling hydrogen by a constant factor.',
    'pe-helion': 'Helion combines few-body nuclear structure with atomic electrons, so it is a bridge case between nuclear composition and atomic presentation.',
    'pe-w-pair': 'The W-pair scenario is best read through relativistic kinematics rather than static atomic intuition.',
    'pe-scattering': 'This is the clean Rutherford-style comparison case: incoming particle, impact parameter, deflection angle, and momentum transfer.',
    'pe-three-body': 'Three-body particle dynamics is where intuition starts to break. Small changes in initial condition can radically change whether the system binds, scatters, or reconfigures.',
    'pe-meson-scattering': 'Meson scattering asks how a light hadronic projectile transfers momentum and bends off a proton target.',
    'pe-muon-scattering': 'Muon scattering is a nice comparison to electron scattering because the projectile is heavier and less easily deflected.',
    'pe-micro-bh': 'Micro black hole is the gravity-dominant outlier in the particle-engine set. Watch capture, escape velocity, and whether nearby constituents accrete or sling past.',
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

const CONSCIOUSNESS_SCENARIO_GUIDES = Object.freeze({
    'cs-threshold': 'Threshold Crossing is the discriminant lesson: the scenario is organized around when Δ_k changes sign and the root structure moves from real to complex.',
    'cs-high-coupling': 'High Coupling is the “well above threshold” comparison case, where strong intensity and stabilized observer-like patterns are the main visual claim.',
    'cs-self-ref': 'Self-Reference is the simplest sLoop fixed-point scenario, so recursive closure matters more than external forcing.',
    'cs-nested-sloop': 'Nested sLoop adds recursion depth, turning one fixed-point story into a hierarchy of self-reference.',
    'cs-chirality': 'Chirality Split is about left/right asymmetry and sector imbalance rather than simple magnitude increase.',
    'cs-boundary-orbit': 'Boundary Orbit imports iterative-complex-dynamics language into the consciousness pedagogy surface via z → z² + c.',
    'cs-entangled': 'Entangled Pair is the consciousness-side correlation case, tying observer-language framing to Bell-type structure.',
    'cs-flow': 'Flow State is the outward-leaning phase regime: lower effective angle, fast coherent motion, and object-dominant dynamics.',
    'cs-meditation': 'Meditation is the inward-leaning phase regime: higher effective angle, slower resonance, and subject-dominant balance.',
    'cs-custom': 'Custom consciousness mode is the pedagogical sandbox for trying your own observer-oriented parameter mix.',
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
            'Helpful controls: Coulomb, gravity, damping, advanced-force toggles, time step, and softening.',
        ],
        notation,
        tags: ['scale1', 'particle-engine'],
    });
}

const PARTICLE_SCENARIO_ENTRIES = Object.freeze([
    makeParticleScenario('pe-hydrogen', 'Hydrogen Atom (p + e−)', 'a Coulomb-bound two-body atom.', ['V(r) = -α/r', 'μ', 'E_n = -μ α²/(2n²)', 'a₀ = 1/(μ α)']),
    makeParticleScenario('pe-helium', 'Helium Atom (He²⁺ + 2e−)', 'a three-body Coulomb problem with electron-electron repulsion.', ['ΣV_ij', 'screening', 'many-body bound state']),
    makeParticleScenario('pe-positronium', 'Positronium (e⁺e−)', 'an equal-mass bound state of matter and antimatter.', ['μ = m_e/2', 'E_n ∝ -μ α²']),
    makeParticleScenario('pe-muonium', 'Muonium (μ⁺e−)', 'a hydrogen-like exotic atom with a different reduced mass.', ['μ', 'Rydberg scaling']),
    makeParticleScenario('pe-true-muonium', 'True Muonium (μ⁺μ−)', 'a heavier equal-mass leptonic bound state.', ['μ = m_μ/2', 'compact bound state']),
    makeParticleScenario('pe-tauonium', 'Tauonium (τ⁺τ−)', 'an ultra-heavy leptonic atom with short lifetime scales.', ['μ = m_τ/2', 'E² = p² + m²']),
    makeParticleScenario('pe-tau-atom', 'Tauonic Hydrogen (τ− + p)', 'a hydrogenic atom where the orbiting lepton is much heavier than the electron.', ['μ', 'smaller Bohr radius']),
    makeParticleScenario('pe-pionic-hydrogen', 'Pionic Hydrogen (π− + p)', 'a hadron-facing exotic atom.', ['Coulomb + short-range corrections', 'reduced mass']),
    makeParticleScenario('pe-kaonic-hydrogen', 'Kaonic Hydrogen (K− + p)', 'a heavier exotic atom sensitive to both binding and short-range interactions.', ['μ', 'short-range potential']),
    makeParticleScenario('pe-sigma-plus-atom', 'Sigma⁺ Atom (Σ⁺ + e−)', 'a baryon-electron bound system with different core mass and charge structure.', ['V(r) = -α/r', 'μ']),
    makeParticleScenario('pe-antiprotonic-hydrogen', 'Protonium (p̄ + p)', 'a matter-antimatter proton pair with annihilation and scattering intuition.', ['annihilation channel', 'two-body bound state']),
    makeParticleScenario('pe-pion-orbit', 'Pionium (π⁺π−)', 'a mesonic bound state of opposite charges.', ['μ', 'two-body Coulomb problem']),
    makeParticleScenario('pe-kaon-pair', 'Kaonium (K⁺K−)', 'a heavier mesonic bound state used to compare mass scaling and binding.', ['μ', 'bound-state scaling']),
    makeParticleScenario('pe-delta-system', 'Delta++ System (Δ++ + 2e−)', 'a multi-charge bound scenario with strong repulsion/attraction balance.', ['many-body Coulomb', 'net charge']),
    makeParticleScenario('pe-omega-scattering', 'Omega− Scattering (Ω− + e⁺)', 'a scattering-focused heavy-hadron interaction setup.', ['impact parameter', 'deflection angle']),
    makeParticleScenario('pe-deuteron', 'Deuteron (p + n + e−)', 'a nuclear composite with extra neutral mass and binding structure.', ['binding energy', 'three-body dynamics']),
    makeParticleScenario('pe-tritium', 'Tritium (p + 2n + e−)', 'a heavier nuclear bound-state approximation.', ['few-body dynamics', 'binding vs decay']),
    makeParticleScenario('pe-helion', 'Helion / He-3 (2p + n + 2e−)', 'a compact few-body nuclear/atomic composite.', ['few-body Coulomb + binding']),
    makeParticleScenario('pe-w-pair', 'W⁺W− Pair', 'a relativistic bosonic pair rather than an atomic bound state.', ['E² = p² + m_W²', 'pair kinematics']),
    makeParticleScenario('pe-scattering', 'Proton-Electron Scattering', 'a clean two-body scattering experiment.', ['dσ/dΩ', 'impact parameter', 'momentum transfer q']),
    makeParticleScenario('pe-three-body', 'Three-Body (p⁺ p⁺ e−)', 'a nontrivial few-body balance between attraction and repulsion.', ['few-body instability', 'Coulomb balance']),
    makeParticleScenario('pe-meson-scattering', 'π⁺ off Proton', 'a hadron-proton scattering test.', ['impact parameter', 'scattering angle']),
    makeParticleScenario('pe-muon-scattering', 'μ− off Proton', 'a lepton-proton scattering comparison case.', ['momentum transfer', 'Rutherford-like scattering']),
    makeParticleScenario('pe-micro-bh', 'Micro Black Hole (Accretion)', 'gravitational collapse and accretion behavior at the particle-engine scale.', ['r_s = 2GM/c²', 'accretion', 'escape velocity']),
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
            notation: molecule.category === 'diatomic' ? ['r_eq', 'vibrational mode'] : MOLECULE_CATEGORY_MATH[molecule.category] || ['bond geometry'],
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
    ['cosmic-baryogenesis', 'Baryogenesis', 'matter-antimatter imbalance at cosmological scale.', ['η = (n_B - n_B̄)/n_γ', 'asymmetry generation']],
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

const CONSCIOUSNESS_SCENARIO_TITLES = Object.freeze({
    'cs-threshold': 'Threshold Crossing',
    'cs-high-coupling': 'High Coupling',
    'cs-self-ref': 'Self-Reference',
    'cs-nested-sloop': 'Nested sLoop',
    'cs-chirality': 'Chirality Split',
    'cs-boundary-orbit': 'Boundary Orbit',
    'cs-entangled': 'Entangled Pair',
    'cs-flow': 'Flow State',
    'cs-meditation': 'Meditation',
    'cs-custom': 'Custom (Manual)',
});

const CONSCIOUSNESS_SCENARIO_ENTRIES = Object.freeze(
    Object.entries(CONSCIOUSNESS_SCENARIO_TITLES).map(([id, title]) =>
        makeScenarioEntry({
            id,
            title: `${title} [Scale 11]`,
            shortTitle: title,
            scale: 'Scale 11 / Consciousness',
            summary: CS_SCENARIO_DESCRIPTIONS[id] || 'A consciousness-mode pedagogical scenario.',
            body: [
                `${title} is a consciousness-mode teaching scenario, so it should be read as a conceptual demonstration as much as a dynamical one.`,
                CONSCIOUSNESS_SCENARIO_GUIDES[id],
                'The math focus depends on the case: threshold discriminants, complex roots, self-reference fixed points, chirality, Bell correlations, or subject/object phase balance.',
            ],
            bullets: [
                id === 'cs-custom' ? 'Custom scenario for user-defined pedagogical exploration.' : 'Best read together with the consciousness description panel and figure selector.',
            ],
            notation: dedupe(
                id === 'cs-threshold' ? ['Δ_k', 'K_C ≈ 3.60', 'real → complex roots']
                    : id.includes('sloop') ? ['x² = K(x − G*)', 'fixed point', 'recursive closure']
                    : id === 'cs-boundary-orbit' ? ['z → z² + c', 'c = 1/G*']
                    : id === 'cs-entangled' ? ['S = 2√2', 'correlation']
                    : id === 'cs-flow' || id === 'cs-meditation' ? ['θ', '52.54° threshold']
                    : ['J(v,t)', '|J|', 'complexification'],
            ),
            tags: ['scale11', 'consciousness'],
        }),
    ),
);

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
        title: 'Scenarios: Scales 4-5-11',
        description: 'Planetary, cosmic, and consciousness-mode scenarios with their conceptual math framing.',
        entries: [...PLANETARY_SCENARIO_ENTRIES, ...COSMIC_SCENARIO_ENTRIES, ...CONSCIOUSNESS_SCENARIO_ENTRIES],
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
