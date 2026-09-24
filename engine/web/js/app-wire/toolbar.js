import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { wireSimulationMenu } from '../ui/components/topbar/simulation-menu.js';
import * as Scale2Controller from '../scales/scale2/controller.js';
import * as Scale4Controller from '../scales/scale4/controller.js?v=12';
import * as Scale5Controller from '../scales/scale5/controller.js';
/** Bind once against live state; dispose before rebinding. */
export function wireToolbar(ctx, actions) {
    const scope = new LifetimeScope();
    scope.defer(wireSimulationMenu({
        select: document.getElementById('engine-mode'),
        trigger: document.getElementById('simulation-menu-trigger'),
        panel: document.getElementById('simulation-menu'),
        label: document.querySelector('label[for="engine-mode"]'),
    }));

    // Play/Pause
    scope.on(document.getElementById('btn-play'), 'click', ctx.togglePlay);

    scope.on(document.getElementById('btn-step'), 'click', actions.step);
    scope.on(document.getElementById('btn-reset'), 'click', actions.reset);

    const slider = document.getElementById('ticks-per-frame');
    ctx.applyTicksPerFrameFromSlider(slider.value);
    let speedInputRaf = null;
    scope.on(slider, 'input', () => {
        if (speedInputRaf !== null) return;
        speedInputRaf = requestAnimationFrame(() => {
            speedInputRaf = null;
            ctx.applyTicksPerFrameFromSlider(slider.value);
        });
    });

    // Engine mode selector (Scale 0 / Scale 1)
    scope.on(document.getElementById('engine-mode'), 'change', (e) => {
        ctx.pauseSimulation();
        actions.switchEngineMode(e.target.value);
    });

    // PE scenario selector
    scope.on(document.getElementById('pe-scenario-select'), 'change', (e) => {
        ctx.running = false;
        ctx.updatePlayButton();
        actions.loadPEScenario(e.target.value);
    });
    // AE scenario selector
    scope.on(document.getElementById('ae-scenario-select'), 'change', (e) => {
        ctx.running = false;
        ctx.updatePlayButton();
        actions.loadAEScenario(e.target.value);
    });

    // Scale 3 molecule scenario selector
    const molSelect = document.getElementById('mol-scenario-select');
    if (molSelect) {
        scope.on(molSelect, 'change', (e) => {
            ctx.running = false;
            ctx.updatePlayButton();
            actions.loadMoleculeScenario(e.target.value);
        });
    }

    // Planetary scenario selector
    const planetarySelect = document.getElementById('planetary-scenario-select');
    if (planetarySelect) {
        scope.on(planetarySelect, 'change', (e) => {
            ctx.running = false;
            ctx.updatePlayButton();
            // Pass live ctx (with getters) so the rafCoordinator loop callback reads live ctx.running/ctx.engineMode (audit P0-5 fix, 2026-05-27).
            Scale4Controller.loadScenario(ctx, e.target.value);
        });
    }

    // Cosmic scenario selector
    const cosmicSelect = document.getElementById('cosmic-scenario-select');
    if (cosmicSelect) {
        scope.on(cosmicSelect, 'change', (e) => {
            ctx.running = false;
            ctx.updatePlayButton();
            Scale5Controller.loadCosmicScenario(ctx, e.target.value);
        });
    }
    // Cosmic camera selector
    const cosmicCamera = document.getElementById('cosmic-camera-select');
    if (cosmicCamera) {
        scope.on(cosmicCamera, 'change', (e) => {
            Scale5Controller.setCameraPreset(e.target.value);
        });
    }

    // Scale 2 empirical orbital-cloud decoration.
    const cloudToggle = document.getElementById('ae-show-clouds');
    if (cloudToggle) {
        scope.on(cloudToggle, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('showOrbitalClouds', e.target.checked);
        });
    }
    // ── Enhanced atom/molecule visual controls ──

    // Nucleus shells (strong force glow)
    const shellToggle = document.getElementById('ae-show-shells');
    if (shellToggle) {
        scope.on(shellToggle, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('showNucleusShells', e.target.checked);
            ctx.viewport.toggleNucleusShells(e.target.checked);
        });
    }

    const labelToggle = document.getElementById('ae-show-labels');
    if (labelToggle) {
        scope.on(labelToggle, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('showElementLabels', e.target.checked);
            ctx.viewport.toggleElementLabels(e.target.checked);
        });
    }

    // Shell boundary spheres
    const shellBoundsToggle = document.getElementById('ae-show-shell-bounds');
    if (shellBoundsToggle) {
        scope.on(shellBoundsToggle, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('showShellBounds', e.target.checked);
            ctx.viewport.toggleOrbitalShells(e.target.checked);
        });
    }

    // Orbital lobes
    const lobeToggle = document.getElementById('ae-show-lobes');
    if (lobeToggle) {
        scope.on(lobeToggle, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('showOrbitalLobes', e.target.checked);
            ctx.viewport.toggleOrbitalLobes(e.target.checked);
        });
    }

    // Bond style selector
    const bondStyleSelect = document.getElementById('bond-style-select');
    if (bondStyleSelect) {
        scope.on(bondStyleSelect, 'change', (e) => {
            Scale2Controller.setAEVisualToggle('bondStyle', e.target.value);
            ctx.viewport.toggleBondCylinders(e.target.value === 'cylinders');
            ctx.viewport.toggleBondLines(e.target.value === 'lines');
        });
    }

    // Force arrow toggles
    const forceToggles = [
        ['ae-force-ionic', '_showAEForceIonic', 'toggleAEForceIonic'],
        ['ae-force-vdw', '_showAEForceVdw', 'toggleAEForceVdw'],
        ['ae-force-bond', '_showAEForceBond', 'toggleAEForceBond'],
        ['ae-force-hbond', '_showAEForceHBond', 'toggleAEForceHBond'],
        ['ae-force-angle', '_showAEForceAngle', 'toggleAEForceAngle'],
        ['ae-force-dipole', '_showAEForceDipole', 'toggleAEForceDipole'],
        ['ae-force-net', '_showAEForceNet', 'toggleAEForceNet'],
    ];
    for (const [id, flag, method] of forceToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            scope.on(btn, 'click', () => {
                const isActive = btn.classList.toggle('active');
                btn.setAttribute('aria-pressed', isActive ? 'true' : 'false');
                switch (flag) {
                    case '_showAEForceIonic': Scale2Controller.setAEVisualToggle('showAEForceIonic', isActive); break;
                    case '_showAEForceVdw': Scale2Controller.setAEVisualToggle('showAEForceVdw', isActive); break;
                    case '_showAEForceBond': Scale2Controller.setAEVisualToggle('showAEForceBond', isActive); break;
                    case '_showAEForceHBond': Scale2Controller.setAEVisualToggle('showAEForceHBond', isActive); break;
                    case '_showAEForceAngle': Scale2Controller.setAEVisualToggle('showAEForceAngle', isActive); break;
                    case '_showAEForceDipole': Scale2Controller.setAEVisualToggle('showAEForceDipole', isActive); break;
                    case '_showAEForceNet': Scale2Controller.setAEVisualToggle('showAEForceNet', isActive); break;
                }
                ctx.viewport[method](isActive);
            });
        }
    }

    scope.defer(() => { if (speedInputRaf !== null) cancelAnimationFrame(speedInputRaf); });
    return () => scope.dispose();
}
