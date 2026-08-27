import { markFieldDirty } from './state/store.js';
import {
    rampViridis, rampEmEnergy, rampVorticity, rampCharge, rampGrayscale,
    rampGravWell, rampDivergingRdBu, rampEPressure, rampBPressure,
} from '../../viewport/color-ramps.js';

const SCALAR_HEATMAP = Object.freeze({
    showPsiSquared:        { key: 'psiSquared',    update: 'updatePsiSquaredField',        toggle: 'togglePsiSquaredField',       ramp: rampViridis,       signed: false },
    showLagrangianDensity: { key: 'lagrangian',    update: 'updateLagrangianDensityField', toggle: 'toggleLagrangianDensityField', ramp: rampDivergingRdBu, signed: true  },
    showEntropyDensity:    { key: 'entropy',       update: 'updateEntropyDensityField',    toggle: 'toggleEntropyDensityField',   ramp: rampGrayscale,     signed: false },
    showGravPotential:     { key: 'gravPotential', update: 'updateGravPotentialField',     toggle: 'toggleGravPotentialField',    ramp: rampGravWell,      signed: true  },
    showEmEnergy:          { key: 'emEnergy',      update: 'updateEmEnergyField',          toggle: 'toggleEmEnergyField',         ramp: rampEmEnergy,      signed: false },
    showChargeDensity:     { key: 'chargeDensity', update: 'updateChargeDensityField',     toggle: 'toggleChargeDensityField',    ramp: rampCharge,        signed: true  },
    showVorticity:         { key: 'vorticity',     update: 'updateVorticityField',         toggle: 'toggleVorticityField',        ramp: rampVorticity,     signed: false },
    showEPressure:         { key: 'ePressure',     update: 'updateEPressureField',         toggle: 'toggleEPressureField',        ramp: rampEPressure,     signed: false },
    showBPressure:         { key: 'bPressure',     update: 'updateBPressureField',         toggle: 'toggleBPressureField',        ramp: rampBPressure,     signed: false },
    showLatency:           { key: 'latency',       update: 'updateLatencyField',           toggle: 'toggleLatencyField',          ramp: rampEmEnergy,      signed: false },
    showGaussResidual:     { key: 'gaussResidual', update: 'updateGaussResidualField',     toggle: 'toggleGaussResidualField',    ramp: rampCharge,        signed: true  },
});

const BY_KEY = new Map(
    Object.values(SCALAR_HEATMAP).map(spec => [spec.key, spec]),
);

export function createScalarOverlayAdapter(viewport) {
    let mode = 'default';

    return Object.freeze({
        setVisible(flag, on) {
            const spec = SCALAR_HEATMAP[flag];
            if (!spec) return false;
            if (mode === 'heatmap') {
                viewport?.[spec.toggle]?.(false);
                viewport?.showScalarHeatmap?.(spec.key, on);
            } else {
                viewport?.showScalarHeatmap?.(spec.key, false);
                viewport?.[spec.toggle]?.(on);
            }
            return true;
        },
        setMode(nextMode) {
            mode = nextMode === 'heatmap' ? 'heatmap' : 'default';
        },
        syncMode(nextMode, fieldState = {}) {
            mode = nextMode === 'heatmap' ? 'heatmap' : 'default';
            if (!viewport) return;
            for (const [flag, spec] of Object.entries(SCALAR_HEATMAP)) {
                const active = !!fieldState[flag];
                viewport[spec.toggle]?.(active && mode === 'default');
                viewport.showScalarHeatmap?.(spec.key, active && mode === 'heatmap');
            }
            markFieldDirty();
        },
        apply(key, data) {
            const spec = BY_KEY.get(key);
            if (!spec) return false;
            if (mode === 'heatmap') {
                viewport?.updateScalarHeatmap?.(key, data, spec.ramp, spec.signed);
            } else {
                viewport?.[spec.update]?.(data);
            }
            return true;
        },
        clear() {
            if (!viewport) return;
            for (const spec of Object.values(SCALAR_HEATMAP)) {
                viewport[spec.toggle]?.(false);
                viewport.showScalarHeatmap?.(spec.key, false);
            }
        },
    });
}
