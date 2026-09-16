"""Admission and complete inventory for the main dashboard's finite owner."""
from pathlib import Path
import json
import os
import subprocess

import pytest

from phi_v2_lattice import web_scenarios as L, native_codec as N, staged as P
from phi_v2_lattice import recovery_carriers as R, recovery_mixed_response as M, recovery_mixed_scattering as X


def test_catalog_covers_every_registered_carrier_and_mixed_case():
    rows = L.scenarios()
    assert len(rows) == 629
    ids = {row.id for row in rows}
    assert len(ids) == len(rows)
    for module in (R, M, X):
        assert {case.case_id for case in module.cases()} <= ids
    assert {row.law_id for row in rows} == {P.LAW_ID}
    assert L.catalog()['canonical_adoption'] is False


def test_inventory_is_complete_and_never_claims_execution():
    rows = L.test_inventory()
    root = L.ROOT / 'scripts/tests/phi_v2_lattice'
    assert {row['module'] for row in rows} == {p.relative_to(root).as_posix() for p in root.rglob('test_*.py')}
    assert all(row['execution_status'] == 'not_run_by_catalog' for row in rows)
    assert any(row['module'] == 'thermal/test_finite_wavelength.py' and not row['scenario_ids'] for row in rows)
    assert all((root / name).is_file() for row in L.scenarios() for name in row.tests)
    assert all((L.ROOT / row['path']).is_file() for row in L.catalog()['backend_checks'])


@pytest.mark.parametrize('row', L.scenarios(), ids=lambda row: row.id)
def test_registered_preparation_has_exact_complete_native_roundtrip(row):
    for size in row.sizes:
        data, digest = L.checkpoint(row.id, size)
        state = N.decode(data)
        assert state.lattice.L == size and state.microtick == 0
        assert N.encode(state) == data
        assert len(digest) == 64


@pytest.mark.parametrize('name,size', [('missing', 3), ('relation', 17), ('sparse', 0), ('empty', True),
                                       ('../../staged.py', 3), ('hydro-shear-wave-t2', 16)])
def test_other_laws_paths_and_unregistered_sizes_rejected(name, size):
    with pytest.raises(ValueError):
        L.prepare(name, size)


def test_every_registered_preparation_loads_into_actual_wasm(tmp_path):
    configured = os.environ.get('FTD_STRICT_WASM_MODULE')
    if not configured:
        pytest.skip('Actual compiled WASM artifact must be explicitly selected')
    module = Path(configured).resolve()
    assert module.is_file()
    manifest = []
    # All 656 registered id/size combinations enter the actual compiled state.
    # A full four-phase cycle is checked against Python for each preparation
    # family, plus seam, expiry and causal-witness adversaries.
    cycle_ids = {'empty', 'relation', 'sparse', 'r5', 'seam', 'witness', 'witness-control', 'expiry-a', 'expiry-b',
                 R.cases()[0].case_id, M.cases()[0].case_id, X.cases()[0].case_id}
    for row in L.scenarios():
        for size in row.sizes:
            data, digest = L.checkpoint(row.id, size)
            path = tmp_path / f'{len(manifest)}.bin'
            path.write_bytes(data)
            expected = None
            if row.id in cycle_ids:
                state = N.decode(data)
                for _ in range(4):
                    state, _ = P.step(state)
                from hashlib import sha256
                expected = sha256(N.encode(state)).hexdigest()
            manifest.append({'file': str(path), 'sha': digest, 'cycle_sha': expected})
    (tmp_path / 'cases.json').write_text(json.dumps(manifest), encoding='utf-8')
    driver = tmp_path / 'verify.mjs'
    driver.write_text("""
import fs from 'node:fs';
import {createHash} from 'node:crypto';
import {pathToFileURL} from 'node:url';
const module = await (await import(pathToFileURL(process.argv[2]))).default();
const rows = JSON.parse(fs.readFileSync(process.argv[3]));
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
for (const row of rows) {
    const state = new module.StrictState(new Uint8Array(fs.readFileSync(row.file)));
    try {
        if (hash(state.checkpoint()) !== row.sha) throw Error('load/roundtrip mismatch ' + row.file);
        if (row.cycle_sha) {
            state.advance('4');
            if (hash(state.checkpoint()) !== row.cycle_sha) throw Error('four-phase parity mismatch ' + row.file);
        }
    } finally { state.delete(); }
}
console.log(JSON.stringify({loaded: rows.length, cycles: rows.filter(r => r.cycle_sha).length}));
""", encoding='utf-8')
    result = subprocess.run(['node', str(driver), str(module), str(tmp_path/'cases.json')],
                            capture_output=True, text=True, timeout=180)
    assert result.returncode == 0, result.stdout + result.stderr
    report = json.loads(result.stdout)
    assert report == {'loaded': 656, 'cycles': 39}
