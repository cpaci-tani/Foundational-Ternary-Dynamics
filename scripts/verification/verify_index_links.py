"""Check public navigation links, optionally requiring targets in the Git index."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
INDEX_PATHS = (
    Path("README.md"),
    Path("CLAUDE.md"),
    Path("REPOSITORY_MAP.md"),
    Path("docs/WHERE_WE_LEFT_OFF.md"),
    Path("docs/theory/META_INDEX.md"),
    Path("docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md"),
    Path("docs/theory/07_assessment/INDEX_07_ASSESSMENT.md"),
)
LINK_PATTERN = re.compile(r"\[([^\]]+)\]\(([^)]+)\)")


def tracked_paths(root: Path) -> set[str]:
    result = subprocess.run(
        ["git", "ls-files", "--cached", "-z"],
        cwd=root,
        check=True,
        capture_output=True,
    )
    return {os.fsdecode(path) for path in result.stdout.split(b"\0") if path}


def verify_file_links(index: Path, root: Path, tracked: set[str] | None = None) -> int:
    print(f"\nChecking links in {index.name}...")
    if not index.is_file():
        print(f"  [MISSING INDEX] {index.relative_to(root).as_posix()}")
        return 1
    if tracked is not None and index.relative_to(root).as_posix() not in tracked:
        print(f"  [UNTRACKED INDEX] {index.relative_to(root).as_posix()}")
        return 1

    total = 0
    broken = 0
    for name, path in LINK_PATTERN.findall(index.read_text(encoding="utf-8")):
        if path.startswith(("http", "mailto:", "#")):
            continue
        clean_path = path.split("#", 1)[0].split("?", 1)[0]
        if not clean_path:
            continue

        total += 1
        target = (index.parent / clean_path).resolve()
        if tracked is None:
            valid = target.exists()
            label = "BROKEN"
        else:
            try:
                relative = target.relative_to(root).as_posix()
                valid = relative in tracked or any(
                    candidate.startswith(relative.rstrip("/") + "/") for candidate in tracked
                )
            except ValueError:
                valid = False
            label = "UNTRACKED"
        if not valid:
            print(f"  [{label}] {name} -> {path}")
            broken += 1

    print(f"Verified {total} file links. Broken: {broken}")
    return broken


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--tracked", action="store_true",
        help="require every local link target to be in git ls-files",
    )
    parser.add_argument("--root", type=Path, default=ROOT, help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    root = args.root.resolve()

    try:
        tracked = tracked_paths(root) if args.tracked else None
    except (OSError, subprocess.CalledProcessError) as exc:
        print(f"Could not list tracked paths: {exc}", file=sys.stderr)
        return 2

    broken = sum(verify_file_links(root / relative, root, tracked) for relative in INDEX_PATHS)
    return 1 if broken else 0


if __name__ == "__main__":
    raise SystemExit(main())
