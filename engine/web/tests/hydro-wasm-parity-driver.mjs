// Node driver for actual compiled hydro-WASM parity (mirrors the inline NODE_DRIVER in
// scripts/tests/phi_v2_lattice/test_wasm_parity.py for the strict candidate, but kept as
// a standalone file per task-10-brief.md). Not itself a test: scripts/tests/
// phi_v2_lattice/hydro/test_wasm_parity.py invokes it as a subprocess and checks the
// emitted rows against the Python oracle.
//
// Usage: node hydro-wasm-parity-driver.mjs MODULE TABLE INPUT_CHECKPOINT OUTPUT MICROTICKS
//   MODULE            path to the built ftd_hydro_wasm.mjs
//   TABLE             path to the SHA-256-verified 64 MiB collision-table blob
//   INPUT_CHECKPOINT  path to a UTF-8 "ftd-hydro-checkpoint-1" JSON file (scripts/
//                     phi_v2_lattice/hydro/codec.py's checkpoint())
//   OUTPUT            path to write the JSON array of per-microtick rows
//   MICROTICKS        how many single-microtick advances to run
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
const microticks = Number(microticksArg);
if (!Number.isInteger(microticks) || microticks < 0) throw new Error('MICROTICKS must be a nonnegative integer');

const createModule = (await import(pathToFileURL(moduleFile).href)).default;
const module = await createModule();

const tableBytes = new Uint8Array(fs.readFileSync(tableFile));
module.loadTable(tableBytes);
// A second load must be idempotent/accepted (re-verifies the same hash); exercised once
// here so the parity test does not need a dedicated fixture for it.
module.loadTable(tableBytes);

const initialCheckpoint = fs.readFileSync(inputFile, 'utf8');
const state = new module.HydroState(initialCheckpoint);

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
