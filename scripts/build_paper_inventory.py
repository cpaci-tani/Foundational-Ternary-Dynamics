"""build_paper_inventory.py — generate a structured database of every paper-shaped artifact.

OUTPUT:
    dissemination/papers/INVENTORY.json   — canonical database
    dissemination/papers/INVENTORY.md     — human-readable view

CLASSIFIES each paper by:
    - location_class:   active / draft / archived / retracted / pdf-only
    - epistemic_tier:   1 (bulletproof) / 2 (defensible) / 3 (conjectural-tagged) /
                        4 (parametric-heavy) / 5 (closed-negative or pre-reframe)
                        — heuristic, manual override allowed via OVERRIDE table below
    - post_reframe:     Y/N (touched after 2026-04-19 OR new in dissemination/papers/)
    - anti_target_clean:Y/N (no unflagged 'derive 1/α' / 'no free parameters' /
                        'first-principles α' phrases in the TeX source)
    - has_source:       Y/N (whether a TeX/MD source exists alongside the PDF)
    - verdict:          KEEP / REVISE / RETIRE / ARCHIVED

Run:
    python scripts/build_paper_inventory.py
"""

from __future__ import annotations

import json
import os
import re
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parent.parent
PAPER_DIRS = [
    ROOT / "docs" / "papers",
    ROOT / "dissemination" / "papers",
    ROOT / "dissemination" / "whitepaper",
]

REFRAME_DATE = datetime(2026, 4, 19, tzinfo=timezone.utc).timestamp()

# Manual classification overrides — paper IDs that we have explicit knowledge about.
# Key = filename stem (without extension), value = dict of fields to override.
OVERRIDE = {
    "PAPER_A_PI_FREE_GENERATOR": {
        "epistemic_tier": 1,
        "verdict": "KEEP",
        "notes": "Paper A — math core; trio submission target (LMP).",
    },
    "PAPER_B_BCC_COMPLEX_STRUCTURE": {
        "epistemic_tier": 1,
        "verdict": "KEEP",
        "notes": "Paper B — BCC complex structure + dual-4 partial unification + no-go.",
    },
    "PAPER_FTD_AS_WILSONIAN_EFT": {
        "epistemic_tier": 2,
        "verdict": "KEEP",
        "notes": "Paper C — Branch-A native EFT measurements; Phase-G reframed.",
    },
    "PAPER_MASTER_QUADRATIC_AND_BRIDGE": {
        "epistemic_tier": 3,
        "verdict": "REVISE",
        "notes": "Long-form master-quadratic companion; predecessor of Paper A.",
    },
}

# Anti-target patterns that should appear ONLY inside negation contexts.
# We look for these phrases and check if a negation word appears within ~80 chars before.
ANTI_TARGET_PATTERNS = [
    r"derive\s+(?:the\s+|both\s+)?fine[- ]structure",
    r"deriv(?:ation|e[ds]?|ing)\s+(?:of\s+)?(?:the\s+)?(?:1/)?(?:α|alpha)\b",
    r"first[- ]principles?\s+(?:α|alpha|fine[- ]structure)",
    r"zero\s+free\s+parameters?",
    r"no\s+free\s+parameters?",
    r"parameter[- ]free\s+(?:derivation|prediction)",
]
NEGATION_WORDS = re.compile(
    r"\b(?:not|no|never|cannot|conjecture|empirical|conditional|retract|retracted|"
    r"identif(?:y|ication)|motivat(?:ed|ion)|do\s+not|does\s+not|did\s+not)\b",
    re.IGNORECASE,
)


def find_papers() -> list[Path]:
    """Walk paper directories and return all .tex / .md / .pdf files (deduped by stem).

    Excludes:
      - meta-files that are not papers themselves: INVENTORY.md/json (would
        self-trigger anti-target audit; this script generates these),
        README.md, MASTER_ABSTRACT_CATALOG.md, DEPRECATED.md.
    """
    SELF_GENERATED = {"INVENTORY", "INVENTORY_OVERRIDES"}
    NOT_PAPERS = {"README", "MASTER_ABSTRACT_CATALOG", "DEPRECATED"}
    EXCLUDE_STEMS = SELF_GENERATED | NOT_PAPERS
    files: list[Path] = []
    for d in PAPER_DIRS:
        if not d.exists():
            continue
        for p in d.rglob("*"):
            if not p.is_file():
                continue
            if p.suffix.lower() not in (".tex", ".md", ".pdf"):
                continue
            if p.suffix.lower() in (".aux", ".log", ".out", ".toc", ".synctex", ".bbl"):
                continue
            # Skip generated build artifacts
            if any(s in p.name for s in (".aux", ".log", ".out", ".toc")):
                continue
            # Skip self-generated docs and non-paper meta-files
            if p.stem in EXCLUDE_STEMS:
                continue
            files.append(p)
    return files


def location_class(path: Path) -> str:
    p = str(path).replace("\\", "/")
    if "/archive/retracted_under_reframe/" in p:
        return "retracted"
    if "/archive/pdf_only_no_source/" in p:
        return "pdf-only"
    if "/archive/" in p:
        return "archived"
    if "/dissemination/papers/" in p:
        # Differentiate active trio from other drafts
        name = path.stem
        if name in ("PAPER_A_PI_FREE_GENERATOR", "PAPER_B_BCC_COMPLEX_STRUCTURE",
                    "PAPER_FTD_AS_WILSONIAN_EFT"):
            return "active"
        return "draft"
    if "/dissemination/whitepaper/" in p:
        return "draft"
    if "/docs/papers/src/" in p:
        return "draft-src"
    if "/docs/papers/speculative/" in p:
        return "speculative"
    if "/docs/papers/" in p:
        return "legacy"
    return "other"


def extract_title(tex_path: Path) -> Optional[str]:
    """Pull the LaTeX \\title{...} content if present. Returns None if not a TeX file or no title."""
    if tex_path.suffix.lower() != ".tex":
        return None
    try:
        text = tex_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    # Look for \title[...]{...} or \title{...}; capture the {...} content
    m = re.search(r"\\title(?:\[[^\]]*\])?\{((?:[^{}]|\{[^{}]*\})*)\}", text, re.DOTALL)
    if not m:
        return None
    raw = m.group(1)
    # Clean LaTeX commands roughly
    raw = re.sub(r"\\\\", " ", raw)
    raw = re.sub(r"\\[a-zA-Z]+\*?", "", raw)
    raw = re.sub(r"[{}]", "", raw)
    raw = re.sub(r"\s+", " ", raw).strip()
    return raw[:200] if raw else None


def anti_target_audit(tex_path: Path) -> tuple[bool, list[str]]:
    """Scan TeX source for anti-target phrases that aren't in a negation context.

    Returns (clean, list_of_offending_passages).
    """
    if tex_path.suffix.lower() not in (".tex", ".md"):
        return True, []
    try:
        text = tex_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return True, []
    offenses: list[str] = []
    for pattern in ANTI_TARGET_PATTERNS:
        for m in re.finditer(pattern, text, re.IGNORECASE):
            start = max(0, m.start() - 80)
            ctx_before = text[start:m.start()]
            if NEGATION_WORDS.search(ctx_before):
                continue  # negated, fine
            # Also check the phrase is not inside a comment line
            line_start = text.rfind("\n", 0, m.start()) + 1
            line = text[line_start:m.end() + 50]
            if line.lstrip().startswith("%"):
                continue
            snippet = text[max(0, m.start() - 30):min(len(text), m.end() + 50)]
            snippet = re.sub(r"\s+", " ", snippet).strip()
            offenses.append(snippet)
            if len(offenses) >= 5:
                return False, offenses
    return len(offenses) == 0, offenses


def heuristic_tier(path: Path, location: str, anti_target_ok: bool, post_reframe: bool) -> int:
    """Heuristic epistemic tier. 1 = bulletproof, 5 = pre-reframe / closed-negative."""
    if location == "retracted":
        return 5
    if location == "pdf-only":
        return 5  # can't audit, treat as untrusted
    if location == "speculative":
        return 4
    if location == "archived":
        return 4
    if location == "active":
        return 1 if anti_target_ok else 3
    if not post_reframe:
        return 4 if anti_target_ok else 5
    if not anti_target_ok:
        return 3
    return 2


def heuristic_verdict(location: str, tier: int, anti_target_ok: bool, has_source: bool) -> str:
    if location == "retracted":
        return "ARCHIVED"
    if location == "pdf-only":
        return "RETIRE"
    if location == "archived":
        return "ARCHIVED"
    if location == "active":
        return "KEEP" if anti_target_ok else "REVISE"
    if not has_source:
        return "RETIRE"
    if tier <= 2:
        return "KEEP"
    if tier == 3:
        return "REVISE"
    return "RETIRE"


def main():
    # Build inventory — group by stem so .tex / .pdf / .md pairs collapse to one row.
    files = find_papers()
    by_stem: dict[str, dict] = {}
    for p in files:
        stem = p.stem
        # If multiple files have the same stem at different depths, prefer the deepest (most specific)
        rel = p.relative_to(ROOT)
        entry = by_stem.setdefault(stem, {"stem": stem, "tex": None, "pdf": None, "md": None})
        ext = p.suffix.lower().lstrip(".")
        if ext in entry and (entry[ext] is None or len(str(rel)) < len(entry[ext])):
            entry[ext] = str(rel).replace("\\", "/")

    # Now per-stem analysis
    rows = []
    for stem, e in sorted(by_stem.items()):
        # Pick the canonical path: TeX > MD > PDF
        canonical_rel: str = e.get("tex") or e.get("md") or e.get("pdf") or ""
        canonical_path = ROOT / canonical_rel
        if not canonical_path.exists():
            continue
        loc = location_class(canonical_path)
        title = extract_title(canonical_path) if canonical_path.suffix.lower() == ".tex" else None
        # mtime: most recent across all formats
        mtimes = []
        for ext in ("tex", "md", "pdf"):
            if e.get(ext):
                p = ROOT / e[ext]
                if p.exists():
                    mtimes.append(p.stat().st_mtime)
        mtime = max(mtimes) if mtimes else 0
        post_reframe = mtime >= REFRAME_DATE
        anti_target_ok, offenses = (True, [])
        if e.get("tex"):
            anti_target_ok, offenses = anti_target_audit(ROOT / e["tex"])
        elif e.get("md"):
            anti_target_ok, offenses = anti_target_audit(ROOT / e["md"])
        has_source = bool(e.get("tex") or e.get("md"))
        size_kb = 0
        if e.get("tex"):
            size_kb = (ROOT / e["tex"]).stat().st_size // 1024
        elif e.get("pdf"):
            size_kb = (ROOT / e["pdf"]).stat().st_size // 1024

        tier = heuristic_tier(canonical_path, loc, anti_target_ok, post_reframe)
        verdict = heuristic_verdict(loc, tier, anti_target_ok, has_source)

        # Apply manual overrides
        ov = OVERRIDE.get(stem, {})
        if "epistemic_tier" in ov:
            tier = ov["epistemic_tier"]
        if "verdict" in ov:
            verdict = ov["verdict"]

        rows.append({
            "stem": stem,
            "title": title,
            "tex": e.get("tex"),
            "md": e.get("md"),
            "pdf": e.get("pdf"),
            "location_class": loc,
            "post_reframe": post_reframe,
            "anti_target_clean": anti_target_ok,
            "anti_target_offenses": offenses[:3] if offenses else [],
            "has_source": has_source,
            "size_kb": size_kb,
            "last_modified": datetime.fromtimestamp(mtime, tz=timezone.utc).isoformat() if mtime else None,
            "epistemic_tier": tier,
            "verdict": verdict,
            "notes": ov.get("notes", ""),
        })

    # Write JSON
    out_json = ROOT / "dissemination" / "papers" / "INVENTORY.json"
    out_json.parent.mkdir(parents=True, exist_ok=True)
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump({
            "generated": datetime.now(tz=timezone.utc).isoformat(),
            "reframe_date": "2026-04-19",
            "n_papers": len(rows),
            "rows": rows,
        }, f, indent=2)

    # Write Markdown view
    out_md = ROOT / "dissemination" / "papers" / "INVENTORY.md"
    write_markdown(out_md, rows)

    # Console summary
    print(f"Generated {out_json}")
    print(f"Generated {out_md}")
    print()
    print(f"Total papers: {len(rows)}")
    print()
    print("By verdict:")
    by_verdict = {}
    for r in rows:
        by_verdict[r["verdict"]] = by_verdict.get(r["verdict"], 0) + 1
    for k in ("KEEP", "REVISE", "RETIRE", "ARCHIVED"):
        print(f"  {k:10s} {by_verdict.get(k, 0):3d}")
    print()
    print("By tier:")
    by_tier = {}
    for r in rows:
        by_tier[r["epistemic_tier"]] = by_tier.get(r["epistemic_tier"], 0) + 1
    for k in sorted(by_tier):
        print(f"  Tier {k} {by_tier[k]:3d}")
    print()
    print("By location:")
    by_loc = {}
    for r in rows:
        by_loc[r["location_class"]] = by_loc.get(r["location_class"], 0) + 1
    for k in sorted(by_loc):
        print(f"  {k:15s} {by_loc[k]:3d}")
    print()
    n_dirty = sum(1 for r in rows if not r["anti_target_clean"])
    n_pre = sum(1 for r in rows if not r["post_reframe"])
    n_pdf_only = sum(1 for r in rows if not r["has_source"])
    print(f"Anti-target offenses (un-negated):  {n_dirty} papers")
    print(f"Pre-reframe (mtime < 2026-04-19):   {n_pre} papers")
    print(f"PDF-only (no .tex / .md source):    {n_pdf_only} papers")


def write_markdown(out: Path, rows: list[dict]):
    lines = []
    lines.append("# Paper Inventory — auto-generated database")
    lines.append("")
    lines.append(f"**Generated:** {datetime.now(tz=timezone.utc).strftime('%Y-%m-%d %H:%M UTC')}  ")
    lines.append(f"**Reframe date:** 2026-04-19  ")
    lines.append(f"**Total papers:** {len(rows)}")
    lines.append("")
    lines.append("**Regenerate** with `python scripts/build_paper_inventory.py`.")
    lines.append("")
    lines.append("Per-row fields: KEEP/REVISE/RETIRE/ARCHIVED, tier 1-5, post-reframe Y/N, anti-target audit Y/N, source-availability Y/N. ")
    lines.append("Tier 1 = bulletproof math theorems. Tier 5 = pre-reframe / closed-negative / PDF-only.")
    lines.append("")

    # Group by verdict
    for verdict in ("KEEP", "REVISE", "RETIRE", "ARCHIVED"):
        group = [r for r in rows if r["verdict"] == verdict]
        if not group:
            continue
        lines.append(f"## {verdict} ({len(group)})")
        lines.append("")
        lines.append("| Stem | Tier | Loc | Reframe | A-T clean | Source | Title / Notes |")
        lines.append("|---|---|---|---|---|---|---|")
        for r in sorted(group, key=lambda x: (x["epistemic_tier"], x["stem"])):
            tier = r["epistemic_tier"]
            loc = r["location_class"]
            reframe = "Y" if r["post_reframe"] else "N"
            atc = "✓" if r["anti_target_clean"] else "✗"
            src = "✓" if r["has_source"] else "PDF only"
            title = (r["title"] or r["notes"] or "—").replace("|", "\\|")[:90]
            lines.append(f"| {r['stem']} | {tier} | {loc} | {reframe} | {atc} | {src} | {title} |")
        lines.append("")

    # Anti-target offenses detail
    dirty = [r for r in rows if not r["anti_target_clean"]]
    if dirty:
        lines.append("## Anti-target offenses (papers with un-negated 'derive α' / 'no free parameters' / 'first-principles α' phrases)")
        lines.append("")
        for r in sorted(dirty, key=lambda x: x["stem"]):
            lines.append(f"### `{r['stem']}` ({r['verdict']}, tier {r['epistemic_tier']})")
            lines.append("")
            for o in r["anti_target_offenses"]:
                lines.append(f"  - `{o}`")
            lines.append("")

    out.write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    main()
