// @ts-check
/** Read-only native parity reductions with explicit numerical qualifications.
 * New servers visit all sites in Float64. The legacy stride-one sampler omits
 * |J|² < 1e-30 and uses Float32; that fallback retains its omission bounds.
 */
export const FLUX_SECTOR_LIMITS = Object.freeze({ maxSize: 64, nativeMaxSize: 256, squaredNormFloor: 1e-30,
    relativeRoundingAllowance: 1e-6, timeoutMs: 10000, nativeTimeoutMs: 30000 });

/** Validate the complete native reduction; never infer unvisited sites or zeros.
 * @param {any} value @param {number} size
 */
export function normalizeNativeFluxSectors(value, size) {
    if (!Number.isInteger(size) || size < 4 || size > FLUX_SECTOR_LIMITS.nativeMaxSize
        || value?.type !== 'flux_sectors' || value.schemaVersion !== 1
        || !['GPU', 'CPU'].includes(value.compute) || value.latticeSize !== size || value.siteCount !== size ** 3
        || value.nonfiniteValueCount !== 0 || value.relativeRoundingAllowance !== 1e-6
        || !Array.isArray(value.sectors) || value.sectors.length !== 2)
        throw new Error('Invalid or non-finite native full-domain flux-sector reduction.');
    const sectors = value.sectors.map((/** @type {any} */ sector, /** @type {number} */ parity) => {
        const totalSites = Math.floor(size ** 3 / 2) + (parity === 0 && size % 2 ? 1 : 0);
        if (sector.parity !== parity || sector.totalSites !== totalSites
            || !Number.isSafeInteger(sector.nonzeroSites) || sector.nonzeroSites < 0 || sector.nonzeroSites > totalSites
            || !Number.isFinite(sector.squaredNorm) || sector.squaredNorm < 0
            || !Number.isFinite(sector.maxAbsComponent) || sector.maxAbsComponent < 0
            || (sector.nonzeroSites === 0 && (sector.squaredNorm !== 0 || sector.maxAbsComponent !== 0))
            || (sector.nonzeroSites > 0 && sector.maxAbsComponent === 0))
            throw new Error('Invalid native parity-sector counts or norm.');
        // Include a conservative normal-minimum allowance for squared-value
        // underflow/FTZ. Positive source components can have a rounded zero norm.
        const absoluteUnderflowAllowance = 3 * totalSites * 2 ** -1022;
        const allowance = sector.squaredNorm * FLUX_SECTOR_LIMITS.relativeRoundingAllowance + absoluteUnderflowAllowance;
        return { parity, totalSites, sampledSites: totalSites, nonzeroSites: sector.nonzeroSites,
            observedSquaredNorm: sector.squaredNorm, maxAbsComponent: sector.maxAbsComponent,
            omittedSites: 0, omittedSquaredNormUpperBound: 0, absoluteUnderflowAllowance,
            squaredNormBounds: { lower: Math.max(0, sector.squaredNorm - allowance), upper: sector.squaredNorm + allowance } };
    });
    const lower = sectors.reduce((/** @type {number} */ n, /** @type {any} */ s) => n + s.squaredNormBounds.lower, 0);
    const upper = sectors.reduce((/** @type {number} */ n, /** @type {any} */ s) => n + s.squaredNormBounds.upper, 0);
    if (!Number.isFinite(upper)) throw new Error('Native aggregate norm overflowed.');
    return { latticeSize: size, siteCount: size ** 3, sampledSites: size ** 3,
        observedSquaredNorm: sectors.reduce((/** @type {number} */ n, /** @type {any} */ s) => n + s.observedSquaredNorm, 0),
        squaredNormBounds: { lower, upper }, sectors: sectors.map((/** @type {any} */ s) => ({ ...s,
            fractionBounds: { lower: upper > 0 ? s.squaredNormBounds.lower / upper : null,
                upper: lower > 0 ? Math.min(1, s.squaredNormBounds.upper / lower) : null } })),
        maxAbsComponent: Math.max(...sectors.map((/** @type {any} */ s) => s.maxAbsComponent)),
        sampling: { kind: 'nativeAggregate', domain: 'full cube', selection: 'all sites; no threshold',
            squaredNormFloor: 0, effectiveStride: 1, origin: 0, compute: value.compute,
            encoding: 'IEEE-754 Float64 native aggregate, round-trip JSON', accumulation: 'Float64 native reduction',
            relativeRoundingAllowance: FLUX_SECTOR_LIMITS.relativeRoundingAllowance,
            absoluteUnderflowAllowance: 3 * size ** 3 * 2 ** -1022,
            qualification: 'All sites visited without a display threshold. Bounds include conservative reduction and underflow allowances, not integration/model error. A rounded zero norm alone is not proof of zero components.' },
        units: 'effective lattice flux squared; not a conserved energy or physical identification' };
}

/** @param {any} sample @param {number} size */
export function reduceFluxSectors(sample, size) {
    if (!Number.isInteger(size) || size < 4 || size > FLUX_SECTOR_LIMITS.maxSize)
        throw new Error('Full-resolution flux-sector measurement requires lattice size 4..64.');
    if (sample?.kind !== 'fluxVector' || sample.effectiveStride !== 1 || sample.origin !== 0)
        throw new Error('Flux-sector measurement requires a full-domain stride-one FluxVector sample.');
    const { positions, vectors, count } = sample;
    if (!(positions instanceof Float32Array) || !(vectors instanceof Float32Array)
        || !Number.isSafeInteger(count) || count < 0 || count > size ** 3
        || positions.length !== count * 3 || vectors.length !== count * 3)
        throw new Error('Invalid or truncated native Float32 field sample.');
    const seen = new Uint8Array(size ** 3);
    const sectors = [0, 1].map(parity => ({ parity, totalSites: Math.floor(size ** 3 / 2)
        + (parity === 0 && size % 2 ? 1 : 0), sampledSites: 0,
        observedSquaredNorm: 0, maxAbsComponent: 0 }));
    // Kahan accumulation keeps arithmetic error comfortably below the declared
    // conservative Float32-to-norm allowance (not an engine-integration bound).
    const corrections = [0, 0];
    for (let i = 0; i < count; i++) {
        const p = i * 3;
        const x = positions[p] - 0.5, y = positions[p + 1] - 0.5, z = positions[p + 2] - 0.5;
        if (![x, y, z].every(v => Number.isInteger(v) && v >= 0 && v < size))
            throw new Error('Native field positions must be unique voxel centers throughout the full domain.');
        const index = x + size * (y + size * z);
        if (seen[index]) throw new Error('Duplicate site in native field sample.');
        seen[index] = 1;
        const values = [vectors[p], vectors[p + 1], vectors[p + 2]];
        if (!values.every(Number.isFinite)) throw new Error('Non-finite native flux value.');
        const norm = values.reduce((sum, v) => sum + v * v, 0), parity = (x + y + z) % 2;
        if (norm < FLUX_SECTOR_LIMITS.squaredNormFloor * (1 - FLUX_SECTOR_LIMITS.relativeRoundingAllowance))
            throw new Error('Native field sample violates its documented support floor.');
        const sector = sectors[parity], adjusted = norm - corrections[parity];
        const sum = sector.observedSquaredNorm + adjusted;
        corrections[parity] = (sum - sector.observedSquaredNorm) - adjusted;
        sector.observedSquaredNorm = sum;
        sector.sampledSites++;
        sector.maxAbsComponent = Math.max(sector.maxAbsComponent, ...values.map(Math.abs));
    }
    const result = sectors.map(sector => {
        const omittedSites = sector.totalSites - sector.sampledSites;
        const omittedSquaredNormUpperBound = omittedSites * FLUX_SECTOR_LIMITS.squaredNormFloor;
        const allowance = sector.observedSquaredNorm * FLUX_SECTOR_LIMITS.relativeRoundingAllowance;
        return { ...sector, omittedSites, omittedSquaredNormUpperBound,
            squaredNormBounds: { lower: Math.max(0, sector.observedSquaredNorm - allowance),
                upper: sector.observedSquaredNorm + allowance + omittedSquaredNormUpperBound } };
    });
    const totalLower = result.reduce((sum, s) => sum + s.squaredNormBounds.lower, 0);
    const totalUpper = result.reduce((sum, s) => sum + s.squaredNormBounds.upper, 0);
    return { latticeSize: size, siteCount: size ** 3, sampledSites: count,
        observedSquaredNorm: result.reduce((sum, s) => sum + s.observedSquaredNorm, 0),
        squaredNormBounds: { lower: totalLower, upper: totalUpper },
        sectors: result.map(sector => ({ ...sector, fractionBounds: {
            lower: totalUpper > 0 ? sector.squaredNormBounds.lower / totalUpper : null,
            upper: totalLower > 0 ? Math.min(1, sector.squaredNormBounds.upper / totalLower) : null } })),
        maxAbsComponent: Math.max(...result.map(sector => sector.maxAbsComponent)),
        sampling: { kind: 'fluxVector', effectiveStride: 1, origin: 0, domain: 'full cube',
            selection: 'squared flux norm >= 1e-30', squaredNormFloor: FLUX_SECTOR_LIMITS.squaredNormFloor,
            encoding: 'IEEE-754 Float32 vectors and voxel-center positions', accumulation: 'Float64 compensated sum',
            relativeRoundingAllowance: FLUX_SECTOR_LIMITS.relativeRoundingAllowance,
            qualification: 'Bounds include sampler omissions and Float32 conversion only; omitted sites are not exact zeros. No integration-error or physical-certification claim.' },
        units: 'effective lattice flux squared; not a conserved energy or physical identification' };
}
