// @ts-check
/**
 * Registry-closure ↔ S0_SEED-metadata guard.
 *
 * Root cause this closes (boson-sector red-team audit, 2026-08-21):
 * `scenario-parity.spec.js` compares S0_SEED_SCENARIO_METADATA against the
 * registry for *key-orphanhood only* — it never compares the metadata's
 * epistemic content against the registry's `epistemicStatus`. So a user-facing
 * card in engine/web/js/config/scenarios.js could re-assert a physical identity
 * that the canonical registry (engine/web/js/scales/scale0/scenario-registry.js)
 * and the validation ledger (scenario-validation.js) have already marked
 * `[CLOSED NEGATIVE]`, and nothing failed.
 *
 * The registry is the source of truth (project rule: LEDGER/registry win over
 * prose). This guard enforces the invariant that the user-facing card must
 * carry the same closure the registry carries:
 *
 *   For every catalog scenario whose registry `epistemicStatus` contains
 *   "[CLOSED NEGATIVE]", if a *live* S0_SEED_SCENARIO_METADATA entry exists for
 *   it, that entry's serialized text (title + desc + every epistemic
 *   [field, tag, note]) must itself contain "[CLOSED NEGATIVE]".
 *
 * A card that omits the closure presents the closed identity as live — which is
 * exactly the defect the audit found. Requiring the closure marker is the
 * minimal, robust, registry-driven rule; it is intentionally NOT a hardcoded id
 * list, so newly-closed registry scenarios are covered automatically.
 *
 * Note: commented-out metadata (the audit-history reference block) is not part
 * of the live S0_SEED_SCENARIO_METADATA object, so it is correctly ignored — a
 * reference-only entry cannot reach a user-facing surface (bindings.js /
 * knowledge-base data.js both look up the live object by id).
 *
 * Why node-style: no WASM/browser load — we import the two modules and compare
 * their data. Fast.
 */
import { test, expect } from '@playwright/test';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = resolve(__dirname, '..');

const CLOSED_NEGATIVE = '[CLOSED NEGATIVE]';

/**
 * Flatten one S0_SEED metadata entry to a single searchable string:
 * its title, its desc, and every cell of every epistemic [field, tag, note].
 */
function serializeMetadata(meta) {
    const parts = [String(meta.title || ''), String(meta.desc || '')];
    for (const row of meta.epistemic || []) {
        for (const cell of row) parts.push(String(cell));
    }
    return parts.join(' • ');
}

test.describe('Registry closure ↔ S0_SEED metadata', () => {
    test('every live card for a [CLOSED NEGATIVE] registry scenario carries the closure', async () => {
        const registryPath = join(WEB_ROOT, 'js', 'scales', 'scale0', 'scenario-registry.js');
        const configPath = join(WEB_ROOT, 'js', 'config', 'scenarios.js');
        const registry = await import(pathToFileURL(registryPath).href);
        const config = await import(pathToFileURL(configPath).href);

        const catalog = registry.SCALE0_SCENARIO_CATALOG;
        const metadata = config.S0_SEED_SCENARIO_METADATA;

        // Wiring / import-rot guards: if either side comes back empty the whole
        // test would vacuously pass, hiding a real regression. Fail loudly.
        expect(Array.isArray(catalog) && catalog.length > 0,
            'registry SCALE0_SCENARIO_CATALOG failed to import or is empty').toBe(true);
        expect(metadata && Object.keys(metadata).length > 0,
            'config S0_SEED_SCENARIO_METADATA failed to import or is empty').toBe(true);

        // The registry is the source of truth for what has been closed.
        const closedNegative = catalog.filter(
            (s) => String(s.epistemicStatus || '').includes(CLOSED_NEGATIVE));
        expect(closedNegative.length,
            `no registry scenarios carry "${CLOSED_NEGATIVE}" — the tag vocabulary or the ` +
            `registry export changed; update this guard`).toBeGreaterThan(0);

        const violations = [];
        for (const scenario of closedNegative) {
            const meta = metadata[scenario.id];
            if (!meta) continue; // no user-facing card for this id → nothing to contradict
            if (!serializeMetadata(meta).includes(CLOSED_NEGATIVE)) {
                violations.push(
                    `  - ${scenario.id}\n` +
                    `      registry epistemicStatus: ${scenario.epistemicStatus}\n` +
                    `      card title:               "${meta.title}"`);
            }
        }

        expect(violations,
            `${violations.length} live S0_SEED_SCENARIO_METADATA card(s) describe a scenario the ` +
            `registry has marked ${CLOSED_NEGATIVE}, yet the card never states the closure — a user ` +
            `reads the closed identity as live.\nAdd a ${CLOSED_NEGATIVE} epistemic row (and align the ` +
            `prose) in engine/web/js/config/scenarios.js so the card matches the registry tag in ` +
            `engine/web/js/scales/scale0/scenario-registry.js.\n` +
            violations.join('\n')
        ).toEqual([]);
    });
});
