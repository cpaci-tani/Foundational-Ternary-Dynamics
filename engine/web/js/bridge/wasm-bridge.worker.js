// Scale-0 WASM physics Web Worker. Hosts the threaded engine (ftd_core_mt) off
// the main render thread so a heavy tick never stalls the UI. Thread pool is
// pre-spawned (-sPTHREAD_POOL_SIZE=8 in engine/wasm/CMakeLists.txt) so nested
// parallel_for does not deadlock. The engine's -pthread heap is a
// SharedArrayBuffer; the main-thread WasmBridgeProxy reads a double-buffered
// flux SAB (copied off the WASM heap each frame) so playback cannot tear.
// Control + frame counters ride a small shared CTRL SAB (Atomics);
// diag/particles ride postMessage.
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
const FTD_WASM_BASE_URL = new URL('../../wasm/', self.location.href).href;
importScripts(new URL('./sampler-registry.classic.js', self.location.href).href);
importScripts(new URL('./sampler-cadence.classic.js?v=2', self.location.href).href);
// The threaded glue is intentionally NOT imported here. initModule() first
// verifies the manifest, glue, and module bytes, then executes the verified
// glue from a Blob URL and supplies the verified module through `wasmBinary`.

// Emscripten pthreads start from the verified glue Blob URL supplied as
// mainScriptUrlOrBlob. They therefore execute only Emscripten's bootstrap,
// never this dashboard-worker controller.

const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, TICKS_PER_FRAME: 5, DATA_VERSION: 6, LEN: 8 };
const TARGET_DT = 1000 / 60;

let mod = null, bridge = null;
let artifactIdentity = null, verifiedGlueBlobUrl = null;
let N = 33, scenarioId = 'flux-pulse', toggles = {}, toggleNames = [];
let activeConfigurationToken = 0;
const workerRuntimeId = self.crypto?.randomUUID?.()
  || `wasm-worker-${Date.now()}-${Math.random().toString(16).slice(2)}`;
let moduleInitCount = 0;
let renderBridgeGeneration = 0;
let poolThreads = 8;       // MUST equal -sPTHREAD_POOL_SIZE in engine/wasm/CMakeLists.txt
                           // (pre-spawned pthread pool; proxy may clamp below this).
let ctrlSab = null, ctrl = null;
let timer = 0, tickAcc = 0;
let initInFlight = false;
let pendingCreate = null;
let lastFluxHeap = null, lastFluxPtr = -1, lastFluxLen = -1;
let fluxPubSab = null, fluxPubN = 0;

function publishFlux(vol) {
  if (!vol || !vol.length) return false;
  const n = vol.length;
  // 8-byte header: Int32 published-slot + 4 bytes pad. Float64Array views
  // require a multiple-of-8 byteOffset; a 4-byte header throws and the
  // catch below would silently revert to the torn single-buffer heap view.
  const header = 8;
  const bytes = n * 8;
  const need = header + 2 * bytes;
  try {
    if (!fluxPubSab || fluxPubSab.byteLength < need || fluxPubN !== n) {
      fluxPubSab = new SharedArrayBuffer(need);
      fluxPubN = n;
      Atomics.store(new Int32Array(fluxPubSab, 0, 1), 0, 0);
      self.postMessage({
        type: 'fluxRebind', fluxSab: fluxPubSab, fluxLen: n, doubleBuffered: true,
        configurationToken: activeConfigurationToken,
      });
    }
    const slot = 1 - Atomics.load(new Int32Array(fluxPubSab, 0, 1), 0);
    new Float64Array(fluxPubSab, header + slot * bytes, n).set(vol);
    Atomics.store(new Int32Array(fluxPubSab, 0, 1), 0, slot);
    return true;
  } catch (e) {
    fluxPubSab = null;
    fluxPubN = 0;
    return false;
  }
}
let lastInspect = null, lastForceAt = null;
let lastDynamicalStateDigest = null;
let publishDynamicalStateDigest = false;

// Overlay sampler registry. Maps proxy kind-key → [C++ method name, 'vec'|'val'|'obj'].
// 'vec' returns {positions, vectors, count}; 'val' returns {positions, values, count};
// 'obj' returns a plain object (no stride argument, e.g. gravityMetricAgg).
const SAMPLER_METHODS = self.FTD_SAMPLER_METHODS || {};

// Samplers currently wanted by the proxy, keyed by "kind@stride".
// Persists across scenario changes (overlay visibility is UI state, not scenario state).
const wantedSamplers = new Map();
const {
  GRAVITY_SAMPLER_INTERVAL_MS,
  isBoundedInstrumentSamplerWant,
  createBoundedSamplerCadence,
  advanceDemandFrameCadence,
  visitScheduledSamplers,
} = self.FTD_SAMPLER_CADENCE;
const gravitySamplerCadence = createBoundedSamplerCadence(GRAVITY_SAMPLER_INTERVAL_MS);

// Knot telemetry/event payloads are WASM heap VIEWS (zero-copy). They are
// invalidated by the next WASM call, so copy every typed array out before the
// payload crosses the postMessage boundary back to the main thread.
const WORKER_COMMAND_ALLOWLIST = new Set([
  'tickScale0', 'setToggle', 'setDt', 'setOmega0',
  'setLangevinTemp', 'setLangevinGamma', 'setFluxBoundary',
  'injectParticle', 'injectFlux', 'injectWavepacket', 'injectWaveVel',
  'createEntangledPair', 'clearField', 'seedRandomFlux',
]);

function cloneAudit(a) {
  if (!a) return null;
  const out = {};
  const keys = [
    'fieldEnergy', 'waveEnergy', 'particleKE', 'totalEnergy', 'gaussViolation',
    'maxGaussError', 'selfFieldInjection', 'coulombPE', 'EFieldEnergy', 'BFieldEnergy',
    'chargeTotal', 'manifested', 'totalPoynting', 'ELTotal', 'ERTotal', 'wvLTotal',
    'wvRTotal', 'chiralityTotal', 'strongEnergy', 'weakEnergy', 'particleRestEnergy',
    'particleEnergy', 'dynamicEnergy', 'cellVolume', 'fieldEnergyDensitySum',
    'waveEnergyDensitySum', 'particleMomentum',
  ];
  for (const k of keys) {
    if (a[k] === undefined) continue;
    const v = a[k];
    if (v && typeof v === 'object' && Number.isFinite(v.x)) {
      out[k] = { x: v.x, y: v.y, z: v.z };
    } else {
      out[k] = v;
    }
  }
  return out;
}

function applyCommand(method, args = []) {
  if (method === 'tickScale0') {
    try {
      bridge.tick();
      return { ok: true };
    } catch (e) {
      const error = 'tickScale0 failed: ' + String(e && e.message || e);
      console.error('[WasmWorker] ' + error);
      return { ok: false, error };
    }
  }
  if (!WORKER_COMMAND_ALLOWLIST.has(method)) {
    const error = 'rejected command: ' + method;
    console.error('[WasmWorker] ' + error);
    return { ok: false, error };
  }
  const fn = typeof mod[method] === 'function'
    ? () => mod[method](bridge, ...args)
    : (typeof bridge[method] === 'function' ? () => bridge[method](...args) : null);
  if (!fn) {
    const error = 'command handler unavailable: ' + method;
    console.error('[WasmWorker] ' + error);
    return { ok: false, error };
  }
  try {
    const result = fn();
    if (result === false) {
      const error = 'command returned false: ' + method;
      console.error('[WasmWorker] ' + error);
      return { ok: false, error };
    }
    return { ok: true };
  } catch (e) {
    const error = 'command ' + method + ' failed: ' + String(e && e.message || e);
    console.error('[WasmWorker] ' + error);
    return { ok: false, error };
  }
}

function readFluxBoundaryMode() {
  if (!mod || !bridge || typeof mod.getFluxBoundary !== 'function') return null;
  try {
    const mode = Number(mod.getFluxBoundary(bridge));
    return Number.isInteger(mode) ? mode : null;
  } catch (e) {
    return null;
  }
}

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

function normalizeDynamicalStateDigest(raw) {
  if (!raw) return null;
  return {
    schemaVersion: raw.schema_version,
    latticeSize: raw.lattice_size,
    siteCount: raw.site_count,
    tick: raw.tick,
    stateVersion: raw.state_version,
    // This worker owns one WASM RenderBridge, not the native telemetry
    // scheduler. Null is explicit unavailability, never an invented epoch.
    sourceEpoch: null,
    telemetrySourceEpoch: null,
    hashLo: raw.hash_lo,
    hashHi: raw.hash_hi,
    nonfiniteValueCount: raw.nonfinite_value_count,
    nondefaultValueCount: raw.nondefault_value_count,
    deviceToHostBytes: raw.device_to_host_bytes,
    fullMirrorCalls: raw.full_mirror_calls,
    exactDefaultRecord: raw.exact_default_record,
    compute: 'CPU',
    runtime: 'wasm',
    transport: 'worker',
  };
}

function captureDynamicalStateDigest() {
  if (!mod || !bridge || typeof mod.captureDynamicalStateDigest !== 'function') return null;
  try { return normalizeDynamicalStateDigest(mod.captureDynamicalStateDigest(bridge)); }
  catch (e) { return null; }
}

async function sha256Hex(bytes) {
  if (!self.crypto?.subtle) throw new Error('WebCrypto SHA-256 is unavailable');
  const digest = await self.crypto.subtle.digest('SHA-256', bytes);
  return [...new Uint8Array(digest)]
    .map((value) => value.toString(16).padStart(2, '0')).join('');
}

function canonicalBundleBytes(manifest) {
  let value = 'ftd-wasm-bundle-v1\n';
  for (const variant of manifest.variants || []) {
    for (const artifact of variant.artifacts || []) {
      value += `${artifact.file}\0${artifact.sizeBytes}\0${artifact.sha256}\n`;
    }
  }
  return new TextEncoder().encode(value);
}

async function fetchVerifiedBytes(artifact, bundleSha256) {
  const url = new URL(artifact.file, FTD_WASM_BASE_URL);
  url.searchParams.set('bundle', bundleSha256);
  const response = await fetch(url, { cache: 'no-store' });
  if (!response.ok) throw new Error(`artifact fetch failed: ${artifact.file}`);
  const bytes = new Uint8Array(await response.arrayBuffer());
  if (bytes.byteLength !== artifact.sizeBytes) {
    throw new Error(`artifact size mismatch: ${artifact.file}`);
  }
  if (await sha256Hex(bytes) !== artifact.sha256) {
    throw new Error(`artifact hash mismatch: ${artifact.file}`);
  }
  return bytes;
}

async function loadVerifiedThreadedBundle() {
  const manifestUrl = new URL('build_info.json', FTD_WASM_BASE_URL);
  const response = await fetch(manifestUrl, { cache: 'no-store' });
  if (!response.ok) throw new Error(`build manifest fetch failed: HTTP ${response.status}`);
  const manifest = await response.json();
  if (manifest.schemaVersion !== 1
      || !/^[0-9a-f]{64}$/.test(String(manifest.bundleSha256 || ''))
      || !/^[0-9a-f]{40}$/.test(String(manifest.source?.commit || ''))
      || typeof manifest.source?.dirty !== 'boolean') {
    throw new Error('build manifest identity is invalid');
  }
  if (await sha256Hex(canonicalBundleBytes(manifest)) !== manifest.bundleSha256) {
    throw new Error('build manifest canonical bundle hash mismatch');
  }
  const variant = manifest.variants?.find((candidate) => candidate?.id === 'wasm32-threads');
  if (!variant || variant.factory !== 'createFTDModuleMT'
      || variant.abi?.pointerBits !== 32 || variant.abi?.threads !== true
      || variant.abi?.sharedMemory !== true || variant.artifacts?.length !== 2) {
    throw new Error('threaded WASM variant contract is invalid');
  }
  const bytesByRole = {};
  for (const artifact of variant.artifacts) {
    bytesByRole[artifact.role] = await fetchVerifiedBytes(artifact, manifest.bundleSha256);
  }
  if (!bytesByRole.loader || !bytesByRole.module) {
    throw new Error('threaded WASM artifact roles are incomplete');
  }
  return {
    identity: {
      schemaVersion: manifest.schemaVersion,
      bundleSha256: manifest.bundleSha256,
      source: manifest.source,
      toolchain: manifest.toolchain,
      variant,
      manifestUrl: manifestUrl.href,
    },
    loaderText: new TextDecoder().decode(bytesByRole.loader),
    moduleBytes: bytesByRole.module,
  };
}

function initModule(cb) {
  loadVerifiedThreadedBundle().then((verified) => {
    artifactIdentity = verified.identity;
    verifiedGlueBlobUrl = URL.createObjectURL(new Blob(
      [verified.loaderText], { type: 'text/javascript' },
    ));
    importScripts(verifiedGlueBlobUrl);
    if (typeof createFTDModuleMT !== 'function') {
      throw new Error('verified threaded loader did not publish createFTDModuleMT');
    }
    return createFTDModuleMT({
      wasmBinary: verified.moduleBytes,
      locateFile: (p) => FTD_WASM_BASE_URL + p,
      // The verified Blob URL is also the pthread bootstrap. It must stay live
      // for the module lifetime and is revoked only on dispose.
      mainScriptUrlOrBlob: verifiedGlueBlobUrl,
    });
  }).then((m) => {
    mod = m;
    moduleInitCount++;
    // Must set the pool BEFORE the first parallel_for (first tick) constructs it.
    if (typeof mod.ftdSetPoolThreads === 'function') mod.ftdSetPoolThreads(poolThreads);
    cb();
  }).catch((e) => self.postMessage({
    type: 'error',
    where: 'init',
    msg: String(e && e.message || e),
    configurationToken: Number(pendingCreate?.configurationToken) || 0,
  }));
}

// After a C++ setupScenario, clamp any TermToggles `requires` dependent that is
// ON while its prerequisite is OFF (e.g. selective_damping with damping off).
// The fresh RenderBridge starts at C++ defaults (selective_damping=true) and
// some scenario setups turn a prerequisite off without clearing the dependent,
// which bursts "[TermToggles] Invalid combination" on every tick. Reads the
// bridge's actual toggle state and corrects it; physics-neutral (the dependent
// is already a no-op when its prerequisite is off). Mirrors WasmBridge.
const TOGGLE_REQUIRES = self.FTD_TOGGLE_REQUIRES || [];
function enforceToggleInvariants() {
  if (!mod || !bridge || typeof mod.getToggle !== 'function' || typeof mod.setToggle !== 'function') return;
  for (const [dep, prereq] of TOGGLE_REQUIRES) {
    try {
      if (mod.getToggle(bridge, dep) && !mod.getToggle(bridge, prereq)) mod.setToggle(bridge, dep, false);
    } catch (e) { /* unknown toggle name in this build — skip */ }
  }
}

// Engine-truth toggle readback.
//
// `mod.setupScenario` rebuilds the RenderBridge at C++ defaults and the C++
// scenario body then sets its own profile, so the toggles the main thread SENT
// are not the toggles the engine is RUNNING. Without publishing the readback,
// the proxy's getToggle can only echo the JS model back at the dashboard, and
// the physics-toggles card asserts engine state the engine does not have.
// Recomputed only when something could have changed (build/resize/command),
// not per frame — it is one Embind crossing per key.
let engineToggles = {};
let engineTogglesDirty = true;

// Telemetry demand mask (see telemetry/demand.js). O(N^3) audit and Gravity
// reductions default OFF because neither has an always-on consumer. The proxy
// publishes visible-panel demand and hydrates immediately when one opens.
// Lagrangian retains its compatibility default and existing gate behavior.
let wantAudit = false;
let wantLag = true;
let wantGravity = false;
let gravityMetricAggVersion = null;

// Energy-audit cadence cache — see postFrame(). getEnergyAudit is a full O(N^3)
// pass, so large lattices sample it less often. The cached audit is published
// with its original sample tick/version between samples; it must never be
// relabelled as a current diagnostic observation. Reset on every rebuild so a
// new scenario/lattice never reuses a stale-N audit.
let lastAudit = null;
let auditFrameCounter = 0;
let diagnosticsStateVersion = 0;
let auditStateVersion = 0;
let lagrangianStateVersion = 0;
let lastAuditMeta = null;
let lastLagrangianMeta = null;

function telemetryGroupMeta({ stateVersion, tick, stale = false, status = 'available' }) {
  return {
    backend: 'wasm-worker',
    sourceEpoch: activeConfigurationToken,
    stateVersion,
    sampleTick: Number.isFinite(tick) ? tick : null,
    tick: Number.isFinite(tick) ? tick : null,
    sampledAt: performance.now(),
    stale,
    status,
  };
}

function readEngineToggles() {
  if (!mod || !bridge || typeof mod.getToggle !== 'function') return null;
  const out = {};
  // `toggles` contains only pre-setup writes from the dashboard. C++ scenario
  // bodies also enable non-UI/research terms (for example Langevin baths), so
  // limiting truth to Object.keys(toggles) made those real engine terms appear
  // false. The proxy supplies the complete TOGGLE_SPECS name registry.
  for (const k of toggleNames) {
    try { out[k] = !!mod.getToggle(bridge, k); } catch (e) { /* not in this build */ }
  }
  engineToggles = out;
  engineTogglesDirty = false;
  return out;
}

function buildBridge(n, scen, configurationToken = 0) {
  activeConfigurationToken = Number(configurationToken) || 0;
  tickAcc = 0;
  lastInspect = null;
  lastForceAt = null;
  gravitySamplerCadence.reset();
  gravityMetricAggVersion = null;
  if (bridge) { try { bridge.delete(); } catch (e) { /* ignore */ } bridge = null; }
  N = n | 0;
  bridge = new mod.RenderBridge(N);
  renderBridgeGeneration++;
  const toggleErrors = [];
  for (const k in toggles) {
    try { mod.setToggle(bridge, k, toggles[k]); }
    catch (e) { toggleErrors.push(k + ': ' + (e && e.message || e)); }
  }
  if (toggleErrors.length) {
    self.postMessage({
      type: 'error', where: 'setToggle', msg: toggleErrors.slice(0, 5).join('; '),
      configurationToken: activeConfigurationToken,
    });
  }
  let setupOk = true;
  let setupError = null;
  try {
    const result = mod.setupScenario(bridge, scen);
    // Older WASM builds return undefined; only an explicit false is failure.
    if (result === false) {
      setupOk = false;
      setupError = 'Unknown or unhandled scenario: ' + scen;
      // Surface via ready.setupOk — avoid a duplicate onSetupFailure from a
      // parallel type:'error' message for the same failure.
    }
  } catch (e) {
    setupOk = false;
    setupError = String(e && e.message || e);
    self.postMessage({
      type: 'error', where: 'setupScenario', msg: setupError,
      configurationToken: activeConfigurationToken,
    });
  }
  enforceToggleInvariants();
  engineTogglesDirty = true;   // the C++ body just replaced the whole profile
  lastAudit = null; auditFrameCounter = 0;   // force a fresh audit for the new N/profile
  lastAuditMeta = null;
  lastLagrangianMeta = null;
  scenarioId = scen;
  // O(N^3), so capture once per newly built scenario and thereafter only on
  // an explicit `captureDigest` request. Never put digest work in the 60 Hz
  // loop. The initial value lets the proxy expose a truthful cached getter as
  // soon as its first frame arrives.
  lastDynamicalStateDigest = captureDynamicalStateDigest();
  publishDynamicalStateDigest = true;
  // Flux-volume cache pointer is stable for a fixed N; publish the heap + offset.
  const vol = mod.getFluxVolume(bridge);
  if (!ctrlSab) { ctrlSab = new SharedArrayBuffer(CTRL.LEN * 4); ctrl = new Int32Array(ctrlSab); }
  Atomics.store(ctrl, CTRL.N, N);
  Atomics.store(ctrl, CTRL.RUNNING, 0);
  lastFluxHeap = vol.buffer;
  lastFluxPtr = vol.byteOffset;
  lastFluxLen = vol.length;
  const doubled = publishFlux(vol);
  self.postMessage({
    type: 'ready', N, ctrl: ctrlSab, heap: vol.buffer, fluxPtr: vol.byteOffset, fluxLen: vol.length,
    setupOk, setupError, artifactIdentity, configurationToken,
    workerRuntimeId, moduleInitCount, renderBridgeGeneration,
    ...(doubled ? { fluxSab: fluxPubSab, doubleBuffered: true } : {}),
  });
  // Standing wants belong to UI state and survive scenario replacement. Force
  // one coherent current-generation population after the new bridge is ready.
  postFrame(true, true);
}

function postFrame(
  fieldChanged = false,
  forceGravitySamplerBatch = false,
  allowUndemandedBoundedInstrument = false,
) {
  if (!bridge) return;
  const vol = mod.getFluxVolume(bridge);            // refresh the flux cache in the shared heap
  const doubled = publishFlux(vol);
  if (!doubled && vol && (vol.buffer !== lastFluxHeap || vol.byteOffset !== lastFluxPtr || vol.length !== lastFluxLen)) {
    lastFluxHeap = vol.buffer;
    lastFluxPtr = vol.byteOffset;
    lastFluxLen = vol.length;
    self.postMessage({
      type: 'fluxRebind', heap: vol.buffer, fluxPtr: vol.byteOffset, fluxLen: vol.length,
      configurationToken: activeConfigurationToken,
    });
  }
  const tick = bridge.currentTick ? bridge.currentTick() : 0;
  let diag = null, parts = null, audit = null, lag = null;
  let diagMeta = null;
  try {
    diag = mod.getDiagnostics(bridge);
    diagMeta = telemetryGroupMeta({
      stateVersion: ++diagnosticsStateVersion, tick,
      stale: !diag,
      status: diag ? 'available' : 'unavailable',
    });
  } catch (e) {
    diagMeta = telemetryGroupMeta({
      stateVersion: ++diagnosticsStateVersion, tick, stale: true, status: 'error',
    });
  }
  // getEnergyAudit is a full O(N^3) pass and, alongside the tick itself, the
  // dominant per-frame cost on large lattices. With no audit consumer it must
  // execute zero times. While demanded, run it at a reduced large-N cadence
  // and publish the last successful observation unchanged between samples. A
  // reused observation remains explicitly tied to its original sample
  // tick/version; it is not copied into a newly-current diagnostics packet.
  const auditEvery = N > 96 ? 8 : (N > 48 ? 4 : 1);
  let auditSampledThisFrame = false;
  const auditGate = advanceDemandFrameCadence(
    wantAudit, auditFrameCounter, !!lastAudit, auditEvery,
  );
  auditFrameCounter = auditGate.nextCounter;
  if (auditGate.sample) {
    try {
      const a = mod.getEnergyAudit(bridge);
      if (a && Number.isFinite(a.dynamicEnergy)) {
        lastAudit = cloneAudit(a) || {
          dynamicEnergy: a.dynamicEnergy, totalEnergy: a.totalEnergy,
          particleRestEnergy: a.particleRestEnergy, fieldEnergy: a.fieldEnergy,
          waveEnergy: a.waveEnergy, particleKE: a.particleKE,
        };
        lastAuditMeta = telemetryGroupMeta({
          stateVersion: ++auditStateVersion, tick,
        });
        auditSampledThisFrame = true;
      } else {
        // Fail closed: an unavailable/non-finite attempt supersedes the prior
        // observation. Keeping lastAudit here would pair retained values with
        // the new failure metadata and leak them into diagnostics/consumers.
        lastAudit = null;
        lastAuditMeta = telemetryGroupMeta({
          stateVersion: ++auditStateVersion, tick, stale: true,
          status: a ? 'nonfinite' : 'unavailable',
        });
      }
    } catch (e) {
      lastAudit = null;
      lastAuditMeta = telemetryGroupMeta({
        stateVersion: ++auditStateVersion, tick, stale: true, status: 'error',
      });
    }
  } else if (!wantAudit) {
    // Inactive is a new fail-closed observation boundary, not a stale value
    // relabelled with the current tick. Publish it once per source generation.
    lastAudit = null;
    if (!lastAuditMeta || lastAuditMeta.status !== 'inactive'
        || lastAuditMeta.sourceEpoch !== activeConfigurationToken) {
      lastAuditMeta = telemetryGroupMeta({
        stateVersion: ++auditStateVersion, tick, stale: true, status: 'inactive',
      });
    }
  }
  audit = wantAudit ? lastAudit : null;
  // The Lagrangian genuinely has no consumer beyond its panels, so it IS gated.
  if (wantLag) {
    try {
      lag = mod.getLagrangian(bridge);
      lastLagrangianMeta = telemetryGroupMeta({
        stateVersion: ++lagrangianStateVersion, tick,
        stale: !lag,
        status: lag ? 'available' : 'unavailable',
      });
    } catch (e) {
      lastLagrangianMeta = telemetryGroupMeta({
        stateVersion: ++lagrangianStateVersion, tick, stale: true, status: 'error',
      });
    }
  } else if (lastLagrangianMeta) {
    lastLagrangianMeta = {
      ...lastLagrangianMeta,
      stale: true,
      status: 'inactive',
    };
  }

  // Audit-derived diagnostics are valid only when both observations describe
  // this exact tick. Reused/staggered audit samples remain separate telemetry
  // and must not be promoted to the diagnostics packet's newer provenance.
  if (diag && auditSampledThisFrame && audit
      && lastAuditMeta?.status === 'available' && lastAuditMeta.stale !== true
      && lastAuditMeta.sampleTick === tick
      && Number.isFinite(audit.dynamicEnergy)) {
    diag.vacuumBaselineEnergy = diag.totalEnergy;
    diag.dynamicEnergy = audit.dynamicEnergy;
    diag.accountedEnergy = audit.totalEnergy;
    diag.restEnergy = audit.particleRestEnergy;
    diag.totalEnergy = audit.dynamicEnergy;
    // Status-bar decomposition (whole-box channels, sim units): lets the UI
    // show field/wave/KE without a second audit fetch.
    diag.fieldEnergy = audit.fieldEnergy;
    diag.waveEnergy = audit.waveEnergy;
    diag.particleKE = audit.particleKE;
  }

  try {
    const p = mod.getParticleData(bridge);    // heap VIEWS — copy before posting
    if (p) parts = {
      positions:   p.positions   ? new Float32Array(p.positions)   : new Float32Array(0),
      colors:      p.colors      ? new Float32Array(p.colors)      : new Float32Array(0),
      sizes:       p.sizes       ? new Float32Array(p.sizes)       : new Float32Array(0),
      spin:        p.spin        ? new Float32Array(p.spin)        : new Float32Array(0),
      colorCharge: p.colorCharge ? new Float32Array(p.colorCharge) : new Float32Array(0),
      locked:      p.locked      ? new Uint8Array(p.locked)        : new Uint8Array(0),
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
  // Overlay samplers — ordinary/direct/viewport owners follow publication
  // cadence. Only Time/Gravity instrument-owned readbacks share the bounded
  // 4 Hz decision; a realtime co-owner wins in sampler-want-set. The helper
  // also injects gravityMetricAgg@0 when Time alone owns telemetry demand.
  const samplers = {};
  let gravityMetricAggSampled = false;
  if (wantedSamplers.size > 0 || wantGravity) {
    visitScheduledSamplers(wantedSamplers, {
      wantGravity,
      cadence: gravitySamplerCadence,
      nowMs: performance.now(),
      forceGravityBatch: forceGravitySamplerBatch,
      allowUndemandedBoundedInstrument,
    }, (key, { kind, stride }) => {
      const spec = SAMPLER_METHODS[kind];
      if (!spec) return;
      const [method, type] = spec;
      if (typeof mod[method] !== 'function') return;
      try {
        if (type === 'obj') {
          const raw = mod[method](bridge);
          if (raw) {
            samplers[key] = raw;
            if (kind === 'gravityMetricAgg') gravityMetricAggSampled = true;
          }
        } else {
          const raw = mod[method](bridge, stride);
          if (!raw) return;
          // raw.positions / raw.vectors / raw.values are WASM heap views — copy before posting.
          if (type === 'vec') {
            samplers[key] = { positions: new Float32Array(raw.positions || 0), vectors: new Float32Array(raw.vectors || 0), count: raw.count | 0 };
          } else {
            samplers[key] = { positions: new Float32Array(raw.positions || 0), values: new Float32Array(raw.values || 0), count: raw.count | 0 };
          }
        }
      } catch { /* ignore — method may not be bound in this WASM build */ }
    });
  }

  if (ctrl) {
    Atomics.store(ctrl, CTRL.TICK, tick | 0);
    Atomics.store(ctrl, CTRL.PCOUNT, parts ? parts.count : 0);
    if (fieldChanged) Atomics.add(ctrl, CTRL.DATA_VERSION, 1);
    Atomics.add(ctrl, CTRL.FRAME, 1);
  }
  const dataVersion = ctrl ? Atomics.load(ctrl, CTRL.DATA_VERSION) : 0;
  if (gravityMetricAggSampled) gravityMetricAggVersion = dataVersion;
  const engineTogglesMsg = engineTogglesDirty ? readEngineToggles() : null;
  const digestMsg = publishDynamicalStateDigest ? lastDynamicalStateDigest : undefined;
  publishDynamicalStateDigest = false;
  self.postMessage({ type: 'frame', configurationToken: activeConfigurationToken,
                     tick: tick | 0, diag, diagMeta, parts,
                     dataVersion,
                     audit, auditMeta: lastAuditMeta,
                     lag, lagMeta: lastLagrangianMeta,
                     samplers, knot, knotEvents, knotAgg,
                     inspect: lastInspect, forceAt: lastForceAt,
                     ...(digestMsg !== undefined ? { dynamicalStateDigest: digestMsg } : {}),
                     ...(engineTogglesMsg ? { engineToggles: engineTogglesMsg } : {}) });
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
    if (toRun > 0) postFrame(true);
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
        toggleNames = Array.isArray(msg.toggleNames) && msg.toggleNames.length
          ? [...msg.toggleNames]
          : Object.keys(toggles);
        if (typeof msg.pool === 'number' && msg.pool >= 1) poolThreads = msg.pool | 0;
        pendingCreate = msg;
        if (mod) {
          pendingCreate = null;
          buildBridge(msg.N, msg.scenarioId || scenarioId, msg.configurationToken);
          if (!timer) loop();
        } else if (!initInFlight) {
          initInFlight = true;
          initModule(() => {
            initInFlight = false;
            const m = pendingCreate;
            pendingCreate = null;
            if (m) buildBridge(m.N, m.scenarioId || scenarioId, m.configurationToken);
            if (!timer) loop();
          });
        }
        break;
      case 'resize':
        if (mod && Number(msg.configurationToken) >= activeConfigurationToken) {
          buildBridge(msg.N, msg.scenarioId || scenarioId, msg.configurationToken);
        }
        break;
      case 'command': {
        if (!mod || !bridge
            || Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
        applyCommand(msg.method, msg.args || []);
        engineTogglesDirty = true;
        postFrame(true);
        break;
      }
      case 'batchCommand': {
        if (!mod || !bridge
            || Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
        const errors = [];
        const expectedToggles = new Map();
        let expectedFluxBoundaryMode = null;
        for (const { method, args = [] } of (msg.commands || [])) {
          const result = applyCommand(method, args);
          if (!result?.ok) errors.push(result?.error || `command failed: ${method}`);
          if (method === 'setToggle' && typeof args[0] === 'string') {
            expectedToggles.set(args[0], !!args[1]);
          } else if (method === 'setFluxBoundary' && Number.isInteger(Number(args[0]))) {
            expectedFluxBoundaryMode = Number(args[0]);
          }
        }
        enforceToggleInvariants();
        engineTogglesDirty = true;
        postFrame(true);
        for (const [name, expected] of expectedToggles) {
          if (!(name in engineToggles) || engineToggles[name] !== expected) {
            errors.push(`toggle readback mismatch: ${name} expected ${expected}`);
          }
        }
        const fluxBoundaryMode = readFluxBoundaryMode();
        if (expectedFluxBoundaryMode !== null && fluxBoundaryMode !== expectedFluxBoundaryMode) {
          errors.push(`flux boundary readback mismatch: expected ${expectedFluxBoundaryMode}, got ${fluxBoundaryMode}`);
        }
        self.postMessage({
          type: 'configurationApplied',
          configurationToken: msg.configurationToken,
          ok: errors.length === 0,
          errors,
          engineToggles: { ...engineToggles },
          fluxBoundaryMode,
        });
        break;
      }
      case 'inspectVoxel': {
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastInspect = null;
        if (mod && bridge && typeof mod.inspectVoxel === 'function') {
          try {
            lastInspect = { x: msg.x | 0, y: msg.y | 0, z: msg.z | 0, voxel: mod.inspectVoxel(bridge, msg.x, msg.y, msg.z) };
          } catch { lastInspect = null; }
        }
        self.postMessage({
          type: 'inspectResult', inspect: lastInspect,
          configurationToken: msg.configurationToken,
        });
        break;
      }
      case 'getForceAt': {
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastForceAt = null;
        if (mod && bridge && typeof mod.getForceAt === 'function') {
          try {
            lastForceAt = { x: msg.x | 0, y: msg.y | 0, z: msg.z | 0, force: mod.getForceAt(bridge, msg.x, msg.y, msg.z) };
          } catch { lastForceAt = null; }
        }
        self.postMessage({
          type: 'forceAtResult', forceAt: lastForceAt,
          configurationToken: msg.configurationToken,
        });
        break;
      }
      case 'coarsen': {
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        // One-shot Scale-0 → Scale-1 coarse-graining snapshot (voxel debug
        // view / promotion pipeline). The embind export builds plain typed
        // arrays in this worker's JS context, so the result is structured-
        // cloneable as-is.
        let data = null;
        if (mod && bridge && typeof mod.coarsenToParticles === 'function') {
          try { data = mod.coarsenToParticles(bridge); } catch (e) { data = null; }
        }
        self.postMessage({
          type: 'coarsenResult', reqId: msg.reqId, data,
          configurationToken: msg.configurationToken,
        });
        break;
      }
      case 'captureDigest': {
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        // Explicit scientific observation request. It is deliberately outside
        // postFrame()/loop() so canonical O(N^3) hashing never taxes 60 Hz
        // rendering. The result is a plain structured-cloneable object whose
        // uint64 lanes were serialized to hex by Embind.
        lastDynamicalStateDigest = captureDynamicalStateDigest();
        self.postMessage({
          type: 'digestResult',
          reqId: msg.reqId,
          digest: lastDynamicalStateDigest,
          configurationToken: msg.configurationToken,
        });
        break;
      }
      case 'wantSampler': {
        const key = `${msg.kind}@${msg.stride}`;
        const added = !wantedSamplers.has(key);
        const cadenceClass = msg.cadenceClass === 'bounded-instrument'
          ? 'bounded-instrument' : 'realtime';
        const want = { kind: msg.kind, stride: msg.stride, cadenceClass };
        const boundedInstrumentAdded = added && (
          isBoundedInstrumentSamplerWant(want)
          || (msg.kind === 'gravityMetricAgg' && cadenceClass === 'bounded-instrument')
        );
        wantedSamplers.set(key, want);
        if (boundedInstrumentAdded) gravitySamplerCadence.reset();
        // When paused the tick loop never calls postFrame(), so the proxy cache
        // stays empty and the overlay never appears. Push a frame immediately so
        // the newly registered sampler is delivered to the proxy right away.
        if (added && bridge && ctrl && !Atomics.load(ctrl, CTRL.RUNNING)) {
          postFrame(false, boundedInstrumentAdded, boundedInstrumentAdded);
        }
        break;
      }
      case 'unwantSampler':
        // Counterpart to 'wantSampler' — a caller no longer needs this
        // kind+stride computed every frame (e.g. a UI overlay row was
        // hidden). Without this, wantedSamplers only ever grows for the
        // life of the worker.
        wantedSamplers.delete(`${msg.kind}@${msg.stride}`);
        break;
      case 'replaceSamplerWants': {
        let added = false;
        let boundedInstrumentAdded = false;
        for (const change of Array.isArray(msg.changes) ? msg.changes : []) {
          const kind = String(change?.kind || '');
          const stride = Number(change?.stride);
          if (!kind || !Number.isFinite(stride)) continue;
          const key = `${kind}@${stride}`;
          if (change.op === 'want') {
            const isNew = !wantedSamplers.has(key);
            const cadenceClass = change.cadenceClass === 'bounded-instrument'
              ? 'bounded-instrument' : 'realtime';
            const want = { kind, stride, cadenceClass };
            wantedSamplers.set(key, want);
            added ||= isNew;
            boundedInstrumentAdded ||= isNew && (
              isBoundedInstrumentSamplerWant(want)
              || (kind === 'gravityMetricAgg' && cadenceClass === 'bounded-instrument')
            );
          } else if (change.op === 'unwant') {
            wantedSamplers.delete(key);
          }
        }
        if (boundedInstrumentAdded) gravitySamplerCadence.reset();
        // One owner-set replacement is one atomic scientific demand change.
        // A paused worker publishes the complete new union exactly once.
        if (added && bridge && ctrl && !Atomics.load(ctrl, CTRL.RUNNING)) {
          postFrame(false, boundedInstrumentAdded, boundedInstrumentAdded);
        }
        break;
      }
      case 'setTelemetryMask': {
        const nextWantAudit = msg.wantAudit !== false;
        const auditChanged = nextWantAudit !== wantAudit;
        const nextWantGravity = msg.wantGravity === true;
        const gravityBecameWanted = nextWantGravity && !wantGravity;
        wantAudit = nextWantAudit;
        wantLag   = msg.wantLag   !== false;
        wantGravity = nextWantGravity;
        if (auditChanged) {
          // Never reuse an observation across an inactive boundary. The next
          // demanded postFrame samples current state; the inactive path emits
          // an explicit null/status boundary.
          lastAudit = null;
          auditFrameCounter = 0;
        }
        let publishPausedMaskChange = auditChanged;
        if (gravityBecameWanted) {
          gravitySamplerCadence.reset();
          const dataVersion = ctrl ? Atomics.load(ctrl, CTRL.DATA_VERSION) : 0;
          // Time may be opened while playback is paused and owns no direct
          // aggregate want. Populate once immediately unless the just-added
          // Gravity batch already supplied this exact data generation.
          publishPausedMaskChange ||= gravityMetricAggVersion !== dataVersion;
        }
        if (publishPausedMaskChange && bridge && ctrl
            && !Atomics.load(ctrl, CTRL.RUNNING)) {
          postFrame(false, gravityBecameWanted);
        }
        break;
      }
      case 'setRunning':
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        if (ctrl) Atomics.store(ctrl, CTRL.RUNNING, msg.value ? 1 : 0);
        // This handler cannot run until any in-progress loop()/postFrame() has
        // returned. Posting the acknowledgement here makes it a FIFO barrier:
        // the proxy receives all committed frames before the settled state.
        self.postMessage({
          type: 'runningState', running: !!msg.value, seq: msg.seq | 0,
          configurationToken: activeConfigurationToken,
        });
        break;
      case 'dispose':
        if (timer) { clearTimeout(timer); timer = 0; }
        try { if (bridge) bridge.delete(); } catch (e) { /* ignore */ }
        bridge = null;
        if (verifiedGlueBlobUrl) {
          try { URL.revokeObjectURL(verifiedGlueBlobUrl); } catch (e) { /* ignore */ }
          verifiedGlueBlobUrl = null;
        }
        wantedSamplers.clear();
        pendingCreate = null;
        self.postMessage({
          type: 'disposed',
          configurationToken: Number(msg.configurationToken) || activeConfigurationToken,
          workerRuntimeId,
          moduleInitCount,
          renderBridgeGeneration,
        });
        break;
    }
  } catch (err) {
    self.postMessage({
      type: 'error',
      where: msg && msg.type,
      msg: String(err && err.message || err),
      configurationToken: Number(msg?.configurationToken) || activeConfigurationToken,
    });
  }
};
