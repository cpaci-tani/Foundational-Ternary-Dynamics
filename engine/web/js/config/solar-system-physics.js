/**
 * Scale 4 effective-physics helpers.
 *
 * These are standard astronomy approximations expressed in the Scale 4 unit
 * system (AU, solar masses, Julian years). They are [PARAMETRIC]/[IMPOSED]
 * effective models, not substrate-native FTD derivations and not a substitute
 * for a JPL ephemeris integrator.
 */

export const AU_METERS = 149_597_870_700;
export const AU_KM = AU_METERS / 1000;
export const JULIAN_YEAR_SECONDS = 31_557_600;
export const SOLAR_MASS_KG = 1.98847e30;
export const C_AU_PER_YEAR = 299_792_458 * JULIAN_YEAR_SECONDS / AU_METERS;
export const G_SI = 6.67430e-11;
export const SOLAR_LUMINOSITY_W = 3.828e26;
export const DEFAULT_SOLAR_MASS_LOSS_PER_YEAR = 9.0e-14;
export const SOLAR_WIND_TO_PR_DRAG = 0.35;

export const SOLAR_PHYSICS_DEFINITIONS = Object.freeze([
    { key: 'newtonianGravity', label: 'Newtonian N-body gravity', status: 'CORE', description: 'Direct pairwise gravity between every modeled massive body.' },
    { key: 'relativity1PN', label: 'Relativity · dominant-star 1PN', status: 'PARAMETRIC', requires: ['newtonianGravity'], description: 'First post-Newtonian Schwarzschild correction for a single-star system; not the full EIH N-body equations.' },
    { key: 'oblatenessJ2', label: 'Oblateness · J₂', status: 'PARAMETRIC', requires: ['newtonianGravity'], description: 'Axisymmetric quadrupole acceleration for bodies whose parent has a configured J₂.' },
    { key: 'equilibriumTides', label: 'Equilibrium tidal migration', status: 'PARAMETRIC', requires: ['newtonianGravity'], description: 'Constant-Q, near-circular secular tidal torque with an explicit spin angular-momentum reservoir.' },
    { key: 'radiationForces', label: 'Radiation pressure + P-R drag', status: 'PARAMETRIC', description: 'Leading-order radiation pressure and Poynting-Robertson drag summed from every luminous star.' },
    { key: 'solarWindDrag', label: 'Solar-wind drag correction', status: 'IMPOSED', requires: ['radiationForces'], description: 'A labeled 0.35× tangential correction to each luminous source\'s Poynting-Robertson drag.' },
    { key: 'stellarMassLoss', label: 'Stellar mass loss', status: 'PARAMETRIC', description: 'Slow isotropic stellar mass loss using a configured fractional rate per Julian year.' },
    { key: 'atmosphericDrag', label: 'Atmospheric entry drag', status: 'PARAMETRIC', description: 'Exponential, co-rotating atmosphere drag within configured atmosphere heights.' },
    { key: 'finiteBodyCollisions', label: 'Finite-body impacts', status: 'IMPOSED', description: 'Physical-radius contact with momentum-conserving perfectly inelastic merging.' },
    { key: 'rocheDisruption', label: 'Fluid Roche disruption', status: 'IMPOSED', description: 'Crossing the fluid Roche limit fragments eligible small bodies into bounded gravitating debris.' },
]);

export const DEFAULT_SOLAR_PHYSICS = Object.freeze(Object.fromEntries(
    SOLAR_PHYSICS_DEFINITIONS.map(({ key }) => [key, true]),
));

function safeRadius(x, y, z) {
    return Math.max(Math.hypot(x, y, z), 1e-15);
}

export function spinAxis(body) {
    const tilt = (Number(body?.axialTiltDeg) || 0) * Math.PI / 180;
    const azimuth = (Number(body?.poleAzimuthDeg) || 0) * Math.PI / 180;
    return {
        x: Math.sin(tilt) * Math.cos(azimuth),
        y: Math.sin(tilt) * Math.sin(azimuth),
        z: Math.cos(tilt),
    };
}

/**
 * Convert a catalog sidereal period into angular speed about the displayed
 * body pole. Catalog periods carry prograde/retrograde in their sign, while a
 * full obliquity above 90 degrees points the local pole into the opposite
 * orbital hemisphere. Combining both signs here prevents retrograde bodies
 * such as Venus and Uranus from being reversed twice by the renderer.
 */
export function siderealSpinRateRadYr(body) {
    const rotationDays = Number(body?.rotationDays);
    if (!Number.isFinite(rotationDays) || rotationDays === 0) return 0;
    const tilt = Number(body?.axialTiltDeg);
    const poleHemisphere = Number.isFinite(tilt) && Math.cos(tilt * Math.PI / 180) < 0 ? -1 : 1;
    const localSense = Math.sign(rotationDays) * poleHemisphere;
    return localSense * 2 * Math.PI * (JULIAN_YEAR_SECONDS / 86_400) / Math.abs(rotationDays);
}

/** Preserve the catalog prograde/retrograde sign after a local spin update. */
export function signedSiderealPeriodDays(body, spinRateRadYr) {
    if (!Number.isFinite(spinRateRadYr) || Math.abs(spinRateRadYr) <= 1e-15) return 0;
    const tilt = Number(body?.axialTiltDeg);
    const poleHemisphere = Number.isFinite(tilt) && Math.cos(tilt * Math.PI / 180) < 0 ? -1 : 1;
    const catalogSense = Math.sign(spinRateRadYr) * poleHemisphere;
    return catalogSense * 2 * Math.PI * (JULIAN_YEAR_SECONDS / 86_400) / Math.abs(spinRateRadYr);
}

/** Dominant-star Schwarzschild 1PN test-body acceleration. */
export function schwarzschild1PNAcceleration(r, v, mu, c = C_AU_PER_YEAR) {
    const radius = safeRadius(r.x, r.y, r.z);
    const r2 = radius * radius;
    const v2 = v.x * v.x + v.y * v.y + v.z * v.z;
    const rv = r.x * v.x + r.y * v.y + r.z * v.z;
    const scale = mu / (c * c * radius * r2);
    const common = 4 * mu / radius - v2;
    return {
        x: scale * (common * r.x + 4 * rv * v.x),
        y: scale * (common * r.y + 4 * rv * v.y),
        z: scale * (common * r.z + 4 * rv * v.z),
    };
}

/** Coordinate-free J2 acceleration around an axisymmetric parent. */
export function j2Acceleration(r, mu, parentRadiusAu, j2, axis = { x: 0, y: 0, z: 1 }) {
    const radius = safeRadius(r.x, r.y, r.z);
    const r2 = radius * radius;
    const projection = r.x * axis.x + r.y * axis.y + r.z * axis.z;
    const q = (projection * projection) / r2;
    const scale = 1.5 * j2 * mu * parentRadiusAu * parentRadiusAu / (radius * r2 * r2);
    return {
        x: scale * (r.x * (5 * q - 1) - 2 * projection * axis.x),
        y: scale * (r.y * (5 * q - 1) - 2 * projection * axis.y),
        z: scale * (r.z * (5 * q - 1) - 2 * projection * axis.z),
    };
}

export function radiationBeta(body, star) {
    const radiusM = Math.max(0, Number(body?.radiusKm) || 0) * 1000;
    const bodyMassKg = Math.max(0, Number(body?.mass) || Number(body?.massSolar) || 0) * SOLAR_MASS_KG;
    const starMassKg = Math.max(0, Number(star?.mass) || Number(star?.massSolar) || 0) * SOLAR_MASS_KG;
    const luminosityW = Math.max(0, Number(star?.luminosityW) || SOLAR_LUMINOSITY_W);
    const qPr = Math.max(0, Number(body?.radiationQpr) || 1);
    if (!(radiusM > 0 && bodyMassKg > 0 && starMassKg > 0 && luminosityW > 0)) return 0;
    return luminosityW * qPr * radiusM * radiusM
        / (4 * 299_792_458 * G_SI * starMassKg * bodyMassKg);
}

/** Leading O(v/c) radiation pressure plus Poynting-Robertson/solar-wind drag. */
export function radiationAcceleration(r, v, mu, beta, solarWindRatio = 0) {
    const radius = safeRadius(r.x, r.y, r.z);
    const invR = 1 / radius;
    const rx = r.x * invR, ry = r.y * invR, rz = r.z * invR;
    const radialVelocity = v.x * rx + v.y * ry + v.z * rz;
    const scale = beta * mu / (radius * radius);
    const prDragScale = 1 / C_AU_PER_YEAR;
    const windDragScale = Math.max(0, solarWindRatio) / C_AU_PER_YEAR;
    const pressure = { x: scale * rx, y: scale * ry, z: scale * rz };
    const prDrag = {
        x: -scale * prDragScale * (radialVelocity * rx + v.x),
        y: -scale * prDragScale * (radialVelocity * ry + v.y),
        z: -scale * prDragScale * (radialVelocity * rz + v.z),
    };
    const windDrag = {
        x: -scale * windDragScale * (radialVelocity * rx + v.x),
        y: -scale * windDragScale * (radialVelocity * ry + v.y),
        z: -scale * windDragScale * (radialVelocity * rz + v.z),
    };
    const drag = {
        x: prDrag.x + windDrag.x,
        y: prDrag.y + windDrag.y,
        z: prDrag.z + windDrag.z,
    };
    return {
        x: pressure.x + drag.x,
        y: pressure.y + drag.y,
        z: pressure.z + drag.z,
        pressure,
        prDrag,
        windDrag,
        drag,
    };
}

/** Constant-Q near-circular tidal torque represented as a tangential acceleration. */
export function equilibriumTideAcceleration(child, parent, r, v, G) {
    const k2 = Number(parent?.loveK2);
    const quality = Number(parent?.tidalQ);
    const parentMass = Number(parent?.mass);
    const childMass = Number(child?.mass);
    const parentRadius = Number(parent?.r);
    if (!(k2 > 0 && quality > 0 && parentMass > 0 && childMass > 0 && parentRadius > 0)) {
        return { x: 0, y: 0, z: 0, power: 0, migrationAuPerYear: 0, torque: 0 };
    }

    const radius = safeRadius(r.x, r.y, r.z);
    const axis = spinAxis(parent);
    const h = {
        x: r.y * v.z - r.z * v.y,
        y: r.z * v.x - r.x * v.z,
        z: r.x * v.y - r.y * v.x,
    };
    const orbitalRate = (h.x * axis.x + h.y * axis.y + h.z * axis.z) / (radius * radius);
    const rotationYears = Math.abs(Number(parent.rotationDays) || 0) / 365.25;
    if (!(rotationYears > 0)) return { x: 0, y: 0, z: 0, power: 0, migrationAuPerYear: 0, torque: 0 };
    const spinRate = Number.isFinite(parent.spinRateRadYr)
        ? parent.spinRateRadYr
        : siderealSpinRateRadYr(parent);
    const n = Math.sqrt(G * (parentMass + childMass) / (radius * radius * radius));
    const migrationMagnitude = 3 * k2 / quality * (childMass / parentMass)
        * Math.pow(parentRadius / radius, 5) * radius * n;
    const progradeTangent = {
        x: axis.y * r.z - axis.z * r.y,
        y: axis.z * r.x - axis.x * r.z,
        z: axis.x * r.y - axis.y * r.x,
    };
    const tangentNorm = safeRadius(progradeTangent.x, progradeTangent.y, progradeTangent.z);
    const direction = Math.sign(spinRate - orbitalRate) || 1;
    const magnitude = 0.5 * n * migrationMagnitude * direction;
    const acceleration = {
        x: magnitude * progradeTangent.x / tangentNorm,
        y: magnitude * progradeTangent.y / tangentNorm,
        z: magnitude * progradeTangent.z / tangentNorm,
    };
    // _applyRelativeAcceleration distributes this relative acceleration over
    // both bodies. The corresponding orbital force therefore uses the reduced
    // mass, which keeps the equal-and-opposite spin reservoir consistent for
    // finite parent/child mass ratios as well as the test-particle limit.
    const reducedMass = parentMass * childMass / (parentMass + childMass);
    const torque = reducedMass * radius * magnitude;
    const power = Math.abs(torque * (spinRate - orbitalRate));
    const orbitalWork = acceleration.x * v.x + acceleration.y * v.y + acceleration.z * v.z;
    const mu = G * (parentMass + childMass);
    return {
        ...acceleration,
        power,
        torque,
        migrationAuPerYear: mu > 0 ? 2 * radius * radius * orbitalWork / mu : 0,
    };
}

export function atmosphereDragAcceleration(child, parent, r, v, dtYears) {
    const rho0 = Number(parent?.atmosphereDensityKgM3);
    const scaleHeightKm = Number(parent?.atmosphereScaleHeightKm);
    const atmosphereHeightKm = Number(parent?.atmosphereHeightKm);
    const parentRadiusKm = Number(parent?.radiusKm);
    const childRadiusKm = Number(child?.radiusKm);
    const childMassKg = Number(child?.mass) * SOLAR_MASS_KG;
    if (!(rho0 > 0 && scaleHeightKm > 0 && atmosphereHeightKm > 0
        && parentRadiusKm > 0 && childRadiusKm > 0 && childMassKg > 0)) {
        return { x: 0, y: 0, z: 0, density: 0, altitudeKm: Infinity, power: 0 };
    }
    const radiusAu = safeRadius(r.x, r.y, r.z);
    const altitudeKm = radiusAu * AU_KM - parentRadiusKm;
    if (altitudeKm < 0 || altitudeKm > atmosphereHeightKm) {
        return { x: 0, y: 0, z: 0, density: 0, altitudeKm, power: 0 };
    }
    const axis = spinAxis(parent);
    const rotationYears = Math.abs(Number(parent.rotationDays) || 0) / 365.25;
    const omega = rotationYears > 0
        ? (Number.isFinite(parent.spinRateRadYr) ? parent.spinRateRadYr : siderealSpinRateRadYr(parent))
        : 0;
    const atmosphereVelocity = {
        x: omega * (axis.y * r.z - axis.z * r.y),
        y: omega * (axis.z * r.x - axis.x * r.z),
        z: omega * (axis.x * r.y - axis.y * r.x),
    };
    const wind = { x: v.x - atmosphereVelocity.x, y: v.y - atmosphereVelocity.y, z: v.z - atmosphereVelocity.z };
    const speedAuYr = safeRadius(wind.x, wind.y, wind.z);
    const speedMS = speedAuYr * AU_METERS / JULIAN_YEAR_SECONDS;
    const density = rho0 * Math.exp(-altitudeKm / scaleHeightKm);
    const areaM2 = Math.PI * Math.pow(childRadiusKm * 1000, 2);
    const accelerationMS2 = 0.5 * (Number(child.dragCoefficient) || 2.2) * density * speedMS * speedMS * areaM2 / childMassKg;
    let accelerationAuYr2 = accelerationMS2 * JULIAN_YEAR_SECONDS * JULIAN_YEAR_SECONDS / AU_METERS;
    // Entry can be much stiffer than orbital dynamics. Prevent one explicit
    // substep from reversing the relative wind while retaining dissipation.
    accelerationAuYr2 = Math.min(accelerationAuYr2, 0.25 * speedAuYr / Math.max(dtYears, 1e-15));
    const result = {
        x: -accelerationAuYr2 * wind.x / speedAuYr,
        y: -accelerationAuYr2 * wind.y / speedAuYr,
        z: -accelerationAuYr2 * wind.z / speedAuYr,
    };
    return {
        ...result,
        density,
        altitudeKm,
        power: child.mass * accelerationAuYr2 * speedAuYr,
    };
}

export function bodyDensityKgM3(body) {
    const radiusM = (Number(body?.radiusKm) || 0) * 1000;
    const massKg = (Number(body?.mass) || Number(body?.massSolar) || 0) * SOLAR_MASS_KG;
    if (!(radiusM > 0 && massKg > 0)) return 0;
    return massKg / ((4 / 3) * Math.PI * radiusM * radiusM * radiusM);
}

export function fluidRocheLimitAu(parent, child) {
    const parentDensity = bodyDensityKgM3(parent);
    const childDensity = bodyDensityKgM3(child);
    if (!(parentDensity > 0 && childDensity > 0 && parent?.r > 0)) return 0;
    return 2.44 * parent.r * Math.cbrt(parentDensity / childDensity);
}

/**
 * Instantaneous circular restricted-three-body Hill radius.
 *
 * This is a [PARAMETRIC] stability-domain approximation, not a finite cutoff
 * of gravity. It is only meaningful when the child is much less massive than
 * its parent; callers should suppress the surface for comparable-mass pairs.
 */
export function hillRadiusAu(child, parent) {
    const childMass = Number(child?.mass);
    const parentMass = Number(parent?.mass);
    if (!(childMass > 0 && parentMass > 0)) return 0;
    const separation = Math.hypot(
        Number(child?.x) - Number(parent?.x),
        Number(child?.y) - Number(parent?.y),
        Number(child?.z) - Number(parent?.z),
    );
    if (!(separation > 0) || !Number.isFinite(separation)) return 0;
    return separation * Math.cbrt(childMass / (3 * parentMass));
}
