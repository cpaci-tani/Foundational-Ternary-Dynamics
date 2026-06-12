/**
 * Molecular Library — 25 molecules for Scale 2 (AtomEngine).
 *
 * Each molecule is a data object: atom positions, charges, metadata.
 * Positions in Bohr-scaled simulation units (S≈5).
 * Molecules centered at origin, principal axis along x, planar in xy.
 *
 * Auto-bonding threshold: 1.2 × sigma_avg.
 *   H σ≈4.0  C σ≈2.2  N σ≈2.1  O σ≈2.0  S σ≈1.6  Cl σ≈1.55  Na σ≈1.79
 *   H-H threshold ≈ 4.8    C-H ≈ 3.7    C-C ≈ 2.6
 *   O-H ≈ 3.6              N-H ≈ 3.7    C-O ≈ 2.5
 *
 * BOND-ORDER INFERENCE (audit P0-12/P0-13, resolved 2026-05-27): the
 * auto-bonder establishes connectivity only; bond orders (double / triple /
 * aromatic) are then inferred by `_aeInferBondOrders()` in mock-atom-engine.js
 * from valence saturation + interatomic distance, and the molecular renderer
 * draws 2 / 3 / delocalised cylinders accordingly. Geometries below are sized
 * so every advertised multiple bond is within the auto-bond threshold; the
 * descriptions name the order the visualization actually shows.
 */

// ── Categories ───────────────────────────────────────────────────────

const CATEGORIES = [
    { id: 'diatomic',    label: 'Diatomic Molecules', order: 1 },
    { id: 'inorganic',   label: 'Simple Inorganic',   order: 2 },
    { id: 'organic',     label: 'Organic Molecules',   order: 3 },
    { id: 'biochemical', label: 'Biochemical & Crystals', order: 4 },
];

// ── Helper: tetrahedral vertices ─────────────────────────────────────

function tetra(d) {
    const t = 1 / Math.sqrt(3);
    return [
        { x:  d * t, y:  d * t, z:  d * t },
        { x:  d * t, y: -d * t, z: -d * t },
        { x: -d * t, y:  d * t, z: -d * t },
        { x: -d * t, y: -d * t, z:  d * t },
    ];
}

// ── Helper: planar ring vertices ─────────────────────────────────────

function ring(n, r) {
    const pts = [];
    for (let i = 0; i < n; i++) {
        const angle = (2 * Math.PI * i) / n;
        pts.push({ x: r * Math.cos(angle), y: r * Math.sin(angle), z: 0 });
    }
    return pts;
}

// ── Molecule Data ────────────────────────────────────────────────────

const MOLECULES = [

    // ═══════════════════════════════════════════════════════════════════
    // DIATOMIC
    // ═══════════════════════════════════════════════════════════════════

    {
        id: 'h2', name: 'Hydrogen', formula: 'H<sub>2</sub>',
        category: 'diatomic',
        description: 'Simplest covalent molecule — two hydrogen atoms sharing electrons.',
        atoms: [
            { Z: 1, x: -2.2, y: 0, z: 0, vx:  0.05 },
            { Z: 1, x:  2.2, y: 0, z: 0, vx: -0.05 },
        ],
        cameraDistance: 20,
    },
    {
        id: 'o2', name: 'Oxygen', formula: 'O<sub>2</sub>',
        category: 'diatomic',
        // 2026-05-27 audit P0-12/P0-13: O-O was 3.0 apart, outside the
        // auto-bond threshold (1.2·σ_avg = 2.4), so no bond formed at all.
        // Shortened to 2.2 (≈ r_eq, < 2.4) so the bond forms and valence
        // saturation infers it as a double (O wants 2 bonds, has 1).
        description: 'Diatomic oxygen — O=O double bond (inferred from valence saturation); paramagnetism not modelled.',
        atoms: [
            { Z: 8, x: -1.1, y: 0, z: 0, vx:  0.05 },
            { Z: 8, x:  1.1, y: 0, z: 0, vx: -0.05 },
        ],
        cameraDistance: 20,
    },
    {
        id: 'n2', name: 'Nitrogen', formula: 'N<sub>2</sub>',
        category: 'diatomic',
        // 2026-05-27 audit P0-12/P0-13: N-N was 3.0 apart, outside the
        // auto-bond threshold (1.2·σ_avg = 2.51), so no bond formed. Shortened
        // to 2.3 (≈ r_eq, < 2.51) so the bond forms and valence saturation
        // infers it as a triple (N wants 3 bonds, has 1).
        description: 'Diatomic nitrogen — N≡N triple bond (inferred from valence saturation); extremely stable.',
        atoms: [
            { Z: 7, x: -1.15, y: 0, z: 0, vx:  0.05 },
            { Z: 7, x:  1.15, y: 0, z: 0, vx: -0.05 },
        ],
        cameraDistance: 20,
    },
    {
        id: 'hcl', name: 'Hydrogen Chloride', formula: 'HCl',
        category: 'diatomic',
        description: 'Polar covalent bond — electronegativity difference.',
        atoms: [
            { Z: 1,  x: -1.5, y: 0, z: 0, vx:  0.05 },
            { Z: 17, x:  1.5, y: 0, z: 0, vx: -0.05 },
        ],
        cameraDistance: 20,
    },

    // ═══════════════════════════════════════════════════════════════════
    // SIMPLE INORGANIC
    // ═══════════════════════════════════════════════════════════════════

    {
        id: 'water', name: 'Water', formula: 'H<sub>2</sub>O',
        category: 'inorganic',
        description: 'Bent geometry (104.5°) — universal solvent, life\'s medium.',
        atoms: (() => {
            const angle = 104.5 * Math.PI / 180;
            const r = 3.4;  // within O-H auto-bond threshold (3.6)
            return [
                { Z: 8, x: 0, y: 0, z: 0 },
                { Z: 1, x: r, y: 0, z: 0 },
                { Z: 1, x: r * Math.cos(angle), y: r * Math.sin(angle), z: 0 },
            ];
        })(),
        cameraDistance: 20,
    },
    {
        id: 'nacl', name: 'Sodium Chloride', formula: 'NaCl',
        category: 'inorganic',
        description: 'Ionic bond — electrostatic attraction between Na⁺ and Cl⁻.',
        atoms: [
            { Z: 11, x: -7.5, y: 0, z: 0, vx:  0.05, charge: 1 },
            { Z: 17, x:  7.5, y: 0, z: 0, vx: -0.05, charge: -1 },
        ],
        cameraDistance: 30,
    },
    {
        id: 'noble', name: 'Noble Gases', formula: 'He + Ne',
        category: 'inorganic',
        description: 'Van der Waals interaction only — no covalent or ionic bonds.',
        atoms: [
            { Z: 2,  x: -4.0, y: 0, z: 0, vx:  0.1 },
            { Z: 10, x:  4.0, y: 0, z: 0, vx: -0.1 },
        ],
        cameraDistance: 20,
    },
    {
        id: 'co2', name: 'Carbon Dioxide', formula: 'CO<sub>2</sub>',
        category: 'inorganic',
        description: 'Linear molecule (180°) — greenhouse gas; two C=O double bonds (inferred from valence saturation).',
        atoms: [
            { Z: 8, x: -2.5, y: 0, z: 0 },
            { Z: 6, x:  0,   y: 0, z: 0 },
            { Z: 8, x:  2.5, y: 0, z: 0 },
        ],
        cameraDistance: 20,
    },
    {
        id: 'nh3', name: 'Ammonia', formula: 'NH<sub>3</sub>',
        category: 'inorganic',
        description: 'Trigonal pyramidal (107.8°) — lone pair pushes geometry.',
        atoms: (() => {
            // N at apex, 3 H below in a triangle
            const bondLen = 3.5;
            const angle = 107.8 * Math.PI / 180;
            const h = bondLen * Math.cos((Math.PI - angle) / 2);
            const r = bondLen * Math.sin((Math.PI - angle) / 2);
            return [
                { Z: 7, x: 0, y: h * 0.5, z: 0 },
                { Z: 1, x: r,              y: -h * 0.5, z: 0 },
                { Z: 1, x: -r * 0.5,       y: -h * 0.5, z:  r * 0.866 },
                { Z: 1, x: -r * 0.5,       y: -h * 0.5, z: -r * 0.866 },
            ];
        })(),
        cameraDistance: 20,
    },
    {
        id: 'h2o2', name: 'Hydrogen Peroxide', formula: 'H<sub>2</sub>O<sub>2</sub>',
        category: 'inorganic',
        description: 'O-O bond with dihedral angle — 3D geometry.',
        atoms: [
            { Z: 8, x: -1.0, y: 0, z: 0 },
            { Z: 8, x:  1.0, y: 0, z: 0 },
            { Z: 1, x: -2.8, y:  2.0, z: 0 },
            { Z: 1, x:  2.8, y: -1.0, z: 1.7 },  // dihedral ~111°
        ],
        cameraDistance: 22,
    },
    {
        id: 'h2s', name: 'Hydrogen Sulfide', formula: 'H<sub>2</sub>S',
        category: 'inorganic',
        description: 'Bent geometry (92.1°) — compare with water\'s 104.5°.',
        atoms: (() => {
            const angle = 92.1 * Math.PI / 180;
            const r = 3.2;  // within S-H auto-bond threshold (3.35)
            return [
                { Z: 16, x: 0, y: 0, z: 0 },
                { Z: 1,  x: r, y: 0, z: 0 },
                { Z: 1,  x: r * Math.cos(angle), y: r * Math.sin(angle), z: 0 },
            ];
        })(),
        cameraDistance: 22,
    },

    // ═══════════════════════════════════════════════════════════════════
    // ORGANIC
    // ═══════════════════════════════════════════════════════════════════

    {
        id: 'methane', name: 'Methane', formula: 'CH<sub>4</sub>',
        category: 'organic',
        description: 'Perfect tetrahedral geometry (109.5°) — simplest alkane.',
        atoms: (() => {
            const verts = tetra(3.5);  // within C-H auto-bond threshold (3.72)
            return [
                { Z: 6, x: 0, y: 0, z: 0 },
                ...verts.map(v => ({ Z: 1, x: v.x, y: v.y, z: v.z })),
            ];
        })(),
        cameraDistance: 22,
    },
    {
        id: 'ethane', name: 'Ethane', formula: 'C<sub>2</sub>H<sub>6</sub>',
        category: 'organic',
        description: 'C-C single bond — two methyl groups in staggered configuration.',
        atoms: (() => {
            const cc = 2.2;  // C-C bond within auto-bond range
            const ch = 3.2;  // C-H distance
            const atoms = [
                { Z: 6, x: -cc / 2, y: 0, z: 0 },
                { Z: 6, x:  cc / 2, y: 0, z: 0 },
            ];
            // 3 H around C1 (pointing -x direction)
            const a1 = [
                { x: -ch * 0.9, y:  ch * 0.5, z: 0 },
                { x: -ch * 0.9, y: -ch * 0.25, z:  ch * 0.43 },
                { x: -ch * 0.9, y: -ch * 0.25, z: -ch * 0.43 },
            ];
            // 3 H around C2 (pointing +x direction, staggered)
            const a2 = [
                { x:  ch * 0.9, y: -ch * 0.5, z: 0 },
                { x:  ch * 0.9, y:  ch * 0.25, z: -ch * 0.43 },
                { x:  ch * 0.9, y:  ch * 0.25, z:  ch * 0.43 },
            ];
            for (const h of a1) atoms.push({ Z: 1, x: h.x - cc / 2, y: h.y, z: h.z });
            for (const h of a2) atoms.push({ Z: 1, x: h.x + cc / 2, y: h.y, z: h.z });
            return atoms;
        })(),
        cameraDistance: 25,
    },
    {
        id: 'ethylene', name: 'Ethylene', formula: 'C<sub>2</sub>H<sub>4</sub>',
        category: 'organic',
        description: 'C=C double bond (inferred from valence saturation) — planar molecule, 120° angles.',
        atoms: (() => {
            const cc = 2.0;
            const ch = 3.2;
            const a120 = 2 * Math.PI / 3;
            return [
                { Z: 6, x: -cc / 2, y: 0, z: 0 },
                { Z: 6, x:  cc / 2, y: 0, z: 0 },
                // H on C1
                { Z: 1, x: -cc / 2 + ch * Math.cos(a120 * 0.5 + Math.PI), y:  ch * Math.sin(a120 * 0.5), z: 0 },
                { Z: 1, x: -cc / 2 + ch * Math.cos(a120 * 0.5 + Math.PI), y: -ch * Math.sin(a120 * 0.5), z: 0 },
                // H on C2
                { Z: 1, x:  cc / 2 + ch * Math.cos(a120 * 0.5), y:  ch * Math.sin(a120 * 0.5), z: 0 },
                { Z: 1, x:  cc / 2 + ch * Math.cos(a120 * 0.5), y: -ch * Math.sin(a120 * 0.5), z: 0 },
            ];
        })(),
        cameraDistance: 25,
    },
    {
        id: 'acetylene', name: 'Acetylene', formula: 'C<sub>2</sub>H<sub>2</sub>',
        category: 'organic',
        // 2026-05-27 audit P0-12 fix: shortened C-C from 4.0 → 2.4 (within
        // auto-bond threshold 2.64) so the central bond actually forms.
        // H-C distance kept at 3.5 (< 3.72 threshold). Valence saturation
        // then infers the central bond as a triple (C wants 4, has 2 bonds).
        description: 'C≡C triple bond (inferred from valence saturation) — linear molecule.',
        atoms: [
            { Z: 1, x: -4.7, y: 0, z: 0 },
            { Z: 6, x: -1.2, y: 0, z: 0 },
            { Z: 6, x:  1.2, y: 0, z: 0 },
            { Z: 1, x:  4.7, y: 0, z: 0 },
        ],
        cameraDistance: 22,
    },
    {
        id: 'methanol', name: 'Methanol', formula: 'CH<sub>3</sub>OH',
        category: 'organic',
        description: 'Hydroxyl group (-OH) — simplest alcohol.',
        atoms: (() => {
            const atoms = [
                { Z: 6, x: 0, y: 0, z: 0 },        // C center
                { Z: 8, x: 2.5, y: 0, z: 0 },      // O bonded to C
                { Z: 1, x: 4.5, y: 1.0, z: 0 },    // H on O
            ];
            // 3 H on the methyl group (tetrahedral, pointing -x)
            const ch = 3.2;
            atoms.push({ Z: 1, x: -ch * 0.85, y:  ch * 0.5, z: 0 });
            atoms.push({ Z: 1, x: -ch * 0.85, y: -ch * 0.25, z:  ch * 0.43 });
            atoms.push({ Z: 1, x: -ch * 0.85, y: -ch * 0.25, z: -ch * 0.43 });
            return atoms;
        })(),
        cameraDistance: 25,
    },
    {
        id: 'formaldehyde', name: 'Formaldehyde', formula: 'CH<sub>2</sub>O',
        category: 'organic',
        description: 'C=O carbonyl group (inferred from valence saturation) — trigonal planar geometry.',
        atoms: [
            { Z: 6, x: 0, y: 0, z: 0 },
            { Z: 8, x: 2.5, y: 0, z: 0 },
            { Z: 1, x: -2.0, y:  2.5, z: 0 },
            { Z: 1, x: -2.0, y: -2.5, z: 0 },
        ],
        cameraDistance: 22,
    },
    {
        id: 'benzene', name: 'Benzene', formula: 'C<sub>6</sub>H<sub>6</sub>',
        category: 'organic',
        description: 'Aromatic ring — six-fold symmetry, delocalized electrons.',
        atoms: (() => {
            const cRing = ring(6, 2.5);
            const hRing = ring(6, 5.5);
            const atoms = [];
            for (let i = 0; i < 6; i++) {
                atoms.push({ Z: 6, x: cRing[i].x, y: cRing[i].y, z: 0 });
            }
            for (let i = 0; i < 6; i++) {
                atoms.push({ Z: 1, x: hRing[i].x, y: hRing[i].y, z: 0 });
            }
            return atoms;
        })(),
        cameraDistance: 30,
    },
    {
        id: 'ethanol', name: 'Ethanol', formula: 'C<sub>2</sub>H<sub>5</sub>OH',
        category: 'organic',
        description: 'Common alcohol — C-C-O chain with hydroxyl group.',
        atoms: (() => {
            const ch = 3.2;
            const atoms = [
                // Carbon chain + oxygen
                { Z: 6, x: -2.5, y: 0, z: 0 },     // C1 (methyl)
                { Z: 6, x:  0,   y: 0, z: 0 },     // C2
                { Z: 8, x:  2.5, y: 0, z: 0 },     // O
                { Z: 1, x:  4.5, y: 1.0, z: 0 },   // H on O
            ];
            // 3 H on C1 (methyl)
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y:  ch * 0.5, z: 0 });
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y: -ch * 0.25, z:  ch * 0.43 });
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y: -ch * 0.25, z: -ch * 0.43 });
            // 2 H on C2
            atoms.push({ Z: 1, x: 0, y:  ch * 0.55, z:  ch * 0.35 });
            atoms.push({ Z: 1, x: 0, y: -ch * 0.55, z: -ch * 0.35 });
            return atoms;
        })(),
        cameraDistance: 28,
    },
    {
        id: 'acetic_acid', name: 'Acetic Acid', formula: 'CH<sub>3</sub>COOH',
        category: 'organic',
        description: 'Carboxyl group (-COOH) — simplest organic acid (vinegar).',
        atoms: (() => {
            const ch = 3.2;
            const atoms = [
                // Backbone
                { Z: 6, x: -2.5, y: 0, z: 0 },     // C1 (methyl)
                { Z: 6, x:  0,   y: 0, z: 0 },     // C2 (carboxyl carbon)
                { Z: 8, x:  0,   y: 2.5, z: 0 },   // =O (carbonyl)
                { Z: 8, x:  2.5, y: 0, z: 0 },     // -OH
                { Z: 1, x:  4.5, y: 0.8, z: 0 },   // H on OH
            ];
            // 3 H on methyl
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y:  ch * 0.5, z: 0 });
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y: -ch * 0.25, z:  ch * 0.43 });
            atoms.push({ Z: 1, x: -2.5 - ch * 0.85, y: -ch * 0.25, z: -ch * 0.43 });
            return atoms;
        })(),
        cameraDistance: 28,
    },

    // ═══════════════════════════════════════════════════════════════════
    // BIOCHEMICAL & CRYSTALS
    // ═══════════════════════════════════════════════════════════════════

    {
        id: 'glycine', name: 'Glycine', formula: 'C<sub>2</sub>H<sub>5</sub>NO<sub>2</sub>',
        category: 'biochemical',
        description: 'Simplest amino acid — H₂N-CH₂-COOH, building block of proteins.',
        atoms: (() => {
            const atoms = [
                // Backbone: NH2-CH2-COOH
                { Z: 7, x: -5.0, y: 0, z: 0 },     // N (amino)
                { Z: 6, x: -2.0, y: 0, z: 0 },     // C (alpha carbon)
                { Z: 6, x:  1.0, y: 0, z: 0 },     // C (carboxyl carbon)
                { Z: 8, x:  1.0, y: 2.5, z: 0 },   // =O
                { Z: 8, x:  3.5, y: 0, z: 0 },     // -OH
                // Hydrogens
                { Z: 1, x: -6.5, y:  1.5, z: 0 },  // H on N
                { Z: 1, x: -6.5, y: -1.5, z: 0 },  // H on N
                { Z: 1, x: -2.0, y:  2.5, z: 0 },  // H on alpha-C
                { Z: 1, x: -2.0, y: -1.5, z: 2.0 },// H on alpha-C
                { Z: 1, x:  5.0, y: 0.8, z: 0 },   // H on OH
            ];
            return atoms;
        })(),
        cameraDistance: 30,
    },
    {
        id: 'urea', name: 'Urea', formula: 'CO(NH<sub>2</sub>)<sub>2</sub>',
        category: 'biochemical',
        description: 'First organic compound synthesized from inorganic — planar C center.',
        atoms: [
            // Central C with =O and two NH2
            { Z: 6, x: 0, y: 0, z: 0 },
            { Z: 8, x: 0, y: 2.5, z: 0 },         // =O (top)
            { Z: 7, x: -2.5, y: -1.5, z: 0 },     // N1
            { Z: 7, x:  2.5, y: -1.5, z: 0 },     // N2
            // H on N1
            { Z: 1, x: -4.5, y: -0.8, z: 0 },
            { Z: 1, x: -2.5, y: -3.5, z: 0 },
            // H on N2
            { Z: 1, x:  4.5, y: -0.8, z: 0 },
            { Z: 1, x:  2.5, y: -3.5, z: 0 },
        ],
        cameraDistance: 28,
    },
    {
        id: 'adenine', name: 'Adenine', formula: 'C<sub>5</sub>H<sub>5</sub>N<sub>5</sub>',
        category: 'biochemical',
        description: 'Purine base — found in DNA, RNA, and ATP.',
        atoms: (() => {
            // Purine: fused 6-membered + 5-membered ring, planar
            // 6-ring (pyrimidine): N1-C2-N3-C4-C5-C6
            // 5-ring (imidazole): C4-C5-N7-C8-N9
            const s = 2.2;  // ring bond length scale
            const atoms = [
                // 6-membered ring (positions approximate)
                { Z: 7, x: -3.0 * s / 2, y: -1.0,    z: 0 },  // N1
                { Z: 6, x: -2.0 * s / 2, y:  1.0,    z: 0 },  // C2
                { Z: 7, x:  0,           y:  1.8,    z: 0 },  // N3
                { Z: 6, x:  1.5 * s / 2, y:  0.5,    z: 0 },  // C4
                { Z: 6, x:  1.0 * s / 2, y: -1.5,    z: 0 },  // C5
                { Z: 6, x: -1.0 * s / 2, y: -2.0,    z: 0 },  // C6
                // 5-membered ring extension
                { Z: 7, x:  3.0 * s / 2, y: -1.0,    z: 0 },  // N7
                { Z: 6, x:  3.5 * s / 2, y:  0.8,    z: 0 },  // C8
                { Z: 7, x:  2.5 * s / 2, y:  1.8,    z: 0 },  // N9
                // Amino group on C6
                { Z: 7, x: -1.0 * s / 2, y: -3.8,    z: 0 },  // NH2 on C6
                // Hydrogens
                { Z: 1, x: -2.5 * s / 2, y:  2.5,    z: 0 },  // H on C2
                { Z: 1, x:  4.5 * s / 2, y:  1.2,    z: 0 },  // H on C8
                { Z: 1, x:  2.8 * s / 2, y:  3.3,    z: 0 },  // H on N9
                { Z: 1, x: -2.3 * s / 2, y: -4.5,    z: 0 },  // H on NH2
                { Z: 1, x:  0.5 * s / 2, y: -4.5,    z: 0 },  // H on NH2
            ];
            return atoms;
        })(),
        cameraDistance: 30,
    },
    {
        id: 'diamond', name: 'Diamond Cell', formula: 'C<sub>8</sub>',
        category: 'biochemical',
        description: 'Diamond cubic unit cell — sp³ carbon network, hardest material.',
        atoms: (() => {
            // Diamond FCC + tetrahedral interstitial (8 atoms)
            const a = 4.0;  // cell parameter in simulation units
            return [
                // FCC corners (4 atoms in conventional cell)
                { Z: 6, x: 0,       y: 0,       z: 0 },
                { Z: 6, x: a / 2,   y: a / 2,   z: 0 },
                { Z: 6, x: a / 2,   y: 0,       z: a / 2 },
                { Z: 6, x: 0,       y: a / 2,   z: a / 2 },
                // Tetrahedral interstitials (4 atoms)
                { Z: 6, x: a / 4,   y: a / 4,   z: a / 4 },
                { Z: 6, x: 3*a / 4, y: 3*a / 4, z: a / 4 },
                { Z: 6, x: 3*a / 4, y: a / 4,   z: 3*a / 4 },
                { Z: 6, x: a / 4,   y: 3*a / 4, z: 3*a / 4 },
            ];
        })(),
        cameraDistance: 25,
    },
    {
        id: 'caffeine', name: 'Caffeine', formula: 'C<sub>8</sub>H<sub>10</sub>N<sub>4</sub>O<sub>2</sub>',
        category: 'biochemical',
        description: 'Purine alkaloid — the world\'s most consumed psychoactive substance.',
        atoms: (() => {
            // Caffeine: fused pyrimidine-imidazole ring with 3 methyl groups + 2 carbonyls
            // Planar ring system, methyl groups out of plane slightly
            const s = 1.1;
            const atoms = [];

            // 6-membered ring (pyrimidine-dione): C2-N3-C4-C5-C6-N1
            const r6 = [
                { el: 6, x: -2.5, y:  1.0 },   // C2 (carbonyl)
                { el: 7, x: -0.5, y:  2.0 },   // N3 (methylated)
                { el: 6, x:  1.5, y:  1.0 },   // C4 (carbonyl)
                { el: 6, x:  1.5, y: -1.0 },   // C5
                { el: 6, x: -0.5, y: -2.0 },   // C6
                { el: 7, x: -2.5, y: -1.0 },   // N1 (methylated)
            ];

            // 5-membered ring (imidazole): C4-C5-N7-C8-N9
            const r5 = [
                { el: 7, x:  3.5, y: -1.5 },   // N7 (methylated)
                { el: 6, x:  4.5, y:  0.5 },   // C8
                { el: 7, x:  3.2, y:  2.0 },   // N9
            ];

            // Ring atoms
            for (const a of r6) atoms.push({ Z: a.el, x: a.x * s, y: a.y * s, z: 0 });
            for (const a of r5) atoms.push({ Z: a.el, x: a.x * s, y: a.y * s, z: 0 });

            // Carbonyl oxygens
            atoms.push({ Z: 8, x: -4.0 * s, y:  1.8 * s, z: 0 });   // =O on C2
            atoms.push({ Z: 8, x:  1.5 * s, y:  3.0 * s, z: 0 });   // =O on C4

            // N1 methyl (C + 3 H)
            atoms.push({ Z: 6, x: -3.8 * s, y: -1.5 * s, z:  0 });    // methyl C
            atoms.push({ Z: 1, x: -4.5 * s, y: -2.5 * s, z:  1.5 });
            atoms.push({ Z: 1, x: -4.5 * s, y: -2.5 * s, z: -1.5 });
            atoms.push({ Z: 1, x: -5.0 * s, y: -0.5 * s, z:  0 });

            // N3 methyl (C + 3 H)
            atoms.push({ Z: 6, x: -0.5 * s, y:  3.5 * s, z:  0 });    // methyl C
            atoms.push({ Z: 1, x: -0.5 * s, y:  4.8 * s, z:  1.5 });
            atoms.push({ Z: 1, x: -0.5 * s, y:  4.8 * s, z: -1.5 });
            atoms.push({ Z: 1, x: -1.8 * s, y:  4.0 * s, z:  0 });

            // N7 methyl (C + 3 H)
            atoms.push({ Z: 6, x:  4.8 * s, y: -2.5 * s, z:  0 });    // methyl C
            atoms.push({ Z: 1, x:  5.5 * s, y: -3.5 * s, z:  1.5 });
            atoms.push({ Z: 1, x:  5.5 * s, y: -3.5 * s, z: -1.5 });
            atoms.push({ Z: 1, x:  6.2 * s, y: -2.0 * s, z:  0 });

            // H on C8
            atoms.push({ Z: 1, x:  6.0 * s, y:  0.5 * s, z: 0 });

            return atoms;
        })(),
        cameraDistance: 35,
    },
];

// ── Accessor Functions ───────────────────────────────────────────────

export function getCategories() {
    return CATEGORIES;
}

export function getAllMolecules() {
    return MOLECULES;
}

export function getMolecule(id) {
    return MOLECULES.find(m => m.id === id) || null;
}

export function getMoleculesByCategory(catId) {
    return MOLECULES.filter(m => m.category === catId);
}

/**
 * Load a molecule into the AtomEngine bridge.
 * @param {object} bridge — WasmBridge or MockBridge with aeAddAtom()
 * @param {string} id — molecule id from the catalog
 * @returns {boolean} true if loaded, false if not found
 */
export function loadMolecule(bridge, id) {
    const mol = getMolecule(id);
    if (!mol) return false;

    for (const atom of mol.atoms) {
        bridge.aeAddAtom(
            atom.Z,
            atom.x || 0, atom.y || 0, atom.z || 0,
            atom.vx || 0, atom.vy || 0, atom.vz || 0,
            atom.charge || 0
        );
    }
    return true;
}

/**
 * Get molecule count by category for display.
 */
export function moleculeSummary() {
    const summary = {};
    for (const cat of CATEGORIES) {
        summary[cat.id] = {
            label: cat.label,
            count: MOLECULES.filter(m => m.category === cat.id).length,
        };
    }
    summary.total = MOLECULES.length;
    return summary;
}
