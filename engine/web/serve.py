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
response, plus `Pragma: no-cache` and `Expires: 0` for the older
browsers. Same MIME defaults as http.server's SimpleHTTPRequestHandler;
no other change.

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


# ── GPU server (ws_server.exe) launcher paths ────────────────────────────────
# The splash screen's "GPU Acceleration" card talks to the three /api/gpu-server/
# routes below to show status, start the CUDA WebSocket server, and download the
# binary. This only works when the dashboard is served by THIS script locally;
# on GitHub Pages the routes are absent and the card degrades to a static note.
_WEB_ROOT = os.path.dirname(os.path.abspath(__file__))          # engine/web
_ENGINE_ROOT = os.path.dirname(_WEB_ROOT)                        # engine
WS_SERVER_EXE = os.path.join(_ENGINE_ROOT, "build", "Release", "ws_server.exe")
GPU_PORT = 9100


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
    def log_message(self, format, *args):
        if not QUIET:
            super().log_message(format, *args)

    def end_headers(self):
        if not ALLOW_CACHE:
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
            self.send_header("Pragma", "no-cache")
            self.send_header("Expires", "0")
        # Cross-origin isolation → unlocks SharedArrayBuffer for the Scale-0
        # physics Web Worker (zero-copy field sharing). COOP+COEP make the page
        # crossOriginIsolated; CORP:same-origin lets every same-origin asset
        # (JS modules, the WASM binary, the worker) satisfy COEP require-corp.
        # All dashboard assets are same-origin, so nothing is blocked. If a
        # cross-origin (CDN) asset is ever added it must send its own CORP/CORS.
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        super().end_headers()

    # ── GPU server (ws_server.exe) launcher API — loopback dev server only ──
    def do_GET(self):
        route = self.path.split("?", 1)[0]
        if route == "/api/gpu-server/status":
            return self._gpu_status()
        if route == "/api/gpu-server/download":
            return self._gpu_download()
        return super().do_GET()

    def do_POST(self):
        route = self.path.split("?", 1)[0]
        if route == "/api/gpu-server/start":
            return self._gpu_start()
        return self.send_error(404, "Not found")

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
        lattice = 0
        length = int(self.headers.get("Content-Length", 0) or 0)
        if length:
            try:
                lattice = int((json.loads(self.rfile.read(length) or b"{}") or {}).get("lattice", 0))
            except (ValueError, TypeError):
                lattice = 0
        args = [WS_SERVER_EXE]
        if 0 < lattice <= 256:  # 0 / out-of-range → the server picks its own default
            args.append(str(lattice))
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
    args = [a for a in raw if a not in ("--cache", "--quiet")]
    port = int(args[0]) if args else 8080
    web_root = os.path.dirname(os.path.abspath(__file__))
    os.chdir(web_root)
    server = http.server.ThreadingHTTPServer((host, port), NoCacheHandler)
    server.allow_reuse_address = True
    mode = "cache" if ALLOW_CACHE else "no-cache"
    quiet = ", quiet" if QUIET else ""
    print(f"FTD dev server: http://{host}:{port} ({mode}, COOP/COEP{quiet})  [Ctrl-C to stop]", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.", flush=True)
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
