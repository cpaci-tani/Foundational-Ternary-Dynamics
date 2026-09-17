/** Human-readable, complete resolved-recipe summary: a DOM render and a copyable
 * plain-text form built from the same section model. Reads only recipe.js and
 * property-schema.js/component-editor.js utilities; never fetches a catalog and
 * never invents a value, label, or quantity beyond what `description`/`receipt` carry. */
import { COMPONENT_KINDS, recipesEqual } from './recipe.js';
import { displayValue } from './property-schema.js';
import { node } from './component-editor.js';

const pathKey = path => path.join('.');

function indexProperties(description) {
    const list = Array.isArray(description) ? description : (description?.properties || []);
    const map = new Map();
    for (const p of list) if (p?.path) map.set(pathKey(p.path), p);
    return map;
}

function formatRaw(value) {
    if (Array.isArray(value)) return value.length ? value.join(', ') : '(none)';
    if (typeof value === 'boolean') return value ? '1 — On' : '0 — Off';
    if (value && typeof value === 'object') return JSON.stringify(value);
    return String(value);
}

/** Resolves one value against its descriptor: enum options decode to their registered
 * label (never a guessed physical name); everything else falls back to the raw value. */
function resolvedText(descriptor, value) {
    if (!descriptor) return formatRaw(value);
    if (Array.isArray(value)) {
        if (!value.length) return '(none)';
        const items = value.map(v => descriptor.options?.length ? displayValue(descriptor, v) : String(v));
        const units = descriptor.units && !descriptor.options?.length ? ` ${descriptor.units} each` : '';
        return `${items.join(', ')}${units}`;
    }
    return displayValue(descriptor, value);
}

function customDiffs(index) {
    const diffs = [];
    for (const p of index.values()) {
        if (!('default' in p) || !('value' in p)) continue;
        if (JSON.stringify(p.value) !== JSON.stringify(p.default)) diffs.push(p);
    }
    return diffs;
}

function appliedStatus(recipe, applied) {
    if (applied === undefined || applied === null) return 'Not compared against the active lattice.';
    if (typeof applied === 'boolean') return applied ? 'Applied to the active lattice.' : 'Draft only — the active lattice is unchanged.';
    try {
        return recipesEqual(recipe, applied) ? 'Applied to the active lattice.' : 'Draft only — differs from the last applied preparation.';
    } catch { return 'Draft only — the active lattice is unchanged.'; }
}

function buildOverview(recipe, scenario, index, applied, custom) {
    const hasDescription = index.size > 0;
    const diffs = hasDescription ? customDiffs(index) : [];
    const isCustom = custom ?? (!!recipe.blank || Object.keys(recipe.overrides || {}).length > 0
        || hasDescription && diffs.length > 0);
    const backendLabel = scenario.backend === 'finite-records' ? 'Finite-record backend' : 'Native backend';
    const statusLine = !hasDescription
        ? 'Modification status unknown — no resolved property descriptions were supplied.'
        : isCustom
            ? `Custom preparation — ${diffs.length} propert${diffs.length === 1 ? 'y' : 'ies'} differ from the registered default.`
            : 'Original registered preset, unmodified.';
    const rows = [
        { term: 'Preparation', value: scenario.title || scenario.scientificTitle || recipe.scenarioId },
        { term: 'Category', value: scenario.category || '(uncategorized)' },
        { term: 'Backend', value: backendLabel },
        { term: 'Lattice size', value: `${recipe.size}³ sites` },
        { term: 'Original vs. custom', value: statusLine },
        { term: 'Draft / applied', value: appliedStatus(recipe, applied) },
    ];
    if (scenario.epistemicStatus) rows.push({ term: 'Epistemic status', value: scenario.epistemicStatus });
    return { section: { title: 'Overview', expanded: true, paragraphs: [], cards: [{ title: null, muted: false, rows }] }, isCustom };
}

function regionRows(region, index, regionPath) {
    const shapeDescriptor = index.get(pathKey([...regionPath, 'shape']));
    const rows = [{ term: 'Region shape', value: shapeDescriptor ? displayValue(shapeDescriptor, region.shape) : region.shape }];
    if (region.shape === 'all') { rows.push({ term: 'Extent', value: 'Entire lattice' }); return rows; }
    rows.push({ term: 'Origin', value: `(${region.x}, ${region.y}, ${region.z})` });
    if (region.shape === 'box') rows.push({ term: 'Box extent', value: `${region.dx} × ${region.dy} × ${region.dz}` });
    if (region.shape === 'sphere') rows.push({ term: 'Radius', value: `${region.radius} cells` });
    return rows;
}

function componentCard(component, i, index, kindLabel) {
    const basePath = ['components', i];
    const rows = [{ term: 'Component ID', value: component.id }, {term: 'Enabled', value: component.enabled ? '1 — On' : '0 — Off'}, ...regionRows(component.region, index, [...basePath, 'region'])];
    for (const key of Object.keys(component.parameters || {})) {
        const descriptor = index.get(pathKey([...basePath, 'parameters', key]));
        rows.push({ term: descriptor?.label || key, value: resolvedText(descriptor, component.parameters[key]) });
    }
    return { title: `${i + 1}. ${kindLabel}`, muted: component.enabled === false, rows };
}

function nativeIngredientCard(component, i, index, label) {
    const basePath = ['components', i];
    const rows = [{ term: 'Component ID', value: component.id }, { term: 'Named preparation', value: component.scenarioId },
        {term: 'Enabled', value: component.enabled ? '1 — On' : '0 — Off'}];
    for (const descriptor of index.values()) {
        if (descriptor.path[0] !== 'components' || descriptor.path[1] !== i || descriptor.path[2] !== 'overrides') continue;
        rows.push({ term: descriptor.label, value: resolvedText(descriptor, descriptor.value) });
    }
    return { title: `${i + 1}. ${label || component.scenarioId}`, muted: component.enabled === false, rows };
}

function buildIngredients(recipe, scenario, index, description) {
    if (scenario.backend === 'finite-records') {
        if (!recipe.components.length) return { title: 'Ordered ingredients', expanded: false, paragraphs: ['The registered preparation is preserved exactly; no explicit component list is present.'], cards: [] };
        const cards = recipe.components.map((c, i) => componentCard(c, i, index,
            description?.ingredients?.find(item=>item.id===c.id)?.label || COMPONENT_KINDS[c.kind] || c.kind));
        return {
            title: 'Ordered ingredients', expanded: false,
            paragraphs: ['Components replace selected records in this exact list order. A later write to the same record address or slot replaces the earlier value; distinct channels and slots remain separate.'],
            cards,
        };
    }
    if (recipe.blank && recipe.components.length) {
        const cards = recipe.components.map((c, i) => nativeIngredientCard(c, i, index, description?.ingredients?.find(item=>item.id===c.id)?.label));
        return {
            title: 'Ordered ingredients', expanded: false,
            paragraphs: ['Multi-preparation native build. Later listed ingredients own their own source/profile fields unless a named global override applies — write order is not uniformly additive.'],
            cards,
        };
    }
    const groups = new Map();
    for (const p of index.values()) {
        if (p.group === 'random' || p.group === 'protocol' || p.path[0] === 'components') continue;
        if (!groups.has(p.group)) groups.set(p.group, []);
        groups.get(p.group).push({term: p.label, value: resolvedText(p, p.value)});
    }
    return {
        title: 'Ordered ingredients', expanded: false,
        paragraphs: [`Resolved ingredients of ${scenario.title || recipe.scenarioId}, in constructor order.`],
        cards: [...groups].map(([title, rows]) => ({title, rows, muted: false})),
    };
}

function buildRandomness(recipe, scenario, index) {
    if (scenario.backend === 'finite-records') {
        const seedDescriptor = index.get('randomSeed');
        const rows = [{ term: seedDescriptor?.label || 'Recipe random seed', value: resolvedText(seedDescriptor, recipe.randomSeed) }];
        recipe.components.forEach((c, i) => {
            if (!('seed' in (c.parameters || {}))) return;
            const descriptor = index.get(pathKey(['components', i, 'parameters', 'seed']));
            rows.push({ term: `${c.id} · ${descriptor?.label || 'seed'}`, value: resolvedText(descriptor, c.parameters.seed) });
        });
        return {
            title: 'Randomness', expanded: false,
            paragraphs: ["Generator: NumPy default_rng (PCG64). Fractional selections (density < 1) derive a component-scoped stream from the recipe seed and each component's own id. A component with its own explicit seed parameter ignores the recipe seed."],
            cards: [{ title: null, muted: false, rows }],
        };
    }
    const randomProps = [...index.values()].filter(p => p.group === 'random' || /(?:^|\.)random\./.test(p.key || ''));
    if (!randomProps.length) return { title: 'Randomness', expanded: false, paragraphs: ['No explicit randomness in this native preparation.'], cards: [] };
    return {
        title: 'Randomness', expanded: false, paragraphs: ['Preparation generator: C++ mt19937; reset independently for each constructor ingredient.'],
        cards: [{ title: null, muted: false, rows: randomProps.map(p => ({
            term: p.path[0] === 'components' ? `${recipe.components[p.path[1]].id} · ${p.label}` : p.label,
            value: resolvedText(p, p.value),
        })) }],
    };
}

function buildProtocol(recipe, scenario, index) {
    if (scenario.backend === 'finite-records') {
        return {
            title: 'Preparation protocol', expanded: false,
            paragraphs: ['Fixed law context, read-only: seeding never edits the tick law. The compiled record above is the only initial-condition input for this preparation.'],
            cards: [],
        };
    }
    const groups = new Map();
    for (const p of index.values()) {
        if (p.group !== 'protocol' && !/(?:^|\.)protocol\./.test(p.key || '')) continue;
        if (!groups.has(p.group)) groups.set(p.group, []);
        groups.get(p.group).push(p);
    }
    const cards = [...groups.entries()].map(([group, props]) => ({
        title: group, muted: false, rows: props.map(p => ({ term: p.label, value: resolvedText(p, p.value) })),
    }));
    return {
        title: 'Preparation protocol', expanded: false,
        paragraphs: cards.length
            ? ["Grouped by resolved constructor profile, in constructor order. A later-listed constructor owns its own source/packet/profile fields unless a named global override applies — this is not uniformly additive.",
                'Fixed law context, read-only: lattice dimensionality, tick-law constants and numerical write conventions remain those of the engine. These controls prepare inputs and select existing protocol terms.']
            : ['No resolved protocol properties were supplied for this preparation.'],
        cards,
    };
}

function buildResults(receipt) {
    if (!receipt) {
        return {
            title: 'Resolved results', expanded: false,
            paragraphs: ['No compiled receipt is available. Resolved counts are unknown until this recipe is previewed or applied — none are claimed here.'],
            cards: [],
        };
    }
    const rows = [];
    const put = (term, key) => { if (receipt[key] !== undefined && receipt[key] !== null) rows.push({ term, value: formatRaw(receipt[key]) }); };
    put('Field tokens written', 'fieldTokens');
    put('Relation tokens written', 'relationTokens');
    put('Manifested markers', 'manifested');
    put('Tick', 'tick');
    put('Checkpoint SHA-256', 'checkpointSHA256');
    put('Positive markers', 'positive'); put('Negative markers', 'negative');
    put('Total flux magnitude (lattice units)', 'totalFlux');
    const known = new Set(['fieldTokens', 'relationTokens', 'manifested', 'tick', 'checkpointSHA256', 'positive','negative','totalFlux','recipeTag','resolvedProperties','properties','geometrySupport']);
    for (const [key, value] of Object.entries(receipt)) if (!known.has(key) && value !== undefined && value !== null) rows.push({ term: key, value: formatRaw(value) });
    return {
        title: 'Resolved results', expanded: false,
        paragraphs: rows.length
            ? ["Only quantities returned by the prepared owner are shown. Writes resolve in ingredient order at each record address; no unmeasured overlap or support count is claimed."]
            : ['The compiled receipt supplied no recognized counts.'],
        cards: [
            ...(rows.length ? [{ title: null, muted: false, rows }] : []),
            ...(receipt.geometrySupport ? [{title:'Discretized geometry',muted:false,rows:[
                {term:'Meaning',value:receipt.geometrySupport.meaning},
                {term:'Selected sites (union)',value:String(receipt.geometrySupport.selectedSites)},
                {term:'Sites shared by geometries',value:String(receipt.geometrySupport.overlappingSites)},
                ...receipt.geometrySupport.components.map(c=>({term:`${c.label} · ${c.id}${c.enabled ? '' : ' (inactive)'}`,
                    value:`${c.selectedSites} selected sites; ${c.overlapWithEarlier} overlap earlier geometry; ${c.regionSites} sites in the mask.`})),
            ]}] : []),
        ],
    };
}

function buildProvenance(recipe, scenario, isCustom, index) {
    const rows = [{ term: 'Evidence status', value: scenario.epistemicStatus || '[OPEN]' }];
    if (scenario.backend === 'finite-records') {
        rows.push({
            term: 'Provenance',
            value: isCustom
                ? 'This is a custom initial state. Registered evidence remains attached to the original preset only; transport recovery for edited preparations is open.'
                : 'Matches the registered preset exactly and inherits its registered evidence.',
        });
    } else {
        rows.push({ term: 'Intent', value: scenario.intent || 'Native scenario defaults are applied by the engine.' });
        const overrideKeys = [...Object.keys(recipe.overrides || {}), ...recipe.components.flatMap(c => Object.keys(c.overrides || {}).map(key=>`${c.id}.${key}`))];
        rows.push({
            term: 'Explicit overrides',
            value: overrideKeys.length
                ? `${overrideKeys.length} explicit override(s) applied over the frozen constructor default: ${overrideKeys.join(', ')}.`
                : recipe.blank ? 'No property overrides; the ordered ingredient activation and composition are custom.' : 'None — the frozen constructor defaults are used unmodified.',
        });
        if (isCustom) rows.push({ term: 'Custom preparations', value: 'Edits here are custom preparations and do not inherit transport/recovery qualification from the parent preset.' });
    }
    const changed = customDiffs(index).map(p=>({term:p.label,
        value:`${resolvedText(p,p.default)} → ${resolvedText(p,p.value)}`}));
    return { title: 'Preparation provenance', expanded: false, paragraphs: [], cards: [
        { title: null, muted: false, rows },
        ...(changed.length ? [{title:'Changed property values',muted:false,rows:changed}] : []),
    ] };
}

function buildModel(recipe, scenario, description, { receipt, applied, custom } = {}) {
    const index = indexProperties(description);
    const overview = buildOverview(recipe, scenario, index, applied, custom);
    return [
        overview.section,
        buildIngredients(recipe, scenario, index, description),
        buildRandomness(recipe, scenario, index),
        buildProtocol(recipe, scenario, index),
        buildResults(receipt),
        buildProvenance(recipe, scenario, overview.isCustom, index),
    ];
}

function renderCard(card) {
    const wrap = node('div', undefined, `seed-recipe-card${card.muted ? ' seed-recipe-card--muted' : ''}`);
    if (card.title) wrap.append(node('h4', card.title, 'seed-recipe-card-title'));
    const dl = node('dl', undefined, 'seed-recipe-rows');
    for (const { term, value } of card.rows) dl.append(node('dt', term), node('dd', value));
    wrap.append(dl);
    return wrap;
}

function renderSection(section) {
    const details = node('details', undefined, 'seed-recipe-section');
    details.open = !!section.expanded;
    details.append(node('summary', section.title));
    for (const p of section.paragraphs) details.append(node('p', p, 'seed-hint'));
    for (const card of section.cards) details.append(renderCard(card));
    if (!section.cards.length && !section.paragraphs.length) details.append(node('p', 'Nothing to show.', 'seed-hint'));
    return details;
}

/** Renders the complete resolved recipe as a DOM element (browser-only). */
export function renderRecipeSummary(recipe, scenario, description, options = {}) {
    const root = node('div', undefined, 'seed-recipe-summary');
    for (const section of buildModel(recipe, scenario, description, options)) root.append(renderSection(section));
    return root;
}

function cardText(card) {
    const lines = [];
    if (card.title) lines.push(`- ${card.title}${card.muted ? ' (disabled)' : ''}`);
    const prefix = card.title ? '    ' : '';
    for (const { term, value } of card.rows) lines.push(`${prefix}${term}: ${value}`);
    return lines.join('\n');
}

function sectionText(section) {
    const lines = [`## ${section.title}`, ...section.paragraphs];
    for (const card of section.cards) lines.push(cardText(card));
    if (!section.cards.length && !section.paragraphs.length) lines.push('Nothing to show.');
    return lines.join('\n');
}

/** Renders the same complete resolved recipe as copyable plain text. Pure — no DOM access. */
export function recipeText(recipe, scenario, description, options = {}) {
    return buildModel(recipe, scenario, description, options).map(sectionText).join('\n\n');
}
