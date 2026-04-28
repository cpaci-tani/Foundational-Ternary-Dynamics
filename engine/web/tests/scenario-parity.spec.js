// @ts-check
/**
 * JS ↔ C++ scenario-parity guard.
 *
 * After the April-2026 WASM-scenario port (engine/src/scenarios.cpp), all 83
 * UI-exposed Scale-0 scenarios exist in BOTH the JS MockBridge
 * (engine/web/js/bridge/scenarios/*.js) and the C++ engine. Without a guard,
 * adding a new scenario to one side and forgetting the other silently
 * regresses the WASM backend.
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

// ── Extractors ──────────────────────────────────────────────────────

function extractJsScenarios() {
    const groupDir = join(WEB_ROOT, 'js', 'bridge', 'scenarios');
    const files = readdirSync(groupDir).filter((f) =>
        f.endsWith('-scenarios.js') && f !== 'index.js'
    );
    const names = new Set();
    for (const f of files) {
        const src = readFileSync(join(groupDir, f), 'utf8');
        // Matches `case 'foo-bar':` (single or double quotes)
        const re = /case\s+['"]([^'"]+)['"]\s*:/g;
        let m;
        while ((m = re.exec(src))) names.add(m[1]);
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
    // Matches `makeScenario('category', 'name', ...)`
    const re = /makeScenario\('[^']+',\s*'([^']+)'/g;
    let m;
    while ((m = re.exec(src))) names.add(m[1]);
    return names;
}

// ── Tests ───────────────────────────────────────────────────────────

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
            if (!js.has(name)) missing.push(name);
        }
        expect(missing,
            `${missing.length} scenarios appear in the UI dropdown but have no JS implementation.\n` +
            `Add the case to a group file in engine/web/js/bridge/scenarios/ or remove from the UI registry.\n` +
            `Missing:\n  - ${missing.join('\n  - ')}`
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
