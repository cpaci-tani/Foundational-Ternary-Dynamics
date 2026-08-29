// @ts-check
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { gotoAndReady, switchMode } from './_helpers.js';

const WEB_ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const SOURCE_EXTENSIONS = new Set(['.css', '.html', '.js', '.svg']);
const FONT_FLOOR_PX = 16;

function sourceFiles(root) {
    const files = [];
    for (const entry of fs.readdirSync(root, { withFileTypes: true })) {
        if (entry.isDirectory() && ['node_modules', 'test-results'].includes(entry.name)) continue;
        const fullPath = path.join(root, entry.name);
        if (entry.isDirectory()) files.push(...sourceFiles(fullPath));
        else if (SOURCE_EXTENSIONS.has(path.extname(entry.name).toLowerCase())) files.push(fullPath);
    }
    return files;
}

function explicitFontViolations() {
    const violations = [];
    const patterns = [
        /font-size\s*:\s*([^;}\n]+)/gi,
        /font-size\s*=\s*['"]([^'"]+)['"]/gi,
        /fontSize\s*(?:=|:)\s*['"]([^'"]+)['"]/g,
        /\bfont\s*:\s*([^;}\n]+)/gi,
        /\.font\s*=\s*([^;\n]+)/g,
    ];

    for (const file of sourceFiles(WEB_ROOT)) {
        const relativePath = path.relative(WEB_ROOT, file).replaceAll('\\', '/');
        const lines = fs.readFileSync(file, 'utf8').split(/\r?\n/);
        lines.forEach((line, lineIndex) => {
            for (const pattern of patterns) {
                pattern.lastIndex = 0;
                let match;
                while ((match = pattern.exec(line))) {
                    const value = match[1].trim();
                    const pxValues = [...value.matchAll(/(?:^|[^\d.])(\d+(?:\.\d+)?)px/gi)]
                        .map((pxMatch) => Number(pxMatch[1]));
                    const relative = value.match(/^([0-9.]+)(rem|em|%)(?:\s*!important)?$/i);
                    const relativeBelowFloor = relative
                        && (relative[2] === '%' ? Number(relative[1]) < 100 : Number(relative[1]) < 1);
                    const reducingKeyword = /^(xx-small|x-small|small|smaller)$/i.test(value);
                    const unguardedUiScale = /^calc\(/i.test(value) && /var\(--ui-scale/i.test(value);
                    const clampMinimum = value.match(/^clamp\(\s*([0-9.]+)(rem|em|px)/i);
                    const clampBelowFloor = clampMinimum && (clampMinimum[2] === 'px'
                        ? Number(clampMinimum[1]) < FONT_FLOOR_PX
                        : Number(clampMinimum[1]) < 1);
                    if (pxValues.some((fontSize) => fontSize < FONT_FLOOR_PX)
                        || relativeBelowFloor
                        || reducingKeyword
                        || unguardedUiScale
                        || clampBelowFloor) {
                        violations.push(`${relativePath}:${lineIndex + 1} ${match[0].trim()}`);
                    }
                }
            }
        });
    }
    return violations;
}

async function computedFontViolations(page, label) {
    return page.evaluate(({ floor, surface }) => {
        const violations = [];
        const describe = (element) => {
            const id = element.id ? `#${element.id}` : '';
            const classes = [...element.classList].slice(0, 3).map((name) => `.${name}`).join('');
            return `${element.tagName.toLowerCase()}${id}${classes}`;
        };
        const record = (element, pseudo = null) => {
            const style = getComputedStyle(element, pseudo);
            const fontSize = Number.parseFloat(style.fontSize);
            if (Number.isFinite(fontSize) && fontSize + 0.01 < floor) {
                violations.push(`${surface}: ${describe(element)}${pseudo ?? ''} = ${fontSize}px`);
            }
        };

        record(document.documentElement);
        record(document.body);
        for (const element of document.body.querySelectorAll('*')) {
            // KaTeX's vlist-s is a geometry-only strut. Its zero-width marker
            // is not rendered text and its 1px font metric is part of layout.
            if (element.matches('.katex .vlist-s')) continue;
            const hasDirectText = [...element.childNodes]
                .some((node) => node.nodeType === Node.TEXT_NODE && node.textContent?.trim());
            const isTextControl = element.matches('button, input, select, textarea, option');
            const isStandaloneText = element.matches('canvas, svg text');
            if (hasDirectText || isTextControl || isStandaloneText) record(element);
            for (const pseudo of ['::before', '::after']) {
                const content = getComputedStyle(element, pseudo).content;
                if (content && content !== 'none' && content !== 'normal' && content !== '""') {
                    record(element, pseudo);
                }
            }
        }
        return violations;
    }, { floor: FONT_FLOOR_PX, surface: label });
}

test('all source-defined font sizes respect the 16px floor', () => {
    expect(explicitFontViolations()).toEqual([]);

    const tokens = fs.readFileSync(path.join(WEB_ROOT, 'css', 'tokens.css'), 'utf8');
    for (const token of ['xs', 'sm', 'base', 'md', 'lg', 'xl', '2xl', '3xl']) {
        expect(tokens, `--fs-${token} must retain its Compact-mode floor`)
            .toMatch(new RegExp(`--fs-${token}:\\s+max\\(16px,`));
    }
});

test('dashboard elements stay at or above 16px in Compact mode on every public scale', async ({ page }) => {
    await page.setViewportSize({ width: 1440, height: 1000 });
    await gotoAndReady(page);
    await page.evaluate(() => {
        document.documentElement.style.setProperty('--ui-scale-base', '0.85');
    });

    const violations = [];
    for (const mode of ['lattice', 'particles', 'atoms', 'molecules', 'planetary', 'meta']) {
        await switchMode(page, mode);
        await page.waitForTimeout(150);
        violations.push(...await computedFontViolations(page, `dashboard/${mode}`));
    }
    expect(violations).toEqual([]);
});

test('responsive dashboard and standalone pages keep a computed 16px minimum', async ({ page }) => {
    await page.setViewportSize({ width: 375, height: 812 });
    await gotoAndReady(page);
    await page.evaluate(() => {
        document.documentElement.style.setProperty('--ui-scale-base', '0.85');
    });

    const violations = await computedFontViolations(page, 'dashboard/mobile');
    for (const standalonePath of [
        '/fields-atlas.html',
        '/wasm-threads-proof.html',
        '/demos/spacetime-forcing-boundary.html',
    ]) {
        await page.goto(standalonePath, { waitUntil: 'domcontentloaded' });
        violations.push(...await computedFontViolations(page, standalonePath));
    }
    expect(violations).toEqual([]);
});
