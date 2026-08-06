#!/usr/bin/env python3
"""Apply one classification batch: git mv every ASSIGN record for a given
programme slug (from a classify_loose_files.py manifest) into a new
subdirectory next to the source directory.

Prints every git mv it runs and writes a moves-log JSON (old_path ->
new_path, both repo-root-relative, forward slashes) that
fix_references.py consumes to update navigation-layer links.

Usage:
    python scripts/theory/reorg/apply_moves.py \
        --manifest scratch/07_assessment.json \
        --programme spine_master_quadratic \
        --moves-log scratch/moves_07_assessment_spine_master_quadratic.json \
        [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--programme", required=True, help="target_slug to move (must match manifest)")
    ap.add_argument("--moves-log", required=True)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    manifest_path = Path(args.manifest)
    if not manifest_path.is_absolute():
        manifest_path = REPO_ROOT / manifest_path
    records = json.loads(manifest_path.read_text(encoding="utf-8"))

    batch = [r for r in records if r["decision"] == "ASSIGN" and r["target_slug"] == args.programme]
    if not batch:
        print(f"No ASSIGN records for programme {args.programme!r} in {manifest_path}")
        return 1

    moves = []
    for r in batch:
        old_rel = r["file"]  # repo-root-relative, forward slashes
        old_path = REPO_ROOT / old_rel
        target_dir = old_path.parent / args.programme
        new_path = target_dir / old_path.name
        new_rel = str(new_path.relative_to(REPO_ROOT)).replace("\\", "/")

        if not old_path.exists():
            print(f"SKIP (already moved?): {old_rel}", file=sys.stderr)
            continue

        target_dir.mkdir(parents=True, exist_ok=True)

        cmd = ["git", "mv", str(old_path), str(new_path)]
        print(" ".join(f'"{c}"' if " " in c else c for c in cmd))
        if not args.dry_run:
            subprocess.run(cmd, cwd=REPO_ROOT, check=True)

        moves.append({"old": old_rel, "new": new_rel})

    moves_log_path = Path(args.moves_log)
    if not moves_log_path.is_absolute():
        moves_log_path = REPO_ROOT / moves_log_path
    moves_log_path.parent.mkdir(parents=True, exist_ok=True)
    moves_log_path.write_text(json.dumps(moves, indent=2) + "\n", encoding="utf-8")

    print(f"\n{'[DRY RUN] Would move' if args.dry_run else 'Moved'} {len(moves)} files.")
    print(f"Moves log: {moves_log_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
