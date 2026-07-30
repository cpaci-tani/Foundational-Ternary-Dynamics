/** Knowledge-base section `foundations` */
export const SECTION_FOUNDATIONS = Object.freeze({
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
    });
