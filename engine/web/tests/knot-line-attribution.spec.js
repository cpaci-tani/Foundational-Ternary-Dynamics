// engine/web/tests/knot-line-attribution.spec.js
import { test, expect } from '@playwright/test';
import { attributeSegmentsToKnots } from '../js/scales/scale0/runtime/knot-line-attribution.js';

test('attributes segments + length to nearest knot centroid', () => {
    // two knots at x=0 and x=10; one 2-vertex line near each.
    const centroids = new Float32Array([0,0,0, 10,0,0]); // count=2
    const buffer = new Float32Array([ 0,0,0, 1,0,0,   10,0,0, 11,0,0 ]);
    const offsets = new Int32Array([0, 6]);
    const lengths = new Int32Array([6, 6]);
    const result = attributeSegmentsToKnots({ count:2, buffer, offsets, lengths }, centroids, 2);
    expect(result.get(0).segments).toBe(1);
    expect(result.get(1).segments).toBe(1);
    expect(result.get(0).length).toBeCloseTo(1.0, 5);
});
