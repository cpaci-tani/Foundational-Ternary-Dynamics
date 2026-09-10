// Bounded complete-state observations of the unchanged compiled hydro law.
// Predictions and weak-balance certification belong to separate instruments.
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync, existsSync, realpathSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { performance } from 'node:perf_hooks';
import { deflateRawSync } from 'node:zlib';

const SELF = fileURLToPath(import.meta.url);
const ROOT = path.resolve(path.dirname(SELF), '../../..');
const LAW = 'phi-hydro-staged-candidate-1';
const TABLE = 'abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375';
const ENCODING = '3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c';
const VELOCITIES = [[1,0,0],[1,0,0],[-1,0,0],[-1,0,0],[0,1,0],[0,1,0],
  [0,-1,0],[0,-1,0],[0,0,1],[0,0,1],[0,0,-1],[0,0,-1],[1,1,0],
  [1,-1,0],[-1,1,0],[-1,-1,0],[1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
  [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]];
const SIZES = {s:1,bank:192,sc:6,fcc:12,admitted_sc:3,admitted_fcc:6,gate_sc:3,gate_fcc:6};
const RUNTIME = {
  module:'engine/build_strict_hydro_wasm/ftd_hydro_wasm.mjs',
  binary:'engine/build_strict_hydro_wasm/ftd_hydro_wasm.wasm',
  table:'engine/build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32',
  node:'engine/build_predictive_response/tools/node-v24.11.0-linux-x64/bin/node',
};
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const demand = (condition, message) => { if (!condition) throw new Error(message); };
const same = (actual, expected, name) => demand(JSON.stringify(actual) === JSON.stringify(expected), `${name} mismatch`);
const counters = {actual_advance_calls:0, actual_microticks:0, completed_cases:0};

function localPath(base, relative) {
  demand(typeof relative === 'string' && relative.length > 0 && !relative.includes('\\') &&
    !path.posix.isAbsolute(relative) && relative.split('/').every(p => p && p !== '.' && p !== '..'), 'invalid relative path');
  const absolute = path.resolve(base, relative);
  demand(realpathSync(absolute).startsWith(`${realpathSync(base)}${path.sep}`), 'path escapes declared directory');
  return absolute;
}

function verifySources(sources) {
  demand(sources && typeof sources === 'object' && !Array.isArray(sources) && Object.keys(sources).length > 0, 'missing sources');
  for (const [relative, expected] of Object.entries(sources)) {
    demand(typeof expected === 'string' && /^[0-9a-f]{64}$/.test(expected), 'invalid source digest');
    demand(sha256(readFileSync(localPath(ROOT, relative))) === expected, `source drift: ${relative}`);
  }
}

function validateLock(lock) {
  demand(lock.schema === 'ftd-hydro-weak-balance-registration-v1', 'registration schema mismatch');
  demand(lock.law?.id === LAW && lock.law.table_sha256 === TABLE &&
    lock.law.encoding_sha256 === ENCODING && lock.law.blank === 4, 'law identity mismatch');
  same(lock.law.velocities, VELOCITIES, 'velocity order');
  for (const [name, expected] of Object.entries(RUNTIME)) same(lock.runtime?.[name], expected, `runtime.${name}`);
  const expected = {Ls:[4,8],backgrounds:['blank','frozen'],cycles:'L',max_runtime_seconds:60};
  for (const [name, value] of Object.entries(expected)) same(lock.experiment?.[name], value, `experiment.${name}`);
  const selfRelative = path.relative(ROOT, SELF).split(path.sep).join('/');
  for (const relative of [...Object.values(RUNTIME), selfRelative]) demand(typeof lock.sources?.[relative] === 'string', `unbound input: ${relative}`);
  demand(realpathSync(process.execPath) === realpathSync(localPath(ROOT, RUNTIME.node)), 'foreign Node executable');
  demand(lock.sources[RUNTIME.table] === TABLE, 'table digest mismatch');
}

function decodeCheckpoint(bytes, L, microtick) {
  const value = JSON.parse(bytes);
  same(Object.keys(value).sort(), ['schema','law','table','encoding','boundary','L','microtick','arrays'].sort(), 'checkpoint keys');
  for (const [key, expected] of Object.entries({schema:'ftd-hydro-checkpoint-2',law:LAW,
    table:TABLE,encoding:ENCODING,boundary:'periodic',L,microtick:String(microtick)})) same(value[key], expected, `checkpoint.${key}`);
  demand(value.arrays && typeof value.arrays === 'object', 'missing checkpoint arrays');
  same(Object.keys(value.arrays).sort(), Object.keys(SIZES).sort(), 'array keys');
  const arrays = {};
  for (const [name, size] of Object.entries(SIZES)) {
    const encoded = value.arrays[name];
    demand(typeof encoded === 'string', `invalid ${name}`);
    const raw = Buffer.from(encoded, 'base64');
    demand(raw.length === L ** 3 * size && raw.toString('base64') === encoded, `noncanonical ${name}`);
    for (const byte of raw) demand(name === 's' ? [0,1,255].includes(byte) :
      name === 'sc' || name === 'fcc' ? byte <= 8 : byte <= 1, `invalid ${name} alphabet`);
    arrays[name] = raw;
  }
  return arrays;
}

function validateInitial(arrays, L, background) {
  for (const [name, raw] of Object.entries(arrays)) if (name !== 'bank') {
    // frozen_background uses idx_of(encode(0,+1)) = 7 in the pinned A9 encoding.
    const expected = name === 'sc' || name === 'fcc' ? (background === 'blank' ? 4 : 7) : 0;
    demand(raw.every(value => value === expected), `initial ${background}/${name} mismatch`);
  }
  for (let site = 0; site < L ** 3; ++site) for (let pol = 0; pol < 2; ++pol) {
    const bytes = createHash('sha256').update(`ftd-weak-balance-v1:L:${L}:site:${site}:polarity:${pol}`).digest();
    const mask = bytes.readUIntLE(0, 3);
    for (let k = 0; k < 4; ++k) for (let v = 0; v < 24; ++v) {
      const expected = k === 0 ? (mask >>> v) & 1 : 0;
      demand(arrays.bank[site * 192 + pol * 96 + k * 24 + v] === expected, 'initial deterministic mask mismatch');
    }
  }
}

function stageRecord(state, L, tick) {
  const bytes = Buffer.from(state.checkpoint(), 'utf8');
  const arrays = decodeCheckpoint(bytes, L, tick);
  const fields = [0,0], momentum = [[0,0,0],[0,0,0]];
  for (let site = 0; site < L ** 3; ++site) for (let pol = 0; pol < 2; ++pol) {
    for (let v = 0; v < 24; ++v) {
      let occupied = 0;
      for (let k = 0; k < 4; ++k) occupied += arrays.bank[site * 192 + pol * 96 + k * 24 + v];
      demand(occupied <= 1, 'velocity exclusion violation');
      fields[pol] += occupied;
      VELOCITIES[v].forEach((component, a) => { momentum[pol][a] += occupied * component; });
    }
  }
  const sc = arrays.sc.reduce((n, b) => n + (b !== 4), 0);
  const fcc = arrays.fcc.reduce((n, b) => n + (b !== 4), 0);
  const ledger = {field_tokens:String(fields[0]+fields[1]),field_tokens_by_polarity:fields.map(String),
    field_momentum_by_polarity:momentum.map(row => row.map(String)),relation_tokens_sc:String(sc),
    relation_tokens_fcc:String(fcc),work_units:String(fields[0]+fields[1]+sc+fcc)};
  const diagnostics = JSON.parse(state.diagnostics());
  demand(diagnostics.microtick === String(tick) && diagnostics.L === String(L) &&
    diagnostics.work_units === ledger.work_units && diagnostics.phase === String(tick % 4), 'runtime ledger mismatch');
  return {microtick:String(tick),checkpoint_sha256:sha256(bytes),checkpoint_bytes:bytes.length,
    checkpoint_codec:'deflate-raw-base64',checkpoint_deflate_raw_base64:deflateRawSync(bytes).toString('base64'),
    array_sha256:Object.fromEntries(Object.entries(arrays).map(([name, raw]) => [name,sha256(raw)])),ledger};
}

function halfStep(state, L, expectedTick) {
  ++counters.actual_advance_calls;
  const returned = state.advance('2');
  counters.actual_microticks += 2;
  const result = JSON.parse(returned);
  demand(result.diagnostics?.microtick === String(expectedTick) && result.diagnostics.L === String(L), 'advance clock mismatch');
  demand(result.events && typeof result.events === 'object', 'missing actual runtime events');
  same(Object.keys(result.events).sort(), ['absorptions','collisions','crossings','gate_holds'].sort(), 'event categories');
  for (const rows of Object.values(result.events)) demand(Array.isArray(rows), 'invalid runtime events');
  return result.events;
}

async function main() {
  const started = performance.now();
  demand(process.platform === 'linux' && process.arch === 'x64' && process.version === 'v24.11.0', 'requires pinned Linux x64 Node v24.11.0');
  demand(process.argv.length === 5, 'usage: node weak_balance_runner.mjs REGISTRATION.json INPUT.json OUTPUT.json');
  const [lockPath,inputPath,outputPath] = process.argv.slice(2).map(p => path.resolve(p));
  demand(!existsSync(outputPath), 'output already exists');
  const lockBytes = readFileSync(lockPath), inputBytes = readFileSync(inputPath);
  const registrationHash = sha256(lockBytes), inputHash = sha256(inputBytes);
  const lock = JSON.parse(lockBytes), input = JSON.parse(inputBytes);
  validateLock(lock);
  verifySources(lock.sources);
  const inputRelative = path.relative(ROOT, realpathSync(inputPath)).split(path.sep).join('/');
  demand(lock.sources[inputRelative] === inputHash, 'input metadata is not bound by registration sources');
  demand(input.schema === 'ftd-hydro-weak-balance-input-v1' && Array.isArray(input.cases) && input.cases.length === 4, 'input schema/case count mismatch');
  if (input.registration_sha256 !== undefined) same(input.registration_sha256, registrationHash, 'input registration');
  const ids = new Set(), cells = new Set();
  const prepared = input.cases.map(row => {
    demand(typeof row.id === 'string' && /^[A-Za-z0-9_-]{1,64}$/.test(row.id) && !ids.has(row.id), 'invalid/duplicate case id');
    demand([4,8].includes(row.L) && row.cycles === row.L && ['blank','frozen'].includes(row.background), 'case parameters mismatch');
    const cell = `${row.L}:${row.background}`;
    demand(!cells.has(cell), 'duplicate registered cell'); ids.add(row.id); cells.add(cell);
    const file = localPath(path.dirname(inputPath), row.checkpoint_file), bytes = readFileSync(file);
    demand(sha256(bytes) === row.checkpoint_sha256, 'checkpoint identity mismatch');
    const relative = path.relative(ROOT, realpathSync(file)).split(path.sep).join('/');
    demand(lock.sources[relative] === row.checkpoint_sha256, 'checkpoint is not bound by registration sources');
    validateInitial(decodeCheckpoint(bytes, row.L, 0), row.L, row.background);
    return {row,file,bytes};
  });
  const wasmBytes = readFileSync(localPath(ROOT,RUNTIME.binary)), tableBytes = readFileSync(localPath(ROOT,RUNTIME.table));
  demand(sha256(wasmBytes) === lock.sources[RUNTIME.binary] && tableBytes.length === 67108864 && sha256(tableBytes) === TABLE, 'owned runtime bytes mismatch');
  const cases = [];
  try {
    const {default:createHydroModule} = await import(pathToFileURL(localPath(ROOT,RUNTIME.module)).href);
    const module = await createHydroModule({wasmBinary:wasmBytes});
    module.loadTable(new Uint8Array(tableBytes));
    for (const {row,bytes} of prepared) {
      const state = new module.HydroState(bytes.toString('utf8'));
      try {
        const initial = stageRecord(state,row.L,0), cycles = [];
        let previous = initial;
        for (let cycle = 1; cycle <= row.cycles; ++cycle) {
          const firstEvents = halfStep(state,row.L,4*cycle-2);
          const prestream = stageRecord(state,row.L,4*cycle-2);
          const secondEvents = halfStep(state,row.L,4*cycle);
          const final = stageRecord(state,row.L,4*cycle);
          demand(prestream.ledger.work_units === initial.ledger.work_units && final.ledger.work_units === initial.ledger.work_units, 'total work changed');
          cycles.push({cycle,initial_checkpoint_sha256:previous.checkpoint_sha256,prestream,final,
            first_half_events:firstEvents,second_half_events:secondEvents});
          previous = final;
        }
        cases.push({id:row.id,L:row.L,background:row.background,input_checkpoint_sha256:row.checkpoint_sha256,initial,cycles});
        ++counters.completed_cases;
      } finally {state.delete();}
    }
  } finally {
    verifySources(lock.sources);
    demand(sha256(readFileSync(lockPath)) === registrationHash && sha256(readFileSync(inputPath)) === inputHash, 'metadata drift');
    for (const {row,file} of prepared) demand(sha256(readFileSync(file)) === row.checkpoint_sha256, 'checkpoint drift');
  }
  const result = {schema:'ftd-hydro-weak-balance-wasm-v1',registration_sha256:registrationHash,input_sha256:inputHash,
    backend:{platform:process.platform,arch:process.arch,node_version:process.version,runtime:'compiled-wasm-cpu',
      node_sha256:lock.sources[RUNTIME.node],module_sha256:lock.sources[RUNTIME.module],binary_sha256:sha256(wasmBytes),
      table_sha256:TABLE,...counters},cases,source_postchecks:true,input_postchecks:true,
    elapsed_seconds:(performance.now()-started)/1000};
  writeFileSync(outputPath,`${JSON.stringify(result)}\n`,{flag:'wx',encoding:'utf8'});
}

main().catch(error => {
  console.error(`WEAK_BALANCE_FAILED: ${error instanceof Error ? error.stack : String(error)}`);
  console.error(JSON.stringify(counters));
  process.exitCode = 1;
});
