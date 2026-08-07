#!/usr/bin/env python3
"""
Render docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md ->
TRACKER_OPEN_ITEMS_INDEX.md

TRACKER_OPEN_ITEMS.md is 3,100+ lines of narrative physics prose -- multiple
paragraphs of detail per item -- the same shape LEDGER.md had before
LEDGER_INDEX.md existed to compress it. Nothing can read TRACKER_OPEN_ITEMS.md
whole, so "has this item already been investigated" requires a compact
companion. Same principle as LEDGER_INDEX.md, deliberately NOT applied to
INDEX_FTD_NATIVE_EFT.md: that file is already 88% one-line table rows (a
per-file catalog, not narrative), so a generated meta-index of it wouldn't
compress anything -- its size is proportional to corpus size, not a
structural inefficiency.

TRACKER_OPEN_ITEMS.md remains canonical. This index is read-only with
respect to it: no item text is rewritten, no status is reclassified, and
open/closed counts are derived mechanically from each item's own heading
text, not asserted independently.

Usage:
    python scripts/theory/build_open_items_index.py            # regenerate
    python scripts/theory/build_open_items_index.py --check    # diff-only mode

Exit codes:
    0   regenerated successfully (or --check matched committed file)
    2   --check found drift (index out of sync with TRACKER_OPEN_ITEMS.md)
"""
from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TRACKER = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS.md"
INDEX = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS_INDEX.md"

SECTION_RE = re.compile(r"^## (§\S+.*)$", re.MULTILINE)
# Only treat the leading token as an item number if it actually looks like
# one (e.g. "1.9", "1.9a", "2.13", "9.5"). A handful of legacy §10 headings
# ("### G* & Master Quadratic...", "### Prior scope") don't follow the N.M
# convention at all; matching any first word as "the number" mis-split
# those into a fake number plus a mangled title. Fall back to the whole
# heading as the title with no number in that case.
NUMBERED_ITEM_RE = re.compile(r"^### (\d+(?:\.\d+)?[a-z]?)\s+(.*)$", re.MULTILINE)
UNNUMBERED_ITEM_RE = re.compile(r"^### (?!\d)(.+)$", re.MULTILINE)
EM_DASH_SPLIT = re.compile(r"\s+—\s+")

CLOSED_RE = re.compile(
    r"CLOSED|RETRACTED|not counted (?:as )?open|not counted open", re.IGNORECASE
)


@dataclass
class Item:
    number: str
    title: str
    status: str
    raw_heading: str
    section: str


def gfm_slug(heading: str) -> str:
    """Best-effort GitHub-flavored-markdown heading anchor. TRACKER_OPEN_ITEMS.md
    uses plain headings (no {#custom-id} / <a id=> syntax), so this is the
    anchor a standard renderer computes. Low-stakes if imperfect: a mismatch
    lands the reader on the right file, just not scrolled to the exact spot."""
    s = heading.replace("**", "").replace("`", "")
    s = s.lower()
    s = re.sub(r"[^\w\- ]+", "", s)
    s = s.strip()
    s = re.sub(r"\s+", "-", s)
    return s


def parse_tracker(text: str) -> list[Item]:
    sections = list(SECTION_RE.finditer(text))

    def section_for(pos: int) -> str:
        section = "§ (unsectioned)"
        for sm in sections:
            if sm.start() < pos:
                section = sm.group(1)
            else:
                break
        return section

    matches: list[tuple[int, str, str]] = []  # (pos, number_or_empty, rest)
    for m in NUMBERED_ITEM_RE.finditer(text):
        matches.append((m.start(), m.group(1), m.group(2).strip()))
    for m in UNNUMBERED_ITEM_RE.finditer(text):
        matches.append((m.start(), "", m.group(1).strip()))
    matches.sort(key=lambda t: t[0])

    items: list[Item] = []
    for pos, number, rest in matches:
        parts = EM_DASH_SPLIT.split(rest, maxsplit=1)
        title = parts[0].strip()
        status = parts[1].strip() if len(parts) > 1 else ""
        items.append(Item(
            number=number or "—", title=title, status=status,
            raw_heading=f"{number} {rest}".strip(), section=section_for(pos),
        ))
    return items


def is_closed(item: Item) -> bool:
    return bool(CLOSED_RE.search(item.status)) or bool(CLOSED_RE.search(item.title))


def render(items: list[Item]) -> str:
    n_open = sum(1 for i in items if not is_closed(i))
    n_closed = len(items) - n_open

    lines = [
        "# Open Items Tracker — Index",
        "",
        f"Generated companion to `TRACKER_OPEN_ITEMS.md` ({len(items)} item "
        f"headings). **Do not edit by hand** — regenerate with "
        "`python scripts/theory/build_open_items_index.py`.",
        "",
        "This is a *navigation aid*, not a source of truth. `TRACKER_OPEN_ITEMS.md` "
        "remains canonical; open/closed status here is read mechanically from "
        "each item's own heading text (a `CLOSED`/`RETRACTED`/`not counted as "
        "open` marker), not reclassified. Where this index and the tracker's "
        "prose disagree, the tracker wins.",
        "",
        f"**{n_open} open, {n_closed} closed/retired** (of {len(items)} item headings).",
        "",
        "---",
        "",
    ]

    by_section: dict[str, list[Item]] = {}
    for it in items:
        by_section.setdefault(it.section, []).append(it)

    for section, sec_items in by_section.items():
        sec_open = sum(1 for i in sec_items if not is_closed(i))
        lines.append(f"## {section}")
        lines.append("")
        lines.append(f"**{sec_open} open / {len(sec_items)} total.**")
        lines.append("")
        lines.append("| # | Item | Status | Open? |")
        lines.append("|---|---|---|---|")
        for it in sec_items:
            anchor = gfm_slug(it.raw_heading)
            title_cell = it.title.replace("|", "\\|")
            status_cell = (it.status or "—").replace("|", "\\|")
            open_mark = "" if is_closed(it) else "**OPEN**"
            lines.append(
                f"| {it.number} | [{title_cell}](TRACKER_OPEN_ITEMS.md#{anchor}) "
                f"| {status_cell} | {open_mark} |"
            )
        lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()

    text = TRACKER.read_text(encoding="utf-8")
    items = parse_tracker(text)
    if not items:
        print("FAIL: no item headings parsed from TRACKER_OPEN_ITEMS.md", file=sys.stderr)
        return 1

    rendered = render(items)

    if args.check:
        if not INDEX.exists() or INDEX.read_text(encoding="utf-8") != rendered:
            print(f"--check: drift detected. {INDEX} is out of sync with {TRACKER}. "
                  f"Re-run `python scripts/theory/build_open_items_index.py` to regenerate.",
                  file=sys.stderr)
            return 2
        print(f"OK: {INDEX.name} is in sync ({len(items)} items).")
        return 0

    INDEX.write_text(rendered, encoding="utf-8")
    print(f"Wrote {INDEX} ({len(items)} items, {len(rendered.splitlines())} lines).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
