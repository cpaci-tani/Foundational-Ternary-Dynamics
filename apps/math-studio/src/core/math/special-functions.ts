export const G_STAR = 2.9586751191886385;

const LANCZOS_COEFFICIENTS = [
  0.9999999999998099,
  676.5203681218851,
  -1259.1392167224028,
  771.3234287776531,
  -176.6150291621406,
  12.507343278686905,
  -0.13857109526572012,
  9.984369578019572e-6,
  1.5056327351493116e-7
];

export function gamma(value: number): number {
  if (value < 0.5) return Math.PI / (Math.sin(Math.PI * value) * gamma(1 - value));
  const z = value - 1;
  let series = LANCZOS_COEFFICIENTS[0];
  for (let index = 1; index < LANCZOS_COEFFICIENTS.length; index += 1) series += LANCZOS_COEFFICIENTS[index] / (z + index);
  const t = z + 7.5;
  return Math.sqrt(2 * Math.PI) * t ** (z + 0.5) * Math.exp(-t) * series;
}

export interface ThetaConstants {
  theta2: number;
  theta3: number;
  theta4: number;
  parameterFromTheta: number;
  completeFirstFromTheta: number;
  quarticResidual: number;
}

export function jacobiThetaConstants(nome: number): ThetaConstants {
  const q = Math.max(0, Math.min(0.999999999, nome));
  let theta2 = 0;
  let theta3 = 1;
  let theta4 = 1;
  for (let index = 0; index < 128; index += 1) {
    const halfTerm = q ** ((index + 0.5) ** 2);
    theta2 += 2 * halfTerm;
    if (index > 0) {
      const wholeTerm = q ** (index ** 2);
      theta3 += 2 * wholeTerm;
      theta4 += 2 * (index % 2 === 0 ? 1 : -1) * wholeTerm;
      if (wholeTerm < 1e-16 && halfTerm < 1e-16) break;
    }
  }
  return {
    theta2,
    theta3,
    theta4,
    parameterFromTheta: (theta2 / theta3) ** 4,
    completeFirstFromTheta: Math.PI / 2 * theta3 ** 2,
    quarticResidual: theta3 ** 4 - theta2 ** 4 - theta4 ** 4
  };
}

export function gammaQuarterBridge() {
  const quarter = gamma(0.25);
  const threeQuarter = gamma(0.75);
  return {
    quarter,
    threeQuarter,
    ratio: quarter / threeQuarter,
    lemniscaticCompleteFirst: quarter ** 2 / (4 * Math.sqrt(Math.PI))
  };
}
