// @ts-check
import { test, expect } from '@playwright/test';

/**
 * Scale-switching smoke suite for the FTD web dashboard.
 *
 * What this suite catches:
 *   - Import graph breakage (missing modules → 404s during module load)
 *   - Console errors during page load and during scale switches
 *   - Mode-specific controllers failing to initialize (no bridge, null ctx)
 *   - Scale 5 cosmic physics cadence regressions (Phase B.1)
 *   - Scale 11 consciousness listener leak regressions (Phase B.2)
 *
 * What it does NOT do:
 *   - Visual regression (GPU nondeterminism makes screenshot diffing unreliable)
 *   - Cross-browser (Chromium only; we use Three.js + importmaps)
 *   - Physics correctness (covered by C++ CTests and Python pytest)
 */

/** Helper: set the engine-mode select and fire its change handler. */
async function switchMode(page, mode) {
    await page.evaluate((m) => {
        const sel = document.getElementById('engine-mode');
        if (!sel) throw new Error('engine-mode select not found');
        sel.value = m;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, mode);
}

/** Helper: collect console errors into an array for later assertion. */
function attachConsoleWatcher(page) {
    const errors = [];
    page.on('console', (msg) => {
        if (msg.type() === 'error') errors.push(msg.text());
    });
    page.on('pageerror', (err) => {
        errors.push(`pageerror: ${err.message}`);
    });
    return errors;
}

/** Helper: collect failed network requests. */
function attachNetworkWatcher(page) {
    const failures = [];
    page.on('requestfailed', (req) => {
        failures.push(`${req.method()} ${req.url()} — ${req.failure()?.errorText}`);
    });
    page.on('response', (resp) => {
        if (resp.status() >= 400) failures.push(`${resp.status()} ${resp.url()}`);
    });
    return failures;
}

const KNOWN_NOISE = [
    // WebAssembly abort from ws-bridge exponential backoff on ws://localhost:9100;
    // optional native GPU path, absence is expected in a browser-only test.
    /^Aborted\(\)$/,
    // ws-bridge reconnect logs — benign
    /\[ws-bridge\]/,
    // Chrome font preload warning
    /was preloaded using link preload/,
];

function isNoise(msg) {
    return KNOWN_NOISE.some((rx) => rx.test(msg));
}

test.beforeEach(async ({ page }) => {
    // Grant a bit of extra time for initial WASM compile + module graph load
    page.setDefaultTimeout(20_000);
});

test('index.html loads, bridge initializes, zero 404s', async ({ page }) => {
    const errors = attachConsoleWatcher(page);
    const failures = attachNetworkWatcher(page);

    await page.goto('/index.html');

    // Wait for the main app to wire up its debug bridge accessor
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000, message: 'window._ftdBridge never became non-null' })
        .toBe(true);

    // Give WASM + scale controllers a moment to settle
    await page.waitForTimeout(1500);

    const relevantErrors = errors.filter((e) => !isNoise(e));
    expect(relevantErrors, `Console errors: ${relevantErrors.join('\n')}`).toHaveLength(0);
    expect(failures, `Failed requests: ${failures.join('\n')}`).toHaveLength(0);
});

const MODES = ['lattice', 'particles', 'atoms', 'molecules', 'planetary', 'cosmic', 'meta', 'consciousness'];

for (const mode of MODES) {
    test(`scale switch: ${mode} loads without errors`, async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        const failures = attachNetworkWatcher(page);

        await page.goto('/index.html');
        await expect.poll(() => page.evaluate(() => typeof window._ftdBridge !== 'undefined'),
            { timeout: 15_000 }).toBe(true);
        await page.waitForTimeout(800);

        await switchMode(page, mode);
        await page.waitForTimeout(1500);

        // Bridge should still be alive after the mode switch
        const bridgeAlive = await page.evaluate(() => !!window._ftdBridge);
        expect(bridgeAlive, `bridge lost after switching to ${mode}`).toBe(true);

        const relevantErrors = errors.filter((e) => !isNoise(e));
        expect(relevantErrors, `Errors switching to ${mode}:\n${relevantErrors.join('\n')}`).toHaveLength(0);

        const bad = failures.filter((f) => !/favicon|\/ws/.test(f));
        expect(bad, `Failed requests switching to ${mode}:\n${bad.join('\n')}`).toHaveLength(0);
    });
}

test('Scale 11 consciousness: listener count stable across 5 re-entries (Phase B.2)', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);
    // Warm up: enter/leave consciousness once BEFORE installing the patch so
    // one-time first-load initialization lands on the untracked baseline.
    await page.evaluate(async () => {
        const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
        const eng = document.getElementById('engine-mode');
        eng.value = 'consciousness';
        eng.dispatchEvent(new Event('change', { bubbles: true }));
        await sleep(800);
        eng.value = 'lattice';
        eng.dispatchEvent(new Event('change', { bubbles: true }));
        await sleep(400);
    });

    const samples = await page.evaluate(async () => {
        let adds = 0, rems = 0;
        const origAdd = EventTarget.prototype.addEventListener;
        const origRem = EventTarget.prototype.removeEventListener;
        EventTarget.prototype.addEventListener = function (...a) { adds++; return origAdd.apply(this, a); };
        EventTarget.prototype.removeEventListener = function (...a) { rems++; return origRem.apply(this, a); };

        const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
        const eng = document.getElementById('engine-mode');

        const cycleAdds = [];
        const cycleRems = [];
        for (let i = 0; i < 5; i++) {
            eng.value = 'consciousness';
            eng.dispatchEvent(new Event('change', { bubbles: true }));
            await sleep(600);
            eng.value = 'lattice';
            eng.dispatchEvent(new Event('change', { bubbles: true }));
            await sleep(400);
            cycleAdds.push(adds);
            cycleRems.push(rems);
        }

        EventTarget.prototype.addEventListener = origAdd;
        EventTarget.prototype.removeEventListener = origRem;
        return { cycleAdds, cycleRems };
    });

    // After warm-up, consciousness re-entries must be pure no-ops for the
    // event-listener count. Phase B.2 keeps _csPedagogy alive, so wireSubTabs
    // and ConsciousnessPedagogy() both run zero times on re-entry.
    // Net = adds - rems should be 0 for every cycle (some internal churn
    // from other scales may still add+remove symmetrically).
    const nets = samples.cycleAdds.map((a, i) => a - samples.cycleRems[i]);
    const firstNet = nets[0];
    for (let i = 1; i < nets.length; i++) {
        expect(nets[i],
            `cycle ${i}: net listener count drifted from ${firstNet} to ${nets[i]} ` +
            `(adds=${JSON.stringify(samples.cycleAdds)}, rems=${JSON.stringify(samples.cycleRems)})`)
            .toBe(firstNet);
    }
});

test('Scale 5 cosmic: no _cosmicInterval leak after Phase B.1', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);

    await switchMode(page, 'cosmic');
    await page.waitForTimeout(1500);

    // After Phase B.1, cosmic physics runs inside animateCosmic (rAF-driven),
    // not via a module-level setInterval. window._cosmicInterval must NEVER
    // be set.
    const hasInterval = await page.evaluate(() => !!window._cosmicInterval);
    expect(hasInterval, 'window._cosmicInterval was set — Phase B.1 regression').toBe(false);

    // Leaving cosmic should still be clean.
    await switchMode(page, 'lattice');
    await page.waitForTimeout(500);
    const stillNoInterval = await page.evaluate(() => !!window._cosmicInterval);
    expect(stillNoInterval).toBe(false);
});

test('Constants: K_B matches 0.511 and is a named export', async ({ page }) => {
    await page.goto('/index.html');
    const k = await page.evaluate(async () => {
        const mod = await import('./js/constants.js');
        return { K_B: mod.K_B, hasAlpha: typeof mod.ALPHA === 'number', hasGStar: typeof mod.G_STAR === 'number' };
    });
    expect(k.K_B).toBe(0.511);
    expect(k.hasAlpha).toBe(true);
    expect(k.hasGStar).toBe(true);
});

test('Scale 0 module contract and scenario registry are wired', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const controller = await import('./js/scales/scale0/controller.js');
        const registry = await import('./js/scales/scale0/scenario-registry.js');
        const requiredFns = ['bindUI', 'enter', 'exit', 'loadScenario', 'animate', 'step', 'reset', 'resize'];
        const moduleShapeOk = requiredFns.every((name) => typeof controller[name] === 'function');
        const validation = registry.validateScale0ScenarioRegistry();
        const select = document.getElementById('scenario-select');
        return {
            moduleShapeOk,
            validation,
            optionCount: select?.options.length || 0,
            scenarioCount: registry.SCALE0_SCENARIOS.length,
            firstScenario: registry.SCALE0_SCENARIOS[0]?.id,
            firstOption: select?.options[0]?.value || null,
        };
    });

    expect(result.moduleShapeOk).toBe(true);
    expect(result.validation.ok, `Registry errors: ${result.validation.errors.join(', ')}`).toBe(true);
    expect(result.optionCount).toBe(result.scenarioCount);
    expect(result.firstOption).toBe(result.firstScenario);
});

test('UI shell initializes mount roots and responsive layout state', async ({ page }) => {
    await page.setViewportSize({ width: 390, height: 844 });
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const shell = await page.evaluate(() => {
        const app = document.getElementById('app');
        const mounts = Array.from(document.querySelectorAll('#app-shell-mounts [data-shell-mount]')).map((el) => el.id);
        return {
            shellReady: app?.dataset.shellReady || null,
            layoutMode: app?.dataset.layoutMode || null,
            orientation: app?.dataset.orientation || null,
            regions: {
                toolbar: document.getElementById('toolbar')?.dataset.shellRegion || null,
                viewport: document.getElementById('viewport')?.dataset.shellRegion || null,
                tabs: document.getElementById('tab-bar')?.dataset.shellRegion || null,
                panels: document.getElementById('panel-area')?.dataset.shellRegion || null,
            },
            ui: {
                mobileTabSelect: !!document.getElementById('tab-select-mobile'),
                assistantButton: !!document.getElementById('btn-ftd-assistant'),
                assistantSidebar: !!document.getElementById('assistant-sidebar'),
                knowledgeBaseButton: !!document.getElementById('btn-knowledge-base'),
                faqButton: !!document.getElementById('btn-faq'),
                knowledgeBaseLibrary: !!document.getElementById('kb-sidebar'),
                scale0ScenarioSelect: !!document.getElementById('scenario-select'),
                scale0LatticeSize: !!document.getElementById('lattice-size'),
                scale1ScenarioSelect: !!document.getElementById('pe-scenario-select'),
                scale2ScenarioSelect: !!document.getElementById('ae-scenario-select'),
                scale3ScenarioSelect: !!document.getElementById('mol-scenario-select'),
                scale23VisualControls: !!document.getElementById('ae-visual-controls'),
                scale23ForceControls: !!document.getElementById('ae-force-controls'),
                scale4ScenarioSelect: !!document.getElementById('planetary-scenario-select'),
                zooPanel: !!document.getElementById('panel-zoo'),
                zooSearch: !!document.getElementById('zoo-search'),
                zooFilter: !!document.getElementById('zoo-filter'),
                zooTableContainer: !!document.getElementById('zoo-table-container'),
                planetaryPanel: !!document.getElementById('panel-planetary'),
                planetaryLayerList: !!document.getElementById('planetary-layer-list'),
                physicsPanel: !!document.getElementById('panel-physics'),
                physicsEnergyLevels: !!document.getElementById('physics-energy-levels'),
                physicsZSlider: !!document.getElementById('physics-z-slider'),
                inspectorPanel: !!document.getElementById('panel-inspector'),
                inspectorModeLabel: !!document.getElementById('insp-mode-label'),
                inspectorSummary: !!document.getElementById('insp-selection-summary'),
                raycastThreshold: !!document.getElementById('raycast-threshold'),
                peInspectorEmpty: !!document.getElementById('pe-inspector-empty'),
                aeInspectorContent: !!document.getElementById('ae-inspector-content'),
                hierarchyPanel: !!document.getElementById('panel-hierarchy'),
                hierarchyTower: !!document.getElementById('hierarchy-tower'),
                scale5ScenarioSelect: !!document.getElementById('cosmic-scenario-select'),
                scale5CameraSelect: !!document.getElementById('cosmic-camera-select'),
                scale5Telemetry: !!document.getElementById('cosmic-telemetry'),
                cosmicInfoPanel: !!document.getElementById('panel-cosmic-info'),
                cosmicPanelDiagnostics: !!document.getElementById('cosmic-panel-diagnostics'),
                scale11ScenarioSelect: !!document.getElementById('cs-scenario-select'),
                scale11FigureSelect: !!document.getElementById('cs-figure-select'),
                scale12MetaControls: !!document.getElementById('meta-controls'),
                metaInfoPanel: !!document.getElementById('meta-info-panel'),
                metaInspectPanel: !!document.getElementById('meta-inspect-panel'),
                verificationLabPanel: !!document.getElementById('panel-verification-lab'),
                verifyHeaderSlot: !!document.getElementById('verify-header-slot'),
                verifyTiersSlot: !!document.getElementById('verify-tiers-slot'),
                verifyExportBtn: !!document.getElementById('verify-export-btn'),
                onticPanelRemoved: !document.getElementById('panel-ontic'),
            },
            mounts,
        };
    });

    expect(shell.shellReady).toBe('true');
    expect(shell.layoutMode).toBe('compact-sm');
    expect(shell.orientation).toBe('portrait');
    expect(shell.regions.toolbar).toBe('toolbar');
    expect(shell.regions.viewport).toBe('viewport');
    expect(shell.regions.tabs).toBe('tabs');
    expect(shell.regions.panels).toBe('panels');
    expect(shell.ui.mobileTabSelect).toBe(true);
    expect(shell.ui.assistantButton).toBe(true);
    expect(shell.ui.assistantSidebar).toBe(true);
    expect(shell.ui.knowledgeBaseButton).toBe(true);
    expect(shell.ui.faqButton).toBe(true);
    expect(shell.ui.knowledgeBaseLibrary).toBe(true);
    expect(shell.ui.scale0ScenarioSelect).toBe(true);
    expect(shell.ui.scale0LatticeSize).toBe(true);
    expect(shell.ui.scale1ScenarioSelect).toBe(true);
    expect(shell.ui.scale2ScenarioSelect).toBe(true);
    expect(shell.ui.scale3ScenarioSelect).toBe(true);
    expect(shell.ui.scale23VisualControls).toBe(true);
    expect(shell.ui.scale23ForceControls).toBe(true);
    expect(shell.ui.scale4ScenarioSelect).toBe(true);
    expect(shell.ui.zooPanel).toBe(true);
    expect(shell.ui.zooSearch).toBe(true);
    expect(shell.ui.zooFilter).toBe(true);
    expect(shell.ui.zooTableContainer).toBe(true);
    expect(shell.ui.planetaryPanel).toBe(true);
    expect(shell.ui.planetaryLayerList).toBe(true);
    expect(shell.ui.physicsPanel).toBe(true);
    expect(shell.ui.physicsEnergyLevels).toBe(true);
    expect(shell.ui.physicsZSlider).toBe(true);
    expect(shell.ui.inspectorPanel).toBe(true);
    expect(shell.ui.inspectorModeLabel).toBe(true);
    expect(shell.ui.inspectorSummary).toBe(true);
    expect(shell.ui.raycastThreshold).toBe(true);
    expect(shell.ui.peInspectorEmpty).toBe(true);
    expect(shell.ui.aeInspectorContent).toBe(true);
    expect(shell.ui.hierarchyPanel).toBe(true);
    expect(shell.ui.hierarchyTower).toBe(true);
    expect(shell.ui.scale5ScenarioSelect).toBe(true);
    expect(shell.ui.scale5CameraSelect).toBe(true);
    expect(shell.ui.scale5Telemetry).toBe(true);
    expect(shell.ui.cosmicInfoPanel).toBe(true);
    expect(shell.ui.cosmicPanelDiagnostics).toBe(true);
    expect(shell.ui.scale11ScenarioSelect).toBe(true);
    expect(shell.ui.scale11FigureSelect).toBe(true);
    expect(shell.ui.scale12MetaControls).toBe(true);
    expect(shell.ui.metaInfoPanel).toBe(true);
    expect(shell.ui.metaInspectPanel).toBe(true);
    expect(shell.ui.verificationLabPanel).toBe(true);
    expect(shell.ui.verifyHeaderSlot).toBe(true);
    expect(shell.ui.verifyTiersSlot).toBe(true);
    expect(shell.ui.verifyExportBtn).toBe(true);
    expect(shell.ui.onticPanelRemoved).toBe(true);
    expect(shell.mounts).toEqual([
        'shell-toolbar-mount',
        'shell-viewport-overlay-mount',
        'shell-panel-mount',
        'shell-modal-mount',
        'shell-toast-mount',
    ]);
});

test('UI panel registry matches rendered shell tabs and panels', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const registry = await import('./js/ui/scale-registry/panel-registry.js');
        const panelArea = document.getElementById('panel-area');
        const validation = registry.validatePanelRegistry(panelArea);
        const tabs = Array.from(document.querySelectorAll('#tab-bar .tab')).map((tab) => ({
            panel: tab.dataset.panel,
            label: tab.textContent?.trim() || '',
        }));
        const options = Array.from(document.querySelectorAll('#tab-select-mobile option')).map((option) => option.value);
        return {
            validation,
            registryCount: registry.getPanelRegistry().length,
            tabPanels: tabs.map((tab) => tab.panel),
            optionPanels: options,
            renderedLabelsMatch: tabs.every((tab) => registry.getPanelLabel(tab.panel) === tab.label),
        };
    });

    expect(result.validation.ok, `Panel registry errors: ${result.validation.errors.join(', ')}`).toBe(true);
    expect(result.registryCount).toBe(result.tabPanels.length);
    expect(result.optionPanels).toEqual(result.tabPanels);
    expect(result.renderedLabelsMatch).toBe(true);
});

test('Inspector helper modules drive chrome copy and selection state', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const chrome = await import('./js/inspector/chrome.js');
        const bindings = await import('./js/inspector/dom-bindings.js');
        const dom = bindings.collectInspectorDom();
        const target = {
            ...dom,
            _engineMode: 'particles',
            selectedIndex: 7,
            _selectedPos: null,
            _selectedPEParticleId: 42,
            _selectedAEAtomId: -1,
            _selectedPlanetaryId: -1,
            _selectedCosmicId: -1,
        };

        const summaryBeforeReset = chrome.getInspectorSelectionSummary(target);
        chrome.updateInspectorChrome(target);
        const labelAfterUpdate = dom.modeLabelEl?.textContent?.trim() || '';
        const summaryAfterUpdate = dom.selectionSummaryEl?.textContent?.trim() || '';
        const clearDisabledAfterUpdate = !!dom.clearSelectionBtn?.disabled;
        const focusDisabledAfterUpdate = !!dom.focusSelectionBtn?.disabled;

        chrome.resetInspectorSelection(target);
        chrome.updateInspectorChrome(target);

        return {
            domReady: !!dom.modeLabelEl && !!dom.selectionSummaryEl && !!dom.clearSelectionBtn && !!dom.focusSelectionBtn,
            summaryBeforeReset,
            labelAfterUpdate,
            summaryAfterUpdate,
            clearDisabledAfterUpdate,
            focusDisabledAfterUpdate,
            summaryAfterReset: dom.selectionSummaryEl?.textContent?.trim() || '',
            clearDisabledAfterReset: !!dom.clearSelectionBtn?.disabled,
        };
    });

    expect(result.domReady).toBe(true);
    expect(result.summaryBeforeReset).toBe('Selected particle #42.');
    expect(result.labelAfterUpdate).toBe('Particles');
    expect(result.summaryAfterUpdate).toBe('Selected particle #42.');
    expect(result.clearDisabledAfterUpdate).toBe(false);
    expect(result.focusDisabledAfterUpdate).toBe(true);
    expect(result.summaryAfterReset).toContain('inspect its identity');
    expect(result.clearDisabledAfterReset).toBe(true);
});

test('Inspector scale modules expose modular handlers', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const lattice = await import('./js/inspector/scales/lattice.js');
        const particles = await import('./js/inspector/scales/particles.js');
        const atoms = await import('./js/inspector/scales/atoms.js');
        const planetary = await import('./js/inspector/scales/planetary.js');
        const cosmic = await import('./js/inspector/scales/cosmic.js');

        return {
            lattice: ['handleLatticeClick', 'showLatticeInspector', 'hideLatticeInspector', 'updateLatticeFields']
                .every((name) => typeof lattice[name] === 'function'),
            particles: ['handlePEClick', 'showPEInspector', 'hidePEInspector', 'updatePEFields']
                .every((name) => typeof particles[name] === 'function'),
            atoms: ['handleAEClick', 'showAEInspector', 'hideAEInspector', 'updateAEFields', 'buildAEBondsList', 'updateAEMoleculeInfo', 'setAEScenarioInfo']
                .every((name) => typeof atoms[name] === 'function'),
            planetary: ['handlePlanetaryClick', 'showPlanetaryInspector', 'hidePlanetaryInspector', 'updatePlanetaryFields']
                .every((name) => typeof planetary[name] === 'function'),
            cosmic: ['handleCosmicClick', 'showCosmicInspector', 'hideCosmicInspector', 'updateCosmicFields']
                .every((name) => typeof cosmic[name] === 'function'),
        };
    });

    expect(result.lattice).toBe(true);
    expect(result.particles).toBe(true);
    expect(result.atoms).toBe(true);
    expect(result.planetary).toBe(true);
    expect(result.cosmic).toBe(true);
});

test('Responsive breakpoint: tablet layout at 768px width', async ({ page }) => {
    await page.setViewportSize({ width: 768, height: 1024 });
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const layout = await page.evaluate(() => {
        const app = document.getElementById('app');
        return {
            layoutMode:  app?.dataset.layoutMode  || null,
            orientation: app?.dataset.orientation || null,
            compact:     app?.dataset.compact     || null,
            tablet:      app?.dataset.tablet      || null,
            htmlMode:    document.documentElement.dataset.layoutMode || null,
        };
    });

    expect(layout.layoutMode).toBe('tablet');
    expect(layout.orientation).toBe('portrait');
    expect(layout.compact).toBe('false');
    expect(layout.tablet).toBe('true');
    expect(layout.htmlMode).toBe('tablet');
});

test('Responsive breakpoint: desktop layout at 1280px width', async ({ page }) => {
    await page.setViewportSize({ width: 1280, height: 800 });
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const layout = await page.evaluate(() => {
        const app = document.getElementById('app');
        return {
            layoutMode:  app?.dataset.layoutMode  || null,
            orientation: app?.dataset.orientation || null,
            compact:     app?.dataset.compact     || null,
            tablet:      app?.dataset.tablet      || null,
            htmlMode:    document.documentElement.dataset.layoutMode || null,
        };
    });

    expect(layout.layoutMode).toBe('desktop');
    expect(layout.orientation).toBe('landscape');
    expect(layout.compact).toBe('false');
    expect(layout.tablet).toBe('false');
    expect(layout.htmlMode).toBe('desktop');
});

test('Inspector app runtime exposes a modular app-shell adapter', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const runtimeMod = await import('./js/inspector/app-runtime.js');
        const inspectorMod = await import('./js/inspector.js');

        class FakeInspector {
            setEngineMode(mode) {
                if (!this.modeCalls) this.modeCalls = [];
                this.modeCalls.push(mode);
            }

            setBridge(nextBridge) {
                this.bridge = nextBridge;
            }

            getSelectedLatticePosition() {
                return this.selectedPos;
            }
        }

        const originalSetEngineMode = inspectorMod.Inspector.prototype.setEngineMode;
        const originalSetBridge = inspectorMod.Inspector.prototype.setBridge;
        const originalGetSelectedLatticePosition = inspectorMod.Inspector.prototype.getSelectedLatticePosition;

        try {
            inspectorMod.Inspector.prototype.setEngineMode = FakeInspector.prototype.setEngineMode;
            inspectorMod.Inspector.prototype.setBridge = FakeInspector.prototype.setBridge;
            inspectorMod.Inspector.prototype.getSelectedLatticePosition = FakeInspector.prototype.getSelectedLatticePosition;

            const syncCalls = [];
            const viewport = {
                setEngineMode(mode) {
                    syncCalls.push(`viewport:${mode}`);
                },
                camera: null,
            };
            const bridge = { id: 'bridge-a' };
            const runtime = runtimeMod.createInspectorAppRuntime({
                viewport,
                bridge,
                setZooMode: (mode) => syncCalls.push(`zoo:${mode}`),
            });

            runtime.syncMode('cosmic');
            runtime.setBridge({ id: 'bridge-b' });

            return {
                exportsOk: ['inspector', 'setBridge', 'syncMode', 'updateFloatingPanels']
                    .every((name) => typeof runtime[name] === 'function' || (name === 'inspector' && !!runtime.inspector)),
                inspectorHasGetter: typeof originalGetSelectedLatticePosition === 'function',
                modeCalls: runtime.inspector.modeCalls || [],
                syncCalls,
                bridgeId: runtime.inspector.bridge?.id || null,
            };
        } finally {
            inspectorMod.Inspector.prototype.setEngineMode = originalSetEngineMode;
            inspectorMod.Inspector.prototype.setBridge = originalSetBridge;
            inspectorMod.Inspector.prototype.getSelectedLatticePosition = originalGetSelectedLatticePosition;
        }
    });

    expect(result.exportsOk).toBe(true);
    expect(result.inspectorHasGetter).toBe(true);
    expect(result.modeCalls).toEqual(['cosmic']);
    expect(result.syncCalls).toEqual(['viewport:cosmic', 'zoo:cosmic']);
    expect(result.bridgeId).toBe('bridge-b');
});

test('Settings modal applies and resets extended shell preferences', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    await page.click('#btn-settings');
    await page.click('[data-setting="density"][data-value="compact"]');
    await page.click('[data-setting="panel-width"][data-value="wide"]');
    await page.click('[data-setting="tooltips"][data-value="off"]');
    await page.click('[data-setting="status-bar"][data-value="hidden"]');
    await page.click('[data-setting="motion"][data-value="reduced"]');

    await page.evaluate(() => {
        const slider = document.getElementById('settings-ui-scale');
        if (!(slider instanceof HTMLInputElement)) throw new Error('settings-ui-scale missing');
        slider.value = '1.2';
        slider.dispatchEvent(new Event('input', { bubbles: true }));
    });

    const applied = await page.evaluate(() => {
        const root = document.documentElement;
        const statusBar = document.getElementById('status-bar');
        return {
            density: root.dataset.density || '',
            panelWidth: root.dataset.panelWidth || '',
            tooltips: root.dataset.tooltips || '',
            statusBar: root.dataset.statusBar || '',
            motion: root.dataset.motion || '',
            uiScale: getComputedStyle(root).getPropertyValue('--ui-scale').trim(),
            statusDisplay: statusBar ? getComputedStyle(statusBar).display : '',
        };
    });

    expect(applied.density).toBe('compact');
    expect(applied.panelWidth).toBe('wide');
    expect(applied.tooltips).toBe('off');
    expect(applied.statusBar).toBe('hidden');
    expect(applied.motion).toBe('reduced');
    expect(applied.uiScale).toBe('1.2');
    expect(applied.statusDisplay).toBe('none');

    await page.click('#settings-reset');

    const reset = await page.evaluate(() => {
        const root = document.documentElement;
        const statusBar = document.getElementById('status-bar');
        return {
            density: root.dataset.density || '',
            panelWidth: root.dataset.panelWidth || '',
            tooltips: root.dataset.tooltips || '',
            statusBar: root.dataset.statusBar || '',
            motion: root.dataset.motion || '',
            uiScale: getComputedStyle(root).getPropertyValue('--ui-scale').trim(),
            theme: root.dataset.theme || 'default',
            statusDisplay: statusBar ? getComputedStyle(statusBar).display : '',
        };
    });

    expect(reset.density).toBe('comfortable');
    expect(reset.panelWidth).toBe('standard');
    expect(reset.tooltips).toBe('on');
    expect(reset.statusBar).toBe('shown');
    expect(reset.motion).toBe('');
    expect(reset.uiScale).toBe('1');
    expect(reset.theme).toBe('default');
    expect(reset.statusDisplay).toBe('flex');
});

test('UI tooltip system annotates controls and telemetry with custom help', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    await page.hover('#btn-play');

    const result = await page.evaluate(() => {
        const tooltip = document.getElementById('ui-tooltip');
        return {
            rootExists: !!tooltip,
            visible: tooltip?.hidden === false,
            text: tooltip?.textContent?.trim() || '',
            playTooltip: document.getElementById('btn-play')?.dataset.uiTooltip || null,
            energyTooltip: document.getElementById('pet-energy')?.dataset.uiTooltip || null,
            statusTooltip: document.getElementById('status-energy')?.dataset.uiTooltip || null,
            scenarioLabelTooltip: document.querySelector('label[for="pe-scenario-select"]')?.dataset.uiTooltip || null,
        };
    });

    expect(result.rootExists).toBe(true);
    expect(result.visible).toBe(true);
    expect(result.text).toContain('simulation loop');
    expect(result.playTooltip).toContain('Keyboard shortcut');
    expect(result.energyTooltip).toContain('Total particle-engine energy');
    expect(result.statusTooltip).toContain('Current total energy');
    expect(result.scenarioLabelTooltip).toContain('particle engine');
});

test('Tooltip palette follows theme switches', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    async function captureTooltipStyle() {
        await page.hover('#btn-play');
        return page.evaluate(() => {
            const tooltip = document.getElementById('ui-tooltip');
            const style = tooltip ? window.getComputedStyle(tooltip) : null;
            return {
                theme: document.documentElement.getAttribute('data-theme') || 'default',
                background: style?.backgroundColor || null,
                border: style?.borderTopColor || null,
                color: style?.color || null,
                shadow: style?.boxShadow || null,
            };
        });
    }

    const defaults = await captureTooltipStyle();

    await page.click('#btn-settings');
    await page.click('.theme-swatch[data-theme="light"]');
    await expect.poll(() => page.evaluate(() => document.documentElement.getAttribute('data-theme')))
        .toBe('light');
    await page.click('#settings-close');
    const light = await captureTooltipStyle();

    await page.click('#btn-settings');
    await page.click('.theme-swatch[data-theme="parchment"]');
    await expect.poll(() => page.evaluate(() => document.documentElement.getAttribute('data-theme')))
        .toBe('parchment');
    await page.click('#settings-close');
    const parchment = await captureTooltipStyle();

    expect(defaults.theme).toBe('default');
    expect(light.theme).toBe('light');
    expect(parchment.theme).toBe('parchment');
    expect(light.background).not.toBe(defaults.background);
    expect(parchment.background).not.toBe(light.background);
    expect(light.border).not.toBe(defaults.border);
    expect(parchment.border).not.toBe(light.border);
    expect(light.color).not.toBe(defaults.color);
    expect(parchment.shadow).not.toBe(defaults.shadow);
});

test('Knowledge base opens as a single responsive library with shared content', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    await page.click('#btn-knowledge-base');

    const libraryState = await page.evaluate(() => ({
        libraryOpen: document.getElementById('app')?.classList.contains('knowledge-base-open') || false,
        libraryVisible: document.getElementById('kb-sidebar')?.getAttribute('aria-hidden') === 'false',
        listCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        title: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        hasFormulaTokens: !!document.querySelector('#kb-sidebar-reader .kb-token-formula'),
    }));

    expect(libraryState.libraryOpen).toBe(true);
    expect(libraryState.libraryVisible).toBe(true);
    expect(libraryState.listCount).toBeGreaterThan(5);
    expect(libraryState.title.length).toBeGreaterThan(0);
    expect(libraryState.hasFormulaTokens).toBe(true);

    await page.fill('#kb-sidebar-search', 'nabla');

    const searchState = await page.evaluate(() => ({
        resultCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        readerTitle: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        readerText: document.getElementById('kb-sidebar-reader')?.textContent || '',
        resultsLabel: document.getElementById('kb-results-label')?.textContent?.trim() || '',
    }));

    expect(searchState.resultCount).toBeGreaterThan(0);
    expect(searchState.resultsLabel.length).toBeGreaterThan(0);
    expect(searchState.readerText.toLowerCase()).toContain('nabla');

    await page.fill('#kb-sidebar-search', 'natural units');

    const unitsState = await page.evaluate(() => ({
        resultCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        readerTitle: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        readerText: document.getElementById('kb-sidebar-reader')?.textContent || '',
    }));

    expect(unitsState.resultCount).toBeGreaterThan(0);
    expect(unitsState.readerTitle).toContain('Natural Units');
    expect(unitsState.readerText).toContain('c = 1');

    await page.fill('#kb-sidebar-search', 'born rule');

    const scenarioState = await page.evaluate(() => ({
        resultCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        readerTitle: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        readerText: document.getElementById('kb-sidebar-reader')?.textContent || '',
    }));

    expect(scenarioState.resultCount).toBeGreaterThan(0);
    expect(scenarioState.readerTitle).toContain('Born Rule');
    expect(scenarioState.readerText).toContain('|J|²');

    await page.fill('#kb-sidebar-search', 'TRAPPIST-1');

    const planetaryScenarioState = await page.evaluate(() => ({
        resultCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        readerTitle: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        readerText: document.getElementById('kb-sidebar-reader')?.textContent || '',
    }));

    expect(planetaryScenarioState.resultCount).toBeGreaterThan(0);
    expect(planetaryScenarioState.readerTitle).toContain('TRAPPIST-1');
    expect(planetaryScenarioState.readerText.toLowerCase()).toContain('resonance');

    await page.fill('#kb-sidebar-search', 'K_C');

    const consciousnessScenarioState = await page.evaluate(() => ({
        resultCount: document.querySelectorAll('#kb-sidebar-list [data-sidelib-entry]').length,
        readerTitle: document.querySelector('#kb-sidebar-reader .kb-reader-title')?.textContent?.trim() || '',
        readerText: document.getElementById('kb-sidebar-reader')?.textContent || '',
    }));

    expect(consciousnessScenarioState.resultCount).toBeGreaterThan(0);
    expect(consciousnessScenarioState.readerTitle).toContain('Threshold Crossing');
    expect(consciousnessScenarioState.readerText).toContain('Δ_k');
});
