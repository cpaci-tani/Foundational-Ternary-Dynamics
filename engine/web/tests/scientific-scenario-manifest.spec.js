// @ts-check
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = path.resolve(HERE, '..');
const MANIFEST_PATH = path.join(
    WEB_ROOT,
    'data',
    'scientific-scenario-manifest.json',
);
const SCHEMA_PATH = path.join(
    WEB_ROOT,
    'data',
    'scientific-scenario-manifest.schema.json',
);

const manifest = JSON.parse(fs.readFileSync(MANIFEST_PATH, 'utf8'));
const schema = JSON.parse(fs.readFileSync(SCHEMA_PATH, 'utf8'));
const GATE_IDS = [...schema.$defs.gateId.enum];

function sameJson(left, right) {
    return JSON.stringify(left) === JSON.stringify(right);
}

function resolveRef(root, ref) {
    if (!ref.startsWith('#/')) throw new Error(`unsupported schema ref: ${ref}`);
    return ref.slice(2).split('/').reduce((node, token) => node[token], root);
}

function matchesType(value, type) {
    if (type === 'null') return value === null;
    if (type === 'array') return Array.isArray(value);
    if (type === 'object') {
        return value !== null && typeof value === 'object' && !Array.isArray(value);
    }
    if (type === 'integer') return Number.isInteger(value);
    if (type === 'number') return typeof value === 'number' && Number.isFinite(value);
    return typeof value === type;
}

/**
 * Small Draft-2020-12 subset validator for the keywords used by the checked-in
 * manifest schema. Keeping it here avoids adding a package dependency while
 * still rejecting missing, extra, mistyped, malformed, or conditionally
 * invalid manifest data.
 */
function schemaErrors(value, rule, root = schema, location = '$') {
    if (!rule || Object.keys(rule).length === 0) return [];
    if (rule.$ref) return schemaErrors(value, resolveRef(root, rule.$ref), root, location);

    const errors = [];
    const types = rule.type == null
        ? null
        : (Array.isArray(rule.type) ? rule.type : [rule.type]);
    if (types && !types.some((type) => matchesType(value, type))) {
        return [`${location}: expected ${types.join('|')}`];
    }
    if (Object.hasOwn(rule, 'const') && !sameJson(value, rule.const)) {
        errors.push(`${location}: const mismatch`);
    }
    if (rule.enum && !rule.enum.some((candidate) => sameJson(value, candidate))) {
        errors.push(`${location}: value is outside enum`);
    }
    if (typeof value === 'string') {
        if (rule.minLength != null && value.length < rule.minLength) {
            errors.push(`${location}: shorter than minLength`);
        }
        if (rule.pattern && !(new RegExp(rule.pattern)).test(value)) {
            errors.push(`${location}: does not match ${rule.pattern}`);
        }
        if (rule.format === 'date') {
            const exactDate = /^\d{4}-\d{2}-\d{2}$/.test(value)
                && new Date(`${value}T00:00:00Z`).toISOString().startsWith(value);
            if (!exactDate) errors.push(`${location}: invalid date`);
        }
    }
    if (typeof value === 'number') {
        if (rule.minimum != null && value < rule.minimum) {
            errors.push(`${location}: below minimum`);
        }
        if (rule.exclusiveMinimum != null && value <= rule.exclusiveMinimum) {
            errors.push(`${location}: not above exclusiveMinimum`);
        }
    }
    if (Array.isArray(value)) {
        if (rule.minItems != null && value.length < rule.minItems) {
            errors.push(`${location}: fewer than minItems`);
        }
        if (rule.maxItems != null && value.length > rule.maxItems) {
            errors.push(`${location}: more than maxItems`);
        }
        if (rule.uniqueItems) {
            const keys = value.map((item) => JSON.stringify(item));
            if (new Set(keys).size !== keys.length) errors.push(`${location}: duplicate items`);
        }
        if (rule.items) {
            value.forEach((item, index) => {
                errors.push(...schemaErrors(item, rule.items, root, `${location}[${index}]`));
            });
        }
    }
    if (value !== null && typeof value === 'object' && !Array.isArray(value)) {
        for (const required of rule.required || []) {
            if (!Object.hasOwn(value, required)) {
                errors.push(`${location}: missing ${required}`);
            }
        }
        if (rule.additionalProperties === false) {
            const allowed = new Set(Object.keys(rule.properties || {}));
            for (const key of Object.keys(value)) {
                if (!allowed.has(key)) errors.push(`${location}: unexpected ${key}`);
            }
        }
        for (const [key, childRule] of Object.entries(rule.properties || {})) {
            if (Object.hasOwn(value, key)) {
                errors.push(...schemaErrors(value[key], childRule, root, `${location}.${key}`));
            }
        }
    }
    for (const childRule of rule.allOf || []) {
        errors.push(...schemaErrors(value, childRule, root, location));
    }
    if (rule.if && schemaErrors(value, rule.if, root, location).length === 0 && rule.then) {
        errors.push(...schemaErrors(value, rule.then, root, location));
    }
    return errors;
}

function expectUnique(values, label) {
    expect(new Set(values).size, `${label} must be unique`).toBe(values.length);
}

function backendKey(record) {
    return `${record.compute_backend}/${record.runtime}/${record.transport}`;
}

function collectEvidence(value, output = []) {
    if (Array.isArray(value)) {
        value.forEach((item) => collectEvidence(item, output));
    } else if (value && typeof value === 'object') {
        if (Object.hasOwn(value, 'commit')
            && Object.hasOwn(value, 'digest')
            && Object.hasOwn(value, 'path_or_uri')) {
            output.push(value);
        }
        Object.values(value).forEach((item) => collectEvidence(item, output));
    }
    return output;
}

test('the initial empty manifest satisfies the checked-in JSON schema', () => {
    expect(schemaErrors(manifest, schema)).toEqual([]);
});

test('scale/scenario identity, references, and aliases are unambiguous', () => {
    const scaleIds = manifest.scales.map((scale) => scale.id);
    const scenarioIds = manifest.scenarios.map((scenario) => scenario.id);
    const aliasIds = manifest.aliases.map((alias) => alias.alias);
    expectUnique(scaleIds, 'scale IDs');
    expectUnique(scenarioIds, 'scenario IDs');
    expectUnique(aliasIds, 'alias IDs');

    const scaleSet = new Set(scaleIds);
    for (const scenario of manifest.scenarios) {
        expect(scaleSet.has(scenario.scale), `${scenario.id} scale must resolve`).toBe(true);
    }

    const canonicalIds = new Set(scenarioIds);
    for (const alias of manifest.aliases) {
        expect(canonicalIds.has(alias.canonical_id), `${alias.alias} must resolve`).toBe(true);
        expect(canonicalIds.has(alias.alias), `${alias.alias} must not shadow a canonical ID`).toBe(false);
        if (alias.support === 'supported') {
            expect(alias.scientific_equivalence,
                `${alias.alias} cannot be supported without scientific equivalence`).toBe(true);
        }
    }
});

test('all gates are present and open gates prevent a qualified state', () => {
    const qualifiedStates = new Set([
        'qualified-within-contract',
        'qualified-parametric',
    ]);
    for (const scenario of manifest.scenarios) {
        expect(Object.keys(scenario.acceptance_gates).sort()).toEqual([...GATE_IDS].sort());
        const openGates = GATE_IDS.filter(
            (gateId) => scenario.acceptance_gates[gateId].status !== 'pass',
        );
        if (qualifiedStates.has(scenario.qualification_state)) {
            expect(openGates, `${scenario.id} cannot qualify with open gates`).toEqual([]);
        }
        for (const gateId of GATE_IDS) {
            const gate = scenario.acceptance_gates[gateId];
            if (gate.status === 'pass') {
                expect(gate.evidence.length, `${scenario.id}/${gateId} pass needs evidence`)
                    .toBeGreaterThan(0);
                expect(gate.blocking_findings, `${scenario.id}/${gateId} pass cannot stay blocked`)
                    .toEqual([]);
            }
        }
    }
});

test('accepted claims require passed supporting gates and frozen evidence', () => {
    for (const scenario of manifest.scenarios) {
        for (const claim of scenario.accepted_claims) {
            expect(claim.disposition).toBe('accepted-within-scope');
            expect(claim.supporting_gates.length, `${claim.claim_id} needs supporting gates`)
                .toBeGreaterThan(0);
            expect(claim.evidence.length, `${claim.claim_id} needs evidence`).toBeGreaterThan(0);
            for (const gateId of claim.supporting_gates) {
                expect(scenario.acceptance_gates[gateId]?.status,
                    `${claim.claim_id} depends on a non-passing gate ${gateId}`).toBe('pass');
            }
        }
    }
});

test('evidence commit and digest syntax is immutable and commit-frozen', () => {
    const evidence = collectEvidence(manifest);
    expect(evidence.length).toBeGreaterThan(0);
    for (const artifact of evidence) {
        expect(artifact.commit).toMatch(/^[0-9a-f]{40}$/);
        expect(artifact.digest).toMatch(/^sha256:[0-9a-f]{64}$/);
        expect(artifact.commit, `${artifact.path_or_uri} is backdated or post-freeze`)
            .toBe(manifest.frozen_at_commit);
    }
});

test('empty remains an imposed null control and blocks Scenario 2', () => {
    expect(manifest.frozen_at_commit).toBe('d5f672ad128584252643c9799da7153468d7a4a4');
    expect(manifest.scenarios).toHaveLength(1);
    const empty = manifest.scenarios[0];
    expect(empty.id).toBe('empty');
    expect(empty.scale).toBe('scale0');
    expect(empty.model_relation).toBe('native-operator');
    expect(empty.experimental_roles).toContain('null-control');
    expect(empty.record_lifecycle).toBe('open');
    expect(empty.qualification_state).toBe('in-progress');
    expect(empty.accepted_claims).toEqual([]);
    expect(empty.parameter_provenance.find((item) => item.name === 'initial-record')?.provenance)
        .toBe('IMPOSED');

    const prohibited = new Set(empty.prohibited_claims.map((claim) => claim.claim_id));
    expect(prohibited).toEqual(new Set([
        'empty-is-physical-vacuum',
        'empty-validates-nonzero-scenarios',
    ]));
    expect(empty.physical_calibration).toContain('None');
    expect(empty.physical_calibration.toLowerCase()).toContain('physical vacuum');
    expect(empty.visual_defaults.overlays).toEqual([]);
    expect(empty.visual_defaults.prohibited_overlays.join(' ').toLowerCase())
        .toContain('standard model');
    expect(empty.known_limitations).toContain(
        'Scenario 2 may not open while any Scenario 1 acceptance gate is not pass.',
    );
    expect(GATE_IDS.some((gateId) => empty.acceptance_gates[gateId].status !== 'pass'))
        .toBe(true);
});

test('execution paths remain separate and unknown never masquerades as zero', () => {
    const empty = manifest.scenarios.find((scenario) => scenario.id === 'empty');
    const records = new Map(empty.backend_support.map((record) => [backendKey(record), record]));
    expect([...records.keys()].sort()).toEqual([
        'cpu/native/in-process',
        'cpu/wasm-main/in-process',
        'cpu/wasm-worker/web-worker',
        'gpu/native/in-process',
        'gpu/native/websocket',
    ]);

    expect(records.get('cpu/native/in-process').availability).toBe('supported');
    expect(records.get('cpu/wasm-worker/web-worker').availability).toBe('partial');
    for (const key of [
        'cpu/wasm-main/in-process',
        'gpu/native/in-process',
        'gpu/native/websocket',
    ]) {
        const record = records.get(key);
        expect(record.availability).toBe('unknown');
        expect(record.tested_configurations, `${key} cannot invent a zero-valued run`).toEqual([]);
        expect(record.evidence, `${key} cannot invent zero-valued evidence`).toEqual([]);
        expect(record.limitations.join(' ').toLowerCase()).toContain('not');
        expect(record.limitations.join(' ').toLowerCase()).toContain('zero');
    }
    expect(records.get('cpu/wasm-worker/web-worker').limitations.join(' ').toLowerCase())
        .toContain('unavailable, not zero');
    expect(empty.backend_tolerances, 'no cross-runtime tolerance was qualified at the frozen commit')
        .toEqual([]);
});
