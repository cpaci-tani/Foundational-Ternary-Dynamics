#!/usr/bin/env python3
"""Drive the full reorg: for every (manifest, programme) pair with pending
ASSIGN records, run apply_moves -> fix_references -> verify_links(baseline)
-> git commit. Stops immediately on the first verification failure so a
human can inspect before anything further changes.

Usage:
    python scripts/theory/reorg/run_batches.py \
        --manifest scratch/07_assessment.json --source-name 07_assessment \
        --manifest scratch/preregistrations.json --source-name preregistrations \
        --manifest scratch/derivations.json --source-name derivations \
        --manifest scratch/eft_root.json --source-name eft_root \
        [--skip spine_master_quadratic:07_assessment] \
        [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
REORG_DIR = Path(__file__).resolve().parent
BASELINE = REORG_DIR / "baseline_broken_links.txt"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    print("  $ " + " ".join(cmd))
    return subprocess.run(cmd, cwd=REPO_ROOT, check=True, capture_output=True, text=True, **kw)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", action="append", required=True)
    ap.add_argument("--source-name", action="append", required=True)
    ap.add_argument("--skip", action="append", default=[], help="slug:source_name pairs already handled")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    if len(args.manifest) != len(args.source_name):
        print("ERROR: --manifest and --source-name counts must match", file=sys.stderr)
        return 2

    skip = set(args.skip)
    batches = []  # (manifest_path, source_name, slug, n_files)
    for manifest_path, source_name in zip(args.manifest, args.source_name):
        records = json.loads(Path(manifest_path).read_text(encoding="utf-8"))
        slugs = sorted({r["target_slug"] for r in records if r["decision"] == "ASSIGN"})
        for slug in slugs:
            key = f"{slug}:{source_name}"
            if key in skip:
                continue
            n = sum(1 for r in records if r["decision"] == "ASSIGN" and r["target_slug"] == slug)
            batches.append((manifest_path, source_name, slug, n))

    total_files = sum(b[3] for b in batches)
    print(f"{len(batches)} batches queued, {total_files} files total.\n")

    done = 0
    for manifest_path, source_name, slug, n in batches:
        print(f"=== [{done+1}/{len(batches)}] {source_name} / {slug} ({n} files) ===")
        moves_log = REORG_DIR / f"moves_{source_name}_{slug}.json"

        if args.dry_run:
            run(["python", "scripts/theory/reorg/apply_moves.py",
                 "--manifest", manifest_path, "--programme", slug,
                 "--moves-log", str(moves_log), "--dry-run"])
            done += 1
            continue

        run(["python", "scripts/theory/reorg/apply_moves.py",
             "--manifest", manifest_path, "--programme", slug,
             "--moves-log", str(moves_log)])

        fix_result = subprocess.run(
            ["python", "scripts/theory/reorg/fix_references.py", "--moves-log", str(moves_log)],
            cwd=REPO_ROOT, capture_output=True, text=True,
        )
        print(fix_result.stdout)
        if fix_result.returncode != 0:
            print(fix_result.stderr, file=sys.stderr)
            print(f"ABORT: fix_references.py found leftover references it couldn't fix "
                  f"for {source_name}/{slug}. Files are already moved (git status shows "
                  f"the batch); resolve manually, then re-run with the remaining batches "
                  f"and --skip for everything already committed.")
            return 1

        verify_result = subprocess.run(
            ["python", "scripts/theory/reorg/verify_links.py", "--baseline", str(BASELINE)],
            cwd=REPO_ROOT, capture_output=True, text=True,
        )
        print(verify_result.stdout)
        if verify_result.returncode != 0:
            print(verify_result.stderr, file=sys.stderr)
            print(f"ABORT: NEW broken links after {source_name}/{slug}. "
                  f"Batch is moved+reference-fixed but NOT committed -- "
                  f"`git status` / `git diff` to inspect, fix, or `git checkout .` "
                  f"and `git clean -fd docs/theory` to fully revert this one batch.")
            return 1

        commit_msg = (
            f"Theory docs reorg: {source_name}/{slug} ({n} files)\n\n"
            f"File {source_name} loose docs classified as {slug} (LEDGER_INDEX.md "
            f"programme) into docs/theory/.../{slug}/, per the plurality-of-cited-"
            f"FTD-ids rule in scripts/theory/reorg/classify_loose_files.py. "
            f"Reference-only edits in navigation files; no content or tag changes."
        )
        run(["git", "add", "-A", "--",
             "docs/theory/", "scripts/theory/reorg/", "CHANGELOG.md", "META_DOCUMENTATION_MAP.md",
             "docs/SPEC_FTD.md"])
        run(["git", "commit", "-m", commit_msg])
        done += 1
        print(f"Committed. ({done}/{len(batches)})\n")

    print(f"\nAll {done} batches complete.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
