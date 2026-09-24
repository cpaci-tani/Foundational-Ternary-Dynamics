import test from 'node:test';
import assert from 'node:assert/strict';
import { reflectPolar, reflectAxial, reflectEulerXYZ, reflectQuaternion, reflectAuthorPatch } from '../js/observer/mirror-frame.js';
import { rotationMatrix, rotate, cross } from '../js/observer/geometry.js';

const close = (a, b) => a.forEach((value, axis) => assert.ok(Math.abs(value - b[axis]) < 1e-12));

test('reflection uses polar forces and axial torques consistently', () => {
    const position = [1, 2, 3], impulse = [4, 5, 6];
    assert.deepEqual(reflectPolar(position), [1, -2, 3]);
    close(reflectAxial(cross(position, impulse)), cross(reflectPolar(position), reflectPolar(impulse)));
    assert.deepEqual(reflectPolar(reflectPolar(position)), position);
    assert.deepEqual(reflectAxial(reflectAxial(position)), position);
});

test('Euler XYZ and quaternion reflection preserve proper orientation and are involutions', () => {
    const rotation = [0.3, -0.7, 1.1], point = [2, 3, -4];
    close(rotate(rotationMatrix(reflectEulerXYZ(rotation)), point),
        reflectPolar(rotate(rotationMatrix(rotation), reflectPolar(point))));
    const quaternion = { x: 0.2, y: -0.3, z: 0.4, w: Math.sqrt(0.71) };
    assert.deepEqual(reflectQuaternion(reflectQuaternion(quaternion)), quaternion);
    assert.ok(Math.abs(Object.values(reflectQuaternion(quaternion)).reduce((sum, v) => sum + v * v, 0) - 1) < 1e-12);
});

test('mirrored author patches preserve identity, dimensions, scalars and input data', () => {
    const patch = { id: 'linked-body', position: [1, 2, 3], velocity: [4, 5, 6],
        rotation: [.1, .2, .3], angularVelocity: [7, 8, 9], properAcceleration: [2, 3, 4],
        size: [2, 4, 6], color: [.2, .4, .6], mass: 2, gravity: true };
    const original = JSON.stringify(patch);
    const reflected = reflectAuthorPatch(patch);
    assert.deepEqual(reflected.position, [1, -2, 3]);
    assert.deepEqual(reflected.velocity, [4, -5, 6]);
    assert.deepEqual(reflected.angularVelocity, [-7, 8, -9]);
    assert.deepEqual(reflected.size, patch.size);
    assert.equal(reflected.id, patch.id); assert.equal(reflected.mass, patch.mass);
    assert.deepEqual(reflectAuthorPatch(reflected), patch);
    assert.equal(reflectAuthorPatch(patch, false), patch);
    assert.equal(JSON.stringify(patch), original);
});
