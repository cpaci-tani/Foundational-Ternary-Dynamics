import { LifetimeScope } from './lifetime-scope.js';

/**
 * Owns the repeated Scale-0 scenario-selector and bounded reconciliation
 * lifecycle. Panels keep their scientific readiness predicate and rendering
 * policy; this owner only guarantees that selector replacement, superseded
 * frames, and teardown have one implementation.
 */
export class ScenarioApplicabilityBinding {
    constructor({
        selectorId = 'scenario-select',
        getCurrentScenarioId = () => '',
        onIntent = () => {},
    } = {}) {
        this.selectorId = selectorId;
        this.getCurrentScenarioId = getCurrentScenarioId;
        this.onIntent = onIntent;
        this.revision = 0;
        this._disposed = false;
        this._select = null;
        this._selectLifetime = null;
        this._reconcileLifetime = null;
    }

    bind() {
        if (this._disposed) return;
        const nextSelect = document.getElementById(this.selectorId);
        if (nextSelect !== this._select) {
            this._selectLifetime?.dispose();
            this._selectLifetime = new LifetimeScope();
            this._select = nextSelect;
            this._selectLifetime.on(nextSelect, 'change', (event) => {
                this.intent(String(event.currentTarget?.value || ''));
            });
        }
        this.intent(String(
            this._select?.value || this.getCurrentScenarioId?.() || '',
        ));
    }

    intent(scenarioId) {
        if (this._disposed) return;
        this.cancelReconcile();
        this.revision++;
        this.onIntent(String(scenarioId || ''), this.revision);
    }

    reconcile({ scenarioId, isReady, onReady, maxFrames = 120 } = {}) {
        if (this._disposed) return;
        this._reconcileLifetime?.dispose();
        this._reconcileLifetime = new LifetimeScope();
        const lifetime = this._reconcileLifetime;
        const revision = this.revision;
        let remaining = Math.max(1, Number(maxFrames) || 1);

        const check = () => {
            if (this._disposed || lifetime.disposed || revision !== this.revision) return;
            if (isReady?.(scenarioId)) {
                lifetime.dispose();
                if (this._reconcileLifetime === lifetime) this._reconcileLifetime = null;
                onReady?.(scenarioId);
                return;
            }
            remaining--;
            if (remaining > 0) lifetime.frame(check);
        };
        check();
    }

    cancelReconcile() {
        this._reconcileLifetime?.dispose();
        this._reconcileLifetime = null;
    }

    dispose() {
        if (this._disposed) return;
        this._disposed = true;
        this.cancelReconcile();
        this._selectLifetime?.dispose();
        this._selectLifetime = null;
        this._select = null;
    }
}
