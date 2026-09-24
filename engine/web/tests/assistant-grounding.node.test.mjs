import test from 'node:test';
import assert from 'node:assert/strict';
import { constrainActions } from '../js/assistant/grounding.js';
import { OBSERVER_ACTION_DESCRIPTORS } from '../js/assistant/observer-control.js';
import { validateValue } from '../js/assistant/contracts.js';

const step = { type: 'lattice.step', args: { type: 'object', properties: { count: { type: 'integer', minimum: 1, maximum: 4096 } }, required: ['count'], additionalProperties: false } };
function fixture({ profile = 'playground', selected = true, mass = 2, shape = 'box' } = {}) {
    return { workspace: 'observer', ownerId: 'test-owner', preparationVersion: '1:2:3', tick: 10,
        capabilities: OBSERVER_ACTION_DESCRIPTORS.map(action => action.type), actions: OBSERVER_ACTION_DESCRIPTORS,
        facts: { profile, units: 'normalized (c = 1)', crosshair: { entityId: 'old-observation', historical: true } },
        selected: selected ? { id: 'live-cube', current: { id: 'live-cube', mass, alive: true, shape } } : null };
}
function result(text, names = ['observer.update'], overrides = {}, transform = value => value) {
    const observation = fixture(overrides);
    return constrainActions(text, observation, transform(OBSERVER_ACTION_DESCRIPTORS.filter(action => names.includes(action.type))));
}
function exact(output, key, value, type = 'observer.update') {
    assert.equal(output.clarification, undefined);
    const descriptor = output.actions.find(action => action.type === type);
    assert.ok(descriptor, `Missing ${type}`);
    assert.deepEqual(descriptor.args.properties[key].enum, [value]);
    assert.ok(descriptor.args.required.includes(key));
    return descriptor;
}
function rejected(output, pattern) {
    assert.deepEqual(output.actions, []);
    assert.equal(typeof output.clarification, 'string');
    assert.match(output.clarification, pattern);
}

test('relative mass language binds the exact ratio and live selected identity', () => {
    for (const [text, factor] of [
        ['Double the mass of this object', 2], ['Make it twice as heavy', 2],
        ['Halve the mass of the selected object', .5], ['Make this object half as heavy', .5],
        ['Triple the selected mass', 3], ['Set this object to 2.75 times its current mass', 2.75],
        ['Give this object three times its current mass', 3],
    ]) {
        const output = result(text);
        const descriptor = exact(output, 'massFactor', factor);
        exact(output, 'id', 'live-cube');
        assert.equal(descriptor.args.properties.mass, undefined);
        assert.throws(() => validateValue({ id: 'live-cube', massFactor: factor + .001 }, descriptor.args));
        assert.throws(() => validateValue({ id: 'live-cube' }, descriptor.args));
    }
});

test('absolute mass is exact, cannot retain an alternative factor, and never clamps', () => {
    const output = result('Set the selected object mass to 0.03125');
    const descriptor = exact(output, 'mass', .03125);
    assert.equal(descriptor.args.properties.massFactor, undefined);
    for (const value of [-1, 0, 100001, '1e309', 'NaN', 'infinity']) rejected(result(`Set this object mass to ${value}`), /mass|explicitly/);
    for (const text of ['Make this object heavier', 'Increase this object mass', 'Set mass to 2 or 3', 'Set mass to 1/2']) rejected(result(text), /mass/);
});

test('relative mass validates both the multiplier and the resulting current mass', () => {
    rejected(result('Double the selected mass', ['observer.update'], { mass: 60000 }), /current mass.*60000/);
    rejected(result('Make this object 0.0001 times its mass'), /multiplier/);
    rejected(result('Make this object 0 times its mass'), /mass range/);
    rejected(result('Double and halve this object mass'), /more than one/);
    rejected(result('Set the mass to 3, then double it'), /either an absolute mass/);
    rejected(result('Create a cube with twice the mass', ['observer.create']), /absolute mass/);
});

test('selected targets require a current selection, never a historical hit', () => {
    for (const [text, types] of [
        ['Make it twice as heavy', ['observer.update']],
        ['Delete that object', ['observer.delete']],
        ['Move this object to [1, 2, 3]', ['observer.update']],
        ['Apply impulse [0, 4, 0] to the selected object', ['observer.impulse']],
        ['Disable gravity for the selected object', ['observer.update']],
    ]) rejected(result(text, types, { selected: false }), /Select a current object/);
    assert.equal(result('Resume this Playground', ['observer.resume'], { selected: false }).clarification, undefined);
});

test('ordered and labelled vectors bind each requested field independently', () => {
    for (const tail of ['[1, -2, 3]', '(1, -2, 3)', '1, -2, 3', 'x=1 y=-2 z=3', 'z=3, x=1, y=-2']) exact(result(`Set this object position to ${tail}`), 'position', [1,-2,3]);
    exact(result('Set the selected dimensions to 2 by 3 by 4'), 'size', [2,3,4]);
    exact(result('Set the selected dimensions to 2 x 3 x 4'), 'size', [2,3,4]);
    const output = result('Set this object position to [1, 2, 3] and velocity to [0, -.2, 0]');
    exact(output, 'position', [1,2,3]); exact(output, 'velocity', [0,-.2,0]);
    exact(result('Set this object color to RGB [0.2, 0.7, 1]'), 'color', [.2,.7,1]);
});

test('vectors cannot lose invalid, missing, conflicting or surplus components', () => {
    for (const text of [
        'Set this object position to [1,2]', 'Set this object position to [1,2,3,4]',
        'Set this object position to x=1 y=2', 'Set this object position to [1,2,3] and position to [4,5,6]',
        'Set this object dimensions to [1,0,1]', 'Set this object dimensions to [-1,1,1]',
        'Set this object position to [1e309,0,0]', 'Move this object over there',
        'Move this object by [1,2,3]',
    ]) rejected(result(text), /Specify|supported|more than one|destination/);
});

test('new objects receive explicit independent scalars and vectors without a selected identity', () => {
    const output = result('Create a box at [0, 4, -8] with dimensions [2, 3, 4] and mass 7', ['observer.create'], { selected: false });
    exact(output, 'position', [0,4,-8], 'observer.create');
    exact(output, 'size', [2,3,4], 'observer.create');
    exact(output, 'mass', 7, 'observer.create');
    assert.equal(output.actions[0].args.properties.id, undefined);
});

test('impulse requires a nonzero complete vector, not only its magnitude', () => {
    exact(result('Apply impulse [0, -2.5, 0] to the selected object', ['observer.impulse']), 'impulse', [0,-2.5,0], 'observer.impulse');
    rejected(result('Apply impulse of 2 to the selected object', ['observer.impulse']), /impulse/);
    rejected(result('Apply impulse [0,0,0] to the selected object', ['observer.impulse']), /nonzero/);
});

test('SR restrictions explain unsupported requests without altering their numbers', () => {
    rejected(result('Apply impulse [0,2,0] to this object', [], { profile: 'sr' }), /Playground/);
    rejected(result('Set this object angular velocity to [0,1,0]', ['observer.update'], { profile: 'sr' }), /rotation requires Playground/);
    rejected(result('Set this object proper acceleration to [1,0,0]', ['observer.update'], { profile: 'sr' }), /clock or beacon/);
    rejected(result('Set this object velocity to [.8,.8,0]', ['observer.update'], { profile: 'sr' }), /magnitude at most 0.99/);
    exact(result('Set this object velocity to [.3,0,0]', ['observer.update'], { profile: 'sr' }), 'velocity', [.3,0,0]);
    exact(result('Set this object proper acceleration to [1,0,0]', ['observer.update'], { profile: 'sr', shape: 'clock' }), 'properAcceleration', [1,0,0]);
    exact(result('Set this object rotation to [0,1,0]', ['observer.update'], { profile: 'sr' }), 'rotation', [0,1,0]);
});

test('whole tick counts are explicit bounded enums in both workspaces', () => {
    for (const [text,count] of [['Advance one tick',1],['Advance thirty seven ticks',37],['Advance one hundred and twenty ticks',120],['Advance exactly 4e3 lattice ticks',4000]]) {
        exact(constrainActions(text, fixture(), [step]), 'count', count, 'lattice.step');
    }
    exact(result('Pause, advance exactly 117 ticks, then resume', ['observer.pause','observer.step','observer.resume']), 'count', 117, 'observer.step');
    for (const value of [-1,0,1.5,4097,'1e309']) rejected(constrainActions(`Advance ${value} ticks`,fixture(),[step]), /tick count/);
    rejected(result('Advance 121 ticks', ['observer.step']), /1 to 120/);
    rejected(constrainActions('Advance a few ticks',fixture(),[step]), /whole tick count/);
    rejected(constrainActions('Advance 2 ticks then advance 3 ticks',fixture(),[step]), /more than one/);
});

test('multi-step scalar grounding does not borrow quantities from other operations', () => {
    const output = result('Set gravity strength to zero, apply impulse [0,2,0] to the selected object, then resume', ['observer.world','observer.impulse','observer.resume']);
    exact(output,'gravityStrength',0,'observer.world'); exact(output,'impulse',[0,2,0],'observer.impulse');
    rejected(result('Set gravity strength to -2', ['observer.world']), /gravity strength/);
    rejected(result('Increase the world gravity', ['observer.world']), /absolute number/);
    rejected(result('Set this object mass and position to [1,2,3]'), /mass/);
});

test('other explicit scalar controls preserve their published ranges', () => {
    exact(result('Set this object restitution to .42'), 'restitution', .42);
    exact(result('Set the selected cube restitution to 0.25.'), 'restitution', .25);
    exact(result('Set the selected cube mass to 2.125.'), 'mass', 2.125);
    exact(result('Set force-gun multiplier to 3.75', ['observer.forceGun']), 'multiplier', 3.75, 'observer.forceGun');
    exact(result('Set camera field of view to 95 degrees', ['observer.camera']), 'fov', 95, 'observer.camera');
    rejected(result('Set camera field of view to 180 degrees', ['observer.camera']), /20 to 120/);
    rejected(result('Set this object restitution to 2'), /0 to 1/);
});

test('unsupported numeric separators cannot silently produce a smaller valid quantity', () => {
    rejected(constrainActions('Advance 1,500 ticks',fixture(),[step]), /whole tick count/);
    rejected(constrainActions('Advance 1_500 ticks',fixture(),[step]), /whole tick count/);
    rejected(result('Set the selected mass to 1,500.'), /mass/);
    rejected(result('Set the selected mass to one two.'), /mass/);
    rejected(constrainActions('Advance ten twenty ticks',fixture(),[step]), /tick count/);
    exact(constrainActions('Advance one thousand two hundred and fifteen ticks',fixture(),[step]), 'count', 1215, 'lattice.step');
});

test('physical units are never silently stripped or converted', () => {
    for (const text of ['Set this object mass to 10 kilograms','Set this object mass to 2kg','Set this object position to [1,2,3] m','Set this object mass to two pounds','Set this object velocity to [1,0,0] meters per second']) rejected(result(text), /physical units/);
    rejected(constrainActions('Advance 5 seconds',fixture(),[step]), /tick count/);
    rejected(result('Set this object rotation to [0,90,0] degrees'), /radians/);
    rejected(result('Set this object rotation to [0,90,0]°'), /radians/);
    rejected(result('Set this object mass to 20%'), /Percentage/);
    exact(result('Set this object mass to 3 in displayed simulation units'), 'mass', 3);
});

test('questions mentioning quantities or missing targets are left for explanation', () => {
    const candidates = OBSERVER_ACTION_DESCRIPTORS.filter(action => action.type === 'observer.update');
    for (const text of ['Why is the mass -1?', 'Explain why this object cannot receive impulse [0,1,0] in SR', 'How do kilograms map to the world?']) {
        const output = constrainActions(text,fixture({selected:false,profile:'sr'}),candidates);
        assert.equal(output.clarification,undefined); assert.deepEqual(output.actions,candidates);
    }
});

test('grounding only intersects existing schemas and leaves input objects untouched', () => {
    const observation = fixture();
    const candidates = structuredClone(OBSERVER_ACTION_DESCRIPTORS.filter(action => action.type === 'observer.update'));
    candidates[0].args.properties.mass.enum = [2,4,6];
    const before = structuredClone({ observation,candidates });
    exact(constrainActions('Set this object mass to 4',observation,candidates),'mass',4);
    rejected(constrainActions('Set this object mass to 3',observation,candidates),/supported values/);
    assert.deepEqual({observation,candidates},before);
    delete candidates[0].args.properties.massFactor;
    rejected(constrainActions('Double this object mass',observation,candidates),/not available/);
    const onlyPause = OBSERVER_ACTION_DESCRIPTORS.filter(action => action.type === 'observer.pause');
    rejected(constrainActions('Set this object mass to 4',observation,onlyPause),/not available/);
});
