// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const AA_RATIO = 4.5;

async function renderedContrastFailures(page) {
    return page.evaluate(async () => {
        const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
        const canvas = document.createElement('canvas');
        canvas.width = 1;
        canvas.height = 1;
        const context = canvas.getContext('2d', { willReadFrequently: true });
        const parse = (value) => {
            context.clearRect(0, 0, 1, 1);
            context.fillStyle = 'rgba(0, 0, 0, 0)';
            context.fillStyle = value;
            context.fillRect(0, 0, 1, 1);
            const data = context.getImageData(0, 0, 1, 1).data;
            return [data[0], data[1], data[2], data[3] / 255];
        };
        const over = (foreground, background) => {
            const alpha = foreground[3] + background[3] * (1 - foreground[3]);
            if (!alpha) return [0, 0, 0, 0];
            return [0, 1, 2].map((index) => (
                foreground[index] * foreground[3]
                + background[index] * background[3] * (1 - foreground[3])
            ) / alpha).concat(alpha);
        };
        const backgroundFor = (element) => {
            const layers = [];
            for (let node = element; node; node = node.parentElement) {
                layers.push(parse(getComputedStyle(node).backgroundColor));
            }
            return layers.reverse().reduce((background, layer) => over(layer, background), [255, 255, 255, 1]);
        };
        const luminance = (rgb) => {
            const linear = rgb.slice(0, 3).map((channel) => {
                const value = channel / 255;
                return value <= 0.04045 ? value / 12.92 : ((value + 0.055) / 1.055) ** 2.4;
            });
            return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2];
        };
        const ratio = (a, b) => (Math.max(luminance(a), luminance(b)) + 0.05)
            / (Math.min(luminance(a), luminance(b)) + 0.05);
        const scan = (surface) => [...document.body.querySelectorAll('*')].flatMap((element) => {
            const style = getComputedStyle(element);
            const rect = element.getBoundingClientRect();
            const directText = [...element.childNodes]
                .some((node) => node.nodeType === Node.TEXT_NODE && node.textContent?.trim());
            const textControl = element.matches(
                'button, input:not([type="range"]):not([type="checkbox"]):not([type="radio"]), select, textarea, option',
            );
            if (rect.width < 1 || rect.height < 1
                || style.display === 'none' || style.visibility === 'hidden'
                || Number(style.opacity) < 0.55
                || element.matches(':disabled, [aria-disabled="true"], .katex .vlist-s')
                || element.closest('.toggle-row-disabled, [aria-disabled="true"]')
                || (!directText && !textControl)) return [];

            const background = backgroundFor(element);
            const score = ratio(over(parse(style.color), background), background);
            if (score + 0.005 >= 4.5) return [];
            return [`${surface}: ${element.tagName.toLowerCase()}#${element.id}`
                + `.${typeof element.className === 'string' ? element.className : ''}`
                + ` = ${score.toFixed(2)} (${style.color} on rgb(${background.slice(0, 3).map(Math.round).join(', ')}))`
                + ` ${(element.outerHTML || '').replace(/\s+/g, ' ').slice(0, 120)}`];
        });

        document.getElementById('btn-kb-sidebar-close')?.click();
        document.getElementById('btn-faq-sidebar-close')?.click();
        const failures = scan('dashboard');
        document.getElementById('btn-knowledge-base')?.click();
        await sleep(50);
        failures.push(...scan('knowledge-base'));
        document.getElementById('btn-kb-sidebar-close')?.click();
        document.getElementById('btn-faq')?.click();
        await sleep(50);
        failures.push(...scan('faq'));
        document.getElementById('btn-faq-sidebar-close')?.click();
        return [...new Set(failures)];
    });
}

test('semantic foregrounds meet WCAG AA on every opaque theme surface', async ({ page }) => {
    await page.emulateMedia({ colorScheme: 'dark', reducedMotion: 'reduce' });
    await gotoAndReady(page);

    for (const theme of ['default', 'abyss', 'light', 'nord', 'parchment']) {
        const results = await page.evaluate((themeName) => {
            const root = document.documentElement;
            root.dataset.glass = 'off';
            root.dataset.themeChanging = 'true';
            if (themeName === 'default') root.removeAttribute('data-theme');
            else root.dataset.theme = themeName;

            const parse = (value) => {
                const numbers = value.match(/[\d.]+/g)?.map(Number) ?? [];
                if (numbers.length < 3) throw new Error(`Unsupported color: ${value}`);
                return numbers.slice(0, 3);
            };
            const luminance = (rgb) => {
                const linear = rgb.map((channel) => {
                    const value = channel / 255;
                    return value <= 0.04045
                        ? value / 12.92
                        : ((value + 0.055) / 1.055) ** 2.4;
                });
                return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2];
            };
            const ratio = (foreground, background) => {
                const a = luminance(parse(foreground));
                const b = luminance(parse(background));
                return (Math.max(a, b) + 0.05) / (Math.min(a, b) + 0.05);
            };
            const pairs = [
                ['text-primary/card', '--text-primary', '--bg-card'],
                ['text-secondary/card', '--text-secondary', '--bg-card'],
                ['text-muted/card', '--text-muted', '--bg-card'],
                ['accent/card', '--accent-text', '--bg-card'],
                ['accent/input', '--accent-text', '--bg-input'],
                ['positive/card', '--positive-text', '--bg-card'],
                ['negative/card', '--negative-text', '--bg-card'],
                ['warning/card', '--warning-text', '--bg-card'],
                ['caution/card', '--caution-text', '--bg-card'],
                ['axis-x/input', '--axis-x-text', '--bg-input'],
                ['axis-y/input', '--axis-y-text', '--bg-input'],
                ['axis-z/input', '--axis-z-text', '--bg-input'],
                ['selection/card', '--selection-text', '--bg-card'],
                ['on-accent/fill', '--on-accent', '--accent-dim'],
                ['on-positive/fill', '--on-positive', '--positive'],
                ['on-negative/fill', '--on-negative', '--negative'],
                ['on-warning/fill', '--on-warning', '--warning'],
            ];
            const host = document.createElement('div');
            host.style.cssText = 'position:fixed;left:-10000px;top:0;visibility:hidden';
            document.body.append(host);
            const measured = pairs.map(([label, foregroundToken, backgroundToken]) => {
                const sample = document.createElement('span');
                sample.style.color = `var(${foregroundToken})`;
                sample.style.backgroundColor = `var(${backgroundToken})`;
                host.append(sample);
                const style = getComputedStyle(sample);
                return {
                    label,
                    foreground: style.color,
                    background: style.backgroundColor,
                    ratio: ratio(style.color, style.backgroundColor),
                };
            });
            host.remove();
            return measured;
        }, theme);

        const failures = results
            .filter((result) => result.ratio + 0.005 < AA_RATIO)
            .map((result) => `${theme}/${result.label}: ${result.ratio.toFixed(2)} `
                + `(${result.foreground} on ${result.background})`);
        expect(failures).toEqual([]);

        await page.waitForTimeout(20);
        const renderedFailures = await renderedContrastFailures(page);
        expect(renderedFailures, `${theme} rendered text contrast`).toEqual([]);
    }
});
