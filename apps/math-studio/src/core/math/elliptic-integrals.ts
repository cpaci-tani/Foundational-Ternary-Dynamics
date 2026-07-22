export interface EllipticIntegralAnalysis {
  incompleteFirst: number;
  incompleteSecond: number;
  incompleteThird: number;
  completeFirst: number;
  completeSecond: number;
  completeThird: number;
  complementaryFirst: number;
  complementarySecond: number;
  agm: number;
  nome: number;
  legendreResidual: number;
  ellipsePerimeter: number;
  pendulumPeriodRatio: number;
  modulus: number;
  complementaryParameter: number;
  modularTauImaginary: number;
  pendulumAmplitude: number;
  completeFirstDerivative: number;
  completeSecondDerivative: number;
}

function adaptiveSimpson(
  fn: (x: number) => number,
  min: number,
  max: number,
  tolerance = 1e-11,
  depth = 16
): number {
  if (min === max) return 0;
  const sign = max >= min ? 1 : -1;
  const a = Math.min(min, max);
  const b = Math.max(min, max);
  const c = (a + b) / 2;
  const fa = fn(a);
  const fb = fn(b);
  const fc = fn(c);
  const whole = (b - a) * (fa + 4 * fc + fb) / 6;
  const recurse = (left: number, right: number, fLeft: number, fMid: number, fRight: number, estimate: number, epsilon: number, remaining: number): number => {
    const middle = (left + right) / 2;
    const leftMid = (left + middle) / 2;
    const rightMid = (middle + right) / 2;
    const fLeftMid = fn(leftMid);
    const fRightMid = fn(rightMid);
    const leftEstimate = (middle - left) * (fLeft + 4 * fLeftMid + fMid) / 6;
    const rightEstimate = (right - middle) * (fMid + 4 * fRightMid + fRight) / 6;
    const delta = leftEstimate + rightEstimate - estimate;
    if (remaining <= 0 || Math.abs(delta) <= 15 * epsilon) return leftEstimate + rightEstimate + delta / 15;
    return recurse(left, middle, fLeft, fLeftMid, fMid, leftEstimate, epsilon / 2, remaining - 1)
      + recurse(middle, right, fMid, fRightMid, fRight, rightEstimate, epsilon / 2, remaining - 1);
  };
  return sign * recurse(a, b, fa, fc, fb, whole, tolerance, depth);
}

export function ellipticIntegrands(theta: number, parameter: number, characteristic: number): [number, number, number] {
  const delta = Math.max(1e-15, 1 - parameter * Math.sin(theta) ** 2);
  const root = Math.sqrt(delta);
  return [1 / root, root, 1 / ((1 - characteristic * Math.sin(theta) ** 2) * root)];
}

export function ellipticFirst(phi: number, parameter: number): number {
  return adaptiveSimpson((theta) => ellipticIntegrands(theta, parameter, 0)[0], 0, phi);
}

export function ellipticSecond(phi: number, parameter: number): number {
  return adaptiveSimpson((theta) => ellipticIntegrands(theta, parameter, 0)[1], 0, phi);
}

export function ellipticThird(phi: number, characteristic: number, parameter: number): number {
  return adaptiveSimpson((theta) => ellipticIntegrands(theta, parameter, characteristic)[2], 0, phi);
}

export function arithmeticGeometricMean(a: number, b: number): number {
  let arithmetic = a;
  let geometric = b;
  for (let iteration = 0; iteration < 32; iteration += 1) {
    const nextArithmetic = (arithmetic + geometric) / 2;
    const nextGeometric = Math.sqrt(arithmetic * geometric);
    arithmetic = nextArithmetic;
    geometric = nextGeometric;
    if (Math.abs(arithmetic - geometric) <= Number.EPSILON * Math.max(1, arithmetic)) break;
  }
  return arithmetic;
}

export function analyzeEllipticIntegrals(parameter: number, phi: number, characteristic: number, semiMajorAxis: number): EllipticIntegralAnalysis {
  const m = Math.max(0, Math.min(0.999999, parameter));
  const amplitude = Math.max(0, Math.min(Math.PI / 2, phi));
  const n = Math.min(0.999999, characteristic);
  const completeAmplitude = Math.PI / 2;
  const completeFirst = ellipticFirst(completeAmplitude, m);
  const completeSecond = ellipticSecond(completeAmplitude, m);
  const complementaryFirst = ellipticFirst(completeAmplitude, 1 - m);
  const complementarySecond = ellipticSecond(completeAmplitude, 1 - m);
  const agm = arithmeticGeometricMean(1, Math.sqrt(1 - m));
  const completeFirstDerivative = m > 1e-10
    ? completeSecond / (2 * m * (1 - m)) - completeFirst / (2 * m)
    : Math.PI / 8;
  const completeSecondDerivative = m > 1e-10 ? (completeSecond - completeFirst) / (2 * m) : -Math.PI / 8;
  return {
    incompleteFirst: ellipticFirst(amplitude, m),
    incompleteSecond: ellipticSecond(amplitude, m),
    incompleteThird: ellipticThird(amplitude, n, m),
    completeFirst,
    completeSecond,
    completeThird: ellipticThird(completeAmplitude, n, m),
    complementaryFirst,
    complementarySecond,
    agm,
    nome: Math.exp(-Math.PI * complementaryFirst / completeFirst),
    legendreResidual: completeFirst * complementarySecond + completeSecond * complementaryFirst - completeFirst * complementaryFirst - Math.PI / 2,
    ellipsePerimeter: 4 * Math.abs(semiMajorAxis) * completeSecond,
    pendulumPeriodRatio: 2 * completeFirst / Math.PI,
    modulus: Math.sqrt(m),
    complementaryParameter: 1 - m,
    modularTauImaginary: complementaryFirst / completeFirst,
    pendulumAmplitude: 2 * Math.asin(Math.sqrt(m)),
    completeFirstDerivative,
    completeSecondDerivative
  };
}
