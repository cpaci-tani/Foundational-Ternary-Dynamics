import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import * as Scale1Controller from '../scales/scale1/controller.js?v=31';
import * as Scale2Controller from '../scales/scale2/controller.js';
import * as Scale4Controller from '../scales/scale4/controller.js?v=12';
import * as Scale5Controller from '../scales/scale5/controller.js';
import * as Scale6Controller from '../scales/scale6/controller.js';

const customControllers = { planetary: Scale4Controller, cosmic: Scale5Controller, meta: Scale6Controller };
const controlIds = { axes: 'toggle-axes', grid: 'toggle-grid', boundaryOrientation: 'toggle-boundary-orientation', globalClock: 'toggle-global-clock' };

/** Reflect the displayed renderer, rather than a hidden shared scene. */
export function syncViewControls(ctx) {
    const controller = customControllers[ctx.engineMode];
    const capabilities = controller?.getViewControlCapabilities() || {
        axes: true, grid: true, boundaryOrientation: ctx.engineMode === 'lattice', globalClock: ctx.engineMode === 'lattice',
    };
    const state = controller?.getViewControlState() || ctx.viewport?.getViewControlState() || {};
    for (const [key, id] of Object.entries(controlIds)) {
        const button = document.getElementById(id);
        if (!button) continue;
        if (!button.dataset.supportedTitle) button.dataset.supportedTitle = button.title || button.getAttribute('aria-label') || key;
        button.disabled = !capabilities[key];
        button.title = button.disabled ? `This view does not provide ${key.replace(/[A-Z]/g, c=>' '+c.toLowerCase())}.` : button.dataset.supportedTitle;
        if (key in state || button.disabled) {
            const on = !button.disabled && !!state[key];
            button.classList.toggle('active', on);
            button.setAttribute('aria-pressed', String(on));
        }
    }
}
/** Bind once against live state; dispose before rebinding. */
export function wireViewportControls(ctx) {
    const scope = new LifetimeScope();
    scope.on(document, 'ftd:view-controls-changed', () => syncViewControls(ctx));

    const setToggleState = (button, on) => {
        button.classList.toggle('active', on);
        button.setAttribute('aria-pressed', on ? 'true' : 'false');
    };

    // Universal toggles (visible on all scales)
    const axesBtn = document.getElementById('toggle-axes');
    if (axesBtn) {
        scope.on(axesBtn, 'click', () => {
            const on = !axesBtn.classList.contains('active');
            setToggleState(axesBtn, on);
            const controller = customControllers[ctx.engineMode];
            if (controller) controller.setAxesVisible?.(on);
            else ctx.viewport.toggleAxes(on);
        });
    }
    // Grid button also controls the wireframe (lattice boundary box) at Scale 0
    const gridBtn = document.getElementById('toggle-grid');
    if (gridBtn) {
        scope.on(gridBtn, 'click', () => {
            const on = !gridBtn.classList.contains('active');
            setToggleState(gridBtn, on);
            const controller = customControllers[ctx.engineMode];
            if (controller) controller.setGridVisible?.(on);
            else {
                ctx.viewport.toggleGrid(on);
                if (ctx.engineMode === 'lattice') ctx.viewport.toggleWireframe(on);
            }
        });
    }

    const orientationBtn = document.getElementById('toggle-boundary-orientation');
    if (orientationBtn) {
        scope.on(orientationBtn, 'click', () => {
            const on = !orientationBtn.classList.contains('active');
            setToggleState(orientationBtn, on);
            ctx.viewport.toggleBoundaryOrientation(on);
        });
    }

    const clockBtn = document.getElementById('toggle-global-clock');
    if (clockBtn) {
        scope.on(clockBtn, 'click', () => {
            const on = !clockBtn.classList.contains('active');
            setToggleState(clockBtn, on);
            ctx.viewport.toggleGlobalClock(on);
        });
    }

    // Camera preset buttons — Scale-0-specific camera viewpoints. Each
    // button snaps the orbit camera to a named direction; "Fit" zooms to
    // frame the active flux volume. Buttons hidden on non-lattice scales
    // via the .scale0-only class on their container.
    for (const btn of document.querySelectorAll('[data-cam-preset]')) {
        const preset = btn.getAttribute('data-cam-preset');
        scope.on(btn, 'click', () => {
            if (!ctx.viewport) return;
            if (preset === 'fit') {
                ctx.viewport.zoomToFit?.();
            } else {
                ctx.viewport.setCameraPreset?.(preset);
            }
            // Transient visual pulse so the user sees the preset was applied,
            // without leaving any button stuck in an active state (these
            // are momentary actions, not persistent toggles).
            btn.classList.add('status-preset-flash');
            scope.timeout(() => btn.classList.remove('status-preset-flash'), 260);
        });
    }


    // PE mode visual overlay toggles (delegated to Scale1Controller)
    const velBtn = document.getElementById('toggle-velocities');
    if (velBtn) scope.on(velBtn, 'click', () => {
        velBtn.classList.toggle('active');
        const on = velBtn.classList.contains('active');
        velBtn.setAttribute('aria-pressed', on ? 'true' : 'false');
        Scale1Controller.setVelocities(on);
        ctx.viewport.toggleVelocityVectors(on);
    });

    const trailBtn = document.getElementById('toggle-trails');
    if (trailBtn) scope.on(trailBtn, 'click', () => {
        trailBtn.classList.toggle('active');
        const trailOn = trailBtn.classList.contains('active');
        trailBtn.setAttribute('aria-pressed', trailOn ? 'true' : 'false');
        Scale1Controller.setTrails(trailOn);
        ctx.viewport.toggleTrails(trailOn);
    });

    // PE field overlay toggles (delegated to Scale1Controller)
    const peFieldToggles = [
        ['toggle-pe-efield', (on) => { Scale1Controller.setPEEField(on); ctx.viewport.togglePEStreamlines(on); }],
        ['toggle-pe-potential', (on) => {
            Scale1Controller.setPEPotential(on);
            const active = Scale1Controller.isPEFieldSurfaceActive();
            ctx.viewport.toggleFieldHeatmap(active); ctx.viewport.toggleFieldVectors(active);
        }],
        ['toggle-pe-field-battery', (on) => {
            Scale1Controller.setPEFieldBattery(on);
            const active = Scale1Controller.isPEFieldSurfaceActive();
            ctx.viewport.toggleFieldHeatmap(active); ctx.viewport.toggleFieldVectors(active);
        }],
        ['toggle-pe-gravity-field', (on) => { Scale1Controller.setPEGravField(on); ctx.viewport.toggleGravityVectors(on); }],
        ['toggle-pe-force-coulomb', (on) => { Scale1Controller.setPEForceCoulomb(on); ctx.viewport.togglePEForceCoulomb(on); }],
        ['toggle-pe-force-gravity', (on) => { Scale1Controller.setPEForceGravity(on); ctx.viewport.togglePEForceGravity(on); }],
        ['toggle-pe-force-lorentz', (on) => { Scale1Controller.setPEForceLorentz(on); ctx.viewport.togglePEForceLorentz(on); }],
        ['toggle-pe-force-exchange', (on) => { Scale1Controller.setPEForceExchange(on); ctx.viewport.togglePEForceExchange(on); }],
        ['toggle-pe-force-strong', (on) => { Scale1Controller.setPEForceStrong(on); ctx.viewport.togglePEForceStrong(on); }],
        ['toggle-pe-force-radiation', (on) => { Scale1Controller.setPEForceRadiation(on); ctx.viewport.togglePEForceRadiation(on); }],
        ['toggle-pe-force-magnetic-dipole', (on) => { Scale1Controller.setPEForceMagneticDipole(on); ctx.viewport.togglePEForceMagneticDipole(on); }],
        ['toggle-pe-force-spin-orbit', (on) => { Scale1Controller.setPEForceSpinOrbit(on); ctx.viewport.togglePEForceSpinOrbit(on); }],
        ['toggle-pe-force-net', (on) => { Scale1Controller.setPEForceNet(on); ctx.viewport.togglePEForceNet(on); }],
        ['toggle-pe-system', (on) => { Scale1Controller.setPESystem(on); ctx.viewport.togglePESystem(on); }],
        ['toggle-pe-admissibility', (on) => { Scale1Controller.setAdmissibilityRing(on); ctx.viewport.toggleAdmissibilityRings(on); }],
        ['toggle-pe-provenance', (on) => { Scale1Controller.setProvenanceLabel(on); ctx.viewport.toggleProvenanceLabels(on); }],
    ];
    for (const [id, handler] of peFieldToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            scope.on(btn, 'click', () => {
                btn.classList.toggle('active');
                const on = btn.classList.contains('active');
                btn.setAttribute('aria-pressed', on ? 'true' : 'false');
                handler(on);
            });
        }
    }

    // AE field overlay toggle
    const aeFieldBtn = document.getElementById('toggle-ae-field');
    if (aeFieldBtn) {
        scope.on(aeFieldBtn, 'click', () => {
            aeFieldBtn.classList.toggle('active');
            const aeFieldOn = aeFieldBtn.classList.contains('active');
            aeFieldBtn.setAttribute('aria-pressed', aeFieldOn ? 'true' : 'false');
            Scale2Controller.setAEVisualToggle('showAEField', aeFieldOn);
            ctx.viewport.toggleFieldHeatmap(aeFieldOn);
            ctx.viewport.toggleFieldVectors(aeFieldOn);
        });
    }

    // AE kinetic/electrostatic structure overlays (Scale 2 deep pass):
    // velocity vectors, dipole arrows, dashed H-bond lines. Flags drive
    // per-frame updates in Scale2Controller.animateAE; the ctx.viewport toggle
    // controls layer visibility immediately.
    const aeStructureToggles = [
        ['toggle-ae-velocities', 'showAEVelocities', (on) => ctx.viewport.toggleVelocityVectors(on)],
        ['toggle-ae-dipoles', 'showAEDipoles', (on) => ctx.viewport.toggleAEDipoles(on)],
        ['toggle-ae-hbonds', 'showAEHBondLines', (on) => ctx.viewport.toggleHBondLines(on)],
        ['toggle-ae-nuclear-events', 'showAENuclearEvents', (on) => ctx.viewport.toggleNuclearEvents?.(on)],
        ['toggle-ae-radiation', 'showAERadiation', (on) => ctx.viewport.toggleNuclearRadiation?.(on)],
        ['toggle-ae-heat', 'showAEHeat', (on) => ctx.viewport.toggleNuclearHeat?.(on)],
        ['toggle-ae-nuclear-boundary', 'showAENuclearBoundary', (on) => ctx.viewport.toggleNuclearBoundary?.(on)],
    ];
    for (const [btnId, flagKey, vpToggle] of aeStructureToggles) {
        const btn = document.getElementById(btnId);
        if (btn) {
            scope.on(btn, 'click', () => {
                const isOn = btn.classList.toggle('active');
                btn.setAttribute('aria-pressed', isOn ? 'true' : 'false');
                Scale2Controller.setAEVisualToggle(flagKey, isOn);
                vpToggle(isOn);
            });
        }
    }



    return () => scope.dispose();
}
