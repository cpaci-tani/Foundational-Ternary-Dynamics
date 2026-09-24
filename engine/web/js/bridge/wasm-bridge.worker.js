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
importScripts(new URL('./sampler-cadence.classic.js?v=6', self.location.href).href);
importScripts(new URL('./flux-publication.classic.js?v=2', self.location.href).href);
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
let backgroundSuspended = false;
let initInFlight = false;
let pendingCreate = null;
let lastFluxHeap = null, lastFluxPtr = -1, lastFluxLen = -1;
let fluxPubSab = null, fluxPubN = 0;

function publishFlux(vol, sampleTick = null) {
  if (!vol || !vol.length) return false;
  const n = vol.length;
  try {
    if (!fluxPubSab || fluxPubN !== n) {
      fluxPubSab = self.FTD_FLUX_PUBLICATION.create(n);
      fluxPubN = n;
      self.postMessage({
        type: 'fluxRebind', fluxSab: fluxPubSab, fluxLen: n, doubleBuffered: true,
        fluxProtocol: self.FTD_FLUX_PUBLICATION.PROTOCOL,
        configurationToken: activeConfigurationToken,
      });
    }
    self.FTD_FLUX_PUBLICATION.publish(fluxPubSab, vol, sampleTick);
    return true;
  } catch (e) {
    fluxPubSab = null;
    fluxPubN = 0;
    return false;
  }
}

const GRAVITY_OBSERVATION_TOGGLE_KEYS = Object.freeze([
  'forces', 'gravity', 'geometric_gravity', 'latency_field', 'field_energy_gravity',
]);

function gravityObservationToggles() {
  const snapshot = {};
  for (const key of GRAVITY_OBSERVATION_TOGGLE_KEYS) {
    try {
      // This is deliberately a same-turn engine read, rather than the cached
      // UI toggle packet: a retained gravity observation must describe the
      // force law which produced its selected field.
      snapshot[key] = typeof mod?.getToggle === 'function' ? !!mod.getToggle(bridge, key) : null;
    } catch { snapshot[key] = null; }
  }
  return snapshot;
}

function completeGravitySamplerBatch(samplers) {
  // A panel can request a different bounded support stride as the lattice
  // grows. Bundle every complete common-stride trio that this one sampler
  // batch actually produced; an incomplete or mixed-stride trio is absent.
  const strides = new Set();
  for (const key of Object.keys(samplers)) {
    const match = /^(?:latency|kretschmann|gravity)@(\d+)$/.exec(key);
    if (match) strides.add(Number(match[1]));
  }
  const samples = [];
  for (const stride of strides) {
    const latency = samplers[`latency@${stride}`];
    const kretschmann = samplers[`kretschmann@${stride}`];
    const gravity = samplers[`gravity@${stride}`];
    if (!latency || !kretschmann || !gravity) continue;
    samples.push({ stride, latency, kretschmann, gravity });
  }
  const gravityMetricAgg = samplers['gravityMetricAgg@0'];
  return samples.length && gravityMetricAgg ? { samples, gravityMetricAgg } : null;
}

function captureGravityVisual(vol, tick) {
  const sampleTick = Number.isSafeInteger(tick) && tick >= 0 ? tick : null;
  if (sampleTick === null || !vol || !self.FTD_FLUX_PUBLICATION?.copySlabsWithMaxRho) return null;

  // getFluxVolume is a heap view. Capture its bounded plane copies before
  // another Embind call can reuse or grow that storage.  The result is owned
  // storage; only a later complete same-turn sampler batch may commit it.
  let visual;
  try {
    const mid = N >> 1;
    visual = self.FTD_FLUX_PUBLICATION.copySlabsWithMaxRho(vol, N,
      [{ axis: 0, index: mid }, { axis: 1, index: mid }, { axis: 2, index: mid }]);
  } catch { return null; }
  if (!visual || visual.slabs.length !== 3) return null;
  return { N, sampleTick, maxRho: visual.maxRho, slabs: visual.slabs };
}

function finishGravityObservation(visual, batch, dataVersion, frameTransfer) {
  if (!visual || !batch) return null;
  const { N: visualN, sampleTick, maxRho, slabs } = visual;
  const { samples, gravityMetricAgg } = batch;
  const metadata = {
    sampleTick, source: 'wasm-worker', sourceEpoch: activeConfigurationToken,
    configurationToken: activeConfigurationToken, loadGeneration: renderBridgeGeneration,
    dataVersion, latticeSize: visualN, representation: 'reference-flux-magnitude',
  };
  for (const slab of slabs) {
    slab.maxRho = maxRho;
    slab.metadata = metadata;
  }
  for (const slab of slabs) frameTransfer.push(slab.data.buffer);
  return {
    N: visualN,
    sampleTick,
    source: 'wasm-worker',
    sourceEpoch: activeConfigurationToken,
    configurationToken: activeConfigurationToken,
    loadGeneration: renderBridgeGeneration,
    dataVersion,
    maxRho,
    slabs,
    samples,
    gravityMetricAgg,
    engineToggles: gravityObservationToggles(),
  };
}

function captureGravityObservation(vol, batch, dataVersion, frameTransfer, tick) {
  return finishGravityObservation(captureGravityVisual(vol, tick), batch, dataVersion, frameTransfer);
}

function completeGravitySamplerWantSignature(wants) {
  if (!wants || typeof wants.has !== 'function') return '';
  const strides = [];
  for (const key of wants.keys()) {
    const match = /^(?:latency|kretschmann|gravity)@(\d+)$/.exec(key);
    if (!match) continue;
    const stride = match[1];
    if (wants.has(`latency@${stride}`)
        && wants.has(`kretschmann@${stride}`)
        && wants.has(`gravity@${stride}`)) strides.push(Number(stride));
  }
  return strides.sort((a, b) => a - b).join(',');
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
  createBoundedReductionCadence,
  visitScheduledSamplers,
} = self.FTD_SAMPLER_CADENCE;
const gravitySamplerCadence = createBoundedSamplerCadence(GRAVITY_SAMPLER_INTERVAL_MS);
// A visible Gravity panel is a coherent per-state observation, unlike the
// bounded Time instrument.  Successes commit once per exact state identity;
// this separate cadence only prevents unavailable batches from retrying on
// every following worker publication.
const gravityObservationRetryCadence = createBoundedReductionCadence();
// Full Lagrangian extraction is O(L^3). Successful observations follow every
// distinct completed state; this cadence only bounds retries after an ABI/error
// outcome so an unavailable getter cannot tax every worker publication.
const lagrangianCadence = createBoundedReductionCadence();

// Knot telemetry/event payloads are WASM heap VIEWS (zero-copy). They are
// invalidated by the next WASM call, so copy every typed array out before the
// payload crosses the postMessage boundary back to the main thread.
const WORKER_COMMAND_ALLOWLIST = new Set([
  'tickScale0', 'setToggle', 'setSorIterations', 'setLinkEnergyObservation', 'setDt', 'setOmega0',
  'setLangevinTemp', 'setLangevinGamma', 'setFluxBoundary', 'setFluxPeriodicAxis',
  'injectParticle', 'injectFlux', 'injectFluxBulk', 'injectWavepacket', 'injectWaveVel',
  'createEntangledPair', 'clearField', 'seedRandomFlux',
  // Flux-cell mechanisms (engine/include/ftd/flux_cell.h, 2026-09-02).
  'setFluxCellRegion', 'clearFluxCellRegion', 'setFluxPump', 'clearFluxPump',
  'setFluxCellPort', 'clearFluxCellPort',
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
    // Append-only flux-cell ledger (2026-09-02).
    'cellSiteCount', 'cellUE', 'cellUB', 'cellUJ', 'cellHWave', 'cellPLeak', 'cellSNet',
    'cellPumpWork', 'cellPumpTicksApplied', 'cellPumpTicksTotal', 'cellPortOpen',
    'cellPortWorkOut', 'cellPortPoyntingOut',
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

// The compact view executes the identical C++ reduction as getLagrangian(),
// but avoids an Embind object and seventeen property writes per worker sample.
// Read every scalar before a later WASM call can invalidate the heap view.
function readLagrangian() {
  if (typeof mod.getLagrangianView !== 'function') return mod.getLagrangian(bridge);
  const values = mod.getLagrangianView(bridge);
  if (!values || values.length < 17) return null;
  return {
    fieldKinetic: values[0], fieldGradient: values[1], bornInfeld: values[2],
    coupling: values[3], velocity: values[4], gauss: values[5],
    dissipation: values[6], total: values[7], hamiltonian: values[8],
    totalAction: values[9], gaussViolation: values[10], maxGaussError: values[11],
    totalFluxMag: values[12], totalWaveEnergy: values[13], manifested: values[14],
    locked: values[15], cellVolume: values[16],
  };
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
  if (method === 'injectFluxBulk') {
    try {
      const buf = args[0];
      const a = buf instanceof Float64Array ? buf : new Float64Array(buf);
      for (let i = 0; i + 5 < a.length; i += 6) {
        mod.injectFlux(bridge, a[i] | 0, a[i + 1] | 0, a[i + 2] | 0, a[i + 3], a[i + 4], a[i + 5]);
      }
      return { ok: true };
    } catch (e) {
      const error = 'injectFluxBulk failed: ' + String(e && e.message || e);
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

function readFluxPeriodicAxis() {
  if (!mod || !bridge || typeof mod.getFluxPeriodicAxis !== 'function') return null;
  try {
    const axis = Number(mod.getFluxPeriodicAxis(bridge));
    return Number.isInteger(axis) ? axis : null;
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
let wantAudit = false;
let wantLag = false;
let wantGravity = false;
let wantProperTime = false;
let gravityMetricAggVersion = null;
let gravityObservationRevision = 0;
let lastGravityObservationIdentity = null;
let gravityObservationSupportSignature = '';
let telemetryTimingEnabled = false;
const telemetryTiming = {
  lagrangianAttempts: 0, lagrangianCompleted: 0, lagrangianLastMs: null,
  lagrangianTotalMs: 0, lagrangianMaxMs: 0,
  tickCompleted: 0, tickLastMs: null, tickTotalMs: 0, tickMaxMs: 0,
};

function resetTelemetryTiming() {
  telemetryTiming.lagrangianAttempts = 0; telemetryTiming.lagrangianCompleted = 0;
  telemetryTiming.lagrangianLastMs = null; telemetryTiming.lagrangianTotalMs = 0;
  telemetryTiming.lagrangianMaxMs = 0;
  telemetryTiming.tickCompleted = 0; telemetryTiming.tickLastMs = null;
  telemetryTiming.tickTotalMs = 0; telemetryTiming.tickMaxMs = 0;
}

function telemetryTimingSnapshot() {
  return {
    lagrangian: {
      attempts: telemetryTiming.lagrangianAttempts,
      completed: telemetryTiming.lagrangianCompleted,
      lastMs: telemetryTiming.lagrangianLastMs,
      averageMs: telemetryTiming.lagrangianCompleted
        ? telemetryTiming.lagrangianTotalMs / telemetryTiming.lagrangianCompleted : null,
      maxMs: telemetryTiming.lagrangianMaxMs,
    },
    tick: {
      completed: telemetryTiming.tickCompleted,
      lastMs: telemetryTiming.tickLastMs,
      averageMs: telemetryTiming.tickCompleted
        ? telemetryTiming.tickTotalMs / telemetryTiming.tickCompleted : null,
      maxMs: telemetryTiming.tickMaxMs,
    },
  };
}

// Energy-audit cadence cache — see postFrame(). getEnergyAudit is a full O(N^3)
// pass, so large lattices sample it less often. The cached audit is published
// with its original sample tick/version between samples; it must never be
// relabelled as a current diagnostic observation. Reset on every rebuild so a
// new scenario/lattice never reuses a stale-N audit.
let lastAudit = null;
let auditFrameCounter = 0;
let lastLagrangian = null;
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

function buildBridge(n, scen, configurationToken = 0, seedOverrides = null) {
  let candidate = null;
  let seedDescription = null;
  try {
    candidate = new mod.RenderBridge(n | 0);
    for (const k in toggles) mod.setToggle(candidate, k, toggles[k]);
    if (seedOverrides !== null) {
      if (!mod.setupScenarioSeed) throw new Error('This worker artifact does not support editable scenario seeds.');
      seedDescription = JSON.parse(mod.setupScenarioSeed(candidate, scen, seedOverrides));
      if (seedDescription.error) throw new Error(seedDescription.error);
    } else if (mod.setupScenario(candidate, scen) === false) {
      throw new Error('Unknown or unhandled scenario: ' + scen);
    }
  } catch (error) {
    candidate?.delete();
    self.postMessage({type: 'error', where: 'setupScenario',
      msg: String(error?.message || error), configurationToken});
    return;
  }
  activeConfigurationToken = Number(configurationToken) || 0;
  tickAcc = 0;
  lastInspect = null;
  lastForceAt = null;
  gravitySamplerCadence.reset();
  gravityObservationRetryCadence.reset();
  gravityObservationRevision = 0;
  lastGravityObservationIdentity = null;
  gravityObservationSupportSignature = '';
  gravityMetricAggVersion = null;
  if (bridge) { try { bridge.delete(); } catch (e) { /* ignore */ } bridge = null; }
  N = n | 0;
  bridge = candidate;
  renderBridgeGeneration++;
  const setupOk = true, setupError = null;
  enforceToggleInvariants();
  engineTogglesDirty = true;   // the C++ body just replaced the whole profile
  // New state belongs to no prior reduction.  The first demanded frame below
  // samples immediately; hidden panels stay reduction-free.
  lastAudit = null; auditFrameCounter = 0;
  lastLagrangian = null; lagrangianCadence.reset();
  lastAuditMeta = null;
  lastLagrangianMeta = null;
  resetTelemetryTiming();
  scenarioId = scen;
  // O(N^3), so capture once per newly built scenario and thereafter only on
  // an explicit `captureDigest` request. Never put digest work in the 60 Hz
  // loop. The initial value lets the proxy expose a truthful cached getter as
  // soon as its first frame arrives.
  lastDynamicalStateDigest = captureDynamicalStateDigest();
  publishDynamicalStateDigest = true;
  // Flux-volume cache pointer is stable for a fixed N; publish the heap + offset.
  const initialTick = typeof bridge.currentTick === 'function' ? bridge.currentTick() : null;
  const vol = mod.getFluxVolume(bridge);
  if (!ctrlSab) { ctrlSab = new SharedArrayBuffer(CTRL.LEN * 4); ctrl = new Int32Array(ctrlSab); }
  Atomics.store(ctrl, CTRL.N, N);
  Atomics.store(ctrl, CTRL.RUNNING, 0);
  lastFluxHeap = vol.buffer;
  lastFluxPtr = vol.byteOffset;
  lastFluxLen = vol.length;
  const doubled = publishFlux(vol, initialTick);
  self.postMessage({
    type: 'ready', N, ctrl: ctrlSab, heap: vol.buffer, fluxPtr: vol.byteOffset, fluxLen: vol.length,
    setupOk, setupError, artifactIdentity, configurationToken, seedDescription,
    workerRuntimeId, moduleInitCount, renderBridgeGeneration,
    constants: (() => { try { const c = mod.getConstants(); const o = {}; for (const k in c) if (typeof c[k] !== 'object') o[k] = c[k]; return o; } catch (e) { return null; } })(),
    ...(doubled ? { fluxSab: fluxPubSab, doubleBuffered: true,
                   fluxProtocol: self.FTD_FLUX_PUBLICATION.PROTOCOL } : {}),
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
  const frameTransfer = [];  // freshly copied sampler buffers, moved (not cloned) with the frame
  if (!bridge || backgroundSuspended) return;
  // Capture the engine clock before taking the synchronous flux view. That
  // tick is the publication's identity; never substitute the later CTRL tick.
  const rawTick = bridge.currentTick ? bridge.currentTick() : null;
  const tick = Number.isSafeInteger(rawTick) && rawTick >= 0 ? rawTick : null;
  const vol = mod.getFluxVolume(bridge);            // refresh the flux cache in the shared heap
  const doubled = publishFlux(vol, tick);
  if (!doubled && vol && (vol.buffer !== lastFluxHeap || vol.byteOffset !== lastFluxPtr || vol.length !== lastFluxLen)) {
    lastFluxHeap = vol.buffer;
    lastFluxPtr = vol.byteOffset;
    lastFluxLen = vol.length;
    self.postMessage({
      type: 'fluxRebind', heap: vol.buffer, fluxPtr: vol.byteOffset, fluxLen: vol.length,
      configurationToken: activeConfigurationToken,
    });
  }
  // A tick can remain unchanged across a user mutation (toggle, injection,
  // reset command).  Track that transition separately so a visible Gravity
  // panel receives one replacement observation for the mutated same-tick
  // state, while passive paused readbacks remain zero-work.
  if (fieldChanged) gravityObservationRevision++;
  const gravitySupportSignature = wantGravity
    ? completeGravitySamplerWantSignature(wantedSamplers) : '';
  if (gravitySupportSignature !== gravityObservationSupportSignature) {
    // The complete support set is part of the atomic observation contract.
    // Adding a stride while paused must create a replacement bundle, rather
    // than leave the proxy with a same-tick subset from an earlier demand.
    gravityObservationSupportSignature = gravitySupportSignature;
    lastGravityObservationIdentity = null;
    gravityObservationRetryCadence.reset();
  }
  const gravityIdentity = tick === null ? null
    : `${activeConfigurationToken}:${gravityObservationRevision}:${tick}:${gravitySupportSignature}`;
  const gravityWantsCompleteBatch = wantGravity && gravitySupportSignature !== '';
  const gravityNeedsObservation = gravityWantsCompleteBatch
    && gravityIdentity !== lastGravityObservationIdentity;
  const gravityObservationDue = gravityNeedsObservation
    && gravityObservationRetryCadence.shouldRun(true, false, performance.now(), gravityObservationRevision);
  // `vol` is a live WASM heap view.  Make owned slices now, before any sampler
  // Embind call can reuse or grow the heap; a later complete sampler batch is
  // the only condition that commits this candidate to the proxy.
  const gravityVisualCandidate = gravityObservationDue
    ? captureGravityVisual(vol, tick) : null;
  let diag = null, parts = null, audit = null, lag = null;
  let diagMeta = null;
  try {
    diag = mod.getDiagnostics(bridge);
    // The engine already updates its rest-offset-free EnergyLedger at the end
    // of every tick. Reuse that cached O(1) observation for the always-visible
    // status readout and core energy chart instead of making either surface
    // depend on the much heavier full EnergyAudit reduction.
    const ledger = typeof mod.getEnergyLedger === 'function'
      ? mod.getEnergyLedger(bridge) : null;
    if (diag && Number.isFinite(ledger?.ECurr)) {
      diag.vacuumBaselineEnergy = diag.totalEnergy;
      diag.dynamicEnergy = ledger.ECurr;
      diag.totalEnergy = ledger.ECurr;
      diag.energySampleSource = 'per-tick-ledger';
    }
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
  // The Lagrangian is a full all-site reduction.  Preserve its own sample
  // identity between reductions, just as audit does: a visible panel gets an
  // immediate sample and later receives exactly one fresh observation for each
  // distinct completed simulation state. Never retimestamp a retained sample.
  const lagStartedAt = performance.now();
  const priorLagTick = lastLagrangianMeta?.sampleTick ?? lastLagrangianMeta?.tick ?? null;
  const retryAfterFailure = lastLagrangianMeta != null
    && lastLagrangianMeta.status !== 'available';
  const lagDue = wantLag && (retryAfterFailure
    ? lagrangianCadence.shouldRun(true, false, lagStartedAt, tick)
    : priorLagTick !== tick);
  if (lagDue) {
    if (telemetryTimingEnabled) telemetryTiming.lagrangianAttempts++;
    try {
      const sampledLag = readLagrangian();
      lastLagrangian = sampledLag || null;
      lastLagrangianMeta = telemetryGroupMeta({
        stateVersion: ++lagrangianStateVersion, tick,
        stale: !lastLagrangian,
        status: lastLagrangian ? 'available' : 'unavailable',
      });
    } catch (e) {
      lastLagrangian = null;
      lastLagrangianMeta = telemetryGroupMeta({
        stateVersion: ++lagrangianStateVersion, tick, stale: true, status: 'error',
      });
    } finally {
      // Only failed/unavailable attempts use cooldown. Successful reads above
      // are intentionally sampled once for every completed worker state.
      const lagFinishedAt = performance.now();
      const lagElapsedMs = Math.max(0, lagFinishedAt - lagStartedAt);
      if (lastLagrangianMeta?.status === 'available') lagrangianCadence.reset();
      else lagrangianCadence.complete(lagFinishedAt, tick, lagElapsedMs, { retry: true });
      if (telemetryTimingEnabled) {
        telemetryTiming.lagrangianCompleted++;
        telemetryTiming.lagrangianLastMs = lagElapsedMs;
        telemetryTiming.lagrangianTotalMs += lagElapsedMs;
        telemetryTiming.lagrangianMaxMs = Math.max(telemetryTiming.lagrangianMaxMs, lagElapsedMs);
        lastLagrangianMeta.reductionMs = lagElapsedMs;
      }
    }
  } else if (!wantLag && (!lastLagrangianMeta || lastLagrangianMeta.status !== 'inactive'
      || lastLagrangianMeta.sourceEpoch !== activeConfigurationToken)) {
    // Inactivity is an explicit observation boundary, including before the
    // first sample and after configuration replacement. Repeated inactive
    // frames retain this identity instead of fabricating new observations.
    lastLagrangianMeta = telemetryGroupMeta({
      stateVersion: ++lagrangianStateVersion, tick, stale: true, status: 'inactive',
    });
    lagrangianCadence.reset();
  } else if (!wantLag) {
    lagrangianCadence.reset();
  }
  lag = wantLag ? lastLagrangian : null;

  // Audit-derived decomposition is valid only when both observations describe
  // this exact tick. Reused/staggered audit samples remain separate telemetry
  // and must not be promoted to the diagnostics packet's newer provenance.
  // dynamicEnergy itself remains sourced from the engine's per-tick ledger.
  if (diag && auditSampledThisFrame && audit
      && lastAuditMeta?.status === 'available' && lastAuditMeta.stale !== true
      && lastAuditMeta.sampleTick === tick
      && Number.isFinite(audit.dynamicEnergy)) {
    if (!Object.hasOwn(diag, 'vacuumBaselineEnergy')) {
      diag.vacuumBaselineEnergy = diag.totalEnergy;
    }
    diag.accountedEnergy = audit.totalEnergy;
    diag.restEnergy = audit.particleRestEnergy;
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
  // cadence. A visible Gravity panel forces its coherent L/K/F/aggregate
  // batch once per new state; Time-only proper-time collection remains
  // independently bounded by the shared cadence helper.
  const samplers = {};
  let gravityMetricAggSampled = false;
  if (wantedSamplers.size > 0 || wantGravity || wantProperTime) {
    visitScheduledSamplers(wantedSamplers, {
      wantGravity,
      wantProperTime,
      cadence: gravitySamplerCadence,
      nowMs: performance.now(),
      forceGravityBatch: gravityObservationDue || (!wantGravity && forceGravitySamplerBatch),
      gravityPerState: wantGravity,
      forceProperTimeBatch: !gravityObservationDue && forceGravitySamplerBatch,
      allowUndemandedBoundedInstrument,
    }, (key, { kind, stride }) => {
      const spec = SAMPLER_METHODS[kind];
      if (!spec) return;
      const [method, type] = spec;
      if (typeof mod[method] !== 'function') return;
      try {
        if (type === 'links') {
          const raw = mod[method](bridge);
          if (!raw) return;
          // links / residual are views into observer memory on the WASM heap — copy before posting.
          samplers[key] = {
            status: String(raw.status), reason: String(raw.reason),
            L: raw.L | 0, tick: Number(raw.tick),
            links: new Float32Array(raw.links || 0), residual: new Float32Array(raw.residual || 0),
            invariant: Number(raw.invariant), maxLocalChange: Number(raw.maxLocalChange),
            maxResidual: Number(raw.maxResidual), closure: Number(raw.closure),
            activeExchangeTerms: raw.activeExchangeTerms >>> 0,
          };
          frameTransfer.push(samplers[key].links.buffer, samplers[key].residual.buffer);
          return;
        }
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
          if (Number.isInteger(raw.effectiveStride) && raw.effectiveStride > 0) {
            samplers[key].effectiveStride = raw.effectiveStride;
          }
          if (Number.isInteger(raw.origin) && raw.origin >= 0) samplers[key].origin = raw.origin;
        }
      } catch { /* ignore — method may not be bound in this WASM build */ }
    });
  }

  if (ctrl) {
    // Retain the last valid control tick when this worker cannot establish an
    // exact clock. Writing zero would fabricate a physics observation.
    if (tick !== null) Atomics.store(ctrl, CTRL.TICK, tick);
    Atomics.store(ctrl, CTRL.PCOUNT, parts ? parts.count : 0);
    if (fieldChanged) Atomics.add(ctrl, CTRL.DATA_VERSION, 1);
    Atomics.add(ctrl, CTRL.FRAME, 1);
  }
  const dataVersion = ctrl ? Atomics.load(ctrl, CTRL.DATA_VERSION) : 0;
  if (gravityMetricAggSampled) gravityMetricAggVersion = dataVersion;
  // Never derive a visual tick from a later publication.  The candidate was
  // copied before samplers from this exact heap view and is committed only
  // with a complete scalar batch from this same worker turn.
  let gravityObservation = null;
  const gravityBatch = gravityObservationDue && gravityVisualCandidate && gravityMetricAggSampled
    ? completeGravitySamplerBatch(samplers) : null;
  if (gravityObservationDue) {
    try {
      gravityObservation = finishGravityObservation(
        gravityVisualCandidate, gravityBatch, dataVersion, frameTransfer,
      );
    } catch {
      gravityObservation = null;
    }
    if (gravityObservation) {
      lastGravityObservationIdentity = gravityIdentity;
      gravityObservationRetryCadence.reset();
    } else {
      // Failure is an honest absence, not a retimestamped retained bundle.
      // It may retry only after the bounded cadence observes a later state.
      gravityObservationRetryCadence.complete(
        performance.now(), gravityObservationRevision, 0, { retry: true },
      );
    }
  }
  const engineTogglesMsg = engineTogglesDirty ? readEngineToggles() : null;
  const digestMsg = publishDynamicalStateDigest ? lastDynamicalStateDigest : undefined;
  publishDynamicalStateDigest = false;
  self.postMessage({ type: 'frame', configurationToken: activeConfigurationToken,
                     tick, diag, diagMeta, parts,
                     dataVersion,
                     audit, auditMeta: lastAuditMeta,
                     lag, lagMeta: lastLagrangianMeta,
                     samplers, gravityObservation, knot, knotEvents, knotAgg,
                     inspect: lastInspect, forceAt: lastForceAt,
                     ...(telemetryTimingEnabled ? { telemetryTiming: telemetryTimingSnapshot() } : {}),
                     ...(digestMsg !== undefined ? { dynamicalStateDigest: digestMsg } : {}),
                     ...(engineTogglesMsg ? { engineToggles: engineTogglesMsg } : {}) }, frameTransfer);
}

function loop() {
  timer = 0;
  if (backgroundSuspended) return;
  if (!bridge) { timer = setTimeout(loop, TARGET_DT); return; }
  const t0 = performance.now();
  if (ctrl && Atomics.load(ctrl, CTRL.RUNNING)) {
    const tpfRaw = Atomics.load(ctrl, CTRL.TICKS_PER_FRAME);
    const tpf = tpfRaw > 0 ? tpfRaw / 1000 : 1.0;
    tickAcc += tpf;
    const whole = Math.floor(tickAcc); tickAcc -= whole;
    const maxTicks = N > 96 ? 1 : (N > 48 ? 1 : (N > 32 ? 2 : whole));
    const toRun = Math.min(whole, maxTicks);
    try {
      const tickStartedAt = performance.now();
      for (let i = 0; i < toRun; i++) bridge.tick();
      const tickElapsedMs = Math.max(0, performance.now() - tickStartedAt);
      if (telemetryTimingEnabled && toRun > 0) {
        const perTickMs = tickElapsedMs / toRun;
        telemetryTiming.tickCompleted += toRun;
        telemetryTiming.tickLastMs = perTickMs;
        telemetryTiming.tickTotalMs += tickElapsedMs;
        telemetryTiming.tickMaxMs = Math.max(telemetryTiming.tickMaxMs, perTickMs);
      }
      if (toRun > 0) postFrame(true);
    } catch (e) {
      Atomics.store(ctrl, CTRL.RUNNING, 0);
      self.postMessage({ type: 'error', where: 'runtime',
        configurationToken: activeConfigurationToken, msg: String(e && e.message || e) });
    }
  }
  const elapsed = performance.now() - t0;
  if (ctrl && Atomics.load(ctrl, CTRL.RUNNING)) {
    timer = setTimeout(loop, Math.max(0, TARGET_DT - elapsed));
  }
}

self.onmessage = (e) => {
  const msg = e.data;
  try {
    if (backgroundSuspended && !['setBackgroundSuspended', 'captureDigest', 'dispose', 'acknowledgedControl'].includes(msg.type)) return;
    switch (msg.type) {
      case 'setBackgroundSuspended':
        backgroundSuspended = msg.value === true;
        if (backgroundSuspended) {
          if (timer) { clearTimeout(timer); timer = 0; }
          if (ctrl) Atomics.store(ctrl, CTRL.RUNNING, 0);
        }
        self.postMessage({ type: 'backgroundSuspended', value: backgroundSuspended,
          seq: msg.seq, configurationToken: activeConfigurationToken });
        break;
      case 'create':
        toggles = msg.toggles || {};
        toggleNames = Array.isArray(msg.toggleNames) && msg.toggleNames.length
          ? [...msg.toggleNames]
          : Object.keys(toggles);
        if (typeof msg.pool === 'number' && msg.pool >= 1) poolThreads = msg.pool | 0;
        pendingCreate = msg;
        if (mod) {
          pendingCreate = null;
          buildBridge(msg.N, msg.scenarioId || scenarioId, msg.configurationToken, msg.seedOverrides ?? null);
          if (!timer) loop();
        } else if (!initInFlight) {
          initInFlight = true;
          initModule(() => {
            initInFlight = false;
            const m = pendingCreate;
            pendingCreate = null;
            if (m) buildBridge(m.N, m.scenarioId || scenarioId, m.configurationToken, m.seedOverrides ?? null);
            if (!timer) loop();
          });
        }
        break;
      case 'resize':
        if (mod && Number(msg.configurationToken) >= activeConfigurationToken) {
          buildBridge(msg.N, msg.scenarioId || scenarioId, msg.configurationToken, msg.seedOverrides ?? null);
        }
        break;
      case 'acknowledgedControl': {
        const reply = { type: 'controlComplete', requestId: msg.requestId,
          configurationToken: msg.configurationToken, ok: false, status: 'rejected' };
        let dispatched = false;
        try {
          if (!mod || !bridge || Number(msg.configurationToken) !== activeConfigurationToken || backgroundSuspended)
            throw new Error('Lattice owner is unavailable or superseded');
          const command = msg.command || {};
          if (!['barrier', 'step', 'setToggle'].includes(command.type)) throw new Error('Unsupported lattice control');
          if (command.type === 'step' && (!Number.isSafeInteger(command.count) || command.count < 1 || command.count > 64))
            throw new Error('Invalid lattice step count');
          if (command.type === 'setToggle' && (!toggleNames.includes(command.name) || typeof command.value !== 'boolean'))
            throw new Error('Invalid lattice toggle');
          reply.beforeTick = bridge.currentTick();
          if (command.type === 'step') {
            if (ctrl && Atomics.load(ctrl, CTRL.RUNNING)) throw new Error('Pause before exact stepping');
            for (let i = 0; i < command.count; i++) {
              dispatched = true;
              const result = applyCommand('tickScale0', []);
              if (!result.ok) throw new Error(result.error);
            }
          } else if (command.type === 'setToggle') {
            dispatched = true;
            const result = applyCommand('setToggle', [command.name, command.value]);
            if (!result.ok) throw new Error(result.error);
            enforceToggleInvariants();
            if (!!mod.getToggle(bridge, command.name) !== command.value) throw new Error('Toggle readback did not match the request');
          }
          if (command.type !== 'barrier') {
            lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
            lastLagrangian = null; lastLagrangianMeta = null; lagrangianCadence.reset();
            engineTogglesDirty = true;
          }
          postFrame(true);
          reply.tick = bridge.currentTick(); reply.ok = true; reply.status = 'applied';
        } catch (error) {
          reply.error = String(error.message || error);
          reply.status = dispatched ? 'unknown' : 'rejected';
          if (dispatched && ctrl) Atomics.store(ctrl, CTRL.RUNNING, 0);
        }
        self.postMessage(reply);
        break;
      }
      case 'command': {
        if (!mod || !bridge
            || Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
        lastLagrangian = null; lastLagrangianMeta = null; lagrangianCadence.reset();
        const result = applyCommand(msg.method, msg.args || []);
        if (!result.ok) {
          if (msg.method === 'tickScale0' && ctrl) Atomics.store(ctrl, CTRL.RUNNING, 0);
          self.postMessage({ type: 'error', where: msg.method,
            configurationToken: activeConfigurationToken, msg: result.error });
          break;
        }
        engineTogglesDirty = true;
        postFrame(true);
        break;
      }
      case 'toggleBatch': {
        if (!mod || !bridge
            || Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
        lastLagrangian = null; lastLagrangianMeta = null; lagrangianCadence.reset();
        const errors = [];
        const expectedToggles = new Map();
        for (const entry of (msg.entries || [])) {
          if (!Array.isArray(entry) || typeof entry[0] !== 'string') {
            errors.push('invalid toggle batch entry');
            continue;
          }
          const name = entry[0];
          const expected = !!entry[1];
          const result = applyCommand('setToggle', [name, expected]);
          if (!result?.ok) errors.push(result?.error || `toggle failed: ${name}`);
          expectedToggles.set(name, expected);
        }
        enforceToggleInvariants();
        engineTogglesDirty = true;
        // One forced publication repaints every checkbox from engine truth.
        postFrame(true);
        for (const [name, expected] of expectedToggles) {
          if (!(name in engineToggles) || engineToggles[name] !== expected) {
            errors.push(`toggle readback mismatch: ${name} expected ${expected}`);
          }
        }
        if (errors.length) {
          self.postMessage({
            type: 'error',
            where: 'toggleBatch',
            msg: errors.slice(0, 8).join('; '),
            configurationToken: activeConfigurationToken,
          });
        }
        break;
      }
      case 'batchCommand': {
        if (!mod || !bridge
            || Number(msg.configurationToken) !== activeConfigurationToken) break;
        lastAudit = null; lastAuditMeta = null; auditFrameCounter = 0;
        lastLagrangian = null; lastLagrangianMeta = null; lagrangianCadence.reset();
        const errors = [];
        const expectedToggles = new Map();
        let expectedFluxBoundaryMode = null;
        let expectedFluxPeriodicAxis = null;
        for (const { method, args = [] } of (msg.commands || [])) {
          const result = applyCommand(method, args);
          if (!result?.ok) errors.push(result?.error || `command failed: ${method}`);
          if (method === 'setToggle' && typeof args[0] === 'string') {
            expectedToggles.set(args[0], !!args[1]);
          } else if (method === 'setFluxBoundary' && Number.isInteger(Number(args[0]))) {
            expectedFluxBoundaryMode = Number(args[0]);
          } else if (method === 'setFluxPeriodicAxis' && Number.isInteger(Number(args[0]))) {
            expectedFluxPeriodicAxis = Number(args[0]);
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
        const fluxPeriodicAxis = readFluxPeriodicAxis();
        if (expectedFluxPeriodicAxis !== null && fluxPeriodicAxis !== expectedFluxPeriodicAxis) {
          errors.push(`periodic axis readback mismatch: expected ${expectedFluxPeriodicAxis}, got ${fluxPeriodicAxis}`);
        }
        self.postMessage({
          type: 'configurationApplied',
          configurationToken: msg.configurationToken,
          ok: errors.length === 0,
          errors,
          engineToggles: { ...engineToggles },
          fluxBoundaryMode,
          fluxPeriodicAxis,
        });
        break;
      }
      case 'inspectVoxel': {
        if (msg.configurationToken !== activeConfigurationToken
            || !Number.isSafeInteger(msg.requestId) || msg.requestId <= 0
            || ![msg.x, msg.y, msg.z].every(v => Number.isSafeInteger(v) && v >= 0 && v < N)) break;
        lastInspect = null;
        let sampleTick = null;
        let dataVersion = null;
        if (mod && bridge && typeof mod.inspectVoxel === 'function') {
          try {
            lastInspect = { x: msg.x, y: msg.y, z: msg.z, voxel: mod.inspectVoxel(bridge, msg.x, msg.y, msg.z) };
            // Inspection and the true core clock read are synchronous in this
            // worker turn. A cached diagnostic/frame clock is not provenance.
            const tick = typeof bridge.currentTick === 'function' ? bridge.currentTick() : null;
            sampleTick = Number.isSafeInteger(tick) && tick >= 0 ? tick : null;
            const version = ctrl ? Atomics.load(ctrl, CTRL.DATA_VERSION) : null;
            dataVersion = Number.isSafeInteger(version) && version >= 0 ? version : null;
          } catch { lastInspect = null; }
        }
        self.postMessage({
          type: 'inspectResult', inspect: lastInspect,
          requestId: msg.requestId, x: msg.x, y: msg.y, z: msg.z,
          sampleTick, dataVersion,
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
      case 'setTelemetryTiming': {
        if (Number(msg.configurationToken) !== activeConfigurationToken) break;
        telemetryTimingEnabled = msg.enabled === true;
        resetTelemetryTiming();
        break;
      }
      case 'setTelemetryMask': {
        const nextWantAudit = msg.wantAudit === true;
        const auditChanged = nextWantAudit !== wantAudit;
        const nextWantLag = msg.wantLag === true;
        const lagChanged = nextWantLag !== wantLag;
        const nextWantGravity = msg.wantGravity === true;
        const gravityBecameWanted = nextWantGravity && !wantGravity;
        const gravityChanged = nextWantGravity !== wantGravity;
        const nextWantProperTime = msg.wantProperTime === true;
        const properTimeBecameWanted = nextWantProperTime && !wantProperTime;
        wantAudit = nextWantAudit;
        wantLag = nextWantLag;
        wantGravity = nextWantGravity;
        wantProperTime = nextWantProperTime;
        if (auditChanged) {
          // Never reuse an observation across an inactive boundary. The next
          // demanded postFrame samples current state; the inactive path emits
          // an explicit null/status boundary.
          lastAudit = null;
          auditFrameCounter = 0;
        }
        if (lagChanged) {
          // An inactive boundary cannot retain a prior visible observation.
          // Reopening samples current state immediately through the reset
          // wall-time cadence.
          lastLagrangian = null;
          lagrangianCadence.reset();
        }
        if (gravityChanged) {
          // A hidden panel has no retained observation contract. Reopening it
          // samples the current state once, even when the ordinal tick paused.
          lastGravityObservationIdentity = null;
          gravityObservationSupportSignature = '';
          gravityObservationRetryCadence.reset();
        }
        // A paused Lagrangian-only transition must publish even when audit is
        // already demanded. This is a readback, not a physics-data advance.
        let publishPausedMaskChange = auditChanged || lagChanged;
        if (gravityBecameWanted || properTimeBecameWanted) {
          gravitySamplerCadence.reset();
          const dataVersion = ctrl ? Atomics.load(ctrl, CTRL.DATA_VERSION) : 0;
          // Time may be opened while playback is paused and owns no direct
          // aggregate want. Populate once immediately unless the just-added
          // Gravity batch already supplied this exact data generation.
          publishPausedMaskChange ||= gravityMetricAggVersion !== dataVersion;
        }
        if (publishPausedMaskChange && bridge && ctrl
            && !Atomics.load(ctrl, CTRL.RUNNING)) {
          postFrame(false, gravityBecameWanted || properTimeBecameWanted);
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
        if (msg.value && !timer) loop();
        else if (!msg.value && timer) { clearTimeout(timer); timer = 0; }
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
