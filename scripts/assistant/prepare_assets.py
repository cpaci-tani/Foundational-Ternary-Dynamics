"""Fetch/stage only the immutable AI artifacts recorded in the checked-in lock.

Usage: python scripts/assistant/prepare_assets.py download|verify|stage [--destination DIR]
Weights stay in ignored engine/.cache; staging is an explicit release operation.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import shutil
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "engine/web/js/assistant/model-manifest.json"
CACHE = ROOT / "engine/.cache/local-ai/models"


def contained(root, relative):
    root = Path(root).resolve()
    target = (root / relative).resolve()
    if not target.is_relative_to(root) or target == root:
        raise ValueError("Asset path escapes its root")
    return target


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def matches(path, entry):
    return path.is_file() and path.stat().st_size == entry["size"] and digest(path) == entry["sha256"]


def files(manifest):
    entries = {entry["path"]: entry for variant in manifest["variants"] for entry in variant["files"]}
    return list(entries.values())


def fetch(entry, cache=CACHE):
    target = contained(cache, entry["path"])
    if matches(target, entry):
        return target
    target.parent.mkdir(parents=True, exist_ok=True)
    partial = target.with_name(target.name + ".partial")
    request = urllib.request.Request(entry["url"], headers={"User-Agent": "FTD-pinned-assistant-assets/1"})
    try:
        with urllib.request.urlopen(request, timeout=120) as response, partial.open("wb") as sink:
            remaining = entry["size"]
            while block := response.read(min(1024 * 1024, remaining + 1)):
                remaining -= len(block)
                if remaining < 0:
                    raise ValueError("Artifact exceeds locked size")
                sink.write(block)
        if not matches(partial, entry):
            raise ValueError("Artifact hash/size differs from lock: " + entry["path"])
        partial.replace(target)
        return target
    finally:
        partial.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("download", "verify", "stage"))
    parser.add_argument("--destination", type=Path)
    args = parser.parse_args()
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    entries = files(manifest)
    if args.command == "download":
        with ThreadPoolExecutor(max_workers=4) as pool:
            list(pool.map(fetch, entries))
    missing = [entry["path"] for entry in entries if not matches(contained(CACHE, entry["path"]), entry)]
    if missing:
        raise SystemExit("Missing or changed locked assets; run download first: " + ", ".join(missing))
    if args.command == "stage":
        if args.destination is None:
            parser.error("stage requires --destination")
        for entry in entries:
            target = contained(args.destination, entry["path"])
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(contained(CACHE, entry["path"]), target)
    print(json.dumps({"verifiedFiles": len(entries), "bytes": sum(e["size"] for e in entries), "command": args.command}))


if __name__ == "__main__":
    main()
