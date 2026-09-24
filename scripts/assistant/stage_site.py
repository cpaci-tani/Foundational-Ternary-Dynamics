"""Create a Pages artifact from an immutable Git revision plus locked AI assets.

The source tree is read through git archive; ignored/untracked files never ship.
The destination must be empty. No existing site or user files are removed.
"""
import argparse
import io
import json
from pathlib import Path
import subprocess
import tarfile
from urllib.parse import urlsplit

from prepare_assets import ROOT

PUBLIC_ROOT_FILES = frozenset({
    "coi-serviceworker.js",
    "fields-atlas.html",
    "index.html",
    "wasm-threads-proof.html",
    "wasm-threads-proof.worker.js",
})
PUBLIC_ASSET_SUFFIXES = {
    "assets": frozenset({".hdr", ".json"}),
    "css": frozenset({".css"}),
    "data": frozenset({".json"}),
    "demos": frozenset({".html"}),
    "js": frozenset({".js", ".mjs", ".cjs", ".css", ".json", ".wasm"}),
    "wasm": frozenset({".js", ".json", ".wasm"}),
}


def is_public_web_file(relative: Path) -> bool:
    if len(relative.parts) == 1:
        return relative.name in PUBLIC_ROOT_FILES
    suffixes = PUBLIC_ASSET_SUFFIXES.get(relative.parts[0])
    return bool(suffixes and relative.suffix in suffixes)


def stage_web_archive(archive: bytes, output: Path) -> None:
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        for entry in source:
            if not entry.isfile():
                continue
            relative = Path(entry.name).relative_to("engine/web")
            if not is_public_web_file(relative):
                continue
            target = (output / relative).resolve()
            if not target.is_relative_to(output):
                raise ValueError("Archive path escapes output")
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(source.extractfile(entry).read())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--revision", default="HEAD")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--api-base", default="")
    args = parser.parse_args()
    revision = subprocess.check_output(["git", "rev-parse", "--verify", args.revision + "^{commit}"], cwd=ROOT, text=True).strip()
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error("Output must be empty; existing files will not be removed")
    if args.api_base:
        endpoint = urlsplit(args.api_base)
        if endpoint.scheme != "https" or not endpoint.hostname or endpoint.username or endpoint.password or endpoint.path not in ("", "/") or endpoint.query or endpoint.fragment:
            parser.error("Public API base must be an HTTPS origin without credentials, query or path")
    output.mkdir(parents=True, exist_ok=True)
    archive = subprocess.check_output(["git", "archive", "--format=tar", revision, "engine/web"], cwd=ROOT)
    stage_web_archive(archive, output)
    release_lock = output / "js/assistant/model-manifest.json"
    if not release_lock.is_file() or json.loads(release_lock.read_text(encoding="utf-8")) != json.loads((ROOT / "engine/web/js/assistant/model-manifest.json").read_text(encoding="utf-8")):
        raise SystemExit("Release revision must contain the same assistant model lock as the build checkout")
    subprocess.run(["python", str(ROOT / "scripts/assistant/prepare_assets.py"), "download"], check=True)
    subprocess.run(["python", str(ROOT / "scripts/assistant/prepare_assets.py"), "stage", "--destination", str(output / "data/assistant/models")], check=True)
    subprocess.run(["python", str(ROOT / "scripts/assistant/build_knowledge.py"), "--mode", "public", "--revision", revision,
                    "--output", str(output / "data/assistant/knowledge")], check=True)
    config = output / "js/assistant/config.json"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text(json.dumps({"apiBase": args.api_base.rstrip("/"), "knowledgeManifest": "./data/assistant/knowledge/manifest.json",
                                 "modelBase": "./data/assistant/models/"}, indent=2) + "\n", encoding="utf-8")
    total = sum(p.stat().st_size for p in output.rglob("*") if p.is_file())
    if total >= 800_000_000:
        raise SystemExit(f"Pages artifact {total} bytes exceeds the 800 MB release safety budget")
    print(json.dumps({"sourceRevision": revision, "artifactBytes": total, "apiConfigured": bool(args.api_base)}))


if __name__ == "__main__":
    main()
