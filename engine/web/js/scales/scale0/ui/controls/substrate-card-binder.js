import { LifetimeScope } from '../../../../ui/utils/lifetime-scope.js';

/**
 * Bind the small Scale-0 substrate cards that share the live scientific
 * mutation gateway.  This module owns only listener lifetime; the caller
 * keeps the transaction/provenance policy in the Scale-0 state owner.
 */
export function wireReadOnlyParameterCard(getEl) {
    const scope = new LifetimeScope();
    const sliders = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 4 },
    ];
    for (const sliderSpec of sliders) {
        const slider = getEl(sliderSpec.id);
        const display = getEl(sliderSpec.valId);
        if (!slider || !display) continue;
        slider.step = 'any';
        slider.disabled = true;
        slider.setAttribute('aria-readonly', 'true');
        slider.setAttribute('aria-disabled', 'true');
        slider.title = `${sliderSpec.param} is a read-only engine constant.`;
        slider.classList.add('ctrl-slider-disabled');
        slider.closest('.pe-ctrl-row')?.classList.add('ctrl-native-readonly');
    }
    return () => scope.dispose();
}

export function wireFieldActionCard({ getEl, ctx, api, scientificHarness, reasons, sources }) {
    const scope = new LifetimeScope();
    scope.on(getEl('btn-clear-field'), 'click', () => {
        const accepted = scientificHarness(
            ctx,
            reasons.CLEAR_FIELD,
            sources.SUBSTRATE_CONTROLS,
            harness => {
                if (typeof harness.clearField === 'function') harness.clearField();
                else harness.reset();
            },
        );
        if (!accepted) return;
        ctx.clearCharts?.();
        api.setLatticeNeedsUpload();
    });
    scope.on(getEl('btn-random-flux'), 'click', () => {
        if (scientificHarness(
            ctx,
            reasons.RANDOM_FLUX,
            sources.SUBSTRATE_CONTROLS,
            harness => harness.seedRandomFlux?.(),
        )) api.setLatticeNeedsUpload();
    });
    return () => scope.dispose();
}
