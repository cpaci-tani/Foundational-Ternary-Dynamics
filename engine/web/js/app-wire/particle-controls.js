import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import * as Scale1Controller from '../scales/scale1/controller.js?v=31';
import { showToast } from './status.js';
import {
    captureScale1Checkpoint,
    importScale1Checkpoint,
    markScale1ReplayStart,
    restoreSavedScale1Checkpoint,
    serializeScale1Checkpoint,
    verifyScale1Replay,
} from '../scales/scale1/checkpoint-replay.js?v=1';
/** Bind once against live state; dispose before rebinding. */
export function wireParticleControls(ctx, { loadPEScenario }) {
    const scope = new LifetimeScope();

    // PE controls — every row carries the exact native registry toggle key.
    // This avoids a second hand-maintained setter map in the browser.
    for (const el of document.querySelectorAll('[data-pe-toggle]')) {
        scope.on(el, 'change', () => {
            const accepted = ctx.bridge.peSetToggle?.(el.dataset.peToggle, el.checked);
            if (accepted === false) {
                el.checked = !!ctx.bridge.peGetToggle?.(el.dataset.peToggle);
            }
            Scale1Controller.markPhysicsProfileModified();
        });
    }

    for (const button of document.querySelectorAll('[data-pe-profile]')) {
        scope.on(button, 'click', () => {
            Scale1Controller.applyPhysicsProfile(ctx.bridge, button.dataset.peProfile);
        });
    }

    // PE sliders
    const dtSlider = document.getElementById('pe-dt-slider');
    const dtValue = document.getElementById('pe-dt-value');
    if (dtSlider) {
        scope.on(dtSlider, 'input', () => {
            const dt = parseFloat(dtSlider.value);
            dtValue.textContent = dt.toFixed(1);
            ctx.bridge.peSetDt(dt);
            Scale1Controller.markObservationDirty();
        });
    }

    const softSlider = document.getElementById('pe-soft-slider');
    const softValue = document.getElementById('pe-soft-value');
    if (softSlider) {
        scope.on(softSlider, 'input', () => {
            const s = parseFloat(softSlider.value);
            softValue.textContent = s.toFixed(2);
            ctx.bridge.peSetSoftening(s);
            Scale1Controller.markObservationDirty();
            ctx.telemetryHub.setScale1Runtime({ softening: s });
        });
    }

    // Trajectory history is presentation-only and tick-aligned. The generic
    // data key keeps all visual history controls on one controller contract.
    for (const input of document.querySelectorAll('[data-pe-trail-setting]')) {
        scope.on(input, 'input', () => {
            Scale1Controller.setTrailSettings({
                [input.dataset.peTrailSetting]: parseFloat(input.value),
            });
        });
    }
    for (const button of document.querySelectorAll('[data-pe-trail-mode]')) {
        scope.on(button, 'click', () => {
            Scale1Controller.setTrailSettings({ renderMode: button.dataset.peTrailMode });
        });
    }
    scope.on(document.getElementById('btn-pe-trail-reset'), 'click', () => {
        Scale1Controller.resetTrailSettings();
    });

    scope.on(document.getElementById('btn-pe-clear'), 'click', () => {
        ctx.running = false;
        ctx.updatePlayButton();
        loadPEScenario(document.getElementById('pe-scenario-select').value);
    });

    const checkpointStatus = document.getElementById('pe-checkpoint-status');
    const setCheckpointStatus = (message, failed = false) => {
        if (checkpointStatus) {
            checkpointStatus.textContent = message;
            checkpointStatus.dataset.status = failed ? 'error' : 'ready';
        }
    };
    const afterCheckpointMutation = () => {
        Scale1Controller.markObservationDirty();
        ctx.telemetryHub.resetScale1?.();
    };
    scope.on(document.getElementById('btn-pe-checkpoint-save'), 'click', () => {
        try {
            const result = captureScale1Checkpoint(ctx.bridge);
            setCheckpointStatus(`Captured tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    scope.on(document.getElementById('btn-pe-checkpoint-restore'), 'click', () => {
        try {
            ctx.running = false;
            ctx.updatePlayButton();
            const result = restoreSavedScale1Checkpoint(ctx.bridge);
            afterCheckpointMutation();
            setCheckpointStatus(`Restored tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    scope.on(document.getElementById('btn-pe-checkpoint-export'), 'click', () => {
        try {
            const captured = captureScale1Checkpoint(ctx.bridge);
            const blob = new Blob([serializeScale1Checkpoint()], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const anchor = document.createElement('a');
            anchor.href = url;
            anchor.download = `ftd-scale1-tick-${captured.tick}.json`;
            anchor.click();
            setTimeout(() => URL.revokeObjectURL(url), 0);
            setCheckpointStatus(`Exported tick ${captured.tick} · ${captured.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    const checkpointFile = document.getElementById('pe-checkpoint-file');
    scope.on(document.getElementById('btn-pe-checkpoint-import'), 'click', () => {
        checkpointFile?.click();
    });
    scope.on(checkpointFile, 'change', async () => {
        const file = checkpointFile.files?.[0];
        if (!file) return;
        try {
            ctx.running = false;
            ctx.updatePlayButton();
            const result = importScale1Checkpoint(await file.text(), ctx.bridge);
            afterCheckpointMutation();
            setCheckpointStatus(`Imported tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        } finally {
            checkpointFile.value = '';
        }
    });
    scope.on(document.getElementById('btn-pe-replay-mark'), 'click', () => {
        try {
            const result = markScale1ReplayStart(ctx.bridge);
            setCheckpointStatus(`Replay start marked at tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    scope.on(document.getElementById('btn-pe-replay-verify'), 'click', async () => {
        try {
            ctx.running = false;
            ctx.updatePlayButton();
            setCheckpointStatus('Replaying the marked segment…');
            const result = await verifyScale1Replay(ctx.bridge);
            afterCheckpointMutation();
            setCheckpointStatus(result.match
                ? `Replay matched ${result.ticks} ticks · ${result.actualDigest}`
                : `Replay mismatch · expected ${result.expectedDigest}, got ${result.actualDigest}`,
            !result.match);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    const updateFieldBatteryStatus = () => {
        const snapshot = ctx.bridge.peGetFinitePortBatterySnapshot?.();
        const status = document.getElementById('pe-field-battery-status');
        if (status && snapshot) {
            status.textContent = `Layer ${snapshot.acceptedLayers}/${snapshot.capacity} · `
                + `E ${Number(snapshot.totalBookedEnergy).toPrecision(6)}`;
        }
        Scale1Controller.markObservationDirty();
    };
    scope.on(document.getElementById('btn-pe-field-battery-step'), 'click', () => {
        if (!ctx.bridge.peStepFinitePortBattery?.()) {
            showToast('Finite ready-port capacity is exhausted.', 'info');
        }
        updateFieldBatteryStatus();
    });
    scope.on(document.getElementById('btn-pe-field-battery-reverse'), 'click', () => {
        if (!ctx.bridge.peReverseFinitePortBattery?.()) {
            showToast('No accepted field layer is available to reverse.', 'info');
        }
        updateFieldBatteryStatus();
    });



    return () => scope.dispose();
}
