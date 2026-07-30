/** Knowledge-base section `runtime` */
export const SECTION_RUNTIME = Object.freeze({
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
    });
