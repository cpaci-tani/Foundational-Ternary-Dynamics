#!/usr/bin/env python3
"""Rewrite markdown links to moved files across the known navigation layers,
then grep the whole tracked tree for any leftover reference to an old path
so nothing silently goes stale.

Consumes a moves-log JSON (list of {"old": repo-relative, "new":
repo-relative} produced by apply_moves.py). For each of the NAV_FILES below,
computes the exact relative-link string a markdown link inside that file
would have used for the old path (relative to the nav file's own directory,
forward slashes, matching how these docs are actually authored) and replaces
it with the equivalent string for the new path. Only exact, parenthesised
markdown-link matches are touched -- e.g. "](07_assessment/AUDIT_X.md)" or
"](../AUDIT_X.md)" -- so a basename that happens to also appear in prose is
left alone.

After rewriting, greps the whole `git ls-files` tree for the OLD relative
path string (repo-root-relative) as a final safety net; anything it finds
outside the moved file's own git-mv history is printed for manual fixup --
this catches references this script's fixed nav-file list doesn't know
about (a script hardcoding a path, a stray doc link, etc).

Usage:
    python scripts/theory/reorg/fix_references.py --moves-log scratch/moves_X.json
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]

# (nav file, base directory the file's own relative links are resolved against)
NAV_FILES = [
    (REPO_ROOT / "docs" / "theory" / "META_INDEX.md", REPO_ROOT / "docs" / "theory"),
    (REPO_ROOT / "docs" / "theory" / "07_assessment" / "INDEX_07_ASSESSMENT.md", REPO_ROOT / "docs" / "theory" / "07_assessment"),
    (REPO_ROOT / "docs" / "theory" / "10_eft_program" / "INDEX_FTD_NATIVE_EFT.md", REPO_ROOT / "docs" / "theory" / "10_eft_program"),
    (REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md", REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers"),
]


def relposix(target: Path, base: Path) -> str:
    return Path(__import__("os").path.relpath(target, base)).as_posix()


def rewrite_nav_file(nav_path: Path, base_dir: Path, moves: list[dict]) -> int:
    if not nav_path.exists():
        return 0
    text = nav_path.read_text(encoding="utf-8")
    n_hits = 0
    for mv in moves:
        old_target = REPO_ROOT / mv["old"]
        new_target = REPO_ROOT / mv["new"]
        old_link = relposix(old_target, base_dir)
        new_link = relposix(new_target, base_dir)
        if old_link == new_link:
            continue
        # Match "](<old_link>)" or "](<old_link>#anchor)" exactly.
        pattern = re.compile(r"\]\(" + re.escape(old_link) + r"(#[^)]*)?\)")
        def _sub(m, new_link=new_link):
            return f"]({new_link}{m.group(1) or ''})"
        text, count = pattern.subn(_sub, text)
        n_hits += count
    if n_hits:
        nav_path.write_text(text, encoding="utf-8")
    return n_hits


def replace_full_path_everywhere(moves: list[dict]) -> int:
    """Beyond markdown-link syntax in the curated NAV_FILES, many docs
    mention a file by its bare repo-root-relative path in prose or inline
    code spans (`docs/theory/07_assessment/AUDIT_X.md`), not as a
    `[text](link)`. Do a literal whole-string replace of the old
    repo-root-relative path with the new one, across every tracked .md/.py
    file. This is safe because a repo-root-relative path string is, by
    construction, not a substring anyone would type by coincidence."""
    tracked = subprocess.run(
        ["git", "ls-files", "--", "*.md", "*.py"],
        cwd=REPO_ROOT, check=True, capture_output=True, text=True,
    ).stdout.splitlines()
    new_paths = {mv["new"] for mv in moves}
    total = 0
    for rel in tracked:
        if rel in new_paths:
            continue  # the moved file's own new location can't contain its old self-path
        p = REPO_ROOT / rel
        if not p.exists():
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        original = text
        for mv in moves:
            if mv["old"] in text:
                text = text.replace(mv["old"], mv["new"])
        if text != original:
            p.write_text(text, encoding="utf-8")
            n = sum(original.count(mv["old"]) for mv in moves)
            print(f"  {n:3d} bare-path mention(s) rewritten in {rel}")
            total += n
    return total


def grep_leftover_references(moves: list[dict]) -> list[str]:
    tracked = subprocess.run(
        ["git", "ls-files", "--", "*.md", "*.py"],
        cwd=REPO_ROOT, check=True, capture_output=True, text=True,
    ).stdout.splitlines()
    old_paths = {mv["old"] for mv in moves}
    new_paths = {mv["new"] for mv in moves}
    hits = []
    for rel in tracked:
        p = REPO_ROOT / rel
        if rel in new_paths or not p.exists():
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for old in old_paths:
            if old in text:
                hits.append(f"{rel}  still contains '{old}'")
    return hits


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--moves-log", required=True)
    args = ap.parse_args()

    moves_log_path = Path(args.moves_log)
    if not moves_log_path.is_absolute():
        moves_log_path = REPO_ROOT / moves_log_path
    moves = json.loads(moves_log_path.read_text(encoding="utf-8"))

    total = 0
    for nav_path, base_dir in NAV_FILES:
        n = rewrite_nav_file(nav_path, base_dir, moves)
        if n:
            print(f"  {n:3d} link(s) rewritten in {nav_path.relative_to(REPO_ROOT)}")
        total += n
    print(f"Rewrote {total} link(s) across {len(NAV_FILES)} navigation files.")

    print("\nSweeping whole tracked tree for bare repo-relative path mentions...")
    bare_total = replace_full_path_everywhere(moves)
    print(f"Rewrote {bare_total} bare-path mention(s).")

    leftovers = grep_leftover_references(moves)
    if leftovers:
        print(f"\n{len(leftovers)} leftover reference(s) NOT covered by NAV_FILES -- review manually:")
        for h in leftovers:
            print(f"  {h}")
        return 1
    print("No leftover references to old paths found outside NAV_FILES.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
