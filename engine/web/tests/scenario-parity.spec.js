// @ts-check
/**
 * JS ↔ C++ scenario-parity guard.
 *
 * After the April-2026 WASM-scenario port (engine/src/scenarios.cpp), all 83
 * UI-exposed Scale-0 scenarios have C++ implementations. The JS scenario group
 * files (engine/web/js/bridge/scenarios/*.js) are kept as dead-code until Phase 7
 * cleanup; in the meantime this lint ensures C++ coverage hasn't drifted.
 *
 * This is refactoring-analyst ticket RF-5 (Option B): a cheap lint that runs
 * as part of the Playwright suite and fails CI if the two sides drift.
 *
 * What it checks
 * --------------
 *   1. Every `case '…':` in JS group files has a matching C++ `name == "…"`
 *      branch in scenarios.cpp.
 *   2. Every C++ branch has a matching JS case (minus a small allowlist of
 *      legacy-only names kept for backward compat in ftd_wasm.cpp's post-
 *      dispatcher switch — listed in KNOWN_LEGACY_ONLY below).
 *   3. Every scenario in the UI registry (scenario-registry.js) has a JS
 *      implementation.
 *
 * Why node: no WASM load needed; we just parse source text. Fast.
 */
import { test, expect } from '@playwright/test';
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = resolve(__dirname, '..');
const ENGINE_ROOT = resolve(WEB_ROOT, '..');
const PROJECT_ROOT = resolve(ENGINE_ROOT, '..');

// Names that live only in the C++ legacy switch (ftd_wasm.cpp) — kept for
// backward compat with older tests/saved dashboards. Not in the JS UI.
const KNOWN_LEGACY_ONLY = new Set([
    // Short legacy aliases resolved in bindings_render_bridge.cpp (not catalog IDs).
    // 'empty' is a first-class dispatcher id in scenarios.cpp + JS index.js — not legacy-only.
    'annihilation', 'cluster', 'dipole', 'entangled', 'flux-collision',
    'flux-damping', 'flux-dispersion', 'flux-gravity-cluster', 'flux-hydrogen',
    'flux-ring', 'force', 'hydrogen', 'interference', 'light-prism', 'pair',
    'production', 'scattering', 'triad', 'vacuum', 'wave',
]);

// Scenarios that are delegated to another scenario in their JS load() method,
// so they don't need a standalone JS mock-bridge case or C++ implementation.
const DELEGATED_SCENARIOS = new Set([
    's0-seed-ew-phase-transition',
]);

// ── Extractors ──────────────────────────────────────────────────────

function extractJsScenarios() {
    const groupDir = join(WEB_ROOT, 'js', 'bridge', 'scenarios');
    const allFiles = readdirSync(groupDir).filter((f) => f.endsWith('.js'));
    // Scenario-id constants defined anywhere in the group dir, e.g.
    // spectrum-comparator.js: `export const RF_LATTICE_WAVE_SCENARIO_ID =
    // 's0-field-rf-lattice-wave';` — group files use these in identifier-form
    // `case RF_LATTICE_WAVE_SCENARIO_ID:` labels, which the string-literal
    // regex below cannot see (revision 0.4 lint-blindness fix: the four
    // spectrum-comparator wave scenarios were implemented all along).
    const idConsts = new Map();
    for (const f of allFiles) {
        const src = readFileSync(join(groupDir, f), 'utf8');
        const reConst = /const\s+([A-Z][A-Z0-9_]*)\s*=\s*['"]([^'"]+)['"]/g;
        let m;
        while ((m = reConst.exec(src))) idConsts.set(m[1], m[2]);
    }
    const files = allFiles.filter((f) =>
        f.endsWith('-scenarios.js') && f !== 'index.js'
    );
    const names = new Set();
    for (const f of files) {
        const src = readFileSync(join(groupDir, f), 'utf8');
        // Matches `case 'foo-bar':` (single or double quotes)
        const re = /case\s+['"]([^'"]+)['"]\s*:/g;
        let m;
        while ((m = re.exec(src))) names.add(m[1]);
        // Matches `case SOME_SCENARIO_ID:` resolved through idConsts.
        const reIdent = /case\s+([A-Z][A-Z0-9_]*)\s*:/g;
        while ((m = reIdent.exec(src))) {
            const resolved = idConsts.get(m[1]);
            if (resolved) names.add(resolved);
        }
    }
    // 'empty' is handled by the dispatcher itself (index.js), not in any group file.
    names.add('empty');
    return names;
}

function extractCppScenarios() {
    // April 2026 post-audit cleanup (ticket S1): scenarios.cpp was split into
    // 5 group files under engine/src/scenarios/. The router still lives in
    // scenarios.cpp but the `name == "..."` branches are spread across:
    //   scenarios/flux.cpp, light.cpp, quantum.cpp, s0_seed.cpp, s0_field.cpp,
    //   scenarios/vacuum.cpp (s0-vacuum-* group, added 2026-04-28)
    const sources = [
        join(ENGINE_ROOT, 'src', 'scenarios.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 'flux.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 'light.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 'quantum.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 's0_seed.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 's0_field.cpp'),
        join(ENGINE_ROOT, 'src', 'scenarios', 'vacuum.cpp'),
    ];
    const names = new Set();
    const re = /name\s*==\s*"([^"]+)"/g;
    for (const path of sources) {
        let src;
        try { src = readFileSync(path, 'utf8'); } catch { continue; }
        let m;
        while ((m = re.exec(src))) names.add(m[1]);
    }
    return names;
}

function extractCppLegacyScenarios() {
    // April 2026 post-audit cleanup (ticket W1): ftd_wasm.cpp was split.
    // The legacy setup_scenario with backward-compat branches now lives in
    // engine/wasm/bindings_render_bridge.cpp. Check both for forward compat.
    const sources = [
        join(ENGINE_ROOT, 'wasm', 'ftd_wasm.cpp'),
        join(ENGINE_ROOT, 'wasm', 'bindings_render_bridge.cpp'),
    ];
    const names = new Set();
    const re = /name\s*==\s*"([^"]+)"/g;
    for (const path of sources) {
        let src;
        try { src = readFileSync(path, 'utf8'); } catch { continue; }
        let m;
        while ((m = re.exec(src))) names.add(m[1]);
    }
    return names;
}

function extractUiRegistryScenarios() {
    const src = readFileSync(
        join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js'),
        'utf8'
    );
    const block = src.match(
        /export const SCALE0_SCENARIO_VALIDATION = Object\.freeze\(\{([\s\S]*?)\n\}\);/
    );
    const names = new Set();
    if (!block) return names;
    const re = /^\s*'([^']+)':\s*Object\.freeze\(/gm;
    let m;
    while ((m = re.exec(block[1]))) names.add(m[1]);
    return names;
}

function extractCatalogScenarios() {
    const src = readFileSync(
        join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js'),
        'utf8'
    );
    const names = new Set();
    // Matches `makeScenario('category', 'name', ...)` (factory form)
    const re = /makeScenario\('[^']+',\s*'([^']+)'/g;
    let m;
    while ((m = re.exec(src))) names.add(m[1]);
    // Also match the custom object-literal form `{ id: 'name', ... }` used by the
    // 10 toggle-preset scenarios (quark-gluon-plasma + emergent-ic*) that the
    // makeScenario regex misses (B5 fix, 2026-06-05).
    const reId = /\bid:\s*'([^']+)'/g;
    while ((m = reId.exec(src))) names.add(m[1]);
    return names;
}

function extractMetadataScenarios() {
    // Live S0_SEED_SCENARIO_METADATA keys (block comments stripped so the
    // reference-only / removed entries don't count). B5/B6 guard, 2026-06-05.
    const src = readFileSync(join(WEB_ROOT, 'js', 'config', 'scenarios.js'), 'utf8')
        .replace(/\/\*[\s\S]*?\*\//g, '');
    const names = new Set();
    const re = /'(s0-seed-[^']+)':\s*\{/g;
    let m;
    while ((m = re.exec(src))) names.add(m[1]);
    return names;
}

function extractTogglesTypedefScenarios() {
    const src = readFileSync(join(WEB_ROOT, 'js', 'config', 'toggles.js'), 'utf8');
    const typedefMatch = src.match(/@typedef\s*\{([^}]+)\}\s*ScenarioId/);
    if (!typedefMatch) return new Set();
    const re = /'([^']+)'/g;
    const names = new Set();
    let m;
    while ((m = re.exec(typedefMatch[1]))) {
        names.add(m[1]);
    }
    return names;
}

// ── Toggle extractors (revision 0.4 toggle-parity lint) ─────────────

function extractScale0Toggles() {
    // Parses the SCALE0_TOGGLES whitelist in engine/web/js/config/toggles.js:
    //   ['toggle_name', <default>, 'dom-id'],
    const src = readFileSync(join(WEB_ROOT, 'js', 'config', 'toggles.js'), 'utf8');
    const blockMatch = src.match(/export const SCALE0_TOGGLES = \[([\s\S]*?)\n\];/);
    if (!blockMatch) return new Map();
    const entries = new Map();
    const re = /\['([a-z0-9_]+)',\s*(true|false)/g;
    let m;
    while ((m = re.exec(blockMatch[1]))) entries.set(m[1], m[2] === 'true');
    return entries;
}

function extractCppToggleSpecs() {
    // Parses TOGGLE_SPECS[] rows in engine/include/ftd/term_toggles.h:
    //   {"name", &TermToggles::field, <default>, ...},
    const src = readFileSync(
        join(ENGINE_ROOT, 'include', 'ftd', 'term_toggles.h'), 'utf8');
    const entries = new Map();
    const re = /\{"([a-z0-9_]+)",\s*&TermToggles::[a-z0-9_]+,\s*(true|false)/g;
    let m;
    while ((m = re.exec(src))) entries.set(m[1], m[2] === 'true');
    return entries;
}

function extractFreeWaveDisabledTerms() {
    const src = readFileSync(
        join(WEB_ROOT, 'js', 'bridge', 'scenarios', '_helpers.js'), 'utf8');
    const blockMatch = src.match(
        /export const FREE_WAVE_DISABLED_TERMS = Object\.freeze\(\[([\s\S]*?)\]\);/);
    if (!blockMatch) return new Set();
    const entries = new Set();
    const re = /'([a-z0-9_]+)'/g;
    let m;
    while ((m = re.exec(blockMatch[1]))) entries.add(m[1]);
    return entries;
}

// SCALE0_TOGGLES defaults are the dashboard's SCENARIO-RESET baseline, not the
// C++ construction default — four toggles intentionally diverge (the dashboard
// baseline profile starts them off; the C++ TermToggles constructor starts
// them on). This map characterizes the known divergences as {js, cpp} pairs.
// If a divergence disappears (or a new one appears) this lint fails, forcing
// the change to be acknowledged here in the same commit.
const INTENTIONAL_DEFAULT_DIVERGENCES = new Map([
    ['gravity',            { js: false, cpp: true }],
    ['lorentz_force',      { js: false, cpp: true }],
    ['dual_substrate',     { js: false, cpp: true }],
    ['weak_transmutation', { js: false, cpp: true }],
]);

// ── Tests ───────────────────────────────────────────────────────────

test.describe('Toggle parity (JS whitelist ⊆ C++ TOGGLE_SPECS)', () => {
    test('every SCALE0_TOGGLES key exists in TOGGLE_SPECS', () => {
        const js = extractScale0Toggles();
        const cpp = extractCppToggleSpecs();
        // Regex-rot guards: both extractions must find a plausible population.
        expect(js.size, 'SCALE0_TOGGLES extraction found too few entries — regex rot?')
            .toBeGreaterThanOrEqual(15);
        expect(cpp.size, 'TOGGLE_SPECS extraction found too few entries — regex rot?')
            .toBeGreaterThanOrEqual(30);
        const unknown = [...js.keys()].filter((name) => !cpp.has(name));
        expect(unknown,
            `${unknown.length} SCALE0_TOGGLES keys have no TOGGLE_SPECS row in ` +
            `engine/include/ftd/term_toggles.h — a C++ rename/removal has stranded the JS ` +
            `whitelist (setToggle on these is silently dropped by rb_toggle_map).\n` +
            `Unknown:\n  - ${unknown.join('\n  - ')}`
        ).toEqual([]);
        // NOTE: subset only — the C++-side extra toggles (research controls,
        // non-whitelisted by design per CONTRACTS.md §4 and the comment block
        // below SCALE0_TOGGLES) are intentionally absent from JS.
    });

    test('scenario-reset defaults match C++ defaults except documented divergences', () => {
        const js = extractScale0Toggles();
        const cpp = extractCppToggleSpecs();
        const problems = [];
        for (const [name, jsDefault] of js) {
            if (!cpp.has(name)) continue; // covered by the subset test above
            const cppDefault = cpp.get(name);
            const known = INTENTIONAL_DEFAULT_DIVERGENCES.get(name);
            if (known) {
                if (known.js !== jsDefault || known.cpp !== cppDefault) {
                    problems.push(`${name}: documented divergence {js:${known.js}, cpp:${known.cpp}} ` +
                        `no longer matches reality {js:${jsDefault}, cpp:${cppDefault}} — update ` +
                        `INTENTIONAL_DEFAULT_DIVERGENCES in this spec`);
                }
            } else if (jsDefault !== cppDefault) {
                problems.push(`${name}: JS scenario-reset default (${jsDefault}) drifted from C++ ` +
                    `TOGGLE_SPECS default (${cppDefault}) with no documented divergence entry`);
            }
        }
        expect(problems, problems.join('\n')).toEqual([]);
    });

    test('isolated JS wave profile disables every non-wave C++ toggle', () => {
        const cpp = extractCppToggleSpecs();
        const disabled = extractFreeWaveDisabledTerms();
        const expected = new Set([...cpp.keys()].filter(
            (name) => name !== 'wave_propagation' && name !== 'gauss_projection'));
        const missing = [...expected].filter((name) => !disabled.has(name));
        const unknown = [...disabled].filter((name) => !cpp.has(name));
        expect(disabled.size, 'FREE_WAVE_DISABLED_TERMS extraction failed').toBeGreaterThan(30);
        expect(missing, `non-wave C++ toggles omitted from the isolated JS profile: ${missing.join(', ')}`)
            .toEqual([]);
        expect(unknown, `isolated JS profile contains unknown toggles: ${unknown.join(', ')}`)
            .toEqual([]);
    });
});

test.describe('Scenario parity (JS ↔ C++)', () => {
    test('every JS scenario has a C++ implementation in scenarios.cpp', () => {
        const js = extractJsScenarios();
        const cpp = extractCppScenarios();
        const missing = [];
        for (const name of js) {
            if (name === 'empty') continue; // handled by dispatcher, not group
            if (!cpp.has(name)) missing.push(name);
        }
        expect(missing,
            `${missing.length} JS scenarios are missing from engine/src/scenarios.cpp.\n` +
            `Add a corresponding C++ branch (name == "X") or remove the JS entry.\n` +
            `Missing:\n  - ${missing.join('\n  - ')}`
        ).toEqual([]);
    });

    test('every C++ scenario in scenarios.cpp has a JS implementation', () => {
        const js = extractJsScenarios();
        const cpp = extractCppScenarios();
        const missing = [];
        for (const name of cpp) {
            // DELEGATED_SCENARIOS reach their physics via another scenario in
            // the JS registry load(); C++ keeps a native branch for CLI/tests.
            if (DELEGATED_SCENARIOS.has(name)) continue;
            if (!js.has(name)) missing.push(name);
        }
        expect(missing,
            `${missing.length} C++ scenarios are missing from JS group files.\n` +
            `Add a corresponding 'case' in engine/web/js/bridge/scenarios/ or remove the C++ branch.\n` +
            `Missing:\n  - ${missing.join('\n  - ')}`
        ).toEqual([]);
    });

    test('every legacy-switch scenario in ftd_wasm.cpp is either shared or on the allowlist', () => {
        const legacy = extractCppLegacyScenarios();
        const cpp = extractCppScenarios();
        const unexplained = [];
        for (const name of legacy) {
            // Matches a ported scenario → dispatcher handles it (legacy is dead code here).
            if (cpp.has(name)) continue;
            // On the allowlist → backward-compat only.
            if (KNOWN_LEGACY_ONLY.has(name)) continue;
            unexplained.push(name);
        }
        expect(unexplained,
            `${unexplained.length} ftd_wasm.cpp legacy scenarios have neither a ported dispatcher branch ` +
            `nor a KNOWN_LEGACY_ONLY allowlist entry. Either port them to scenarios.cpp or add them to ` +
            `the allowlist in tests/scenario-parity.spec.js.\n` +
            `Unexplained:\n  - ${unexplained.join('\n  - ')}`
        ).toEqual([]);
    });

    test('every UI-registered scenario has a JS (and therefore C++) implementation', () => {
        const ui = extractUiRegistryScenarios();
        const js = extractJsScenarios();
        const missing = [];
        for (const name of ui) {
            if (DELEGATED_SCENARIOS.has(name)) continue;
            if (!js.has(name)) missing.push(name);
        }
        expect(missing,
            `${missing.length} scenarios appear in the UI dropdown but have no JS implementation.\n` +
            `Add the case to a group file in engine/web/js/bridge/scenarios/ or remove from the UI registry.\n` +
            `Missing:\n  - ${missing.join('\n  - ')}`
        ).toEqual([]);
    });

    test('ScenarioId JSDoc typedef covers the validated UI and contains only catalogued IDs', () => {
        const ui = extractUiRegistryScenarios();
        const catalog = extractCatalogScenarios();
        const typedefScenarios = extractTogglesTypedefScenarios();

        const missingInTypedef = [...ui].filter(name => !typedefScenarios.has(name));
        const unknownInTypedef = [...typedefScenarios].filter(name => !catalog.has(name));

        expect(missingInTypedef,
            `The JSDoc @typedef ScenarioId in toggles.js is missing these scenarios:\n  - ${missingInTypedef.join('\n  - ')}`
        ).toEqual([]);

        expect(unknownInTypedef,
            `The JSDoc @typedef ScenarioId in toggles.js contains these uncatalogued scenarios:\n  - ${unknownInTypedef.join('\n  - ')}`
        ).toEqual([]);
    });

    test('every UI scenario has behavioral evidence in a real automated test', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const registry = await import(pathToFileURL(modulePath).href);
        const visible = registry.SCALE0_SCENARIOS;
        const catalog = registry.SCALE0_SCENARIO_CATALOG;
        const evidence = registry.SCALE0_SCENARIO_VALIDATION;

        expect(visible.length, 'validated menu must not be empty').toBeGreaterThan(0);
        expect(catalog.length, 'every catalogued implementation should now be admitted')
            .toBe(visible.length);
        expect(visible.map((s) => s.id).sort(), 'menu and evidence manifest must match exactly')
            .toEqual(Object.keys(evidence).sort());

        const defects = [];
        for (const scenario of visible) {
            const proof = scenario.validation;
            if (!proof || proof.level !== 'behavioral') {
                defects.push(`${scenario.id}: missing behavioral validation level`);
                continue;
            }
            const testPath = resolve(PROJECT_ROOT, proof.test || '');
            if (!existsSync(testPath)) {
                defects.push(`${scenario.id}: missing test file ${proof.test}`);
                continue;
            }
            const testSource = readFileSync(testPath, 'utf8');
            if (!testSource.includes(`"${scenario.id}"`)
                && !testSource.includes(`'${scenario.id}'`)) {
                defects.push(`${scenario.id}: evidence file does not name the scenario ID`);
            }
            if (typeof proof.assertion !== 'string' || proof.assertion.length < 24) {
                defects.push(`${scenario.id}: validation assertion is absent or non-specific`);
            }
            if (typeof proof.qualification !== 'string' || proof.qualification.length < 24) {
                defects.push(`${scenario.id}: qualification is absent or non-specific`);
            }
        }
        expect(defects, defects.join('\n')).toEqual([]);
    });

    test('every catalog scenario has a consistent admission state', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const registry = await import(pathToFileURL(modulePath).href);
        const visibleIds = new Set(registry.SCALE0_SCENARIOS.map((s) => s.id));
        const defects = [];
        let hiddenCount = 0;

        for (const scenario of registry.SCALE0_SCENARIO_CATALOG) {
            const admitted = visibleIds.has(scenario.id);
            if (typeof scenario.sourceTitle !== 'string' || !scenario.sourceTitle) {
                defects.push(`${scenario.id}: source title missing`);
            }
            if (typeof scenario.qualification !== 'string' || scenario.qualification.length < 24) {
                defects.push(`${scenario.id}: qualification missing or non-specific`);
            }
            if (admitted) {
                if (scenario.admissionStatus !== 'admitted-behavioral'
                    || scenario.evidenceLevel !== 'behavioral'
                    || !scenario.validation) {
                    defects.push(`${scenario.id}: admitted scenario lacks behavioral evidence state`);
                }
            } else {
                hiddenCount += 1;
                if (scenario.admissionStatus !== 'hidden-research'
                    || scenario.evidenceLevel !== 'mechanical-smoke-only'
                    || scenario.validation !== null) {
                    defects.push(`${scenario.id}: hidden scenario has an invalid admission/evidence state`);
                }
                if (!scenario.title.includes('Research Setup (Behavior Unvalidated)')) {
                    defects.push(`${scenario.id}: hidden title is not explicitly qualified`);
                }
                if (scenario.mechanicalTest !== 'engine/web/tests/scale0-scenario-health.spec.js') {
                    defects.push(`${scenario.id}: mechanical smoke-test provenance missing`);
                }
            }
        }

        expect(hiddenCount, 'hidden count must equal the catalog/menu difference')
            .toBe(registry.SCALE0_SCENARIO_CATALOG.length - visibleIds.size);
        expect(defects, defects.join('\n')).toEqual([]);
    });

    test('every metadata entry maps to a real scenario (no orphan docs)', () => {
        const meta = extractMetadataScenarios();
        const js = extractJsScenarios();
        const orphan = [...meta].filter((n) => !js.has(n));
        expect(orphan,
            `${orphan.length} S0_SEED_SCENARIO_METADATA entries describe scenarios with no JS ` +
            `implementation (orphaned docs). Remove them from engine/web/js/config/scenarios.js ` +
            `or add the scenario.\nOrphans:\n  - ${orphan.join('\n  - ')}`
        ).toEqual([]);
    });

    test('inventory summary (informational — no assertion)', () => {
        const ui = extractUiRegistryScenarios();
        const catalog = extractCatalogScenarios();
        const js = extractJsScenarios();
        const cpp = extractCppScenarios();
        const legacy = extractCppLegacyScenarios();

        console.log('\n=== Scenario inventory ===');
        console.log(`  UI registry (dropdown):     ${ui.size}`);
        console.log(`  Internal research catalog:  ${catalog.size}`);
        console.log(`  JS group files (cases):     ${js.size}`);
        console.log(`  C++ scenarios.cpp:          ${cpp.size}`);
        console.log(`  C++ legacy (ftd_wasm.cpp):  ${legacy.size}`);
        const shared = new Set([...js].filter((n) => cpp.has(n)));
        console.log(`  JS ∩ C++ (shared):          ${shared.size}`);

        // No assertion; this is for visibility only.
        expect(shared.size).toBeGreaterThan(0);
    });
});

// ---------------------------------------------------------------------------
// Catalog-count guard.
//
// The scenario total is derivable from the registry; every prose statement of
// it is hand-written. When the catalog last grew, the five definition layers
// stayed consistent with each other and the documents did not — and nothing
// detected it, because the inventory test above prints the counts without
// asserting one. These tests make the registry the sole source: the layers are
// pinned to one absolute number, and every documented restatement of it must
// equal that number.
// ---------------------------------------------------------------------------
const QUALIFICATION_DOC = 'docs/audits/AUDIT_SCALE0_SCENARIO_QUALIFICATION_2026-07-24.md';
const HEALTH_DOC = 'docs/audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md';

/** Every prose restatement of a derived catalog number, and what it must equal. */
function documentedCountClaims({ total, behavioral, remainder }) {
    const tuple = Array(5).fill(total).join('/');
    return [
        // `\s+` rather than a literal space throughout: these are prose claims in
        // wrapped markdown, and a guard that breaks when a paragraph is rewrapped
        // would be reported as drift when nothing drifted.
        { file: QUALIFICATION_DOC, label: 'scope line',
          re: /\ball\s+(\d{2,4})\s+Scale-0\s+scenario\s+IDs\b/g, want: String(total) },
        { file: QUALIFICATION_DOC, label: 'class-table total',
          re: /\|\s*\*\*Total\*\*\s*\|\s*\*\*(\d{2,4})\*\*\s*\|/g, want: String(total) },
        { file: QUALIFICATION_DOC, label: 'five-layer tuple',
          re: /\bexactly\s+((?:\d{2,4}\/){4}\d{2,4})\b/g, want: tuple },
        { file: QUALIFICATION_DOC, label: 'browser campaign',
          re: /\bloaded\s+all\s+(\d{2,4})\s+production\s+WASM\s+scenarios\b/g, want: String(total) },
        { file: QUALIFICATION_DOC, label: 'campaign artifact row count',
          re: /\bthe\s+(\d{2,4})-row\s+mechanical\s+campaign\b/g, want: String(total) },
        { file: QUALIFICATION_DOC, label: 'primary evidence-file citation count',
          re: /\b(\d{2,4})\s+scenarios\s+cite\s+`engine\/tests\/test_scenario_behavior\.cpp`/g,
          want: String(behavioral) },
        { file: QUALIFICATION_DOC, label: 'remaining evidence-file citation count',
          re: /\bthe\s+remaining\s+(\d{1,3})\s+cite\b/g, want: String(remainder) },
        { file: 'docs/INDEX.md', label: 'INDEX pointer',
          re: /current\s+(\d{2,4})-scenario\s+behavioral\s+closure/g, want: String(total) },
        { file: HEALTH_DOC, label: 'health-audit supersession pointer',
          re: /current\s+(\d{2,4})-scenario\s+behavioral\s+qualification/g, want: String(total) },
    ];
}

test.describe('Catalog counts (derived, never hand-written)', () => {
    test('every definition layer agrees on one absolute count', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const registry = await import(pathToFileURL(modulePath).href);
        const total = registry.SCALE0_SCENARIO_CATALOG.length;

        expect(total, 'catalog must not be empty').toBeGreaterThan(0);
        const layers = {
            'UI registry (module)': registry.SCALE0_SCENARIOS.length,
            'catalog (module)': total,
            'scenario map (module)': registry.SCALE0_SCENARIO_MAP.size,
            'evidence manifest (module)': Object.keys(registry.SCALE0_SCENARIO_VALIDATION).length,
            'UI registry (source text)': extractUiRegistryScenarios().size,
            'JS group files (source text)': extractJsScenarios().size,
            'C++ scenarios.cpp (source text)': extractCppScenarios().size,
        };
        const divergent = Object.entries(layers).filter(([, n]) => n !== total);
        expect(divergent,
            `every definition layer must hold exactly ${total} scenarios.\n` +
            divergent.map(([name, n]) => `  ${name}: ${n}`).join('\n')
        ).toEqual([]);
    });

    test('every documented scenario count matches the registry', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const registry = await import(pathToFileURL(modulePath).href);
        const total = registry.SCALE0_SCENARIO_CATALOG.length;
        const evidence = Object.values(registry.SCALE0_SCENARIO_VALIDATION);
        const behavioral = evidence.filter(
            (e) => (e.test || '').endsWith('test_scenario_behavior.cpp')).length;

        const drift = [];
        for (const claim of documentedCountClaims(
            { total, behavioral, remainder: total - behavioral })) {
            const path = join(WEB_ROOT, claim.file);
            if (!existsSync(path)) {
                drift.push(`${claim.file}: missing (claim "${claim.label}" cannot be checked)`);
                continue;
            }
            const source = readFileSync(path, 'utf8');
            const found = [...source.matchAll(claim.re)].map((m) => m[1]);
            if (found.length === 0) {
                drift.push(`${claim.file}: claim "${claim.label}" no longer present — ` +
                    `update the pattern or the document`);
                continue;
            }
            for (const value of found) {
                if (value !== claim.want) {
                    drift.push(`${claim.file}: "${claim.label}" states ${value}, registry says ${claim.want}`);
                }
            }
        }
        expect(drift, `documented counts have drifted from the registry:\n  ${drift.join('\n  ')}`)
            .toEqual([]);
    });

    test('SCALE0_SCENARIO_RESEARCH_TERMS keys are outside SCALE0_TOGGLES', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'config', 'toggles.js');
        const toggles = await import(pathToFileURL(modulePath).href);
        const uiKeys = new Set(toggles.SCALE0_TOGGLES.map(([k]) => k));
        const research = toggles.SCALE0_SCENARIO_RESEARCH_TERMS || {};
        for (const [scenarioId, terms] of Object.entries(research)) {
            for (const key of Object.keys(terms)) {
                expect(
                    uiKeys.has(key),
                    `${scenarioId} research term '${key}' must stay outside SCALE0_TOGGLES`,
                ).toBe(false);
            }
        }
    });

    test('JS helpers that pin Periodic have SCALE0_SCENARIO_BOUNDARY entries for callers', async () => {
        const helpersPath = join(WEB_ROOT, 'js', 'bridge', 'scenarios', '_helpers.js');
        const helpersSrc = readFileSync(helpersPath, 'utf8');
        const helperRe = /export function (\w+)\([^)]*\)\s*\{/g;
        const helpers = [];
        let hm;
        while ((hm = helperRe.exec(helpersSrc))) helpers.push({ name: hm[1], idx: hm.index });
        const periodicHelpers = [];
        for (let i = 0; i < helpers.length; i++) {
            const body = helpersSrc.slice(helpers[i].idx, i + 1 < helpers.length ? helpers[i + 1].idx : helpersSrc.length);
            if (/setFluxBoundaryMode\?\.\(\s*0\s*\)/.test(body)) periodicHelpers.push(helpers[i].name);
        }
        expect(periodicHelpers.length).toBeGreaterThan(0);

        const groupDir = join(WEB_ROOT, 'js', 'bridge', 'scenarios');
        const files = readdirSync(groupDir).filter((f) => f.endsWith('-scenarios.js'));
        const needsPin = new Set();
        for (const f of files) {
            const text = readFileSync(join(groupDir, f), 'utf8');
            const caseRe = /case\s+['"]([^'"]+)['"]\s*:/g;
            const cases = [];
            let m;
            while ((m = caseRe.exec(text))) cases.push({ id: m[1], idx: m.index });
            for (let i = 0; i < cases.length; i++) {
                const body = text.slice(cases[i].idx, i + 1 < cases.length ? cases[i + 1].idx : text.length);
                const uses =
                    /setFluxBoundaryMode\?\.\(\s*0\s*\)/.test(body) ||
                    periodicHelpers.some((h) => new RegExp(`\\b${h}\\s*\\(`).test(body));
                if (uses) needsPin.add(cases[i].id);
            }
        }

        const togglesPath = join(WEB_ROOT, 'js', 'config', 'toggles.js');
        const toggles = await import(pathToFileURL(togglesPath).href);
        const missing = [...needsPin].filter((id) => !(id in toggles.SCALE0_SCENARIO_BOUNDARY)).sort();
        expect(missing, `Periodic-pinning scenarios missing SCALE0_SCENARIO_BOUNDARY:\n  ${missing.join('\n  ')}`)
            .toEqual([]);
    });

    test('the qualification class table matches the registry categories', async () => {
        const modulePath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const registry = await import(pathToFileURL(modulePath).href);

        const actual = new Map();
        for (const s of registry.SCALE0_SCENARIO_CATALOG) {
            const name = String(s.category || '').replace(/^\d+\.\s*/, '').trim();
            actual.set(name, (actual.get(name) || 0) + 1);
        }

        const source = readFileSync(join(WEB_ROOT, QUALIFICATION_DOC), 'utf8');
        const documented = new Map();
        for (const m of source.matchAll(/^\|\s*([A-Z][^|*]+?)\s*\|\s*(\d{1,4})\s*\|/gm)) {
            documented.set(m[1].trim(), Number(m[2]));
        }
        expect(documented.size, 'the class table should be parseable').toBeGreaterThan(0);

        const drift = [];
        for (const [name, count] of actual) {
            if (!documented.has(name)) drift.push(`registry class "${name}" (${count}) is undocumented`);
            else if (documented.get(name) !== count) {
                drift.push(`class "${name}": doc says ${documented.get(name)}, registry has ${count}`);
            }
        }
        for (const name of documented.keys()) {
            if (!actual.has(name)) drift.push(`documented class "${name}" has no registry members`);
        }
        expect(drift, `class table has drifted:\n  ${drift.join('\n  ')}`).toEqual([]);
    });
});
