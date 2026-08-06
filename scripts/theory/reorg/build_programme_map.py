#!/usr/bin/env python3
"""Build an FTD-NNNN -> programme-slug lookup table from LEDGER_INDEX.md.

LEDGER_INDEX.md groups all LEDGER claims into 19 research programmes, one
"## <Programme Title>" section each, with a Markdown table of
"| `FTD-NNNN` | tag | claim | Lxxx |" rows underneath. This script parses
that structure once and emits a JSON map every other reorg script consumes,
so the programme taxonomy has exactly one source of truth: LEDGER_INDEX.md
itself.

Usage:
    python scripts/theory/reorg/build_programme_map.py [--out PATH]

Exits nonzero if a programme heading has no matching slug in SLUG_TABLE
(the fixed table in the reorg design spec) -- that is a configuration bug,
not a data problem, and should never happen silently.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
LEDGER_INDEX = REPO_ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER_INDEX.md"

# Fixed programme-title -> directory-slug table. Titles must match the
# `## ` headings in LEDGER_INDEX.md verbatim (after stripping markdown
# link/anchor syntax, if any). This table is authoritative -- see
# docs/superpowers/specs/2026-08-06-theory-docs-structural-reorg-design.md
SLUG_TABLE = {
    "Algebraic spine — master quadratic": "spine_master_quadratic",
    "Algebraic spine — G*, CM curves, modular": "spine_gstar_cm_modular",
    "Algebraic spine — periods, Watson, transcendence": "spine_periods_watson_transcendence",
    "Framework — postulates & constitution": "framework_postulates_constitution",
    "Framework — boundary, imports, consumption": "framework_boundary_imports_consumption",
    "Framework — audits, red-teams, reconciliation": "framework_audits_redteams_reconciliation",
    "Quantum foundations": "quantum_foundations",
    "SM constants — mass & flavour": "sm_constants_mass_flavour",
    "Alpha readout programme (MC-T4.3)": "alpha_readout_programme",
    "QCD, colour & electroweak": "qcd_colour_electroweak",
    "Gravity & cosmology": "gravity_cosmology",
    "Engine infrastructure & RG": "engine_infrastructure_rg",
    "Engine emergence campaigns": "engine_emergence_campaigns",
    "Lorentz recovery & causal structure": "lorentz_recovery_causal_structure",
    "Charge, Gauss & native EM emergence": "charge_gauss_native_em",
    "Common-action mechanics & reciprocity": "common_action_mechanics_reciprocity",
    "Constituent-complete matter": "constituent_complete_matter",
    "Native time & the carrier programme": "native_time_carrier_programme",
    "Meta — papers, tooling, project process": "meta_papers_tooling_process",
}

# Sections in LEDGER_INDEX.md that are not programmes and must be skipped.
NON_PROGRAMME_HEADINGS = {"Contents", "Tag frequency across all rows"}

HEADING_RE = re.compile(r"^## (.+?)\s*$", re.MULTILINE)
ROW_ID_RE = re.compile(r"^\|\s*`(FTD-\d{4})`\s*\|")


def parse_ledger_index(text: str) -> dict[str, str]:
    """Return {ftd_id: programme_title} for every claim row under every
    programme heading."""
    headings = list(HEADING_RE.finditer(text))
    id_to_programme: dict[str, str] = {}
    for i, m in enumerate(headings):
        title = m.group(1).strip()
        if title in NON_PROGRAMME_HEADINGS:
            continue
        start = m.end()
        end = headings[i + 1].start() if i + 1 < len(headings) else len(text)
        body = text[start:end]
        for line in body.splitlines():
            row = ROW_ID_RE.match(line)
            if row:
                ftd_id = row.group(1)
                if ftd_id in id_to_programme and id_to_programme[ftd_id] != title:
                    print(
                        f"WARN: {ftd_id} appears under both "
                        f"'{id_to_programme[ftd_id]}' and '{title}' -- keeping first",
                        file=sys.stderr,
                    )
                    continue
                id_to_programme[ftd_id] = title
    return id_to_programme


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--out",
        default=str(REPO_ROOT / "scripts" / "theory" / "reorg" / "programme_map.json"),
    )
    args = ap.parse_args()

    text = LEDGER_INDEX.read_text(encoding="utf-8")
    id_to_title = parse_ledger_index(text)

    unknown_titles = {t for t in id_to_title.values() if t not in SLUG_TABLE}
    if unknown_titles:
        print("FAIL: programme heading(s) with no slug in SLUG_TABLE:", file=sys.stderr)
        for t in sorted(unknown_titles):
            print(f"  - {t!r}", file=sys.stderr)
        return 1

    id_to_slug = {fid: SLUG_TABLE[title] for fid, title in id_to_title.items()}

    out_path = Path(args.out)
    out_path.write_text(
        json.dumps(
            {
                "generated_from": "docs/theory/07_assessment/core_ledgers/LEDGER_INDEX.md",
                "id_count": len(id_to_slug),
                "slug_table": SLUG_TABLE,
                "id_to_slug": dict(sorted(id_to_slug.items())),
            },
            indent=2,
            ensure_ascii=False,
        )
        + "\n",
        encoding="utf-8",
    )

    per_slug: dict[str, int] = {}
    for slug in id_to_slug.values():
        per_slug[slug] = per_slug.get(slug, 0) + 1

    print(f"Parsed {len(id_to_title)} FTD-id -> programme rows from LEDGER_INDEX.md")
    print(f"Wrote {out_path}")
    for slug, n in sorted(per_slug.items(), key=lambda kv: -kv[1]):
        print(f"  {n:4d}  {slug}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
