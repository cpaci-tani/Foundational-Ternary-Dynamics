// @ts-check
import { existsSync, readFileSync } from 'node:fs';
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

const WEB_ROOT = new URL('../', import.meta.url);
const RETIRED_TOKENS = [
    'floating-symmetry-panel',
    'sym-u1',
    'sym-su2',
    'sym-su3',
    'setSymmetryHighlights',
    '_symHighlights',
    'mountSymmetryPanel',
];

test('retired pending symmetry surface has no live UI, runtime, renderer, or CSS owner', async ({ page }) => {
    const retiredModule = new URL('js/scales/scale0/ui/overlays/symmetry-panel.js', WEB_ROOT);
    expect(existsSync(retiredModule), 'pending symmetry module is retired').toBe(false);

    const liveFiles = [
        'js/app.js',
        'js/inspector.js',
        'js/inspector/app-runtime.js',
        'js/inspector/scales/lattice.js',
        'js/scales/scale0/controller.js',
        'js/viewport.js',
        'js/viewport/scene-core.js',
        'css/ui/scales/scale0/toolbar.css',
    ];
    const stale = [];
    for (const relativePath of liveFiles) {
        const source = readFileSync(new URL(relativePath, WEB_ROOT), 'utf8');
        for (const token of RETIRED_TOKENS) {
            if (source.includes(token)) stale.push(`${relativePath}: ${token}`);
        }
    }
    expect(stale, `retired symmetry references remain:\n  ${stale.join('\n  ')}`).toEqual([]);

    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page);
    await expect(page.locator('#floating-symmetry-panel')).toHaveCount(0);
    expect(realErrors(consoleErrors)).toEqual([]);
});
