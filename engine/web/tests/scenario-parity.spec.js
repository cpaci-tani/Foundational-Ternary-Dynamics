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
import { readFileSync, readdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = resolve(__dirname, '..');
const ENGINE_ROOT = resolve(WEB_ROOT, '..');

// Names that live only in the C++ legacy switch (ftd_wasm.cpp) — kept for
// backward compat with older tests/saved dashboards. Not in the JS UI.
const KNOWN_LEGACY_ONLY = new Set([
    // 'empty' is handled by the dispatcher's early-return in JS (index.js) and
    // by the C++ legacy switch's explicit branch; it has no body in scenarios.cpp.
    'empty',
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

    test('ScenarioId JSDoc typedef in toggles.js matches UI registry scenarios exactly', () => {
        const ui = extractUiRegistryScenarios();
        const typedefScenarios = extractTogglesTypedefScenarios();

        const missingInTypedef = [...ui].filter(name => !typedefScenarios.has(name));
        const extraInTypedef = [...typedefScenarios].filter(name => !ui.has(name));

        expect(missingInTypedef,
            `The JSDoc @typedef ScenarioId in toggles.js is missing these scenarios:\n  - ${missingInTypedef.join('\n  - ')}`
        ).toEqual([]);

        expect(extraInTypedef,
            `The JSDoc @typedef ScenarioId in toggles.js contains these unregistered/unknown scenarios:\n  - ${extraInTypedef.join('\n  - ')}`
        ).toEqual([]);
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
        const js = extractJsScenarios();
        const cpp = extractCppScenarios();
        const legacy = extractCppLegacyScenarios();

        console.log('\n=== Scenario inventory ===');
        console.log(`  UI registry (dropdown):     ${ui.size}`);
        console.log(`  JS group files (cases):     ${js.size}`);
        console.log(`  C++ scenarios.cpp:          ${cpp.size}`);
        console.log(`  C++ legacy (ftd_wasm.cpp):  ${legacy.size}`);
        const shared = new Set([...js].filter((n) => cpp.has(n)));
        console.log(`  JS ∩ C++ (shared):          ${shared.size}`);

        // No assertion; this is for visibility only.
        expect(shared.size).toBeGreaterThan(0);
    });
});
