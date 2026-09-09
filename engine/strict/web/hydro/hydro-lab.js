// Opt-in local laboratory for phi-hydro-staged-candidate-1 (Task 13). All evolution
// occurs in the compiled worker; this file only converts the FTDHY01 preparation
// transport into the WASM's "ftd-hydro-checkpoint-1" JSON checkpoint schema (the hydro
// bindings never accept the native binary transport directly -- see
// engine/strict/hydro/wasm_bindings_hydro.cpp and task-10-report.md), drives
// advance/observe, and renders the two field canvases and the projected-mode chart.
const byId = (id) => document.getElementById(id);
const elements = Object.fromEntries([
    'preparation', 'size', 'reset', 'step', 'run', 'status',
    'stage', 'gammaFit', 'nuFit', 'probedName', 'probedValue',
    'nuT2', 'nuE', 'csq', 'gconst', 'mass', 'momentum',
    'density', 'momentum-canvas', 'chart', 'provenance',
].map((id) => [id, byId(id)]));
const densityCanvas = elements.density;
const momentumCanvas = elements['momentum-canvas'];

let worker = null, ownerId = null, generation = '0', requestSequence = 0, epoch = 0;
let running = false, busy = false, lastPublish = 0;
let manifest = null, sidecar = null, stageIndex = 0, amplitudeHistory = [];
let lastBlocks = null, lastSide = 0, lastZMid = 0;
const pending = new Map();

// ---- FTDHY01 -> "ftd-hydro-checkpoint-1" conversion --------------------------------
// scripts/phi_v2_lattice/hydro/codec.py's encode()/checkpoint() both serialize the same
// eight arrays via numpy .tobytes(order="C") in the same order; every element is one
// byte (int8 or bool) either way, so the raw byte ranges after the FTDHY01 header are
// byte-identical to what codec.checkpoint()'s base64 fields would hold for the same
// state. No dtype reinterpretation is needed here -- only slicing and base64 encoding.
const HEADER_SIZE = 8 + 4 + 8 + 32 + 32 + 32; // magic + L(u32) + microtick(u64) + 3x32-byte hashes
const MAGIC = [0x46, 0x54, 0x44, 0x48, 0x59, 0x30, 0x31, 0x00]; // "FTDHY01\0"
const ARRAY_ELEMENTS_PER_SITE = [
    ['s', 1], ['bank', 192], ['sc', 6], ['fcc', 12],
    ['admitted_sc', 3], ['admitted_fcc', 6], ['gate_sc', 3], ['gate_fcc', 6],
];

function base64FromBytes(bytes) {
    let binary = '';
    const chunk = 0x8000;
    for (let i = 0; i < bytes.length; i += chunk) binary += String.fromCharCode(...bytes.subarray(i, i + chunk));
    return btoa(binary);
}

function ftdhy01ToCheckpointJson(buffer, manifestForHashes) {
    const bytes = new Uint8Array(buffer);
    for (let i = 0; i < MAGIC.length; i++) {
        if (bytes[i] !== MAGIC[i]) throw new Error('preparation is not a valid FTDHY01 transport');
    }
    const view = new DataView(buffer);
    const L = view.getUint32(8, true);
    const microtick = view.getBigUint64(12, true);
    const n = L ** 3;
    let offset = HEADER_SIZE;
    const arrays = {};
    for (const [name, perSite] of ARRAY_ELEMENTS_PER_SITE) {
        const size = perSite * n;
        arrays[name] = base64FromBytes(bytes.subarray(offset, offset + size));
        offset += size;
    }
    if (offset !== bytes.length) throw new Error('preparation transport length does not match its declared L');
    return JSON.stringify({
        schema: 'ftd-hydro-checkpoint-1', law: manifestForHashes.law_id, table: manifestForHashes.table_hash,
        encoding: manifestForHashes.encoding_hash, boundary: 'periodic', L, microtick: Number(microtick), arrays,
    });
}

async function sha256Hex(buffer) {
    const digest = await crypto.subtle.digest('SHA-256', buffer);
    return [...new Uint8Array(digest)].map((b) => b.toString(16).padStart(2, '0')).join('');
}

// ---- worker transport (mirrors ../laboratory.js's request/terminate pattern) -------
function request(op, fields = {}) {
    const requestId = String(++requestSequence);
    const message = { requestId, op, ...fields };
    if (op !== 'init') Object.assign(message, { ownerId, generation });
    return new Promise((resolve, reject) => {
        const nextGeneration = op === 'init' ? '0'
            : String(BigInt(generation) + ((op === 'restore' || (op === 'advance' && BigInt(fields.microticks) > 0n)) ? 1n : 0n));
        pending.set(requestId, { resolve, reject, expectedGeneration: nextGeneration });
        worker.postMessage(message);
    });
}
function terminate() {
    if (worker) worker.terminate();
    worker = null;
    for (const item of pending.values()) item.reject(new Error('Owner replaced'));
    pending.clear();
}

// ---- rendering ----------------------------------------------------------------------
function sizePixels(canvas) {
    const pixels = Math.max(200, Math.floor(canvas.clientWidth * devicePixelRatio));
    if (canvas.width !== pixels || canvas.height !== pixels) { canvas.width = pixels; canvas.height = pixels; }
    return pixels;
}

function paintDensity(canvas, blocks, side, zMid) {
    if (!blocks) return;
    const pixels = sizePixels(canvas);
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#0c131b'; ctx.fillRect(0, 0, pixels, pixels);
    const cell = pixels / side;
    const values = [];
    for (let bx = 0; bx < side; bx++) for (let by = 0; by < side; by++) {
        const block = blocks[(bx * side + by) * side + zMid];
        values.push(Number(block.field_tokens[0]) + Number(block.field_tokens[1]));
    }
    const max = Math.max(1, ...values);
    for (let bx = 0; bx < side; bx++) for (let by = 0; by < side; by++) {
        const value = values[bx * side + by];
        const bright = 13 + 62 * Math.sqrt(value / max);
        ctx.fillStyle = `hsl(164 50% ${bright}%)`;
        ctx.fillRect(bx * cell + 1, by * cell + 1, Math.max(1, cell - 2), Math.max(1, cell - 2));
    }
}

function paintMomentum(canvas, blocks, side, zMid) {
    if (!blocks) return;
    const pixels = sizePixels(canvas);
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#0c131b'; ctx.fillRect(0, 0, pixels, pixels);
    const cell = pixels / side;
    const vectors = [];
    let maxMag = 1e-9;
    for (let bx = 0; bx < side; bx++) for (let by = 0; by < side; by++) {
        const block = blocks[(bx * side + by) * side + zMid];
        const px = Number(block.momentum[0][0]) + Number(block.momentum[1][0]);
        const py = Number(block.momentum[0][1]) + Number(block.momentum[1][1]);
        vectors.push([px, py]);
        maxMag = Math.max(maxMag, Math.hypot(px, py));
    }
    ctx.strokeStyle = '#e1c887'; ctx.fillStyle = '#e1c887'; ctx.lineWidth = Math.max(1, cell * 0.06);
    for (let bx = 0; bx < side; bx++) for (let by = 0; by < side; by++) {
        const [px, py] = vectors[bx * side + by];
        const cx = bx * cell + cell / 2, cy = by * cell + cell / 2;
        const scale = (cell * 0.42) / maxMag;
        const ex = cx + px * scale, ey = cy + py * scale;
        ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(ex, ey); ctx.stroke();
        const angle = Math.atan2(ey - cy, ex - cx);
        const head = Math.max(2, cell * 0.12);
        ctx.beginPath();
        ctx.moveTo(ex, ey);
        ctx.lineTo(ex - head * Math.cos(angle - Math.PI / 6), ey - head * Math.sin(angle - Math.PI / 6));
        ctx.lineTo(ex - head * Math.cos(angle + Math.PI / 6), ey - head * Math.sin(angle + Math.PI / 6));
        ctx.closePath(); ctx.fill();
    }
}

function paintChart(canvas, measured, predicted) {
    const pixels = sizePixels(canvas);
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#0c131b'; ctx.fillRect(0, 0, pixels, pixels);
    const margin = pixels * 0.08;
    const xmax = Math.max(1, measured.length - 1, predicted.length - 1);
    const allValues = [...measured, ...predicted].filter(Number.isFinite);
    const ymax = Math.max(1e-12, ...allValues, 0);
    const xScale = (pixels - 2 * margin) / xmax;
    const yScale = (pixels - 2 * margin) / (ymax || 1);
    const toX = (i) => margin + i * xScale;
    const toY = (v) => pixels - margin - v * yScale;
    ctx.strokeStyle = '#385163'; ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(margin, margin); ctx.lineTo(margin, pixels - margin); ctx.lineTo(pixels - margin, pixels - margin); ctx.stroke();
    ctx.strokeStyle = '#78dac1'; ctx.lineWidth = Math.max(1, pixels * 0.004);
    ctx.beginPath();
    predicted.forEach((v, i) => { const x = toX(i), y = toY(v); if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y); });
    ctx.stroke();
    ctx.fillStyle = '#e1c887';
    const radius = Math.max(1.5, pixels * 0.006);
    measured.forEach((v, i) => { const x = toX(i), y = toY(v); ctx.beginPath(); ctx.arc(x, y, radius, 0, Math.PI * 2); ctx.fill(); });
}

function repaint() { paintDensity(densityCanvas, lastBlocks, lastSide, lastZMid); paintMomentum(momentumCanvas, lastBlocks, lastSide, lastZMid); paintChart(elements.chart, amplitudeHistory, sidecar ? sidecar.prediction.amplitude : []); }

// ---- decay-rate fit -------------------------------------------------------------------
function fitDecayRate(history) {
    const windowed = history.slice(-32);
    if (windowed.length < 2) return null;
    const startIndex = history.length - windowed.length;
    const xs = windowed.map((_, i) => startIndex + i);
    const ys = windowed.map((v) => Math.log(Math.max(v, 1e-300)));
    const n = xs.length;
    const xbar = xs.reduce((a, b) => a + b, 0) / n;
    const ybar = ys.reduce((a, b) => a + b, 0) / n;
    let num = 0, den = 0;
    for (let i = 0; i < n; i++) { num += (xs[i] - xbar) * (ys[i] - ybar); den += (xs[i] - xbar) ** 2; }
    if (den === 0) return null;
    return -(num / den);
}

function projectedAmplitude(momentsPayload, polarizationVector) {
    const [px, py, pz] = momentsPayload.moments.slice(1);
    const [tx, ty, tz] = polarizationVector;
    const re = tx * px[0] + ty * py[0] + tz * pz[0];
    const im = tx * px[1] + ty * py[1] + tz * pz[1];
    return Math.hypot(re, im);
}

function aggregateTotals(blocks) {
    let mass = 0n, px = 0n, py = 0n, pz = 0n;
    for (const block of blocks) {
        mass += BigInt(block.field_tokens[0]) + BigInt(block.field_tokens[1]);
        px += BigInt(block.momentum[0][0]) + BigInt(block.momentum[1][0]);
        py += BigInt(block.momentum[0][1]) + BigInt(block.momentum[1][1]);
        pz += BigInt(block.momentum[0][2]) + BigInt(block.momentum[1][2]);
    }
    return { mass, momentum: [px, py, pz] };
}

function updateReadouts() {
    elements.stage.textContent = String(stageIndex);
    const gamma = fitDecayRate(amplitudeHistory);
    if (gamma === null || !Number.isFinite(gamma)) {
        elements.gammaFit.textContent = '—'; elements.nuFit.textContent = '—';
    } else {
        elements.gammaFit.textContent = gamma.toExponential(4);
        elements.nuFit.textContent = (gamma / sidecar.k_squared).toFixed(6);
    }
    const probed = sidecar.constant_probed;
    elements.probedName.textContent = `Constant probed: ${probed}`;
    elements.probedValue.textContent = sidecar.constants[probed].float.toFixed(6);
    elements.nuT2.textContent = sidecar.constants.nu_T2.float.toFixed(6);
    elements.nuE.textContent = sidecar.constants.nu_E.float.toFixed(6);
    elements.csq.textContent = sidecar.constants.c_s2.float.toFixed(6);
    elements.gconst.textContent = sidecar.constants.g.float.toFixed(6);
}

function controls() {
    elements.step.disabled = busy || running || !ownerId;
    elements.run.disabled = !ownerId || (busy && !running);
    elements.run.textContent = running ? 'Pause' : 'Run';
}

function clearPublication() {
    lastBlocks = null; amplitudeHistory = []; stageIndex = 0; sidecar = null; manifest = null;
    window.__hydroLabSnapshot = null;
    for (const canvas of [densityCanvas, momentumCanvas, elements.chart]) canvas.getContext('2d').clearRect(0, 0, canvas.width, canvas.height);
    for (const key of ['stage', 'gammaFit', 'nuFit', 'probedValue', 'nuT2', 'nuE', 'csq', 'gconst', 'mass', 'momentum']) elements[key].textContent = '—';
    elements.probedName.textContent = 'Constant probed';
    elements.provenance.textContent = 'Waiting for one authoritative owner.';
}

function fail(error, localEpoch = epoch) {
    if (localEpoch !== epoch) return;
    epoch++; running = false; busy = false; ownerId = null; generation = '0';
    terminate(); clearPublication(); elements.status.textContent = String(error.message || error); controls();
}

async function observeAndRecord(localEpoch) {
    const L = Number(elements.size.value);
    const width = String(L === 32 ? 2 : 1);
    const side = L / Number(width);
    const [mx, my, mz] = sidecar.k_integers;
    const fields = await request('observe', { width, observable: 'fields' });
    const moments = await request('observe', { width: '1', observable: `moments:${mx},${my},${mz},0` });
    if (localEpoch !== epoch) return;
    if (fields.ownerId !== moments.ownerId || fields.generation !== moments.generation
        || fields.payload.microtick !== moments.payload.microtick) {
        throw new Error('Observation lineage mismatch');
    }
    const blocks = fields.payload.blocks;
    lastBlocks = blocks; lastSide = side; lastZMid = Math.floor(side / 2);
    const amplitude = projectedAmplitude(moments.payload, sidecar.polarization_vector);
    amplitudeHistory.push(amplitude);
    const totals = aggregateTotals(blocks);
    elements.mass.textContent = totals.mass.toString();
    elements.momentum.textContent = `(${totals.momentum.map((v) => v.toString()).join(', ')})`;
    repaint();
    updateReadouts();
    window.__hydroLabSnapshot = {
        ownerId, generation, stage: stageIndex, microtick: fields.payload.microtick,
        preparation: elements.preparation.value, L, mass: totals.mass.toString(),
        momentum: totals.momentum.map((v) => v.toString()), amplitude,
    };
}

async function initialize() {
    const localEpoch = ++epoch;
    running = false; busy = true; terminate(); ownerId = null; generation = '0'; clearPublication(); controls();
    elements.status.textContent = 'Loading a fresh finite preparation…';
    try {
        const preparation = elements.preparation.value;
        const L = Number(elements.size.value);
        const manifestUrl = new URL('../../../build_strict_hydro/lab/manifest.json', import.meta.url);
        const manifestResponse = await fetch(manifestUrl);
        if (!manifestResponse.ok) throw new Error('Local hydro preparations are missing. Run prepare_hydro_lab.py.');
        const fetchedManifest = await manifestResponse.json();

        const sidecarUrl = new URL(`../../../build_strict_hydro/lab/${preparation}_${L}.json`, import.meta.url);
        const sidecarResponse = await fetch(sidecarUrl);
        if (!sidecarResponse.ok) throw new Error('Local hydro preparations are missing. Run prepare_hydro_lab.py.');
        const fetchedSidecar = await sidecarResponse.json();

        const binUrl = new URL(`../../../build_strict_hydro/lab/${preparation}_${L}.bin`, import.meta.url);
        const binResponse = await fetch(binUrl);
        if (!binResponse.ok) throw new Error('Local hydro preparations are missing. Run prepare_hydro_lab.py.');
        const binBuffer = await binResponse.arrayBuffer();
        if (localEpoch !== epoch) return;

        const checkpoint = ftdhy01ToCheckpointJson(binBuffer, fetchedManifest);
        const tableUrl = new URL(`../../../build_strict_hydro_tables/hydro_collision_${fetchedManifest.table_hash16}.u32`, import.meta.url);
        const wasmModuleUrl = new URL('../../../build_strict_hydro_wasm/ftd_hydro_wasm.mjs', import.meta.url);
        const wasmBinaryUrl = new URL('../../../build_strict_hydro_wasm/ftd_hydro_wasm.wasm', import.meta.url);
        const wasmBinaryResponse = await fetch(wasmBinaryUrl);
        const wasmModuleHash = wasmBinaryResponse.ok ? await sha256Hex(await wasmBinaryResponse.arrayBuffer()) : 'unavailable';
        if (localEpoch !== epoch) return;

        worker = new Worker(new URL('../../../web/js/strict/hydro-worker.js', import.meta.url), { type: 'module' });
        worker.onmessage = ({ data }) => {
            if (localEpoch !== epoch) return;
            const item = pending.get(data.requestId); if (!item) return;
            pending.delete(data.requestId);
            if (!data.ok) { item.reject(new Error(data.error)); return; }
            if (ownerId !== null && data.ownerId !== ownerId) { item.reject(new Error('Foreign owner response')); return; }
            if (typeof data.ownerId !== 'string' || !data.ownerId || data.generation !== item.expectedGeneration) {
                item.reject(new Error('Invalid response lineage or generation')); return;
            }
            ownerId = data.ownerId; generation = data.generation; item.resolve(data);
        };
        worker.onerror = (event) => { if (localEpoch !== epoch) return; fail(new Error(event.message || 'Worker failed'), localEpoch); };

        await request('init', { wasmModuleUrl: wasmModuleUrl.href, tableUrl: tableUrl.href, checkpoint });
        manifest = fetchedManifest; sidecar = fetchedSidecar; stageIndex = 0; amplitudeHistory = [];
        elements.provenance.textContent = JSON.stringify({
            lawId: manifest.law_id, tableHash: manifest.table_hash, tableHash16: manifest.table_hash16,
            preparationSha256: sidecar.bin_sha256, wasmModuleHash, ownerId, generation,
        }, null, 2);
        await observeAndRecord(localEpoch);
        if (localEpoch === epoch) elements.status.textContent = 'Paused. One stage advances four microticks.';
    } catch (error) { fail(error, localEpoch); }
    finally { if (localEpoch === epoch) { busy = false; controls(); } }
}

async function step() {
    if (busy || !ownerId) return;
    const localEpoch = epoch; busy = true; controls();
    try {
        await request('advance', { microticks: '4' });
        stageIndex += 1;
        await observeAndRecord(localEpoch);
        if (localEpoch === epoch) elements.status.textContent = running ? 'Running the successor.' : 'Paused. One stage (4 microticks) completed.';
    } catch (error) { fail(error, localEpoch); }
    finally { if (localEpoch === epoch) { busy = false; controls(); } }
}

elements.reset.onclick = initialize;
elements.size.onchange = initialize;
elements.preparation.onchange = initialize;
elements.step.onclick = () => step();
elements.run.onclick = () => { running = !running; controls(); };

function frame(time) {
    if (running && !busy && time - lastPublish >= 40) { lastPublish = time; void step(); }
    requestAnimationFrame(frame);
}
new ResizeObserver(() => repaint()).observe(document.body);
window.addEventListener('pagehide', () => {
    epoch++; running = false; busy = false; ownerId = null; generation = '0';
    terminate(); clearPublication(); controls();
    elements.status.textContent = 'Runtime unloaded. Reset to start a fresh preparation.';
});
requestAnimationFrame(frame);
void initialize();
