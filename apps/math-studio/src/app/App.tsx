import {
  Box,
  Braces,
  ChevronDown,
  Download,
  Eye,
  EyeOff,
  FileUp,
  Focus,
  KeyRound,
  Pause,
  PenLine,
  Play,
  Redo2,
  RotateCcw,
  Trash2,
  Undo2
} from "lucide-react";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { evaluateCameraTimeline, evaluateTimeline } from "../core/animation/timeline";
import { sampleParametricCurve } from "../core/math/sampler";
import { analyzeCurve } from "../core/math/differential-geometry";
import { keyboardStep } from "../core/input/keyboard-step";
import { parseProject, serializeProject } from "../core/project";
import { useStudioStore } from "../core/store/studio-store";
import type { Interpolation, ParameterDefinition, Vec3, ViewLayout } from "../core/types";
import { listExperiments } from "../experiments/registry";
import { dyadicDisplayEquations, MAX_DYADIC_MODES } from "../experiments/dyadic-model";
import { CompositeRecorder } from "../recording/CompositeRecorder";
import { Canvas2DViewport } from "../rendering/Canvas2DViewport";
import { ThreeViewport } from "../rendering/ThreeViewport";
import { EllipticIntegralPanel } from "./EllipticIntegralPanel";

const experiments = listExperiments();

function downloadText(text: string, filename: string) {
  const url = URL.createObjectURL(new Blob([text], { type: "application/json" }));
  const anchor = document.createElement("a");
  anchor.href = url;
  anchor.download = filename;
  anchor.click();
  URL.revokeObjectURL(url);
}

export function App() {
  const project = useStudioStore((state) => state.project);
  const playing = useStudioStore((state) => state.playing);
  const currentTime = useStudioStore((state) => state.currentTime);
  const selectedKeyframeId = useStudioStore((state) => state.selectedKeyframeId);
  const expressionError = useStudioStore((state) => state.expressionError);
  const drawScene = useStudioStore((state) => state.drawScene);
  const pedagogy = useStudioStore((state) => state.pedagogy);
  const store = useStudioStore();
  const canvas2dRef = useRef<HTMLCanvasElement>(null);
  const canvas3dRef = useRef<HTMLCanvasElement>(null);
  const fileRef = useRef<HTMLInputElement>(null);
  const [fitNonce, setFitNonce] = useState(0);
  const [leftTab, setLeftTab] = useState<"parameters" | "formulas">("parameters");
  const [modeMatrixView, setModeMatrixView] = useState<"value" | "min" | "max">("value");
  const parameterSignature = project.parameters.map((parameter) => `${parameter.id}:${parameter.value}`).join("|");
  const parameterTimelineSignature = JSON.stringify(project.timeline.keyframes.map((frame) => [frame.time, frame.interpolation, frame.parameters]));
  const formulaSignature = `${project.formulas.x}|${project.formulas.y}|${project.formulas.z}|${project.formulas.tMin}|${project.formulas.tMax}`;
  const styleSignature = JSON.stringify(project.style);
  const baseParameters = useMemo(
    () => Object.fromEntries(project.parameters.map((parameter) => [parameter.id, parameter.value])),
    [parameterSignature]
  );
  const animatedParameters = useMemo(
    () => evaluateTimeline(project.timeline, currentTime, baseParameters),
    [baseParameters, currentTime, parameterTimelineSignature]
  );
  const animatedCameras = useMemo(
    () => evaluateCameraTimeline(project.timeline, currentTime, project.camera2d, project.camera3d),
    [currentTime, project.camera2d, project.camera3d, project.timeline]
  );
  const sample = useMemo(
    () => sampleParametricCurve(project.formulas, animatedParameters, playing ? Math.min(project.samples, 1024) : project.samples),
    [animatedParameters, playing, formulaSignature, project.samples]
  );
  const renderStyle = useMemo(() => ({ ...project.style, drawScene }), [styleSignature, drawScene]);
  const phase = project.timeline.duration > 0 ? currentTime / project.timeline.duration : 0;
  const analysis = useMemo(() => analyzeCurve(sample, phase), [phase, sample]);
  const activeExperiment = experiments.find((experiment) => experiment.id === project.experimentId) ?? experiments[0];
  const modeCount = Math.round(project.parameters.find((parameter) => parameter.id === "modeCount")?.value ?? 0);
  const isDyadic = project.experimentId === "dyadic-fourier";
  const isElliptic = project.experimentId === "elliptic-integrals";
  const displayEquations = useMemo<string[]>(() => isDyadic
    ? dyadicDisplayEquations(modeCount)
    : isElliptic
      ? [
        "G* = Γ(1/4)/Γ(3/4),   a = λG*,   m = k²",
        "F(φ|m) = ∫₀^φ (1 - m sin²θ)^(-1/2) dθ",
        "E(φ|m) = ∫₀^φ (1 - m sin²θ)^(1/2) dθ",
        "Π(n;φ|m) = ∫₀^φ [(1 - n sin²θ)√(1 - m sin²θ)]^(-1) dθ"
      ]
    : [`x(t) = ${project.formulas.x}`, `y(t) = ${project.formulas.y}`, `z(t) = ${project.formulas.z}`],
  [isDyadic, isElliptic, modeCount, formulaSignature]);
  const highestActiveMode = isDyadic
    ? Array.from({ length: modeCount }, (_, index) => index).filter((index) => Math.abs(animatedParameters[`a${index}`] ?? 0) > 1e-12).at(-1) ?? 0
    : 0;
  const maxDyadicFrequency = isDyadic ? 2 ** highestActiveMode * (animatedParameters.timeScale ?? 1) : 0;
  const aliasRisk = isDyadic && maxDyadicFrequency > sample.count / 2;

  useEffect(() => {
    store.setExpressionError(sample.error);
  }, [sample.error]);

  useEffect(() => {
    if (!playing) return;
    let frame = 0;
    let previous = performance.now();
    const tick = (now: number) => {
      const delta = Math.min(0.1, (now - previous) / 1000);
      previous = now;
      const state = useStudioStore.getState();
      let next = state.currentTime + delta;
      if (next >= state.project.timeline.duration) {
        if (state.project.timeline.loop) next %= state.project.timeline.duration;
        else {
          next = state.project.timeline.duration;
          state.setPlaying(false);
        }
      }
      state.setCurrentTime(next);
      frame = requestAnimationFrame(tick);
    };
    frame = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(frame);
  }, [playing]);

  useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === "z") {
        event.preventDefault();
        if (event.shiftKey) store.redo();
        else store.undo();
      }
      if (event.code === "Space" && !(event.target instanceof HTMLInputElement) && !(event.target instanceof HTMLTextAreaElement)) {
        event.preventDefault();
        store.setPlaying(!useStudioStore.getState().playing);
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, []);

  const importProject = async (file: File) => {
    try {
      store.setProject(parseProject(await file.text()));
      setFitNonce((value) => value + 1);
    } catch (error) {
      window.alert(error instanceof Error ? error.message : "Unable to load project");
    }
  };

  const frameViews = () => setFitNonce((value) => value + 1);
  const setLayout = (layout: ViewLayout) => store.setLayout(layout);
  const orient3d = (orientation: "front" | "side" | "top" | "iso") => {
    const [x, y, z] = sample.bounds.center;
    const distance = Math.max(sample.bounds.radius * 3, 1);
    const positions = {
      front: [x, y - distance, z],
      side: [x + distance, y, z],
      top: [x, y, z + distance],
      iso: [x + distance * 0.72, y - distance * 0.82, z + distance * 0.56]
    } as const;
    store.setCamera3D({ position: [...positions[orientation]], target: [x, y, z], fov: project.camera3d.fov });
  };

  return (
    <div className="studio-shell">
      <header className="topbar">
        <div className="product-lockup">
          <Box size={19} />
          <div><strong>Curve Geometry Studio</strong><span>{activeExperiment.category} · {project.name}</span></div>
        </div>
        <div className="topbar-actions">
          <button className="icon-button" type="button" aria-label="Undo" onClick={store.undo} disabled={!store.past.length}><Undo2 size={16} /></button>
          <button className="icon-button" type="button" aria-label="Redo" onClick={store.redo} disabled={!store.future.length}><Redo2 size={16} /></button>
          <span className="toolbar-separator" />
          <button className="command-button" type="button" onClick={frameViews}><Focus size={16} />Frame</button>
          <button className="icon-button" type="button" aria-label="Import project" onClick={() => fileRef.current?.click()}><FileUp size={16} /></button>
          <button className="icon-button" type="button" aria-label="Export project" onClick={() => downloadText(serializeProject(project), `${project.name.toLowerCase().replace(/[^a-z0-9]+/g, "-")}.math.json`)}><Download size={16} /></button>
          <input ref={fileRef} className="visually-hidden" type="file" accept="application/json,.json" aria-hidden="true" tabIndex={-1} onChange={(event) => {
            const file = event.target.files?.[0];
            if (file) void importProject(file);
            event.target.value = "";
          }} />
        </div>
      </header>

      <main className="workspace">
        <aside className="inspector left-inspector">
          <div className="panel-section">
            <label className="field-label" htmlFor="experiment">Workspace</label>
            <div className="select-wrap"><select id="experiment" value={project.experimentId} onChange={(event) => { store.chooseExperiment(event.target.value); setFitNonce((value) => value + 1); }}>
              {experiments.map((experiment) => <option key={experiment.id} value={experiment.id}>{experiment.name}</option>)}
            </select><ChevronDown size={14} /></div>
            <input className="text-input project-name" aria-label="Project name" value={project.name} onChange={(event) => store.setName(event.target.value)} />
            <div className="model-description"><strong>{activeExperiment.category}</strong><span>{activeExperiment.description}</span></div>
          </div>

          <div className="segmented-tabs" role="tablist" aria-label="Edit mode">
            <button type="button" role="tab" aria-selected={leftTab === "parameters"} onClick={() => setLeftTab("parameters")}>Parameters</button>
            <button type="button" role="tab" aria-selected={leftTab === "formulas"} onClick={() => setLeftTab("formulas")}>Formulas</button>
          </div>

          {leftTab === "parameters" ? (
            <div className="panel-section parameter-workbench">
              {isDyadic ? <>
                <div className="section-heading"><span>Global parameter register</span><span>{modeCount} / {MAX_DYADIC_MODES} modes</span></div>
                <ScientificParameterTable
                  parameters={project.parameters.filter((parameter) => parameter.category === "mode-count" || parameter.category === "global")}
                  animatedValues={animatedParameters}
                  onValue={(parameter, value) => parameter.id === "modeCount" ? store.setDyadicModeCount(value) : store.setParameter(parameter.id, value)}
                  onRange={(parameter, min, max) => store.setParameterRange(parameter.id, min, parameter.id === "modeCount" ? Math.min(MAX_DYADIC_MODES, max) : max)}
                />
                <div className="mode-register-head">
                  <div><strong>Dyadic mode register</strong><span>aₖ · φₖ · cₖ</span></div>
                  <div className="matrix-view-control" aria-label="Mode matrix entry type">
                    {(["value", "min", "max"] as const).map((view) => <button key={view} type="button" aria-pressed={modeMatrixView === view} onClick={() => setModeMatrixView(view)}>{view === "value" ? "Values" : view === "min" ? "Lower" : "Upper"}</button>)}
                  </div>
                </div>
                <ModeMatrix
                  parameters={project.parameters}
                  modeCount={modeCount}
                  view={modeMatrixView}
                  onValue={(id, value) => store.setParameter(id, value)}
                  onRange={(id, min, max) => store.setParameterRange(id, min, max)}
                />
              </> : <ScientificParameterTable
                parameters={project.parameters}
                animatedValues={animatedParameters}
                onValue={(parameter, value) => store.setParameter(parameter.id, value)}
                onRange={(parameter, min, max) => store.setParameterRange(parameter.id, min, max)}
              />}
            </div>
          ) : (
            <div className="panel-section formula-editor">
              {(["x", "y", "z"] as const).map((axis) => (
                <label key={axis}><span>{axis}(t)</span><textarea value={project.formulas[axis]} onChange={(event) => store.setFormula(axis, event.target.value)} rows={3} /></label>
              ))}
              <div className="two-fields">
                <label><span>t minimum</span><NumericDraftInput label="Domain minimum" value={project.formulas.tMin} step={0.01} onCommit={(value) => store.setDomain("tMin", value)} /></label>
                <label><span>t maximum</span><NumericDraftInput label="Domain maximum" value={project.formulas.tMax} step={0.01} onCommit={(value) => store.setDomain("tMax", value)} /></label>
              </div>
              {expressionError && <div className="error-message">{expressionError}</div>}
            </div>
          )}

          <div className="panel-section">
            <div className="section-heading"><span>Render</span><span>{project.samples.toLocaleString()} points</span></div>
            <div className="render-register">
              <label><span>Samples</span><NumericDraftInput label="Render samples" value={project.samples} step={256} min={256} max={4096} onCommit={(value) => store.setSamples(Math.round(value))} /></label>
              <label><span>Line width</span><NumericDraftInput label="Line width" value={project.style.lineWidth} step={0.1} min={0.5} max={8} onCommit={(value) => store.setStyle("lineWidth", value)} /></label>
              <label><span>Marker radius</span><NumericDraftInput label="Marker radius" value={project.style.pointSize} step={1} min={1} max={16} onCommit={(value) => store.setStyle("pointSize", value)} /></label>
            </div>
            <input className="samples-range" aria-label="Render samples slider" type="range" min="256" max="4096" step="256" value={project.samples} onChange={(event) => store.setSamples(Number(event.target.value))} />
            <div className="style-row">
              <label><span>Primary</span><input type="color" value={project.style.color} onChange={(event) => store.setStyle("color", event.target.value)} /></label>
              <label><span>Secondary</span><input type="color" value={project.style.secondaryColor} onChange={(event) => store.setStyle("secondaryColor", event.target.value)} /></label>
            </div>
            <label className="check-row"><input type="checkbox" checked={project.style.showGrid} onChange={(event) => store.setStyle("showGrid", event.target.checked)} />Grid</label>
            <label className="check-row"><input type="checkbox" checked={project.style.showAxes} onChange={(event) => store.setStyle("showAxes", event.target.checked)} />Axes</label>
          </div>
        </aside>

        <section className="stage-column">
          <div className="stage-toolbar">
            <div className="segmented-control" aria-label="Viewport layout">
              <button type="button" aria-pressed={project.layout === "2d"} onClick={() => setLayout("2d")}>2D</button>
              <button type="button" aria-pressed={project.layout === "split"} onClick={() => setLayout("split")}>Split</button>
              <button type="button" aria-pressed={project.layout === "3d"} onClick={() => setLayout("3d")}>3D</button>
            </div>
            <span className={`status-readout ${expressionError || aliasRisk ? "error" : ""}`}>{expressionError ? "Expression error" : aliasRisk ? `Alias risk · fmax ${maxDyadicFrequency.toLocaleString()} · ${sample.count} samples` : `${sample.count.toLocaleString()} samples · radius ${sample.bounds.radius.toFixed(3)}`}</span>
            <div className="stage-actions">
              <label className="draw-toggle"><input aria-label="Global draw" type="checkbox" checked={drawScene} onChange={(event) => store.setDrawScene(event.target.checked)} /><PenLine size={15} />Draw</label>
              <button className="icon-button" type="button" aria-label={project.equationOverlay ? "Hide equations" : "Show equations"} onClick={() => store.setEquationOverlay(!project.equationOverlay)}>
                {project.equationOverlay ? <Eye size={16} /> : <EyeOff size={16} />}
              </button>
            </div>
          </div>
          <div className={`viewports layout-${project.layout}`}>
            {project.layout !== "3d" && <Canvas2DViewport sample={sample} style={renderStyle} camera={animatedCameras.camera2d} phase={phase} analysis={analysis} pedagogy={pedagogy} canvasRef={canvas2dRef} fitNonce={fitNonce} onCameraChange={store.setCamera2D} />}
            {project.layout !== "2d" && <ThreeViewport sample={sample} style={renderStyle} cameraState={animatedCameras.camera3d} phase={phase} analysis={analysis} pedagogy={pedagogy} canvasRef={canvas3dRef} fitNonce={fitNonce} onCameraChange={store.setCamera3D} />}
            {project.equationOverlay && (
              <div className="equation-overlay">
                {displayEquations.map((equation) => <span key={equation}>{equation}</span>)}
              </div>
            )}
          </div>
          <Timeline />
        </section>

        <aside className="inspector right-inspector">
          <CompositeRecorder source2d={canvas2dRef} source3d={canvas3dRef} equations={displayEquations} />
          {isElliptic && <EllipticIntegralPanel
            parameter={animatedParameters.m ?? 0}
            amplitude={animatedParameters.phi ?? 0}
            characteristic={animatedParameters.n ?? 0}
            semiMajorAxis={(animatedParameters.Gstar ?? 1) * (animatedParameters.scale ?? 1)}
            gStarParameter={animatedParameters.Gstar ?? 1}
          />}
          <div className="panel-section geometry-lab">
            <div className="section-heading"><span>Differential geometry</span><span>Frenet frame</span></div>
            {analysis ? <div className="geometry-readout" role="table" aria-label="Local curve invariants">
              <GeometryValue symbol="t" label="parameter" value={formatScalar(analysis.t)} />
              <GeometryValue symbol="r(t)" label="position" value={formatVector(analysis.position)} />
              <GeometryValue symbol="r'(t)" label="velocity" value={formatVector(analysis.velocity)} />
              <GeometryValue symbol="|r'|" label="speed" value={formatScalar(analysis.speed)} />
              <GeometryValue symbol="κ" label="curvature" value={formatScalar(analysis.curvature)} />
              <GeometryValue symbol="τ" label="torsion" value={formatScalar(analysis.torsion)} />
              <GeometryValue symbol="s / L" label="arc length" value={`${formatScalar(analysis.arcLength)} / ${formatScalar(analysis.totalLength)}`} />
              <GeometryValue symbol="ρ" label="curvature radius" value={analysis.osculatingRadius ? formatScalar(analysis.osculatingRadius) : "∞"} />
            </div> : <div className="error-message">At least five valid samples are required.</div>}
            <div className="frame-controls" aria-label="Geometric construction overlays">
              <label className="vector-toggle"><input type="checkbox" checked={pedagogy.pointMarker} onChange={(event) => store.setPedagogy("pointMarker", event.target.checked)} /><span>P</span>Point</label>
              <label className="vector-toggle"><input type="checkbox" checked={pedagogy.radiusVector} onChange={(event) => store.setPedagogy("radiusVector", event.target.checked)} /><span>r</span>Radius vector</label>
              <label className="vector-toggle"><input type="checkbox" checked={pedagogy.projections} onChange={(event) => store.setPedagogy("projections", event.target.checked)} /><span>⊥</span>Projections</label>
              <label className="vector-toggle"><input type="checkbox" checked={pedagogy.parameterAngle} onChange={(event) => store.setPedagogy("parameterAngle", event.target.checked)} /><span>θ</span>Angle</label>
              <label className="vector-toggle"><input type="checkbox" checked={pedagogy.arcLengthLabel} onChange={(event) => store.setPedagogy("arcLengthLabel", event.target.checked)} /><span>s</span>Arc length</label>
              <label className="vector-toggle tangent"><input type="checkbox" checked={pedagogy.tangent} onChange={(event) => store.setPedagogy("tangent", event.target.checked)} /><span>T</span>Tangent</label>
              <label className="vector-toggle normal"><input type="checkbox" checked={pedagogy.normal} onChange={(event) => store.setPedagogy("normal", event.target.checked)} /><span>N</span>Normal</label>
              <label className="vector-toggle binormal"><input type="checkbox" checked={pedagogy.binormal} onChange={(event) => store.setPedagogy("binormal", event.target.checked)} /><span>B</span>Binormal</label>
              <label className="vector-toggle circle"><input type="checkbox" checked={pedagogy.osculatingCircle} onChange={(event) => store.setPedagogy("osculatingCircle", event.target.checked)} /><span>O</span>Osculating circle</label>
            </div>
            <label className="vector-scale"><span>Frame scale</span><NumericDraftInput label="Frenet frame scale" value={pedagogy.vectorScale} step={0.1} min={0.1} max={8} onCommit={(value) => store.setPedagogy("vectorScale", value)} /></label>
          </div>
          <div className="panel-section">
            <div className="section-heading"><span>Camera framing</span><span>orientation</span></div>
            <button className="command-button full-width" type="button" onClick={frameViews}><Focus size={16} />Frame active curve</button>
            <div className="orientation-grid" aria-label="Camera orientation presets">
              <button type="button" onClick={() => orient3d("front")}>Front</button>
              <button type="button" onClick={() => orient3d("side")}>Side</button>
              <button type="button" onClick={() => orient3d("top")}>Top</button>
              <button type="button" onClick={() => orient3d("iso")}>Iso</button>
            </div>
            <div className="camera-readout"><span>2D center</span><code>{project.camera2d.center.map((value) => value.toFixed(2)).join(", ")}</code></div>
            <div className="camera-readout"><span>3D position</span><code>{project.camera3d.position.map((value) => value.toFixed(2)).join(", ")}</code></div>
            <div className="camera-readout"><span>3D target</span><code>{project.camera3d.target.map((value) => value.toFixed(2)).join(", ")}</code></div>
          </div>
          <div className="panel-section architecture-note">
            <Braces size={16} />
            <div><strong>Project document v1</strong><span>Expressions, parameters, cameras, style, and keyframes are stored together.</span></div>
          </div>
        </aside>
      </main>
    </div>
  );
}

function formatScalar(value: number): string {
  if (!Number.isFinite(value)) return "undefined";
  const magnitude = Math.abs(value);
  return magnitude !== 0 && (magnitude >= 1e4 || magnitude < 1e-3) ? value.toExponential(4) : value.toFixed(5);
}

function formatVector(vector: Vec3): string {
  return `(${vector.map(formatScalar).join(", ")})`;
}

function GeometryValue({ symbol, label, value }: { symbol: string; label: string; value: string }) {
  return <div className="geometry-value" role="row"><span className="geometry-symbol" role="cell">{symbol}</span><span role="cell">{label}</span><code role="cell">{value}</code></div>;
}

function NumericDraftInput({ label, value, step, min, max, onCommit }: { label: string; value: number; step: number; min?: number; max?: number; onCommit: (value: number) => void }) {
  const [draft, setDraft] = useState(String(value));
  useEffect(() => { setDraft(String(value)); }, [value]);
  const commit = () => {
    const parsed = Number(draft);
    if (Number.isFinite(parsed)) {
      const clamped = Math.max(min ?? -Infinity, Math.min(max ?? Infinity, parsed));
      setDraft(String(clamped));
      if (Math.abs(clamped - value) > 1e-12) onCommit(clamped);
    }
    else setDraft(String(value));
  };
  const stepWithKeyboard = (direction: 1 | -1, shiftKey: boolean, altKey: boolean) => {
    const current = Number(draft);
    const next = keyboardStep(Number.isFinite(current) ? current : value, step, direction, { shiftKey, altKey }, { min, max });
    setDraft(String(next));
    onCommit(next);
  };
  return <input
    aria-label={label}
    type="number"
    step="any"
    min={min}
    max={max}
    aria-keyshortcuts="ArrowUp ArrowDown Shift+ArrowUp Shift+ArrowDown Alt+ArrowUp Alt+ArrowDown"
    value={draft}
    onChange={(event) => setDraft(event.target.value)}
    onBlur={commit}
    onKeyDown={(event) => {
      if (event.key === "Enter") event.currentTarget.blur();
      else if (event.key === "ArrowUp" || event.key === "ArrowDown") {
        event.preventDefault();
        stepWithKeyboard(event.key === "ArrowUp" ? 1 : -1, event.shiftKey, event.altKey);
      }
    }}
  />;
}

function ScientificParameterTable({ parameters, animatedValues, onValue, onRange }: {
  parameters: ParameterDefinition[];
  animatedValues: Record<string, number>;
  onValue: (parameter: ParameterDefinition, value: number) => void;
  onRange: (parameter: ParameterDefinition, min: number, max: number) => void;
}) {
  return <div className="parameter-register" role="table" aria-label="Parameter register">
    <div className="parameter-register-header" role="row">
      <span>Symbol</span><span>Parameter</span><span>Value</span><span>Min</span><span>Max</span>
    </div>
    {parameters.map((parameter) => {
      const live = animatedValues[parameter.id] ?? parameter.value;
      const decimals = parameter.step < 0.01 ? 3 : parameter.step < 1 ? 2 : 0;
      return <div className="parameter-register-row" role="row" key={parameter.id}>
        <span className="math-symbol" role="cell">{scientificSymbol(parameter)}</span>
        <span className="parameter-name" role="cell">{parameter.label}<small>Δ {parameter.step}</small></span>
        <span role="cell"><NumericDraftInput label={`${parameter.label} value`} value={parameter.value} step={parameter.step} onCommit={(value) => onValue(parameter, value)} /></span>
        <span role="cell"><NumericDraftInput label={`${parameter.label} minimum`} value={parameter.min} step={parameter.step} onCommit={(min) => onRange(parameter, min, parameter.max)} /></span>
        <span role="cell"><NumericDraftInput label={`${parameter.label} maximum`} value={parameter.max} step={parameter.step} onCommit={(max) => onRange(parameter, parameter.min, max)} /></span>
        <input className="register-slider" aria-label={`${parameter.label} slider`} type="range" min={parameter.min} max={parameter.max} step={parameter.step} value={parameter.value} onChange={(event) => onValue(parameter, Number(event.target.value))} />
        <output className="live-value">live {Number(live).toFixed(decimals)}</output>
      </div>;
    })}
  </div>;
}

function scientificSymbol(parameter: ParameterDefinition): string {
  const symbols: Record<string, string> = {
    modeCount: "K", timeScale: "s", rotation: "θ", xScale: "sₓ", yScale: "sᵧ", zScale: "s_z",
    xOffset: "oₓ", yOffset: "oᵧ", zOffset: "o_z", depth: "d_z", liftFrequency: "f_z", liftPhase: "φ_z",
    angle: "θ₀", phase: "φ", gamma: "γ", omega: "ω", w1: "ω₁", w2: "ω₂", flightTime: "T",
    height: "h₀", growth: "κ", turns: "N", pitch: "p", taper: "κ", r0: "r₀",
    x0: "x₀", x1: "x₁", x2: "x₂", x3: "x₃", y0: "y₀", y1: "y₁", y2: "y₂", y3: "y₃"
  };
  return symbols[parameter.id] ?? parameter.symbol ?? parameter.id;
}

function ModeMatrix({ parameters, modeCount, view, onValue, onRange }: {
  parameters: ParameterDefinition[];
  modeCount: number;
  view: "value" | "min" | "max";
  onValue: (id: string, value: number) => void;
  onRange: (id: string, min: number, max: number) => void;
}) {
  const getModeParameter = (index: number, category: ParameterDefinition["category"]) => parameters.find((parameter) => parameter.modeIndex === index && parameter.category === category)!;
  const renderCell = (parameter: ParameterDefinition) => {
    const displayed = view === "value" ? parameter.value : view === "min" ? parameter.min : parameter.max;
    const commit = (value: number) => {
      if (view === "value") onValue(parameter.id, value);
      else if (view === "min") onRange(parameter.id, value, parameter.max);
      else onRange(parameter.id, parameter.min, value);
    };
    return <NumericDraftInput label={`${parameter.label} ${view}`} value={displayed} step={parameter.step} onCommit={commit} />;
  };
  return <div className="mode-matrix" role="table" aria-label={`Dyadic mode ${view} matrix`}>
    <div className="mode-matrix-row mode-matrix-header" role="row"><span>k</span><span>2ᵏ</span><span>aₖ</span><span>φₖ</span><span>cₖ</span></div>
    {Array.from({ length: modeCount }, (_, index) => {
      const amplitude = getModeParameter(index, "mode-amplitude");
      const phase = getModeParameter(index, "mode-phase");
      const chirality = getModeParameter(index, "mode-chirality");
      return <div className={`mode-matrix-row ${Math.abs(amplitude.value) > 1e-12 ? "active" : ""}`} role="row" key={index}>
        <span className="mode-index">{index}</span>
        <span className="mode-frequency">{(2 ** index).toLocaleString()}</span>
        <span>{renderCell(amplitude)}</span><span>{renderCell(phase)}</span><span>{renderCell(chirality)}</span>
      </div>;
    })}
  </div>;
}

function Timeline() {
  const project = useStudioStore((state) => state.project);
  const currentTime = useStudioStore((state) => state.currentTime);
  const playing = useStudioStore((state) => state.playing);
  const selectedId = useStudioStore((state) => state.selectedKeyframeId);
  const store = useStudioStore();
  const selected = project.timeline.keyframes.find((frame) => frame.id === selectedId);
  const togglePlay = () => {
    if (!playing && currentTime >= project.timeline.duration) store.setCurrentTime(0);
    store.setPlaying(!playing);
  };
  return (
    <section className="timeline">
      <div className="timeline-transport">
        <button className="icon-button transport" type="button" aria-label={playing ? "Pause" : "Play"} onClick={togglePlay}>{playing ? <Pause size={17} /> : <Play size={17} />}</button>
        <button className="icon-button" type="button" aria-label="Return to start" onClick={() => { store.setPlaying(false); store.setCurrentTime(0); }}><RotateCcw size={15} /></button>
        <code>{currentTime.toFixed(2)} / {project.timeline.duration.toFixed(2)} s</code>
        <button className="command-button" type="button" onClick={store.addKeyframe}><KeyRound size={15} />Add keyframe</button>
      </div>
      <div className="timeline-track-wrap">
        <input className="timeline-scrubber" aria-label="Timeline position" type="range" min="0" max={project.timeline.duration} step={1 / project.timeline.fps} value={currentTime} onChange={(event) => { store.setPlaying(false); store.setCurrentTime(Number(event.target.value)); }} />
        <div className="keyframe-track" aria-label="Keyframes">
          {project.timeline.keyframes.map((frame) => (
            <button
              key={frame.id}
              className="keyframe-marker"
              type="button"
              aria-label={`Keyframe at ${frame.time.toFixed(2)} seconds`}
              aria-pressed={selectedId === frame.id}
              style={{ left: `${frame.time / project.timeline.duration * 100}%` }}
              onClick={() => { store.selectKeyframe(frame.id); store.setCurrentTime(frame.time); }}
            />
          ))}
        </div>
      </div>
      <div className="timeline-options">
        <label>Duration <NumericDraftInput label="Timeline duration" value={project.timeline.duration} step={0.5} min={0.5} max={600} onCommit={(value) => store.setTimelineOption("duration", value)} /></label>
        <label>FPS <select value={project.timeline.fps} onChange={(event) => store.setTimelineOption("fps", Number(event.target.value))}><option>24</option><option>30</option><option>60</option></select></label>
        <label className="check-row"><input type="checkbox" checked={project.timeline.loop} onChange={(event) => store.setTimelineOption("loop", event.target.checked)} />Loop</label>
        {selected && <>
          <label>Interpolation <select value={selected.interpolation} onChange={(event) => store.setKeyframeInterpolation(selected.id, event.target.value as Interpolation)}><option value="smooth">Smooth</option><option value="linear">Linear</option><option value="hold">Hold</option></select></label>
          <button className="icon-button danger" type="button" aria-label="Delete selected keyframe" onClick={() => store.removeKeyframe(selected.id)}><Trash2 size={15} /></button>
        </>}
      </div>
    </section>
  );
}
