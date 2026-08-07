#!/usr/bin/env python3
"""General markdown link resolver/repairer for docs/theory/ (and optionally
wider).

Unlike fix_references.py / fix_all_remaining.py (which only look for the
exact old-path STRING from a recorded move), this actually parses every
markdown link in every file, resolves it relative to *that file's own
directory*, and checks the target exists. When it doesn't, it searches the
whole tracked tree by basename:

  - exactly one candidate  -> repair the link to the correct relative path
  - zero or >1 candidates  -> report only, never guess

This catches link rot the move-log-driven tools structurally cannot: wrong
"../" depth after a target moved deeper into a new subdirectory, links from
files the per-batch reference-fixer never scanned, and pre-existing rot from
earlier reorganizations unrelated to this session's moves.

Usage:
    python scripts/theory/reorg/resolve_links.py --root docs/theory [--apply] [--out report.json]

Without --apply: report-only (dry run). With --apply: rewrites the files.
Either way, prints a summary and writes a full JSON report.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
from collections import defaultdict
from pathlib import Path
from urllib.parse import unquote

REPO_ROOT = Path(__file__).resolve().parents[3]
LINK_RE = re.compile(r"(?<!!)\[([^\]]*)\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")


def is_external(path: str) -> bool:
    return path.startswith(("http://", "https://", "mailto:", "#", "data:", "file:"))


def split_anchor(path: str) -> tuple[str, str]:
    if "#" in path:
        p, a = path.split("#", 1)
        return p, "#" + a
    return path, ""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="docs/theory", help="restrict scan to files under this repo-relative dir")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    root = REPO_ROOT / args.root
    tracked_all = subprocess.run(
        ["git", "ls-files"], cwd=REPO_ROOT, check=True, capture_output=True, text=True,
    ).stdout.splitlines()
    tracked_md = [f for f in tracked_all if f.endswith(".md") and f.startswith(args.root.replace("\\", "/") + "/")]

    # basename -> list of repo-relative paths, for repair lookup (search the WHOLE repo,
    # not just --root, since a target may have moved outside the scanned root, and
    # across ALL tracked file types -- a link to a .h/.cpp/.py file is just as real
    # a target as a link to a .md file, and excluding non-.md files here previously
    # made every such link misclassified as "dead" purely because the index couldn't
    # see it, even when the file genuinely exists a few '../' short of where linked)
    basename_index: dict[str, list[str]] = defaultdict(list)
    for f in tracked_all:
        basename_index[Path(f).name].append(f)

    n_links = 0
    n_broken = 0
    n_repaired = 0
    n_ambiguous = 0
    n_dead = 0
    repairs: list[dict] = []
    ambiguous: list[dict] = []
    dead: list[dict] = []

    for rel in tracked_md:
        p = REPO_ROOT / rel
        text = p.read_text(encoding="utf-8", errors="ignore")
        file_dir = p.parent
        changed = False
        out_chunks = []
        last_end = 0

        for m in LINK_RE.finditer(text):
            link_text, raw_target = m.group(1), m.group(2)
            if is_external(raw_target):
                continue
            n_links += 1
            target_path, anchor = split_anchor(unquote(raw_target))
            if not target_path:
                continue  # pure same-file anchor
            resolved = (file_dir / target_path).resolve()
            try:
                resolved_rel = resolved.relative_to(REPO_ROOT)
            except ValueError:
                resolved_rel = None
            if resolved.exists():
                continue

            n_broken += 1
            basename = Path(target_path).name
            candidates = basename_index.get(basename, [])
            record = {
                "file": rel, "link_text": link_text, "raw_target": raw_target,
                "resolved_attempt": str(resolved_rel) if resolved_rel else str(resolved),
            }
            if len(candidates) == 1:
                new_target_abs = REPO_ROOT / candidates[0]
                import os
                new_rel_path = Path(os.path.relpath(new_target_abs, file_dir)).as_posix()
                record["new_target"] = candidates[0]
                record["new_relative_link"] = new_rel_path
                repairs.append(record)
                n_repaired += 1
                if args.apply:
                    out_chunks.append(text[last_end:m.start(2)])
                    out_chunks.append(new_rel_path + anchor)
                    last_end = m.end(2)
                    changed = True
            elif len(candidates) > 1:
                record["candidates"] = candidates
                ambiguous.append(record)
                n_ambiguous += 1
            else:
                dead.append(record)
                n_dead += 1

        if args.apply and changed:
            out_chunks.append(text[last_end:])
            p.write_text("".join(out_chunks), encoding="utf-8")

    report = {
        "root": args.root, "applied": args.apply,
        "n_files_scanned": len(tracked_md), "n_links": n_links, "n_broken": n_broken,
        "n_repaired": n_repaired, "n_ambiguous": n_ambiguous, "n_dead": n_dead,
        "repairs": repairs, "ambiguous": ambiguous, "dead": dead,
    }
    out_path = Path(args.out) if args.out else (REPO_ROOT / "scripts" / "theory" / "reorg" / "link_resolve_report.json")
    out_path.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")

    mode = "APPLIED" if args.apply else "DRY RUN"
    print(f"[{mode}] Scanned {len(tracked_md)} files under {args.root}: {n_links} links, {n_broken} broken")
    print(f"  {n_repaired} repairable (unique basename match){' -- fixed' if args.apply else ' -- would fix, re-run with --apply'}")
    print(f"  {n_ambiguous} ambiguous (multiple basename matches, needs manual pick)")
    print(f"  {n_dead} genuinely dead (target not found anywhere in repo)")
    print(f"Full report: {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
