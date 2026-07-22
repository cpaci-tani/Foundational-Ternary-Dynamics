import type { CurveAnalysis, CurveSample, Vec3 } from "../types";

const EPSILON = 1e-12;

function point(sample: CurveSample, index: number): Vec3 {
  return [sample.positions[index * 3], sample.positions[index * 3 + 1], sample.positions[index * 3 + 2]];
}

function scale(vector: Vec3, factor: number): Vec3 {
  return [vector[0] * factor, vector[1] * factor, vector[2] * factor];
}

function subtract(a: Vec3, b: Vec3): Vec3 {
  return [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
}

function add(a: Vec3, b: Vec3): Vec3 {
  return [a[0] + b[0], a[1] + b[1], a[2] + b[2]];
}

function cross(a: Vec3, b: Vec3): Vec3 {
  return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
}

function dot(a: Vec3, b: Vec3): number {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

function norm(vector: Vec3): number {
  return Math.hypot(...vector);
}

function normalize(vector: Vec3): Vec3 {
  const length = norm(vector);
  return length > EPSILON ? scale(vector, 1 / length) : [0, 0, 0];
}

export function analyzeCurve(sample: CurveSample, phase: number): CurveAnalysis | null {
  if (sample.count < 5 || Math.abs(sample.domain.step) < EPSILON) return null;
  const index = Math.max(2, Math.min(sample.count - 3, Math.round(Math.max(0, Math.min(1, phase)) * (sample.count - 1))));
  const h = sample.domain.step;
  const p0 = point(sample, index);
  const pm2 = point(sample, index - 2);
  const pm1 = point(sample, index - 1);
  const pp1 = point(sample, index + 1);
  const pp2 = point(sample, index + 2);
  const velocity = scale(subtract(pp1, pm1), 1 / (2 * h));
  const acceleration = scale(add(subtract(pp1, scale(p0, 2)), pm1), 1 / (h * h));
  const jerk = scale(add(subtract(pp2, scale(pp1, 2)), subtract(scale(pm1, 2), pm2)), 1 / (2 * h * h * h));
  const speed = norm(velocity);
  const velocityCrossAcceleration = cross(velocity, acceleration);
  const crossNorm = norm(velocityCrossAcceleration);
  const tangent = normalize(velocity);
  const binormal = normalize(velocityCrossAcceleration);
  const normal = normalize(cross(binormal, tangent));
  const curvature = speed > EPSILON ? crossNorm / speed ** 3 : 0;
  const torsion = crossNorm > EPSILON ? dot(velocityCrossAcceleration, jerk) / crossNorm ** 2 : 0;
  let arcLength = 0;
  let totalLength = 0;
  for (let cursor = 1; cursor < sample.count; cursor += 1) {
    const segment = norm(subtract(point(sample, cursor), point(sample, cursor - 1)));
    totalLength += segment;
    if (cursor <= index) arcLength += segment;
  }
  return {
    index,
    t: sample.domain.min + index * h,
    position: p0,
    velocity,
    speed,
    tangent,
    normal,
    binormal,
    curvature,
    torsion,
    arcLength,
    totalLength,
    osculatingRadius: curvature > EPSILON ? 1 / curvature : null
  };
}
