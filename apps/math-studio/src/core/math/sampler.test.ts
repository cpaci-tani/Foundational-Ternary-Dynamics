import { describe, expect, it } from "vitest";
import { sampleParametricCurve } from "./sampler";

describe("sampleParametricCurve", () => {
  it("samples a unit circle and computes stable bounds", () => {
    const sample = sampleParametricCurve({ x: "cos(t)", y: "sin(t)", z: "0", tMin: 0, tMax: Math.PI * 2 }, {}, 257);
    expect(sample.error).toBeNull();
    expect(sample.count).toBe(257);
    expect(sample.bounds.radius).toBeCloseTo(1, 3);
    expect(sample.bounds.center[0]).toBeCloseTo(0, 3);
  });

  it("returns a structured error for an invalid expression", () => {
    const sample = sampleParametricCurve({ x: "cos(", y: "t", z: "0", tMin: 0, tMax: 1 }, {}, 64);
    expect(sample.count).toBe(0);
    expect(sample.error).toBeTruthy();
  });
});
