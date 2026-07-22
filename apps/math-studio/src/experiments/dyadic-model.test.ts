import { describe, expect, it } from "vitest";
import { sampleParametricCurve } from "../core/math/sampler";
import { buildDyadicFormulas, createDyadicParameters, MAX_DYADIC_MODES } from "./dyadic-model";

describe("dyadic mode model", () => {
  it("builds and evaluates the full 25-mode expression", () => {
    const parameters = Object.fromEntries(createDyadicParameters().map((parameter) => [parameter.id, parameter.value]));
    const sample = sampleParametricCurve(buildDyadicFormulas(MAX_DYADIC_MODES), parameters, 128);
    expect(sample.error).toBeNull();
    expect(sample.count).toBe(128);
    expect(sample.positions.every(Number.isFinite)).toBe(true);
  });

  it("creates amplitude, phase, and chirality controls for every mode", () => {
    const parameters = createDyadicParameters();
    expect(parameters.filter((parameter) => parameter.category === "mode-amplitude")).toHaveLength(25);
    expect(parameters.filter((parameter) => parameter.category === "mode-phase")).toHaveLength(25);
    expect(parameters.filter((parameter) => parameter.category === "mode-chirality")).toHaveLength(25);
  });
});
