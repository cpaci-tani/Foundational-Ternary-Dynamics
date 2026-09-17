import test from 'node:test';
import assert from 'node:assert/strict';
import {advisoryMessage, assertPropertyDescriptor, propertyError, propertyTooltip, sliderBounds, stepChoice} from '../js/seeding/property-schema.js';

const amplitude = {key: 'packet.amplitude', label: 'Amplitude', group: 'packet', type: 'real', units: 'lattice units',
    default: .5, min: 0, max: 100, step: .01, recommended: [.25, 1], recommendationBasis: 'Exploratory starting range.',
    description: 'Increasing the amplitude increases the prepared field.', binding: 'native:wave:packet.amplitude'};

test('advisory recommendations preserve legal values beyond the slider starting span', () => {
    assert.equal(assertPropertyDescriptor(amplitude), amplitude);
    assert.equal(propertyError(amplitude, 12.3456789), '');
    assert.match(advisoryMessage(amplitude, 12.3456789), /retained/);
    assert.deepEqual(sliderBounds(amplitude, 12.3456789), [.25, 12.3456789]);
    assert.match(propertyError(amplitude, 101), /Legal range/);
    assert.match(propertyError(amplitude, NaN), /finite/);
    assert.match(propertyTooltip(amplitude), /Increasing.*\nPreset default.*\nRecommended.*\nLegal bounds/s);
});

test('stepped choices skip forbidden values and preserve semantic labels', () => {
    const p = {...amplitude, type: 'choice', default: 1, min: -1, max: 1, recommended: [-1, 1], options: [[-1, 'Negative'], [1, 'Positive']]};
    assert.equal(stepChoice(p, -1, 1), 1);
    assert.equal(stepChoice(p, 1, -1), -1);
    assert.equal(stepChoice(p, 1, 1), 1);
    assert.notEqual(propertyError(p, 0), '');
    assert.match(propertyTooltip(p), /1 — Positive/);
});

test('exact integer seeds and required metadata reject misleading controls', () => {
    const p = {...amplitude, type: 'integer', default: 0, min: 0, max: 4294967295, recommended: [0, 4294967295]};
    assert.equal(propertyError(p, 4294967295), '');
    assert.notEqual(propertyError(p, 3.5), '');
    assert.throws(() => assertPropertyDescriptor({...p, recommendationBasis: ''}), /recommendationBasis/);
    assert.throws(() => assertPropertyDescriptor({...p, recommended: [3, 2]}), /Invalid/);
});
