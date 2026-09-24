import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { AE_PHYSICS_SPECS } from '../scales/scale2/scenario-registry.js';
import { showToast } from './status.js';
/** Bind once against live state; dispose before rebinding. */
export function wireAtomControls(ctx, { loadAEScenario }) {
    const scope = new LifetimeScope();
    // AE controls — force & dynamics toggles
    for (const spec of AE_PHYSICS_SPECS) {
        const el = document.getElementById(spec.elementId);
        if (!el) continue;
        scope.on(el, 'change', () => ctx.bridge[spec.setter]?.(el.checked));
    }

    // AE sliders
    const aeDtSlider = document.getElementById('ae-dt-slider');
    const aeDtValue = document.getElementById('ae-dt-value');
    if (aeDtSlider) {
        scope.on(aeDtSlider, 'input', () => {
            const dt = parseFloat(aeDtSlider.value);
            aeDtValue.textContent = dt.toFixed(3);
            ctx.bridge.aeSetDt(dt);
        });
    }

    const aeSoftSlider = document.getElementById('ae-soft-slider');
    const aeSoftValue = document.getElementById('ae-soft-value');
    if (aeSoftSlider) {
        scope.on(aeSoftSlider, 'input', () => {
            const s = parseFloat(aeSoftSlider.value);
            aeSoftValue.textContent = s.toFixed(2);
            ctx.bridge.aeSetSoftening(s);
        });
    }

    const aeThermostatSlider = document.getElementById('ae-thermostat-slider');
    const aeThermostatValue = document.getElementById('ae-thermostat-value');
    if (aeThermostatSlider) {
        scope.on(aeThermostatSlider, 'input', () => {
            const target = parseFloat(aeThermostatSlider.value);
            if (aeThermostatValue) aeThermostatValue.textContent = target.toFixed(2);
            ctx.bridge.aeSetThermostatTemp(target);
        });
    }

    // Dynamic Scale-2 nuclear laboratory. These controls mutate the live
    // environment; they never reload or branch on the selected scenario.
    const nuclearPatch = (patch) => ctx.bridge.aeSetNuclearEnvironment?.(patch);
    const bindNuclearRange = (id, valueId, key, format) => {
        const input = document.getElementById(id);
        const value = document.getElementById(valueId);
        scope.on(input, 'input', () => {
            const numeric = Number(input.value);
            if (value) value.textContent = format(numeric);
            nuclearPatch({ [key]: numeric });
        });
    };
    bindNuclearRange('ae-nuclear-reactivity', 'ae-nuclear-reactivity-value', 'reactivityScale', v => v.toFixed(1));
    bindNuclearRange('ae-nuclear-collision-radius', 'ae-nuclear-collision-radius-value', 'collisionRadiusScale', v => `${v.toFixed(2)}×`);
    bindNuclearRange('ae-nuclear-transport-radius', 'ae-nuclear-transport-radius-value', 'transportRadius', v => `${v.toFixed(0)} lu`);
    bindNuclearRange('ae-nuclear-moderator', 'ae-nuclear-moderator-value', 'moderatorStrength', v => v.toFixed(2));
    bindNuclearRange('ae-nuclear-absorber', 'ae-nuclear-absorber-value', 'absorberStrength', v => v.toFixed(2));
    bindNuclearRange('ae-nuclear-source-rate', 'ae-nuclear-source-rate-value', 'sourceRate', v => `${v.toFixed(2)}/tick`);
    scope.on(document.getElementById('ae-nuclear-boundary'), 'change', event =>
        nuclearPatch({ boundaryMode: event.currentTarget.value }));
    scope.on(document.getElementById('ae-nuclear-source-energy'), 'change', event =>
        nuclearPatch({ sourceEnergyMeV: Number(event.currentTarget.value) }));
    scope.on(document.getElementById('ae-nuclear-source-enabled'), 'change', event =>
        nuclearPatch({ sourceEnabled: event.currentTarget.checked }));
    scope.on(document.getElementById('ae-nuclear-channel'), 'change', event => {
        const channel = event.currentTarget.value;
        if (!channel) {
            ctx.bridge.aeConfigureNuclearReaction?.('');
            return;
        }
        ctx.bridge.aeConfigureNuclearReaction?.({
            channel,
            mode: 'sandbox',
            eventLimit: 100000,
            seed: 0x5eed235,
        });
        const valueOf = id => Number(document.getElementById(id)?.value);
        nuclearPatch({
            reactivityScale: valueOf('ae-nuclear-reactivity'),
            collisionRadiusScale: valueOf('ae-nuclear-collision-radius'),
            transportRadius: valueOf('ae-nuclear-transport-radius'),
            boundaryMode: document.getElementById('ae-nuclear-boundary')?.value,
            moderatorStrength: valueOf('ae-nuclear-moderator'),
            absorberStrength: valueOf('ae-nuclear-absorber'),
            sourceRate: valueOf('ae-nuclear-source-rate'),
            sourceEnergyMeV: valueOf('ae-nuclear-source-energy'),
            sourceEnabled: !!document.getElementById('ae-nuclear-source-enabled')?.checked,
        });
    });
    for (const [id, kind] of [
        ['btn-ae-inject-neutron', 'neutron'],
        ['btn-ae-inject-dt', 'dt-pair'],
        ['btn-ae-inject-u235', 'u235'],
    ]) {
        scope.on(document.getElementById(id), 'click', () => {
            if (ctx.bridge.aeInjectNuclearParticle?.(kind) === false) {
                showToast('Select a nuclear channel before injecting reactants.', 'info');
            }
        });
    }

    scope.on(document.getElementById('btn-ae-clear'), 'click', () => {
        ctx.running = false;
        ctx.updatePlayButton();
        loadAEScenario(document.getElementById('ae-scenario-select').value);
    });


    return () => scope.dispose();
}
