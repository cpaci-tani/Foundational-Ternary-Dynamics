"""
Dev http server for engine/web/ that disables browser caching.

Why this exists: ES-modules served via `python -m http.server` are
cached aggressively by browsers. After every JS edit the cache had
to be defeated by a hard refresh, by URL-param cache-busting in code,
or by bouncing the server. All three are friction we don't need.

Usage (replaces `python -m http.server 8080 -d engine/web`):

    python engine/web/serve.py            # port 8080, bind 127.0.0.1
    python engine/web/serve.py 9090       # custom port
    python engine/web/serve.py --host 0.0.0.0 9090   # LAN bind (opt-in)

The handler adds `Cache-Control: no-store, must-revalidate` to every
response, plus `Pragma: no-cache` and `Expires: 0` for the older browsers.
The local strict-fluid laboratory has narrow static/runtime routes and an
availability endpoint; other assets retain the public engine/web root.

Threading + allow_reuse_address are enabled so the server survives
parent-process detachment (the common case under preview managers
on Windows) and binds cleanly after a hot-restart.

H-4 cleanup ticket; pairs with the punch list in
`C:\\Users\\cpaci\\.claude\\plans\\i-want-to-try-crispy-charm.md`.
"""

import http.server
import json
import os
import socket
import subprocess
import sys
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlsplit

# server_controls.py lives next to this file. `python engine/web/serve.py`
# already puts the script's own directory on sys.path[0], but a test (or
# any other loader) that execs this module via importlib from a different
# cwd does not — insert defensively so `import server_controls` resolves
# either way.
_SERVE_DIR = os.path.dirname(os.path.abspath(__file__))
if _SERVE_DIR not in sys.path:
    sys.path.insert(0, _SERVE_DIR)
import server_controls

# Idle auto-stop + "stop all engine servers" (spec 2026-09-16-idle-shutdown-
# and-kill-all.md, §2-5): one process-lifetime controller. Every new route
# handler, the activity clock, and the idle watchdog thread live in
# server_controls.py — see NoCacheHandler.do_GET/do_POST and main() below
# for its three-line hook.
CONTROLS = server_controls.ServerControls()


# ── GPU server (ws_server.exe) launcher paths ────────────────────────────────
# The splash screen's "GPU Acceleration" card talks to the three /api/gpu-server/
# routes below to show status, start the CUDA WebSocket server, and download the
# binary. This only works when the dashboard is served by THIS script locally;
# on GitHub Pages the routes are absent and the card degrades to a static note.
_WEB_ROOT = os.path.dirname(os.path.abspath(__file__))          # engine/web
_ENGINE_ROOT = os.path.dirname(_WEB_ROOT)                        # engine
WS_SERVER_EXE = os.path.join(_ENGINE_ROOT, "build", "Release", "ws_server.exe")
GPU_PORT = 9100
STRICT_HYDRO_URL = "/strict/web/hydro/"
STRICT_HYDRO_ARTIFACTS = (
    "build_strict_hydro_wasm/ftd_hydro_wasm.mjs",
    "build_strict_hydro_wasm/ftd_hydro_wasm.wasm",
    "build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32",
)
RECORD_ARTIFACTS = (
    "build_strict_wasm/ftd_strict_wasm.mjs",
    "build_strict_wasm/ftd_strict_wasm.wasm",
)


def _record_catalog_module():
    # The endpoint only admits registered finite preparations, never Python
    # supplied by the request. Tests are inventoried through AST, not executed.
    scripts = str(Path(_ENGINE_ROOT).parent / "scripts")
    if scripts not in sys.path:
        sys.path.insert(0, scripts)
    from phi_v2_lattice import web_scenarios
    return web_scenarios


def _contained(root, relative):
    """Resolve a static resource without permitting symlink/junction escapes."""
    root = Path(root).resolve()
    resource = (root / relative).resolve()
    if not resource.is_relative_to(root):
        raise ValueError("resource escapes its static root")
    return resource


def _static_resource(request_path, directory):
    """Public web tree plus narrow local strict-lab/runtime routes only."""
    route = unquote(urlsplit(request_path).path, errors="strict")
    if (not route.startswith("/") or route.startswith("//") or "\\" in route
            or "\x00" in route or ":" in route
            or any(part in (".", "..") for part in route.split("/"))):
        raise ValueError("invalid static resource path")
    if route == STRICT_HYDRO_URL.rstrip("/") or route.startswith(STRICT_HYDRO_URL):
        relative = route[len(STRICT_HYDRO_URL):] if route.startswith(STRICT_HYDRO_URL) else ""
        return _contained(Path(_ENGINE_ROOT) / "strict" / "web" / "hydro", relative)
    if route.startswith("/web/"):
        return _contained(_WEB_ROOT, route[len("/web/"):])
    if route.lstrip("/") in STRICT_HYDRO_ARTIFACTS + RECORD_ARTIFACTS:
        return _contained(_ENGINE_ROOT, route.lstrip("/"))
    # There is deliberately no mount of engine/ or any build directory.
    if route.startswith(("/build_strict_", "/strict/")):
        raise ValueError("strict resource is not allowlisted")
    return _contained(directory, route.lstrip("/"))


def _gpu_running(host="127.0.0.1", port=GPU_PORT, timeout=0.25):
    """True if something is accepting connections on the GPU server port."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


# When True (via the --cache CLI flag), the no-store headers are omitted so
# assets cache normally. The cross-origin-isolation headers are ALWAYS sent.
# Used by the Playwright worker tests: they need SharedArrayBuffer (COOP/COEP)
# but the per-test fresh page loads time out if the large wasm64 binary is
# re-fetched every time (no-cache). The dev preview keeps no-cache (default).
ALLOW_CACHE = False
QUIET = False


class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {**http.server.SimpleHTTPRequestHandler.extensions_map,
                      ".mjs": "text/javascript", ".wasm": "application/wasm"}

    def translate_path(self, path):
        return str(_static_resource(path, self.directory))

    def send_head(self):
        try:
            self.translate_path(self.path)
        except (ValueError, UnicodeError, OSError):
            self.send_error(404, "Not found")
            return None
        return super().send_head()

    def handle(self):
        """Suppress normal browser disconnects without hiding server faults."""
        try:
            super().handle()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            # Browsers routinely cancel speculative/static/API fetches when a
            # test page or tab closes. The response is no longer deliverable;
            # logging a full request-thread traceback is only noise.
            pass

    def log_message(self, format, *args):
        if not QUIET:
            super().log_message(format, *args)

    def end_headers(self):
        if not ALLOW_CACHE:
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
            self.send_header("Pragma", "no-cache")
            self.send_header("Expires", "0")
        # Cross-origin isolation → SharedArrayBuffer for the Scale-0 worker.
        # COEP credentialless matches coi-serviceworker.js on GH Pages so
        # Google Fonts / KaTeX CDN can load; require-corp blocked them.
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "credentialless")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header(
            "Content-Security-Policy",
            "default-src 'self'; "
            "script-src 'self' 'unsafe-inline' 'unsafe-eval' 'wasm-unsafe-eval' blob: https://cdn.jsdelivr.net; "
            "style-src 'self' 'unsafe-inline' https://fonts.googleapis.com https://cdn.jsdelivr.net; "
            "font-src 'self' https://fonts.gstatic.com https://cdn.jsdelivr.net; "
            "img-src 'self' data:; "
            "connect-src 'self' ws://127.0.0.1:* ws://localhost:* "
            "http://127.0.0.1:* http://localhost:*; "
            "worker-src 'self' blob:; "
            "child-src 'self' blob:",
        )
        super().end_headers()

    # ── GPU server (ws_server.exe) launcher API — loopback dev server only ──
    def do_GET(self):
        route = self.path.split("?", 1)[0]
        if CONTROLS.handle(self, route):
            return
        if route in ("/api/lattice/records/catalog", "/api/lattice/records/checkpoint"):
            return self._record_lattice(route)
        if route == "/api/strict-hydro/status":
            return self._strict_hydro_status()
        if route == "/api/gpu-server/status":
            return self._gpu_status()
        if route == "/api/gpu-server/download":
            return self._gpu_download()
        return super().do_GET()

    def _record_lattice(self, route):
        try:
            available = all(_contained(_ENGINE_ROOT, name).is_file() for name in RECORD_ARTIFACTS)
            if not available:
                self.send_error(503, "Build the strict Phi-v2 WASM runtime first")
                return
            module = _record_catalog_module()
            if route.endswith("/catalog"):
                data = module.catalog()
                data["available"] = True
                data["wasm_module_url"] = "/" + RECORD_ARTIFACTS[0]
                data["seeding"] = {"version": 2, "sizes": [3, 4, 7, 9, 17],
                    "channels": [{"id": c, "phase": module.C.phase(c),
                                  "polarity": module.C.polarity(c),
                                  "flag": module.C.STATES[c % module.C.N_STATES][0]}
                                 for c in range(module.C.N_CHANNELS)]}
                body = json.dumps(data).encode("utf-8")
                mime, digest = "application/json", None
            else:
                query = parse_qs(urlsplit(self.path).query, strict_parsing=True)
                if set(query) != {"scenario", "size"} or any(len(v) != 1 for v in query.values()):
                    raise ValueError("one registered scenario and size required")
                size = query["size"][0]
                if size not in {"3", "4", "7", "9", "17"}:
                    raise ValueError("unregistered size")
                body, digest = module.checkpoint(query["scenario"][0], int(size))
                mime = "application/octet-stream"
            self.send_response(200)
            self.send_header("Content-Type", mime)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("X-Phi-Law", module.P.LAW_ID)
            if digest:
                self.send_header("X-Checkpoint-SHA256", digest)
            self.end_headers()
            self.wfile.write(body)
        except (ValueError, KeyError):
            self.send_error(400, "Unregistered Phi-v2 preparation")

    def _strict_hydro_status(self):
        try:
            available = all(_contained(_ENGINE_ROOT, name).is_file()
                            for name in STRICT_HYDRO_ARTIFACTS)
        except (ValueError, OSError):
            available = False
        self._send_json({"available": available,
                         "url": STRICT_HYDRO_URL if available else None})

    def do_POST(self):
        route = self.path.split("?", 1)[0]
        if CONTROLS.handle(self, route):
            return
        if route == "/api/lattice/records/seed":
            return self._record_seed()
        if route == "/api/lattice/records/seed-description":
            return self._record_seed(describe=True)
        if route == "/api/gpu-server/start":
            return self._gpu_start()
        return self.send_error(404, "Not found")

    def _record_seed(self, describe=False):
        if not self._client_is_local():
            return self._send_json({"error": "Local preparation service required"}, 403)
        if not all(_contained(_ENGINE_ROOT, name).is_file() for name in RECORD_ARTIFACTS):
            return self._send_json({"error": "Local finite-state artifacts unavailable"}, 503)
        origin = self.headers.get("Origin")
        if origin and origin != "http://" + self.headers.get("Host", ""):
            return self._send_json({"error": "Same-origin preparation request required"}, 403)
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 262144:
                return self._send_json({"error": "Recipe must be between 1 byte and 256 KiB"}, 413)
            if self.headers.get_content_type() != "application/json":
                return self._send_json({"error": "JSON recipe required"}, 415)
            module = _record_catalog_module()
            from phi_v2_lattice import web_seeding
            recipe = json.loads(self.rfile.read(length))
            if describe:
                from phi_v2_lattice.web_seed_presets import describe_recipe, prepare_v2
                prepare_v2(recipe)  # validate before allocating descriptor masks
                return self._send_json(describe_recipe(recipe))
            body, digest, recipe_digest = web_seeding.checkpoint(recipe)
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("X-Phi-Law", module.P.LAW_ID)
            self.send_header("X-Checkpoint-SHA256", digest)
            self.send_header("X-Recipe-SHA256", recipe_digest)
            self.end_headers()
            self.wfile.write(body)
        except (ValueError, KeyError, TypeError) as error:
            return self._send_json({"error": str(error)}, 400)

    def _client_is_local(self):
        return bool(self.client_address) and self.client_address[0] in ("127.0.0.1", "::1")

    def _send_json(self, obj, code=200):
        body = json.dumps(obj).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _gpu_status(self):
        if not self._client_is_local():
            return self._send_json({"error": "forbidden (loopback only)"}, 403)
        exists = os.path.isfile(WS_SERVER_EXE)
        self._send_json({
            "running": _gpu_running(),
            "port": GPU_PORT,
            "exeExists": exists,
            "exeSize": os.path.getsize(WS_SERVER_EXE) if exists else 0,
        })

    def _gpu_start(self):
        # Loopback only, and only ever launch the ONE known binary with a single
        # validated integer arg — never a client-supplied command, path, or flag.
        if not self._client_is_local():
            return self._send_json({"error": "forbidden (loopback only)"}, 403)
        if _gpu_running():
            return self._send_json({"running": True, "message": "already running"})
        if not os.path.isfile(WS_SERVER_EXE):
            return self._send_json(
                {"error": "ws_server.exe not built — run engine\\build_native.bat"}, 404)
        body = server_controls.read_json_body(self)
        try:
            lattice = int(body.get("lattice", 0))
        except (ValueError, TypeError):
            lattice = 0
        args = [WS_SERVER_EXE]
        if 0 < lattice <= 256:  # 0 / out-of-range → the server picks its own default
            args.append(str(lattice))
        # idleMinutes/autoStop (spec §3): at most one extra validated flag,
        # never anything else derived from the request body.
        args.extend(server_controls.idle_timeout_args(body))
        try:
            creationflags = 0
            if os.name == "nt":
                # Detach so the GPU server outlives this dev server — Ctrl-C here
                # must not kill the running simulation.
                creationflags = subprocess.DETACHED_PROCESS | subprocess.CREATE_NEW_PROCESS_GROUP
            proc = subprocess.Popen(
                args, cwd=_ENGINE_ROOT, creationflags=creationflags,
                stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            self._send_json({"started": True, "pid": proc.pid,
                             "lattice": lattice if 0 < lattice <= 256 else "default"})
        except OSError as exc:
            self._send_json({"error": str(exc)}, 500)

    def _gpu_download(self):
        if not self._client_is_local():
            return self.send_error(403, "forbidden (loopback only)")
        if not os.path.isfile(WS_SERVER_EXE):
            return self.send_error(404, "ws_server.exe not built")
        size = os.path.getsize(WS_SERVER_EXE)
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Disposition", 'attachment; filename="ws_server.exe"')
        self.send_header("Content-Length", str(size))
        self.end_headers()
        with open(WS_SERVER_EXE, "rb") as fh:
            while True:
                chunk = fh.read(1 << 16)
                if not chunk:
                    break
                self.wfile.write(chunk)


class DashboardHTTPServer(http.server.ThreadingHTTPServer):
    # Chromium imports hundreds of dashboard modules at startup. The default
    # five-connection backlog can refuse a module during that initial burst.
    request_queue_size = 128


def main():
    global ALLOW_CACHE, QUIET
    raw = sys.argv[1:]
    if "--cache" in raw:
        ALLOW_CACHE = True
    if "--quiet" in raw:
        QUIET = True
    host = "127.0.0.1"
    if "--host" in raw:
        idx = raw.index("--host")
        if idx + 1 >= len(raw):
            print("serve.py: --host requires an address", file=sys.stderr)
            sys.exit(2)
        host = raw[idx + 1]
        raw = raw[:idx] + raw[idx + 2:]
    idle_minutes, raw = server_controls.parse_idle_timeout_arg(raw)
    args = [a for a in raw if a not in ("--cache", "--quiet")]
    port = int(args[0]) if args else 8080
    web_root = os.path.dirname(os.path.abspath(__file__))
    os.chdir(web_root)
    server = DashboardHTTPServer((host, port), NoCacheHandler)
    server.allow_reuse_address = True
    mode = "cache" if ALLOW_CACHE else "no-cache"
    quiet = ", quiet" if QUIET else ""
    idle_desc = server_controls.idle_policy_banner(idle_minutes)
    print(f"FTD dev server: http://{host}:{port} ({mode}, COOP/COEP{quiet}, {idle_desc})  [Ctrl-C to stop]", flush=True)
    CONTROLS.start_watchdog(idle_minutes, server.shutdown)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.", flush=True)
    finally:
        CONTROLS.stop_watchdog()
        server.server_close()


if __name__ == "__main__":
    main()
