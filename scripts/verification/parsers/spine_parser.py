"""
P2: parser for docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md.

Extracts the 9 algebraic-spine theorems (sections §1-§9) plus the
subsidiary theorems listed at the bottom in the §13 cross-references
table.  Each theorem is normalised to:

    {
      "id":              "T1",
      "name":            "G* algebraic identity",
      "tag":             "THEOREM" or "NUMERICAL_FACT" or "THEOREM_AT_L=2",
      "ledger_ref":      "FTD-0002",
      "primary_doc":     "MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md",
      "verifier_script": "proof_motivic_master_quadratic.py",
      "honest_tier":     "theorem-grade" | "honestly-tiered"
    }

Subsidiary theorems are tagged with `id = "S1"`, `"S2"`, ... so the
node-map can distinguish primary spine theorems (T1-T9) from
subsidiaries (D=3, Moore integers, a_phys no-go, Phase H scaling).
"""
from __future__ import annotations

import re
from pathlib import Path

# The §0 paragraph identifies which 6 of 9 are theorem-grade vs which 3 are
# honestly tiered below theorem grade.
THEOREM_GRADE = {"T1", "T2", "T5", "T6", "T8", "T9"}  # per §0
HONESTLY_TIERED = {"T3", "T4", "T7"}                  # per §0

# Manual lookup of honest-tier tag from §N section headers
# (Theorem 3 is [NUMERICAL FACT], Theorem 4 is value-level/structural-conjecture,
# Theorem 7 is [THEOREM at L=2] + [DISCONFIRMED for general L] after Session A1.)
HONEST_TAGS = {
    "T1": "THEOREM",
    "T2": "THEOREM",
    "T3": "NUMERICAL_FACT",
    "T4": "VALUE_LEVEL_IDENTITY",
    "T5": "THEOREM",
    "T6": "THEOREM",
    "T7": "THEOREM_AT_L_2_DISCONFIRMED_GENERAL_L",
    "T8": "THEOREM",
    "T9": "THEOREM",
}


_THEOREM_HEADER = re.compile(
    r"^##\s+(\d+)\s+·\s+Theorem\s+\d+\s+[—\-]\s+(.+?)$",
    re.MULTILINE,
)
_XREF_ROW = re.compile(
    r"^\|\s*(\d+)\s+(.+?)\s*\|\s*`?([A-Z_0-9]+\.md)`?\s*\|\s*(.+?)\s*\|\s*(.+?)\s*\|$"
)
# Subsidiary rows: tolerant cell splitter (handles unicode like ℓ + § suffixes).
_FTD_REF = re.compile(r"\b(FTD-\d{4})\b")
_MD_FILE = re.compile(r"`?([A-Z][A-Z_0-9]*\.md)`?")

# Theorems 8 + 9 are spine-grade per §0 but lack §13 cross-ref-table rows.
# We synthesise them from their §N section content (filed below).
T8_T9_MANUAL = [
    {
        "id": "T8", "name": "Harmonic invariant of the master-quadratic tower (1/y+ + 1/y- = 1)",
        "tag": "THEOREM", "honest_tier": "theorem-grade",
        "ledger_ref": "FTD-0111", "ledger_cell_raw": "FTD-0111 (T8 manual entry)",
        "primary_doc": "DERIV_LEMNISCATIC_TOWER_HARMONIC.md",
        "verifier_script": "proof_tower_harmonic_invariant.py",
        "verifier_notes": None, "kind": "spine",
    },
    {
        "id": "T9", "name": "Field-theoretic characterization of Q(G*)",
        "tag": "THEOREM", "honest_tier": "theorem-grade",
        "ledger_ref": "FTD-0112", "ledger_cell_raw": "FTD-0112 (T9 manual entry)",
        "primary_doc": "DERIV_QGSTAR_FIELD_THEORETIC.md",
        "verifier_script": "proof_field_theoretic_qgstar.py",
        "verifier_notes": None, "kind": "spine",
    },
]


def _parse_pipe_row(line: str) -> list[str] | None:
    """Split a markdown table row into cells.  None if not a table row."""
    s = line.strip()
    if not s.startswith("|") or not s.endswith("|"):
        return None
    inner = s[1:-1]
    return [c.strip() for c in inner.split("|")]


def parse_spine(spine_path: Path) -> list[dict]:
    """Parse SPEC_ALGEBRAIC_SPINE.md into a list of theorem dicts."""
    text = spine_path.read_text(encoding="utf-8")
    lines = text.splitlines()

    # Step 1: collect theorem section names from §1-§9 headers.
    section_names: dict[str, str] = {}
    for m in _THEOREM_HEADER.finditer(text):
        n = m.group(1)
        name = m.group(2).strip()
        section_names[f"T{n}"] = name

    # Step 2: parse the §13 cross-references table.
    # Find the §13 heading first, then walk rows from there.
    cross_ref_lines: list[str] = []
    in_table = False
    for line in lines:
        if line.startswith("## 13") or line.startswith("## 13 "):
            in_table = True
            continue
        if in_table:
            if line.startswith("## "):  # next section
                break
            cross_ref_lines.append(line)

    theorems: list[dict] = []
    seen: set[str] = set()

    for line in cross_ref_lines:
        m = _XREF_ROW.match(line)
        if not m:
            continue
        n, name, primary_doc, verifier_script, ledger_cell = m.groups()
        theorem_id = f"T{n}"
        if theorem_id in seen:
            continue
        seen.add(theorem_id)

        # Extract LEDGER ref from the messy 4th cell (may contain text).
        ftd_match = _FTD_REF.search(ledger_cell)
        ledger_ref = ftd_match.group(1) if ftd_match else None

        # Clean up verifier_script (may be "(analytic)" or "(audit-derived; ...)").
        if verifier_script.startswith("(") or verifier_script.startswith("included"):
            verifier_script_clean = None
            verifier_notes = verifier_script
        else:
            # May contain "X.py + Y.py" -- keep first.
            verifier_script_clean = verifier_script.replace("`", "").strip()
            verifier_notes = None

        theorems.append({
            "id": theorem_id,
            "name": section_names.get(theorem_id, name.strip()),
            "tag": HONEST_TAGS.get(theorem_id, "THEOREM"),
            "honest_tier": ("theorem-grade" if theorem_id in THEOREM_GRADE
                            else "honestly-tiered"),
            "ledger_ref": ledger_ref,
            "ledger_cell_raw": ledger_cell.strip(),
            "primary_doc": primary_doc,
            "verifier_script": verifier_script_clean,
            "verifier_notes": verifier_notes,
            "kind": "spine",
        })

    # Step 3: append manually-curated T8 + T9 (theorem-grade per §0, but
    # absent from the §13 cross-references table).
    for entry in T8_T9_MANUAL:
        if entry["id"] not in seen:
            theorems.append(entry)
            seen.add(entry["id"])

    # Step 4: parse the subsidiary-theorems table (the second small table
    # in the same §13 region: "| Subsidiary | Primary doc | LEDGER |").
    sub_id_counter = 0
    in_sub_table = False
    for line in cross_ref_lines:
        if "| Subsidiary |" in line:
            in_sub_table = True
            continue
        if in_sub_table:
            stripped = line.strip()
            if not stripped or stripped.startswith("|---"):
                continue
            if "Empirical observation" in line:
                break
            cells = _parse_pipe_row(line)
            if not cells or len(cells) != 3:
                continue
            name, primary_doc_cell, ledger_cell = cells
            md_match = _MD_FILE.search(primary_doc_cell)
            primary_doc = md_match.group(1) if md_match else primary_doc_cell
            ftd_match = _FTD_REF.search(ledger_cell)
            sub_id_counter += 1
            theorems.append({
                "id": f"S{sub_id_counter}",
                "name": name,
                "tag": "SUBSIDIARY",
                "honest_tier": "subsidiary",
                "ledger_ref": ftd_match.group(1) if ftd_match else None,
                "ledger_cell_raw": ledger_cell,
                "primary_doc": primary_doc,
                "verifier_script": None,
                "verifier_notes": None,
                "kind": "subsidiary",
            })

    return theorems


if __name__ == "__main__":
    import io, sys
    # Force UTF-8 on Windows consoles that default to cp1252.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    from pathlib import Path as _P
    root = _P(__file__).resolve().parents[3]
    rows = parse_spine(root / "docs" / "theory" / "01_reference" / "SPEC_ALGEBRAIC_SPINE.md")
    print(f"Parsed {len(rows)} spine + subsidiary theorems.")
    for r in rows:
        print(f"  {r['id']:>4}  {r['tag']:<25}  ledger={r['ledger_ref'] or '(none)':<10}  {r['name']}")
