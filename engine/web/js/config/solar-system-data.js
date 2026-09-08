/**
 * Solar System reference data used by Scale 4.
 *
 * Dynamics use AU, solar masses, and Julian years. Planetary orbital elements
 * are the JPL Solar System Dynamics approximate J2000 elements (1800-2050
 * table). Planetary `radiusKm` values are the JPL volume-equivalent mean radii;
 * the renderer converts them with the exact IAU astronomical unit below and
 * never applies a display multiplier. They are imported astrophysical data,
 * not FTD derivations.
 */

export const KM_PER_AU = 149_597_870.7;
export const DAYS_PER_YEAR = 365.25;

export const SOLAR_SYSTEM_PLANETS = Object.freeze([
    {
        key: 'mercury', name: 'Mercury', className: 'Terrestrial planet', style: 1,
        massSolar: 1.66012e-7, radiusKm: 2439.4, temperatureK: 440,
        rotationDays: 58.6462, axialTiltDeg: 0.034, moonsKnown: 0,
        j2: 6.0e-5, loveK2: 0.45, tidalQ: 190, momentFactor: 0.346,
        atmosphere: 'Exosphere', magneticField: 'Weak global field',
        color: '#9c9489',
        orbit: { a: 0.38709927, e: 0.20563593, i: 7.00497902, L: 252.25032350, longPeri: 77.45779628, longNode: 48.33076593 },
    },
    {
        key: 'venus', name: 'Venus', className: 'Terrestrial planet', style: 2,
        massSolar: 2.44784e-6, radiusKm: 6051.8, temperatureK: 737,
        rotationDays: -243.018, axialTiltDeg: 177.36, moonsKnown: 0,
        j2: 4.458e-6, loveK2: 0.295, tidalQ: 100, momentFactor: 0.337,
        atmosphereDensityKgM3: 65, atmosphereScaleHeightKm: 15.9, atmosphereHeightKm: 250,
        atmosphere: 'Dense CO₂ and sulfuric-acid clouds', magneticField: 'No intrinsic global field',
        color: '#d8b36a',
        orbit: { a: 0.72333566, e: 0.00677672, i: 3.39467605, L: 181.97909950, longPeri: 131.60246718, longNode: 76.67984255 },
    },
    {
        key: 'earth', name: 'Earth', className: 'Terrestrial planet', style: 3,
        massSolar: 3.00349e-6, radiusKm: 6371.0084, temperatureK: 288,
        rotationDays: 0.99726968, axialTiltDeg: 23.439, moonsKnown: 1,
        j2: 1.08262668e-3, loveK2: 0.299, tidalQ: 12, momentFactor: 0.3308,
        atmosphereDensityKgM3: 1.225, atmosphereScaleHeightKm: 8.5, atmosphereHeightKm: 100,
        atmosphere: 'N₂/O₂ atmosphere; liquid-water hydrosphere', magneticField: 'Global dynamo field',
        orbitReference: 'Earth-Moon barycenter',
        color: '#3d82d7',
        orbit: { a: 1.00000261, e: 0.01671123, i: -0.00001531, L: 100.46457166, longPeri: 102.93768193, longNode: 0.0 },
    },
    {
        key: 'mars', name: 'Mars', className: 'Terrestrial planet', style: 4,
        massSolar: 3.22716e-7, radiusKm: 3389.5, temperatureK: 210,
        rotationDays: 1.02595676, axialTiltDeg: 25.19, moonsKnown: 2,
        j2: 1.96045e-3, loveK2: 0.152, tidalQ: 100, momentFactor: 0.366,
        atmosphereDensityKgM3: 0.020, atmosphereScaleHeightKm: 11.1, atmosphereHeightKm: 120,
        atmosphere: 'Thin CO₂ atmosphere', magneticField: 'Crustal remanent fields',
        color: '#b75f3a',
        orbit: { a: 1.52371034, e: 0.09339410, i: 1.84969142, L: -4.55343205, longPeri: -23.94362959, longNode: 49.55953891 },
    },
    {
        key: 'jupiter', name: 'Jupiter', className: 'Gas giant', style: 5,
        massSolar: 9.54588e-4, radiusKm: 69_911, temperatureK: 165,
        rotationDays: 0.41354, axialTiltDeg: 3.13, moonsKnown: 115,
        j2: 0.01469643, loveK2: 0.565, tidalQ: 100_000, momentFactor: 0.254,
        atmosphereDensityKgM3: 0.16, atmosphereScaleHeightKm: 27, atmosphereHeightKm: 1000,
        atmosphere: 'Hydrogen/helium with ammonia cloud decks', magneticField: 'Strong global magnetosphere',
        color: '#d8aa7a',
        orbit: { a: 5.20288700, e: 0.04838624, i: 1.30439695, L: 34.39644051, longPeri: 14.72847983, longNode: 100.47390909 },
    },
    {
        key: 'saturn', name: 'Saturn', className: 'Gas giant', style: 6,
        massSolar: 2.85815e-4, radiusKm: 58_232, temperatureK: 134,
        rotationDays: 0.44401, axialTiltDeg: 26.73, moonsKnown: 293,
        j2: 0.01629071, loveK2: 0.39, tidalQ: 1800, momentFactor: 0.22,
        atmosphereDensityKgM3: 0.19, atmosphereScaleHeightKm: 59.5, atmosphereHeightKm: 1500,
        atmosphere: 'Hydrogen/helium with ammonia clouds', magneticField: 'Global magnetosphere',
        color: '#d8c28d', rings: { innerRadius: 1.11, outerRadius: 2.42, opacity: 0.78 },
        orbit: { a: 9.53667594, e: 0.05386179, i: 2.48599187, L: 49.95424423, longPeri: 92.59887831, longNode: 113.66242448 },
    },
    {
        key: 'uranus', name: 'Uranus', className: 'Ice giant', style: 7,
        massSolar: 4.36624e-5, radiusKm: 25_362, temperatureK: 76,
        rotationDays: -0.71833, axialTiltDeg: 97.77, moonsKnown: 29,
        j2: 0.00334129, loveK2: 0.104, tidalQ: 10_000, momentFactor: 0.23,
        atmosphereDensityKgM3: 0.42, atmosphereScaleHeightKm: 27.7, atmosphereHeightKm: 1000,
        atmosphere: 'Hydrogen/helium/methane', magneticField: 'Strongly tilted, offset field',
        color: '#83d5dc', rings: { innerRadius: 1.55, outerRadius: 2.0, opacity: 0.36 },
        orbit: { a: 19.18916464, e: 0.04725744, i: 0.77263783, L: 313.23810451, longPeri: 170.95427630, longNode: 74.01692503 },
    },
    {
        key: 'neptune', name: 'Neptune', className: 'Ice giant', style: 8,
        massSolar: 5.15139e-5, radiusKm: 24_622, temperatureK: 72,
        rotationDays: 0.67125, axialTiltDeg: 28.32, moonsKnown: 16,
        j2: 0.00340843, loveK2: 0.127, tidalQ: 10_000, momentFactor: 0.23,
        atmosphereDensityKgM3: 0.45, atmosphereScaleHeightKm: 19.7, atmosphereHeightKm: 1000,
        atmosphere: 'Hydrogen/helium/methane', magneticField: 'Tilted, offset field',
        color: '#356dc4',
        orbit: { a: 30.06992276, e: 0.00859048, i: 1.77004347, L: -55.12002969, longPeri: 44.96476227, longNode: 131.78422574 },
    },
]);

// A compact major-moon set keeps the default system legible while allowing
// real nested two-body motion. Orbital phases are presentation choices; masses,
// radii, semimajor axes, eccentricities, and periods are reference inputs.
export const SOLAR_SYSTEM_MOONS = Object.freeze([
    { key: 'moon', name: 'Moon', parent: 'earth', style: 9, massSolar: 3.69430e-8, radiusKm: 1737.4, aKm: 384_400, e: 0.0549, i: 5.145, periodDays: 27.321661, phaseDeg: 125.1, temperatureK: 220, color: '#b8b8b2', className: 'Rocky moon' },
    { key: 'io', name: 'Io', parent: 'jupiter', style: 11, massSolar: 4.49190e-8, radiusKm: 1821.6, aKm: 421_700, e: 0.0041, i: 0.05, periodDays: 1.769138, phaseDeg: 210, temperatureK: 110, color: '#d8c45e', className: 'Volcanic moon' },
    { key: 'europa', name: 'Europa', parent: 'jupiter', style: 10, massSolar: 2.40915e-8, radiusKm: 1560.8, aKm: 671_034, e: 0.0094, i: 0.47, periodDays: 3.551181, phaseDeg: 15, temperatureK: 102, color: '#d7c9a9', className: 'Icy moon' },
    { key: 'ganymede', name: 'Ganymede', parent: 'jupiter', style: 9, massSolar: 7.45062e-8, radiusKm: 2634.1, aKm: 1_070_412, e: 0.0013, i: 0.20, periodDays: 7.154553, phaseDeg: 95, temperatureK: 110, color: '#9e8d78', className: 'Icy-rocky moon' },
    { key: 'callisto', name: 'Callisto', parent: 'jupiter', style: 9, massSolar: 5.42635e-8, radiusKm: 2410.3, aKm: 1_882_709, e: 0.0074, i: 0.28, periodDays: 16.689018, phaseDeg: 300, temperatureK: 134, color: '#74675e', className: 'Icy-rocky moon' },
    { key: 'enceladus', name: 'Enceladus', parent: 'saturn', style: 10, massSolar: 5.43e-11, radiusKm: 252.1, aKm: 237_948, e: 0.0047, i: 0.01, periodDays: 1.370218, phaseDeg: 45, temperatureK: 75, color: '#e6eef3', className: 'Icy moon' },
    { key: 'titan', name: 'Titan', parent: 'saturn', style: 12, massSolar: 6.764e-8, radiusKm: 2574.7, aKm: 1_221_870, e: 0.0288, i: 0.35, periodDays: 15.945421, phaseDeg: 170, temperatureK: 94, color: '#d69b42', className: 'Atmospheric moon', atmosphereDensityKgM3: 5.3, atmosphereScaleHeightKm: 40, atmosphereHeightKm: 600 },
    { key: 'titania', name: 'Titania', parent: 'uranus', style: 10, massSolar: 1.769e-9, radiusKm: 788.9, aKm: 435_910, e: 0.0011, i: 0.08, periodDays: 8.705872, phaseDeg: 255, temperatureK: 70, color: '#b7c2c7', className: 'Icy moon' },
    { key: 'triton', name: 'Triton', parent: 'neptune', style: 10, massSolar: 1.080e-8, radiusKm: 1353.4, aKm: 354_759, e: 0.000016, i: 156.865, periodDays: -5.876854, phaseDeg: 25, temperatureK: 38, color: '#c4b9ae', className: 'Retrograde icy moon' },
]);

export const SOLAR_SYSTEM_DWARFS = Object.freeze([
    {
        key: 'pluto', name: 'Pluto', className: 'Dwarf planet', style: 12,
        massSolar: 6.55e-9, radiusKm: 1188.3, temperatureK: 44,
        rotationDays: -6.3872, axialTiltDeg: 122.53, moonsKnown: 5,
        j2: 0, loveK2: 0.058, tidalQ: 100, momentFactor: 0.31,
        atmosphere: 'Seasonal N₂/CH₄/CO atmosphere', magneticField: 'No known global field',
        color: '#b59c87',
        orbit: { a: 39.482, e: 0.2488, i: 17.16, L: 238.93, longPeri: 224.07, longNode: 110.30 },
    },
]);

export const SUN_REFERENCE = Object.freeze({
    key: 'sun', name: 'Sun', className: 'G2 V main-sequence star', style: 0,
    massSolar: 1.0, radiusKm: 695_700, temperatureK: 5772,
    rotationDays: 25.38, axialTiltDeg: 7.25, moonsKnown: 0,
    luminosityW: 3.828e26, massLossPerYear: 9.0e-14,
    j2: 2.2e-7, loveK2: 0.03, tidalQ: 1_000_000, momentFactor: 0.07,
    atmosphere: 'Photosphere, chromosphere, transition region, corona',
    magneticField: 'Global dynamo; approximately 11-year activity cycle',
    color: '#ffd36a',
});

export const SOLAR_SYSTEM_PROVENANCE = Object.freeze({
    epoch: 'J2000.0',
    dynamics: 'Newtonian N-body plus toggle-gated effective perturbations, split-kick Verlet',
    units: 'AU · M☉ · Julian yr',
    orbitalSource: 'JPL approximate planetary elements (1800-2050 table)',
    physicalSource: 'JPL planetary and satellite physical parameters',
    rotationSource: 'JPL sidereal rotation periods; signed values denote retrograde rotation',
    rotationPhase: 'Relative phase zero at scenario load; rates are period-faithful, not prime-meridian ephemerides',
    renderingScale: '1 world unit = 1 AU; body surface radius = mean radius / AU',
    perturbationSource: 'Standard 1PN, J2, constant-Q tide, radiation/drag, collision, and Roche approximations',
    epistemic: '[PARAMETRIC] imported astronomy; not an FTD derivation',
});
