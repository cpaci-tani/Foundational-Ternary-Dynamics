// @ts-check
/**
 * Gallery Screenshot Generator
 * 
 * Boots the FTD dashboard, sets up various physical scenarios,
 * collapses/hides all UI overlays, panels, docks, and toolbars,
 * configures the camera to be orientated to the front and zoomed out,
 * and takes high-resolution screenshots of the 3D WebGL simulation canvas.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';
import path from 'path';
import fs from 'fs';

test.describe('FTD Simulation Gallery Generator', () => {
    test('Capture beautiful front-orientated, zoomed-out simulation screenshots', async ({ page }) => {
        // Set test timeout to 90000ms to allow multi-step renders to finish
        test.setTimeout(90000);

        // Set a high-quality, high-resolution viewport size (16:9)
        await page.setViewportSize({ width: 1920, height: 1080 });

        // Navigate to the local server
        await gotoAndReady(page);

        // Pause the live rendering loop so we can control ticks manually and precisely
        await page.evaluate(() => {
            if (window.__ftdCtx) {
                window.__ftdCtx.running = false;
            }
        });

        const targetDir = 'C:\\Users\\cpaci\\.gemini\\antigravity\\brain\\72ac71da-1d05-4f01-a50c-8823bc3a580f';
        if (!fs.existsSync(targetDir)) {
            fs.mkdirSync(targetDir, { recursive: true });
        }

        const scenarios = [
            {
                id: 'light-rainbow',
                ticks: 60,
                name: 'light_rainbow',
                enableOverlays: ['toggle-e-field', 'toggle-b-field']
            },
            {
                id: 's0-seed-emergent-ic3-collision',
                ticks: 80,
                name: 'emergent_ic3_collision',
                enableOverlays: ['toggle-e-field', 'toggle-charge-density', 'toggle-flux-lines']
            },
            {
                id: 's0-seed-emergent-ic1-viz',
                ticks: 50,
                name: 'emergent_ic1_viz',
                enableOverlays: ['toggle-e-field', 'toggle-b-field', 'toggle-charge-density']
            },
            {
                id: 's0-seed-quark-gluon-plasma',
                ticks: 40,
                name: 'quark_gluon_plasma',
                enableOverlays: ['toggle-e-field', 'toggle-b-field', 'toggle-charge-density', 'toggle-flux-lines']
            }
        ];

        // Clean-up and collapse HUD/panels function using the new live button
        const cleanUIAndOrientCamera = async (distMultiplier = 1.35) => {
            // Click the Toggle UI button if it hasn't hidden the UI yet
            await page.evaluate(() => {
                const btn = document.getElementById('btn-toggle-ui');
                if (btn && !document.documentElement.classList.contains('ui-hidden')) {
                    btn.click();
                }
            });

            // Adjust camera zoom to 1.35x
            await page.evaluate((mul) => {
                const b = window._ftdBridge;
                const v = window.__ftdCtx.viewport;
                const N = b.latticeSize || 64;
                const c = N / 2;
                const dist = N * 1.6 * mul; // Zoomed out by multiplier factor
                v.camera.position.set(c, c, c + dist);
                v.controls.target.set(c, c, c);
                v.controls.update();
            }, distMultiplier);

            await page.waitForTimeout(500); // let resize repaint settle
        };

        for (const sc of scenarios) {
            console.log(`\n========================================`);
            console.log(`Loading scenario: ${sc.id}`);
            
            // 1. Load scenario via the select element
            await page.evaluate((scenarioId) => {
                const select = document.getElementById('scenario-select');
                if (select) {
                    select.value = scenarioId;
                    select.dispatchEvent(new Event('change', { bubbles: true }));
                } else {
                    // Fallback to direct registry load if select is not found
                    window._ftdBridge.reset(64);
                    window._ftdBridge.setupScenario(scenarioId);
                }
                // Always pause to prevent background tick racing
                if (window.__ftdCtx) {
                    window.__ftdCtx.running = false;
                }
            }, sc.id);

            // Wait for scenario load to propagate and render initial state
            await page.waitForTimeout(400);

            // Configure visual overlays
            await page.evaluate((overlays) => {
                overlays.forEach(id => {
                    const btn = document.getElementById(id);
                    if (btn && !btn.classList.contains('active')) {
                        btn.click();
                    }
                });
            }, sc.enableOverlays);

            // Hide panels, collapse layout, orient camera
            await cleanUIAndOrientCamera(1.35);

            await page.waitForTimeout(400); // let Three.js repaint
            let screenshotPathT0 = path.join(targetDir, `${sc.name}_t0.png`);
            console.log(`Taking screenshot of tick 0 at: ${screenshotPathT0}`);
            await page.locator('#viewport').screenshot({ path: screenshotPathT0 });

            // 2. Step to target tick count
            console.log(`Stepping ${sc.ticks} ticks...`);
            await page.evaluate((ticks) => {
                const b = window._ftdBridge;
                for (let i = 0; i < ticks; i++) {
                    b.tick();
                }
                // Force sync and updates
                if (window.__ftdCtx && window.__ftdCtx.state) {
                    window.__ftdCtx.state.latticeNeedsUpload = true;
                    window.__ftdCtx.state.fieldNeedsUpdate = true;
                }
            }, sc.ticks);

            // Wait for step updates and repaint
            await page.waitForTimeout(400);

            // Hide panels, collapse layout, orient camera again to clean up any re-renders
            await cleanUIAndOrientCamera(1.35);

            await page.waitForTimeout(400); // let Three.js repaint
            let screenshotPathT = path.join(targetDir, `${sc.name}_t${sc.ticks}.png`);
            console.log(`Taking screenshot of tick ${sc.ticks} at: ${screenshotPathT}`);
            await page.locator('#viewport').screenshot({ path: screenshotPathT });
        }
    });
});
