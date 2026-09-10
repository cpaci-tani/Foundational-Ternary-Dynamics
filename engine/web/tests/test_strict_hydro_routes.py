"""Small HTTP and path-containment regressions for the local strict-fluid link."""
from functools import partial
import http.client
import importlib.util
import json
from pathlib import Path
import threading

import pytest


SPEC = importlib.util.spec_from_file_location("ftd_web_serve_routes", Path(__file__).parents[1] / "serve.py")
serve = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(serve)


@pytest.fixture
def local_server(tmp_path, monkeypatch):
    engine = tmp_path / "engine"
    web = engine / "web"
    web.mkdir(parents=True)
    (web / "index.html").write_text("dashboard", encoding="utf-8")
    lab = engine / "strict" / "web" / "hydro"
    lab.mkdir(parents=True)
    (lab / "index.html").write_text("fluid laboratory", encoding="utf-8")
    (lab / "hydro-lab.js").write_text("export const local = true;", encoding="utf-8")
    for name in ("js/strict/hydro-worker.js", "js/strict/fluid-observables.js",
                 "js/ui/components/validity-status.js", "css/ui/components/validity-status.css"):
        file = web / name
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_text(name, encoding="utf-8")
    for name in serve.STRICT_HYDRO_ARTIFACTS:
        file = engine / name
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_bytes(b"test-runtime")
    (engine / "private.txt").write_text("never public", encoding="utf-8")
    monkeypatch.setattr(serve, "_ENGINE_ROOT", str(engine))
    monkeypatch.setattr(serve, "_WEB_ROOT", str(web))
    monkeypatch.setattr(serve, "QUIET", True)
    server = serve.http.server.ThreadingHTTPServer(
        ("127.0.0.1", 0), partial(serve.NoCacheHandler, directory=str(web)))
    thread = threading.Thread(target=server.serve_forever, kwargs={"poll_interval": 0.01}, daemon=True)
    thread.start()

    def request(path, method="GET"):
        connection = http.client.HTTPConnection("127.0.0.1", server.server_port, timeout=2)
        try:
            connection.request(method, path)
            response = connection.getresponse()
            return response.status, {key.lower(): value for key, value in response.getheaders()}, response.read()
        finally:
            connection.close()

    yield engine, web, request
    server.shutdown()
    server.server_close()
    thread.join(timeout=2)
    assert not thread.is_alive()


def test_local_status_requires_all_three_runtime_artifacts(local_server):
    engine, _, request = local_server
    status, _, body = request("/api/strict-hydro/status")
    assert status == 200
    assert json.loads(body) == {"available": True, "url": "/strict/web/hydro/"}
    (engine / serve.STRICT_HYDRO_ARTIFACTS[2]).unlink()
    assert json.loads(request("/api/strict-hydro/status")[2]) == {"available": False, "url": None}


@pytest.mark.parametrize("path", ("/", "/strict/web/hydro/", "/strict/web/hydro/hydro-lab.js",
    "/web/js/strict/hydro-worker.js", "/web/js/strict/fluid-observables.js",
    "/web/js/ui/components/validity-status.js", "/web/css/ui/components/validity-status.css",
    *("/"+name for name in serve.STRICT_HYDRO_ARTIFACTS)))
def test_allowlisted_runtime_and_public_dependencies_work(local_server, path):
    _, _, request = local_server
    status, headers, body = request(path+"?revision=test")
    assert status == 200 and body
    assert headers["cross-origin-opener-policy"] == "same-origin"
    assert "no-store" in headers["cache-control"]
    if path.endswith(".wasm"):
        assert headers["content-type"] == "application/wasm"
    if path.endswith(".mjs"):
        assert "javascript" in headers["content-type"]
    assert request(path, "HEAD")[0] == 200


@pytest.mark.parametrize("path", (
    "/strict/web/hydro/../private.txt", "/strict/web/hydro/%2e%2e/%2e%2e/private.txt",
    "/strict/web/hydro/%5c..%5cprivate.txt", "/web/../private.txt",
    "/web/%2e%2e/private.txt", "/web/C:/private.txt", "/web/%00private.txt",
    "/build_strict_hydro_wasm/", "/build_strict_hydro_tables/",
    "/build_strict_hydro_wasm/private.txt", "/build_strict_hydro/lab/manifest.json",
    "/strict/private.txt", "/private.txt", "/web/%252e%252e/private.txt",
))
def test_traversal_and_unlisted_build_resources_not_served(local_server, path):
    assert local_server[2](path)[0] == 404


def test_lab_directory_redirect_retains_usable_relative_urls(local_server):
    status, headers, _ = local_server[2]("/strict/web/hydro")
    assert status == 301
    assert headers["location"] == "/strict/web/hydro/"


def test_containment_rejects_relative_escape_without_http(local_server):
    _, web, _ = local_server
    with pytest.raises(ValueError, match="escapes"):
        serve._contained(web, "../private.txt")


def test_existing_symlink_is_checked_after_resolution(local_server):
    engine, web, request = local_server
    link = web / "outside.txt"
    try:
        link.symlink_to(engine / "private.txt")
    except OSError:
        pytest.skip("host does not permit creation of symlink test fixture")
    assert request("/outside.txt")[0] == 404
