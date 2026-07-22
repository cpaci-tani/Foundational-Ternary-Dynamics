import { describe, expect, it } from "vitest";
import { G_STAR, gamma, gammaQuarterBridge, jacobiThetaConstants } from "./special-functions";

describe("gamma and theta bridges", () => {
  it("evaluates the quarter gamma ratio", () => {
    expect(gamma(0.5)).toBeCloseTo(Math.sqrt(Math.PI), 13);
    expect(gammaQuarterBridge().ratio).toBeCloseTo(G_STAR, 13);
  });

  it("recovers the elliptic parameter from theta constants", () => {
    const theta = jacobiThetaConstants(Math.exp(-Math.PI));
    expect(theta.parameterFromTheta).toBeCloseTo(0.5, 13);
    expect(theta.quarticResidual).toBeCloseTo(0, 13);
  });
});
