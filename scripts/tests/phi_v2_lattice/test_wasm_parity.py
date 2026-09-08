"""Actual compiled WASM parity, distinct from fake-adapter protocol tests.

Set FTD_STRICT_WASM_MODULE to the freshly built ftd_strict_wasm.mjs. Without
that explicit artifact the suite skips; a skip is never a backend certificate.
"""
import json
import os
from dataclasses import asdict
from pathlib import Path
import subprocess

import pytest

from phi_v2_lattice import channels as C, coarse as B, native_codec as N, prepare as P
from phi_v2_lattice import staged as T, state as S


@pytest.fixture(scope="module")
def wasm_module():
    configured = os.environ.get("FTD_STRICT_WASM_MODULE")
    if not configured:
        pytest.skip("actual WASM module not requested")
    path = Path(configured).resolve()
    assert path.is_file(), path
    return path


NODE_DRIVER = r"""
import fs from 'node:fs';
import vm from 'node:vm';
import {pathToFileURL} from 'node:url';
const [moduleFile, input, output] = process.argv.slice(2);
const createModule = (await import(pathToFileURL(moduleFile))).default;
const module = await createModule();
const initial = new Uint8Array(fs.readFileSync(input));
const state = new module.StrictState(initial);
const rows=[];
try {
  for(let i=0;i<8;i++) {
    const step=JSON.parse(state.advance('1'));
    const checkpoint=state.checkpoint();
    rows.push({step, snapshot:Buffer.from(checkpoint).toString('base64'),
      observation:JSON.parse(state.observe('1','counts'))});
    const roundtrip=new module.StrictState(checkpoint);
    try {
      if(!Buffer.from(roundtrip.checkpoint()).equals(Buffer.from(checkpoint)))
        throw new Error('WASM checkpoint roundtrip mismatch');
    } finally { roundtrip.delete(); }
  }
  const before=state.checkpoint();
  let rejected=0;
  for(const invalid of ['-1','01','18446744073709551616']) {
    try { state.advance(invalid); } catch { rejected++; }
  }
  const bad=new Uint8Array(before); bad[0]^=1;
  try { state.restore(bad); } catch { rejected++; }
  const shared=new Uint8Array(new SharedArrayBuffer(before.length));shared.set(before);
  try { const unsafe=new module.StrictState(shared);unsafe.delete(); } catch { rejected++; }
  try { state.restore(shared); } catch { rejected++; }
  const foreign=new Uint8Array(vm.runInNewContext(`new SharedArrayBuffer(${before.length})`));
  foreign.set(before);
  try { const unsafe=new module.StrictState(foreign);unsafe.delete(); } catch { rejected++; }
  try { state.restore(foreign); } catch { rejected++; }
  Object.defineProperty(foreign,'buffer',{value:new ArrayBuffer(before.length)});
  try { const unsafe=new module.StrictState(foreign);unsafe.delete(); } catch { rejected++; }
  try { state.restore(foreign); } catch { rejected++; }
  if(rejected!==10) throw new Error(`WASM rejected ${rejected}/10 invalid or shared-buffer operations`);
  if(!Buffer.from(before).equals(Buffer.from(state.checkpoint())))
    throw new Error('WASM rejected operation changed authoritative state');
  fs.writeFileSync(output,JSON.stringify(rows));
} finally { state.delete(); }
"""


@pytest.mark.parametrize("size,kind", [(3, "isolated"), (4, "sparse"), (4, "vacuum"), (7, "sparse")])
def test_complete_wasm_state_events_and_observations_match_python(wasm_module, tmp_path, size, kind):
    if kind == "isolated":
        seed = P.isolated_relation(size)
    elif kind == "vacuum":
        seed = P.r5_vacuum(size, seed=11)
    else:
        seed = P.sparse_material(size, seed=5, n_tokens=12, field_occupation=.02)
    state = T.initialize(seed)
    input_path, output_path = tmp_path / "input.bin", tmp_path / "output.json"
    driver = tmp_path / "parity.mjs"
    driver.write_text(NODE_DRIVER, encoding="utf-8")
    input_path.write_bytes(N.encode(state))
    completed = subprocess.run(["node", str(driver), str(wasm_module), str(input_path), str(output_path)],
                               capture_output=True, text=True, timeout=90)
    assert completed.returncode == 0, completed.stdout + completed.stderr
    rows = json.loads(output_path.read_text(encoding="utf-8"))
    import base64
    for row in rows:
        state, events = T.step(state)
        assert base64.b64decode(row["snapshot"]) == N.encode(state)
        assert row["step"]["events"] == [json.loads(json.dumps(asdict(events)))]
        assert row["step"]["diagnostics"]["microtick"] == str(state.microtick)
        observed = row["observation"]
        expected = asdict(B.restrict(state.lattice, 1))

        def exact_strings(value):
            if isinstance(value, (list, tuple)):
                return [exact_strings(v) for v in value]
            return str(value)

        for key, value in expected.items():
            assert observed[key] == exact_strings(value), key
        assert observed["microtick"] == str(state.microtick)
        assert observed["phase"] == str(state.phase)
        assert observed["law_id"] == T.LAW_ID
        assert observed["collision_hash"] == C.COLLISION_HASH
        assert observed["tick_start"] == observed["tick_end"] == str(state.microtick)
        assert observed["status"] == "exact_observation"
        assert observed["spatial_support"] == "aligned_periodic_blocks"
        assert observed["length_unit"] == "microscopic_node"
        assert observed["time_unit"] == "staged_microtick"
        assert observed["observer_kind"] == "external_diagnostic"
        assert observed["physical_calibration"] == "unidentified"
