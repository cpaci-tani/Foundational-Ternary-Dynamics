// @ts-check
import { validateValue } from './contracts.js';
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */

// This is a conservative quantity reader, not a second planner. It only restricts
// existing descriptor properties, and leaves the choice/order of actions to the
// model. Unrecognised or conflicting requested quantities require clarification.
const small = ['zero','one','two','three','four','five','six','seven','eight','nine','ten',
    'eleven','twelve','thirteen','fourteen','fifteen','sixteen','seventeen','eighteen','nineteen'];
const tens = ['twenty','thirty','forty','fifty','sixty','seventy','eighty','ninety'];
const wordPart = `(?:${[...small,...tens,'hundred','thousand'].join('|')})`;
const words = `(?:(?:minus|negative)\\s+)?${wordPart}(?:[ -]+(?:and[ -]+)?${wordPart})*`;
const literal = '[+-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:e[+-]?\\d+)?';
const numberSource = `(?:${literal}|${words})`;
const numberAtStart = new RegExp(`^(${numberSource})(?!\\w|\\.\\d)`, 'i');
const connecting = /^(?:\s|[:=]|\b(?:to|of|is|at|by|exactly|equal|equals|equaling|the|a|an|vector|value|strength|magnitude|rest|dimensions)\b)*/i;

/** @param {string} token */
function numeric(token) {
    if (new RegExp(`^${literal}$`, 'i').test(token)) return Number(token);
    const parts = token.toLowerCase().split(/[ -]+/);
    const sign = ['minus','negative'].includes(parts[0]) ? (parts.shift(), -1) : 1;
    /** @param {string[]} group */
    const belowHundred = group => {
        if (group.length === 1 && small.includes(group[0])) return small.indexOf(group[0]);
        if (group.length >= 1 && group.length <= 2 && tens.includes(group[0]) && (group.length === 1 || small.indexOf(group[1]) > 0 && small.indexOf(group[1]) < 10)) return (tens.indexOf(group[0]) + 2) * 10 + (group.length === 2 ? small.indexOf(group[1]) : 0);
        return NaN;
    };
    /** @param {string[]} group */
    const belowThousand = group => {
        const at = group.indexOf('hundred');
        if (at < 0) return belowHundred(group);
        const hundreds = at === 0 ? 1 : at === 1 ? small.indexOf(group[0]) : NaN;
        if (!(hundreds > 0 && hundreds < 10)) return NaN;
        const rest = group.slice(at + 1);
        if (rest[0] === 'and') rest.shift();
        return hundreds * 100 + (rest.length ? belowHundred(rest) : 0);
    };
    const at = parts.indexOf('thousand');
    if (at < 0) return sign * belowThousand(parts);
    const rest = parts.slice(at + 1);
    if (rest[0] === 'and') rest.shift();
    return sign * ((at === 0 ? 1 : belowThousand(parts.slice(0,at))) * 1000 + (rest.length ? belowThousand(rest) : 0));
}

/** @param {string} tail */
function scalar(tail) {
    const rest = tail.replace(connecting, '');
    const match = rest.match(numberAtStart);
    if (match && new RegExp(`^\\s*(?:or|to|[-/])\\s*${numberSource}`, 'i').test(rest.slice(match[0].length))) return null;
    if (match && /^\s*,\s*[+-]?(?:\d|\.\d)/.test(rest.slice(match[0].length))) return null;
    return match ? numeric(match[1]) : null;
}

/** Read an explicit ordered triple or a completely labelled x/y/z triple.
 * @param {string} tail @returns {number[]|null}
 */
function vector(tail) {
    const source = tail.replace(connecting, '');
    const bracket = source.match(/^[\[(]([^\])]+)[\])]/);
    const body = bracket ? bracket[1] : source;
    const axes = [...body.matchAll(new RegExp(`\\b([xyz])\\s*[:=]\\s*(${numberSource})(?![\\w.])`, 'gi'))];
    if (axes.length) {
        if (axes.length !== 3 || new Set(axes.map(match => match[1])).size !== 3) return null;
        return ['x','y','z'].map(axis => numeric(axes.find(match => match[1] === axis)?.[2] || 'NaN'));
    }
    const values = [];
    let rest = body.trim();
    for (let index = 0; index < 3; index++) {
        const match = rest.match(numberAtStart);
        if (!match) return null;
        values.push(numeric(match[1]));
        rest = rest.slice(match[0].length);
        if (index < 2) {
            const separator = rest.match(/^\s*(?:,|\bby\b|[x×]|\band\b)\s*|^\s+/);
            if (!separator) return null;
            rest = rest.slice(separator[0].length);
        }
    }
    // Never truncate a four-component vector into an apparently valid triple.
    if (bracket ? rest.trim() : /^\s*(?:,|\bby\b|[x×])\s*[+-]?(?:\d|\.)/.test(rest)) return null;
    return values;
}

/** @param {string} text @param {RegExp} pattern */
function tails(text, pattern) {
    return [...text.matchAll(new RegExp(pattern.source, 'gi'))].map(match => text.slice((match.index ?? 0) + match[0].length));
}

/** @param {unknown[]} values @param {string} label */
function unique(values, label) {
    if (!values.length || values.some(value => value === null)) throw new Error(`Specify ${label} explicitly in the displayed simulation units.`);
    if (new Set(values.map(value => JSON.stringify(value))).size !== 1) throw new Error(`The request gives more than one ${label}. Send these changes separately so each value has a clear target.`);
    return values[0];
}

/** @param {any} schema */
function range(schema) {
    const bounds = schema.type === 'array' ? schema.items : schema;
    const limits = [bounds.minimum,bounds.maximum];
    return limits.some(value => value !== undefined)
        ? ` Allowed ${schema.type === 'array' ? 'component values' : 'values'}: ${limits[0] ?? '-infinity'} to ${limits[1] ?? 'infinity'}${bounds.type === 'integer' ? ', whole numbers only' : ''}.`
        : '';
}

/** Restrict candidate descriptors using explicit requested quantities. The return
 * value contains action DESCRIPTORS, never executable actions. No inputs mutate.
 * Any clarification aborts the entire proposed request; no partial plan is safe.
 * @param {string} text @param {ObservationEnvelope} observation @param {any[]} candidates
 * @returns {{actions:any[],clarification?:string}}
 */
export function constrainActions(text, observation, candidates) {
    const actions = structuredClone(candidates);
    const source = text.toLowerCase().replaceAll('−','-').replaceAll('–','-');
    // Questions quote quantities without authorising those quantities as edits.
    if (/^\s*(?:why|what|how|explain|describe|tell me|is\b|are\b|does\b|can you explain)\b/.test(source)) return { actions };
    try {
        const selected = observation.selected;
        const entity = selected?.current;
        const create = /\b(?:create|spawn|add (?:a|an|one)|new (?:object|shape|cube|box|sphere))\b/.test(source);
        const entityActions = create ? ['observer.create'] : ['observer.update'];
        const camera = /\b(?:camera|field of view|fov)\b/.test(source);
        const personal = /\b(?:selected|this|that|it|current object)\b/.test(source);
        const targetTypes = new Set(['observer.update','observer.delete','observer.restore','observer.impulse']);
        const bodyRequest = actions.some(action => targetTypes.has(action.type)) || /\b(?:mass|heavy|heavier|weight|dimensions|resize|position|velocity|move|impulse|push|kick|delete|remove|restore|undelete|rotate|rotation|acceleration|restitution|friction|damping|colou?r|rename)\b/.test(source);
        const needsTarget = !create && !camera && personal && bodyRequest;
        if (needsTarget && (!selected?.id || !entity || entity.alive === false && !/\b(?:restore|undelete)\b/.test(source))) {
            throw new Error('Select a current object first, then repeat the request. A historical crosshair observation is not a current selected target.');
        }

        /** Each enum is an intersection with the original property schema. Never
         * add a property that relevantActions or the live owner did not offer.
         * @param {string[]} types @param {string} key @param {any} value @param {string} label
         */
        function bind(types, key, value, label) {
            let count = 0;
            for (const action of actions) {
                if (!types.includes(action.type)) continue;
                const schema = action.args?.properties?.[key];
                if (!schema) continue;
                try { validateValue(value, schema, label); }
                catch { throw new Error(`The requested ${label} (${JSON.stringify(value)}) is outside this action's supported values.${range(schema)} Restate it; the value will not be clamped.`); }
                action.args.properties[key] = { ...schema, enum: [structuredClone(value)] };
                action.args.required = [...new Set([...(action.args.required || []),key])];
                count++;
            }
            if (!count) throw new Error(`Changing ${label} is not available in the current action catalog. Choose a supported operation in the current workspace.`);
        }

        // No implicit SI/imperial conversion exists in either workspace's API.
        // Degrees are accepted only by the degree-valued camera FOV property.
        const units = /\b(?:kilograms?|kg|grams?|pounds?|lbs?|ounces?|meters?|metres?|centimeters?|centimetres?|kilometers?|kilometres?|feet|foot|inches?|newtons?|joules?|seconds?|milliseconds?|minutes?|hours?|mph|kph)\b/;
        const shortUnits = new RegExp(`(?:${literal}|[\\]\\)])\\s*(?:kg|g|mg|m|cm|mm|km|s|ms|n|j)\\b`, 'i');
        if (units.test(source) || shortUnits.test(source)) throw new Error('This request uses physical units without an available conversion. Restate the quantity in the displayed simulation units, or use an explicit tick count for time.');
        if (/%|\bpercent(?:age)?\b/.test(source)) throw new Error('State an absolute value or an explicit multiplier in simulation units. Percentage conversion is not available in this command.');
        if (/\bdegrees?\b|°/.test(source) && !/\b(?:field of view|fov)\b/.test(source)) throw new Error('Only camera field of view accepts degrees here. State other angles in radians using the displayed units.');

        if (needsTarget) for (const action of actions) if (targetTypes.has(action.type) && action.args?.properties?.id) bind([action.type], 'id', selected.id, 'selected object id');

        const sr = observation.facts.profile === 'sr';
        if (sr && /\b(?:impulse|kick|push)\b/.test(source)) throw new Error('An impulse needs the Playground profile. Switch profiles explicitly before requesting an impulse.');

        // Mass ratios must remain ratios of the current stable target, rather
        // than arbitrary nearby positive numbers chosen by the language model.
        const massIntent = /\b(?:mass|weight|heavy|heavier)\b/.test(source);
        if (massIntent) {
            const factors = [];
            if (/\b(?:double|twice)\b/.test(source)) factors.push(2);
            if (/\b(?:triple|thrice)\b/.test(source)) factors.push(3);
            if (/\b(?:half|halve)\b/.test(source)) factors.push(0.5);
            for (const match of source.matchAll(new RegExp(`(?<![\\w.,])(${numberSource})\\s*times\\b`, 'gi'))) factors.push(numeric(match[1]));
            const factorTails = tails(source, /\bmass\s*factor\b/);
            factors.push(...factorTails.map(scalar));
            if (factors.length) {
                const factor = /** @type {number} */(unique(factors, 'mass multiplier'));
                if (tails(source,/\b(?:mass|weight)(?!\s*factor)\b/).some(tail => scalar(tail) !== null)) throw new Error('Specify either an absolute mass or a mass multiplier in this request, then observe before the next mass change.');
                if (create) throw new Error('A new object has no current mass to multiply. Specify its absolute mass in simulation units.');
                if (!entity || !selected?.id) throw new Error('Select a current object before changing its mass by a multiplier.');
                const absoluteSchema = observation.actions?.find(action => action.type === 'observer.update')?.args.properties.mass;
                if (!absoluteSchema || !Number.isFinite(entity.mass)) throw new Error('The selected object has no usable current mass for this relative edit.');
                try { validateValue(entity.mass * factor, absoluteSchema, 'resulting mass'); }
                catch { throw new Error(`Multiplying the current mass (${entity.mass}) by ${factor} would exceed the supported mass range.${range(absoluteSchema)} Choose another multiplier.`); }
                bind(entityActions, 'massFactor', factor, 'mass multiplier');
                for (const action of actions) if (entityActions.includes(action.type) && action.args.properties.mass && !action.args.required?.includes('mass')) delete action.args.properties.mass;
            } else {
                const values = tails(source, /\b(?:mass|weight)\b/).map(scalar);
                if (/\b(?:increase|decrease|reduce|add|subtract)\b[^.;]*\b(?:mass|weight)\b|\b(?:mass|weight)\b[^.;]*\bby\b/.test(source)) throw new Error('State the new absolute mass, or an explicit multiplier such as twice the current mass. An unspecified increase or decrease will not choose a value.');
                bind(entityActions, 'mass', unique(values,'mass'), 'mass');
                for (const action of actions) if (entityActions.includes(action.type) && action.args.properties.massFactor && !action.args.required?.includes('massFactor')) delete action.args.properties.massFactor;
            }
        }

        // Fields are anchored to their own words so a mass or tick elsewhere in
        // a multi-action sentence cannot accidentally supply a missing vector.
        /** @type {[string,RegExp,string[],string][]} */
        const vectors = [
            ['size',/\b(?:rest\s+)?(?:dimensions|size)\b/,entityActions,'dimensions [x, y, z]'],
            ['position',/\bposition\b/,camera ? ['observer.camera'] : entityActions,'position [x, y, z]'],
            ['velocity',/(?<!angular\s)\b(?:linear\s+)?velocity\b/,entityActions,'velocity [x, y, z]'],
            ['angularVelocity',/\bangular\s+velocity\b/,entityActions,'angular velocity [x, y, z]'],
            ['properAcceleration',/\b(?:proper\s+)?acceleration\b/,camera ? [] : entityActions,'proper acceleration [x, y, z]'],
            ['rotation',/\b(?:rotation|orientation)\b/,entityActions,'rotation [x, y, z]'],
            ['impulse',/\bimpulse\b/,['observer.impulse'],'impulse [x, y, z]'],
            ['color',/\b(?:colou?r|rgb)\b/ ,entityActions,'RGB color [red, green, blue]'],
        ];
        for (const [key,pattern,types,label] of vectors) {
            if (!types.length) continue;
            let found = tails(source,pattern);
            if (key === 'position' && !found.length && create) found = tails(source,/\bat\s*(?=[\[(])/);
            if (key === 'position' && !found.length && !camera && /\b(?:move|translate)\b/.test(source)) {
                found = tails(source,/\b(?:move|translate)\b[^.;]*?\bto\b/);
                if (!found.length) throw new Error('Specify the destination position as [x, y, z] in the selected object\'s authoring frame.');
            }
            if (!found.length) continue;
            // Named colors remain a model/catalog task; only explicit RGB
            // triples belong to this numeric reader.
            if (key === 'color') { found = found.map(tail => tail.replace(/^\s*(?:to\s+)?rgb\s*/,' ')); if (!found.some(tail => /^[\s:=]*(?:to\s*)?[\[(]/.test(tail))) continue; }
            const value = /** @type {number[]} */(unique(found.map(vector),label));
            if (key === 'velocity' && sr && Math.hypot(...value) > 0.99) throw new Error('SR velocity must have magnitude at most 0.99 in the c = 1 reference world. Supply a slower vector; it will not be rescaled.');
            if (key === 'angularVelocity' && sr && value.some(component => component !== 0)) throw new Error('Continuous rigid rotation requires Playground. Switch profiles explicitly before setting nonzero angular velocity.');
            if (key === 'properAcceleration' && sr && value.some(component => component !== 0) && !['clock','beacon'].includes(entity?.shape)) throw new Error('SR proper acceleration is supported only for clock or beacon markers. This selected extended object requires Playground.');
            if (key === 'impulse' && value.every(component => component === 0)) throw new Error('An impulse must be nonzero. Specify its three components and direction.');
            bind(types,key,value,label);
        }

        const steps = actions.filter(action => action.type.endsWith('.step'));
        if (/\b(?:tick|ticks|steps?)\b/.test(source) || /\badvance\b/.test(source)) {
            const counts = [...source.matchAll(new RegExp(`(?<![\\w.,])(${numberSource})\\s*(?:lattice\\s+|observer\\s+)?(?:ticks?|steps?)\\b`,'gi'))].map(match => numeric(match[1]));
            if (!counts.length) for (const tail of tails(source,/\b(?:step|advance)\b/)) { const value = scalar(tail); if (value !== null) counts.push(value); }
            if (!counts.length && /\b(?:one|single)\s+(?:tick|step)\b/.test(source)) counts.push(1);
            if (steps.length || /\b(?:tick|ticks)\b/.test(source)) bind(steps.map(action => action.type),'count',unique(counts,'whole tick count'),'tick count');
        }

        /** @type {[string,RegExp,string[],string][]} */
        const scalars = [
            ['gravityStrength',/\b(?:world\s+)?gravity\s+strength\b/,['observer.world'],'gravity strength'],
            ['restitution',/\brestitution\b/,entityActions,'restitution'],
            ['friction',/\bfriction\b/,entityActions,'friction'],
            ['damping',/\bdamping\b/,entityActions,'damping'],
            ['emission',/\bemission\b/,entityActions,'emission'],
            ['multiplier',/\b(?:force[- ]gun\s+)?multiplier\b/,['observer.forceGun'],'force-gun multiplier'],
            ['fov',/\b(?:field of view|fov)\b/,['observer.camera'],'camera field of view'],
        ];
        for (const [key,pattern,types,label] of scalars) {
            const found = tails(source,pattern);
            if (!found.length || !actions.some(action => types.includes(action.type))) continue;
            bind(types,key,unique(found.map(scalar),label),label);
        }
        if (/\b(?:increase|decrease|reduce|stronger|weaker)\b[^.;]*\bgravity\b|\bgravity\b[^.;]*\b(?:stronger|weaker)\b/.test(source) && !/\bgravity\s+strength\b/.test(source)) throw new Error('Specify the world gravity strength as an absolute number in simulation units. A vague increase or decrease will not choose a value.');
        return { actions };
    } catch (error) {
        return { actions: [], clarification: error instanceof Error ? error.message : 'Restate the requested quantities and target explicitly.' };
    }
}
