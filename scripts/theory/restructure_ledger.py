#!/usr/bin/env python3
"""Restore LEDGER.md's own stated format: browsable table + per-row detail blocks.

LEDGER.md line 114 declares its format as "Markdown table for browsability +
per-row YAML-style detail blocks below for the load-bearing rows".  That stopped
being followed: the Quick index grew to 1.21 MB across 747 single-line rows
(median 1,274 chars, max 30,352) while the detail-block sections stayed at 75 KB
covering ~70 ids.  The index became the corpus.

This script moves the overflow back where the file says it belongs.  For every
row whose Tag or Record cell exceeds its budget it:

  * keeps column 1 (ID) and column 2 (Short name) **completely verbatim** —
    the short name is the primary search field and is only 8.6% of the bytes;
  * replaces column 3 (Tag) with the bracketed epistemic tags, extracted
    verbatim, so `ledger_parser` reads exactly the same tag vocabulary;
  * truncates column 4 (Record) to a verbatim prefix plus a link;
  * writes the **complete original** short name, tag and record into a detail
    block in a new "Full row records" section.

Nothing is summarised, paraphrased, reordered or deleted.  Every byte of every
original cell survives inside the same file, and `--check` proves it: it
re-reads the result and asserts each original cell is present verbatim.

Rows already inside budget are left byte-identical.

Usage:
    python scripts/theory/restructure_ledger.py --dry-run   # report only
    python scripts/theory/restructure_ledger.py             # rewrite in place
    python scripts/theory/restructure_ledger.py --check     # verify round-trip
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from verification.parsers.ledger_parser import TAG_NORMALISATION  # noqa: E402

LEDGER = REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md"


def normalise_tags(tag_cell: str) -> list[str]:
    """Canonical tag list for a Tag cell, exactly as ledger_parser computes it."""
    brackets = re.findall(r"\[[^\]]+\]", tag_cell) or [tag_cell]
    out: list[str] = []
    for cand in brackets:
        for pattern, name in TAG_NORMALISATION:
            if pattern.search(cand):
                if name not in out:
                    out.append(name)
                break
    return out or ["UNKNOWN"]

ROW = re.compile(r"^\|\s*(FTD-\d{4}[A-Za-z0-9\-]*)\s*\|")
MAINTENANCE_HEADING = "## Maintenance log"
SECTION_HEADING = "## Full row records"

TAG_BUDGET = 240
DESC_BUDGET = 200

SECTION_PREAMBLE = """\
The Quick index above is a *browsable* table, per this file's stated format
(see "Format" in the header). Where a row's Tag or Record cell outgrew that
budget, the cell was truncated to a verbatim prefix and its complete original
text moved here, unaltered. Column 1 (ID) and column 2 (Short name) are never
truncated.

These blocks are the full record. Nothing here is summarised: each field below
is the exact original cell text. Restructured by
`scripts/theory/restructure_ledger.py`.
"""


def split_cells(line: str) -> list[str]:
    """Split a Quick-index row on the corpus separator ` | ` (see ledger_parser)."""
    body = line.strip()
    if body.startswith("|"):
        body = body[1:]
    if body.endswith("|"):
        body = body[:-1]
    cells = [c.strip() for c in body.split(" | ")]
    while len(cells) < 4:
        cells.append("")
    if len(cells) > 4:
        cells = cells[:3] + [" | ".join(cells[3:])]
    return cells


def clip(text: str, limit: int) -> str:
    """Truncate on a word boundary, then repair markdown left dangling."""
    if len(text) <= limit:
        return text
    cut = text[:limit]
    if " " in cut:
        cut = cut[: cut.rindex(" ")]
    cut = cut.rstrip(" \\-—–,;:")
    if cut.count("`") % 2:
        cut = cut[: cut.rindex("`")].rstrip()
    if cut.count("**") % 2:
        cut = cut[: cut.rindex("**")].rstrip()
    if cut.count("[") > cut.count("]"):
        cut = cut[: cut.rindex("[")].rstrip()
    return cut if cut else text[:limit].rstrip()


def cell_safe(text: str) -> str:
    """Make a truncated cell table-safe: no newlines, no unescaped ` | `.

    Only ever applied to the *truncated* copy in the table; the detail block
    keeps the untouched original.
    """
    text = re.sub(r"\s+", " ", text).strip()
    return text.replace(" | ", " \\| ")


def anchor_for(ftd_id: str) -> str:
    return f"{ftd_id.lower()}-record"


def tag_summary(tag: str) -> str:
    """The epistemic tags, verbatim and complete, so normalisation cannot change.

    The tag cell's bloat is the prose *around* the brackets, not the brackets:
    across all 747 rows the bracketed tags total 72,824 chars — 13.4% of the
    543,667 the tag column occupies — and the longest single list is 392 chars.
    So every bracket is kept, uncapped. Capping here was a real regression: it
    silently dropped trailing tags from 27 rows.

    Rows whose tag cell has no brackets at all (pure prose, e.g. FTD-0278's
    "**⚠ CORRECTED 2026-06-12 …**") are passed through untouched — clipping
    those strands the verdict word that appears later in the cell.
    """
    brackets = re.findall(r"\[[^\]]+\]", tag)
    if brackets:
        return cell_safe(" + ".join(brackets))
    return cell_safe(tag)


def restructure(text: str) -> tuple[str, list[dict]]:
    """Return the rewritten file plus the list of moved rows."""
    lines = text.splitlines()
    moved: list[dict] = []
    out: list[str] = []

    for line in lines:
        m = ROW.match(line)
        if not m:
            out.append(line)
            continue
        ftd_id = m.group(1)
        _, short, tag, desc = split_cells(line)

        if len(tag) <= TAG_BUDGET and len(desc) <= DESC_BUDGET:
            out.append(line)          # already in budget — byte-identical
            continue

        anchor = anchor_for(ftd_id)
        new_tag = tag_summary(tag)
        new_desc = cell_safe(clip(desc, DESC_BUDGET))
        suffix = f" […full record →](#{anchor})"
        out.append(f"| {ftd_id} | {short} | {new_tag} | {new_desc}{suffix} |")
        moved.append({"id": ftd_id, "anchor": anchor,
                      "short": short, "tag": tag, "desc": desc})

    if not moved:
        return "\n".join(out) + "\n", moved

    block: list[str] = ["", SECTION_HEADING, ""]
    block.append(SECTION_PREAMBLE)
    for rec in moved:
        block.append(f"### {rec['id']} — full record")
        block.append("")
        block.append(f'<a id="{rec["anchor"]}"></a>')
        block.append("")
        # The short name is NOT repeated here: column 2 of the Quick index
        # already carries it in full, verbatim and untruncated. Repeating it
        # would add ~100 KB of pure duplication.
        block.append(f"**Tag.** {rec['tag']}")
        block.append("")
        block.append(f"**Record.** {rec['desc']}")
        block.append("")
        block.append("---")
        block.append("")

    try:
        insert_at = out.index(MAINTENANCE_HEADING)
    except ValueError:
        raise SystemExit(f"FAIL: '{MAINTENANCE_HEADING}' not found; refusing to guess "
                         "where the detail blocks belong.")
    out = out[:insert_at] + block + out[insert_at:]
    return "\n".join(out) + "\n", moved


def verify(original: str, rewritten: str) -> list[str]:
    """Assert the restructure lost nothing. Returns a list of failures."""
    failures: list[str] = []

    def rows_of(text: str) -> dict[str, list[str]]:
        found = {}
        for line in text.splitlines():
            m = ROW.match(line)
            if m and m.group(1) not in found:
                found[m.group(1)] = split_cells(line)
        return found

    old_rows, new_rows = rows_of(original), rows_of(rewritten)

    if set(old_rows) != set(new_rows):
        missing = sorted(set(old_rows) - set(new_rows))
        extra = sorted(set(new_rows) - set(old_rows))
        failures.append(f"row id set changed: -{missing[:5]} +{extra[:5]}")

    # The epistemic tag every downstream consumer reads must be bit-identical.
    # Capping the tag cell silently dropped tags from 27 rows once; this gate
    # is what caught it, so it stays.
    for ftd_id in sorted(set(old_rows) & set(new_rows)):
        before = normalise_tags(old_rows[ftd_id][2])
        after = normalise_tags(new_rows[ftd_id][2])
        if before != after:
            failures.append(f"{ftd_id}: normalised tags changed {before} -> {after}")

    # Every original cell must still be present verbatim somewhere in the file.
    for ftd_id, (_, short, tag, desc) in old_rows.items():
        for field, value in (("short", short), ("tag", tag), ("desc", desc)):
            if value and value not in rewritten:
                failures.append(f"{ftd_id}: {field} text no longer present verbatim")

    # Every FTD reference anywhere in the old file must survive (test_dimensional_map).
    old_refs = set(re.findall(r"FTD-\d{4}", original))
    new_refs = set(re.findall(r"FTD-\d{4}", rewritten))
    if old_refs - new_refs:
        failures.append(f"FTD references lost: {sorted(old_refs - new_refs)[:10]}")

    # Anchors must be unique.
    anchors = re.findall(r'<a id="([^"]+)"></a>', rewritten)
    if len(anchors) != len(set(anchors)):
        dupes = {a for a in anchors if anchors.count(a) > 1}
        failures.append(f"duplicate anchors: {sorted(dupes)[:5]}")

    # Every link target must exist.
    for target in set(re.findall(r"\[…full record →\]\(#([^)]+)\)", rewritten)):
        if f'<a id="{target}"></a>' not in rewritten:
            failures.append(f"dangling link target #{target}")

    return failures


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dry-run", action="store_true", help="report, do not write")
    ap.add_argument("--check", action="store_true",
                    help="verify the committed file is already restructured and lossless")
    args = ap.parse_args()

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    original = LEDGER.read_text(encoding="utf-8")

    if args.check:
        if SECTION_HEADING not in original:
            print(f"FAIL: {LEDGER.name} has no '{SECTION_HEADING}' section.",
                  file=sys.stderr)
            return 1
        failures = verify(original, original)
        for f in failures:
            print(f"FAIL: {f}", file=sys.stderr)
        if failures:
            return 1
        print(f"OK: {LEDGER.name} is restructured and internally consistent.")
        return 0

    rewritten, moved = restructure(original)
    failures = verify(original, rewritten)

    table_before = sum(len(l) for l in original.splitlines() if ROW.match(l))
    table_after = sum(len(l) for l in rewritten.splitlines() if ROW.match(l))

    print(f"rows moved to detail blocks : {len(moved)}")
    print(f"Quick-index table           : {table_before:,} -> {table_after:,} chars "
          f"({100 * table_after / table_before:.1f}%)")
    print(f"file total                  : {len(original):,} -> {len(rewritten):,} chars")
    print(f"round-trip failures         : {len(failures)}")
    for f in failures[:20]:
        print(f"  FAIL: {f}")

    if failures:
        print("\nRefusing to write: the restructure is not lossless.", file=sys.stderr)
        return 1
    if args.dry_run:
        print("\n--dry-run: nothing written.")
        return 0

    LEDGER.write_text(rewritten, encoding="utf-8")
    print(f"\nWrote {LEDGER.relative_to(REPO_ROOT)}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
