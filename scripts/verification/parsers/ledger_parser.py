"""
P1: parser for docs/theory/07_assessment/LEDGER.md.

Extracts every FTD-NNNN row in the Quick index table into a normalised
record:

    {
      "id":            "FTD-0201",
      "short_name":    "...",
      "tag_raw":       "[METHODOLOGICAL CLARIFICATION]",
      "tags":          ["METHODOLOGICAL_CLARIFICATION"],
      "primary_tag":   "METHODOLOGICAL_CLARIFICATION",
      "epistemic_color": "#00897b",
      "description":   "<column 4 of the table row, truncated to 1KB>",
      "sector":        "pure-math/G*-family",
      "primary_doc":   "<best-effort extraction from description>",
      "script_refs":   ["<py paths mentioned in description>"],
      "deps":          ["FTD-NNNN", ...]   # FTD-* references in description
    }

Used by scripts/verification/build_math_node_map.py to populate
layers.ledger of the canonical math_node_map.json output.

The parser is deliberately tolerant: cells are separated by ` | `
(space-pipe-space) which the corpus uses consistently; internal pipes
in math notation (|Aut(E)|^2, |J|^2, etc.) do not have spaces and so
do not confuse the split.
"""
from __future__ import annotations

import re
from pathlib import Path
from typing import Iterable

# ----------------------------------------------------------------------
# Tag normalisation
# ----------------------------------------------------------------------

# Map raw bracket-content strings to canonical enum names.
# The first match wins; longer / more specific patterns come first.
TAG_NORMALISATION = [
    # ---- withdrawal markers, checked FIRST (added 2026-08-04) --------------
    # These must outrank the positive tags. A tag like
    #   [RETAG - CONDITIONAL THEOREM TO SELECTION, UNEARNED]
    # contains the word THEOREM and was normalising to THEOREM — i.e. a retag
    # *away from* theorem grade was being reported as a theorem. Likewise
    #   [DERIVED — LITERAL FORM REFUTED AT I5]  ->  DERIVED.
    # In a corpus that is majority negative results, that is an overclaim
    # generator, so withdrawal always wins the first-match race.
    (re.compile(r"REFUT", re.I),                               "REFUTATION"),
    (re.compile(r"RETAG", re.I),                               "RETAG"),
    (re.compile(r"WITHDRAW", re.I),                            "WITHDRAWN"),
    (re.compile(r"SUPERSED", re.I),                            "SUPERSEDED"),
    (re.compile(r"DEMOT", re.I),                               "DEMOTED"),
    (re.compile(r"OVERCLAIM", re.I),                           "OVERCLAIM"),
    (re.compile(r"TAUTOLOG", re.I),                            "TAUTOLOGY"),
    (re.compile(r"STAGE\s+1\s+CLOSED\s+POSITIVE", re.I),       "STAGE_1_CLOSED_POSITIVE"),
    (re.compile(r"CLOSED\s+NEGATIVE", re.I),                   "CLOSED_NEGATIVE"),
    (re.compile(r"STRONGLY\s+MOTIVATED\s+CONJECTURE", re.I),   "SMC"),
    (re.compile(r"METHODOLOGICAL\s+CLARIFICATION", re.I),      "METHODOLOGICAL_CLARIFICATION"),
    (re.compile(r"METHODOLOGICAL\s+REFRAME", re.I),            "METHODOLOGICAL_REFRAME"),
    (re.compile(r"INFRASTRUCTURE", re.I),                      "INFRASTRUCTURE"),
    (re.compile(r"SCOPING\s+MEMO", re.I),                      "SCOPING_MEMO"),
    (re.compile(r"PRE[- ]?REGISTRATION", re.I),                "PRE_REGISTRATION"),
    (re.compile(r"AUDIT\s+FINDING", re.I),                     "AUDIT_FINDING"),
    (re.compile(r"NUMERICAL\s+FACT", re.I),                    "NUMERICAL_FACT"),
    (re.compile(r"CANDIDATE\s+RECON", re.I),                   "CANDIDATE_RECONSTRUCTION"),
    (re.compile(r"BRIDGE[- ]?ANALYZED", re.I),                 "BRIDGE_ANALYZED"),
    (re.compile(r"SYNTHESIS", re.I),                           "SYNTHESIS"),
    (re.compile(r"DERIVED", re.I),                             "DERIVED"),
    (re.compile(r"THEOREM", re.I),                             "THEOREM"),
    (re.compile(r"SELECTION", re.I),                           "SELECTION"),
    (re.compile(r"HYPOTHESIS", re.I),                          "HYPOTHESIS"),
    (re.compile(r"CONJECTURE", re.I),                          "CONJECTURE"),
    (re.compile(r"STRUCTURALLY\s+MOTIVATED\s+PARAMETRIC", re.I), "STRUCTURAL_PARAMETRIC"),
    (re.compile(r"PARAMETRIC", re.I),                          "PARAMETRIC"),
    (re.compile(r"DEFINITION", re.I),                          "DEFINITION"),
    (re.compile(r"OPEN", re.I),                                "OPEN"),
    (re.compile(r"AXIOM", re.I),                               "AXIOM"),
    (re.compile(r"IMPOSED", re.I),                             "IMPOSED"),
    (re.compile(r"EMERGENT", re.I),                            "EMERGENT"),
    (re.compile(r"RETRACTED", re.I),                           "RETRACTED"),
    (re.compile(r"MEASURED", re.I),                            "MEASURED"),
    (re.compile(r"PARTIAL", re.I),                             "PARTIAL"),
    (re.compile(r"STRONG\s+POSITIVE", re.I),                   "STRONG_POSITIVE"),
    (re.compile(r"POSITIVE", re.I),                            "POSITIVE"),
    # ---- vocabulary added 2026-08-04 -------------------------------------
    # These verdict words entered the corpus after the original list was
    # written and were normalising to UNKNOWN: 642 bracketed tags across 380
    # of 747 rows.  Appended (not inserted) so that first-match-wins ordering
    # leaves every previously-matched tag untouched — only brackets that
    # matched nothing before can be caught here.
    (re.compile(r"FOUNDATIONAL\s+OBSTRUCTION", re.I),          "FOUNDATIONAL_OBSTRUCTION"),
    (re.compile(r"EXECUTION\s+INVALID", re.I),                 "EXECUTION_INVALID"),
    (re.compile(r"NOT\s+EXECUTED", re.I),                      "NOT_EXECUTED"),
    (re.compile(r"SELECTED\s+DYNAMICS", re.I),                 "SELECTED_DYNAMICS"),
    (re.compile(r"ENGINE\s+FACT", re.I),                       "ENGINE_FACT"),
    (re.compile(r"NO[- ]?GO", re.I),                           "NO_GO"),
    (re.compile(r"CLOSED\s+RESOLVED", re.I),                   "CLOSED_RESOLVED"),
    (re.compile(r"UNDERDETERMINED", re.I),                     "UNDERDETERMINED"),
    (re.compile(r"INCONCLUSIVE", re.I),                        "INCONCLUSIVE"),
    (re.compile(r"RECONCILIATION|TAG[- ]?HONESTY", re.I),      "RECONCILIATION"),
    (re.compile(r"RETRACTION", re.I),                          "RETRACTED"),
    (re.compile(r"CORRECTION", re.I),                          "CORRECTION"),
    (re.compile(r"CONSTRUCTIVE", re.I),                        "CONSTRUCTIVE"),
    (re.compile(r"COHERENT[- ]?INTERPRETATION", re.I),         "COHERENT_INTERPRETATION"),
    (re.compile(r"STRUCTURAL\s+OBSERVATION|OBSERVATION", re.I), "OBSERVATION"),
    (re.compile(r"SPECULATION", re.I),                         "SPECULATION"),
    (re.compile(r"VERIFIED", re.I),                            "VERIFIED"),
    (re.compile(r"BOUNDARY", re.I),                            "BOUNDARY"),
    (re.compile(r"CONDITIONAL", re.I),                         "CONDITIONAL"),
    (re.compile(r"EXACT", re.I),                               "EXACT"),
    (re.compile(r"\bSMC\b"),                                   "SMC"),
    # Long tail. UNRESOLVED must precede RESOLVED — `RESOLVED` matches inside it.
    (re.compile(r"UNRESOLVED", re.I),                          "UNRESOLVED"),
    (re.compile(r"QUALIFIED", re.I),                           "QUALIFIED_OBSERVER"),
    (re.compile(r"INVALID", re.I),                             "INVALID"),
    (re.compile(r"ABORTED", re.I),                             "EXECUTION_ABORTED"),
    (re.compile(r"CONSUMED", re.I),                            "CONSUMED"),
    (re.compile(r"CALIBRATION", re.I),                         "CALIBRATION"),
    (re.compile(r"CHARTER|CONTRACT|\bSCOPE\b", re.I),          "SCOPE_CONTRACT"),
    (re.compile(r"FOUNDATION", re.I),                          "FOUNDATION"),
    (re.compile(r"POLICY", re.I),                              "POLICY"),
    (re.compile(r"\bSPEC\b", re.I),                            "SPEC"),
    (re.compile(r"\bBUG\b", re.I),                             "BUG"),
    (re.compile(r"NOT\s+PURSUED", re.I),                       "NOT_PURSUED"),
    (re.compile(r"RESOLVED", re.I),                            "RESOLVED"),
    (re.compile(r"STRUCTURAL", re.I),                          "STRUCTURAL"),
]

# Canonical epistemic-tag → color mapping (matches plan §Phase 1 schema).
EPISTEMIC_COLORS = {
    "THEOREM":                    "#2e7d32",
    "DERIVED":                    "#388e3c",
    "STAGE_1_CLOSED_POSITIVE":    "#2e7d32",
    "POSITIVE":                   "#43a047",
    "STRONG_POSITIVE":            "#1b5e20",
    "MEASURED":                   "#66bb6a",
    "SELECTION":                  "#fbc02d",
    "SMC":                        "#f57c00",
    "CONJECTURE":                 "#fb8c00",
    "HYPOTHESIS":                 "#ffa000",
    "PARAMETRIC":                 "#9e9e9e",
    "STRUCTURAL_PARAMETRIC":      "#bdbdbd",
    "IMPOSED":                    "#7e57c2",
    "EMERGENT":                   "#26a69a",
    "AXIOM":                      "#4527a0",
    "NUMERICAL_FACT":             "#7cb342",
    "AUDIT_FINDING":              "#1976d2",
    "PRE_REGISTRATION":           "#1976d2",
    "DEFINITION":                 "#5e35b1",
    "SYNTHESIS":                  "#00897b",
    "INFRASTRUCTURE":             "#00897b",
    "METHODOLOGICAL_CLARIFICATION":"#00838f",
    "METHODOLOGICAL_REFRAME":     "#00acc1",
    "SCOPING_MEMO":               "#26c6da",
    "BRIDGE_ANALYZED":            "#00bcd4",
    "CANDIDATE_RECONSTRUCTION":   "#7b1fa2",
    "PARTIAL":                    "#ffb74d",
    "CLOSED_NEGATIVE":            "#c62828",
    "OPEN":                       "#757575",
    "RETRACTED":                  "#424242",
    # ---- added 2026-08-04 alongside the TAG_NORMALISATION extension -------
    "FOUNDATIONAL_OBSTRUCTION":   "#b71c1c",
    "NO_GO":                      "#c62828",
    "EXECUTION_INVALID":          "#8d6e63",
    "NOT_EXECUTED":               "#a1887f",
    "UNDERDETERMINED":            "#9e9e9e",
    "INCONCLUSIVE":               "#bdbdbd",
    "BOUNDARY":                   "#ef6c00",
    "SELECTED_DYNAMICS":          "#fbc02d",
    "ENGINE_FACT":                "#66bb6a",
    "CLOSED_RESOLVED":            "#2e7d32",
    "EXACT":                      "#2e7d32",
    "VERIFIED":                   "#43a047",
    "CONSTRUCTIVE":               "#00897b",
    "CONDITIONAL":                "#ffb74d",
    "CORRECTION":                 "#1976d2",
    "RECONCILIATION":             "#1976d2",
    "OBSERVATION":                "#90a4ae",
    "COHERENT_INTERPRETATION":    "#26c6da",
    "SPECULATION":                "#ffa000",
    "REFUTATION":                 "#b71c1c",
    "RETAG":                      "#ef6c00",
    "WITHDRAWN":                  "#424242",
    "SUPERSEDED":                 "#616161",
    "DEMOTED":                    "#ef6c00",
    "OVERCLAIM":                  "#c62828",
    "TAUTOLOGY":                  "#c62828",
    "UNRESOLVED":                 "#9e9e9e",
    "QUALIFIED_OBSERVER":         "#00838f",
    "INVALID":                    "#8d6e63",
    "EXECUTION_ABORTED":          "#8d6e63",
    "CONSUMED":                   "#795548",
    "CALIBRATION":                "#7e57c2",
    "SCOPE_CONTRACT":             "#26c6da",
    "FOUNDATION":                 "#4527a0",
    "POLICY":                     "#546e7a",
    "SPEC":                       "#5e35b1",
    "BUG":                        "#1976d2",
    "NOT_PURSUED":                "#757575",
    "RESOLVED":                   "#2e7d32",
    "STRUCTURAL":                 "#00acc1",
    "UNKNOWN":                    "#bdbdbd",
}

# ----------------------------------------------------------------------
# Sector classification (keyword-driven; first match wins)
# ----------------------------------------------------------------------

# Sector classification uses simple case-insensitive substring matching.
# First match wins.  Order matters: more specific sectors come first so that
# e.g. "physics/EM-alpha" catches alpha rows before they fall through to a
# broader bucket.  All patterns are plain strings (no regex meta-chars).

SECTOR_KEYWORDS = [
    ("pure-math/structure",          ["synonymy graph", "node map", "roadmap", "infrastructure",
                                       "scoping memo", "methodological clarification",
                                       "methodological reframe"]),
    ("pure-math/master-quadratic",   ["master quadratic", "x_+", "x_-", "characteristic polynomial",
                                       "16 G", "16(G*)", "16*G", "master-quadratic"]),
    ("pure-math/modular-FQCR",       ["FQCR", "Eisenstein", "theta function", "quarter conjugacy",
                                       "quarter-conjugacy", "quarter-twisted", "(1+i)-tower",
                                       "harmonic invariant tower", "tower harmonic", "modular form"]),
    ("pure-math/Watson-Catalan",     ["Watson identity", "Watson constant", "W^(3)", "W^(4)",
                                       "W^(5)", "Catalan", "L(chi_-4, 2)"]),
    ("pure-math/CM-curves",          ["CM-curve", "CM curve", "Chowla-Selberg", "Chowla Selberg",
                                       "class number", "lemniscatic curve", "Aut(E)",
                                       "y^2 = x^3", "elliptic"]),
    ("pure-math/G*-family",          ["G* identity", "G*-identity", "G-star", "Gamma(1/4)",
                                       "Gamma(3/4)", "lemniscate", "lemniscat",
                                       "bridge constant", "G_G", "AGM", "BCC", "Q(G*)",
                                       "field-theoretic"]),
    ("physics/EM-alpha",             ["1/alpha", "fine structure", "fine-structure", "Coulomb",
                                       "Z-factor", "Z factor", "g_c", "ARC-A", "ARC-B", "ARC-C",
                                       "ARC-D", "alpha-readout", "alpha readout", "MC-T4.3",
                                       "Cherenkov", "Larmor", "Maxwell", "observable-selection",
                                       "bivector"]),
    ("physics/QCD",                  ["color charge", "QCD", "confinement", "string tension",
                                       "Wilson loop", "gluon", "N_c", "color-singlet",
                                       "color singlet", "SU(3)"]),
    ("physics/EW-Higgs",             ["Weinberg", "sin^2 theta_W", "sin2theta_W", "electroweak",
                                       "weak SU(2)", "Higgs", "weak coupling", "neutral lock"]),
    ("physics/flavor",               ["CKM", "PMNS", "mass ratio", "m_mu", "m_tau", "m_p/m_e",
                                       "neutrino", "generation graph", "flavor"]),
    ("physics/gravity",              ["gravity", "G_N", "Newton", "Schwarzschild", "graviton",
                                       "spin-2", "Einstein equations", "Frontier 4",
                                       "gravitational"]),
    ("physics/cosmology",            ["cosmolog", "Hubble", "Lambda-CDM", "dark matter",
                                       "dark energy", "inflation"]),
    ("physics/QM-foundations",       ["Born rule", "Born equilibrium", "Born-equilibrium",
                                       "wavefunction", "measurement", "DGZ", "Lindblad",
                                       "collapse", "reference frame structure", "frame-relative"]),
    ("engine-bridge",                ["lattice spacing", "a_phys", "Langevin", "Phase G",
                                       "Phase J", "Phase H", "Moore", "cluster", "FTD-0110",
                                       "engine campaign", "GPU", "L=64", "L=128",
                                       "27-block", "BCC sublattice", "nonlinear bridge"]),
]


def classify_sector(text: str) -> str:
    """First-match-wins sector classifier on the short_name + description.

    Uses case-insensitive substring matching against the SECTOR_KEYWORDS table.
    """
    t = text.lower()
    for label, patterns in SECTOR_KEYWORDS:
        for p in patterns:
            if p.lower() in t:
                return label
    return "pure-math/unclassified"


# ----------------------------------------------------------------------
# Row parsing
# ----------------------------------------------------------------------

_FTD_ROW = re.compile(r"^\|\s*(FTD-\d{4})\s*\|")
_FTD_REF = re.compile(r"\b(FTD-\d{4})\b")
_SCRIPT_REF = re.compile(r"`?(scripts/[A-Za-z0-9_/\-]+\.py)`?")
_DOC_REF = re.compile(r"`?(docs/theory/[A-Za-z0-9_/\-]+\.md)`?")
_PRIMARY_DOC = re.compile(r"`([A-Z][A-Z_0-9]+\.md)`")  # first uppercase-snake-case .md file


_RECORD_LINK = re.compile(r"\s*\[…full record →\]\(#([a-z0-9\-]+)\)\s*$")
_RECORD_BLOCK = re.compile(
    r'<a id="([a-z0-9\-]+-record)"></a>\s*\n\s*'
    r"\*\*Tag\.\*\*(.*?)\n\s*\*\*Record\.\*\*(.*?)(?=\n\s*---\s*\n)",
    re.S,
)


def load_detail_blocks(text: str) -> dict[str, tuple[str, str]]:
    """Map anchor -> (full tag text, full record text) from 'Full row records'.

    The 2026-08-04 restructure moved each oversized row's Tag and Record cells
    into a detail block, leaving a verbatim prefix plus a link in the table.
    Reading only the table would silently lose the dependency and script
    references that live in the moved text — 369 rows' `deps` and 85 rows'
    `script_refs` in the ledger as it stands.
    """
    return {m.group(1): (m.group(2).strip(), m.group(3).strip())
            for m in _RECORD_BLOCK.finditer(text)}


def parse_quick_index_row(line: str,
                          details: dict[str, tuple[str, str]] | None = None) -> dict | None:
    """Parse one Quick-index table row.  Returns None for non-rows.

    `details` is the mapping from `load_detail_blocks`.  When supplied, a row
    that was restructured is re-joined with its full text before dependencies,
    script references and the sector are extracted.  Omitting it keeps the old
    table-only behaviour, so existing callers are unaffected.
    """
    if not _FTD_ROW.match(line):
        return None

    # Strip leading/trailing whitespace + outer pipes.
    body = line.strip()
    if body.startswith("|"):
        body = body[1:]
    if body.endswith("|"):
        body = body[:-1]

    # Split on space-pipe-space (the canonical column separator in this corpus).
    # Internal pipes in math notation (|Aut(E)|^2, |J|^2, etc.) do not have
    # spaces and so do not confuse this split.
    cells = [c.strip() for c in body.split(" | ")]
    if len(cells) < 4:
        return None
    if len(cells) > 4:
        # Re-merge any over-splits into the last cell.
        cells = cells[:3] + [" | ".join(cells[3:])]

    ftd_id, short_name, tag_raw, description = cells

    # Re-join a restructured row with the full text held in its detail block.
    # The table cell keeps a verbatim prefix; the block keeps the whole record.
    link = _RECORD_LINK.search(description)
    if link:
        description = _RECORD_LINK.sub("", description)
        if details:
            full = details.get(link.group(1))
            if full:
                tag_raw, description = full

    # Tag normalisation.  The corpus has two row formats:
    #   (a) bare-tag rows  -- "| ... | THEOREM | UNAFFECTED |"
    #   (b) bracketed-tag rows -- "| ... | [CLOSED NEGATIVE] (description) | NEW ... |"
    # Format (b) is the modern style; format (a) is preserved in ~33 older rows.
    bracketed = re.findall(r"\[[^\]]+\]", tag_raw)
    candidates = bracketed if bracketed else [tag_raw]
    tags: list[str] = []
    for cand in candidates:
        norm = None
        for pat, name in TAG_NORMALISATION:
            if pat.search(cand):
                norm = name
                break
        if norm and norm not in tags:
            tags.append(norm)
    if not tags:
        tags = ["UNKNOWN"]
    primary_tag = tags[0]
    epistemic_color = EPISTEMIC_COLORS.get(primary_tag, EPISTEMIC_COLORS["UNKNOWN"])

    # Best-effort primary-doc / script-refs / deps from the description.
    desc_for_classification = (short_name + " " + tag_raw + " " + description)
    sector = classify_sector(desc_for_classification)

    deps = sorted({m for m in _FTD_REF.findall(description) if m != ftd_id})
    script_refs = sorted({m for m in _SCRIPT_REF.findall(description)})
    primary_doc_match = _DOC_REF.search(description) or _PRIMARY_DOC.search(description)
    primary_doc = primary_doc_match.group(1) if primary_doc_match else None

    # Truncate description if absurdly long -- the JSON keeps it compact.
    desc_trunc = description if len(description) <= 1000 else description[:1000] + "..."

    return {
        "id": ftd_id,
        "short_name": short_name,
        "tag_raw": tag_raw,
        "tags": tags,
        "primary_tag": primary_tag,
        "epistemic_color": epistemic_color,
        "description": desc_trunc,
        "sector": sector,
        "primary_doc": primary_doc,
        "script_refs": script_refs,
        "deps": deps,
    }


def parse_ledger(ledger_path: Path) -> list[dict]:
    """Parse the LEDGER.md Quick-index table.  Returns list of ledger-claim dicts."""
    text = ledger_path.read_text(encoding="utf-8")
    details = load_detail_blocks(text)
    rows: list[dict] = []
    seen_ids: set[str] = set()
    for line in text.splitlines():
        row = parse_quick_index_row(line, details)
        if row is None:
            continue
        if row["id"] in seen_ids:
            # Duplicate row -- skip; the Quick-index should be unique per id.
            continue
        seen_ids.add(row["id"])
        rows.append(row)
    return rows


if __name__ == "__main__":
    import json, sys
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    from pathlib import Path as _P
    root = _P(__file__).resolve().parents[3]
    rows = parse_ledger(root / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md")
    print(f"Parsed {len(rows)} LEDGER rows.")
    print(f"Primary-tag distribution:")
    from collections import Counter
    for tag, count in Counter(r["primary_tag"] for r in rows).most_common():
        print(f"  {count:4d}  {tag}")
    print(f"Sector distribution:")
    for sec, count in Counter(r["sector"] for r in rows).most_common():
        print(f"  {count:4d}  {sec}")
    print(f"Sample row (FTD-0001):")
    sample = [r for r in rows if r["id"] == "FTD-0001"]
    if sample:
        print(json.dumps(sample[0], indent=2, ensure_ascii=False)[:600])
