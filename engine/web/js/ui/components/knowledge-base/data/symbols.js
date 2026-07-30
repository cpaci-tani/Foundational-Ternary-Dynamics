/** Knowledge-base section `symbols` */
export const SECTION_SYMBOLS = Object.freeze({
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
    });
