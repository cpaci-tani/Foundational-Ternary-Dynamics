/**
 * Bound-orbit period proxy from 2-body separation history: record the
 * starting separation, then report the tick delta the first time
 * separation returns within `tolerancePct` of that starting value. This is
 * a return-to-start proxy, not a Kepler fit — it reads "—" (null) for
 * trajectories that never return (escaping, hyperbolic, or too short a
 * history to judge).
 *
 * @param {Array<{tick:number, separation:number}>} history - ordered oldest-first
 * @param {number} [tolerancePct=0.05] - fractional tolerance for "returned"
 * @returns {number|null} tick delta, or null if undetermined
 */
export function estimateOrbitPeriod(history, tolerancePct = 0.05) {
    if (!Array.isArray(history) || history.length < 2) return null;
    const start = history[0];
    const tol = Math.abs(start.separation) * tolerancePct;
    // Start at i=1 (not 0) so the start sample is never compared to itself;
    // dTick<=0 guards any out-of-order/duplicate-tick entries.
    for (let i = 1; i < history.length; i++) {
        const dTick = history[i].tick - start.tick;
        if (dTick <= 0) continue;
        if (Math.abs(history[i].separation - start.separation) <= tol) {
            return dTick;
        }
    }
    return null;
}
