/** Scale-0 flux-volume card listener owner. */
import { LifetimeScope } from '../../../../ui/utils/lifetime-scope.js';

export function wireFluxVolumeCard(ctx, api, deps) {
    const { getEl, createLatestInputFrame, formatFluxThreshold, sliderPositionToFluxThreshold, setDisplayText } = deps;
    const scope = new LifetimeScope();
    // Range inputs can outpace the display refresh rate on high-polling mice.
    // Collapse every card-wide burst to one latest-value transaction per frame,
    // and collapse point-size/threshold upload invalidation to one dirty write.
    // A scenario load increments _loadGeneration; jobs from an older generation
    // are discarded because the loader has already captured/restored the DOM.
    const scheduleInput = createLatestInputFrame(ctx, {
        afterFlush: jobs => {
            if (jobs.some(job => job.metadata?.needsUpload)) api.setLatticeNeedsUpload();
        },
    });
    scope.defer(() => scheduleInput.dispose());

    const shapeSelect = getEl('flux-shape-select');
    if (shapeSelect) {
        scope.on(shapeSelect, 'change', () => {
            const shape = parseInt(shapeSelect.value, 10);
            if (!Number.isInteger(shape) || shape < 0 || shape > 7) return;
            ctx.viewport.setFluxShape(shape);
            ctx.viewport.setFluxSliceShape?.(shape);
        });
    }

    const opacitySlider = getEl('flux-opacity');
    const opacityVal = getEl('flux-opacity-val');
    if (opacitySlider && opacityVal) {
        scope.on(opacitySlider, 'input', () => {
            scheduleInput('opacity', opacitySlider, opacityVal, v => v.toFixed(2), (v) => {
                if (ctx._scale0ForcedVisualParameterPreferences
                    && 'fluxOpacity' in ctx._scale0ForcedVisualParameterPreferences) {
                    ctx._scale0ForcedVisualParameterPreferences.fluxOpacity = v;
                }
                ctx.viewport.setFluxOpacity(v);
                ctx.viewport.setFluxSliceOpacity?.(v);
            });
        });
    }

    const scaleSlider = getEl('flux-point-scale');
    const scaleVal = getEl('flux-point-scale-val');
    if (scaleSlider && scaleVal) {
        scope.on(scaleSlider, 'input', () => {
            scheduleInput('point-scale', scaleSlider, scaleVal, v => v.toFixed(1), (v) => {
                if (ctx._scale0ForcedVisualParameterPreferences
                    && 'fluxPointScale' in ctx._scale0ForcedVisualParameterPreferences) {
                    ctx._scale0ForcedVisualParameterPreferences.fluxPointScale = v;
                }
                ctx.viewport.setFluxPointScale(v);
                ctx.viewport.setFluxSlicePointScale?.(v);
            }, { needsUpload: true });
        });
    }

    const threshSlider = getEl('flux-threshold');
    const threshVal = getEl('flux-threshold-val');
    if (threshSlider && threshVal) {
        scope.on(threshSlider, 'input', () => {
            scheduleInput('threshold', threshSlider, threshVal,
                raw => formatFluxThreshold(sliderPositionToFluxThreshold(raw)), (raw) => {
                    const v = sliderPositionToFluxThreshold(raw);
                    threshSlider.setAttribute('aria-valuetext', formatFluxThreshold(v));
                    if (ctx._scale0ForcedVisualParameterPreferences
                        && 'fluxThreshold' in ctx._scale0ForcedVisualParameterPreferences) {
                        ctx._scale0ForcedVisualParameterPreferences.fluxThreshold = v;
                    }
                    ctx.viewport.setFluxThreshold(v);
                    ctx.viewport.setFluxSliceThreshold?.(v);
                }, { needsUpload: true });
        });
    }

    const scenarioScaleSlider = getEl('flux-scenario-scale');
    const scenarioScaleVal = getEl('flux-scenario-scale-val');
    if (scenarioScaleSlider && scenarioScaleVal) {
        scope.on(scenarioScaleSlider, 'input', () => {
            scheduleInput('scenario-scale', scenarioScaleSlider, scenarioScaleVal,
                v => v.toFixed(1), v => ctx.viewport.setScenarioScale(v));
        });
    }

    const latticeSpacingSlider = getEl('flux-lattice-spacing');
    const latticeSpacingVal = getEl('flux-lattice-spacing-val');
    if (latticeSpacingSlider && latticeSpacingVal) {
        scope.on(latticeSpacingSlider, 'input', () => {
            scheduleInput('lattice-spacing', latticeSpacingSlider, latticeSpacingVal,
                v => v.toFixed(2), v => ctx.viewport.setFluxLatticeSpacing?.(v));
        });
    }

    const wireframeBrightnessSlider = getEl('wireframe-brightness');
    const wireframeBrightnessVal = getEl('wireframe-brightness-val');
    if (wireframeBrightnessSlider && wireframeBrightnessVal) {
        scope.on(wireframeBrightnessSlider, 'input', () => {
            scheduleInput('wireframe-brightness', wireframeBrightnessSlider,
                wireframeBrightnessVal, v => v.toFixed(2),
                v => ctx.viewport.setWireframeBrightness?.(v));
        });
    }
    return () => scope.dispose();
}
