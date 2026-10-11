// @ts-check
/**
 * Zoom-out from the Scale 0 lattice to the scale of the elementary particles.
 *
 * Nothing is simulated beyond the lattice. The view reaches out through
 * twenty powers of ten and marks external reference lengths as rings; this
 * spec checks that the range is reachable (framed views, wheel, travel), that
 * the layer says what it is, and that it disappears back at the lattice.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

async function readLayer(page) {
    return page.evaluate(() => {
        const viewport = window.__ftdCtx.viewport;
        const root = document.querySelector('.scale-landmarks');
        const shown = (el) => !!el && !el.hidden && el.getClientRects().length > 0;
        return {
            distance: viewport.camera.position.distanceTo(viewport.controls.target),
            layer: shown(root),
            labels: root ? [...root.querySelectorAll('.scale-landmark-label')].filter(shown).map((el) => el.textContent) : [],
            caption: shown(root) ? root.querySelector('.scale-landmarks-caption').textContent : '',
            centre: shown(root?.querySelector('.scale-landmarks-centre'))
                ? root.querySelector('.scale-landmarks-centre-label').textContent : null,
            viewLabel: document.querySelector('.live-ruler-view .live-ruler-label')?.textContent || '',
        };
    });
}

test.describe('Scale 0 zoom-out to the particle scale', () => {
    test.beforeEach(async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    });

    test('framed views reach each reference length and label it as external', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const frame = async (id) => {
            await page.evaluate((view) => window.__ftdCtx.viewport.setFramedView(view), id);
            await page.waitForTimeout(250);
            return readLayer(page);
        };

        const quasi = await frame('quasi');
        expect(quasi.layer, 'no reference layer while the quasi-domain is in view').toBe(false);

        const lhc = await frame('lhc');
        expect(lhc.layer).toBe(true);
        expect(lhc.labels).toEqual(['LHC resolution · 1.451×10⁻²⁰ m']);
        expect(lhc.caption).toContain('Nothing is simulated beyond the lattice');
        expect(lhc.caption).toContain('not FTD results');
        expect(lhc.caption).toContain('Next reference out: top quark');
        expect(lhc.centre, 'the lattice is shown as a labelled dot').toMatch(/^lattice · \d\.\d+×10⁻[⁰¹²³⁴⁵⁶⁷⁸⁹]+ m$/);

        const electroweak = await frame('electroweak');
        expect(electroweak.labels.map((text) => text.split(' · ')[0]))
            .toEqual(['top quark', 'Higgs boson', 'Z boson', 'W boson']);

        const nuclear = await frame('nuclear');
        expect(nuclear.labels.map((text) => text.split(' · ')[0]))
            .toEqual(['tau', 'charm quark', 'nuclear scale', 'muon']);

        const electron = await frame('electron');
        expect(electron.labels).toEqual(['electron · 3.862×10⁻¹³ m']);
        expect(electron.caption).not.toContain('Next reference out');
        expect(electron.viewLabel).toMatch(/×10⁻¹[²³] m/);
        expect(electron.distance).toBeGreaterThan(1e22);

        const lattice = await frame('lattice');
        expect(lattice.layer, 'the layer is gone back at the lattice').toBe(false);
        expect(lattice.distance).toBeLessThan(1e3);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('the wheel crosses the empty range in a practical number of notches, and back', async ({ page }) => {
        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('quasi'));
        const box = await page.evaluate(() => {
            const rect = window.__ftdCtx.viewport.renderer.domElement.getBoundingClientRect();
            return { x: rect.x + rect.width / 2, y: rect.y + rect.height / 2 };
        });
        await page.mouse.move(box.x, box.y);
        const start = (await readLayer(page)).distance;
        let notches = 0;
        let state = await readLayer(page);
        while (notches < 200 && !state.labels.some((text) => text.startsWith('electron'))) {
            for (let i = 0; i < 5; i++) await page.mouse.wheel(0, 100);
            notches += 5;
            await page.waitForTimeout(60);
            state = await readLayer(page);
        }
        expect(state.labels.some((text) => text.startsWith('electron')), 'the electron ring was reached').toBe(true);
        expect(Math.log10(state.distance / start)).toBeGreaterThan(18);
        expect(notches, 'about nineteen powers of ten').toBeLessThanOrEqual(130);

        // And back in: the same wheel returns to the lattice and slows there.
        let back = 0;
        while (back < 260 && state.distance > start) {
            for (let i = 0; i < 5; i++) await page.mouse.wheel(0, -100);
            back += 5;
            await page.waitForTimeout(60);
            state = await readLayer(page);
        }
        expect(state.distance).toBeLessThanOrEqual(start);
        expect(state.layer).toBe(false);
        const speed = await page.evaluate(() => window.__ftdCtx.viewport.controls.zoomSpeed);
        expect(speed, 'near the lattice the wheel is fine again').toBeLessThan(1.5);
    });

    test('a zoom-menu stop beyond the lattice is travelled to, and input takes over', async ({ page }) => {
        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('lattice'));
        await page.locator('#play-bar-zoom-btn').click();
        await expect(page.locator('#play-bar-zoom-menu')).toBeVisible();
        await expect(page.locator('#play-bar-zoom-menu .play-bar-zoom-note')).toContainText('nothing simulated');
        await page.locator('[data-framed-view="nuclear"]').click();
        await expect(page.locator('#play-bar-zoom-menu')).toBeHidden();
        expect(await page.evaluate(() => !!window.__ftdCtx.viewport._cameraFlight), 'the camera is travelling').toBe(true);
        await page.waitForFunction(() => !window.__ftdCtx.viewport._cameraFlight, undefined, { timeout: 15_000 });
        const arrived = await readLayer(page);
        expect(arrived.labels.some((text) => text.startsWith('nuclear scale'))).toBe(true);
        expect(arrived.distance).toBeGreaterThan(1e20);

        // A wheel notch during a flight cancels it where it is.
        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('lattice'));
        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('electron', { animate: true }));
        await page.waitForTimeout(900);
        const box = await page.evaluate(() => {
            const rect = window.__ftdCtx.viewport.renderer.domElement.getBoundingClientRect();
            return { x: rect.x + rect.width / 2, y: rect.y + rect.height / 2 };
        });
        await page.mouse.move(box.x, box.y);
        await page.mouse.wheel(0, 100);
        await page.waitForTimeout(200);
        const interrupted = await page.evaluate(() => ({
            flying: !!window.__ftdCtx.viewport._cameraFlight,
            distance: window.__ftdCtx.viewport.camera.position.distanceTo(window.__ftdCtx.viewport.controls.target),
        }));
        expect(interrupted.flying).toBe(false);
        expect(interrupted.distance, 'stopped short of the electron').toBeLessThan(1e22);
    });

    test('the way back is travelled too: Moore from the electron scale, and a short hop', async ({ page }) => {
        const view = () => page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const shown = (el) => !el.hidden && el.getClientRects().length > 0;
            return {
                flying: !!viewport._cameraFlight,
                distance: viewport.camera.position.distanceTo(viewport.controls.target),
                readouts: [...document.querySelectorAll('.moore-readout')].filter(shown).length,
                bracket: document.querySelector('.live-ruler-lattice .live-ruler-label')?.textContent || '',
            };
        });
        const pick = async (id) => {
            await page.locator('#play-bar-zoom-btn').click();
            await page.locator(`#play-bar-zoom-menu [data-framed-view="${id}"]`).click();
        };
        // Where the Moore stop ends up, taken without travelling.
        const moore = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            viewport.setFramedView('moore');
            return viewport.camera.position.distanceTo(viewport.controls.target);
        });

        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('electron'));
        const far = await view();
        expect(far.distance).toBeGreaterThan(1e22);

        // From the electron scale the Moore stop flies in, monotonically.
        await pick('moore');
        const leaving = await view();
        expect(leaving.flying, 'the camera travels back in').toBe(true);
        await page.waitForTimeout(1200);
        const midway = await view();
        expect(midway.flying).toBe(true);
        expect(midway.distance).toBeLessThan(leaving.distance);
        expect(midway.distance).toBeGreaterThan(moore);
        await page.waitForFunction(() => !window.__ftdCtx.viewport._cameraFlight, undefined, { timeout: 15_000 });
        await expect.poll(async () => (await view()).readouts, { timeout: 5_000 }).toBeGreaterThan(0);
        const arrived = await view();
        expect(arrived.distance).toBeCloseTo(moore, 6);
        expect(arrived.bracket).toMatch(/^(voxel|moore) /);

        // A short hop travels as well, and is over quickly.
        await pick('lattice');
        expect((await view()).flying).toBe(true);
        const started = Date.now();
        await page.waitForFunction(() => !window.__ftdCtx.viewport._cameraFlight, undefined, { timeout: 5_000 });
        expect(Date.now() - started).toBeLessThan(2_000);
        const lattice = await view();
        expect(lattice.distance).toBeGreaterThan(moore * 5);
        expect(lattice.readouts).toBe(0);

        // The view arrives looking at the lattice centre along the canned direction.
        const pose = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const centre = viewport._latticeCenterWorld();
            const direction = viewport.camera.position.clone().sub(viewport.controls.target).normalize();
            const canned = direction.clone().set(1, 0.62, 1.05).normalize();
            return { offTarget: viewport.controls.target.distanceTo(centre), offDirection: direction.distanceTo(canned) };
        });
        expect(pose.offTarget).toBeLessThan(1e-6);
        expect(pose.offDirection).toBeLessThan(1e-6);
    });

    test('other scales keep their zoom limit', async ({ page }) => {
        const limits = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const lattice = viewport._zoomOutLimit();
            const previous = viewport._engineMode;
            viewport._engineMode = 'particles';
            const particles = viewport._zoomOutLimit();
            const centre = viewport._latticeCentreOnScreen(1000, 600);
            viewport._engineMode = previous;
            return { lattice, particles, centre };
        });
        expect(limits.lattice).toBeGreaterThan(1e22);
        expect(limits.particles).toBe(1e8);
        expect(limits.centre, 'no lattice landmarks outside the lattice view').toBeNull();
    });
});
