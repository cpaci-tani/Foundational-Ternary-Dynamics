// @ts-check
import { traceObserverRay, visibleSegments } from './optics.js';
/** A read-only event export. Empty rays are not classified as missing history.
 * Only a hit in a still-retained interval preceding the policy cutoff is a
 * known unavailable witness; pruned or never-created intervals remain unknown.
 * @param {import('./types.js').WorldSnapshot} snapshot
 * @param {import('./optics.js').OpticalSettings} settings
 * @param {import('./optics.js').OpticalHit|null} hit
 * @param {import('./optics.js').OpticalSegment[]} [segments]
 * @param {number} [ndcX] @param {number} [ndcY]
 */
export function observationDiagnostics(snapshot, settings, hit, segments = visibleSegments(snapshot, settings), ndcX = 0, ndcY = 0) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride };
    const opticalMode = !!snapshot.spacetime || snapshot.profile === 'sr' && settings.optical !== false;
    const segment = hit ? segments[hit.segmentIndex] : null;
    let historyStatus = hit ? 'available' : 'no-hit';
    if (!hit && opticalMode) {
        const earliest = Math.min(snapshot.historyStart, ...snapshot.segments.map(s => s.start));
        if (earliest < snapshot.historyStart) {
            const witness = traceObserverRay({ ...snapshot, historyStart: earliest }, settings, ndcX, ndcY);
            if (witness && witness.emissionTime < snapshot.historyStart) historyStatus = 'before-history-start';
        }
    }
    return {
        sessionId: snapshot.sessionId, epoch: snapshot.epoch, observerWorldlineId: observer.worldline,
        tick: snapshot.tick, integratorVersion: snapshot.integratorVersion ?? 'legacy-unspecified',
        ...(snapshot.spacetime?{spacetime:structuredClone(snapshot.spacetime),velocityConvention:'local static orthonormal frame, fraction of c',distanceConvention:'Euclidean arclength in isotropic coordinates; travel time is observationTime minus emissionTime',sourceClockConvention:snapshot.spacetime.kind==='schwarzschild-black-hole'?'Finite static sky shell; the horizon has no emitting clock':'static surface proper time; star uses its surface lapse',modelBoundary:snapshot.spacetime.kind==='schwarzschild-black-hole'?'Adopted nonrotating, uncharged Schwarzschild vacuum exterior; non-emitting horizon; finite sky shell; observer remains outside horizon; no native FTD recovery':'Adopted Schwarzschild exterior; fixed floating emitting sphere; no native FTD recovery'}:{}),
        worldlineReason: observer.worldlineReason ?? 'legacy-unspecified', worldlineStart: observer.worldlineStart ?? null,
        observationTime: snapshot.time, observerPosition: [...observer.position], observerVelocity: [...observer.velocity], observerProperTime: observer.properTime,
        historyStart: snapshot.historyStart, availableInterval: [snapshot.historyStart, snapshot.time], historyStatus, opticalMode,
        capApplied: observer.capApplied === true || segment?.capApplied === true,
        hit: hit && segment ? {
            sourceId: hit.entityId, sourceRevision: hit.revision, segmentStart: segment.start ?? null, segmentEnd: segment.end ?? null,
            integratorVersion: segment.integratorVersion ?? 'legacy-unspecified',
            emissionTime: hit.emissionTime, emissionPosition: [...hit.sourcePosition], apparentPosition: [...hit.position],
            sourceProperTime: hit.properTime, dopplerFactor: hit.doppler, distance: hit.distance, mirrored: hit.mirrored,
        } : null,
    };
}
