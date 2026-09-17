"""Only registered finite states and the two compiled artifacts are public."""
import json
from hashlib import sha256
from pathlib import Path

import pytest

from test_strict_hydro_routes import local_server, serve


@pytest.fixture
def record_server(local_server, monkeypatch):
    engine, web, request = local_server
    monkeypatch.syspath_prepend(str(Path(__file__).resolve().parents[3] / 'scripts'))
    from phi_v2_lattice import web_scenarios
    monkeypatch.setattr(serve, '_record_catalog_module', lambda: web_scenarios)
    for name in serve.RECORD_ARTIFACTS:
        file = engine / name
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_bytes(b'test artifact')
    return engine, request


def test_catalog_and_checkpoint_have_explicit_law_and_digest(record_server):
    _, request = record_server
    status, _, body = request('/api/lattice/records/catalog')
    assert status == 200
    catalog = json.loads(body)
    assert len(catalog['scenarios']) == 629 and not catalog['canonical_adoption']
    status, headers, body = request('/api/lattice/records/checkpoint?scenario=relation&size=3')
    assert status == 200 and body.startswith(b'FTDSC01\0')
    assert headers['x-checkpoint-sha256'] == sha256(body).hexdigest()
    assert headers['x-phi-law'] == catalog['law_id']
    assert len(catalog['seeding']['channels']) == 384
    assert {r['id'] for r in catalog['seeding']['channels']} == set(range(384))


def test_custom_seed_endpoint_validates_and_returns_complete_checkpoint(record_server):
    _, request = record_server
    recipe = dict(version=1, scenarioId='record-relation', size=3, blank=False, randomSeed=0, components=[])
    status, headers, data = request('/api/lattice/records/seed', 'POST', json.dumps(recipe), {'Content-Type': 'application/json'})
    assert status == 200 and data == request('/api/lattice/records/checkpoint?scenario=relation&size=3')[2]
    assert headers['x-checkpoint-sha256'] == sha256(data).hexdigest()
    assert len(headers['x-recipe-sha256']) == 64
    recipe['components'] = [{'code': 'not a component'}]
    assert request('/api/lattice/records/seed', 'POST', json.dumps(recipe), {'Content-Type': 'application/json'})[0] == 400


def test_custom_seed_endpoint_rejects_unavailable_and_invalid_inputs(record_server):
    engine, request = record_server
    route = '/api/lattice/records/seed'
    assert request(route, 'POST', '{}', {'Content-Type': 'text/plain'})[0] == 415
    assert request(route, 'POST', '{}', {'Content-Type': 'application/json', 'Origin': 'https://unrelated.example'})[0] == 403
    assert request(route, 'POST', 'x' * 262145, {'Content-Type': 'application/json'})[0] == 413
    assert request(route, 'POST', 'broken', {'Content-Type': 'application/json'})[0] == 400
    (engine / serve.RECORD_ARTIFACTS[1]).unlink()
    assert request(route, 'POST', '{}', {'Content-Type': 'application/json'})[0] == 503


@pytest.mark.parametrize('query', ['scenario=bad&size=3', 'scenario=relation&size=17',
    'scenario=relation&size=03', 'scenario=relation&size=3&size=4', 'scenario=../secrets&size=3',
    'scenario=relation', 'scenario=relation&size=3&code=anything'])
def test_invalid_or_ambiguous_preparations_rejected(record_server, query):
    assert record_server[1]('/api/lattice/records/checkpoint?' + query)[0] == 400


def test_missing_runtime_fails_closed_and_build_tree_stays_private(record_server):
    engine, request = record_server
    for name in serve.RECORD_ARTIFACTS:
        assert request('/' + name)[0] == 200
    for path in ['/build_strict_wasm/', '/build_strict_wasm/CMakeCache.txt', '/build_strict/lab/manifest.json']:
        assert request(path)[0] == 404
    (engine / serve.RECORD_ARTIFACTS[1]).unlink()
    assert request('/api/lattice/records/catalog')[0] == 503
    assert request('/api/lattice/records/checkpoint?scenario=relation&size=3')[0] == 503


def test_removed_workflows_are_not_routes(record_server):
    _, request = record_server
    for path in ['/api/phi-v2/catalog', '/strict/web/', '/strict/web/laboratory.js']:
        assert request(path)[0] == 404
