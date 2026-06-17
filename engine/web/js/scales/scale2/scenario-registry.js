/**
 * Scale 2 — AE scenario registry (canonical metadata + select population).
 *
 * Curated demos live here; element scenarios (ae-el-*) are generated from
 * elements.js at populate time. Hydrogen is covered by ae-hydrogen-atom — Z=1
 * is omitted from the element list to avoid a duplicate entry.
 */

import { getElement } from '../../elements.js';

export const AE_DEFAULT_SCENARIO = 'ae-hydrogen-atom';

/** @typedef {{ id: string, title: string, category: string, summary: string, tags?: string[] }} AEScenarioMeta */

/** @type {AEScenarioMeta[]} */
export const AE_CURATED_SCENARIOS = [
    {
        id: 'ae-hydrogen-atom',
        category: 'Single-Atom Physics',
        title: 'Hydrogen Atom (p + e\u2212)',
        summary: 'Hydrogen is the central-potential baseline for the atom engine and the cleanest place to compare orbital intuition, cloud display, and one-center attraction.',
        tags: ['static', 'ionic'],
    },
    {
        id: 'ae-rutherford-scattering',
        category: 'Single-Atom Physics',
        title: 'Rutherford Scattering',
        summary: 'Rutherford scattering is about large-angle deflection from a compact charged center, so treat it as a geometry-and-impact-parameter problem.',
        tags: ['ionic', 'dynamics'],
    },
    {
        id: 'ae-he-cluster',
        category: 'Noble Gas Clusters',
        title: 'He Cluster (6 atoms, vdW)',
        summary: 'Helium clustering is a weak-binding problem dominated by van der Waals attraction and excluded-volume repulsion, not strong covalent directionality.',
        tags: ['vdw'],
    },
    {
        id: 'ae-ar-cluster',
        category: 'Noble Gas Clusters',
        title: 'Ar Cluster (8 atoms, vdW)',
        summary: 'Argon makes the same noble-gas story visually stronger because dispersion attraction is deeper and the cluster compacts more readily.',
        tags: ['vdw'],
    },
    {
        id: 'ae-noble-mix',
        category: 'Noble Gas Clusters',
        title: 'Noble Mix (He + Ne + Ar)',
        summary: 'The noble mix scenario is about species-dependent \u03c3 and \u03b5 values: same broad force law, different preferred spacing and clustering depth.',
        tags: ['vdw'],
    },
    {
        id: 'ae-nacl-form',
        category: 'Ionic Formation',
        title: 'Na + Cl \u2192 NaCl',
        summary: 'NaCl formation is the textbook ionic case: opposite charges attract, a preferred separation appears, and the bond is governed mainly by electrostatic balance.',
        tags: ['ionic'],
    },
    {
        id: 'ae-nacl-lattice',
        category: 'Ionic Formation',
        title: 'NaCl 3\u00d73 Lattice',
        summary: 'NaCl lattice extends ionic bonding into periodic packing, so lattice energy and coordination become the right language.',
        tags: ['ionic'],
    },
    {
        id: 'ae-mgf2',
        category: 'Ionic Formation',
        title: 'Mg\u00b2\u207a + 2F\u207b \u2192 MgF\u2082',
        summary: 'MgF\u2082 is a stoichiometry lesson as much as a force lesson: total charge balance determines the preferred assembly pattern.',
        tags: ['ionic'],
    },
    {
        id: 'ae-h2-form',
        category: 'Covalent Formation',
        title: 'H + H \u2192 H\u2082',
        summary: 'H\u2082 formation is the simplest covalent-bonding case, where bond length and spring-like stabilization are the main quantities to watch.',
        tags: ['covalent'],
    },
    {
        id: 'ae-o2-form',
        category: 'Covalent Formation',
        title: 'O + O \u2192 O\u2082',
        summary: 'O\u2082 formation pushes beyond the minimal H\u2082 picture and invites discussion of stronger bonding and molecular stability.',
        tags: ['covalent'],
    },
    {
        id: 'ae-ch4-form',
        category: 'Covalent Formation',
        title: 'C + 4H \u2192 CH\u2084',
        summary: 'CH\u2084 is the tetrahedral geometry showcase, so symmetry and bond-angle stabilization matter as much as raw radial attraction.',
        tags: ['covalent', 'vsepr'],
    },
    {
        id: 'ae-water-dimer',
        category: 'H-Bonding',
        title: 'Water Dimer (H-bond)',
        summary: 'The water dimer is the entry point for hydrogen bonding, dipole alignment, and directional intermolecular preference.',
        tags: ['hbond'],
    },
    {
        id: 'ae-water-cluster',
        category: 'H-Bonding',
        title: 'Water Pentamer',
        summary: 'Water clusters quickly turn into network problems: local H-bond rules create global geometry.',
        tags: ['hbond'],
    },
    {
        id: 'ae-vsepr-linear',
        category: 'VSEPR Geometry',
        title: 'CO\u2082 \u2192 Linear (180\u00b0)',
        summary: 'The CO\u2082 case shows how repulsion geometry can favor a 180\u00b0 arrangement even when the molecule is built from more than two atoms.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-vsepr-tetrahedral',
        category: 'VSEPR Geometry',
        title: 'CH\u2084 \u2192 Tetrahedral (109.5\u00b0)',
        summary: 'CH\u2084 tetrahedral is the classic 109.5\u00b0 geometry lesson.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-vsepr-bent',
        category: 'VSEPR Geometry',
        title: 'H\u2082O \u2192 Bent (104.5\u00b0)',
        summary: 'H\u2082O bent geometry is the standard \u201clone pairs change the angle\u201d teaching case.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-thermal-gas',
        category: 'Thermal Dynamics',
        title: 'Ar Gas (12 atoms + thermostat)',
        summary: 'Thermal gas is about ensemble behavior, temperature control, and whether kinetic agitation overwhelms short-range ordering.',
        tags: ['thermal', 'vdw'],
    },
    {
        id: 'ae-collision',
        category: 'Thermal Dynamics',
        title: 'Head-On Collision',
        summary: 'Head-on collision is the atom-engine momentum-conservation demo.',
        tags: ['vdw', 'dynamics'],
    },
    {
        id: 'ae-fe-bcc',
        category: 'Metallic Clusters',
        title: 'Fe BCC Cluster (9 atoms)',
        summary: 'Fe BCC is a packing-and-coordination scenario where geometry matters as much as pair potential.',
        tags: ['metallic'],
    },
    {
        id: 'ae-cu-fcc',
        category: 'Metallic Clusters',
        title: 'Cu FCC Seed (7 atoms)',
        summary: 'Cu FCC is the close-packed comparison case to BCC iron.',
        tags: ['metallic'],
    },
    {
        id: 'ae-periodic',
        category: 'Special',
        title: 'Periodic Table (All 118)',
        summary: 'Periodic Table mode is a parameter atlas rather than one fixed simulation; the lesson is periodic trends, valence, and how element identity changes force-relevant quantities.',
        tags: ['static', 'elements'],
    },
    {
        id: 'ae-custom',
        category: 'Special',
        title: 'Custom (Manual)',
        summary: 'Custom atom mode lets you test your own composition, force toggles, and geometry under the same atom-engine rules.',
        tags: ['sandbox'],
    },
];

const AE_CURATED_MAP = new Map(AE_CURATED_SCENARIOS.map((s) => [s.id, s]));

const ELEMENT_PERIODS = [
    { label: 'Period 1', start: 1, end: 2 },
    { label: 'Period 2', start: 3, end: 10 },
    { label: 'Period 3', start: 11, end: 18 },
    { label: 'Period 4', start: 19, end: 36 },
    { label: 'Period 5', start: 37, end: 54 },
    { label: 'Period 6', start: 55, end: 86 },
    { label: 'Period 7', start: 87, end: 118 },
];

/** Skip Z=1 — ae-hydrogen-atom is the canonical hydrogen entry. */
const ELEMENT_Z_SKIP = new Set([1]);

/**
 * @param {string} id
 * @returns {AEScenarioMeta | null}
 */
export function getAEScenarioMeta(id) {
    if (!id) return null;
    const curated = AE_CURATED_MAP.get(id);
    if (curated) return curated;
    if (id.startsWith('ae-el-')) {
        const Z = parseInt(id.slice(6), 10);
        const el = getElement(Z);
        if (!el) return null;
        return {
            id,
            category: 'Elements',
            title: `${Z} ${el.symbol} \u2014 ${el.name}`,
            summary: `Isolated ${el.name} atom (Z = ${Z}). Orbital clouds and shell boundary spheres are enabled; dynamics are off (locked atom).`,
            tags: ['elements', 'static'],
        };
    }
    return null;
}

/**
 * @param {HTMLSelectElement | null} select
 * @param {string} [selectedId]
 */
export function populateAEScenarioSelect(select, selectedId = AE_DEFAULT_SCENARIO) {
    if (!select) return;

    const groups = new Map();
    for (const scenario of AE_CURATED_SCENARIOS) {
        if (!groups.has(scenario.category)) groups.set(scenario.category, []);
        groups.get(scenario.category).push(scenario);
    }

    select.innerHTML = '';

    for (const [category, scenarios] of groups) {
        const group = document.createElement('optgroup');
        group.label = category;
        for (const scenario of scenarios) {
            const option = document.createElement('option');
            option.value = scenario.id;
            option.textContent = scenario.title;
            option.selected = scenario.id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }

    for (const period of ELEMENT_PERIODS) {
        const group = document.createElement('optgroup');
        group.label = period.label;
        for (let Z = period.start; Z <= period.end; Z++) {
            if (ELEMENT_Z_SKIP.has(Z)) continue;
            const el = getElement(Z);
            if (!el) continue;
            const id = `ae-el-${Z}`;
            const option = document.createElement('option');
            option.value = id;
            option.textContent = `${Z} ${el.symbol} \u2014 ${el.name}`;
            option.selected = id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }

    if (!select.querySelector(`option[value="${selectedId}"]`)) {
        select.value = AE_DEFAULT_SCENARIO;
    } else {
        select.value = selectedId;
    }
}

export function validateAEScenarioRegistry() {
    const seen = new Set();
    const errors = [];
    for (const scenario of AE_CURATED_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        if (!scenario.category) errors.push(`category:${scenario.id}`);
        if (!scenario.title) errors.push(`title:${scenario.id}`);
    }
    return { ok: errors.length === 0, errors, count: AE_CURATED_SCENARIOS.length };
}

{
    const check = validateAEScenarioRegistry();
    if (!check.ok) {
        console.warn('[scale2/scenario-registry] validation failed:', check.errors.join(', '));
    }
}
