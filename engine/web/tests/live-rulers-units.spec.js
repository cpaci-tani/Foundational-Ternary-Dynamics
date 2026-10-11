// @ts-check
/**
 * The view ruler reads each scale in that scale's own engine unit.
 *
 * For every scale the visible width is worked out here from the camera, in
 * world units, and turned into a length with the unit the engine's own
 * readouts use (units.js, the Scale 4 constants, the Scale 5 diagnostics).
 * The ruler label has to say the same thing. Scales whose engine declares no
 * metre length must read in that engine's unit and show no metres.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, bootDashboard, realErrors, switchMode } from './_helpers.js';

const SUPERSCRIPT = '⁰¹²³⁴⁵⁶⁷⁸⁹';

/** 'view · 3.704×10⁻⁹ m' or 'view · 46.24 lu' → { value, unit }. */
function parseReading(text) {
    const body = text.includes(' · ') ? text.slice(text.indexOf(' · ') + 3) : text;
    const match = body.match(/^(-?\d+(?:\.\d+)?)(?:×10(⁻?[⁰¹²³⁴⁵⁶⁷⁸⁹]+))? (\S+)$/);
    if (!match) return { value: NaN, unit: '', text: body };
    let exponent = 0;
    if (match[2]) {
        const digits = [...match[2].replace('⁻', '')].map((ch) => SUPERSCRIPT.indexOf(ch)).join('');
        exponent = (match[2].startsWith('⁻') ? -1 : 1) * Number(digits);
    }
    return { value: Number(match[1]) * 10 ** exponent, unit: match[3], text: body };
}

/** Ruler DOM and the camera it was drawn from, read in one synchronous step. */
async function readRuler(page) {
    return page.evaluate(() => {
        const viewport = window.__ftdCtx.viewport;
        viewport._updateLiveRulers(true);
        const view = document.querySelector('.live-ruler-view');
        const bracket = document.querySelector('.live-ruler-lattice');
        const rect = viewport.container.getBoundingClientRect();
        const distance = viewport.camera.position.distanceTo(viewport.controls.target);
        const worldWidth = 2 * Math.tan((viewport.camera.fov * Math.PI) / 360) * distance * (rect.width / rect.height);
        const open = rect.width - parseFloat(view.style.left) - parseFloat(view.style.right);
        // The tooltip layer shows data-ui-tooltip; a native title is moved there.
        const tooltip = (el) => el.dataset.uiTooltip || el.title;
        return {
            label: view.querySelector('.live-ruler-label').textContent,
            title: tooltip(view),
            ticks: [...view.querySelectorAll('.live-ruler-tick span')].map((el) => el.textContent),
            bracketShown: !bracket.hidden,
            bracketLabel: bracket.querySelector('.live-ruler-label').textContent,
            bracketTitle: tooltip(bracket),
            unitsAcross: (worldWidth / (viewport.scene.scale.x || 1)) * (open / rect.width),
        };
    });
}

/** The length unit each engine's own readouts use, from the engine's modules. */
async function engineUnits(page) {
    return page.evaluate(async () => {
        const units = await import('/js/units.js');
        const solar = await import('/js/config/solar-system-physics.js');
        const cosmic = await import('/js/ui/panels/diagnostics-panel/descriptors/scale5.js');
        const atom = units.formatLength(1, 2);
        const boxRow = cosmic.sections.flatMap((section) => section.rows).find((row) => row.id === 'box-size');
        return {
            voxelMetres: units.FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M,
            particleUnit: units.formatLength(1, 1).unit,
            atomUnit: atom.unit,
            atomMetres: atom.value * 1e-10,
            auMetres: solar.AU_METERS,
            cosmicUnit: boxRow.unit,
        };
    });
}

const SCALES = [
    { mode: 'lattice', metres: (engine) => engine.voxelMetres, title: /One voxel is the electron-primary Planck length/ },
    { mode: 'particles', unit: (engine) => engine.particleUnit, title: /lattice units \(lu\).*no metre/ },
    { mode: 'atoms', metres: (engine) => engine.atomMetres, title: /Bohr radius.*input/ },
    { mode: 'molecules', metres: (engine) => engine.atomMetres, title: /Bohr radius.*input/ },
    { mode: 'planetary', metres: (engine) => engine.auMetres, title: /astronomical unit/ },
    { mode: 'cosmic', unit: (engine) => engine.cosmicUnit, title: /lattice units \(lu\).*no metre/ },
];

test.describe('view ruler units', () => {
    test('each scale reads in the length unit of its own engine', async ({ page }) => {
        test.setTimeout(240_000);
        const consoleErrors = attachConsoleWatcher(page);
        await bootDashboard(page, { engine: 'wasm' });
        const engine = await engineUnits(page);
        expect(engine.particleUnit).toBe('lu');
        expect(engine.atomUnit).toBe('Å');
        expect(engine.cosmicUnit).toBe('lu');

        for (const scale of SCALES) {
            await test.step(scale.mode, async () => {
                if (scale.mode !== 'lattice') await switchMode(page, scale.mode);
                await page.waitForTimeout(600);
                const ruler = await readRuler(page);
                const reading = parseReading(ruler.label);
                expect.soft(ruler.unitsAcross, `${scale.mode}: the camera shows a finite width`).toBeGreaterThan(0);
                expect.soft(ruler.title, `${scale.mode}: tooltip`).toMatch(scale.title);

                if (scale.metres) {
                    const expected = ruler.unitsAcross * scale.metres(engine);
                    expect.soft(reading.unit, `${scale.mode}: "${ruler.label}" is in metres`).toBe('m');
                    expect.soft(Math.abs(reading.value / expected - 1),
                        `${scale.mode}: "${ruler.label}" against ${expected.toExponential(4)} m`).toBeLessThan(2e-3);
                } else {
                    const unit = scale.unit(engine);
                    expect.soft(reading.unit, `${scale.mode}: "${ruler.label}" is in the engine unit`).toBe(unit);
                    expect.soft(Math.abs(reading.value / ruler.unitsAcross - 1),
                        `${scale.mode}: "${ruler.label}" against ${ruler.unitsAcross.toPrecision(5)} ${unit}`).toBeLessThan(2e-3);
                    expect.soft(ruler.ticks.every((text) => !/ m$/.test(text)),
                        `${scale.mode}: no tick is captioned in metres`).toBe(true);
                    expect.soft(ruler.bracketLabel, `${scale.mode}: bracket`).not.toMatch(/ m$/);
                }
                if (scale.mode !== 'lattice') {
                    expect.soft(ruler.title, `${scale.mode}: tooltip does not describe a voxel`).not.toMatch(/voxel|Planck/);
                    expect.soft(ruler.bracketTitle, `${scale.mode}: bracket tooltip`).not.toMatch(/voxel|Planck/);
                    expect.soft(ruler.bracketLabel, `${scale.mode}: bracket is not named a voxel`).not.toMatch(/^voxel/);
                }
            });
        }
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
