#!/usr/bin/env python3
"""Generate LEDGER_INDEX.md — a compact, categorised index of every LEDGER claim.

Read-only with respect to LEDGER.md: no claim text is rewritten, no tag is
changed, no row is dropped.  Every field emitted here is a verbatim substring of
the ledger row it came from, or a count derived from it.

Why this file exists
--------------------
LEDGER.md is ~1.6 MB.  Nothing can read it whole, so "has X already been
looked at?" degrades to grep, and a grep hit returns a 30 KB table row.  This
index is ~1 line per claim, grouped by research programme, so an agent can read
the entire claim space in one pass and then jump to the two or three rows that
matter.

Usage:
    python scripts/theory/build_ledger_index.py            # write the index
    python scripts/theory/build_ledger_index.py --check    # exit 1 if stale
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter, OrderedDict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from verification.parsers.ledger_parser import TAG_NORMALISATION  # noqa: E402
from verification.parsers.ledger_taxonomy import (  # noqa: E402
    CATEGORIES,
    category_for,
    subcategory_for,
)

LEDGER = REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md"
INDEX = REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER_INDEX.md"

# Row id pattern is looser than ledger_parser's: it must also catch the one
# non-numeric-suffix row, FTD-0136-PhaseB-final.
ROW = re.compile(r"^\|\s*(FTD-\d{4}[A-Za-z0-9\-]*)\s*\|")

SUMMARY_CHARS = 150
TAG_CHARS = 90


def split_cells(line: str) -> list[str]:
    """Split a Quick-index row on the corpus separator ` | `.

    Mirrors ledger_parser.parse_quick_index_row: internal pipes in math
    notation (|Aut(E)|^2, |J|^2) carry no surrounding spaces and so do not
    split.  Over-splits are re-merged into the final cell.
    """
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


def flatten(text: str) -> str:
    """Collapse whitespace, drop bold markers, and keep pipes table-safe.

    `**` is pure emphasis inside a one-line table cell, and leaving it in makes
    truncation destructive (clipping a `**bold row**` at a word boundary strands
    the opening marker).  Words are untouched; only the emphasis markup goes.
    Pipes are escaped once — the corpus already escapes some of them.
    """
    text = re.sub(r"\s+", " ", text).strip()
    text = text.replace("**", "")
    return re.sub(r"(?<!\\)\|", r"\\|", text)


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
    if cut.count("[") > cut.count("]"):
        cut = cut[: cut.rindex("[")].rstrip()
    return (cut + "…") if cut else text[:limit].rstrip() + "…"


def normalised_tags(tag_cell: str) -> list[str]:
    """Canonical tag names for `tag_cell`, via the ledger_parser normaliser.

    Reuses the project's existing tag vocabulary so this index and
    math_node_map.json cannot drift into two different tag namespaces.
    """
    brackets = re.findall(r"\[[^\]]+\]", tag_cell) or [tag_cell]
    out: list[str] = []
    for cand in brackets:
        for pattern, name in TAG_NORMALISATION:
            if pattern.search(cand):
                if name not in out:
                    out.append(name)
                break
    return out or ["UNKNOWN"]


def tags_of(tag_cell: str) -> str:
    """The COMPLETE canonical tag list — never truncated.

    This column originally showed the verbatim bracket text clipped to 90
    chars. That was actively dangerous: 170 rows rendered as `[THEOREM] …`
    while a `[CLOSED NEGATIVE]`, `[FOUNDATIONAL OBSTRUCTION]` or `[RETRACTED]`
    later in the same cell fell off the end. An index that makes a refuted
    claim read as a theorem is worse than no index at all, and this corpus is
    majority negative results.

    Canonical names are short, so the full verdict set always fits. The
    verbatim bracket prose stays one click away in LEDGER.md.
    """
    return ", ".join(normalised_tags(tag_cell))


def collect() -> list[dict]:
    """Parse every Quick-index row into an index record."""
    records: list[dict] = []
    seen: set[str] = set()
    for lineno, line in enumerate(LEDGER.read_text(encoding="utf-8").splitlines(), 1):
        m = ROW.match(line)
        if not m:
            continue
        ftd_id = m.group(1)
        if ftd_id in seen:
            continue
        seen.add(ftd_id)
        _, short, tag, desc = split_cells(line)
        cat = category_for(ftd_id)
        records.append(
            {
                "id": ftd_id,
                "line": lineno,
                "summary": clip(flatten(short), SUMMARY_CHARS),
                "tags": tags_of(tag),
                "norm_tags": normalised_tags(tag),
                "category": cat,
                "subcategory": subcategory_for(ftd_id, cat) if cat else None,
                # kept for the drift check only, never emitted
                "_raw_len": len(short) + len(tag) + len(desc),
            }
        )
    return records


def sort_key(rec: dict) -> tuple:
    m = re.match(r"FTD-(\d{4})(.*)", rec["id"])
    return (int(m.group(1)), m.group(2)) if m else (10**6, rec["id"])


def render(records: list[dict]) -> str:
    by_cat: OrderedDict[str, list[dict]] = OrderedDict((k, []) for k in CATEGORIES)
    for r in records:
        by_cat[r["category"]].append(r)

    out: list[str] = []
    add = out.append

    add("# LEDGER index — categorised claim map")
    add("")
    add(f"**Generated** by `scripts/theory/build_ledger_index.py` from "
        f"`LEDGER.md` ({len(records)} claim rows). **Do not edit by hand** — "
        "regenerate instead.")
    add("")
    add("This is a *navigation aid*, not a source of truth. `LEDGER.md` remains "
        "the single source of truth for claim status; where this index and the "
        "ledger disagree, **the ledger wins**.")
    add("")
    add("**Reading the tag column.** It lists the row's *complete* canonical tag "
        "set — never truncated — so a row carrying both `[THEOREM]` and "
        "`[CLOSED NEGATIVE]` shows both. This corpus is majority negative "
        "results, and a claim's verdict is frequently the *last* tag in its "
        "cell. The `Claim` column is a verbatim (possibly truncated) substring "
        "of the row's short name; the verbatim tag prose is in `LEDGER.md`.")
    add("")
    add("**How to use this file:** read it whole to see what has already been "
        "investigated, then open the cited `LEDGER.md` line for the full record. "
        "Searching the ledger directly is the failure mode this file exists to "
        "prevent — its rows run to 30 KB each.")
    add("")

    add("## Contents")
    add("")
    add("| Programme | Claims | Scope |")
    add("|---|---:|---|")
    for key, (title, scope) in CATEGORIES.items():
        anchor = re.sub(r"[^a-z0-9]+", "-", title.lower()).strip("-")
        add(f"| [{title}](#{anchor}) | {len(by_cat[key])} | {scope} |")
    add("")
    add("---")
    add("")

    for key, (title, scope) in CATEGORIES.items():
        rows = sorted(by_cat[key], key=sort_key)
        add(f"## {title}")
        add("")
        add(f"*{scope}*")
        add("")
        add(f"**{len(rows)} claims.**")
        add("")
        subs: OrderedDict[str | None, list[dict]] = OrderedDict()
        for r in rows:
            subs.setdefault(r["subcategory"], []).append(r)
        for sub, sub_rows in subs.items():
            if sub is not None:
                add(f"### {title} — {sub}")
                add("")
            add("| ID | Epistemic tag | Claim | LEDGER |")
            add("|---|---|---|---:|")
            for r in sub_rows:
                add(f"| `{r['id']}` | {r['tags']} | {r['summary']} | "
                    f"L{r['line']} |")
            add("")
        add("---")
        add("")

    tag_counts = Counter()
    for r in records:
        for norm in r["norm_tags"]:
            tag_counts[norm] += 1
    add("## Tag frequency across all rows")
    add("")
    add("Canonical tags via `ledger_parser.TAG_NORMALISATION` — the same "
        "normaliser that feeds `math_node_map.json`. A row carrying several "
        "tags is counted under each, so the total exceeds the row count.")
    add("")
    add("| Tag | Rows |")
    add("|---|---:|")
    for tag, n in tag_counts.most_common(30):
        add(f"| {tag} | {n} |")
    add("")
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--check", action="store_true",
                    help="exit 1 if the committed index differs from a fresh build")
    args = ap.parse_args()

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    records = collect()
    unassigned = [r["id"] for r in records if r["category"] is None]
    if unassigned:
        print(f"FAIL: {len(unassigned)} ledger ids have no category: "
              f"{unassigned[:10]}", file=sys.stderr)
        print("Add them to scripts/verification/parsers/ledger_taxonomy.py",
              file=sys.stderr)
        return 1

    rendered = render(records)

    if args.check:
        if not INDEX.exists():
            print(f"FAIL: {INDEX.name} does not exist; run without --check",
                  file=sys.stderr)
            return 1
        if INDEX.read_text(encoding="utf-8") != rendered:
            print(f"FAIL: {INDEX.name} is stale relative to LEDGER.md.",
                  file=sys.stderr)
            print("Regenerate: python scripts/theory/build_ledger_index.py",
                  file=sys.stderr)
            return 1
        print(f"OK: {INDEX.name} is in sync ({len(records)} rows).")
        return 0

    INDEX.write_text(rendered, encoding="utf-8")
    print(f"Wrote {INDEX.relative_to(REPO_ROOT)} "
          f"({len(rendered):,} bytes, {len(records)} rows, "
          f"{len(CATEGORIES)} categories, 0 unclassified).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
