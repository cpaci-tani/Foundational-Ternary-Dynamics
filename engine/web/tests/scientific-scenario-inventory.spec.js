import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import {
    SCALE0_SCENARIOS,
} from '../js/scales/scale0/scenario-registry.js';
import {
    SCALE1_SCENARIOS, SCALE1_SCENARIO_TARGET_COUNT,
} from '../js/scales/scale1/scenario-registry.js';
import {
    AE_CURATED_SCENARIOS,
} from '../js/scales/scale2/scenario-registry.js';
import { ELEMENT_COUNT } from '../js/elements.js';
import { getAllMolecules } from '../js/molecules.js';
import {
    getScale4ScenarioToolbarTemplate,
} from '../js/scales/scale4/ui/toolbar/template.js';
import {
    getScale5ScenarioToolbarTemplate,
} from '../js/scales/scale5/ui/toolbar/template.js';
import {
    getKnowledgeBaseEntry,
} from '../js/ui/components/knowledge-base/data.js';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = path.resolve(HERE, '..');

function optionIds(html, prefixPattern) {
    return [...html.matchAll(/<option\s+value="([^"]+)"/g)]
        .map((match) => match[1])
        .filter((id) => prefixPattern.test(id));
}

function expectUnique(ids, label) {
    expect(new Set(ids).size, `${label} IDs must remain unique`).toBe(ids.length);
}

test('the frozen live inventory remains synchronized with the audit baseline', () => {
    const scale0Ids = SCALE0_SCENARIOS.map((scenario) => scenario.id);
    const scale1Ids = SCALE1_SCENARIOS.map((scenario) => scenario.id);
    const scale2Count = AE_CURATED_SCENARIOS.length + ELEMENT_COUNT - 1;
    const scale3Count = getAllMolecules().length + 2;
    const scale4Ids = optionIds(
        getScale4ScenarioToolbarTemplate(),
        /^(?:planetary-|exo-)/,
    );
    const scale5Ids = optionIds(
        getScale5ScenarioToolbarTemplate(),
        /^cosmic-/,
    );

    expectUnique(scale0Ids, 'Scale 0');
    expectUnique(scale1Ids, 'Scale 1');
    expectUnique(scale4Ids, 'Scale 4');
    expectUnique(scale5Ids, 'Scale 5');

    expect(scale0Ids).toHaveLength(142);
    // Node sees the one-row pre-WASM bootstrap. The authoritative native
    // registry hydrates the full particle-scale program in the browser.
    expect(scale1Ids).toHaveLength(1);
    expect(SCALE1_SCENARIO_TARGET_COUNT).toBe(36);
    expect(AE_CURATED_SCENARIOS).toHaveLength(29);
    expect(ELEMENT_COUNT).toBe(118);
    expect(scale2Count).toBe(146);
    expect(getAllMolecules()).toHaveLength(25);
    expect(scale3Count).toBe(27);
    expect(scale4Ids).toHaveLength(8);
    expect(scale5Ids).toHaveLength(13);

    const totalPresentationEntries = scale0Ids.length
        + SCALE1_SCENARIO_TARGET_COUNT
        + scale2Count
        + scale3Count
        + scale4Ids.length
        + scale5Ids.length
        + 1; // Scale 6 structural exhibit, not a physics scenario.
    // Includes the Scale-1 batteries, the Scale-0 membrane/resonant-cell
    // additions, and Scale 2's validation and nuclear-reaction laboratories.
    expect(totalPresentationEntries).toBe(373);
});

test('the manifest schema contains every pinned scientific-contract field', () => {
    const schemaPath = path.join(
        WEB_ROOT,
        'data',
        'scientific-scenario-manifest.schema.json',
    );
    const schema = JSON.parse(fs.readFileSync(schemaPath, 'utf8'));
    const required = new Set(schema.$defs.scenario.required);
    const pinned = [
        'id',
        'display_name',
        'scale',
        'model_relation',
        'experimental_roles',
        'record_lifecycle',
        'mathematical_model',
        'state_space',
        'initial_conditions',
        'boundary_conditions',
        'enabled_terms',
        'parameter_provenance',
        'native_units',
        'physical_calibration',
        'observables',
        'accepted_claims',
        'prohibited_claims',
        'validation_protocol',
        'acceptance_gates',
        'falsification_gates',
        'known_limitations',
        'backend_support',
        'backend_tolerances',
        'resolution_domain',
        'cross_scale_inputs',
        'visual_defaults',
        'performance_budget',
        'evidence_links',
        'contract_version',
    ];

    for (const field of pinned) {
        expect(required.has(field), `schema must require ${field}`).toBe(true);
    }
    expect(schema.additionalProperties).toBe(false);
    expect(schema.$defs.scenario.additionalProperties).toBe(false);
    expect(schema.$defs.gates.required).toHaveLength(7);
    expect(schema.$defs.claim.required).toContain('epistemic_tag');
    expect(schema.$defs.observable.required).toContain('allowed_interpretation');
});

test('Scenario 1 remains open and blocks progression', () => {
    const recordPath = path.join(
        WEB_ROOT,
        'docs',
        'audits',
        'scenarios',
        'AUDIT_SCALE0_EMPTY_BASELINE_v1.md',
    );
    const record = fs.readFileSync(recordPath, 'utf8');

    expect(record).toContain('**Scenario ID:** `empty`');
    expect(record).toContain('**[OPEN]** qualification in progress');
    expect(record).toContain('**Next scenario may open:** `no`');
    expect(record).toContain('vacuum physics');
    expect(record).toContain('16.67 ms frame target');
});

test('the empty-scenario knowledge entry states the imposed null-control boundary', () => {
    const entry = getKnowledgeBaseEntry('scenario-empty');
    expect(entry).toBeTruthy();

    const scientificCopy = [
        entry.summary,
        ...(entry.body || []),
        ...(entry.bullets || []),
        ...(entry.notation || []),
    ].join(' ');

    expect(scientificCopy).toContain('imposed');
    expect(scientificCopy).toContain('null control');
    expect(scientificCopy).toContain('not a physical-vacuum identification');
    expect(scientificCopy).toContain('Clocks and bookkeeping may advance');
    expect(scientificCopy).not.toContain('vacuum-like');
});
