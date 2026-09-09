"""Read-only verification of hash-linked, immutable recovery evidence.

This checks retained bytes and inventories. It does not infer scientific
acceptance from a manifest, run a campaign, or copy old fixtures into a new
worker's payload.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import time
import zipfile


SCHEMA = "strict-recovery-preservation-check-1"
_CACHES = frozenset(("__pycache__", ".pytest_cache"))


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024*1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _digest(value):
    if (type(value) is not str or len(value) != 64
            or any(c not in "0123456789abcdef" for c in value)):
        raise ValueError("invalid SHA256 identity")
    return value


def _path(directory, name):
    if (type(name) is not str or not name or "\\" in name
            or Path(name).is_absolute() or ":" in name
            or any(part in ("", ".", "..") for part in name.split("/"))):
        raise ValueError("expected canonical repository-relative path")
    result = (directory/name).resolve()
    if not result.is_relative_to(directory.resolve()) or not result.is_file():
        raise ValueError("missing file or path outside declared directory: "+name)
    return result


def _json(data):
    def unique(pairs):
        result = {}
        for name, value in pairs:
            if name in result:
                raise ValueError("duplicate JSON key")
            result[name] = value
        return result
    return json.loads(data, object_pairs_hook=unique)


def verify_preservation(root, seals):
    """Verify complete named seals and return a compact preservation receipt.

    Each descriptor has exactly manifest, sha256, source_count, artifact_count.
    Paths are canonical relative to root. Referenced manifest source maps and
    artifact maps remain authoritative only at the caller's approved hash.
    Both current source files and every archived source member are checked.
    Failures raise; the execution parent must retain its own failure receipt.
    """
    began = time.monotonic()
    root = Path(root).resolve()
    if not root.is_dir() or not isinstance(seals, (list, tuple)) or not seals:
        raise ValueError("a repository root and nonempty seal list are required")
    pins, manifests, seen = {}, [], set()

    def pin(path, digest):
        digest = _digest(digest)
        if path in pins and pins[path] != digest:
            raise ValueError("conflicting retained identities: "+str(path))
        pins[path] = digest

    for descriptor in seals:
        if (type(descriptor) is not dict
                or set(descriptor) != {"manifest", "sha256", "source_count", "artifact_count"}
                or any(type(descriptor[name]) is not int or descriptor[name] <= 0
                       for name in ("source_count", "artifact_count"))):
            raise ValueError("invalid seal descriptor")
        path = _path(root, descriptor["manifest"])
        if path in seen:
            raise ValueError("duplicate seal descriptor")
        seen.add(path)
        raw = path.read_bytes()
        expected = _digest(descriptor["sha256"])
        if hashlib.sha256(raw).hexdigest() != expected:
            raise ValueError("prior manifest identity changed")
        manifest = _json(raw)
        if type(manifest) is not dict:
            raise ValueError("manifest must be an object")
        sources, artifacts = manifest.get("source_sha256"), manifest.get("artifact_sha256")
        if (type(sources) is not dict or len(sources) != descriptor["source_count"]
                or type(artifacts) is not dict or len(artifacts) != descriptor["artifact_count"]):
            raise ValueError("frozen source or artifact count differs")
        actual = {item.relative_to(path.parent).as_posix() for item in path.parent.rglob("*")
                  if item.is_file() and item != path and not _CACHES.intersection(item.relative_to(path.parent).parts)}
        if actual != set(artifacts):
            raise ValueError("retained artifact inventory differs")
        for name, digest in sources.items():
            pin(_path(root, name), digest)
        for name, digest in artifacts.items():
            pin(_path(path.parent, name), digest)
        archive = _path(path.parent, manifest.get("source_archive"))
        archive_digest = _digest(manifest.get("source_archive_sha256"))
        if artifacts.get(archive.relative_to(path.parent).as_posix()) != archive_digest:
            raise ValueError("source archive is not bound by artifact inventory")
        pin(archive, archive_digest)
        pin(path, expected)
        manifests.append((descriptor, path, sources, archive))

    # Read each unique physical file once in this verification transaction.
    # Conflicting duplicate declarations were rejected before this pass.
    byte_count = 0
    for path, digest in sorted(pins.items()):
        if sha256(path) != digest:
            raise ValueError("retained input identity changed: "+str(path))
        byte_count += path.stat().st_size
    results = []
    for descriptor, path, sources, archive in manifests:
        with zipfile.ZipFile(archive) as zipped:
            names = zipped.namelist()
            if len(names) != len(sources) or set(names) != set(sources):
                raise ValueError("source archive member inventory differs")
            for name, digest in sources.items():
                if hashlib.sha256(zipped.read(name)).hexdigest() != digest:
                    raise ValueError("archived source bytes differ: "+name)
        results.append(dict(descriptor, archive_members_verified=len(sources),
                            archive_sha256=pins[archive]))
    return {"schema": SCHEMA, "status": "PASS", "seals": results,
            "unique_files_verified": len(pins), "file_bytes_hashed": byte_count,
            "elapsed_seconds": time.monotonic()-began,
            "scientific_result_recomputed": False, "canonical_adoption": False}
