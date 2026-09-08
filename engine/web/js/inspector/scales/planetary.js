import { setInspectorSectionVisibility } from '../chrome.js';

const EARTH_MASS_SOLAR = 3.00349e-6;
const AU_PER_YEAR_TO_KM_PER_SECOND = 4.740470463;
const AU_METERS = 149_597_870_700;

// Backward-compatible helper for generic exoplanet scenarios. The default
// Solar System inspector uses imported body metadata instead of pretending a
// distance-only heuristic is a physical surface or biome model.
export function classifyBiome(d) {
    if (d < 0.5) return { uTemp: 1.0, biome: 'Lava World' };
    if (d > 2.0) return { uTemp: -1.0, biome: 'Ice World' };
    return { uTemp: (1.25 - d), biome: 'Temperate Earthlike' };
}

export function handlePlanetaryClick(target, intersects) {
    if (intersects.length > 0) {
        const mesh = intersects[0].object;
        target.selectPlanetaryBody?.(mesh.userData.id);
        return;
    }
    target.clearSelection?.();
}

export function showPlanetaryInspector(target) {
    setInspectorSectionVisibility(target.planetaryEmptyEl, target.planetaryContentEl, true);
    updatePlanetaryFields(target);
    target._updateInspectorChrome();
}

export function hidePlanetaryInspector(target) {
    setInspectorSectionVisibility(target.planetaryEmptyEl, target.planetaryContentEl, false);
    target._updateInspectorChrome();
}

export function updatePlanetaryFields(target) {
    if (target._selectedPlanetaryId === -1 || !target.bridge) return;

    const data = target.bridge.getPlanetaryData();
    if (!data || !data.buffer) return;

    let index = -1;
    for (let i = 0; i < data.count; i++) {
        if (data.buffer[i * 16 + 6] === target._selectedPlanetaryId) {
            index = i;
            break;
        }
    }
    if (index === -1) {
        hidePlanetaryInspector(target);
        return;
    }

    const body = data.bodies?.[index] || target.bridge.getBody?.(target._selectedPlanetaryId) || {};
    const off = index * 16;
    const x = data.buffer[off + 0];
    const y = data.buffer[off + 1];
    const z = data.buffer[off + 2];
    const type = data.buffer[off + 3];
    const mass = data.buffer[off + 4];
    const vx = data.buffer[off + 8];
    const vy = data.buffer[off + 9];
    const vz = data.buffer[off + 10];
    const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);

    const parent = data.bodies?.find((candidate) => candidate.id === body.parentId) || null;
    const dx = x - (parent?.x || 0);
    const dy = y - (parent?.y || 0);
    const dz = z - (parent?.z || 0);
    const d = Math.sqrt(dx * dx + dy * dy + dz * dz);
    const fallbackClass = type === 0 ? 'Host star'
        : type === 2 ? 'Giant planet'
            : type === 3 ? 'Natural satellite'
                : type === 5 ? 'Dwarf planet' : 'Terrestrial planet';
    const displayName = body.name || fallbackClass;
    const bodyClass = body.className || fallbackClass;
    const temp = Number(body.temperatureK);
    const radiusKm = Number(body.radiusKm);
    const rotationDays = Number(body.rotationDays);
    const earthMasses = mass / EARTH_MASS_SOLAR;

    target.planetaryFields.id.textContent = target._selectedPlanetaryId;
    target.planetaryFields.type.textContent = `${displayName} · ${bodyClass}`;
    target.planetaryFields.dot.style.background = body.color || (type === 0 ? '#facc15' : '#60a5fa');
    target.planetaryFields.mass.textContent = type === 0
        ? `${mass.toPrecision(7)} M☉`
        : `${mass.toExponential(6)} M☉ · ${earthMasses.toPrecision(6)} M⊕`;
    target.planetaryFields.temp.textContent = Number.isFinite(temp) ? `${temp.toLocaleString()} K` : '—';
    target.planetaryFields.biome.textContent = bodyClass;
    target.planetaryFields.pos.textContent = `(${x.toFixed(9)}, ${y.toFixed(9)}, ${z.toFixed(9)}) AU`;
    target.planetaryFields.vel.textContent = `(${vx.toFixed(5)}, ${vy.toFixed(5)}, ${vz.toFixed(5)}) AU/yr`;
    target.planetaryFields.speed.textContent = `${speed.toFixed(6)} AU/yr · ${(speed * AU_PER_YEAR_TO_KM_PER_SECOND).toFixed(3)} km/s`;
    target.planetaryFields.radius.textContent = Number.isFinite(radiusKm)
        ? `${radiusKm.toLocaleString(undefined, { maximumFractionDigits: 4 })} km · ${(radiusKm / (AU_METERS / 1000)).toExponential(9)} AU`
        : '—';
    target.planetaryFields.tilt.textContent = Number.isFinite(body.axialTiltDeg) ? `${body.axialTiltDeg.toFixed(3)}°` : '—';
    target.planetaryFields.day.textContent = Number.isFinite(rotationDays)
        ? `${Math.abs(rotationDays).toFixed(5)} d${rotationDays < 0 ? ' · retrograde' : ''}` : '—';
    target.planetaryFields.distance.textContent = parent
        ? `${d.toFixed(9)} AU · ${(d * AU_METERS / 1000).toLocaleString(undefined, { maximumFractionDigits: 1 })} km`
        : 'System barycenter';
    target.planetaryFields.sma.textContent = Number.isFinite(body.orbit?.a) ? `${body.orbit.a.toFixed(7)} AU` : '—';
    target.planetaryFields.ecc.textContent = Number.isFinite(body.orbit?.e) ? body.orbit.e.toFixed(7) : '—';
    target.planetaryFields.atmosphere.textContent = body.atmosphere || '—';
    target.planetaryFields.magnetic.textContent = body.magneticField || '—';
    target.planetaryFields.parent.textContent = parent?.name || 'Solar System barycenter';
    target.planetaryFields.moons.textContent = Number.isFinite(body.moonsKnown) ? String(body.moonsKnown) : '—';
    if (target.planetaryFields.j2) {
        target.planetaryFields.j2.textContent = Number.isFinite(body.j2) && body.j2 > 0 ? body.j2.toExponential(6) : '—';
    }
    if (target.planetaryFields.beta) {
        target.planetaryFields.beta.textContent = Number.isFinite(body.radiationBeta) && body.radiationBeta > 0
            ? body.radiationBeta.toExponential(6) : '—';
    }
    if (target.planetaryFields.tide) {
        const migrationMPerYear = Number(body.tidalMigrationAuPerYear) * AU_METERS;
        target.planetaryFields.tide.textContent = Number.isFinite(migrationMPerYear) && migrationMPerYear !== 0
            ? `${migrationMPerYear.toExponential(4)} m/yr` : '—';
    }
    if (target.planetaryFields.altitude) {
        const sampledAltitudeKm = Number(body.liveAltitudeKm);
        const altitudeKm = Number.isFinite(sampledAltitudeKm)
            ? sampledAltitudeKm
            : (parent && Number.isFinite(parent.radiusKm) ? d * AU_METERS / 1000 - parent.radiusKm : NaN);
        target.planetaryFields.altitude.textContent = Number.isFinite(altitudeKm) ? `${altitudeKm.toLocaleString(undefined, { maximumFractionDigits: 2 })} km` : '—';
    }
    if (target.planetaryFields.density) {
        const density = Number(body.localAtmosphereDensityKgM3);
        target.planetaryFields.density.textContent = Number.isFinite(density) && density > 0 ? `${density.toExponential(5)} kg/m³` : 'vacuum / outside envelope';
    }
}
