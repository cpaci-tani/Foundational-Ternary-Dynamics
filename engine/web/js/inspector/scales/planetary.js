import { setInspectorSectionVisibility } from '../chrome.js';

export function handlePlanetaryClick(target, intersects) {
    if (intersects.length > 0) {
        const mesh = intersects[0].object;
        target._selectedPlanetaryId = mesh.userData.id;
        showPlanetaryInspector(target);
        return;
    }
    target._selectedPlanetaryId = -1;
    hidePlanetaryInspector(target);
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

    let starPos = { x: 0, y: 0, z: 0 };
    for (let i = 0; i < data.count; i++) {
        if (data.buffer[i * 16 + 3] === 0) {
            starPos = { x: data.buffer[i * 16 + 0], y: data.buffer[i * 16 + 1], z: data.buffer[i * 16 + 2] };
            break;
        }
    }

    const d = Math.sqrt(((x - starPos.x) ** 2) + ((y - starPos.y) ** 2) + ((z - starPos.z) ** 2));
    let uTemp = 0.0;
    let biome = 'Deep Space';

    if (type === 0) {
        target.planetaryFields.type.textContent = 'Host Star';
        target.planetaryFields.dot.style.background = '#facc15';
        biome = 'Stellar Plasma';
    } else {
        target.planetaryFields.type.textContent = 'Rocky Exoplanet';
        target.planetaryFields.dot.style.background = '#4ade80';

        if (d < 0.5) {
            uTemp = 1.0;
            biome = 'Lava World';
        } else if (d > 2.0) {
            uTemp = -1.0;
            biome = 'Ice World';
        } else {
            uTemp = (1.25 - d);
            biome = 'Temperate Earthlike';
        }

        if (type === 2) {
            target.planetaryFields.type.textContent = 'Gas Giant';
            target.planetaryFields.dot.style.background = '#38bdf8';
            biome = 'Gas/Fluid Envelope';
        }
    }

    target.planetaryFields.id.textContent = target._selectedPlanetaryId;
    target.planetaryFields.mass.textContent = typeof mass === 'number' ? `${mass.toFixed(4)} M☉` : mass;
    const tK = 280 + (uTemp * 500);
    target.planetaryFields.temp.textContent = Math.round(tK).toString();
    target.planetaryFields.biome.textContent = biome;
    target.planetaryFields.pos.textContent = `(${x.toFixed(4)}, ${y.toFixed(4)}, ${z.toFixed(4)})`;
    target.planetaryFields.vel.textContent = `(${vx.toFixed(4)}, ${vy.toFixed(4)}, ${vz.toFixed(4)})`;
    target.planetaryFields.speed.textContent = `${speed.toFixed(6)} AU/t`;
}
