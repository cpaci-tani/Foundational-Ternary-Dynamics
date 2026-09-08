/**
 * Catalog-backed example planetary systems for Scale 4.
 *
 * Distances are AU, masses are Earth masses, and radii are Earth radii in the
 * source records below. The bridge converts those quantities to its physical
 * AU / solar-mass gauge without any display enlargement or orbit compression.
 *
 * The NASA Exoplanet Archive does not provide visible-light surface colors for
 * these planets. `color` and `style` are therefore explicitly modeled cues,
 * inferred from broad radius class and catalog equilibrium temperature. They
 * are presentation metadata, not observations or FTD-derived predictions.
 */

export const EARTH_RADIUS_KM = 6371.0084;
export const EARTH_MASS_SOLAR = 3.0034896161241036e-6;

export const EXOPLANET_PROVENANCE = Object.freeze({
    source: 'NASA Exoplanet Archive Planetary Systems Composite Parameters (PSCompPars)',
    retrieved: '2026-09-04',
    epistemic: '[PARAMETRIC]',
    scale: 'Catalog semimajor axes and radii; rendered at 1 world unit = 1 AU with no body enlargement',
    appearance: 'Modeled color cue from radius class and equilibrium temperature; not observed true color',
    url: 'https://exoplanetarchive.ipac.caltech.edu/TAP/sync',
});

function appearanceFor(hostName, radiusEarth, equilibriumTemperatureK) {
    const temperature = Number(equilibriumTemperatureK);
    if (hostName === 'HR 8799') {
        return {
            className: 'Young self-luminous gas giant',
            appearanceClass: 'young-gas-giant',
            style: 15,
            color: temperature >= 1250 ? '#d97850' : '#c5684c',
        };
    }
    if (radiusEarth > 4) {
        return {
            className: 'Neptune-like exoplanet',
            appearanceClass: 'sub-neptune',
            style: 14,
            color: temperature >= 700 ? '#caa77b' : '#789db0',
        };
    }
    if (radiusEarth > 1.6) {
        return {
            className: 'Sub-Neptune exoplanet',
            appearanceClass: 'sub-neptune',
            style: 14,
            color: temperature >= 900 ? '#d6a06b'
                : temperature >= 700 ? '#c5ad82'
                    : temperature >= 500 ? '#8eb3ad' : '#7896b4',
        };
    }
    return {
        className: 'Rocky exoplanet',
        appearanceClass: 'rocky',
        style: 13,
        color: temperature >= 350 ? '#b85b40'
            : temperature >= 250 ? '#9a735c'
                : temperature >= 210 ? '#877d73' : '#82939c',
    };
}

function planet(hostName, name, radiusEarth, massEarth, semiMajorAxisAu, eccentricity, periodDays, equilibriumTemperatureK) {
    const appearance = appearanceFor(hostName, radiusEarth, equilibriumTemperatureK);
    const modeledEccentricity = Number.isFinite(eccentricity) ? eccentricity : 0;
    return Object.freeze({
        name,
        radiusEarth,
        massEarth,
        semiMajorAxisAu,
        eccentricity: modeledEccentricity,
        catalogEccentricity: eccentricity,
        eccentricityBasis: Number.isFinite(eccentricity)
            ? 'NASA Exoplanet Archive composite value'
            : '[IMPOSED] circular initialization because the catalog composite value is unavailable',
        periodDays,
        equilibriumTemperatureK,
        radiusKm: radiusEarth * EARTH_RADIUS_KM,
        massSolar: massEarth * EARTH_MASS_SOLAR,
        colorBasis: 'modeled from radius class and equilibrium temperature; not observed true color',
        ...appearance,

        // Compatibility aliases for older catalog consumers.
        pl_name: name,
        pl_rade: radiusEarth,
        pl_masse: massEarth,
        pl_orbsmax: semiMajorAxisAu,
        pl_orbeccen: eccentricity,
        pl_orbper: periodDays,
        pl_eqt: equilibriumTemperatureK,
        mass_sol: massEarth * EARTH_MASS_SOLAR,
    });
}

function system(host, planets) {
    return Object.freeze({
        host: Object.freeze(host),
        planets: Object.freeze(planets),
        provenance: EXOPLANET_PROVENANCE,
    });
}

export const EXOPLANET_SYSTEMS = Object.freeze({
    'TRAPPIST-1': system({
        name: 'TRAPPIST-1', key: 'trappist-1', massSolar: 0.0898, radiusSolar: 0.1192,
        temperatureK: 2566, spectralType: 'M8.0 V', color: '#ff9b69',
    }, [
        planet('TRAPPIST-1', 'TRAPPIST-1 b', 1.116, 1.374, 0.01154, 0.00622, 1.510826, 397.6),
        planet('TRAPPIST-1', 'TRAPPIST-1 c', 1.097, 1.308, 0.0158, 0.00654, 2.421937, 339.7),
        planet('TRAPPIST-1', 'TRAPPIST-1 d', 0.788, 0.388, 0.02227, 0.00837, 4.049219, 286.2),
        planet('TRAPPIST-1', 'TRAPPIST-1 e', 0.92, 0.692, 0.02925, 0.0051, 6.101013, 249.7),
        planet('TRAPPIST-1', 'TRAPPIST-1 f', 1.045, 1.039, 0.03849, 0.01007, 9.20754, 217.7),
        planet('TRAPPIST-1', 'TRAPPIST-1 g', 1.129, 1.321, 0.04683, 0.00208, 12.352446, 197.3),
        planet('TRAPPIST-1', 'TRAPPIST-1 h', 0.755, 0.326, 0.06189, 0.00567, 18.772866, 171.7),
    ]),
    'Kepler-11': system({
        name: 'Kepler-11', key: 'kepler-11', massSolar: 0.961, radiusSolar: 1.065,
        temperatureK: 5663, spectralType: 'G-type', color: '#fff2df',
    }, [
        planet('Kepler-11', 'Kepler-11 b', 1.8, 1.9, 0.091, 0.045, 10.3039, 849),
        planet('Kepler-11', 'Kepler-11 c', 2.87, 2.9, 0.107, 0.026, 13.0241, 786),
        planet('Kepler-11', 'Kepler-11 d', 3.12, 7.3, 0.155, 0.004, 22.6845, 653),
        planet('Kepler-11', 'Kepler-11 e', 4.19, 8.0, 0.195, 0.012, 31.9996, 582),
        planet('Kepler-11', 'Kepler-11 f', 2.49, 2.0, 0.25, 0.013, 46.6888, 513),
        planet('Kepler-11', 'Kepler-11 g', 3.33, 25.0, 0.466, 0.15, 118.3807, 376),
    ]),
    'HR 8799': system({
        name: 'HR 8799', key: 'hr-8799', massSolar: 1.5, radiusSolar: 1.49338,
        temperatureK: 7204.58, spectralType: 'A5 V', color: '#dfe9ff',
    }, [
        planet('HR 8799', 'HR 8799 e', 13.11453, 3178.3, 16.4, 0.15, 20815.6, 1150),
        planet('HR 8799', 'HR 8799 d', 13.0, 3000.0, 24.0, 0.6, 37000, 1300),
        planet('HR 8799', 'HR 8799 c', 13.0, 3000.0, 38.0, 0.5, 69000, 1200),
        planet('HR 8799', 'HR 8799 b', 13.0, 2000.0, 68.0, null, 170000, 1200),
    ]),
    'Kepler-20': system({
        name: 'Kepler-20', key: 'kepler-20', massSolar: 0.929, radiusSolar: 0.9164,
        temperatureK: 5495, spectralType: 'G-type', color: '#ffeedc',
    }, [
        planet('Kepler-20', 'Kepler-20 b', 1.773, 9.7, 0.04565, 0.083, 3.6961049, 1187),
        planet('Kepler-20', 'Kepler-20 e', 0.821, 0.76, 0.0637, 0.092, 6.0984882, 1004),
        planet('Kepler-20', 'Kepler-20 c', 2.894, 11.1, 0.0936, 0.076, 10.8540774, 828),
        planet('Kepler-20', 'Kepler-20 f', 0.952, 1.4, 0.1387, 0.094, 19.578328, 681),
        planet('Kepler-20', 'Kepler-20 g', 4.71, 19.96, 0.2055, 0.15, 34.94, 524),
        planet('Kepler-20', 'Kepler-20 d', 2.606, 13.4, 0.3474, 0.082, 77.611455, 430),
    ]),
});

// Retained export shape for dropdown/catalog parity checks.
export const EXOPLANET_SEEDS = Object.freeze(Object.fromEntries(
    Object.entries(EXOPLANET_SYSTEMS).map(([name, entry]) => [name, entry.planets]),
));
