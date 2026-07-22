import { describe, expect, it } from "vitest";
import { sampleParametricCurve } from "../core/math/sampler";
import { listExperiments } from "./registry";

describe("curve laboratory registry", () => {
  it("contains the foundational curve and special-function workspaces", () => {
    const experiments = listExperiments();
    expect(experiments.map((experiment) => experiment.id)).toEqual(["parametric-workbench", "elliptic-integrals", "dyadic-fourier"]);
  });

  it("evaluates every registered model with its default parameters", () => {
    listExperiments().forEach((experiment) => {
      const project = experiment.createProject();
      const parameters = Object.fromEntries(project.parameters.map((parameter) => [parameter.id, parameter.value]));
      const sample = sampleParametricCurve(project.formulas, parameters, 128);
      expect(sample.error, experiment.name).toBeNull();
      expect(sample.count, experiment.name).toBe(128);
    });
  });
});
