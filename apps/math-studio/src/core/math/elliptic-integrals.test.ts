import { describe, expect, it } from "vitest";
import { analyzeEllipticIntegrals, arithmeticGeometricMean, ellipticFirst, ellipticSecond, ellipticThird } from "./elliptic-integrals";

describe("Legendre elliptic integrals", () => {
  it("reduces to elementary functions at m = n = 0", () => {
    const phi = 0.73;
    expect(ellipticFirst(phi, 0)).toBeCloseTo(phi, 11);
    expect(ellipticSecond(phi, 0)).toBeCloseTo(phi, 11);
    expect(ellipticThird(phi, 0, 0)).toBeCloseTo(phi, 11);
  });

  it("agrees with the AGM identity and Legendre relation", () => {
    const result = analyzeEllipticIntegrals(0.64, 1.1, 0.2, 3);
    expect(result.completeFirst).toBeCloseTo(Math.PI / (2 * arithmeticGeometricMean(1, 0.6)), 10);
    expect(result.legendreResidual).toBeCloseTo(0, 10);
    expect(result.ellipsePerimeter).toBeGreaterThan(12);
  });
});
