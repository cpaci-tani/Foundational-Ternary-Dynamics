// engine/web/tests/field-line-knots-seeds.spec.js
// Coverage seeds: particle-anchored + importance-sampled field peaks, voxel-deduped.
import { test, expect } from '@playwright/test';
import { unionStreamlineSeeds } from '../js/fieldlines.js';
import {
    overlayWorkActive,
    wantsStreamlineApply,
    wantsStreamlineJob,
} from '../js/scales/scale0/runtime/knot-streamline-plan.js';

test('unionStreamlineSeeds keeps particle seeds and fills leftover slots from field peaks', () => {
    const particles = [[1.2, 1.1, 1.0], [2.4, 2.1, 2.0]];
    const peaks = [[1.4, 1.2, 1.3], [20.1, 21.2, 22.3], [30, 31, 32]];
    const out = unionStreamlineSeeds(particles, peaks, 4);
    expect(out).toEqual([
        [1.2, 1.1, 1.0],
        [2.4, 2.1, 2.0],
        [20.1, 21.2, 22.3],
        [30, 31, 32],
    ]);
});

test('unionStreamlineSeeds respects maxSeeds and ignores empty inputs', () => {
    expect(unionStreamlineSeeds([[1, 1, 1], [8, 8, 8]], [[9, 9, 9]], 1)).toEqual([[1, 1, 1]]);
    expect(unionStreamlineSeeds(null, [[4, 5, 6]], 8)).toEqual([[4, 5, 6]]);
    expect(unionStreamlineSeeds([[4, 5, 6]], undefined, 8)).toEqual([[4, 5, 6]]);
});

test('tracking without visual overlays still schedules E, B, and flux streamline jobs', () => {
    const flags = { showEField: false, showBField: false, showFluxLines: false };
    expect(overlayWorkActive(false, true)).toBe(true);
    expect(wantsStreamlineJob(flags, true, 'e')).toBe(true);
    expect(wantsStreamlineJob(flags, true, 'b')).toBe(true);
    expect(wantsStreamlineJob(flags, true, 'flux')).toBe(true);
    expect(wantsStreamlineApply(flags, 'e')).toBe(false);
    expect(wantsStreamlineApply(flags, 'b')).toBe(false);
    expect(wantsStreamlineApply(flags, 'flux')).toBe(false);
});

test('visual overlay flags still schedule and apply their own streamline jobs', () => {
    const flags = { showEField: true, showBField: false, showFluxLines: true };
    expect(wantsStreamlineJob(flags, false, 'e')).toBe(true);
    expect(wantsStreamlineJob(flags, false, 'b')).toBe(false);
    expect(wantsStreamlineJob(flags, false, 'flux')).toBe(true);
    expect(wantsStreamlineApply(flags, 'e')).toBe(true);
    expect(wantsStreamlineApply(flags, 'b')).toBe(false);
    expect(wantsStreamlineApply(flags, 'flux')).toBe(true);
});
