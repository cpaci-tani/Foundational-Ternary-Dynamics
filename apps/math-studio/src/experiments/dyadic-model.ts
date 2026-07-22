import type { FormulaDefinition, ParameterDefinition } from "../core/types";

export const MAX_DYADIC_MODES = 25;

export function buildDyadicFormulas(modeCount: number): FormulaDefinition {
  const count = Math.max(1, Math.min(MAX_DYADIC_MODES, Math.round(modeCount)));
  const xTerms: string[] = [];
  const yTerms: string[] = [];
  for (let k = 0; k < count; k += 1) {
    const frequency = 2 ** k;
    xTerms.push(`a${k}*cos(${frequency}*timeScale*t + phase${k})`);
    yTerms.push(`chirality${k}*a${k}*sin(${frequency}*timeScale*t + phase${k})`);
  }
  const u = `(${xTerms.join(" + ")})`;
  const v = `(b*(${yTerms.join(" + ")}))`;
  return {
    x: `xOffset + xScale*(${u}*cos(rotation) - ${v}*sin(rotation))`,
    y: `yOffset + yScale*(${u}*sin(rotation) + ${v}*cos(rotation))`,
    z: "zOffset + zScale*depth*sin(liftFrequency*t + liftPhase)",
    tMin: 0,
    tMax: Math.PI * 2
  };
}

export function createDyadicParameters(): ParameterDefinition[] {
  const parameters: ParameterDefinition[] = [
    { id: "modeCount", label: "Mode count", value: 4, min: 1, max: MAX_DYADIC_MODES, step: 1, category: "mode-count" },
    { id: "b", label: "Y ratio", value: 2, min: -8, max: 8, step: 0.01, category: "global" },
    { id: "timeScale", label: "Frequency scale", value: 1, min: 0.01, max: 4, step: 0.01, category: "global" },
    { id: "rotation", label: "Planar rotation", value: 0, min: -3.14, max: 3.14, step: 0.01, category: "global" },
    { id: "xScale", label: "X scale", value: 1, min: -4, max: 4, step: 0.01, category: "global" },
    { id: "yScale", label: "Y scale", value: 1, min: -4, max: 4, step: 0.01, category: "global" },
    { id: "zScale", label: "Z scale", value: 1, min: -4, max: 4, step: 0.01, category: "global" },
    { id: "xOffset", label: "X offset", value: 0, min: -10, max: 10, step: 0.01, category: "global" },
    { id: "yOffset", label: "Y offset", value: 0, min: -10, max: 10, step: 0.01, category: "global" },
    { id: "zOffset", label: "Z offset", value: 0, min: -10, max: 10, step: 0.01, category: "global" },
    { id: "depth", label: "Lift depth", value: 0.7, min: 0, max: 6, step: 0.01, category: "global" },
    { id: "liftFrequency", label: "Lift frequency", value: 1, min: 0, max: 64, step: 0.01, category: "global" },
    { id: "liftPhase", label: "Lift phase", value: 0, min: -3.14, max: 3.14, step: 0.01, category: "global" }
  ];

  for (let k = 0; k < MAX_DYADIC_MODES; k += 1) {
    const seed = [1, 0.5, 0.5, 0.375][k] ?? 0;
    parameters.push(
      { id: `a${k}`, label: `k${k} amplitude`, value: seed, min: -2, max: 2, step: 0.001, category: "mode-amplitude", modeIndex: k },
      { id: `phase${k}`, label: `k${k} phase`, value: 0, min: -3.14, max: 3.14, step: 0.01, category: "mode-phase", modeIndex: k },
      { id: `chirality${k}`, label: `k${k} chirality`, value: k % 2 === 0 ? 1 : -1, min: -1, max: 1, step: 0.01, category: "mode-chirality", modeIndex: k }
    );
  }
  return parameters;
}

export function dyadicDisplayEquations(modeCount: number): [string, string, string] {
  const last = Math.max(0, Math.min(MAX_DYADIC_MODES, Math.round(modeCount)) - 1);
  return [
    `u(t) = Σ[k=0..${last}] a_k cos(2^k s t + φ_k)`,
    `v(t) = b Σ[k=0..${last}] c_k a_k sin(2^k s t + φ_k)`,
    "C(t) = (ox + sx(u cosθ - v sinθ), oy + sy(u sinθ + v cosθ), oz + sz d sin(ft + φz))"
  ];
}
