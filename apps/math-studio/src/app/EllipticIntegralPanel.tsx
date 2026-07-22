import { useMemo, useState } from "react";
import { analyzeEllipticIntegrals, ellipticIntegrands } from "../core/math/elliptic-integrals";
import { gammaQuarterBridge, jacobiThetaConstants } from "../core/math/special-functions";

interface Props {
  parameter: number;
  amplitude: number;
  characteristic: number;
  semiMajorAxis: number;
  gStarParameter: number;
}

type MeasurementKey = "integrals" | "gamma" | "theta" | "modular" | "identities" | "applications";
type MeasurementSettings = Record<MeasurementKey, boolean>;
const MEASUREMENT_KEY = "math-studio.elliptic-measurements.v1";
const DEFAULT_MEASUREMENTS: MeasurementSettings = { integrals: true, gamma: true, theta: true, modular: true, identities: true, applications: true };

function loadMeasurements(): MeasurementSettings {
  try { return { ...DEFAULT_MEASUREMENTS, ...JSON.parse(localStorage.getItem(MEASUREMENT_KEY) ?? "{}") }; }
  catch { return DEFAULT_MEASUREMENTS; }
}

function value(number: number): string {
  const magnitude = Math.abs(number);
  return magnitude !== 0 && (magnitude >= 1e4 || magnitude < 1e-4) ? number.toExponential(5) : number.toFixed(7);
}

export function EllipticIntegralPanel({ parameter, amplitude, characteristic, semiMajorAxis, gStarParameter }: Props) {
  const [measurements, setMeasurements] = useState<MeasurementSettings>(loadMeasurements);
  const setMeasurement = (key: MeasurementKey, enabled: boolean) => {
    const next = { ...measurements, [key]: enabled };
    setMeasurements(next);
    try { localStorage.setItem(MEASUREMENT_KEY, JSON.stringify(next)); } catch { /* Best-effort view preference. */ }
  };
  const analysis = useMemo(
    () => analyzeEllipticIntegrals(parameter, amplitude, characteristic, semiMajorAxis),
    [amplitude, characteristic, parameter, semiMajorAxis]
  );
  const gammaBridge = useMemo(gammaQuarterBridge, []);
  const theta = useMemo(() => jacobiThetaConstants(analysis.nome), [analysis.nome]);
  const plot = useMemo(() => {
    const width = 260;
    const height = 112;
    const padding = 8;
    const samples = Array.from({ length: 97 }, (_, index) => {
      const thetaValue = index / 96 * Math.PI / 2;
      return ellipticIntegrands(thetaValue, parameter, characteristic);
    });
    const max = Math.max(1, ...samples.flatMap((sample) => sample.map((entry) => Math.min(entry, 20))));
    const paths = [0, 1, 2].map((series) => samples.map((sample, index) => {
      const x = padding + index / 96 * (width - 2 * padding);
      const y = height - padding - Math.min(sample[series], 20) / max * (height - 2 * padding);
      return `${index === 0 ? "M" : "L"}${x.toFixed(2)},${y.toFixed(2)}`;
    }).join(" "));
    return { paths, cursor: padding + Math.max(0, Math.min(1, amplitude / (Math.PI / 2))) * (width - 2 * padding), width, height };
  }, [amplitude, characteristic, parameter]);
  const modularY = 100 - Math.min(2, analysis.modularTauImaginary) / 2 * 82;

  return <div className="panel-section elliptic-lab">
    <div className="section-heading"><span>Elliptic measurements</span><span>layers</span></div>
    <div className="measurement-toggle-grid" aria-label="Elliptic measurement layers">
      <MeasurementToggle symbol="∫" label="Integrals" checked={measurements.integrals} onChange={(enabled) => setMeasurement("integrals", enabled)} />
      <MeasurementToggle symbol="Γ" label="Gamma" checked={measurements.gamma} onChange={(enabled) => setMeasurement("gamma", enabled)} />
      <MeasurementToggle symbol="ϑ" label="Theta" checked={measurements.theta} onChange={(enabled) => setMeasurement("theta", enabled)} />
      <MeasurementToggle symbol="i" label="Complex τ" checked={measurements.modular} onChange={(enabled) => setMeasurement("modular", enabled)} />
      <MeasurementToggle symbol="=" label="Identities" checked={measurements.identities} onChange={(enabled) => setMeasurement("identities", enabled)} />
      <MeasurementToggle symbol="↔" label="Applications" checked={measurements.applications} onChange={(enabled) => setMeasurement("applications", enabled)} />
    </div>

    {measurements.integrals && <>
      <svg className="elliptic-integrand-plot" viewBox={`0 0 ${plot.width} ${plot.height}`} role="img" aria-label="First, second, and third elliptic integral integrands from zero to pi over two">
        <line className="plot-axis" x1="8" y1="104" x2="252" y2="104" /><line className="plot-axis" x1="8" y1="8" x2="8" y2="104" />
        <path className="integrand-first" d={plot.paths[0]} /><path className="integrand-second" d={plot.paths[1]} /><path className="integrand-third" d={plot.paths[2]} />
        <line className="amplitude-cursor" x1={plot.cursor} y1="8" x2={plot.cursor} y2="104" />
        <text className="label-second" x="12" y="18">E integrand</text><text className="label-first" x="12" y="31">F integrand</text><text className="label-third" x="12" y="44">Π integrand</text>
        <text x="8" y="111">0</text><text x="233" y="111">π/2</text>
      </svg>
      <div className="elliptic-block"><strong>Incomplete at φ</strong>
        <EllipticValue symbol="F(φ|m)" result={analysis.incompleteFirst} /><EllipticValue symbol="E(φ|m)" result={analysis.incompleteSecond} /><EllipticValue symbol="Π(n;φ|m)" result={analysis.incompleteThird} />
      </div>
      <div className="elliptic-block"><strong>Complete at π/2</strong>
        <EllipticValue symbol="K(m)" result={analysis.completeFirst} /><EllipticValue symbol="E(m)" result={analysis.completeSecond} /><EllipticValue symbol="Π(n|m)" result={analysis.completeThird} />
      </div>
    </>}

    {measurements.gamma && <div className="elliptic-block gamma-bridge"><strong>Quarter-gamma bridge</strong>
      <EllipticValue symbol="Γ(1/4)" result={gammaBridge.quarter} /><EllipticValue symbol="Γ(3/4)" result={gammaBridge.threeQuarter} />
      <EllipticValue symbol="Γ(1/4)/Γ(3/4)" result={gammaBridge.ratio} /><EllipticValue symbol="live G*" result={gStarParameter} />
      <EllipticValue symbol="G* residual" result={gStarParameter - gammaBridge.ratio} /><EllipticValue symbol="K(1/2) gamma form" result={gammaBridge.lemniscaticCompleteFirst} />
    </div>}

    {measurements.theta && <div className="elliptic-block"><strong>Jacobi theta constants</strong>
      <EllipticValue symbol="ϑ₂(0,q)" result={theta.theta2} /><EllipticValue symbol="ϑ₃(0,q)" result={theta.theta3} /><EllipticValue symbol="ϑ₄(0,q)" result={theta.theta4} />
      <EllipticValue symbol="m from (ϑ₂/ϑ₃)⁴" result={theta.parameterFromTheta} /><EllipticValue symbol="K from πϑ₃²/2" result={theta.completeFirstFromTheta} />
    </div>}

    {measurements.modular && <div className="elliptic-block"><strong>Complex modular coordinate</strong>
      <svg className="modular-plane" viewBox="0 0 260 112" role="img" aria-label={`Modular point tau equals ${analysis.modularTauImaginary.toFixed(5)} i`}>
        <line x1="18" y1="100" x2="246" y2="100" /><line x1="130" y1="106" x2="130" y2="8" />
        <text x="235" y="109">Re τ</text><text x="136" y="14">Im τ</text><text x="136" y="61">i</text>
        <circle cx="130" cy={modularY} r="4" /><text className="modular-point-label" x="139" y={Math.max(14, modularY - 4)}>τ = iK'/K</text>
      </svg>
      <EllipticValue symbol="τ" text={`${value(analysis.modularTauImaginary)} i`} /><EllipticValue symbol="q = exp(iπτ)" result={analysis.nome} />
    </div>}

    {measurements.identities && <div className="elliptic-block"><strong>Identity residuals and sensitivity</strong>
      <EllipticValue symbol="Legendre relation" result={analysis.legendreResidual} /><EllipticValue symbol="ϑ₃⁴-ϑ₂⁴-ϑ₄⁴" result={theta.quarticResidual} />
      <EllipticValue symbol="mθ-m" result={theta.parameterFromTheta - parameter} /><EllipticValue symbol="Kθ-K" result={theta.completeFirstFromTheta - analysis.completeFirst} />
      <EllipticValue symbol="dK/dm" result={analysis.completeFirstDerivative} /><EllipticValue symbol="dE/dm" result={analysis.completeSecondDerivative} />
    </div>}

    {measurements.applications && <div className="elliptic-block"><strong>Geometry and dynamics</strong>
      <EllipticValue symbol="ellipse perimeter" result={analysis.ellipsePerimeter} /><EllipticValue symbol="θ₀ pendulum" result={analysis.pendulumAmplitude} /><EllipticValue symbol="T/T₀ pendulum" result={analysis.pendulumPeriodRatio} />
    </div>}
  </div>;
}

function MeasurementToggle({ symbol, label, checked, onChange }: { symbol: string; label: string; checked: boolean; onChange: (enabled: boolean) => void }) {
  return <label className="measurement-toggle"><input type="checkbox" checked={checked} onChange={(event) => onChange(event.target.checked)} /><span>{symbol}</span>{label}</label>;
}

function EllipticValue({ symbol, result, text }: { symbol: string; result?: number; text?: string }) {
  return <div className="elliptic-value"><span>{symbol}</span><code>{text ?? value(result ?? Number.NaN)}</code></div>;
}
