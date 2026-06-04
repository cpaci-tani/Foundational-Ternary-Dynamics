"""
Dev http server for engine/web/ that disables browser caching.

Why this exists: ES-modules served via `python -m http.server` are
cached aggressively by browsers. After every JS edit the cache had
to be defeated by a hard refresh, by URL-param cache-busting in code,
or by bouncing the server. All three are friction we don't need.

Usage (replaces `python -m http.server 8080 -d engine/web`):

    python engine/web/serve.py            # port 8080
    python engine/web/serve.py 9090       # custom port

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
import os
import sys


class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
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


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    web_root = os.path.dirname(os.path.abspath(__file__))
    os.chdir(web_root)
    server = http.server.ThreadingHTTPServer(("", port), NoCacheHandler)
    server.allow_reuse_address = True
    print(f"FTD dev server: http://localhost:{port} (no-cache)  [Ctrl-C to stop]", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.", flush=True)
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
