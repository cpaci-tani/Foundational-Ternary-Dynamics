#!/usr/bin/env python3
"""Classify loose theory-doc files into programme subdirectories.

For each *.md file directly inside a given source directory (non-recursive),
extract every FTD-NNNN id it cites, look each up in programme_map.json
(built by build_programme_map.py from LEDGER_INDEX.md), and decide:

  ASSIGN         -- one programme dominates the citations; file the doc there.
  CROSS_CUTTING  -- citations spread across many programmes with no clear
                    lead; likely a synthesis/roadmap doc. Leave at the
                    directory root; record the reason.
  UNCITED        -- zero FTD-id citations found anywhere in the file.
                    Needs a human read.

Decision rule (see docs/superpowers/specs/2026-08-06-theory-docs-structural-
reorg-design.md): plurality wins by default. A file is CROSS_CUTTING instead
of ASSIGN when it cites ids from >= CROSS_CUTTING_PROGRAMME_MIN distinct
programmes AND the leading programme's share of citations is below
CROSS_CUTTING_SHARE_MAX -- i.e. no programme clearly owns the document.

A file with an explicit "**Identifier:** `FTD-NNNN`" line (the
preregistrations/ convention) uses that id's programme directly, ignoring
the plurality rule -- a prereg's own identifier is authoritative over
whatever else it cites as context/parents.

Usage:
    python scripts/theory/reorg/classify_loose_files.py \
        --source-dir docs/theory/07_assessment \
        --out scratch/07_assessment_manifest.json

Output is JSON: a list of records, one per file, with the full decision
trail (cited ids, per-programme counts, decision, target path) so the
manifest is reviewable before anything moves.
"""
from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
PROGRAMME_MAP = Path(__file__).resolve().parent / "programme_map.json"

FTD_ID_RE = re.compile(r"FTD-(\d{4})")
IDENTIFIER_RE = re.compile(r"^\*\*Identifier:\*\*\s*`(FTD-\d{4})`", re.MULTILINE)

CROSS_CUTTING_PROGRAMME_MIN = 6
CROSS_CUTTING_SHARE_MAX = 0.35

# A plurality computed from very few citations isn't trustworthy -- e.g. a
# file citing just {FTD-0013, FTD-0014, FTD-0030} "wins" 2/3 for whichever
# programme FTD-0013/0014 happen to sit in, even though the file itself
# (observed in practice: PARKING_LOT.md, REF_CLAIMS_MATRIX.md) is a
# cross-cutting project/meta reference, not about that programme at all.
# Below this many known-programme citations, route to LOW_CONFIDENCE instead
# of ASSIGN. Set to 1 (i.e. effectively off -- any known_cited count already
# implies >=1 by the time this is reached) because the real failure mode
# observed in practice (PARKING_LOT.md, REF_CLAIMS_MATRIX.md: a low-count
# plurality landing a meta/reference doc in a random programme folder) is
# caught precisely by is_meta_filename() above, not by citation count --
# most focused single-topic audits legitimately cite only 1-3 ids and are
# correctly confident. Raise this only if a *different* low-count failure
# mode turns up during manual batch review.
MIN_KNOWN_CITED_FOR_ASSIGN = 1

# Filenames whose underscore-delimited tokens include one of these words
# are structurally reference/meta/index documents whose job is to span many
# programmes -- e.g. REF_CLAIMS_MATRIX.md ("canonical reference for all
# headline claims") cites a handful of ids from one programme almost by
# accident. Force these to CROSS_CUTTING regardless of citation stats,
# rather than trusting the plurality count.
# NOTE: filenames are ALL_CAPS_WITH_UNDERSCORES, and regex \b does not
# split on "_" (it's a \w character) -- token-split on "_" instead of
# using a \b-bounded regex, or "PARKING_LOT" silently never matches.
META_FILENAME_TOKENS = {
    "INDEX", "MATRIX", "MANIFEST", "PARKING", "CHANGELOG", "CATALOG",
    "ROADMAP", "GLOSSARY", "SYNONYMY", "MAP",
}


def is_meta_filename(stem: str) -> bool:
    tokens = re.split(r"[_\-]", stem.upper())
    return any(t in META_FILENAME_TOKENS for t in tokens)


def load_id_to_slug() -> dict[str, str]:
    data = json.loads(PROGRAMME_MAP.read_text(encoding="utf-8"))
    return data["id_to_slug"]


def classify_one(path: Path, id_to_slug: dict[str, str]) -> dict:
    text = path.read_text(encoding="utf-8", errors="replace")
    cited = sorted(set(f"FTD-{m}" for m in FTD_ID_RE.findall(text)))
    known_cited = [c for c in cited if c in id_to_slug]

    record = {
        "file": str(path.relative_to(REPO_ROOT)).replace("\\", "/"),
        "cited_ids": cited,
        "unmapped_ids": sorted(set(cited) - set(known_cited)),
    }

    identifier_match = IDENTIFIER_RE.search(text)
    if identifier_match:
        primary = identifier_match.group(1)
        record["primary_identifier"] = primary
        if primary in id_to_slug:
            record["decision"] = "ASSIGN"
            record["target_slug"] = id_to_slug[primary]
            record["reason"] = f"explicit Identifier field: {primary}"
            return record
        # fall through to plurality if the stated identifier isn't in the map

    if is_meta_filename(path.stem):
        record["decision"] = "CROSS_CUTTING"
        record["target_slug"] = None
        record["reason"] = "filename matches meta/reference/index pattern"
        return record

    if not known_cited:
        record["decision"] = "UNCITED"
        record["target_slug"] = None
        record["reason"] = "no FTD-id citation found (or none map to a known programme)"
        return record

    slug_counts = Counter(id_to_slug[c] for c in known_cited)
    n_programmes = len(slug_counts)
    top_slug, top_count = slug_counts.most_common(1)[0]
    top_share = top_count / len(known_cited)
    record["programme_counts"] = dict(slug_counts)

    if n_programmes >= CROSS_CUTTING_PROGRAMME_MIN and top_share < CROSS_CUTTING_SHARE_MAX:
        record["decision"] = "CROSS_CUTTING"
        record["target_slug"] = None
        record["reason"] = (
            f"cites {n_programmes} distinct programmes, "
            f"leading share {top_share:.0%} < {CROSS_CUTTING_SHARE_MAX:.0%}"
        )
        return record

    if len(known_cited) < MIN_KNOWN_CITED_FOR_ASSIGN:
        record["decision"] = "LOW_CONFIDENCE"
        record["target_slug"] = top_slug
        record["reason"] = (
            f"only {len(known_cited)} known-programme citation(s) "
            f"(< {MIN_KNOWN_CITED_FOR_ASSIGN}); plurality would be {top_slug} "
            f"({top_count}/{len(known_cited)}) but that's too thin to trust automatically"
        )
        return record

    record["decision"] = "ASSIGN"
    record["target_slug"] = top_slug
    record["reason"] = f"plurality: {top_count}/{len(known_cited)} citations ({top_share:.0%})"
    return record


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-dir", required=True, help="directory to scan, non-recursive")
    ap.add_argument("--out", required=True, help="output manifest JSON path")
    args = ap.parse_args()

    id_to_slug = load_id_to_slug()
    source_dir = Path(args.source_dir)
    if not source_dir.is_absolute():
        source_dir = REPO_ROOT / source_dir

    records = []
    for path in sorted(source_dir.glob("*.md")):
        records.append(classify_one(path, id_to_slug))

    out_path = Path(args.out)
    if not out_path.is_absolute():
        out_path = REPO_ROOT / out_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(records, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    decisions = Counter(r["decision"] for r in records)
    slugs = Counter(r["target_slug"] for r in records if r["decision"] == "ASSIGN")

    print(f"Scanned {len(records)} files in {source_dir}")
    print(f"Wrote manifest: {out_path}")
    print(f"Decisions: {dict(decisions)}")
    print("Per-programme ASSIGN counts:")
    for slug, n in sorted(slugs.items(), key=lambda kv: -kv[1]):
        print(f"  {n:4d}  {slug}")
    if decisions.get("CROSS_CUTTING"):
        print("\nCROSS_CUTTING files (left at root):")
        for r in records:
            if r["decision"] == "CROSS_CUTTING":
                print(f"  {r['file']}  -- {r['reason']}")
    if decisions.get("UNCITED"):
        print("\nUNCITED files (need manual read):")
        for r in records:
            if r["decision"] == "UNCITED":
                print(f"  {r['file']}")
    if decisions.get("LOW_CONFIDENCE"):
        print("\nLOW_CONFIDENCE files (thin plurality, need manual read):")
        for r in records:
            if r["decision"] == "LOW_CONFIDENCE":
                print(f"  {r['file']}  -- {r['reason']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
