import { create } from "zustand";
import { cloneProject, createId } from "../project";
import type { Camera2DState, Camera3DState, CurveStyle, FormulaDefinition, Interpolation, MathProject, PedagogySettings, ViewLayout } from "../types";
import { createExperimentProject, hasExperiment } from "../../experiments/registry";
import { projectRepository } from "../persistence/project-repository";
import { buildDyadicFormulas, MAX_DYADIC_MODES } from "../../experiments/dyadic-model";

const DRAW_PREFERENCE_KEY = "math-studio.global-draw.v1";
const PEDAGOGY_PREFERENCE_KEY = "math-studio.pedagogy.v1";
const DEFAULT_PEDAGOGY: PedagogySettings = {
  pointMarker: true,
  radiusVector: false,
  projections: false,
  parameterAngle: false,
  arcLengthLabel: false,
  tangent: true,
  normal: true,
  binormal: true,
  osculatingCircle: false,
  vectorScale: 1
};

function loadInitialProject(): MathProject {
  const saved = projectRepository.load();
  if (saved && !hasExperiment(saved.experimentId)) return createExperimentProject("parametric-workbench");
  if (saved?.experimentId === "elliptic-integrals" && !saved.parameters.some((parameter) => parameter.id === "Gstar")) {
    return createExperimentProject("elliptic-integrals");
  }
  if (saved?.experimentId === "dyadic-fourier" && !saved.parameters.some((parameter) => parameter.id === "modeCount")) {
    return createExperimentProject("dyadic-fourier");
  }
  return saved ?? createExperimentProject("dyadic-fourier");
}

function loadPedagogyPreference(): PedagogySettings {
  try {
    return { ...DEFAULT_PEDAGOGY, ...JSON.parse(localStorage.getItem(PEDAGOGY_PREFERENCE_KEY) ?? "{}") };
  } catch {
    return DEFAULT_PEDAGOGY;
  }
}

function persistPedagogyPreference(value: PedagogySettings): void {
  try { localStorage.setItem(PEDAGOGY_PREFERENCE_KEY, JSON.stringify(value)); } catch { /* Best-effort UI preference. */ }
}

function loadDrawPreference(): boolean {
  try {
    return localStorage.getItem(DRAW_PREFERENCE_KEY) === "true";
  } catch {
    return false;
  }
}

function persistDrawPreference(value: boolean): void {
  try {
    localStorage.setItem(DRAW_PREFERENCE_KEY, String(value));
  } catch {
    // Global UI preferences are best-effort.
  }
}

interface StudioState {
  project: MathProject;
  past: MathProject[];
  future: MathProject[];
  playing: boolean;
  currentTime: number;
  selectedKeyframeId: string | null;
  expressionError: string | null;
  drawScene: boolean;
  pedagogy: PedagogySettings;
  setProject: (project: MathProject, record?: boolean) => void;
  chooseExperiment: (id: string) => void;
  setName: (name: string) => void;
  setFormula: (axis: keyof Pick<FormulaDefinition, "x" | "y" | "z">, value: string) => void;
  setDomain: (key: "tMin" | "tMax", value: number) => void;
  setParameter: (id: string, value: number) => void;
  setParameterRange: (id: string, min: number, max: number) => void;
  setDyadicModeCount: (count: number) => void;
  setStyle: <K extends keyof CurveStyle>(key: K, value: CurveStyle[K]) => void;
  setLayout: (layout: ViewLayout) => void;
  setSamples: (samples: number) => void;
  setEquationOverlay: (visible: boolean) => void;
  setCamera2D: (camera: Camera2DState) => void;
  setCamera3D: (camera: Camera3DState) => void;
  setTimelineOption: (key: "duration" | "fps" | "loop", value: number | boolean) => void;
  addKeyframe: () => void;
  removeKeyframe: (id: string) => void;
  setKeyframeInterpolation: (id: string, interpolation: Interpolation) => void;
  selectKeyframe: (id: string | null) => void;
  setCurrentTime: (time: number) => void;
  setPlaying: (playing: boolean) => void;
  setExpressionError: (error: string | null) => void;
  setDrawScene: (visible: boolean) => void;
  setPedagogy: <K extends keyof PedagogySettings>(key: K, value: PedagogySettings[K]) => void;
  undo: () => void;
  redo: () => void;
}

function persist(project: MathProject): void {
  projectRepository.save(project);
}

export const useStudioStore = create<StudioState>((set, get) => {
  const commit = (mutate: (project: MathProject) => void, record = true) => {
    const previous = get().project;
    const project = cloneProject(previous);
    mutate(project);
    project.updatedAt = new Date().toISOString();
    persist(project);
    set({
      project,
      past: record ? [...get().past.slice(-49), previous] : get().past,
      future: record ? [] : get().future
    });
  };

  return {
    project: loadInitialProject(),
    past: [],
    future: [],
    playing: false,
    currentTime: 0,
    selectedKeyframeId: null,
    expressionError: null,
    drawScene: loadDrawPreference(),
    pedagogy: loadPedagogyPreference(),
    setProject: (project, record = true) => {
      const previous = get().project;
      persist(project);
      set({ project, past: record ? [...get().past.slice(-49), previous] : get().past, future: [], playing: false, currentTime: 0 });
    },
    chooseExperiment: (id) => get().setProject(createExperimentProject(id)),
    setName: (name) => commit((project) => { project.name = name; }),
    setFormula: (axis, value) => commit((project) => { project.formulas[axis] = value; }),
    setDomain: (key, value) => commit((project) => { project.formulas[key] = value; }),
    setParameter: (id, value) => commit((project) => {
      const parameter = project.parameters.find((item) => item.id === id);
      if (parameter) parameter.value = Math.max(parameter.min, Math.min(parameter.max, value));
    }),
    setParameterRange: (id, min, max) => commit((project) => {
      const parameter = project.parameters.find((item) => item.id === id);
      if (!parameter) return;
      if (id === "modeCount" && project.experimentId === "dyadic-fourier") {
        parameter.min = Math.max(1, Math.min(MAX_DYADIC_MODES, Math.min(min, max)));
        parameter.max = Math.max(parameter.min, Math.min(MAX_DYADIC_MODES, Math.max(min, max)));
        parameter.value = Math.max(parameter.min, Math.min(parameter.max, Math.round(parameter.value)));
        project.formulas = buildDyadicFormulas(parameter.value);
      } else {
        parameter.min = Math.min(min, max);
        parameter.max = Math.max(min, max);
        parameter.value = Math.max(parameter.min, Math.min(parameter.max, parameter.value));
      }
    }),
    setDyadicModeCount: (count) => commit((project) => {
      if (project.experimentId !== "dyadic-fourier") return;
      const parameter = project.parameters.find((item) => item.id === "modeCount");
      const lower = Math.max(1, parameter?.min ?? 1);
      const upper = Math.min(MAX_DYADIC_MODES, parameter?.max ?? MAX_DYADIC_MODES);
      const next = Math.max(lower, Math.min(upper, Math.round(count)));
      if (parameter) parameter.value = next;
      project.formulas = buildDyadicFormulas(next);
    }),
    setStyle: (key, value) => commit((project) => { project.style[key] = value; }),
    setLayout: (layout) => commit((project) => { project.layout = layout; }),
    setSamples: (samples) => commit((project) => { project.samples = samples; }),
    setEquationOverlay: (visible) => commit((project) => { project.equationOverlay = visible; }),
    setCamera2D: (camera) => commit((project) => {
      project.camera2d = camera;
      const selected = project.timeline.keyframes.find((frame) => frame.id === get().selectedKeyframeId);
      if (selected) selected.camera2d = camera;
    }, false),
    setCamera3D: (camera) => commit((project) => {
      project.camera3d = camera;
      const selected = project.timeline.keyframes.find((frame) => frame.id === get().selectedKeyframeId);
      if (selected) selected.camera3d = camera;
    }, false),
    setTimelineOption: (key, value) => commit((project) => {
      if (key === "loop") project.timeline.loop = Boolean(value);
      else project.timeline[key] = Number(value);
    }),
    addKeyframe: () => commit((project) => {
      const parameters = Object.fromEntries(project.parameters.map((parameter) => [parameter.id, parameter.value]));
      const id = createId("keyframe");
      project.timeline.keyframes = project.timeline.keyframes.filter((frame) => Math.abs(frame.time - get().currentTime) > 1 / project.timeline.fps);
      project.timeline.keyframes.push({
        id,
        time: get().currentTime,
        interpolation: "smooth",
        parameters,
        camera2d: cloneProject(project).camera2d,
        camera3d: cloneProject(project).camera3d
      });
      project.timeline.keyframes.sort((a, b) => a.time - b.time);
      set({ selectedKeyframeId: id });
    }),
    removeKeyframe: (id) => commit((project) => {
      project.timeline.keyframes = project.timeline.keyframes.filter((frame) => frame.id !== id);
      if (get().selectedKeyframeId === id) set({ selectedKeyframeId: null });
    }),
    setKeyframeInterpolation: (id, interpolation) => commit((project) => {
      const keyframe = project.timeline.keyframes.find((frame) => frame.id === id);
      if (keyframe) keyframe.interpolation = interpolation;
    }),
    selectKeyframe: (id) => set({ selectedKeyframeId: id }),
    setCurrentTime: (time) => set({ currentTime: Math.max(0, Math.min(time, get().project.timeline.duration)) }),
    setPlaying: (playing) => set({ playing }),
    setExpressionError: (expressionError) => set({ expressionError }),
    setDrawScene: (drawScene) => {
      persistDrawPreference(drawScene);
      set({ drawScene });
    },
    setPedagogy: (key, value) => {
      const pedagogy = { ...get().pedagogy, [key]: value };
      persistPedagogyPreference(pedagogy);
      set({ pedagogy });
    },
    undo: () => {
      const past = get().past;
      const previous = past[past.length - 1];
      if (!previous) return;
      set({ project: previous, past: past.slice(0, -1), future: [get().project, ...get().future.slice(0, 49)] });
      persist(previous);
    },
    redo: () => {
      const future = get().future;
      const next = future[0];
      if (!next) return;
      set({ project: next, past: [...get().past.slice(-49), get().project], future: future.slice(1) });
      persist(next);
    }
  };
});
