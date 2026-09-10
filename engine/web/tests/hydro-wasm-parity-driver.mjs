// Node driver for actual compiled hydro-WASM parity (mirrors the inline NODE_DRIVER in
// scripts/tests/phi_v2_lattice/test_wasm_parity.py for the strict candidate, but kept as
// a standalone file per task-10-brief.md). Not itself a test: scripts/tests/
// phi_v2_lattice/hydro/test_wasm_parity.py invokes it as a subprocess and checks the
// emitted rows against the Python oracle.
//
// Usage: node hydro-wasm-parity-driver.mjs MODULE TABLE INPUT_CHECKPOINT OUTPUT MICROTICKS
//   MODULE            path to the built ftd_hydro_wasm.mjs
//   TABLE             path to the SHA-256-verified 64 MiB collision-table blob
//   INPUT_CHECKPOINT  path to a UTF-8 "ftd-hydro-checkpoint-2" JSON file (scripts/
//                     phi_v2_lattice/hydro/codec.py's checkpoint())
//   OUTPUT            path to write the JSON array of per-microtick rows
//   MICROTICKS        how many single-microtick advances to run, --boundary, or --lexical-alias
//
// Each row: {advance (parsed advance() result: diagnostics+events), checkpoint (JSON
// text after that microtick), counts/fields/moments (parsed observe() results; fields
// is null when L is odd, since the brief's fields check is width=2)}.
import fs from 'node:fs';
import { pathToFileURL } from 'node:url';

const [moduleFile, tableFile, inputFile, outputFile, microticksArg] = process.argv.slice(2);
if (!moduleFile || !tableFile || !inputFile || !outputFile || !microticksArg) {
    console.error('usage: node hydro-wasm-parity-driver.mjs MODULE TABLE INPUT OUTPUT MICROTICKS');
    process.exit(2);
}
const boundaryMode = microticksArg === '--boundary';
const lexicalAliasMode = microticksArg === '--lexical-alias';
const microticks = boundaryMode || lexicalAliasMode ? 0 : Number(microticksArg);
if (!Number.isSafeInteger(microticks) || microticks < 0) throw new Error('MICROTICKS must be a nonnegative safe integer');

const createModule = (await import(pathToFileURL(moduleFile).href)).default;
const module = await createModule();

const tableBytes = new Uint8Array(fs.readFileSync(tableFile));
module.loadTable(tableBytes);
// A second load must be idempotent/accepted (re-verifies the same hash); exercised once
// here so the parity test does not need a dedicated fixture for it.
module.loadTable(tableBytes);

const initialCheckpoint = fs.readFileSync(inputFile, 'utf8');
const state = new module.HydroState(initialCheckpoint);

if (lexicalAliasMode) {
    const pristine = state.checkpoint();
    const rejected = [];
    const rejectCheckpoint = (name, text) => {
        for (const [action, run] of [
            ['constructor', () => { const value = new module.HydroState(text); value.delete(); }],
            ['restore', () => state.restore(text)],
        ]) {
            let didReject = false;
            try { run(); } catch { didReject = true; }
            if (!didReject) throw new Error(`accepted raw noninteger token: ${name}:${action}`);
            if (state.checkpoint() !== pristine) throw new Error(`failure mutated state: ${name}:${action}`);
            rejected.push(`${name}:${action}`);
        }
    };
    for (const token of ['4.0000000000000001', '4.0', '4e0', '4E+0', '-0', '-0.0', '4.000e0']) {
        rejectCheckpoint(`L:${token}`, pristine.replace(/"L":\d+/, `"L":${token}`));
    }
    const legacy = JSON.parse(pristine);
    legacy.schema = 'ftd-hydro-checkpoint-1'; legacy.microtick = 1;
    const legacyText = JSON.stringify(legacy);
    for (const token of ['1.0000000000000001', '1.0', '1e0', '1E+0', '-0', '-0.0', '0e0',
        '9007199254740991.0000000000000001']) {
        rejectCheckpoint(`tick:${token}`, legacyText.replace('"microtick":1', `"microtick":${token}`));
    }
    const legacyState = new module.HydroState(legacyText);
    const legacyDiagnostics = JSON.parse(legacyState.diagnostics());
    legacyState.delete();
    // A fixed three-token fixture gives nonzero phases without any transition.
    const fixture = JSON.parse(pristine);
    const bank = Buffer.from(fixture.arrays.bank, 'base64');
    bank.fill(0);
    const L = fixture.L;
    for (const [x, y, z, channel] of [[1, 2, 3, 0], [3, 1, 2, 18], [2, 3, 1, 116]]) {
        bank[((x * L + y) * L + z) * 192 + channel] = 1;
    }
    fixture.arrays.bank = bank.toString('base64');
    const observed = new module.HydroState(JSON.stringify(fixture));
    const observedCheckpoint = observed.checkpoint();
    const bound = 2n ** 63n;
    const requested = [
        [1n, -1n, 2n], [5n, -5n, -2n], [2n ** 53n + 1n, 2n ** 53n + 2n, -(2n ** 53n) + 3n],
        [bound - 1n, -bound, bound - 2n], [-bound + 1n, bound - 5n, -bound + 2n],
    ];
    const aliases = [];
    for (const k of requested) {
        const reduced = k.map(v => { let r = ((v % BigInt(L)) + BigInt(L)) % BigInt(L);
            if (r > BigInt(Math.floor(L / 2))) r -= BigInt(L); return r; });
        for (const pol of [0, 1]) {
            const raw = JSON.parse(observed.observe('1', `moments:${k.join(',')},${pol}`));
            const canonical = JSON.parse(observed.observe('1', `moments:${reduced.join(',')},${pol}`));
            if (JSON.stringify(raw.moments) !== JSON.stringify(canonical.moments))
                throw new Error(`periodic alias changed moments: ${k.join(',')}`);
            if (raw.k.join(',') !== k.join(',')) throw new Error('requested k metadata was replaced');
            if (observed.checkpoint() !== observedCheckpoint) throw new Error('moments mutated checkpoint');
            aliases.push({requested: k.map(String), reduced: reduced.map(String), pol, raw, canonical});
        }
    }
    observed.delete(); state.delete();
    fs.writeFileSync(outputFile, JSON.stringify({rejected, aliases, legacyDiagnostics,
        fixture: observedCheckpoint, actualAdvanceCalls: 0}));
    process.exit(0);
}

if (boundaryMode) {
    const pristine = state.checkpoint();
    const source = JSON.parse(pristine);
    const rejected = [];
    const expectReject = (name, action) => {
        let didReject = false;
        try { action(); } catch { didReject = true; }
        if (!didReject) throw new Error(`accepted invalid boundary: ${name}`);
        if (state.checkpoint() !== pristine) throw new Error(`failure mutated owned state: ${name}`);
        rejected.push(name);
    };
    const rejectCheckpoint = (name, text) => {
        expectReject(`${name}:constructor`, () => { const value = new module.HydroState(text); value.delete(); });
        expectReject(`${name}:restore`, () => state.restore(text));
    };
    const mutate = (edit) => { const doc = structuredClone(source); edit(doc); return JSON.stringify(doc); };
    for (const name of ['schema', 'law', 'table', 'encoding', 'boundary']) {
        rejectCheckpoint(`identity:${name}`, mutate(doc => { doc[name] = 'incompatible'; }));
        rejectCheckpoint(`identity-type:${name}`, mutate(doc => { doc[name] = {}; }));
    }
    for (const [name, value] of Object.entries({fractional: 3.5, negative: -3, small: 2, zero: 0,
        uint32_overflow: 4294967296, count_overflow: 4294967295, missing_array_capacity: 200,
        string: '4', boolean: true, null: null, object: {}, array: []})) {
        rejectCheckpoint(`L:${name}`, mutate(doc => { doc.L = value; }));
    }
    for (const value of ['NaN', 'Infinity', '-Infinity']) {
        rejectCheckpoint(`nonfinite-json:${value}`, pristine.replace(/"L":\d+/, `"L":${value}`));
    }
    for (const [name, value] of Object.entries({fractional: 0.5, negative: -1,
        unsafe: 9007199254740992, string: '1', boolean: true, null: null})) {
        rejectCheckpoint(`v1-tick:${name}`, mutate(doc => { doc.schema = 'ftd-hydro-checkpoint-1'; doc.microtick = value; }));
    }
    const v1 = mutate(doc => { doc.schema = 'ftd-hydro-checkpoint-1'; doc.microtick = 0; });
    rejectCheckpoint('v1-tick:raw-rounded-odd', v1.replace('"microtick":0', '"microtick":9007199254740993'));
    rejectCheckpoint('v1-tick:negative-zero', v1.replace('"microtick":0', '"microtick":-0'));
    for (const value of ['', '-1', '-0', '01', '+1', '1.5', '1e1', ' 1', 'NaN', '18446744073709551616', 1, true, null]) {
        rejectCheckpoint(`v2-tick:${JSON.stringify(value)}`, mutate(doc => { doc.microtick = value; }));
    }
    rejectCheckpoint('shape:extra', mutate(doc => { doc.extra = 1; }));
    rejectCheckpoint('shape:missing', mutate(doc => { delete doc.encoding; }));
    rejectCheckpoint('shape:arrays-null', mutate(doc => { doc.arrays = null; }));
    rejectCheckpoint('shape:arrays-list', mutate(doc => { doc.arrays = []; }));
    rejectCheckpoint('shape:arrays-extra', mutate(doc => { doc.arrays.extra = ''; }));
    rejectCheckpoint('shape:arrays-missing', mutate(doc => { delete doc.arrays.s; }));
    rejectCheckpoint('duplicate:encoding', pristine.replace('{', '{"encoding":"wrong",'));
    rejectCheckpoint('duplicate:escaped-encoding', pristine.replace('{', '{"encod\\u0069ng":"wrong",'));
    rejectCheckpoint('duplicate:array', pristine.replace('"arrays":{', '"arrays":{"s":"",'));
    for (const name of Object.keys(source.arrays)) {
        rejectCheckpoint(`array-type:${name}`, mutate(doc => { doc.arrays[name] = []; }));
        rejectCheckpoint(`array-size:${name}`, mutate(doc => { doc.arrays[name] = doc.arrays[name].slice(4); }));
        rejectCheckpoint(`array-domain:${name}`, mutate(doc => {
            const bytes = Buffer.from(doc.arrays[name], 'base64');
            bytes[0] = name === 'sc' || name === 'fcc' ? 9 : 2;
            doc.arrays[name] = bytes.toString('base64');
        }));
    }
    for (const [name, encoded] of Object.entries({
        newline: source.arrays.s.slice(0, -1) + '\n',
        misplaced_padding: '=' + source.arrays.s.slice(1),
        missing_padding: source.arrays.s.replace(/=/g, ''),
        trailing_padding: source.arrays.s + '=',
        nonzero_pad_bits: source.arrays.s.slice(0, -3) + 'B==',
        invalid_alphabet: '*' + source.arrays.s.slice(1),
    })) rejectCheckpoint(`base64:${name}`, mutate(doc => { doc.arrays.s = encoded; }));
    for (const value of [1, 0.5, NaN, Infinity, true, null, {}, [], '01', '-1', '18446744073709551616']) {
        expectReject(`advance:${String(value)}`, () => state.advance(value));
    }
    for (const value of [1, true, null, {}]) {
        expectReject(`width-type:${String(value)}`, () => state.observe(value, 'counts'));
        expectReject(`observable-type:${String(value)}`, () => state.observe('1', value));
        rejectCheckpoint(`checkpoint-type:${String(value)}`, value);
    }
    for (const spec of ['9223372036854775808,0,0,0', '-9223372036854775809,0,0,0',
        '-0,0,0,0', '01,0,0,0', '1e0,0,0,0', '1,0,0,2', '1,0,0', '1,0,0,0,0']) {
        expectReject(`moments:${spec}`, () => state.observe('1', `moments:${spec}`));
    }
    const accepted = [];
    for (const [schema, tick] of [
        ['ftd-hydro-checkpoint-1', 0], ['ftd-hydro-checkpoint-1', 9007199254740991],
        ...['0', '9007199254740992', '9007199254740993', '9007199254740994',
            '18446744073709551614', '18446744073709551615'].map(t => ['ftd-hydro-checkpoint-2', t]),
    ]) {
        const instance = new module.HydroState(mutate(doc => { doc.schema = schema; doc.microtick = tick; }));
        const before = instance.checkpoint();
        const diagnostics = JSON.parse(instance.diagnostics());
        if (BigInt(tick) === 18446744073709551615n) {
            let overflowRejected = false;
            try { instance.advance('1'); } catch { overflowRejected = true; }
            if (!overflowRejected || instance.checkpoint() !== before) throw new Error('uint64 overflow failed rollback');
        }
        const advance = JSON.parse(instance.advance(BigInt(tick) === 18446744073709551615n ? '0' : '1'));
        accepted.push({schema, tick: String(tick), before, diagnostics, advance, after: instance.checkpoint()});
        instance.delete();
    }
    const extremeMoments = JSON.parse(state.observe('1', 'moments:-9223372036854775808,9223372036854775807,0,1'));
    const counts = JSON.parse(state.observe('1', 'counts'));
    const fields = JSON.parse(state.observe('1', 'fields'));
    fs.writeFileSync(outputFile, JSON.stringify({rejected, accepted, extremeMoments, counts, fields}));
    state.delete();
    process.exit(0);
}

const rows = [];
for (let i = 0; i < microticks; i++) {
    const advance = JSON.parse(state.advance('1'));
    const checkpoint = state.checkpoint();
    const diagnostics = JSON.parse(state.diagnostics());
    const L = Number(diagnostics.L);
    const counts = JSON.parse(state.observe('1', 'counts'));
    const fields = L % 2 === 0 ? JSON.parse(state.observe('2', 'fields')) : null;
    const moments = JSON.parse(state.observe('1', 'moments:1,0,0,0'));
    rows.push({ advance, checkpoint, diagnostics, counts, fields, moments });

    // Checkpoint round trip: restoring what checkpoint() just produced must reproduce it
    // exactly, using a fresh HydroState (never the live one under test).
    const roundtrip = new module.HydroState(checkpoint);
    if (roundtrip.checkpoint() !== checkpoint) throw new Error('WASM checkpoint roundtrip mismatch');
    roundtrip.delete();
}

// Sanity: canonical-decimal, width, observable and schema/law/table rejections never
// silently succeed (3 decimal + width=0 + unknown observable + missing "arrays" = 6).
let rejected = 0;
for (const invalid of ['-1', '01', '18446744073709551616']) {
    try { state.advance(invalid); } catch { rejected++; }
}
try { state.observe('0', 'counts'); } catch { rejected++; }
try { state.observe('1', 'not-a-real-observable'); } catch { rejected++; }
try { new module.HydroState('{}'); } catch { rejected++; }
if (rejected !== 6) throw new Error(`WASM rejected ${rejected}/6 invalid operations`);

fs.writeFileSync(outputFile, JSON.stringify(rows));
state.delete();
