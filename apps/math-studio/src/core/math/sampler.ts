import { compile } from "mathjs/number";
import type { CurveSample, FormulaDefinition, Vec3 } from "../types";

const EMPTY_BOUNDS = { min: [0, 0, 0] as Vec3, max: [0, 0, 0] as Vec3, center: [0, 0, 0] as Vec3, radius: 1 };

export function sampleParametricCurve(
  formulas: FormulaDefinition,
  parameters: Record<string, number>,
  sampleCount: number
): CurveSample {
  try {
    const xExpr = compile(formulas.x);
    const yExpr = compile(formulas.y);
    const zExpr = compile(formulas.z);
    const count = Math.max(32, Math.min(8192, Math.floor(sampleCount)));
    const positions = new Float32Array(count * 3);
    const min: Vec3 = [Infinity, Infinity, Infinity];
    const max: Vec3 = [-Infinity, -Infinity, -Infinity];
    const span = formulas.tMax - formulas.tMin;

    for (let index = 0; index < count; index += 1) {
      const t = formulas.tMin + span * index / Math.max(1, count - 1);
      const scope = { ...parameters, t, pi: Math.PI, tau: Math.PI * 2 };
      const values = [Number(xExpr.evaluate(scope)), Number(yExpr.evaluate(scope)), Number(zExpr.evaluate(scope))] as Vec3;
      if (!values.every(Number.isFinite)) throw new Error(`Non-finite value at t=${t.toFixed(4)}`);
      for (let axis = 0; axis < 3; axis += 1) {
        positions[index * 3 + axis] = values[axis];
        min[axis] = Math.min(min[axis], values[axis]);
        max[axis] = Math.max(max[axis], values[axis]);
      }
    }

    const center: Vec3 = [(min[0] + max[0]) / 2, (min[1] + max[1]) / 2, (min[2] + max[2]) / 2];
    let radius = 0;
    for (let index = 0; index < count; index += 1) {
      radius = Math.max(radius, Math.hypot(
        positions[index * 3] - center[0],
        positions[index * 3 + 1] - center[1],
        positions[index * 3 + 2] - center[2]
      ));
    }
    return {
      positions,
      count,
      domain: { min: formulas.tMin, max: formulas.tMax, step: span / Math.max(1, count - 1) },
      bounds: { min, max, center, radius: Math.max(radius, 1e-3) },
      error: null
    };
  } catch (error) {
    return {
      positions: new Float32Array(),
      count: 0,
      domain: { min: formulas.tMin, max: formulas.tMax, step: 0 },
      bounds: EMPTY_BOUNDS,
      error: error instanceof Error ? error.message : "Expression evaluation failed"
    };
  }
}
