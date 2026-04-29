"""
Render docs/theory/01_reference/dimensional_map.json → SPEC_DIMENSIONAL_MAP.md

The JSON is the canonical source of truth. This script is the deterministic
renderer that produces the human-readable Markdown reference. Re-run after any
JSON edit to keep the Markdown in sync.

Validates the JSON against category invariants before rendering. Aborts with a
clear error message if any invariant is violated.

Usage:
    python scripts/proofs/build_dimensional_map.py            # regenerate
    python scripts/proofs/build_dimensional_map.py --check    # diff-only mode

Exit codes:
    0   regenerated successfully (or --check matched committed file)
    1   JSON validation failed
    2   --check found drift (Markdown out of sync with JSON)
"""

from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
JSON_PATH = ROOT / "docs" / "theory" / "01_reference" / "dimensional_map.json"
MD_PATH = ROOT / "docs" / "theory" / "01_reference" / "SPEC_DIMENSIONAL_MAP.md"


# ── Validation ─────────────────────────────────────────────────────────────


CATEGORY_INVARIANTS = {
    "spine_theorem": {
        "is_dimensionless": True,
        "calibration_required": False,
        "epistemic_tag": ("THEOREM",),
        "lab_measurement_required": False,
    },
    "dimensionless_prediction": {
        "is_dimensionless": True,
        "calibration_required": False,
        "epistemic_tag": None,  # any tag allowed
        "lab_measurement_required": False,
    },
    "calibration_declaration": {
        "is_dimensionless": False,
        "calibration_required": True,
        "epistemic_tag": ("CALIBRATION", "IMPOSED"),
        "lab_measurement_required": False,
    },
    "calibration_application": {
        "is_dimensionless": False,
        "calibration_required": True,
        "epistemic_tag": None,
        "lab_measurement_required": False,
        "depends_on_required": True,
    },
}


def validate(data: dict) -> None:
    """Check JSON against category invariants. Raise ValueError on first failure."""
    errors: list[str] = []
    seen_ids: set[str] = set()
    for entry in data["entries"]:
        eid = entry.get("id", "<missing-id>")
        if eid in seen_ids:
            errors.append(f"duplicate id: {eid}")
        seen_ids.add(eid)

        cat = entry.get("category")
        if cat not in CATEGORY_INVARIANTS:
            errors.append(f"{eid}: unknown category {cat!r}")
            continue
        inv = CATEGORY_INVARIANTS[cat]

        if entry.get("is_dimensionless") != inv["is_dimensionless"]:
            errors.append(
                f"{eid}: category={cat} requires is_dimensionless={inv['is_dimensionless']}"
            )
        if entry.get("calibration_required") != inv["calibration_required"]:
            errors.append(
                f"{eid}: category={cat} requires calibration_required={inv['calibration_required']}"
            )
        tag_constraint = inv["epistemic_tag"]
        if tag_constraint is not None and entry.get("epistemic_tag") not in tag_constraint:
            errors.append(
                f"{eid}: category={cat} requires epistemic_tag in {tag_constraint}, got {entry.get('epistemic_tag')!r}"
            )
        if inv.get("depends_on_required") and not entry.get("depends_on"):
            errors.append(
                f"{eid}: category={cat} requires non-empty depends_on"
            )

    # Cross-reference resolution within depends_on
    for entry in data["entries"]:
        for dep in entry.get("depends_on", []) or []:
            if dep not in seen_ids:
                errors.append(f"{entry.get('id')}: depends_on={dep!r} does not match any entry id")

    if errors:
        raise ValueError(
            f"dimensional_map.json validation failed ({len(errors)} errors):\n"
            + "\n".join(f"  - {e}" for e in errors)
        )


# ── Rendering ──────────────────────────────────────────────────────────────


HEADER_BANNER = """<!--
  AUTO-GENERATED — DO NOT HAND-EDIT.
  Source: docs/theory/01_reference/dimensional_map.json
  Renderer: scripts/proofs/build_dimensional_map.py
  To update: edit the JSON, then run `python scripts/proofs/build_dimensional_map.py`.
-->
"""


def fmt_value(entry: dict) -> str:
    """Render the FTD value field (handles None, ftd_value_aux, units)."""
    val = entry.get("ftd_value")
    aux = entry.get("ftd_value_aux")
    units = entry.get("ftd_value_units")
    if aux:
        parts = [f"{k} = {v}" for k, v in aux.items()]
        return ", ".join(parts)
    if val is None:
        return "—"
    val_str = f"{val:.10g}"
    if units:
        val_str += f" {units}"
    return val_str


def fmt_lab(entry: dict) -> str:
    """Render the lab measurement value with sigma + source if present."""
    lm = entry.get("lab_measurement")
    if not lm:
        return "—"
    if "value_note" in lm:
        return f"{lm['value_note']} ({lm.get('source','')})"
    val = lm.get("value")
    sig = lm.get("sigma")
    units = lm.get("units")
    src = lm.get("source", "")
    s = f"{val:.10g}" if isinstance(val, (int, float)) else str(val)
    if sig:
        s += f" ± {sig:.2g}"
    if units:
        s += f" {units}"
    if src:
        s += f" ({src})"
    return s


def fmt_comparison(entry: dict) -> str:
    cmp = entry.get("comparison")
    if not cmp:
        return "—"
    parts = []
    if cmp.get("delta_ppb") is not None:
        parts.append(f"Δ = {cmp['delta_ppb']:.3g} ppb")
    if cmp.get("pull_sigma") is not None:
        parts.append(f"pull = {cmp['pull_sigma']:.3g} σ")
    if cmp.get("tier"):
        parts.append(f"tier: {cmp['tier']}")
    return "; ".join(parts) if parts else "—"


def fmt_ledger(ids: list[str]) -> str:
    return ", ".join(f"[{i}](../07_assessment/LEDGER.md#{i.lower()})" for i in (ids or []))


def fmt_sources(paths: list[str]) -> str:
    if not paths:
        return "—"
    return "; ".join(f"`{p}`" for p in paths)


def render(data: dict) -> str:
    """Render the JSON to a Markdown string. Deterministic and idempotent."""
    lines: list[str] = []
    lines.append(HEADER_BANNER)
    lines.append("# FTD Dimensionless ↔ Dimensional Map")
    lines.append("")
    lines.append(
        f"**Schema:** v{data['schema_version']} · "
        f"**Scope:** {data['scope']} · "
        f"**FTD version:** {data.get('ftd_version','')} · "
        f"**Generated:** {data.get('build_stamp', {}).get('date', '')}"
    )
    lines.append("")
    lines.append(f"> {data['scope_note']}")
    lines.append("")

    # ── §1. Why three layers ──
    lines.append("## §1 · Why three layers")
    lines.append("")
    lines.append(
        "FTD's predictions sit in three distinct epistemic layers. "
        "The boundaries between them are **theorem-enforced**, not stylistic:"
    )
    lines.append("")
    lines.append(
        "1. **Dimensionless layer** — pure-number theorems derivable from D=3 + "
        "varpi without any physical-unit calibration. The algebraic spine (G\\*, "
        "master quadratic, Watson, |Aut(E)|², CM uniqueness, Phase G, Phase J) "
        "lives entirely here."
    )
    lines.append(
        "2. **Calibration layer** — declared (not derived) anchors that map "
        "lattice units to physical units. **Exactly two** SI-dimensional "
        "calibrations are theorem-enforced as the irreducible minimum: "
        "`a_phys ≡ ℓ_P` (length) and `K_B = m_e` (mass). The no-go theorems "
        "FTD-0059 (length) and FTD-0096 (mass) close all four mechanism "
        "candidates (α/β/γ/δ); these calibrations are theorem-enforced *by "
        "exclusion*, not convenience."
    )
    lines.append(
        "3. **Dimensional layer** — physical-unit predictions reachable only "
        "after passing through the calibration. Every dimensional FTD value "
        "(m_e in MeV, lifetimes in seconds, lengths in metres) is a "
        "dimensionless ratio multiplied by one of the two calibration anchors."
    )
    lines.append("")

    # ── §2. Spine theorems ──
    lines.append("## §2 · Spine theorems")
    lines.append("")
    lines.append(
        "Pure-mathematics theorems. No physical-unit content; falsifiable on "
        "their algebraic claims alone. Tagged `[THEOREM]`."
    )
    lines.append("")
    lines.append("| ID | Theorem | Formula | Value | LEDGER |")
    lines.append("|---|---|---|---:|---|")
    for e in data["entries"]:
        if e["category"] != "spine_theorem":
            continue
        lines.append(
            f"| `{e['id']}` | {e['label']} | {e['ftd_formula']} | "
            f"{fmt_value(e)} | {fmt_ledger(e.get('ledger_ids', []))} |"
        )
    lines.append("")

    for e in data["entries"]:
        if e["category"] != "spine_theorem":
            continue
        lines.append(f"### {e['label']} (`{e['id']}`)")
        lines.append("")
        lines.append(f"- **Formula:** {e['ftd_formula']}")
        if e.get("ftd_value") is not None or e.get("ftd_value_aux"):
            lines.append(f"- **Value:** {fmt_value(e)}")
        if e.get("depends_on"):
            lines.append(f"- **Depends on:** " + ", ".join(f"`{d}`" for d in e["depends_on"]))
        lines.append(f"- **LEDGER:** {fmt_ledger(e.get('ledger_ids', []))}")
        lines.append(f"- **Sources:** {fmt_sources(e.get('source_files', []))}")
        if e.get("notes"):
            lines.append(f"- **Notes:** {e['notes']}")
        lines.append("")

    # ── §3. Dimensionless predictions ──
    lines.append("## §3 · Dimensionless predictions")
    lines.append("")
    lines.append(
        "Dimensionless quantities FTD predicts and that have direct experimental "
        "analogues. No calibration enters; comparison to lab is direct."
    )
    lines.append("")
    lines.append("| ID | Quantity | FTD value | Lab measurement | Comparison | Tag | LEDGER |")
    lines.append("|---|---|---:|---|---|---|---|")
    for e in data["entries"]:
        if e["category"] != "dimensionless_prediction":
            continue
        lines.append(
            f"| `{e['id']}` | {e['label']} | {fmt_value(e)} | "
            f"{fmt_lab(e)} | {fmt_comparison(e)} | {e['epistemic_tag']} | "
            f"{fmt_ledger(e.get('ledger_ids', []))} |"
        )
    lines.append("")

    for e in data["entries"]:
        if e["category"] != "dimensionless_prediction":
            continue
        lines.append(f"### {e['label']} (`{e['id']}`)")
        lines.append("")
        lines.append(f"- **Formula:** {e['ftd_formula']}")
        lines.append(f"- **FTD value:** {fmt_value(e)}")
        lines.append(f"- **Lab:** {fmt_lab(e)}")
        lines.append(f"- **Comparison:** {fmt_comparison(e)}")
        lines.append(f"- **Tag:** `{e['epistemic_tag']}`")
        if e.get("depends_on"):
            lines.append(f"- **Depends on:** " + ", ".join(f"`{d}`" for d in e["depends_on"]))
        lines.append(f"- **LEDGER:** {fmt_ledger(e.get('ledger_ids', []))}")
        lines.append(f"- **Sources:** {fmt_sources(e.get('source_files', []))}")
        if e.get("notes"):
            lines.append(f"- **Notes:** {e['notes']}")
        lines.append("")

    # ── §4. Calibration declarations ──
    lines.append("## §4 · Calibration declarations")
    lines.append("")
    lines.append(
        "Two SI-dimensional calibrations are theorem-enforced as the irreducible "
        "minimum (`FTD-0059` for length, `FTD-0096` for mass; calibration-interface "
        "theorem in the latter). Time follows from length + the cubic-lattice CFL "
        "constraint."
    )
    lines.append("")
    lines.append("| ID | Anchor | Formula | Value | Tag | LEDGER |")
    lines.append("|---|---|---|---|---|---|")
    for e in data["entries"]:
        if e["category"] != "calibration_declaration":
            continue
        lines.append(
            f"| `{e['id']}` | {e['label']} | {e['ftd_formula']} | "
            f"{fmt_value(e)} | {e['epistemic_tag']} | "
            f"{fmt_ledger(e.get('ledger_ids', []))} |"
        )
    lines.append("")

    for e in data["entries"]:
        if e["category"] != "calibration_declaration":
            continue
        lines.append(f"### {e['label']} (`{e['id']}`)")
        lines.append("")
        lines.append(f"- **Formula:** {e['ftd_formula']}")
        lines.append(f"- **Value:** {fmt_value(e)}")
        lines.append(f"- **Tag:** `{e['epistemic_tag']}`")
        if e.get("depends_on"):
            lines.append(f"- **Depends on:** " + ", ".join(f"`{d}`" for d in e["depends_on"]))
        lines.append(f"- **LEDGER:** {fmt_ledger(e.get('ledger_ids', []))}")
        lines.append(f"- **Sources:** {fmt_sources(e.get('source_files', []))}")
        if e.get("calibration_note"):
            lines.append(f"- **Calibration note:** {e['calibration_note']}")
        if e.get("notes"):
            lines.append(f"- **Notes:** {e['notes']}")
        lines.append("")

    # ── §5. Calibration applications (worked example) ──
    lines.append("## §5 · Calibration applications (worked example)")
    lines.append("")
    lines.append(
        "How a dimensional FTD prediction is reached by composing a dimensionless "
        "ratio with a calibration anchor. Only m_e is worked here as an exemplar; "
        "every other dimensional consequence (m_μ in MeV, m_p in MeV, lifetimes "
        "in seconds, lengths in metres) follows the same pattern and is enumerated "
        "individually in `CATALOG_PARAMETRIC_INSERTIONS.md`."
    )
    lines.append("")
    for e in data["entries"]:
        if e["category"] != "calibration_application":
            continue
        lines.append(f"### {e['label']} (`{e['id']}`)")
        lines.append("")
        lines.append(f"- **Formula:** {e['ftd_formula']}")
        lines.append(f"- **FTD value:** {fmt_value(e)}")
        lines.append(f"- **Lab:** {fmt_lab(e)}")
        lines.append(f"- **Comparison:** {fmt_comparison(e)}")
        lines.append(f"- **Tag:** `{e['epistemic_tag']}`")
        if e.get("depends_on"):
            lines.append(f"- **Depends on:** " + ", ".join(f"`{d}`" for d in e["depends_on"]))
        lines.append(f"- **LEDGER:** {fmt_ledger(e.get('ledger_ids', []))}")
        lines.append(f"- **Sources:** {fmt_sources(e.get('source_files', []))}")
        if e.get("calibration_note"):
            lines.append(f"- **Calibration note:** {e['calibration_note']}")
        if e.get("notes"):
            lines.append(f"- **Notes:** {e['notes']}")
        lines.append("")

    # ── §6. Cross-reference summary ──
    lines.append("## §6 · Cross-reference summary")
    lines.append("")
    lines.append(
        "LEDGER ids touched by this map, with the entries that reference them. "
        "Use this section to find the map entry for a given LEDGER row, or vice versa."
    )
    lines.append("")
    ledger_index: dict[str, list[str]] = {}
    for e in data["entries"]:
        for lid in e.get("ledger_ids", []) or []:
            ledger_index.setdefault(lid, []).append(e["id"])
    lines.append("| LEDGER id | Map entries |")
    lines.append("|---|---|")
    for lid in sorted(ledger_index):
        eids = ", ".join(f"`{x}`" for x in sorted(ledger_index[lid]))
        lines.append(f"| {lid} | {eids} |")
    lines.append("")

    lines.append("## §7 · Editing this map")
    lines.append("")
    lines.append(
        "1. Edit `docs/theory/01_reference/dimensional_map.json` (the canonical "
        "data file)."
    )
    lines.append(
        "2. Run `python scripts/proofs/build_dimensional_map.py` to regenerate "
        "this Markdown."
    )
    lines.append(
        "3. Run `pytest scripts/tests/test_dimensional_map.py -v` to verify "
        "schema + cross-references + value agreement against `scripts/constants.py`."
    )
    lines.append("")
    lines.append(
        "Drift detection: the renderer is deterministic. CI can run "
        "`build_dimensional_map.py --check` to confirm the committed Markdown "
        "matches what the JSON would produce."
    )
    lines.append("")

    return "\n".join(lines)


# ── Entrypoint ─────────────────────────────────────────────────────────────


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="Diff-only: render in memory and compare to committed Markdown; "
        "exit 2 if drift detected.",
    )
    args = parser.parse_args(argv)

    with open(JSON_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    try:
        validate(data)
    except ValueError as e:
        print(str(e), file=sys.stderr)
        return 1

    rendered = render(data)

    if args.check:
        if not MD_PATH.exists():
            print(f"--check: {MD_PATH} does not exist; run without --check to create it", file=sys.stderr)
            return 2
        committed = MD_PATH.read_text(encoding="utf-8")
        if committed != rendered:
            print(
                f"--check: drift detected. {MD_PATH} is out of sync with "
                f"{JSON_PATH}. Re-run `python scripts/proofs/build_dimensional_map.py` "
                f"to regenerate.",
                file=sys.stderr,
            )
            return 2
        print(f"--check: {MD_PATH} matches the JSON-rendered output (no drift).")
        return 0

    MD_PATH.write_text(rendered, encoding="utf-8")
    print(f"Wrote {MD_PATH} ({len(data['entries'])} entries, {len(rendered.splitlines())} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
