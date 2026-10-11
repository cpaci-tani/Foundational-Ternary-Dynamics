/**
 * Scale-0 telemetry read helpers — prefer hub snapshots, fall back to bridge.
 * CONTRACTS.md §5: panels/overlays read hub first; bridge only when hub empty.
 */

import { E_REST } from '../constants.js';
import { telemetryHub } from '../telemetry-hub.js';

/**
 * @param {object|null} bridge - active Scale-0 bridge (optional fallback)
 * @returns {{ diag: object|null, audit: object|null }}
 */
export function readScale0DiagAudit(bridge = null) {
    const diagMeta = telemetryHub.getScale0TelemetryMeta?.('diagnostics') ?? null;
    const auditMeta = telemetryHub.getScale0TelemetryMeta?.('audit') ?? null;
    const diag = diagMeta
        ? (diagMeta.stale ? null : telemetryHub.s0?.diag ?? null)
        : (telemetryHub.s0?.diag ?? bridge?.getDiagnostics?.() ?? null);
    const audit = auditMeta
        ? (auditMeta.stale ? null : telemetryHub.s0?.audit ?? null)
        : (telemetryHub.s0?.audit ?? bridge?.getEnergyAudit?.() ?? null);
    return { diag, audit };
}

/** A group is usable as a current scientific observation only with provenance. */
export function isCurrentScale0TelemetryMeta(meta) {
    return !!meta
        && meta.stale !== true
        && (meta.status == null || meta.status === 'available')
        && Number.isFinite(meta.tick);
}

/**
 * Audit energy may accompany current diagnostics only when both observations
 * name the same engine tick. A current audit remains independently usable when
 * diagnostics are unavailable.
 */
export function isCurrentScale0AuditEnergy(diagMeta, auditMeta) {
    if (!isCurrentScale0TelemetryMeta(auditMeta)) return false;
    if (!isCurrentScale0TelemetryMeta(diagMeta)) return true;
    return auditMeta.tick === diagMeta.tick;
}

/**
 * Rest-offset-free accounted dynamic energy.
 *
 * `diag.totalEnergy` is intentionally excluded: compact/native diagnostics can
 * use it for the observer/vacuum baseline, so interpreting it as excitation
 * energy fabricates a current dynamic measurement. Exact numeric zero remains
 * valid when published by a current dynamic channel.
 */
export function readScale0TotalEnergy(diag, audit, {
    diagMeta = null,
    auditMeta = null,
    allowAuditSampleHold = false,
} = {}) {
    if (isCurrentScale0AuditEnergy(diagMeta, auditMeta)
        && Number.isFinite(audit?.dynamicEnergy)) {
        return audit.dynamicEnergy;
    }
    if (isCurrentScale0TelemetryMeta(diagMeta)
        && Number.isFinite(diag?.dynamicEnergy)) {
        return diag.dynamicEnergy;
    }
    // Status/readout surfaces may retain the newest independently current
    // audit observation between expensive audit reductions. This is a
    // zero-order sample hold, not a relabelling: callers opting in must expose
    // the audit sample tick separately from the faster diagnostics tick.
    if (allowAuditSampleHold
        && isCurrentScale0TelemetryMeta(auditMeta)
        && Number.isFinite(audit?.dynamicEnergy)) {
        return audit.dynamicEnergy;
    }
    return null;
}

/**
 * Total accounted energy of the lattice: the dynamic channel plus the rest
 * energy of every manifested site. This is the engine audit's `total_energy`
 * (diagnostics_compute.cpp). A current audit of the same tick is used as it
 * is; between audits the same sum is formed from the per-tick ledger and the
 * manifested count, which belong to one diagnostics tick.
 *
 * `diag.totalEnergy` is not this quantity (see readScale0TotalEnergy).
 * Returns `{ value, tick, epoch, source }` in engine energy units, or null.
 */
export function readScale0LatticeEnergy(diag, audit, { diagMeta = null, auditMeta = null } = {}) {
    if (isCurrentScale0AuditEnergy(diagMeta, auditMeta) && Number.isFinite(audit?.totalEnergy)) {
        return {
            value: audit.totalEnergy,
            tick: auditMeta.tick,
            epoch: auditMeta.sourceEpoch ?? auditMeta.epoch ?? null,
            source: 'same-tick-audit',
        };
    }
    if (isCurrentScale0TelemetryMeta(diagMeta)
        && Number.isFinite(diag?.dynamicEnergy) && Number.isFinite(diag?.manifested)) {
        return {
            value: diag.dynamicEnergy + diag.manifested * E_REST,
            tick: diagMeta.tick,
            epoch: diagMeta.sourceEpoch ?? diagMeta.epoch ?? null,
            source: diag.energySampleSource || 'diagnostics',
        };
    }
    return null;
}

/** Wave (kinetic) energy — ½Σ|wave_vel|². */
export function readScale0WaveEnergy(diag, audit) {
    if (audit != null && Number.isFinite(audit.waveEnergy)) return audit.waveEnergy;
    return Number.isFinite(diag?.totalWaveEnergy) ? diag.totalWaveEnergy : null;
}

/** Field energy — ½Σ|J|². */
export function readScale0FieldEnergy(audit) {
    return Number.isFinite(audit?.fieldEnergy) ? audit.fieldEnergy : null;
}
