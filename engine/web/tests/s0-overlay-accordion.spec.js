// @ts-check
/**
 * Scale-0 Visualization panel — accordion revamp (2026-06-05).
 *
 * Pins the three behaviours added when the always-open 6-column grid became a
 * compact collapsible accordion (see overlays/panel-shell.js):
 *   - all categories collapsed by default, click-to-expand, multiple open;
 *   - an Active strip of removable chips derived from button .active state;
 *   - a filter that hides non-matching overlays + auto-expands matching categories.
 * The toggle wiring itself is covered by toggle-coverage.spec.js and is unchanged.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const expand = (page) => page.evaluate(() => {
    const c = document.querySelector('#viewport-overlay .s0-overlay-collapse');
    if (c && c.getAttribute('aria-expanded') === 'false') c.click();
});

test.describe('Scale-0 Visualization accordion', () => {
    test('collapses by default, compact, with an active-overlay chip', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        const s = await page.evaluate(() => {
            const cols = [...document.querySelectorAll('#viewport-overlay .s0-overlay-col')];
            return {
                hasSearch: !!document.getElementById('s0-overlay-search'),
                hasStrip: !!document.getElementById('s0-overlay-active'),
                width: Math.round(document.getElementById('viewport-overlay').getBoundingClientRect().width),
                categories: cols.length,
                allCollapsed: cols.every((c) => c.classList.contains('is-collapsed')),
                stripHidden: document.getElementById('s0-overlay-active').hidden,
                chips: [...document.querySelectorAll('#s0-overlay-active .s0-overlay-chip-label')].map((e) => e.textContent),
            };
        });
        expect(s.hasSearch && s.hasStrip).toBe(true);
        expect(s.categories).toBe(7);
        expect(s.allCollapsed, 'all categories collapsed by default').toBe(true);
        expect(s.width, 'panel is compact, not the ~700px grid').toBeLessThan(320);
        // flux-volume is on by default → strip shown with a Flux Volume chip.
        expect(s.stripHidden).toBe(false);
        expect(s.chips.some((c) => /Flux Volume/.test(c))).toBe(true);
    });

    test('clicking a category header expands it (multiple stay open)', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        const isOpen = (col) => page.evaluate((c) => !document.querySelector(`#viewport-overlay [data-col="${c}"]`).classList.contains('is-collapsed'), col);
        const clickHead = (col) => page.evaluate((c) => document.querySelector(`#viewport-overlay [data-col="${c}"] .s0-overlay-col-head`).click(), col);

        await clickHead('volume');
        await clickHead('fields');
        expect(await isOpen('volume'), 'volume open').toBe(true);
        expect(await isOpen('fields'), 'fields open too (multi-open)').toBe(true);
        await clickHead('volume');
        expect(await isOpen('volume'), 'volume re-collapses on second click').toBe(false);
        expect(await isOpen('fields'), 'fields stays open').toBe(true);
    });

    test('active strip chip × turns the overlay off', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        expect(await page.evaluate(() => document.getElementById('toggle-flux-volume').classList.contains('active'))).toBe(true);
        await page.evaluate(() => document.querySelector('#s0-overlay-active .s0-overlay-chip-x').click());
        await page.waitForTimeout(150);
        const after = await page.evaluate(() => ({
            active: document.getElementById('toggle-flux-volume').classList.contains('active'),
            chips: document.querySelectorAll('#s0-overlay-active .s0-overlay-chip').length,
            stripHidden: document.getElementById('s0-overlay-active').hidden,
        }));
        expect(after.active, 'flux-volume turned off by the chip ×').toBe(false);
        expect(after.chips, 'chip removed').toBe(0);
        expect(after.stripHidden, 'strip hidden when nothing active').toBe(true);
    });

    test('filter hides non-matches and auto-expands matching categories', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        await page.evaluate(() => { const s = document.getElementById('s0-overlay-search'); s.value = 'helic'; s.dispatchEvent(new Event('input', { bubbles: true })); });
        await page.waitForTimeout(150);
        let r = await page.evaluate(() => ({
            topoShown: !document.querySelector('#viewport-overlay [data-col="topology"]').classList.contains('is-filtered-out'),
            topoOpen: !document.querySelector('#viewport-overlay [data-col="topology"]').classList.contains('is-collapsed'),
            volHidden: document.querySelector('#viewport-overlay [data-col="volume"]').classList.contains('is-filtered-out'),
            helicVisible: !document.getElementById('toggle-helicity').classList.contains('is-filtered-out'),
        }));
        expect(r.topoShown && r.topoOpen, 'Topology shown + auto-expanded').toBe(true);
        expect(r.volHidden, 'Volume hidden (no match)').toBe(true);
        expect(r.helicVisible, 'Helicity visible').toBe(true);

        // Clearing restores: filter classes gone, default-collapsed back.
        await page.evaluate(() => { const s = document.getElementById('s0-overlay-search'); s.value = ''; s.dispatchEvent(new Event('input', { bubbles: true })); });
        await page.waitForTimeout(150);
        r = await page.evaluate(() => ({
            anyFiltered: !!document.querySelector('#viewport-overlay .is-filtered-out'),
            volCollapsed: document.querySelector('#viewport-overlay [data-col="volume"]').classList.contains('is-collapsed'),
        }));
        expect(r.anyFiltered, 'no filter classes after clear').toBe(false);
        expect(r.volCollapsed, 'categories back to collapsed default').toBe(true);
    });
});
