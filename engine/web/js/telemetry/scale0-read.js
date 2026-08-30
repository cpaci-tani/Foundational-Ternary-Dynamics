/**
 * Scale-0 telemetry read helpers — prefer hub snapshots, fall back to bridge.
 * CONTRACTS.md §5: panels/overlays read hub first; bridge only when hub empty.
 */

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
} = {}) {
    if (isCurrentScale0AuditEnergy(diagMeta, auditMeta)
        && Number.isFinite(audit?.dynamicEnergy)) {
        return audit.dynamicEnergy;
    }
    if (isCurrentScale0TelemetryMeta(diagMeta)
        && Number.isFinite(diag?.dynamicEnergy)) {
        return diag.dynamicEnergy;
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
