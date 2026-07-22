import type { ExperimentDefinition, MathProject, ParameterDefinition, ViewLayout } from "../core/types";
import { createId } from "../core/project";
import { buildDyadicFormulas, createDyadicParameters } from "./dyadic-model";
import { G_STAR } from "../core/math/special-functions";

function parameter(id: string, label: string, value: number, min: number, max: number, step = 0.01, symbol = id): ParameterDefinition {
  return { id, label, value, min, max, step, symbol };
}

function baseProject(name: string, experimentId: string, layout: ViewLayout = "split"): Omit<MathProject, "formulas" | "parameters"> {
  return {
    schemaVersion: 1,
    id: createId("project"),
    name,
    experimentId,
    style: {
      color: "#64d8c8",
      secondaryColor: "#e7b453",
      lineWidth: 2,
      pointSize: 5,
      showGrid: true,
      showAxes: true,
      showTrail: true,
      drawScene: false
    },
    camera2d: { center: [0, 0], zoom: 1 },
    camera3d: { position: [6, -8, 5], target: [0, 0, 0], fov: 45 },
    timeline: { duration: 8, fps: 60, loop: true, keyframes: [] },
    layout,
    samples: 1536,
    equationOverlay: true,
    updatedAt: new Date().toISOString()
  };
}

const experiments: ExperimentDefinition[] = [
  {
    id: "parametric-workbench",
    name: "Parametric Curve Laboratory",
    category: "General curve analysis",
    description: "Author x(t), y(t), and z(t) directly, then inspect the moving Frenet frame and differential invariants.",
    createProject: () => ({
      ...baseProject("Untitled Parametric Study", "parametric-workbench"),
      formulas: { x: "R*cos(t)", y: "R*sin(t)", z: "pitch*t", tMin: 0, tMax: Math.PI * 4 },
      parameters: [
        parameter("R", "Radius", 2, 0.05, 8, 0.01, "R"),
        parameter("pitch", "Axial pitch", 0.25, -2, 2, 0.001, "p")
      ]
    })
  },
  {
    id: "elliptic-integrals",
    name: "Elliptic Integral Laboratory",
    category: "Special functions and geometry",
    description: "Connect Legendre's three integrals to ellipse arc length, the AGM, the nome, and nonlinear pendulum periods.",
    createProject: () => ({
      ...baseProject("Elliptic Integral Study", "elliptic-integrals"),
      formulas: { x: "scale*Gstar*cos(t)", y: "scale*Gstar*sqrt(1-m)*sin(t)", z: "0", tMin: 0, tMax: Math.PI * 2 },
      parameters: [
        parameter("Gstar", "Gamma bridge constant", G_STAR, 0.1, 8, 0.000001, "G*"),
        parameter("scale", "Geometric multiplier", 1, 0.05, 4, 0.01, "λ"),
        parameter("m", "Elliptic parameter", 0.64, 0, 0.999, 0.001, "m = k²"),
        parameter("phi", "Amplitude", 1, 0, Math.PI / 2, 0.001, "φ"),
        parameter("n", "Characteristic", 0.2, -2, 0.95, 0.001, "n")
      ]
    })
  },
  {
    id: "dyadic-fourier",
    name: "Dyadic Fourier Laboratory",
    category: "Lacunary mode analysis",
    description: "Build a single curve from up to 25 octave-separated modes and study how its local geometry changes.",
    createProject: () => ({
      ...baseProject("Dyadic Fourier Study", "dyadic-fourier"),
      formulas: buildDyadicFormulas(4),
      parameters: createDyadicParameters()
    })
  }
];

export function listExperiments(): ExperimentDefinition[] {
  return experiments;
}

export function hasExperiment(id: string): boolean {
  return experiments.some((experiment) => experiment.id === id);
}

export function createExperimentProject(id: string): MathProject {
  return (experiments.find((experiment) => experiment.id === id) ?? experiments[0]).createProject();
}
