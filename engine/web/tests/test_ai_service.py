"""AI routes keep keys private, constrain upstream requests and guard local assets."""
import hashlib
import json
from pathlib import Path
import sys

import pytest
from test_strict_hydro_routes import local_server, serve

ai = serve.ai_service
BODY = {"intent": "Pause", "observation": {"workspace": "lattice"}, "plan": {"action": "pause"}}
HEADERS = {"Content-Type": "application/json", "Authorization": "Bearer TEST_ONLY_KEY"}


def test_status_and_proxy_are_loopback_same_origin_and_do_not_return_key(local_server, monkeypatch):
    request = local_server[2]
    monkeypatch.setenv("TYPESAFE_API_KEY", "ENV_TEST_ONLY_KEY")
    status, _, body = request('/api/ai/status')
    assert status == 200 and not json.loads(body)['jevConfigured']
    assert b'ENV_TEST_ONLY_KEY' not in body
    assert request('/api/ai/status', headers={'Host': 'attacker.example'})[0] == 403
    assert request('/api/ai/jev', 'POST', json.dumps(BODY), {**HEADERS, 'Origin': 'https://attacker.example'})[0] == 403
    received = []
    def upstream(value, key):
        received.append((value, key))
        return {'decision': 'clarify', 'confidence': .8, 'model': 'jev-test', 'usage': {}}
    monkeypatch.setattr(ai, '_upstream', upstream)
    assert request('/api/ai/jev', 'POST', json.dumps(BODY), {'Content-Type': 'application/json'})[0] == 401
    assert received == []
    assert request('/api/ai/jev', 'POST', json.dumps(BODY), HEADERS)[0] == 200
    assert received == [(BODY, 'TEST_ONLY_KEY')]


def test_rejects_bad_input_before_upstream(local_server, monkeypatch):
    request = local_server[2]
    monkeypatch.setattr(ai, '_upstream', lambda *args: pytest.fail('must not call upstream'))
    for body in ['broken', '[]', json.dumps({**BODY, 'questions': {}}), json.dumps({**BODY, 'plan': []})]:
        assert request('/api/ai/jev', 'POST', body, HEADERS)[0] == 400
    assert request('/api/ai/jev', 'POST', 'x' * 65537, HEADERS)[0] == 413
    assert request('/api/ai/jev', 'POST', '{}', {'Content-Type': 'text/plain'})[0] == 415


def test_assets_reject_unknown_paths_changed_bytes_and_serve_immutable(local_server, monkeypatch, tmp_path):
    root = tmp_path / 'models'; root.mkdir()
    data = b'locked artifact'; (root / 'known.bin').write_bytes(data)
    lock = tmp_path / 'model-manifest.json'
    lock.write_text(json.dumps({'variants': [{'files': [{'path': 'known.bin', 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}]}]}))
    monkeypatch.setattr(ai, 'MODEL_ROOT', root); monkeypatch.setattr(ai, 'MODEL_MANIFEST', lock)
    request = local_server[2]
    status, headers, body = request('/api/ai/assets/known.bin')
    assert status == 200 and body == data and 'immutable' in headers['cache-control']
    for path in ['../model-manifest.json', '%2e%2e/model-manifest.json', 'unknown.bin', 'file:secret', 'known.bin/']:
        assert request('/api/ai/assets/' + path)[0] == 404
    (root / 'known.bin').write_bytes(b'x' * len(data))
    assert request('/api/ai/assets/known.bin')[0] == 404


@pytest.mark.parametrize('answer', [None, {}, {'answers': None}, {'answers': {'decision': None}},
    {'answers': {'decision': {'type': 'choice', 'choice': 'execute', 'confidence': 2}}, 'model': 'bad'}])
def test_malformed_upstream_fails_closed(answer):
    with pytest.raises(ValueError): ai.normalize_answer(answer)


def test_model_lock_paths_remain_pinned_webllm_resolve_urls():
    manifest = json.loads(ai.MODEL_MANIFEST.read_text(encoding='utf-8'))
    assert manifest['runtimeVersion'] == '0.2.85'
    for variant in manifest['variants']:
        assert '/resolve/' + variant['sourceRevision'] + '/' in variant['modelPath']
        assert len(variant['sourceRevision']) == 40
        paths = {entry['path'] for entry in variant['files']}
        for filename in ['mlc-chat-config.json', 'ndarray-cache.json', 'tokenizer.json']:
            assert variant['modelPath'] + filename in paths
        assert variant['modelLibPath'] in paths
        for entry in variant['files']:
            assert len(entry['sha256']) == 64 and entry['size'] > 0
            assert '/main/' not in entry['url'] and '..' not in entry['path'].split('/')


def test_public_extractor_uses_git_revision_not_current_local_docs():
    sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'scripts/assistant'))
    import build_knowledge as builder
    sources = dict(builder.public_sources('HEAD'))
    assert 'docs/theory/07_assessment/core_ledgers/LEDGER.md' in sources
    assert not any(path.startswith(('docs/internal/', '.codex/', '.claude/')) for path in sources)
    assert not builder.allowed('engine/thirdparty/library/README.md')
    assert not builder.allowed('engine/build/private.md')
    chunk = next(builder.chunks('docs/theory/archive/OLD.md', b'# Old\n[RETRACTED] FTD-1234\n', 'a' * 40, True))
    assert chunk['historical'] and chunk['sourceHash'] == hashlib.sha256(b'# Old\n[RETRACTED] FTD-1234\n').hexdigest()
    assert chunk['sourceUrl'].endswith('/docs/theory/archive/OLD.md#L1')
    ledger = b'# Master Claim Ledger\n**Status:** Active\n\n**Canonical supersession notice:** FTD-0012 is [RETRACTED].\n\n### FTD-0001: Algebra\n**tag:** THEOREM\n'
    rows = list(builder.chunks('docs/theory/07_assessment/core_ledgers/LEDGER.md', ledger, 'a' * 40, True))
    assert not any(row['historical'] for row in rows)
    assert 'RETRACTED' not in rows[-1]['statusTags']


def test_local_source_pages_are_inert_line_addressed_manifest_assets(local_server, monkeypatch, tmp_path):
    sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'scripts/assistant'))
    import build_knowledge as builder
    source = b'# Citation\n<script>throw "must not execute"</script>\nFTD-1234 [OPEN]\n'
    root = tmp_path / 'knowledge'
    entry = builder.write_source_page(root, 'docs/local & private.md', source)
    chunk = next(builder.chunks(entry['sourcePath'], source, 'a' * 40, False))
    assert chunk['sourceUrl'] == '/api/ai/knowledge/' + entry['path'] + '#L1'
    assert chunk['sourceHash'] == entry['sourceHash'] == hashlib.sha256(source).hexdigest()
    (root / 'manifest.json').write_text(json.dumps({'files': [entry]}), encoding='utf-8')
    monkeypatch.setattr(ai, 'KNOWLEDGE_ROOT', root)
    request = local_server[2]
    status, headers, body = request(chunk['sourceUrl'].split('#')[0])
    assert status == 200 and headers['content-type'] == 'text/html; charset=utf-8'
    assert b'id="L2"' in body and b'&lt;script&gt;' in body and b'<script>' not in body
    assert b'docs/local &amp; private.md' in body and b"default-src 'none'" in body
    assert b'Line basis: source' in body and b'working-tree snapshot' in body
    assert 'immutable' in headers['cache-control']
    (root / 'secret.html').write_text('not indexed')
    for path in ['secret.html', '../../docs/local%20%26%20private.md', 'sources/%2e%2e/secret.html']:
        assert request('/api/ai/knowledge/' + path)[0] == 404
    target = root / entry['path']; target.write_bytes(b'x' * entry['size'])
    assert request(chunk['sourceUrl'].split('#')[0])[0] == 404


def test_local_source_page_uses_extracted_notebook_lines_and_build_manifest(monkeypatch, tmp_path):
    sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'scripts/assistant'))
    import build_knowledge as builder
    source = json.dumps({'cells': [{'cell_type': 'markdown', 'source': ['# Notebook\n', 'Evidence [OPEN]\n']},
                                 {'cell_type': 'code', 'source': ['private code'], 'outputs': ['private output']}]}).encode()
    root = tmp_path / 'knowledge'
    monkeypatch.setattr(builder, 'local_sources', lambda: iter([('docs/note.ipynb', source)]))
    monkeypatch.setattr(sys, 'argv', ['build_knowledge.py', '--mode', 'local', '--output', str(root)])
    builder.main()
    manifest = json.loads((root / 'manifest.json').read_text())
    entry = next(entry for entry in manifest['files'] if entry.get('kind') == 'source-document')
    page = (root / entry['path']).read_text(encoding='utf-8')
    assert 'Line basis: extracted' in page and 'id="L2"' in page
    assert 'private code' not in page and 'private output' not in page
    rows = json.loads((root / manifest['shards'][0]['chunksPath']).read_text(encoding='utf-8'))
    assert next(iter(rows.values()))['sourceUrl'] == '/api/ai/knowledge/' + entry['path'] + '#L1'
    public = next(builder.chunks('docs/note.ipynb', source, 'b' * 40, True))
    assert public['sourceUrl'] == 'https://github.com/cpaci-tani/Foundational-Ternary-Dynamics/blob/' + 'b' * 40 + '/docs/note.ipynb'
    public_root = tmp_path / 'public-knowledge'
    monkeypatch.setattr(builder, 'public_sources', lambda revision: iter([('docs/note.ipynb', source)]))
    monkeypatch.setattr(builder, 'local_sources', lambda: pytest.fail('Public build must not read local sources'))
    monkeypatch.setattr(sys, 'argv', ['build_knowledge.py', '--mode', 'public', '--output', str(public_root)])
    builder.main()
    public_manifest = json.loads((public_root / 'manifest.json').read_text())
    assert not any(entry.get('kind') == 'source-document' for entry in public_manifest['files'])
    assert not (public_root / 'sources').exists()
