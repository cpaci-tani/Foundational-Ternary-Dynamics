import { formatS0SeedMetadata } from '../../../config/scenarios.js';
import {
    FORCE_FIELD_KEYS,
    SCALE0_MUTATION_REASONS,
    SCALE0_MUTATION_SOURCES,
    commitScale0ScientificMutation,
    getActiveScale0Bridge,
    getActiveScale0Capability,
    getActiveLatticeSize,
    getFieldStateSnapshot,
    getScalarRenderMode,
    getScale0QualificationState,
    getScale0State,
    setFieldToggle,
    setForceStyle,
    setScalarRenderMode,
} from '../state/store.js';
import { getScale0Scenario, populateScale0ScenarioSelect } from '../scenario-registry.js';
import {
    FIELD_TOGGLE_BINDINGS,
    FORCE_STYLE_VALUES,
    getEl,
    getSelectedScenarioId,
    readButtonActive,
    renderScenarioDescription,
    /* v=2: Tier 1 quantum overlay bindings added — see SPEC_S0_QUANTUM_OVERLAYS.md */
    setButtonActive,
    setForceStyleButtons,
    setScalarRenderButtons,
} from './dom.js?v=4';
import { COL_TO_TOGGLES } from './overlays/presets.js';
import { applyScale0OverlayApplicability } from './overlays/applicability.js?v=4';
import {
    initOverlayPanelShell,
    refreshOverlayPanelShell,
    scheduleOverlayPanelShellRefresh,
} from './overlays/panel-shell.js?v=6';
import { syncScale0LatticeSizeAvailability } from './toolbar/limits.js?v=2';
import {
    setScale0StandardModelReferenceVisible,
} from './overlays/standard-model.js?v=2';

let _bound = false;

function refreshFluxSliceOnly(ctx, api) {
    const adapter = api.viewportAdapter(ctx);
    const scale0 = getActiveScale0Capability(ctx);
    if (!adapter || !scale0) return;

    const latticeSize = getActiveLatticeSize(ctx);
    const sliceIndex = Math.floor(latticeSize / 2);
    const planes = [];
    for (const axis of adapter.getEnabledFluxSliceAxes()) {
        const data = scale0.getScale0FluxSlice(axis, sliceIndex);
        if (data?.length > 0) planes.push({ axis, data });
    }
    if (planes.length > 0) adapter.applyFluxSlices(planes, latticeSize, sliceIndex);
}

function setDisplayText(display, text) {
    if (!display || display.textContent === text) return;
    const node = display.firstChild;
    if (display.childNodes.length === 1 && node?.nodeType === 3) node.nodeValue = text;
    else display.textContent = text;
}

function createLatestInputFrame(ctx) {
    const pending = new Map();
    let frame = null;
    return (key, input, display, apply) => {
        pending.set(key, {
            generation: ctx._loadGeneration || 0,
            input,
            display,
            apply,
        });
        if (frame !== null) return;
        frame = requestAnimationFrame(() => {
            frame = null;
            const jobs = [...pending.values()];
            pending.clear();
            if (ctx.engineMode && ctx.engineMode !== 'lattice') return;
            const generation = ctx._loadGeneration || 0;
            for (const job of jobs) {
                if (job.generation !== generation) continue;
                const value = Number.parseFloat(job.input.value);
                if (!Number.isFinite(value)) continue;
                setDisplayText(job.display, value.toFixed(2));
                job.apply(value);
            }
        });
    };
}

export function updateScenarioMetadata(
    scenarioId,
    {
        profileModified = false,
        preserveDisclosure = false,
        qualificationState = getScale0QualificationState(),
    } = {},
) {
    const scenario = getScale0Scenario(scenarioId);
    const sections = [];
    if (qualificationState?.status === 'pending') {
        const pending = qualificationState.authoritativeLoad;
        sections.push(
            'AUTHORITATIVE SCENARIO LOAD PENDING — QUALIFICATION SUSPENDED',
            `Scenario: ${pending?.scenarioId || scenario.id}`,
            `Load generation: ${pending?.loadGeneration ?? 'unavailable'}`,
            'The registered claim is shown below, but this live record is not citable until the matching engine setup acknowledgement arrives.',
            '',
        );
    } else if (qualificationState?.status === 'suspended'
        && qualificationState.authoritativeLoad?.status === 'failed') {
        const failed = qualificationState.authoritativeLoad;
        sections.push(
            'AUTHORITATIVE SCENARIO LOAD FAILED — QUALIFICATION SUSPENDED',
            `Scenario: ${failed.scenarioId}; load generation: ${failed.loadGeneration}`,
            `Failure: ${failed.failureReason || 'setup failed without a reported reason'}`,
            'Any earlier qualified anchor is retained only as provenance; it is not applied to the partially replaced or unresolved live engine state.',
            'Reload the scenario and wait for a matching successful engine acknowledgement before citing its validation.',
            '',
        );
    } else if (qualificationState?.status === 'suspended') {
        const mutation = qualificationState.lastMutation;
        sections.push(
            'LIVE SCIENTIFIC RECORD MODIFIED — QUALIFICATION SUSPENDED',
            mutation
                ? `Reason: ${mutation.reason}; source: ${mutation.source}`
                : 'No authoritative scenario baseline has been acknowledged for this live record.',
            mutation
                ? `Mutation epoch: ${mutation.mutationEpoch}; observed dispatch tick: ${mutation.tick ?? 'unavailable'}; load generation: ${mutation.loadGeneration}`
                : `Mutation epoch: ${qualificationState.mutationEpoch}`,
            mutation?.dispatchStatus === 'unknown'
                ? 'Dashboard write intent was accepted; engine application is not acknowledged by this transport.'
                : (mutation ? `Dispatch status: ${mutation.dispatchStatus}` : ''),
            'Reload the scenario and wait for its matching engine acknowledgement before citing its validation.',
            '',
        );
    }
    if (profileModified) {
        sections.push(
            'MODIFIED PHYSICS PROFILE — QUALIFICATION SUSPENDED',
            'The visible scenario no longer matches its registered term set.',
            'Restore the scenario profile or reload the scenario before citing its validation.',
            '',
        );
    }
    // Surface the original precise name for renamed scenarios — the dropdown
    // now shows a plain-language title, so the technical name lives here.
    if (scenario?.laymanTitle && scenario.sourceTitle
        && scenario.sourceTitle !== scenario.laymanTitle) {
        sections.push(`Technical name: ${scenario.sourceTitle}`, '');
    }
    // The registry is the canonical user-facing epistemic source. Keep this
    // explicit even when validation or seed metadata follows: those sections
    // qualify behavior, but they do not replace the scenario's claim status.
    if (scenario?.epistemicStatus) {
        sections.push(
            'REGISTERED EPISTEMIC STATUS',
            scenario.epistemicStatus,
            `Evidence level: ${scenario.evidenceLevel || 'unspecified'}`,
            '',
        );
    }
    if (scenario?.validation) {
        sections.push(
            'AUTOMATED BEHAVIORAL VALIDATION',
            `Qualification: ${scenario.validation.qualification}`,
            scenario.validation.assertion,
            `Test target: ${scenario.validation.target}`,
        );
    } else if (scenario?.admissionStatus === 'hidden-research') {
        sections.push(
            'HIDDEN RESEARCH SCENARIO',
            `Qualification: ${scenario.qualification}`,
            `Mechanical test only: ${scenario.mechanicalTest}`,
        );
    }
    const seedMetadata = formatS0SeedMetadata(scenarioId);
    if (seedMetadata) sections.push('', seedMetadata);
    renderScenarioDescription(scenarioId, sections.join('\n'), { preserveOpen: preserveDisclosure });
}

export function bindScale0UI(ctx, api) {
    if (_bound) return;
    _bound = true;

    populateScale0ScenarioSelect(getEl('scenario-select'), getSelectedScenarioId('flux-pulse'));
    updateScenarioMetadata(getSelectedScenarioId('flux-pulse'));
    syncScale0LatticeSizeAvailability(ctx.bridge?.isNativeGPU);

    const boundarySelect = getEl('boundary-select');
    if (boundarySelect) {
        boundarySelect.addEventListener('change', () => {
            // Presentation geometry only. The discrete engine's scientific
            // boundary law is fluxBoundaryMode below; boundary shape is not
            // applied by any Scale-0 backend and must not suspend qualification.
            ctx.applyBoundaryShape(boundarySelect.value);
        });
    }

    const fluxBoundaryMode = getEl('flux-boundary-mode');
    if (fluxBoundaryMode) {
        fluxBoundaryMode.addEventListener('change', () => {
            const owner = getActiveScale0Bridge(ctx, getScale0State());
            commitScale0ScientificMutation(ctx, {
                reason: SCALE0_MUTATION_REASONS.FLUX_BOUNDARY,
                source: SCALE0_MUTATION_SOURCES.TOOLBAR_BOUNDARY,
                loadGeneration: ctx._loadGeneration || 0,
                owner,
                dispatchStatus: 'unknown',
            }, () => ctx.applyFluxBoundaryMode(parseInt(fluxBoundaryMode.value, 10)));
        });
    }

    const fluxPeriodicAxis = getEl('flux-periodic-axis');
    if (fluxPeriodicAxis) {
        fluxPeriodicAxis.addEventListener('change', () => {
            const owner = getActiveScale0Bridge(ctx, getScale0State());
            commitScale0ScientificMutation(ctx, {
                reason: SCALE0_MUTATION_REASONS.FLUX_BOUNDARY,
                source: SCALE0_MUTATION_SOURCES.TOOLBAR_BOUNDARY,
                loadGeneration: ctx._loadGeneration || 0,
                owner,
                dispatchStatus: 'unknown',
            }, () => ctx.applyFluxPeriodicAxis(parseInt(fluxPeriodicAxis.value, 10)));
        });
    }

    const scenarioSelect = getEl('scenario-select');
    if (scenarioSelect) {
        scenarioSelect.addEventListener('change', () => {
            ctx.pauseSimulation();
            const scenarioId = getScale0Scenario(scenarioSelect.value).id;
            api.loadScenario(ctx, scenarioId);
            updateScenarioMetadata(scenarioId);
        });
    }

    const latticeSize = getEl('lattice-size');
    if (latticeSize) {
        latticeSize.addEventListener('change', () => {
            api.resize(ctx, parseInt(latticeSize.value, 10));
        });
    }

    const fluxVolBtn = getEl('toggle-flux-volume');
    if (fluxVolBtn) {
        fluxVolBtn.addEventListener('click', () => {
            if (fluxVolBtn.classList.contains('is-inapplicable')) return;
            const on = !readButtonActive('toggle-flux-volume');
            setButtonActive('toggle-flux-volume', on);
            if (typeof ctx._scale0ForcedFluxVolumePreference === 'boolean') {
                ctx._scale0ForcedFluxVolumePreference = on;
            }
            api.viewportAdapter(ctx).setFluxVolumeVisible(on);
            api.setLatticeNeedsUpload();
        });
    }

    const fluxSliceBtn = getEl('toggle-flux-slice');
    if (fluxSliceBtn) {
        fluxSliceBtn.addEventListener('click', () => {
            if (fluxSliceBtn.classList.contains('is-inapplicable')) return;
            const on = !readButtonActive('toggle-flux-slice');
            setButtonActive('toggle-flux-slice', on);
            api.viewportAdapter(ctx).setFluxSliceVisible(on);
            // A slice toggle must not schedule the generic lattice upload: that
            // also rebuilds particles and the full flux volume. Refresh only
            // the three requested planes so paused simulations show current
            // data without introducing a frame hitch.
            if (on) refreshFluxSliceOnly(ctx, api);
        });
    }

    // Per-axis flux-slice plane toggles (xy/xz/yz). Axis index: yz=0, xz=1, xy=2.
    // These modify which mid-planes the all-axis flux slice overlay renders; they
    // default all-on and are independent of the volume-column badge/clear.
    for (const [axisName, axisIdx] of [['xy', 2], ['xz', 1], ['yz', 0]]) {
        const btnId = `flux-slice-axis-${axisName}`;
        const axisBtn = getEl(btnId);
        if (!axisBtn) continue;
        axisBtn.addEventListener('click', () => {
            const on = !readButtonActive(btnId);
            setButtonActive(btnId, on);
            api.viewportAdapter(ctx).setFluxSliceAxisEnabled(axisIdx, on);
            api.setLatticeNeedsUpload();
        });
    }

    // Flux-volume style sub-toggles (organic scatter vs grid; additive glow on/off).
    // These live with the Volume overlay controls so the status-bar Scene
    // menus only cover view, camera, environment, and boundary controls.
    const applyFluxOrganic = (on) => {
        setButtonActive('toggle-flux-organic', on);
        api.viewportAdapter(ctx).setFluxOrganic(on);
        api.setLatticeNeedsUpload();   // dot positions change → re-upload
    };
    const applyFluxGlow = (on) => {
        setButtonActive('toggle-flux-glow', on);
        api.viewportAdapter(ctx).setFluxGlow(on);   // live material change, no re-upload
    };
    getEl('toggle-flux-organic')?.addEventListener('click', () => applyFluxOrganic(!readButtonActive('toggle-flux-organic')));
    getEl('toggle-flux-glow')?.addEventListener('click', () => applyFluxGlow(!readButtonActive('toggle-flux-glow')));

    // Shared apply-toggle helper: works for both user clicks and
    // programmatic clear-column actions. `silent` means skip the
    // latticeNeedsUpload push (useful for bulk updates where we push
    // once at the end). Always keeps DOM button state, state-store
    // flag, and viewport visibility in sync.
    const setToggleState = (buttonId, fieldKey, on, { silent = false } = {}) => {
        setButtonActive(buttonId, on);
        setFieldToggle(fieldKey, on);
        const adapter = api.viewportAdapter(ctx);
        if (FORCE_FIELD_KEYS.has(fieldKey)) {
            const style = api.getForceStyle();
            if (style === 'arrows') {
                adapter.setOverlayVisible(fieldKey, on);
            } else {
                adapter.setOverlayVisible(fieldKey, false);
                adapter.syncForceStyle(style, getFieldStateSnapshot());
            }
        } else {
            adapter.setOverlayVisible(fieldKey, on);
        }
        if (!silent) api.setLatticeNeedsUpload();
    };

    // Map from buttonId → fieldKey so clear-column can look up the
    // state-store key for any managed toggle.
    const buttonIdToFieldKey = new Map(FIELD_TOGGLE_BINDINGS);

    for (const [buttonId, fieldKey] of FIELD_TOGGLE_BINDINGS) {
        const btn = getEl(buttonId);
        if (!btn) continue;
        btn.addEventListener('click', () => {
            if (btn.classList.contains('is-inapplicable')) return;
            const on = !readButtonActive(buttonId);
            setToggleState(buttonId, fieldKey, on);
            scheduleOverlayPanelShellRefresh();
        });
    }

    // Rubber-sheet slice-height sliders (Topology / Stress-Energy overlays):
    // each slides its sheet up/down and re-samples the field at that y-plane
    // (viewport.setTopologySheetHeight → TopologySheetRenderer.setHeight).
    const SHEET_HEIGHT_SLIDERS = [
        ['sheet-height-grav-potential', 'gravPotential'],
        ['sheet-height-em-energy',      'emEnergy'],
        ['sheet-height-charge-density', 'chargeDensity'],
        ['sheet-height-vorticity',      'vorticity'],
        ['sheet-height-e-pressure',     'ePressure'],
        ['sheet-height-b-pressure',     'bPressure'],
    ];
    const scheduleSheetHeight = createLatestInputFrame(ctx);
    for (const [sliderId, key] of SHEET_HEIGHT_SLIDERS) {
        const slider = getEl(sliderId);
        if (!slider) continue;
        const valEl = getEl(sliderId + '-val');
        slider.addEventListener('input', () => {
            scheduleSheetHeight(sliderId, slider, valEl,
                (value) => api.viewportAdapter(ctx).setTopologySheetHeight(key, value));
        });
    }

    // Per-column × clear buttons — turn off every toggle in one column.
    for (const clearBtn of document.querySelectorAll('.s0-overlay-col-clear')) {
        const colName = clearBtn.getAttribute('data-clear-col');
        const toggles = COL_TO_TOGGLES[colName];
        if (!toggles) continue;
        clearBtn.addEventListener('click', () => {
            let changed = false;
            for (const buttonId of toggles) {
                if (getEl(buttonId)?.classList.contains('is-inapplicable')) continue;
                if (!readButtonActive(buttonId)) continue;
                if (buttonId === 'toggle-flux-volume') {
                    setButtonActive(buttonId, false);
                    if (typeof ctx._scale0ForcedFluxVolumePreference === 'boolean') {
                        ctx._scale0ForcedFluxVolumePreference = false;
                    }
                    api.viewportAdapter(ctx).setFluxVolumeVisible(false);
                    changed = true;
                    continue;
                }
                if (buttonId === 'toggle-flux-slice') {
                    setButtonActive(buttonId, false);
                    api.viewportAdapter(ctx).setFluxSliceVisible(false);
                    changed = true;
                    continue;
                }
                if (buttonId === 'toggle-sm-reference') {
                    setButtonActive(buttonId, false);
                    setScale0StandardModelReferenceVisible(false);
                    changed = true;
                    continue;
                }
                const fieldKey = buttonIdToFieldKey.get(buttonId);
                if (!fieldKey) continue;
                setToggleState(buttonId, fieldKey, false, { silent: true });
                changed = true;
            }
            if (!changed) return;
            api.setLatticeNeedsUpload();
            scheduleOverlayPanelShellRefresh();
        });
    }

    const applyForceStyle = (style) => {
        if (!FORCE_STYLE_VALUES.includes(style) || style === api.getForceStyle()) return;
        setForceStyle(style);
        setForceStyleButtons(style);
        api.viewportAdapter(ctx).syncForceStyle(style, getFieldStateSnapshot());
        api.setLatticeNeedsUpload();
    };
    const styleRow = getEl('force-style-row');
    for (const btn of styleRow?.querySelectorAll('.style-btn') ?? []) {
        btn.addEventListener('click', () => applyForceStyle(btn.dataset.style));
    }
    getEl('scene-force-flow')?.addEventListener('click', (event) => {
        applyForceStyle(event.currentTarget.dataset.style);
    });
    setForceStyleButtons(api.getForceStyle());

    // Volumetric-scalar render-mode meta-toggle (Default / Heat Map). Flips every
    // active scalar overlay between its native sheet/cloud and a thermal glow cloud.
    const scalarModeRow = getEl('scalar-render-row');
    if (scalarModeRow) {
        for (const btn of scalarModeRow.querySelectorAll('.style-btn')) {
            btn.addEventListener('click', () => {
                const mode = btn.dataset.scalarMode === 'heatmap' ? 'heatmap' : 'default';
                if (mode === getScalarRenderMode()) return;
                setScalarRenderMode(mode);
                setScalarRenderButtons(mode);
                api.viewportAdapter(ctx).syncScalarRenderMode(mode, getFieldStateSnapshot());
                api.setLatticeNeedsUpload();
            });
        }
    }

    // Wire the panel shell: per-category accordion collapse, the active-overlays
    // strip, and the filter. Self-contained + idempotent (overlays/panel-shell.js).
    initOverlayPanelShell();
    applyScale0OverlayApplicability(
        getSelectedScenarioId('flux-pulse'),
        api.viewportAdapter(ctx),
    );
}

export function handleScale0ShortcutKey(key) {
    const shortcutMap = {
        '1': 'toggle-e-field',
        '2': 'toggle-b-field',
        '3': 'toggle-poynting',
        '4': 'toggle-div-field',
        '5': 'toggle-flux-lines',
        '6': 'toggle-force-em',
        '7': 'toggle-dual-substrate',
        '8': 'toggle-chirality',
    };
    const buttonId = shortcutMap[key];
    if (!buttonId) return false;
    getEl(buttonId)?.click();
    return true;
}
