/** Knowledge-base section `scales` */
export const SECTION_SCALES = Object.freeze({
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
    });
