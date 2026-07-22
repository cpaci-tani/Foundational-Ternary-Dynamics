# Mathematical Animation Studio Architecture

**Status:** initial standalone architecture  
**Boundary:** `apps/math-studio` is independent of the FTD engine dashboard.

## Runtime Shape

```text
React application shell
  -> versioned MathProject document
  -> Zustand command/history store
  -> experiment registry
  -> expression sampler (Math.js)
  -> Canvas2DViewport + ThreeViewport
  -> timeline evaluator (parameters + cameras)
  -> composite recorder (Canvas captureStream -> MediaRecorder)
```

## Ownership Boundaries

- `core/types.ts` defines the public project document and experiment contracts.
- `core/math` contains pure expression sampling and geometry bounds; it has no DOM dependency.
- `core/animation` contains deterministic timeline interpolation; it has no renderer dependency.
- `core/store` owns commands, undo/redo, playback state, and project mutation.
- Cross-project workspace preferences, including progressive drawing, live beside the project document and persist independently so model changes cannot reset them.
- `core/persistence` is an adapter boundary. The first adapter is browser local storage; IndexedDB, filesystem, or remote repositories can implement the same contract later.
- `experiments` contributes project factories through `ExperimentDefinition`. Experiments do not own application chrome or rendering loops.
- `experiments/dyadic-model.ts` generates exact finite expressions and typed controls for one to 25 dyadic modes; inactive modes are omitted from evaluation rather than merely hidden in the interface.
- `rendering` owns imperative Canvas 2D and Three.js resources. Renderers consume sampled geometry and camera state rather than evaluating formulas.
- `recording` composites renderer canvases and equation text into a deterministic 16:9 output surface.

## Performance Model

- Expression compilation and sampling are pure and memoized by React inputs.
- Playback limits live geometry to 1,024 points; idle editing uses the configured sample count up to 4,096.
- Three.js geometry buffers are replaced only when sampled positions change; camera and marker updates do not rebuild geometry.
- Canvas backing stores cap device-pixel ratio at 2.
- Renderer resources and observers are disposed on unmount.
- The sampler boundary can move behind a Web Worker without changing project, timeline, or renderer interfaces when profiling justifies it.

## Project Compatibility

`MathProject.schemaVersion` is the persistence compatibility boundary. Import rejects unknown schemas rather than silently coercing them. Future schema changes must add an explicit migration before incrementing the version.

## Extension Path

1. Add an experiment by registering an `ExperimentDefinition` that returns a valid `MathProject`.
2. Add new render modes behind renderer components that consume `CurveSample`.
3. Add persistence targets by implementing `ProjectRepository`.
4. Add export codecs behind the composite recording surface.
5. Move expensive sampling to a worker while preserving `sampleParametricCurve` semantics and tests.
