"""Serve local candidate artifacts with platform-independent module MIME types."""
from __future__ import annotations

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


class LaboratoryHandler(SimpleHTTPRequestHandler):
    extensions_map = {
        **SimpleHTTPRequestHandler.extensions_map,
        ".js": "application/javascript",
        ".mjs": "application/javascript",
        ".wasm": "application/wasm",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8092)
    args = parser.parse_args()
    engine = Path(__file__).resolve().parents[1]
    handler = partial(LaboratoryHandler, directory=str(engine))
    with ThreadingHTTPServer(("127.0.0.1", args.port), handler) as server:
        print(f"Strict candidate: http://127.0.0.1:{args.port}/strict/web/", flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
