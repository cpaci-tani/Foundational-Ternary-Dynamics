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

export function setupScale3Scenario(bridge, scenario) {
    if (scenario.moleculeId) return addReference(bridge, scenario.moleculeId);

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
        case 'custom':
            return { atomIds: [] };
        default:
            throw new Error(`Missing Scale 3 setup recipe: ${scenario.id}`);
    }
}
