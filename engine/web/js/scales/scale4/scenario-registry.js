/**
 * Canonical Scale 4 Solar System scenario registry.
 *
 * The registry owns admission to the toolbar and bridge. Its epistemic labels
 * describe standard effective celestial mechanics and designed laboratories;
 * none of these scenarios is a substrate-native FTD derivation.
 */

export const SCALE4_DEFAULT_SCENARIO = 'planetary-solar';

const entries = [
    ['planetary-solar', 'Solar-system dynamics', 'Our Solar System · J2000', 'Imported J2000 reference state evolved by the complete applicable effective-physics stack.', 'parametric'],
    ['planetary-binary', 'Solar-system dynamics', 'Binary Star System', 'Two equal stars and a circumbinary world evolved about their common barycenter.', 'imposed'],
    ['planetary-threebody', 'Solar-system dynamics', 'Three-Body Problem', 'Chenciner–Montgomery figure-eight initial data in G=1 natural units.', 'parametric'],
    ['planetary-mercury-relativity', 'Physics laboratories', 'Mercury · 1PN Precession', 'Sun–Mercury reference subset for comparing Newtonian and dominant-star Schwarzschild 1PN motion.', 'parametric'],
    ['planetary-earth-moon-tides', 'Physics laboratories', 'Earth–Moon · Tidal Migration', 'Near-circular constant-Q tidal migration with an explicit Earth spin reservoir.', 'parametric'],
    ['planetary-radiation-lab', 'Physics laboratories', 'Dust · Radiation + P-R Drag', 'Equal-density dust grains expose radiation-pressure, Poynting–Robertson, and solar-wind scaling.', 'parametric'],
    ['planetary-atmosphere-entry', 'Physics laboratories', 'Earth · Atmospheric Entry', 'A finite capsule crosses a co-rotating exponential atmosphere.', 'parametric'],
    ['planetary-roche-lab', 'Physics laboratories', 'Saturn · Roche Disruption', 'An eligible icy moon begins inside Saturn\'s fluid Roche threshold.', 'imposed'],
    ['planetary-impact-lab', 'Physics laboratories', 'Protoplanet Impact', 'Two finite rocky bodies undergo a momentum-conserving perfectly inelastic merge.', 'imposed'],
    ['exo-TRAPPIST-1', 'NASA Exoplanet Archive Data', 'TRAPPIST-1 System', 'Catalog-scale compact seven-planet system with modeled presentation colors.', 'parametric'],
    ['exo-Kepler-11', 'NASA Exoplanet Archive Data', 'Kepler-11 System', 'Catalog-scale compact six-planet system with modeled presentation colors.', 'parametric'],
    ['exo-HR 8799', 'NASA Exoplanet Archive Data', 'HR 8799 System', 'Catalog-scale wide four-planet system with modeled presentation colors.', 'parametric'],
    ['exo-Kepler-20', 'NASA Exoplanet Archive Data', 'Kepler-20 System', 'Catalog-scale six-planet system with modeled presentation colors.', 'parametric'],
];

export const SCALE4_SCENARIOS = Object.freeze(entries.map(([id, category, title, summary, epistemicStatus]) => Object.freeze({
    id,
    category,
    title,
    summary,
    epistemicStatus,
    scenarioClass: category === 'Physics laboratories' ? 'effective_physics_lab' : 'effective_dynamics',
    owner: 'js_effective_solar_system_engine',
})));

const scenarioMap = new Map(SCALE4_SCENARIOS.map((scenario) => [scenario.id, scenario]));

export function getScale4Scenario(id) {
    return scenarioMap.get(id) || null;
}

export function populateScale4ScenarioSelect(select, selectedId = SCALE4_DEFAULT_SCENARIO) {
    if (!select) return;
    const groups = new Map();
    for (const scenario of SCALE4_SCENARIOS) {
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
            option.title = `${scenario.summary} [${scenario.epistemicStatus.toUpperCase()}]`;
            group.appendChild(option);
        }
        select.appendChild(group);
    }
    select.value = scenarioMap.has(selectedId) ? selectedId : SCALE4_DEFAULT_SCENARIO;
}

export function validateScale4ScenarioRegistry() {
    const errors = [];
    const seen = new Set();
    for (const scenario of SCALE4_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        for (const field of ['category', 'title', 'summary', 'epistemicStatus', 'scenarioClass', 'owner']) {
            if (!scenario[field]) errors.push(`${field}:${scenario.id}`);
        }
        if (!['parametric', 'imposed'].includes(scenario.epistemicStatus)) {
            errors.push(`epistemicStatus:${scenario.id}`);
        }
    }
    if (!scenarioMap.has(SCALE4_DEFAULT_SCENARIO)) errors.push('default:missing');
    return { ok: errors.length === 0, errors, count: SCALE4_SCENARIOS.length };
}

const validation = validateScale4ScenarioRegistry();
if (!validation.ok) console.warn('[scale4/scenario-registry] validation failed:', validation.errors.join(', '));
