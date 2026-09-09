/** Generic construction recipes for Scale 3 scenario contracts. */

import { instantiateMolecule } from '../../molecules.js';

function setVelocity(bridge, ids, velocities) {
    ids.forEach((id, index) => bridge.aeSetAtomVelocity?.(id, ...velocities[index]));
}

function addReference(bridge, moleculeId, options) {
    const result = instantiateMolecule(bridge, moleculeId, options);
    if (!result) throw new Error(`Unknown molecule reference: ${moleculeId}`);
    return result;
}

/** Deterministic linear-congruential generator; seed is an unsigned 32-bit int. */
function lcg(seed) {
    let s = seed >>> 0;
    return () => { s = (s * 1664525 + 1013904223) >>> 0; return s / 4294967296; };
}

/** Standard-normal deviate via Box–Muller, driven by the seeded rng. */
function gauss(rng) {
    let u = 0, v = 0;
    while (u <= 1e-12) u = rng();
    while (v <= 1e-12) v = rng();
    return Math.sqrt(-2 * Math.log(u)) * Math.cos(2 * Math.PI * v);
}

/**
 * Place nx*ny*nz water molecules on a centred cubic lattice with position
 * jitter and Maxwell-distributed center-of-mass velocities (std sqrt(T/mass),
 * mass 18), then remove the total momentum so the liquid slab starts at rest
 * in aggregate.
 */
function placeWaters(bridge, rng, nx, ny, nz, spacing, jitter, temperature) {
    const mass = 18;
    const sigma = Math.sqrt(temperature / mass);
    const sites = [];
    for (let ix = 0; ix < nx; ix++) {
        for (let iy = 0; iy < ny; iy++) {
            for (let iz = 0; iz < nz; iz++) {
                const x = (ix - (nx - 1) / 2) * spacing + jitter * (2 * rng() - 1);
                const y = (iy - (ny - 1) / 2) * spacing + jitter * (2 * rng() - 1);
                const z = (iz - (nz - 1) / 2) * spacing + jitter * (2 * rng() - 1);
                const rotation = [rng() * 2 * Math.PI, rng() * 2 * Math.PI, rng() * 2 * Math.PI];
                const velocity = [sigma * gauss(rng), sigma * gauss(rng), sigma * gauss(rng)];
                sites.push({ x, y, z, rotation, velocity });
            }
        }
    }
    const n = sites.length;
    const drift = [0, 0, 0];
    for (const s of sites) { drift[0] += s.velocity[0]; drift[1] += s.velocity[1]; drift[2] += s.velocity[2]; }
    if (n > 0) { drift[0] /= n; drift[1] /= n; drift[2] /= n; }
    const atomIds = [];
    for (const s of sites) {
        const velocity = [s.velocity[0] - drift[0], s.velocity[1] - drift[1], s.velocity[2] - drift[2]];
        const result = addReference(bridge, 'water', { offset: [s.x, s.y, s.z], rotation: s.rotation, velocity });
        atomIds.push(...result.atomIds);
    }
    return { atomIds };
}

export function setupScale3Scenario(bridge, scenario) {
    if (scenario.moleculeId) return addReference(bridge, scenario.moleculeId);
    const rng = lcg(scenario.seed);

    switch (scenario.setup) {
        case 'h2-vibration': {
            const result = addReference(bridge, 'h2');
            setVelocity(bridge, result.atomIds, [[-0.16, 0, 0], [0.16, 0, 0]]);
            return result;
        }
        case 'water-rotation':
            return addReference(bridge, 'water', { angularVelocity: [0, 0, 0.12] });
        case 'h2-dissociation': {
            const result = addReference(bridge, 'h2');
            setVelocity(bridge, result.atomIds, [[-1.1, 0, 0], [1.1, 0, 0]]);
            return result;
        }
        case 'h2-recombination': {
            const left = bridge.aeAddAtom(1, -6, 0, 0, 0.24, 0, 0, 0);
            const right = bridge.aeAddAtom(1, 6, 0, 0, -0.24, 0, 0, 0);
            return { atomIds: [left, right] };
        }
        case 'water-dimer': {
            const left = addReference(bridge, 'water', { offset: [-4.8, 0, 0] });
            const right = addReference(bridge, 'water', { offset: [4.8, 0, 0], rotation: [0, 0, Math.PI] });
            return { atomIds: [...left.atomIds, ...right.atomIds] };
        }
        case 'dipole-alignment': {
            const left = addReference(bridge, 'hcl', { offset: [-5, -1.8, 0], rotation: [0, 0, Math.PI / 3] });
            const right = addReference(bridge, 'hcl', { offset: [5, 1.8, 0], rotation: [0, 0, -Math.PI / 2] });
            return { atomIds: [...left.atomIds, ...right.atomIds] };
        }
        case 'molecular-collision': {
            const left = addReference(bridge, 'methane', { offset: [-10, 0, 0], velocity: [0.24, 0, 0] });
            const right = addReference(bridge, 'methane', { offset: [10, 0, 0], velocity: [-0.24, 0, 0], rotation: [0, Math.PI / 4, 0] });
            return { atomIds: [...left.atomIds, ...right.atomIds] };
        }
        case 'water-thermal-cycle': {
            const offsets = [[-5, -5, 0], [5, -5, 0], [-5, 5, 0], [5, 5, 0]];
            const atomIds = [];
            offsets.forEach((offset, index) => {
                const result = addReference(bridge, 'water', {
                    offset,
                    rotation: [0, 0, index * Math.PI / 2],
                    velocity: [index % 2 ? -0.05 : 0.05, index < 2 ? 0.04 : -0.04, 0],
                });
                atomIds.push(...result.atomIds);
            });
            return { atomIds };
        }
        case 'nacl-crystal': {
            const atomIds = [];
            const spacing = 7.5;
            for (let ix = 0; ix < 3; ix++) for (let iy = 0; iy < 3; iy++) for (let iz = 0; iz < 3; iz++) {
                const even = (ix + iy + iz) % 2 === 0;
                atomIds.push(bridge.aeAddAtom(even ? 11 : 17,
                    (ix - 1) * spacing, (iy - 1) * spacing, (iz - 1) * spacing,
                    0, 0, 0, even ? 1 : -1));
            }
            return { atomIds };
        }
        case 'liquid-shear-layer':
            return placeWaters(bridge, rng, 4, 8, 2, 4.2, 0.3, 1.0);
        case 'liquid-channel': {
            const h = 21, wallY = [h / 2 + 2.1, -(h / 2 + 2.1)];
            const atomIds = [];
            for (const y of wallY) {
                for (let i = 0; i < 6; i++) {
                    const x = -10.5 + i * 4.2;
                    for (let j = 0; j < 3; j++) {
                        const z = -4.2 + j * 4.2;
                        atomIds.push(bridge.aeAddLockedAtom(18, x, y, z, 0, 22));
                    }
                }
            }
            const waters = placeWaters(bridge, rng, 4, 4, 3, 4.2, 0.3, 1.0);
            atomIds.push(...waters.atomIds);
            return { atomIds };
        }
        case 'liquid-droplet':
            return placeWaters(bridge, rng, 4, 4, 4, 4.2, 0.3, 1.0);
        case 'liquid-spinning-droplet':
            return placeWaters(bridge, rng, 4, 4, 4, 4.2, 0.3, 1.0);
        case 'custom':
            return { atomIds: [] };
        default:
            throw new Error(`Missing Scale 3 setup recipe: ${scenario.id}`);
    }
}
