// @ts-check
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const WEB_ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const CSS_ROOT = path.join(WEB_ROOT, 'css');
const UI_ROOT = path.join(CSS_ROOT, 'ui');

function cssFiles(root) {
    return fs.readdirSync(root, { withFileTypes: true }).flatMap((entry) => {
        const fullPath = path.join(root, entry.name);
        if (entry.isDirectory()) return cssFiles(fullPath);
        return entry.name.endsWith('.css') ? [fullPath] : [];
    });
}

function relative(file) {
    return path.relative(WEB_ROOT, file).replaceAll('\\', '/');
}

function matchingLines(file, expression) {
    return fs.readFileSync(file, 'utf8')
        .split(/\r?\n/)
        .flatMap((line, index) => expression.test(line)
            ? [`${relative(file)}:${index + 1} ${line.trim()}`]
            : []);
}

test('CSS architecture guardrails reject known cascade and animation hazards', () => {
    const violations = [];
    for (const file of cssFiles(UI_ROOT)) {
        violations.push(...matchingLines(file, /transition\s*:\s*all\b/i));
        violations.push(...matchingLines(file, /z-index\s*:(?!\s*(?:var\(--z-|calc\(var\(--z-))/i));
        violations.push(...matchingLines(file, /font-family\s*:\s*var\(--font-sans\s*\)\s*;/i));
        violations.push(...matchingLines(file, /var\(--(?:bg|text|surface-[23])\s*[,)]/i));
    }
    expect(violations).toEqual([]);
});

test('Scale 0 visualization CSS has one visual owner', () => {
    const withoutComments = (css) => css.replace(/\/\*[\s\S]*?\*\//g, '');
    const shared = withoutComments(fs.readFileSync(
        path.join(UI_ROOT, 'components', 'viewport-overlays.css'),
        'utf8',
    ));
    const scale0 = withoutComments(fs.readFileSync(
        path.join(UI_ROOT, 'scales', 'scale0', 'overlay-panel.css'),
        'utf8',
    ));

    for (const selector of [
        '.s0-overlay-col',
        '.s0-overlay-chip',
        '.s0-overlay-group',
        '.s0-overlay-search',
        '.s0-overlay-active',
        '.s0-sheet-height-row',
    ]) {
        expect(shared, `${selector} belongs only to scale0/overlay-panel.css`)
            .not.toContain(selector);
        expect(scale0, `${selector} is present in the Scale 0 owner`)
            .toContain(selector);
    }
});

test('the stylesheet manifest preserves dependency order and omits empty stubs', () => {
    const index = fs.readFileSync(path.join(WEB_ROOT, 'index.html'), 'utf8');
    const buttonIndex = index.indexOf('css/ui/primitives/button.css');
    const toggleIndex = index.indexOf('css/ui/primitives/toggle.css');
    const scale0Index = index.indexOf('css/ui/scales/scale0/overlay-panel.css');

    expect(buttonIndex).toBeGreaterThanOrEqual(0);
    expect(toggleIndex).toBeGreaterThan(buttonIndex);
    expect(scale0Index).toBeGreaterThan(toggleIndex);
    expect(index).not.toContain('css/ui/primitives/modal.css');
});

test('every named theme implements the semantic foreground contract', () => {
    const semanticTokens = [
        'accent-text',
        'on-accent',
        'positive-text',
        'negative-text',
        'warning-text',
        'caution-text',
        'on-positive',
        'on-negative',
        'on-warning',
        'axis-x-text',
        'axis-y-text',
        'axis-z-text',
        'selection-text',
    ];
    for (const name of ['abyss', 'light', 'nord', 'parchment']) {
        const css = fs.readFileSync(path.join(CSS_ROOT, 'themes', `${name}.css`), 'utf8');
        for (const token of semanticTokens) {
            expect(css, `${name}.css defines --${token}`)
                .toMatch(new RegExp(`--${token}\\s*:`));
        }
    }
});
