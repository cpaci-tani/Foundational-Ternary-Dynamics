// One compiled worker owns evolution. Published views never write microscopic state.
import { prepareFluid, HYDRO_IDENTITY } from './fluid-preparation.js';
import { analyzeFluid, fitMode } from './fluid-analysis.js';
import { fmt, renderSlice, renderChart, sliceBlock } from './fluid-renderer.js';
import { createValidityIndicator } from '../../../web/js/ui/components/validity-status.js';

const $ = id => document.getElementById(id);
let validity = createValidityIndicator($('hydro-validity'), { id: 'hydro-validity-status' });
let worker = null, ownerId = null, generation = '0', sequence = 0, epoch = 0;
let running = false, busy = false, live = true, dirty = false, lastAdvance = 0, frameId = 0;
let config = null, publication = null, analysis = null, history = [], initialLedger = null;
let observer = null, preparationHash = null;
const pending = new Map();
const paintTimes = [];
function request(op, fields = {}) {
    const requestId = String(++sequence);
    const message = { requestId, op, ...fields };
    if (op !== 'init') Object.assign(message, { ownerId, generation });
    return new Promise((resolve, reject) => {
        const expectedGeneration = op === 'init' ? '0' : String(BigInt(generation) + (op === 'advance' && BigInt(fields.microticks) > 0n ? 1n : 0n));
        const timeout = setTimeout(() => { pending.delete(requestId); reject(new Error('Runtime request timed out')); }, 30000);
        pending.set(requestId, { resolve, reject, expectedGeneration, timeout });
        worker.postMessage(message);
    });
}
function terminate() {
    worker?.terminate(); worker = null;
    for (const item of pending.values()) { clearTimeout(item.timeout); item.reject(new Error('Owner replaced')); }
    pending.clear(); ownerId = null; generation = '0';
}
function controls() {
    $('step').disabled = busy || running || !ownerId;
    $('run').disabled = !ownerId || (busy && !running);
    $('run').textContent = running ? 'Pause' : 'Run';
    $('width').disabled = busy;
}
function clearPublication() {
    publication = null; analysis = null; history = []; initialLedger = null; dirty = false;
    window.__hydroLabSnapshot = null;
    for (const id of ['microtick','stage','phase','mass','momentum','speed','pressure','stress','gammaFit','nuFit','r2','all-fields','relations','work']) $(id).textContent = '—';
    for (const id of ['density','stress-canvas','flow-canvas','chart']) {
        const c = $(id); c.getContext('2d').clearRect(0,0,c.width,c.height);
    }
    for (const id of ['density-legend','stress-legend','flow-legend']) $(id).textContent = '—';
    $('source').textContent = 'No advance interval yet.';
    $('fitScope').textContent = 'Waiting for observations.';
    $('provenance').textContent = '';
    $('inspector').textContent = 'Cell inspection will appear here.';
}
function fail(error, localEpoch) {
    if (localEpoch !== epoch) return;
    epoch++; running = false; busy = false; terminate(); clearPublication(); controls();
    $('status').textContent = String(error.message || error);
    validity.set({ label: 'Invalid', severity: 'error', details: String(error.message || error) + '. No valid observation is published.' });
}
function validateLineage(response) {
    const p = response.payload;
    if (response.ownerId !== ownerId || response.generation !== generation || p.law !== HYDRO_IDENTITY.law
        || p.table_hash !== HYDRO_IDENTITY.table || p.L !== String(config.L)
        || !/^(0|[1-9][0-9]*)$/.test(p.microtick) || p.phase !== String(BigInt(p.microtick) % 4n)) throw new Error('Observation lineage mismatch');
    return p;
}
function updateViews() {
    if (!publication) return;
    analysis = analyzeFluid(publication, $('polarity').value);
    const side = analysis.side;
    $('slice').max = String(side - 1); $('slice').value = String(Math.min(Number($('slice').value), side - 1));
    $('slice-label').textContent = String(Number($('slice').value) * analysis.width) + ' cells';
    $('mass').textContent = String(analysis.total);
    $('momentum').textContent = 'P = (' + analysis.momentum.join(', ') + ')';
    $('speed').textContent = fmt(analysis.rmsSpeed);
    $('pressure').textContent = fmt(analysis.pressure); $('stress').textContent = fmt(analysis.stressNorm);
    const ledger = publication.ledger;
    $('microtick').textContent = publication.microtick;
    $('stage').textContent = String(BigInt(publication.microtick) / 4n); $('phase').textContent = publication.phase;
    $('all-fields').textContent = String(ledger.field_tokens_by_polarity.reduce((s,v)=>s+BigInt(v),0n));
    $('relations').textContent = String(BigInt(ledger.relation_tokens_sc)+BigInt(ledger.relation_tokens_fcc));
    $('work').textContent = ledger.work_units;
    const src = publication.source;
    $('source').textContent = src.status === 'available'
        ? 'Ticks ' + src.from_microtick + ' → ' + src.to_microtick + ': ' + src.transferred_tokens + ' tokens transferred to relations. Field ΔN = (' + src.tokens_by_polarity.join(', ') + '); ΔP₀ = (' + src.momentum_by_polarity[0].join(', ') + '), ΔP₁ = (' + src.momentum_by_polarity[1].join(', ') + ').'
        : 'No completed advance interval in this owner.';
    const fit = fitMode(history, config.kSquared, config.shear);
    $('gammaFit').textContent = fit.available ? fmt(fit.gamma) : '—';
    $('nuFit').textContent = fit.available ? fmt(fit.diffusivity) : '—';
    $('r2').textContent = fit.available ? fmt(fit.r2) : '—';
    $('fitScope').textContent = fit.available ? fit.samples + ' samples · cycles ' + fit.from + '–' + fit.to + ' · log RMS ' + fmt(fit.logRms) + '. Uncertainty not certified.' : fit.reason;
    $('provenance').textContent = JSON.stringify({
        ownerId, generation, law: publication.law, table: publication.table_hash, encoding: publication.encoding_hash,
        microtick: publication.microtick, phase: publication.phase, observationPhase: 'snapshot',
        spatialSupport: { L: config.L, blockWidth: analysis.width, sitesPerBlock: publication.block_sites },
        interval: src.status === 'available' ? [src.from_microtick,src.to_microtick] : null,
        snapshotInterval: [publication.microtick,publication.microtick], preparation: config, preparationSha256: preparationHash,
        rawMoments: 'exact integer decimal strings; bank phases collapsed',
        derivedArithmetic: 'binary64 display arithmetic; no certified error bound',
        stencil: 'periodic central differences; adjacent blocks; unavailable at vacuum or side<3',
        polarityPolicy: $('polarity').value, units: 'lattice cells, four-microtick cycles, field tokens',
        fit: { ...fit, status: 'empirical finite-wave diagnostic' },
        viscosity: 'unavailable: constitutive recovery open',
        runtimeIdentityScope: 'Native law/encoding and SHA-verified collision table; loader bytes are not independently pinned by this interactive UI.'
    }, null, 2);
    window.__hydroLabSnapshot = { ownerId, generation, microtick: publication.microtick, stage: Number(BigInt(publication.microtick)/4n),
        preparation: config.preparation, L: config.L, width: analysis.width, polarity: analysis.polarity,
        mass: String(analysis.total), momentum: analysis.momentum.map(String), work: ledger.work_units,
        historyLength: history.length, amplitude: history.at(-1)?.amplitude ?? null, source: src,
        derivativeBlocks: analysis.derivativeBlocks, viscosityAvailable: false, observationValid: true };
    dirty = true;
}
async function observe(localEpoch, record = true) {
    const response = await request('observe', { width: $('width').value, observable: 'fluid' });
    if (localEpoch !== epoch) return;
    const p = validateLineage(response);
    if (p.width !== $('width').value || p.encoding_hash !== HYDRO_IDENTITY.encoding) throw new Error('Block observation support mismatch');
    if (record) {
        const momentsResponse = await request('observe', { width: '1', observable: 'moments:' + config.k.join(',') + ',0' });
        if (localEpoch !== epoch) return;
        const m = validateLineage(momentsResponse);
        if (m.microtick !== p.microtick || m.polarity !== '0' || m.k.join(',') !== config.k.join(',')) throw new Error('Fourier observation lineage mismatch');
        let re = 0, im = 0;
        config.polarization.forEach((v,i)=>{ re += v*m.moments[i+1][0]; im += v*m.moments[i+1][1]; });
        const amplitude = Math.hypot(re,im) / config.L ** 3; // native moments are extensive
        const cycle = Number(BigInt(p.microtick))/4;
        if (!Number.isSafeInteger(cycle)) throw new Error('Clock exceeds chart precision');
        if (history.at(-1)?.cycle !== cycle) history.push({ cycle, amplitude: Number.isFinite(amplitude) ? amplitude : null });
        if (history.length > 256) history.shift();
    }
    // Frozen occupied relations make this interactive sector exactly source-free.
    // Check accounting independently of the display's selected polarity/block width.
    const ledger = p.ledger;
    if (initialLedger && (ledger.work_units !== initialLedger.work_units
        || ledger.field_tokens_by_polarity.join(',') !== initialLedger.field_tokens_by_polarity.join(',')
        || JSON.stringify(ledger.momentum_by_polarity) !== JSON.stringify(initialLedger.momentum_by_polarity))) throw new Error('Frozen-sector conservation mismatch');
    initialLedger ??= structuredClone(ledger);
    publication = p; updateViews();
    validity.set({ label: 'Observed', severity: 'neutral', details: 'Exact finite-record moments and source accounting are available. Central stress includes unresolved motion. Viscosity and nonlinear fluid recovery remain open; numerical observations are not physical certification.' });
}
async function initialize() {
    const localEpoch = ++epoch; running = false; busy = true; terminate(); clearPublication(); controls();
    validity.reset('Loading a deterministic interactive preparation.');
    $('status').textContent = 'Loading one compiled state owner…';
    try {
        const prepared = prepareFluid({ L: Number($('size').value), preparation: $('preparation').value,
            seed: Number($('seed').value), density: Number($('occupation').value) });
        config = prepared.config;
        // Bound main-thread publication/analysis cost without coarsening the state.
        const minimumWidth = Math.max(1, config.L / 16);
        for (const option of $('width').options) option.disabled = Number(option.value) < minimumWidth;
        if (Number($('width').value) < minimumWidth) $('width').value = String(minimumWidth);
        const digest = await crypto.subtle.digest('SHA-256', new TextEncoder().encode(prepared.checkpoint));
        if (localEpoch !== epoch) return;
        preparationHash = Array.from(new Uint8Array(digest), b=>b.toString(16).padStart(2,'0')).join('');
        const w = new Worker(new URL('../../../web/js/strict/hydro-worker.js', import.meta.url), { type:'module' }); worker = w;
        w.onmessage = ({ data }) => {
            if (localEpoch !== epoch) return;
            const item = pending.get(data.requestId); if (!item) return;
            pending.delete(data.requestId); clearTimeout(item.timeout);
            if (!data.ok) { item.reject(new Error(data.error)); return; }
            if (typeof data.ownerId !== 'string' || !data.ownerId || (ownerId && data.ownerId !== ownerId)
                || data.generation !== item.expectedGeneration) { item.reject(new Error('Invalid owner or generation')); return; }
            ownerId = data.ownerId; generation = data.generation; item.resolve(data);
        };
        w.onerror = event => fail(new Error(event.message || 'Worker failed'), localEpoch);
        await request('init', {
            wasmModuleUrl: new URL('../../../build_strict_hydro_wasm/ftd_hydro_wasm.mjs', import.meta.url).href,
            tableUrl: new URL('../../../build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32', import.meta.url).href,
            checkpoint: prepared.checkpoint
        });
        if (localEpoch !== epoch) return;
        await observe(localEpoch);
        if (localEpoch === epoch) $('status').textContent = 'Paused · seeded interactive preparation · one cycle = 4 physical microticks.';
    } catch (error) { fail(error,localEpoch); }
    finally { if (localEpoch === epoch) { busy = false; controls(); } }
}
async function step() {
    if (busy || !ownerId) return;
    const localEpoch = epoch; busy = true; controls();
    try {
        await request('advance', { microticks:'4', publication:'diagnostics' });
        if (localEpoch !== epoch) return;
        await observe(localEpoch);
        if (localEpoch === epoch) $('status').textContent = running ? 'Running · every required microtick executes · observations at most 5 times / second.' : 'Paused · cycle completed and observed.';
    } catch(error) { fail(error,localEpoch); }
    finally { if (localEpoch === epoch) { busy = false; controls(); } }
}
function repaint() {
    if (!analysis) return;
    const options = { plane:$('plane').value, slice:Number($('slice').value) };
    const density = renderSlice($('density'),analysis,options,b=>b.density,false,$('arrows').checked);
    const sm = $('stress-mode').value, component = {xy:3,xz:4,yz:5}[sm];
    const stress = renderSlice($('stress-canvas'),analysis,options,b=>component === undefined ? b[sm] : b.stress[component],component !== undefined);
    const fm = $('flow-mode').value;
    const signedFlow = fm==='vorticity' || fm==='divergence';
    const flow = renderSlice($('flow-canvas'),analysis,options,(b,normal)=>fm==='vorticity' ? b.curl?.[normal] ?? null : fm==='divergence' ? b.divergence : fm==='strainNorm' ? b.strainNorm : b.advection ? Math.hypot(...b.advection) : null,signedFlow);
    $('density-legend').textContent = density.range + ' tokens / cell³ · arrow max ' + fmt(density.speedMax) + ' cells / cycle'
        + ' · occupied ' + fmt(100 * analysis.total / (config.L ** 3 * (analysis.polarity === 'both' ? 48 : 24))) + '%';
    $('stress-legend').textContent = stress.range + ' token flux density · ' + (component===undefined ? 'dark → bright' : 'blue − / amber +');
    const normalAxis = {XY:'Z',XZ:'Y',YZ:'X'}[options.plane];
    $('flow-legend').textContent = (fm==='vorticity' ? 'ω' + normalAxis + ' · ' : '') + flow.range + (fm==='advection' ? ' cells / cycle²' : ' / cycle')
        + (signedFlow ? ' · blue − / amber +' : ' · dark → bright') + ' · ' + flow.missing + ' undefined cells';
    renderChart($('chart'),history);
}
function frame(time) {
    if (!live) return;
    if (running && !busy && time - lastAdvance >= 200) { lastAdvance = time; void step(); }
    if (dirty) { const start = performance.now(); repaint(); dirty = false; paintTimes.push(performance.now()-start); if(paintTimes.length>256)paintTimes.shift(); }
    frameId = requestAnimationFrame(frame);
}
function inspect(canvas,event) {
    if (!analysis) return;
    const rect = canvas.getBoundingClientRect();
    const col = Math.max(0,Math.min(analysis.side-1,Math.floor((event.clientX-rect.left)/rect.width*analysis.side)));
    const row = analysis.side-1-Math.max(0,Math.min(analysis.side-1,Math.floor((event.clientY-rect.top)/rect.height*analysis.side)));
    const {xyz,block:b} = sliceBlock(analysis,$('plane').value,Number($('slice').value),col,row);
    $('inspector').textContent = 'Block ('+xyz.join(', ')+') · N='+b.N+' · P=('+b.P.join(', ')+') · u='+(b.velocity?'('+b.velocity.map(fmt).join(', ')+')':'undefined (vacuum)')+' · C[xx,yy,zz,xy,xz,yz]=('+b.stress.map(fmt).join(', ')+') · div u='+fmt(b.divergence);
}
$('reset').onclick = initialize;
for(const id of ['preparation','size','occupation','seed']) $(id).onchange = initialize;
$('step').onclick = step;
$('run').onclick = ()=>{running=!running;controls();};
$('width').onchange = async ()=>{
    if (!ownerId || busy) return;
    const localEpoch=epoch;busy=true;controls();
    try {await observe(localEpoch,false);}catch(error){fail(error,localEpoch);}finally{if(localEpoch===epoch){busy=false;controls();}}
};
$('polarity').oninput = ()=>{if(publication)updateViews();};
for(const id of ['plane','slice','arrows','stress-mode','flow-mode']) $(id).oninput = ()=>{
    if (!analysis) return;
    $('slice-label').textContent = String(Number($('slice').value) * analysis.width) + ' cells';
    dirty = true;
};
for(const id of ['density','stress-canvas','flow-canvas']) {
    $(id).onpointermove = event=>inspect($(id),event);
    $(id).onclick = event=>inspect($(id),event);
    $(id).onkeydown = event=>{ if(event.key==='Enter'||event.key===' '){event.preventDefault();const rect=$(id).getBoundingClientRect();inspect($(id),{clientX:rect.left+rect.width/2,clientY:rect.top+rect.height/2});}};
}
function startRendering(){live=true;observer=new ResizeObserver(()=>{dirty=true;});observer.observe(document.body);frameId=requestAnimationFrame(frame);}
window.addEventListener('pagehide',()=>{epoch++;live=false;running=false;busy=false;terminate();observer?.disconnect();observer=null;cancelAnimationFrame(frameId);clearPublication();controls();validity.destroy();});
window.addEventListener('pageshow',event=>{if(event.persisted){validity=createValidityIndicator($('hydro-validity'),{id:'hydro-validity-status'});startRendering();void initialize();}});
window.__hydroLabPerformance = ()=>({paintMs:[...paintTimes],pendingRequests:pending.size,running,busy,publicationCadenceMs:200,historyLength:history.length});
startRendering(); void initialize();
