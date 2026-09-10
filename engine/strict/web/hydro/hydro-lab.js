// Opt-in local laboratory for phi-hydro-staged-candidate-1 (Task 13). All evolution
// occurs in the compiled worker; this file only converts the FTDHY01 preparation
// transport into the WASM's "ftd-hydro-checkpoint-2" JSON checkpoint schema (the hydro
// bindings never accept the native binary transport directly -- see
// engine/strict/hydro/wasm_bindings_hydro.cpp and task-10-report.md), drives
// advance/observe, and renders the two field canvases and the projected-mode chart.
import { validateHydroPreparation, verifyRuntimeArtifact, supportsShearDecayFit } from './hydro-preparation.js';
import { hydroObservationValidity, hydroFailureValidity } from './hydro-validity.js';
import { createValidityIndicator } from '../../../web/js/ui/components/validity-status.js';
const byId = (id) => document.getElementById(id);
const validity = createValidityIndicator(byId('hydro-validity'), { id: 'hydro-validity-status' });
validity.reset('Waiting for a validated preparation and its first observation.');
const elements = Object.fromEntries([
    'preparation', 'size', 'reset', 'step', 'run', 'status',
    'stage', 'gammaFit', 'nuFit', 'probedName', 'probedValue', 'probedLabel', 'fitScope',
    'nuT2', 'nuE', 'csq', 'gconst', 'mass', 'momentum',
    'density', 'momentum-canvas', 'chart', 'provenance',
].map((id) => [id, byId(id)]));
const densityCanvas = elements.density;
const momentumCanvas = elements['momentum-canvas'];

let worker = null, ownerId = null, generation = '0', requestSequence = 0, epoch = 0;
let running = false, busy = false, lastPublish = 0;
let manifest = null, sidecar = null, stageIndex = 0, amplitudeHistory = [];
let lastBlocks = null, lastSide = 0, lastZMid = 0;
let lastObservedMicrotick = null;
const pending = new Map();

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
    measured.forEach((v, i) => {
        if (!Number.isFinite(v)) return;
        const x = toX(i), y = toY(v); ctx.beginPath(); ctx.arc(x, y, radius, 0, Math.PI * 2); ctx.fill();
    });
}

function repaint() { paintDensity(densityCanvas, lastBlocks, lastSide, lastZMid); paintMomentum(momentumCanvas, lastBlocks, lastSide, lastZMid); paintChart(elements.chart, amplitudeHistory, sidecar ? sidecar.prediction.amplitude : []); }

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

function updateReadouts(fit) {
    elements.stage.textContent = String(stageIndex);
    const shearFit = supportsShearDecayFit(sidecar);
    const gamma = fit.gamma;
    elements.fitScope.textContent = shearFit ? 'last ≤32 stages; empirical amplitude fit' : 'Sound frequency fit unavailable';
    elements.probedLabel.textContent = shearFit ? 'Decay-based diffusivity estimate' : 'Unavailable for oscillating sound mode';
    if (gamma === null || !Number.isFinite(gamma)) {
        elements.gammaFit.textContent = '—'; elements.nuFit.textContent = '—';
    } else {
        elements.gammaFit.textContent = gamma.toExponential(4);
        elements.nuFit.textContent = fit.diffusivity.toFixed(6);
    }
    const probed = sidecar.constant_probed;
    elements.probedName.textContent = `Reference constant: ${probed}`;
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
    lastObservedMicrotick = null;
    validity.reset('Waiting for a validated preparation and its first observation.');
    window.__hydroLabSnapshot = null;
    for (const canvas of [densityCanvas, momentumCanvas, elements.chart]) canvas.getContext('2d').clearRect(0, 0, canvas.width, canvas.height);
    for (const key of ['stage', 'gammaFit', 'nuFit', 'probedValue', 'nuT2', 'nuE', 'csq', 'gconst', 'mass', 'momentum']) elements[key].textContent = '—';
    elements.probedName.textContent = 'Reference constant';
    elements.probedLabel.textContent = 'Decay-based diffusivity estimate';
    elements.fitScope.textContent = 'last ≤32 stages; empirical amplitude fit';
    elements.provenance.textContent = 'Waiting for one authoritative owner.';
}

function fail(error, localEpoch = epoch) {
    if (localEpoch !== epoch) return;
    const failure = hydroFailureValidity(error, lastObservedMicrotick);
    epoch++; running = false; busy = false; ownerId = null; generation = '0';
    terminate(); clearPublication(); elements.status.textContent = String(error.message || error); controls();
    validity.set(failure);
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
        || fields.payload.microtick !== moments.payload.microtick
        || fields.payload.L !== String(L) || moments.payload.L !== String(L)
        || fields.payload.phase !== moments.payload.phase || moments.payload.polarity !== '0'
        || !Array.isArray(moments.payload.k) || moments.payload.k.length !== 3
        || moments.payload.k.some((value, i) => value !== String(sidecar.k_integers[i]))
        || fields.payload.width !== width || fields.payload.law !== manifest.law_id
        || moments.payload.law !== manifest.law_id
        || fields.payload.table_hash !== manifest.table_hash || moments.payload.table_hash !== manifest.table_hash) {
        throw new Error('Observation lineage mismatch');
    }
    const blocks = fields.payload.blocks;
    lastBlocks = blocks; lastSide = side; lastZMid = Math.floor(side / 2);
    const amplitude = projectedAmplitude(moments.payload, sidecar.polarization_vector);
    // Retain a gap at this stage, rather than shifting later points in time.
    const displayedAmplitude = Number.isFinite(amplitude) && amplitude >= 0 ? amplitude : null;
    amplitudeHistory.push(displayedAmplitude);
    const checked = hydroObservationValidity({ amplitude, history: amplitudeHistory,
        kSquared: sidecar.k_squared, fitEnabled: supportsShearDecayFit(sidecar), microtick: fields.payload.microtick });
    lastObservedMicrotick = fields.payload.microtick;
    const totals = aggregateTotals(blocks);
    elements.mass.textContent = totals.mass.toString();
    elements.momentum.textContent = `(${totals.momentum.map((v) => v.toString()).join(', ')})`;
    repaint();
    updateReadouts(checked.fit);
    validity.set(checked.status);
    window.__hydroLabSnapshot = {
        ownerId, generation, stage: stageIndex, microtick: fields.payload.microtick,
        preparation: elements.preparation.value, L, mass: totals.mass.toString(),
        momentum: totals.momentum.map((v) => v.toString()), amplitude: displayedAmplitude,
        observationValid: checked.amplitudeValid && !checked.fit.issue,
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
        const sidecarBuffer = await sidecarResponse.arrayBuffer();

        const binUrl = new URL(`../../../build_strict_hydro/lab/${preparation}_${L}.bin`, import.meta.url);
        const binResponse = await fetch(binUrl);
        if (!binResponse.ok) throw new Error('Local hydro preparations are missing. Run prepare_hydro_lab.py.');
        const binBuffer = await binResponse.arrayBuffer();
        if (localEpoch !== epoch) return;

        const { checkpoint, sidecar: fetchedSidecar, binHash } = await validateHydroPreparation({
            manifest: fetchedManifest, sidecarBuffer, binBuffer, preparation, L,
        });
        const tableUrl = new URL(`../../../build_strict_hydro_tables/hydro_collision_${fetchedManifest.table_hash16}.u32`, import.meta.url);
        const wasmModuleUrl = new URL('../../../build_strict_hydro_wasm/ftd_hydro_wasm.mjs', import.meta.url);
        const wasmBinaryUrl = new URL('../../../build_strict_hydro_wasm/ftd_hydro_wasm.wasm', import.meta.url);
        const fetchedRuntimeArtifacts = {};
        for (const [name, url] of [['loader', wasmModuleUrl], ['binary', wasmBinaryUrl]]) {
            const response = await fetch(url);
            if (!response.ok) throw new Error(`Hydro WASM ${name} fetch failed: ${response.status}`);
            fetchedRuntimeArtifacts[name] = await verifyRuntimeArtifact(
                await response.arrayBuffer(), url, fetchedManifest, manifestUrl);
        }
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
            preparationSha256: binHash, preparationAndSidecarVerified: true,
            fetchedRuntimeArtifacts, runtimeDigestScope: 'Fetched artifact bytes; worker loads its own module',
            ownerId, generation,
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
