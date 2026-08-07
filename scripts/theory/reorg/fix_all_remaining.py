#!/usr/bin/env python3
"""One-time comprehensive final pass: replace every old->new path from the
consolidated moves list, across every tracked file (any extension) except
confirmed provenance/historical snapshots (moves logs themselves,
engine/results/ run artifacts, docs/theory/10_eft_program/data/ experiment
metadata, *_lock*.json pre-registration locks).

The per-batch fix_references.py runs only checked each batch's own moved
paths against .md/.py files, so cross-references written by an EARLIER
batch's files, or living in other extensions (.qmd, .tex, .json, .cpp,
.html, .js, .lean), were missed. This closes that gap in one pass.

Usage:
    python scripts/theory/reorg/fix_all_remaining.py --moves-log scratch/all_moves_consolidated.json
"""
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]

EXCLUDE_PREFIXES = (
    "scripts/theory/reorg/moves_",
    "scripts/theory/reorg/baseline_broken_links.txt",
    "engine/results/",
    "docs/theory/10_eft_program/data/",
)
EXCLUDE_SUFFIXES = ("_lock.json", "_lock_v1.json", "_lock_v2.json")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--moves-log", required=True)
    args = ap.parse_args()

    moves = json.loads(Path(args.moves_log).read_text(encoding="utf-8"))
    tracked = subprocess.run(
        ["git", "ls-files"], cwd=REPO_ROOT, check=True, capture_output=True, text=True,
    ).stdout.splitlines()

    total_files = 0
    total_hits = 0
    for rel in tracked:
        if rel.startswith(EXCLUDE_PREFIXES) or rel.endswith(EXCLUDE_SUFFIXES):
            continue
        p = REPO_ROOT / rel
        if not p.is_file():
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="strict")
        except (UnicodeDecodeError, OSError):
            continue
        original = text
        n = 0
        for mv in moves:
            c = text.count(mv["old"])
            if c:
                text = text.replace(mv["old"], mv["new"])
                n += c
        if text != original:
            p.write_text(text, encoding="utf-8")
            print(f"  {n:3d} hit(s) rewritten in {rel}")
            total_files += 1
            total_hits += n

    print(f"\nRewrote {total_hits} hit(s) across {total_files} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
