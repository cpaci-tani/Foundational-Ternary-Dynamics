#!/usr/bin/env python3
"""Verify every markdown link in the reorg's navigation-layer files resolves
to a file that exists on disk. Strict superset of
scripts/verification/verify_index_links.py (which checks only META_INDEX.md
and INDEX_FTD_NATIVE_EFT.md and never exits nonzero): this also checks
INDEX_07_ASSESSMENT.md and LEDGER.md, and exits 1 if anything is broken, so
the reorg driver can gate a commit on it.

Usage:
    python scripts/theory/reorg/verify_links.py
"""
from __future__ import annotations

import os
import re
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")

REPO_ROOT = Path(__file__).resolve().parents[3]
LINK_RE = re.compile(r"\[([^\]]+)\]\(([^)]+)\)")

NAV_FILES = [
    (REPO_ROOT / "docs" / "theory" / "META_INDEX.md", REPO_ROOT / "docs" / "theory"),
    (REPO_ROOT / "docs" / "theory" / "07_assessment" / "INDEX_07_ASSESSMENT.md", REPO_ROOT / "docs" / "theory" / "07_assessment"),
    (REPO_ROOT / "docs" / "theory" / "10_eft_program" / "INDEX_FTD_NATIVE_EFT.md", REPO_ROOT / "docs" / "theory" / "10_eft_program"),
    (REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md", REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers"),
]


def check(nav_path: Path, base_dir: Path) -> tuple[int, list[str]]:
    if not nav_path.exists():
        return 0, []
    text = nav_path.read_text(encoding="utf-8")
    total = 0
    broken = []
    for _name, path in LINK_RE.findall(text):
        if path.startswith(("http", "mailto:", "#")):
            continue
        clean = path.split("#")[0].split("?")[0]
        if not clean:
            continue
        total += 1
        target = os.path.normpath(os.path.join(base_dir, clean))
        if not os.path.exists(target):
            broken.append(f"{nav_path.relative_to(REPO_ROOT)}: [{_name}]({path}) -> {target}")
    return total, broken


def main() -> int:
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--baseline",
        help="path to a text file of known-pre-existing broken-link lines "
             "(one per line, as printed by this script) to ignore -- the "
             "reorg driver gates on NEW breakage only, not corpus debt this "
             "reorg didn't create.",
    )
    args = ap.parse_args()

    baseline: set[str] = set()
    if args.baseline and Path(args.baseline).exists():
        baseline = {ln.strip() for ln in Path(args.baseline).read_text(encoding="utf-8").splitlines() if ln.strip()}

    all_broken: list[str] = []
    grand_total = 0
    for nav_path, base_dir in NAV_FILES:
        total, broken = check(nav_path, base_dir)
        grand_total += total
        all_broken.extend(broken)
        print(f"{nav_path.relative_to(REPO_ROOT)}: {total} links, {len(broken)} broken")

    new_broken = [b for b in all_broken if b not in baseline]
    known_broken = [b for b in all_broken if b in baseline]

    if known_broken:
        print(f"\n{len(known_broken)} broken link(s) match the known baseline (pre-existing, ignored):")
        for b in known_broken:
            print(f"  {b}")

    if new_broken:
        print(f"\nNEW BROKEN ({len(new_broken)}):")
        for b in new_broken:
            print(f"  {b}")
        return 1
    print(f"\nNo new broken links. ({grand_total} total links checked, {len(known_broken)} pre-existing/baseline)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
