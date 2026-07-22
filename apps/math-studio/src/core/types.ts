export type Vec3 = [number, number, number];
export type ViewLayout = "split" | "2d" | "3d";
export type Interpolation = "linear" | "smooth" | "hold";

export interface ParameterDefinition {
  id: string;
  label: string;
  value: number;
  min: number;
  max: number;
  step: number;
  category?: "global" | "mode-count" | "mode-amplitude" | "mode-phase" | "mode-chirality";
  modeIndex?: number;
  symbol?: string;
  unit?: string;
}

export interface FormulaDefinition {
  x: string;
  y: string;
  z: string;
  tMin: number;
  tMax: number;
}

export interface CurveStyle {
  color: string;
  secondaryColor: string;
  lineWidth: number;
  pointSize: number;
  showGrid: boolean;
  showAxes: boolean;
  showTrail: boolean;
  drawScene: boolean;
}

export interface Camera2DState {
  center: [number, number];
  zoom: number;
}

export interface Camera3DState {
  position: Vec3;
  target: Vec3;
  fov: number;
}

export interface Keyframe {
  id: string;
  time: number;
  interpolation: Interpolation;
  parameters: Record<string, number>;
  camera2d: Camera2DState;
  camera3d: Camera3DState;
}

export interface TimelineState {
  duration: number;
  fps: number;
  loop: boolean;
  keyframes: Keyframe[];
}

export interface MathProject {
  schemaVersion: 1;
  id: string;
  name: string;
  experimentId: string;
  formulas: FormulaDefinition;
  parameters: ParameterDefinition[];
  style: CurveStyle;
  camera2d: Camera2DState;
  camera3d: Camera3DState;
  timeline: TimelineState;
  layout: ViewLayout;
  samples: number;
  equationOverlay: boolean;
  updatedAt: string;
}

export interface CurveSample {
  positions: Float32Array;
  count: number;
  domain: { min: number; max: number; step: number };
  bounds: { min: Vec3; max: Vec3; center: Vec3; radius: number };
  error: string | null;
}

export interface CurveAnalysis {
  index: number;
  t: number;
  position: Vec3;
  velocity: Vec3;
  speed: number;
  tangent: Vec3;
  normal: Vec3;
  binormal: Vec3;
  curvature: number;
  torsion: number;
  arcLength: number;
  totalLength: number;
  osculatingRadius: number | null;
}

export interface PedagogySettings {
  pointMarker: boolean;
  radiusVector: boolean;
  projections: boolean;
  parameterAngle: boolean;
  arcLengthLabel: boolean;
  tangent: boolean;
  normal: boolean;
  binormal: boolean;
  osculatingCircle: boolean;
  vectorScale: number;
}

export interface ExperimentDefinition {
  id: string;
  name: string;
  category: string;
  description: string;
  createProject: () => MathProject;
}
