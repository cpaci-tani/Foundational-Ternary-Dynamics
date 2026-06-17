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

importScripts('../../wasm/ftd_core_mt.js');

const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, TICKS_PER_FRAME: 5, LEN: 8 };
const TARGET_DT = 1000 / 60;

let mod = null, bridge = null;
let N = 33, scenarioId = 'flux-pulse', toggles = {};
let poolThreads = 1;       // Phase 1 = 1 (serial off-thread). Phase 2 raises this
                           // for the 1.8-2.2x in-worker threading (on-demand
                           // nested pthread_create; POOL_SIZE=0 => no pre-spawn).
let ctrlSab = null, ctrl = null;
let timer = 0, tickAcc = 0;

function initModule(cb) {
  createFTDModuleMT({ locateFile: (p) => '../../wasm/' + p }).then((m) => {
    mod = m;
    // Must set the pool BEFORE the first parallel_for (first tick) constructs it.
    if (typeof mod.ftdSetPoolThreads === 'function') mod.ftdSetPoolThreads(poolThreads);
    cb();
  }).catch((e) => self.postMessage({ type: 'error', where: 'init', msg: String(e && e.message || e) }));
}

function buildBridge(n, scen) {
  if (bridge) { try { bridge.delete(); } catch (e) { /* ignore */ } bridge = null; }
  N = n | 0;
  bridge = new mod.RenderBridge(N);
  for (const k in toggles) { try { mod.setToggle(bridge, k, toggles[k]); } catch (e) { /* ignore */ } }
  try { mod.setupScenario(bridge, scen); } catch (e) { /* ignore */ }
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

  if (diag && audit && Number.isFinite(audit.totalEnergy)) {
    // Native Diagnostics::total_energy is the Born-Infeld vacuum
    // baseline summed over every voxel. Replace it with the scenario
    // budget (field + wave + particle KE) from EnergyAudit so the UI
    // matches the MockBridge convention.
    diag.vacuumBaselineEnergy = diag.totalEnergy;
    diag.totalEnergy = audit.totalEnergy;
  }

  try {
    const p = mod.getParticleData(bridge);    // heap VIEWS — copy before posting
    if (p) parts = {
      positions: p.positions ? new Float32Array(p.positions) : new Float32Array(0),
      colors:    p.colors    ? new Float32Array(p.colors)    : new Float32Array(0),
      sizes:     p.sizes     ? new Float32Array(p.sizes)     : new Float32Array(0),
      count: p.count | 0,
    };
  } catch (e) { /* ignore */ }
  if (ctrl) {
    Atomics.store(ctrl, CTRL.TICK, tick | 0);
    Atomics.store(ctrl, CTRL.PCOUNT, parts ? parts.count : 0);
    Atomics.add(ctrl, CTRL.FRAME, 1);
  }
  self.postMessage({ type: 'frame', tick: tick | 0, diag, parts, audit, lag });
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
