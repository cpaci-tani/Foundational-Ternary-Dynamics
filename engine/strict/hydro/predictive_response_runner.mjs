// Fixed, preregistered one-cycle probe of the existing compiled candidate law.
// No prediction, acceptance threshold, RNG, or table evolution is implemented here.
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync, existsSync, realpathSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { performance } from 'node:perf_hooks';

const SELF = fileURLToPath(import.meta.url);
const ROOT = path.resolve(path.dirname(SELF), '../../..');
const TABLE = 'abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375';
const ENCODING = '3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c';
const LAW = 'phi-hydro-staged-candidate-1';
const VELOCITIES = [[1,0,0],[1,0,0],[-1,0,0],[-1,0,0],[0,1,0],[0,1,0],
  [0,-1,0],[0,-1,0],[0,0,1],[0,0,1],[0,0,-1],[0,0,-1],[1,1,0],
  [1,-1,0],[-1,1,0],[-1,-1,0],[1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
  [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1]];
const SIZES = { s:1, bank:192, sc:6, fcc:12, admitted_sc:3,
  admitted_fcc:6, gate_sc:3, gate_fcc:6 };
const L = 16, SITES = L ** 3, REPLICATES = 32;
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const demand = (condition, message) => { if (!condition) throw new Error(message); };
const same = (actual, expected, name) => demand(JSON.stringify(actual) === JSON.stringify(expected), `${name} mismatch`);

function repositoryPath(relative) {
  demand(typeof relative === 'string' && relative.length > 0 &&
    !relative.includes('\\') && !path.posix.isAbsolute(relative) &&
    relative.split('/').every(part => part !== '' && part !== '.' && part !== '..'),
  'invalid repository path');
  const absolute = path.resolve(ROOT, relative);
  const real = realpathSync(absolute);
  demand(real.startsWith(`${realpathSync(ROOT)}${path.sep}`), 'repository path escapes root');
  return absolute;
}

function verifySources(sources) {
  demand(sources && typeof sources === 'object' && !Array.isArray(sources), 'missing source inventory');
  demand(Object.keys(sources).length > 0, 'empty source inventory');
  for (const [relative, expected] of Object.entries(sources)) {
    demand(typeof expected === 'string' && /^[0-9a-f]{64}$/.test(expected), 'invalid source digest');
    demand(sha256(readFileSync(repositoryPath(relative))) === expected, `source drift: ${relative}`);
  }
}

function validateLock(lock) {
  demand(lock.schema === 'ftd-hydro-predictive-response-v1', 'registration schema mismatch');
  demand(lock.law?.id === LAW && lock.law.table_sha256 === TABLE &&
    lock.law.encoding_sha256 === ENCODING && lock.law.blank === 4, 'law identity mismatch');
  same(lock.law.velocities, VELOCITIES, 'velocity order');
  const expected = { L:16, replicates:32, microticks:4, source_velocity:0,
    profile_direction:[1,2,3], profile_positive_residues:8, delta:'1/4', N:131072 };
  for (const [key, value] of Object.entries(expected)) same(lock.experiment?.[key], value, `experiment.${key}`);
  const paths = {
    module:'engine/build_strict_hydro_wasm/ftd_hydro_wasm.mjs',
    binary:'engine/build_strict_hydro_wasm/ftd_hydro_wasm.wasm',
    table:'engine/build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32',
  };
  for (const [key, value] of Object.entries(paths)) same(lock.runtime?.[key], value, `runtime.${key}`);
  const selfRelative = path.relative(ROOT, SELF).split(path.sep).join('/');
  const nodeRelative = path.relative(ROOT, realpathSync(process.execPath)).split(path.sep).join('/');
  for (const relative of [...Object.values(paths), selfRelative, nodeRelative]) {
    demand(typeof lock.sources?.[relative] === 'string', `unbound runtime input: ${relative}`);
  }
  demand(lock.sources[paths.table] === TABLE, 'table inventory mismatch');
}

function checkpoint(arrays, microtick) {
  return { schema:'ftd-hydro-checkpoint-2', law:LAW, table:TABLE, encoding:ENCODING,
    boundary:'periodic', L, microtick,
    arrays:Object.fromEntries(Object.entries(arrays).map(([name, raw]) => [name, raw.toString('base64')])) };
}

function initialArrays(masks, caseIndex) {
  const arrays = Object.fromEntries(Object.entries(SIZES).map(([name, size]) =>
    [name, Buffer.alloc(SITES * size, name === 'sc' || name === 'fcc' ? 4 : 0)]));
  let count = 0;
  for (let site = 0; site < SITES; ++site) {
    const mask = masks.readUInt32LE((caseIndex * SITES + site) * 4);
    demand(mask < 2 ** 24, 'mask exceeds 24-bit alphabet');
    for (let c = 0; c < 24; ++c) {
      const bit = (mask >>> c) & 1;
      arrays.bank[site * 192 + c] = bit;
      count += bit;
    }
  }
  return { arrays, count };
}

function finalArrays(text, initial) {
  const value = JSON.parse(text);
  for (const [key, expected] of Object.entries({ schema:'ftd-hydro-checkpoint-2', law:LAW,
    table:TABLE, encoding:ENCODING, boundary:'periodic', L, microtick:'4' })) same(value[key], expected, `final.${key}`);
  same(Object.keys(value).sort(), ['schema','law','table','encoding','boundary','L','microtick','arrays'].sort(), 'final keys');
  demand(value.arrays && typeof value.arrays === 'object', 'missing final arrays');
  same(Object.keys(value.arrays).sort(), Object.keys(SIZES).sort(), 'final array keys');
  const arrays = {};
  let count = 0;
  for (const [name, size] of Object.entries(SIZES)) {
    const encoded = value.arrays[name];
    demand(typeof encoded === 'string', `invalid final ${name}`);
    const raw = Buffer.from(encoded, 'base64');
    demand(raw.length === SITES * size && raw.toString('base64') === encoded, `noncanonical final ${name}`);
    if (name !== 'bank') demand(raw.equals(initial[name]), `unexpected final ${name} change`);
    else for (let index = 0; index < raw.length; ++index) {
      const channel = index % 192;
      demand(raw[index] <= 1, 'final bank alphabet violation');
      demand((channel >= 24 && channel < 48) || raw[index] === 0, 'final polarity/passive-phase violation');
      count += raw[index];
    }
    arrays[name] = raw;
  }
  return { arrays, count };
}

function pairedSums(initialBank, finalBank) {
  // Each sum is an integer in [-4096,4096]; all Number arithmetic here is exact.
  const sums = Array(24).fill(0);
  for (let x = 0; x < L; ++x) for (let y = 0; y < L; ++y) for (let z = 0; z < L; ++z) {
    const site = (x * L + y) * L + z;
    const sign = (x + 2 * y + 3 * z) % L < 8 ? 1 : -1;
    for (let c = 0; c < 24; ++c) {
      const [dx,dy,dz] = VELOCITIES[c];
      const destination = (((x + dx + L) % L) * L + (y + dy + L) % L) * L + (z + dz + L) % L;
      sums[c] += sign * (finalBank[destination * 192 + 24 + c] - initialBank[site * 192 + c]);
    }
  }
  return sums;
}

async function main() {
  const started = performance.now();
  demand(process.platform === 'linux' && process.arch === 'x64' && process.version === 'v24.11.0', 'requires pinned Linux x64 Node v24.11.0');
  demand(process.argv.length === 5, 'usage: node predictive_response_runner.mjs LOCK.json INPUT.json OUTPUT.json');
  const [lockPath, inputPath, outputPath] = process.argv.slice(2).map(p => path.resolve(p));
  demand(!existsSync(outputPath), 'output already exists');
  const lockBytes = readFileSync(lockPath), inputBytes = readFileSync(inputPath);
  const lockHash = sha256(lockBytes), inputHash = sha256(inputBytes);
  const lock = JSON.parse(lockBytes), input = JSON.parse(inputBytes);
  validateLock(lock);
  verifySources(lock.sources);
  demand(input.schema === 'ftd-hydro-response-input-v1' && input.registration_sha256 === lockHash, 'input registration mismatch');
  demand(input.masks_file === 'masks.bin' && input.entropy_file === 'entropy.bin', 'input filenames mismatch');
  const maskPath = path.join(path.dirname(inputPath), input.masks_file);
  const entropyPath = path.join(path.dirname(inputPath), input.entropy_file);
  const masks = readFileSync(maskPath), entropy = readFileSync(entropyPath);
  demand(masks.length === REPLICATES * SITES * 4 && sha256(masks) === input.masks_sha256, 'mask identity mismatch');
  demand(sha256(entropy) === input.entropy_sha256, 'entropy identity mismatch');
  // Reject the entire malformed mask input before constructing any runtime state.
  for (let offset = 0; offset < masks.length; offset += 4) demand(masks.readUInt32LE(offset) < 2 ** 24, 'mask exceeds 24-bit alphabet');
  const wasmBytes = readFileSync(repositoryPath(lock.runtime.binary));
  const tableBytes = readFileSync(repositoryPath(lock.runtime.table));
  demand(sha256(wasmBytes) === lock.sources[lock.runtime.binary] &&
    tableBytes.length === 67108864 && sha256(tableBytes) === TABLE, 'owned binary/table identity mismatch');
  const cases = [], totals = Array(24).fill(0);
  try {
    const { default:createHydroModule } = await import(pathToFileURL(repositoryPath(lock.runtime.module)).href);
    const module = await createHydroModule({ wasmBinary:wasmBytes });
    module.loadTable(new Uint8Array(tableBytes));
    for (let index = 0; index < REPLICATES; ++index) {
      const initial = initialArrays(masks, index);
      const state = new module.HydroState(JSON.stringify(checkpoint(initial.arrays, '0')));
      try {
        state.advance('4');
        const final = finalArrays(state.checkpoint(), initial.arrays);
        demand(final.count === initial.count, 'field token conservation failure');
        const sums = pairedSums(initial.arrays.bank, final.arrays.bank);
        sums.forEach((value, c) => { totals[c] += value; });
        cases.push({ index, microtick:'4', delta_sums:sums.map(String),
          array_sha256:Object.fromEntries(Object.entries(final.arrays).map(([name, raw]) => [name, sha256(raw)])),
          field_tokens_before:String(initial.count), field_tokens_after:String(final.count) });
      } finally { state.delete(); }
    }
  } finally {
    // Postchecks also run after a candidate exception; no successful result is published on drift.
    verifySources(lock.sources);
    demand(sha256(readFileSync(lockPath)) === lockHash && sha256(readFileSync(inputPath)) === inputHash, 'metadata drift');
    demand(sha256(readFileSync(maskPath)) === input.masks_sha256 &&
      sha256(readFileSync(entropyPath)) === input.entropy_sha256, 'input byte drift');
  }
  const result = { schema:'ftd-hydro-response-wasm-v1', registration_sha256:lockHash,
    input_sha256:inputHash, backend:{ platform:process.platform, arch:process.arch,
      node_version:process.version, runtime:'compiled-wasm-cpu',
      module_sha256:lock.sources[lock.runtime.module], binary_sha256:sha256(wasmBytes),
      table_sha256:TABLE, actual_advance_calls:REPLICATES, actual_microticks:REPLICATES * 4 },
    cases, total_delta_sums:totals.map(String), elapsed_seconds:(performance.now() - started) / 1000 };
  writeFileSync(outputPath, `${JSON.stringify(result)}\n`, { flag:'wx', encoding:'utf8' });
}

main().catch(error => {
  console.error(`PREDICTIVE_RESPONSE_FAILED: ${error instanceof Error ? error.stack : String(error)}`);
  process.exitCode = 1;
});
