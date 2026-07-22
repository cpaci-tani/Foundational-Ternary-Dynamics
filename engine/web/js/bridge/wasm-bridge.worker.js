// Scale-0 WASM physics Web Worker. Hosts the threaded engine (ftd_core_mt) off
// the main render thread so a heavy tick never stalls the UI. Phase 1: pool=1
// (pure serial; no nested pthreads — PTHREAD_POOL_SIZE=0 means zero pre-spawn).
// The engine's -pthread heap is a SharedArrayBuffer; the main-thread
// WasmBridgeProxy reads the flux field zero-copy from it. Control + frame
// counters ride a small shared CTRL SAB (Atomics); diag/particles ride
// postMessage. Mirrors mock-bridge.worker.js for the JS MockBridge.
//
// CLASSIC worker (Emscripten MODULARIZE exposes createFTDModuleMT as a global
// via importScripts) — so this file CANNOT be an ES module; CTRL is inlined
// (must match shared-field.js CTRL).

// Load the Emscripten MT glue. If this fails (e.g. a NetworkError because the
// worker context isn't crossOriginIsolated / the -pthread glue's subresource
// fetch is blocked by COEP), post a clean init-error back to the proxy so it can
// fall back to the in-thread WASM engine instead of leaving the engine dead.
// (Without this, importScripts throws uncaught — which surfaces only via the
// worker's onerror, and not reliably in every browser.)
try {
    importScripts('../../wasm/ftd_core_mt.js');
} catch (e) {
    try { self.postMessage({ type: 'error', where: 'init', msg: 'importScripts failed: ' + String((e && e.message) || e) }); } catch (_) { /* ignore */ }
    throw e;   // still abort the worker; the proxy fallback is already signalled
}

const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, TICKS_PER_FRAME: 5, LEN: 8 };
const TARGET_DT = 1000 / 60;

let mod = null, bridge = null;
let N = 33, scenarioId = 'flux-pulse', toggles = {};
let poolThreads = 1;       // Phase 1 = 1 (serial off-thread). Phase 2 raises this
                           // for the 1.8-2.2x in-worker threading (on-demand
                           // nested pthread_create; POOL_SIZE=0 => no pre-spawn).
let ctrlSab = null, ctrl = null;
let timer = 0, tickAcc = 0;

// Overlay sampler registry. Maps proxy kind-key → [C++ method name, 'vec'|'val'|'obj'].
// 'vec' returns {positions, vectors, count}; 'val' returns {positions, values, count};
// 'obj' returns a plain object (no stride argument, e.g. gravityMetricAgg).
const SAMPLER_METHODS = {
  'e':             ['getEFieldSampled',       'vec'],
  'b':             ['getBFieldSampled',        'vec'],
  'poynting':      ['getPoyntingSampled',      'vec'],
  'divJ':          ['getDivJSampled',          'val'],
  'fluxVector':    ['getFluxVectorSampled',    'vec'],
  'vorticity':     ['getVorticitySampled',     'val'],
  'helicity':      ['getHelicitySampled',      'val'],
  'kretschmann':   ['getKretschmannSampled',   'val'],
  'latency':       ['getLatencySampled',       'val'],
  'fisher':        ['getFisherSampled',        'val'],
  'coherence':     ['getCoherenceSampled',     'val'],
  'curlJ':         ['getCurlJSampled',         'vec'],
  'state':         ['getStateFieldSampled',    'val'],
  'gaussResidual': ['getGaussResidualSampled', 'val'],
  'em':            ['getEMForceField',         'vec'],
  'gravity':       ['getGravityForceField',    'vec'],
  'strong':        ['getStrongForceField',     'vec'],
  'gravityMetricAgg': ['getGravityMetricAgg', 'obj'],
};

// Samplers currently wanted by the proxy, keyed by "kind@stride".
// Persists across scenario changes (overlay visibility is UI state, not scenario state).
const wantedSamplers = new Map();

// Knot telemetry/event payloads are WASM heap VIEWS (zero-copy). They are
// invalidated by the next WASM call, so copy every typed array out before the
// payload crosses the postMessage boundary back to the main thread.
function copyKnotTelemetry(r) {
  if (!r || !r.count) return null;
  return { ids: new Int32Array(r.ids), signs: new Int32Array(r.signs), birth: new Int32Array(r.birth),
           age: new Int32Array(r.age), size: new Int32Array(r.size), peak: new Int32Array(r.peak),
           fields: new Float32Array(r.fields), stride: r.stride, count: r.count };
}
function copyKnotEvents(r) {
  if (!r) return null;
  return { tick: new Int32Array(r.tick), type: new Int32Array(r.type), nparents: new Int32Array(r.nparents),
           nchildren: new Int32Array(r.nchildren), sign: new Int32Array(r.sign), count: r.count };
}

function initModule(cb) {
  createFTDModuleMT({ locateFile: (p) => '../../wasm/' + p }).then((m) => {
    mod = m;
    // Must set the pool BEFORE the first parallel_for (first tick) constructs it.
    if (typeof mod.ftdSetPoolThreads === 'function') mod.ftdSetPoolThreads(poolThreads);
    cb();
  }).catch((e) => self.postMessage({ type: 'error', where: 'init', msg: String(e && e.message || e) }));
}

// After a C++ setupScenario, clamp any TermToggles `requires` dependent that is
// ON while its prerequisite is OFF (e.g. selective_damping with damping off).
// The fresh RenderBridge starts at C++ defaults (selective_damping=true) and
// some scenario setups turn a prerequisite off without clearing the dependent,
// which bursts "[TermToggles] Invalid combination" on every tick. Reads the
// bridge's actual toggle state and corrects it; physics-neutral (the dependent
// is already a no-op when its prerequisite is off). Mirrors WasmBridge.
const TOGGLE_REQUIRES = [
  ['selective_damping', 'damping'],
  ['larmor_radiation', 'damping'],
  ['lorentz_force', 'forces'],
  ['weak_transmutation', 'dual_substrate'],
  ['triad_binding', 'dual_substrate'],
  ['latency_field', 'gravity'],
];
function enforceToggleInvariants() {
  if (!mod || !bridge || typeof mod.getToggle !== 'function' || typeof mod.setToggle !== 'function') return;
  for (const [dep, prereq] of TOGGLE_REQUIRES) {
    try {
      if (mod.getToggle(bridge, dep) && !mod.getToggle(bridge, prereq)) mod.setToggle(bridge, dep, false);
    } catch (e) { /* unknown toggle name in this build — skip */ }
  }
}

function buildBridge(n, scen) {
  if (bridge) { try { bridge.delete(); } catch (e) { /* ignore */ } bridge = null; }
  N = n | 0;
  bridge = new mod.RenderBridge(N);
  for (const k in toggles) { try { mod.setToggle(bridge, k, toggles[k]); } catch (e) { /* ignore */ } }
  try { mod.setupScenario(bridge, scen); } catch (e) { /* ignore */ }
  enforceToggleInvariants();
  scenarioId = scen;
  // Flux-volume cache pointer is stable for a fixed N; publish the heap + offset.
  const vol = mod.getFluxVolume(bridge);
  if (!ctrlSab) { ctrlSab = new SharedArrayBuffer(CTRL.LEN * 4); ctrl = new Int32Array(ctrlSab); }
  Atomics.store(ctrl, CTRL.N, N);
  Atomics.store(ctrl, CTRL.RUNNING, 0);
  self.postMessage({ type: 'ready', N, ctrl: ctrlSab, heap: vol.buffer, fluxPtr: vol.byteOffset, fluxLen: vol.length });
  postFrame();
}

function postFrame() {
  if (!bridge) return;
  mod.getFluxVolume(bridge);            // refresh the flux cache in the shared heap
  const tick = bridge.currentTick ? bridge.currentTick() : 0;
  let diag = null, parts = null, audit = null, lag = null;
  try { diag = mod.getDiagnostics(bridge); } catch (e) { /* ignore */ }
  try { audit = mod.getEnergyAudit(bridge); } catch (e) { /* ignore */ }
  try { lag = mod.getLagrangian(bridge); } catch (e) { /* ignore */ }

  if (diag && audit && Number.isFinite(audit.dynamicEnergy)) {
    diag.vacuumBaselineEnergy = diag.totalEnergy;
    diag.dynamicEnergy = audit.dynamicEnergy;
    diag.accountedEnergy = audit.totalEnergy;
    diag.restEnergy = audit.particleRestEnergy;
    diag.totalEnergy = audit.dynamicEnergy;
  }

  try {
    const p = mod.getParticleData(bridge);    // heap VIEWS — copy before posting
    if (p) parts = {
      positions:   p.positions   ? new Float32Array(p.positions)   : new Float32Array(0),
      colors:      p.colors      ? new Float32Array(p.colors)      : new Float32Array(0),
      sizes:       p.sizes       ? new Float32Array(p.sizes)       : new Float32Array(0),
      spin:        p.spin        ? new Float32Array(p.spin)        : new Float32Array(0),
      colorCharge: p.colorCharge ? new Float32Array(p.colorCharge) : new Float32Array(0),
      count: p.count | 0,
    };
  } catch (e) { /* ignore */ }
  // Knot telemetry — only when the tracking build/toggle is present, so the cost
  // is zero otherwise. Copy the heap views out immediately (before the sampler
  // loop's WASM calls below invalidate them).
  let knot = null, knotEvents = null, knotAgg = null;
  try {
    if (mod.getKnotAggregate) {
      knotAgg = mod.getKnotAggregate(bridge);
      knot = copyKnotTelemetry(mod.getKnotTelemetry(bridge));
      knotEvents = copyKnotEvents(mod.getKnotEvents(bridge));
    }
  } catch (e) { /* tracking off or not built */ }
  // Overlay samplers — compute only the kinds the proxy has registered.
  const samplers = {};
  if (wantedSamplers.size > 0) {
    for (const [key, { kind, stride }] of wantedSamplers) {
      const spec = SAMPLER_METHODS[kind];
      if (!spec) continue;
      const [method, type] = spec;
      if (typeof mod[method] !== 'function') continue;
      try {
        if (type === 'obj') {
          const raw = mod[method](bridge);
          if (raw) samplers[key] = raw;
        } else {
          const raw = mod[method](bridge, stride);
          if (!raw || !raw.count) continue;
          // raw.positions / raw.vectors / raw.values are WASM heap views — copy before posting.
          if (type === 'vec') {
            samplers[key] = { positions: new Float32Array(raw.positions), vectors: new Float32Array(raw.vectors), count: raw.count };
          } else {
            samplers[key] = { positions: new Float32Array(raw.positions), values: new Float32Array(raw.values), count: raw.count };
          }
        }
      } catch { /* ignore — method may not be bound in this WASM build */ }
    }
  }

  if (ctrl) {
    Atomics.store(ctrl, CTRL.TICK, tick | 0);
    Atomics.store(ctrl, CTRL.PCOUNT, parts ? parts.count : 0);
    Atomics.add(ctrl, CTRL.FRAME, 1);
  }
  self.postMessage({ type: 'frame', tick: tick | 0, diag, parts, audit, lag, samplers, knot, knotEvents, knotAgg });
}

function loop() {
  timer = 0;
  if (!bridge) { timer = setTimeout(loop, TARGET_DT); return; }
  const t0 = performance.now();
  if (ctrl && Atomics.load(ctrl, CTRL.RUNNING)) {
    const tpfRaw = Atomics.load(ctrl, CTRL.TICKS_PER_FRAME);
    const tpf = tpfRaw > 0 ? tpfRaw / 1000 : 1.0;
    tickAcc += tpf;
    const whole = Math.floor(tickAcc); tickAcc -= whole;
    const maxTicks = N > 96 ? 1 : (N > 48 ? 1 : (N > 32 ? 2 : whole));
    const toRun = Math.min(whole, maxTicks);
    for (let i = 0; i < toRun; i++) bridge.tick();
    if (toRun > 0) postFrame();
  }
  const elapsed = performance.now() - t0;
  timer = setTimeout(loop, Math.max(0, TARGET_DT - elapsed));
}

self.onmessage = (e) => {
  const msg = e.data;
  try {
    switch (msg.type) {
      case 'create':
        toggles = msg.toggles || {};
        if (typeof msg.pool === 'number' && msg.pool >= 1) poolThreads = msg.pool | 0;
        if (!mod) initModule(() => { buildBridge(msg.N, msg.scenarioId || scenarioId); if (!timer) loop(); });
        else { buildBridge(msg.N, msg.scenarioId || scenarioId); if (!timer) loop(); }
        break;
      case 'resize':
        if (mod) buildBridge(msg.N, msg.scenarioId || scenarioId);
        break;
      case 'command': {
        if (!mod || !bridge) break;
        const { method, args = [] } = msg;
        if (method === 'tickScale0') { bridge.tick(); postFrame(); break; }
        if (typeof mod[method] === 'function') { try { mod[method](bridge, ...args); } catch (e) { /* ignore */ } }
        else if (typeof bridge[method] === 'function') { try { bridge[method](...args); } catch (e) { /* ignore */ } }
        postFrame();          // reflect the effect immediately (even while paused)
        break;
      }
      case 'batchCommand': {
        // Replay of commands that were sent before the bridge was ready (e.g.
        // seedSpectrumComparator voxel injections during scenario load).
        // Processes all commands in one synchronous pass, then calls postFrame()
        // once at the end — avoids spamming the main thread with a frame per voxel.
        if (!mod || !bridge) break;
        for (const { method, args = [] } of (msg.commands || [])) {
          if (method === 'tickScale0') { bridge.tick(); continue; }
          if (typeof mod[method] === 'function') { try { mod[method](bridge, ...args); } catch (e) { /* ignore */ } }
          else if (typeof bridge[method] === 'function') { try { bridge[method](...args); } catch (e) { /* ignore */ } }
        }
        postFrame();
        break;
      }
      case 'wantSampler':
        // Proxy registers a sampler kind+stride it wants computed each frame.
        wantedSamplers.set(`${msg.kind}@${msg.stride}`, { kind: msg.kind, stride: msg.stride });
        // When paused the tick loop never calls postFrame(), so the proxy cache
        // stays empty and the overlay never appears. Push a frame immediately so
        // the newly registered sampler is delivered to the proxy right away.
        if (bridge && ctrl && !Atomics.load(ctrl, CTRL.RUNNING)) postFrame();
        break;
      case 'setRunning':
        if (ctrl) Atomics.store(ctrl, CTRL.RUNNING, msg.value ? 1 : 0);
        break;
      case 'dispose':
        if (timer) { clearTimeout(timer); timer = 0; }
        try { if (bridge) bridge.delete(); } catch (e) { /* ignore */ }
        bridge = null;
        break;
    }
  } catch (err) {
    self.postMessage({ type: 'error', where: msg && msg.type, msg: String(err && err.message || err) });
  }
};
