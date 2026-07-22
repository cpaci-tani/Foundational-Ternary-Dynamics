import { describe, expect, it } from "vitest";
import { sampleParametricCurve } from "./sampler";
import { analyzeCurve } from "./differential-geometry";

describe("differential curve analysis", () => {
  it("recovers the curvature and torsion of a circle", () => {
    const sample = sampleParametricCurve({ x: "R*cos(t)", y: "R*sin(t)", z: "0", tMin: 0, tMax: 2 * Math.PI }, { R: 2 }, 513);
    const analysis = analyzeCurve(sample, 0.37)!;
    expect(analysis.curvature).toBeCloseTo(0.5, 3);
    expect(analysis.torsion).toBeCloseTo(0, 4);
    expect(analysis.totalLength).toBeCloseTo(4 * Math.PI, 3);
  });

  it("finds nonzero curvature and torsion on a circular helix", () => {
    const sample = sampleParametricCurve({ x: "2*cos(t)", y: "2*sin(t)", z: "0.5*t", tMin: 0, tMax: 4 * Math.PI }, {}, 2049);
    const analysis = analyzeCurve(sample, 0.5)!;
    expect(analysis.curvature).toBeCloseTo(2 / 4.25, 3);
    expect(analysis.torsion).toBeCloseTo(0.5 / 4.25, 3);
  });
});
