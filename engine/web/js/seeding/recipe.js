/** Versioned initial-condition drafts. No physics executes in this module.
 * v2 finite recipes carry the actual fully-expanded, ordered component list
 * the compiler builds from (9 component kinds); v2 native recipes carry the
 * same six keys plus an `overrides` map applied to the frozen constructor. */
export const RECIPE_VERSION = 2;
export const RECORD_SEED_SIZES = Object.freeze([3, 4, 7, 9, 17]);
export const COMPONENT_KINDS = Object.freeze({
    field: 'Field channel',
    field_list: 'Exact field list',
    field_random: 'Random field fill',
    relation: 'Relation token',
    relation_background: 'Relation background',
    sparse_tokens: 'Sparse tokens',
    manifestation: 'Stored manifestation',
    collision: 'Collision layer',
    index_pattern: 'Index pattern',
});
const KIND_NAMES = Object.keys(COMPONENT_KINDS);
const REGION_SHAPES = ['all', 'point', 'box', 'sphere'];
const PARAMETER_KEY = /^[a-zA-Z][a-zA-Z0-9_]{0,31}$/;
/** Native override keys are dotted paths into the constructor's own property tree
 * (e.g. "protocol.boundary", "ingredient.2.channel"); up to 160 characters, matching
 * the compiler's own legal alphabet — not the 32-character finite parameter-key limit. */
const OVERRIDE_KEY = /^[a-zA-Z][a-zA-Z0-9_.]{0,159}$/;
const UINT32_MAX = 4294967295;
// Exact RNG leaves in the constructor catalogs: random.seed,
// protocol.bathSeed, and the finite recipe/component seed fields. A physical
// quantity such as packet.seedSpeed is a real value, not a random stream ID.
const RNG_SEED_KEY = /(?:^|\.)(?:seed|randomSeed|bathSeed)$/;

export const cloneRecipe = value => JSON.parse(JSON.stringify(value));

function canonical(value) {
    if (Array.isArray(value)) return value.map(canonical);
    if (value && typeof value === 'object') {
        const out = {};
        for (const key of Object.keys(value).sort()) out[key] = canonical(value[key]);
        return out;
    }
    return value;
}
/** Structural equality ignoring key order; used to tell an edited recipe from its registered default. */
export function recipesEqual(a, b) { return JSON.stringify(canonical(a)) === JSON.stringify(canonical(b)); }

export function getAtPath(recipe, path) {
    let node = recipe;
    for (const key of path) node = node?.[key];
    return node;
}
export function setAtPath(recipe, path, value) {
    let node = recipe;
    for (let i = 0; i < path.length - 1; i++) node = node[path[i]];
    node[path.at(-1)] = value;
}

/** Stable property identity survives reordering ingredients. Only explicit user
 * edits belong here; resolved defaults remain free to follow domain changes. */
export function overrideIdentity(recipe, path) {
    return JSON.stringify(path[0] === 'components' ? ['component', recipe.components[path[1]].id, ...path.slice(2)] : path);
}
export function markFiniteOverride(recipe, path, enabled = true) {
    const key = overrideIdentity(recipe, path), paths = new Set(recipe.explicitOverrides || []);
    if (enabled) paths.add(key); else paths.delete(key);
    if (paths.size) recipe.explicitOverrides = [...paths].sort(); else delete recipe.explicitOverrides;
}
export function hasFiniteOverride(recipe, path) { return recipe.explicitOverrides?.includes(overrideIdentity(recipe, path)) || false; }

export function createRecipe(scenario, size) {
    const n = scenario.sizes?.includes(size) ? size : scenario.sizes?.at(-1) || size || 33;
    const recipe = {version: RECIPE_VERSION, scenarioId: scenario.id, size: n, blank: false, randomSeed: 0, components: []};
    return scenario.backend === 'finite-records' ? recipe : {...recipe, overrides: {}};
}

/** A new instance of the constructor-authored component exemplar. */
export function createComponent(template, id, size) {
    const component = cloneRecipe(template);
    component.id = id; component.enabled = true;
    return component;
}

/** The full inactive category base (every finite kind, once each) for a given scenario, from its catalog schema. */
export function createBaseRecipe(scenario, baseSchema) {
    if (scenario.backend !== 'finite-records') throw new Error('Native category bases need parameterized native constructors.');
    const recipe = cloneRecipe(baseSchema.recipe);
    recipe.scenarioId = scenario.id;
    return recipe;
}

/** The full inactive union of every native constructor registered in the same
 * category as `scenario`, one ordered ingredient per constructor, all disabled —
 * the native counterpart of createBaseRecipe. `categoryScenarios` is every
 * non-finite scenario sharing `scenario.category` (including `scenario` itself),
 * as returned by getScale0SeedingScenarios(). The anchor scenarioId is preserved
 * (parent registry lookup, telemetry, and evidence stay attached to a real id). */
export function createNativeBaseRecipe(scenario, categoryScenarios) {
    if (scenario.backend === 'finite-records') throw new Error('Finite category bases use createBaseRecipe.');
    const seen = new Set();
    const components = categoryScenarios
        .filter(s => s.backend !== 'finite-records' && s.category === scenario.category && !seen.has(s.id) && seen.add(s.id))
        .map(s => ({id: `native-${s.id}`, kind: 'native', enabled: false, scenarioId: s.id, overrides: {}}));
    return {version: RECIPE_VERSION, scenarioId: scenario.id, size: scenario.sizes?.at(-1) || 33,
        blank: true, randomSeed: 0, components, overrides: {}};
}

/** True only when a native recipe's own structure says it differs from the
 * registered preset: a category base, an enabled ingredient, or any explicit
 * override — never a network/catalog comparison (native has no compiler default
 * to diff against; the frozen constructor default is the only baseline). */
export function isCustomNativeRecipe(recipe) {
    return !!recipe.blank || recipe.components.some(c => c.enabled) || Object.keys(recipe.overrides || {}).length > 0;
}

const exactKeys = (value, keys, name) => {
    if (!value || Array.isArray(value) || typeof value !== 'object'
        || Object.keys(value).sort().join('|') !== [...keys].sort().join('|')) throw new Error(`${name}: unexpected or missing properties`);
};
const integer = (value, lo, hi, name) => {
    if (!Number.isSafeInteger(value) || value < lo || value > hi) throw new Error(`${name}: integer from ${lo} to ${hi} required`);
};
/** `key` may be a finite parameter name or a dotted native override path.
 * Catalog RNG leaves are exact uint32 stream identities; other scalar values
 * retain the generic ±1e7 sanity bound before compiler-specific validation. */
function sanitizeParameterValue(value, name, key) {
    if (key && RNG_SEED_KEY.test(key)) {
        if (typeof value !== 'number' || !Number.isSafeInteger(value) || value < 0 || value > UINT32_MAX)
            throw new Error(`${name}: seed must be an integer from 0 to ${UINT32_MAX}`);
        return;
    }
    if (typeof value === 'boolean') return;
    if (typeof value === 'number') {
        if (!Number.isFinite(value) || Math.abs(value) > 1e7) throw new Error(`${name}: must be a finite number`);
        return;
    }
    if (typeof value === 'string') { if (value.length > 32) throw new Error(`${name}: too long`); return; }
    if (Array.isArray(value)) {
        if (value.length > 384) throw new Error(`${name}: list too long`);
        value.forEach((entry, i) => sanitizeParameterValue(entry, `${name}[${i}]`, key));
        return;
    }
    throw new Error(`${name}: unsupported value type`);
}

/** Structural validation only: exact per-kind parameter legality is the compiler's own job (no compiler defaults in JS). */
export function validateRecipe(recipe, scenarios) {
    if (!recipe || typeof recipe !== 'object' || Array.isArray(recipe) || typeof recipe.scenarioId !== 'string') throw new Error('Recipe: unexpected or missing properties');
    const scenario = scenarios.find(row => row.id === recipe.scenarioId);
    if (!scenario) throw new Error('Unknown scenario; its preparation service may be unavailable');
    const commonKeys = ['version', 'scenarioId', 'size', 'blank', 'randomSeed', 'components'];
    if (scenario.backend === 'finite-records' && Object.hasOwn(recipe, 'explicitOverrides')) {
        commonKeys.push('explicitOverrides');
        if (!Array.isArray(recipe.explicitOverrides) || recipe.explicitOverrides.length > 4096
            || recipe.explicitOverrides.some(key => typeof key !== 'string' || key.length > 240)) throw new Error('Invalid explicit seed property identities');
    }
    exactKeys(recipe, scenario.backend === 'finite-records' ? commonKeys : [...commonKeys, 'overrides'], 'Recipe');
    if (recipe.version !== RECIPE_VERSION) throw new Error('Unsupported recipe version; migrate it first');
    integer(recipe.size, 3, 256, 'Lattice size'); integer(recipe.randomSeed, 0, 4294967295, 'Random seed');
    if (typeof recipe.blank !== 'boolean' || !Array.isArray(recipe.components) || recipe.components.length > 64) throw new Error('Invalid recipe; at most 64 components');
    if (scenario.backend !== 'finite-records') {
        if (recipe.randomSeed !== 0) throw new Error('Use the native random.seed property to select a constructor stream');
        if (!recipe.overrides || typeof recipe.overrides !== 'object' || Array.isArray(recipe.overrides)) throw new Error('Invalid overrides');
        for (const key of Object.keys(recipe.overrides)) {
            if (!OVERRIDE_KEY.test(key)) throw new Error(`Invalid override name: ${key}`);
            sanitizeParameterValue(recipe.overrides[key], `overrides.${key}`, key);
        }
        if (!recipe.blank) {
            if (recipe.components.length) throw new Error('This native preset does not yet accept component overrides');
            return scenario;
        }
        // A native category base: recipe.blank + an ordered list of disableable
        // constructor ingredients, each a full named native preparation (never a
        // finite record kind). Recipe-level `overrides` (validated above) still
        // apply last, over every ingredient's own resolved protocol.
        if (!recipe.components.length) throw new Error('A native category base needs at least one constructor ingredient');
        const nativeIds = new Set();
        for (const c of recipe.components) {
            exactKeys(c, ['id', 'kind', 'enabled', 'scenarioId', 'overrides'], 'Native ingredient');
            if (typeof c.id !== 'string' || !c.id.length || c.id.length > 80 || nativeIds.has(c.id)) throw new Error('Ingredient IDs must be unique');
            nativeIds.add(c.id);
            if (c.kind !== 'native') throw new Error('Native ingredients must have kind "native"');
            if (typeof c.enabled !== 'boolean') throw new Error('Invalid ingredient activation');
            if (typeof c.scenarioId !== 'string' || !c.scenarioId.length) throw new Error('Invalid ingredient scenario id');
            if (!c.overrides || typeof c.overrides !== 'object' || Array.isArray(c.overrides)) throw new Error('Invalid ingredient overrides');
            for (const key of Object.keys(c.overrides)) {
                if (!OVERRIDE_KEY.test(key)) throw new Error(`Invalid override name: ${key}`);
                sanitizeParameterValue(c.overrides[key], `${c.id}.${key}`, key);
            }
        }
        return scenario;
    }
    if (!RECORD_SEED_SIZES.includes(recipe.size)) throw new Error('Unsupported finite preparation size');
    if (!recipe.blank && !scenario.sizes.includes(recipe.size)) throw new Error('Choose a registered preset size, or start from an empty category base');
    const ids = new Set();
    for (const c of recipe.components) {
        exactKeys(c, ['id', 'kind', 'enabled', 'region', 'parameters'], 'Component');
        if (typeof c.id !== 'string' || !c.id.length || c.id.length > 80 || ids.has(c.id)) throw new Error('Component IDs must be unique');
        ids.add(c.id);
        if (!KIND_NAMES.includes(c.kind) || typeof c.enabled !== 'boolean') throw new Error('Invalid component kind or activation');
        exactKeys(c.region, ['shape', 'x', 'y', 'z', 'dx', 'dy', 'dz', 'radius'], 'Region');
        const r = c.region;
        if (!REGION_SHAPES.includes(r.shape)) throw new Error('Unknown region shape');
        for (const k of ['x', 'y', 'z']) integer(r[k], 0, recipe.size - 1, k);
        for (const k of ['dx', 'dy', 'dz']) integer(r[k], 1, recipe.size, k);
        if (typeof r.radius !== 'number' || !Number.isFinite(r.radius) || r.radius < 0 || r.radius > recipe.size) throw new Error('Invalid radius');
        if (r.shape === 'box' && ['x', 'y', 'z'].some(k => r[k] + r[`d${k}`] > recipe.size)) throw new Error('Box extends outside the lattice');
        if (!c.parameters || typeof c.parameters !== 'object' || Array.isArray(c.parameters)) throw new Error('Invalid parameters');
        const keys = Object.keys(c.parameters);
        if (keys.length > 8) throw new Error('Too many parameters');
        for (const key of keys) {
            if (!PARAMETER_KEY.test(key)) throw new Error(`Invalid parameter name: ${key}`);
            sanitizeParameterValue(c.parameters[key], `${c.id}.${key}`, key);
        }
    }
    return scenario;
}

/** Parses and, only for the current version, fully validates. A v1 payload is returned as-is for migrateRecipe(). */
export function parseRecipe(text, scenarios) {
    if (new TextEncoder().encode(text).length > 262144) throw new Error('Recipe exceeds 256 KiB');
    const recipe = JSON.parse(text);
    if (recipe && typeof recipe === 'object' && !Array.isArray(recipe) && recipe.version === RECIPE_VERSION) {
        validateRecipe(recipe, scenarios); return recipe;
    }
    exactKeys(recipe, ['version', 'scenarioId', 'size', 'blank', 'randomSeed', 'components'], 'Recipe');
    if (recipe.version !== 1) throw new Error('Unsupported recipe version');
    return recipe;
}

/** Expands a legacy (v1) finite recipe into v2: the registered default's ordered components,
 * followed by the original recipe's own components in their original order (never prepended
 * when the original was already a blank category base). */
export function migrateFiniteRecipeV1(recipe, defaultRecipe) {
    if (recipe.blank) return {...cloneRecipe(recipe), version: RECIPE_VERSION};
    const expanded = cloneRecipe(defaultRecipe);
    expanded.blank = recipe.blank; expanded.randomSeed = recipe.randomSeed;
    const overlay = cloneRecipe(recipe).components;
    // The overlay's own ids are the legacy recipe's real identity (its
    // component-scoped RNG stream is derived from id) and must never change on
    // migration. A same-id expanded default component is renamed instead.
    const overlayIds = new Set(overlay.map(c => c.id));
    const renamed = new Set(expanded.components.map(c => c.id));
    expanded.components = expanded.components.map(c => {
        if (!overlayIds.has(c.id)) return c;
        let candidate = `${c.id}-base`, suffix = 0;
        while (overlayIds.has(candidate) || renamed.has(candidate)) candidate = `${c.id}-base-${++suffix}`;
        renamed.add(candidate);
        return {...c, id: candidate};
    });
    expanded.components = [...expanded.components, ...overlay];
    return expanded;
}
/** A legacy native recipe carried no overrides; v2 adds an empty map, never copying defaults into it. */
export function migrateNativeRecipeV1(recipe) { return {...cloneRecipe(recipe), version: RECIPE_VERSION, overrides: {}}; }

export function migrateRecipe(recipe, scenario, defaultRecipe) {
    if (recipe.version === RECIPE_VERSION) return recipe;
    if (recipe.version !== 1) throw new Error('Unsupported recipe version');
    return scenario.backend === 'finite-records' ? migrateFiniteRecipeV1(recipe, defaultRecipe) : migrateNativeRecipeV1(recipe);
}

export function seedCoverage(scenarios) {
    return scenarios.map(s => ({scenarioId: s.id, category: s.category,
        capability: s.backend === 'finite-records' ? 'record-components' : 'native-preparation',
        parameterAudit: s.backend === 'finite-records' ? 'nine-kind-record-editors' : 'typed-constructor-bindings'}));
}

/** Versioned export/import envelope: the recipe's own six/seven keys stay the
 * single source of truth (never duplicated elsewhere in the envelope); everything
 * else is read straight from an already-resolved description, never invented here. */
export const ENVELOPE_FORMAT = 'SeedRecipeV2';
export const MAX_RECIPE_ENVELOPE_BYTES = 2 * 1024 * 1024;

function envelopeRandomness(recipe, scenario, description) {
    if (scenario.backend === 'finite-records') {
        return {generator: 'component-scoped-rng', recipeSeed: recipe.randomSeed,
            componentSeeds: recipe.components.filter(c => 'seed' in (c.parameters || {})).map(c => ({id: c.id, seed: c.parameters.seed}))};
    }
    return {generator: 'std::mt19937', streams: (description?.properties || [])
        .filter(p=>p.group === 'random' || /(?:^|\.)random\./.test(p.key))
        .map(p=>({key:p.key,path:p.path,seed:p.value,
            ingredientId:p.path?.[0] === 'components' ? recipe.components[p.path[1]].id : recipe.scenarioId}))};
}

function envelopeProtocol(recipe, description) {
    const properties = Array.isArray(description) ? description : (description?.properties || []);
    return properties.filter(p => p.group === 'protocol' || /(?:^|\.)protocol\./.test(p.key))
        .map(p => ({key: p.key, label: p.label, path: p.path, value: p.value}));
}

/** `description` is whatever describeFiniteRecipe()/describeNativeRecipe() already
 * produced for this exact recipe (source of the schemaIdentity + protocol list);
 * `custom` is the caller's own already-computed custom/edited verdict. */
export function buildRecipeEnvelope(recipe, scenario, description, {custom = null, receipt = null} = {}) {
    return {
        format: ENVELOPE_FORMAT, version: RECIPE_VERSION,
        schemaIdentity: description?.schemaIdentity ?? null,
        domain: {kind: scenario.backend === 'finite-records' ? 'finite-records' : 'native', size: recipe.size},
        recipe: cloneRecipe(recipe),
        randomness: envelopeRandomness(recipe, scenario, description),
        protocol: envelopeProtocol(recipe, description),
        resolvedProperties: (description?.properties || []).map(p=>({key:p.key,value:p.value,units:p.units})),
        receipt,
        provenance: {parentPreset: recipe.scenarioId, custom},
    };
}

/** Parses either a versioned envelope or a bare v1/v2 recipe (parseRecipe's own
 * legacy contract). An envelope's domain is cross-checked against its own carried
 * recipe (kind and size must agree) before the recipe is validated as usual. */
export function parseRecipeEnvelope(text, scenarios) {
    const bytes = new TextEncoder().encode(text).length;
    if (bytes > MAX_RECIPE_ENVELOPE_BYTES) throw new Error('Recipe envelope exceeds 2 MiB');
    let parsed;
    try { parsed = JSON.parse(text); }
    catch (error) { if (bytes > 262144) throw new Error('Recipe exceeds 256 KiB'); throw error; }
    if (!parsed || typeof parsed !== 'object' || Array.isArray(parsed) || parsed.format !== ENVELOPE_FORMAT) {
        return parseRecipe(text, scenarios);
    }
    if (parsed.version !== RECIPE_VERSION) throw new Error('Unsupported envelope version');
    const recipe = parsed.recipe;
    if (!recipe || typeof recipe !== 'object' || Array.isArray(recipe)) throw new Error('Envelope: missing recipe');
    if (new TextEncoder().encode(JSON.stringify(recipe)).length > 262144) throw new Error('Recipe exceeds 256 KiB');
    const scenario = scenarios.find(s => s.id === recipe.scenarioId);
    if (!scenario) throw new Error('Unknown scenario; its preparation service may be unavailable');
    const expectedKind = scenario.backend === 'finite-records' ? 'finite-records' : 'native';
    const identities = expectedKind === 'native' ? ['native-seed-2'] : ['finite-seed-2', 'finite-seed-2-base'];
    if (!identities.includes(parsed.schemaIdentity)) throw new Error('Unsupported preparation schema identity');
    if (parsed.domain && (parsed.domain.kind !== expectedKind || parsed.domain.size !== recipe.size)) {
        throw new Error('Envelope domain does not match its own carried recipe');
    }
    if (recipe.version === RECIPE_VERSION) validateRecipe(recipe, scenarios);
    return recipe;
}
