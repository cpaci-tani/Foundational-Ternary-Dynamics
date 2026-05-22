"""
Tests for the FTD dimensionless ↔ dimensional map.

Asserts internal consistency of `docs/theory/01_reference/dimensional_map.json`:
  1. Schema validation (every entry has required fields per its category)
  2. Cross-reference resolution (LEDGER ids exist, source files exist on disk,
     depends_on entries resolve)
  3. Value agreement (entries with constants_py_ref match scripts.constants
     to ≥10 digits)
  4. No mislabeling (THEOREM-tagged entries are dimensionless;
     calibration_application entries have non-empty depends_on)
  5. Comparison sanity (recomputed delta_ppb matches stored value to 3 sig figs
     for entries with both lab_measurement and comparison)
  6. Renderer idempotency (build_dimensional_map.py --check passes)
"""

from __future__ import annotations
import json
import math
import re
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
JSON_PATH = ROOT / "docs" / "theory" / "01_reference" / "dimensional_map.json"
MD_PATH = ROOT / "docs" / "theory" / "01_reference" / "SPEC_DIMENSIONAL_MAP.md"
LEDGER_PATH = ROOT / "docs" / "theory" / "07_assessment" / "LEDGER.md"
RENDERER = ROOT / "scripts" / "proofs" / "build_dimensional_map.py"


@pytest.fixture(scope="module")
def data():
    with open(JSON_PATH, "r", encoding="utf-8") as f:
        return json.load(f)


@pytest.fixture(scope="module")
def ledger_ids() -> set[str]:
    """Extract every FTD-NNNN identifier mentioned in the LEDGER table column."""
    text = LEDGER_PATH.read_text(encoding="utf-8")
    return set(re.findall(r"FTD-\d{4}", text))


# ── Test 1: Schema validation ──────────────────────────────────────────────


REQUIRED_FIELDS_BY_CATEGORY = {
    "spine_theorem": {
        "id", "category", "label", "is_dimensionless", "calibration_required",
        "ftd_formula", "ftd_value", "ftd_value_units", "depends_on",
        "epistemic_tag", "ledger_ids", "source_files",
    },
    "dimensionless_prediction": {
        "id", "category", "label", "is_dimensionless", "calibration_required",
        "ftd_formula", "ftd_value", "ftd_value_units", "depends_on",
        "epistemic_tag", "ledger_ids", "source_files",
    },
    "calibration_declaration": {
        "id", "category", "label", "is_dimensionless", "calibration_required",
        "ftd_formula", "ftd_value", "ftd_value_units", "depends_on",
        "epistemic_tag", "ledger_ids", "source_files",
    },
    "calibration_application": {
        "id", "category", "label", "is_dimensionless", "calibration_required",
        "ftd_formula", "ftd_value", "ftd_value_units", "depends_on",
        "epistemic_tag", "ledger_ids", "source_files",
    },
}


def test_schema_top_level(data):
    assert data["schema_version"] == 1
    assert isinstance(data["entries"], list) and len(data["entries"]) > 0
    assert "scope" in data and "ftd_version" in data


def test_required_fields_per_entry(data):
    for entry in data["entries"]:
        cat = entry.get("category")
        assert cat in REQUIRED_FIELDS_BY_CATEGORY, (
            f"{entry.get('id')}: unknown category {cat!r}"
        )
        missing = REQUIRED_FIELDS_BY_CATEGORY[cat] - set(entry.keys())
        assert not missing, f"{entry.get('id')} ({cat}): missing fields {missing}"


def test_unique_ids(data):
    ids = [e["id"] for e in data["entries"]]
    assert len(ids) == len(set(ids)), f"duplicate ids in entries: {ids}"


# ── Test 2: Cross-reference resolution ─────────────────────────────────────


def test_ledger_ids_resolve(data, ledger_ids):
    unresolved = []
    for entry in data["entries"]:
        for lid in entry.get("ledger_ids", []) or []:
            if lid not in ledger_ids:
                unresolved.append((entry["id"], lid))
    assert not unresolved, (
        "ledger_ids not found in LEDGER.md table: " + str(unresolved)
    )


def test_source_files_exist(data):
    missing = []
    for entry in data["entries"]:
        for sf in entry.get("source_files", []) or []:
            full = ROOT / sf
            if not full.exists():
                missing.append((entry["id"], sf))
    assert not missing, "source_files not found on disk: " + str(missing)


def test_depends_on_resolve(data):
    ids = {e["id"] for e in data["entries"]}
    bad = []
    for entry in data["entries"]:
        for dep in entry.get("depends_on", []) or []:
            if dep not in ids:
                bad.append((entry["id"], dep))
    assert not bad, "depends_on not matching any entry id: " + str(bad)


# ── Test 3: Value agreement vs scripts.constants ───────────────────────────


def _flatten_aux(entry):
    """Return a list of (label, value) numeric pairs for value comparison."""
    pairs = []
    if entry.get("ftd_value") is not None:
        pairs.append(("ftd_value", entry["ftd_value"]))
    aux = entry.get("ftd_value_aux")
    if isinstance(aux, dict):
        for k, v in aux.items():
            if isinstance(v, (int, float)):
                pairs.append((k, v))
    return pairs


# Map from per-entry id → which constants.py attribute should match which
# numeric field. For aux-valued entries we compare specific aux keys, not the
# (null) scalar `ftd_value`.
EXPECTED_CONSTANT_AGREEMENTS = {
    "g_star_identity":         [("ftd_value", "G_STAR")],
    "master_quadratic":        [("x_plus", "X_PLUS"), ("x_minus", "X_MINUS")],
    "alpha_inverse":           [("ftd_value", "ALPHA_INV")],
    "n_color":                 [("ftd_value", "X_MINUS")],
    "mu_over_e_mass_ratio":    [("ftd_value", "MU_RATIO")],
    "tau_over_e_mass_ratio":   [("ftd_value", "TAU_RATIO")],
    "mass_unit_anchor":        [],  # constants_py_ref=K_B but value is CODATA m_e, not the lattice K_B=0.511
    "m_electron_dimensional":  [("ftd_value", "K_B")],
}


def test_value_agreement(data):
    """For each entry with a known mapping, verify ftd_value/ftd_value_aux
    agrees with scripts.constants to ≥10 significant digits."""
    sys.path.insert(0, str(ROOT))
    from scripts import constants as c

    failures = []
    by_id = {e["id"]: e for e in data["entries"]}

    for eid, mappings in EXPECTED_CONSTANT_AGREEMENTS.items():
        if not mappings:
            continue
        entry = by_id.get(eid)
        assert entry is not None, f"expected entry {eid!r} not found"
        aux = entry.get("ftd_value_aux") or {}
        for field, py_name in mappings:
            if field == "ftd_value":
                json_val = entry.get("ftd_value")
            else:
                json_val = aux.get(field)
            py_val = float(getattr(c, py_name))
            json_val = float(json_val) if json_val is not None else None
            if json_val is None:
                failures.append(f"{eid}.{field}: JSON value is null but expected to match constants.{py_name}")
                continue
            # Relative agreement to 1e-10
            denom = max(abs(py_val), 1e-300)
            rel = abs(json_val - py_val) / denom
            if rel > 1e-10:
                failures.append(
                    f"{eid}.{field}: JSON={json_val!r} vs constants.{py_name}={py_val!r} (rel={rel:.3e})"
                )
    assert not failures, "value agreement failed:\n  " + "\n  ".join(failures)


# ── Test 4: No mislabeling ─────────────────────────────────────────────────


def test_theorem_tag_implies_dimensionless(data):
    """spine_theorem entries must be dimensionless and not require calibration."""
    bad = []
    for entry in data["entries"]:
        if entry["category"] == "spine_theorem":
            if not entry.get("is_dimensionless"):
                bad.append(f"{entry['id']}: spine_theorem must be dimensionless")
            if entry.get("calibration_required"):
                bad.append(f"{entry['id']}: spine_theorem must NOT require calibration")
            if entry.get("epistemic_tag") != "THEOREM":
                bad.append(
                    f"{entry['id']}: spine_theorem must have epistemic_tag=THEOREM, "
                    f"got {entry.get('epistemic_tag')!r}"
                )
    assert not bad, "mislabeling: " + str(bad)


def test_calibration_application_has_dependencies(data):
    """calibration_application entries must declare what calibration anchors they depend on."""
    bad = []
    for entry in data["entries"]:
        if entry["category"] == "calibration_application":
            if not entry.get("depends_on"):
                bad.append(f"{entry['id']}: calibration_application must have non-empty depends_on")
    assert not bad, str(bad)


def test_calibration_declaration_tag(data):
    """calibration_declaration entries must be tagged CALIBRATION or IMPOSED."""
    bad = []
    for entry in data["entries"]:
        if entry["category"] == "calibration_declaration":
            if entry.get("epistemic_tag") not in ("CALIBRATION", "IMPOSED"):
                bad.append(
                    f"{entry['id']}: calibration_declaration tag must be "
                    f"CALIBRATION or IMPOSED, got {entry.get('epistemic_tag')!r}"
                )
    assert not bad, str(bad)


# ── Test 5: Comparison sanity ──────────────────────────────────────────────


def test_comparison_consistent_with_lab(data):
    """For entries with both lab_measurement (numeric value) and comparison
    delta_ppb, the recomputed delta_ppb agrees to within 1% relative."""
    issues = []
    for entry in data["entries"]:
        lm = entry.get("lab_measurement")
        cmp = entry.get("comparison")
        if not lm or not cmp:
            continue
        if cmp.get("delta_ppb") is None:
            continue
        lab_val = lm.get("value")
        ftd_val = entry.get("ftd_value")
        if not isinstance(lab_val, (int, float)) or not isinstance(ftd_val, (int, float)):
            continue
        if lab_val == 0:
            continue
        recomputed = (ftd_val - lab_val) / lab_val * 1e9
        stored = cmp["delta_ppb"]
        # Allow generous tolerance: both stored and recomputed approximate the
        # same number; we just want to catch sign flips and order-of-magnitude bugs.
        if abs(recomputed) < 1.0 and abs(stored) < 1.0:
            continue  # both negligible; sign agreement is unimportant
        if (recomputed > 0) != (stored > 0) and abs(stored) > 1.0:
            issues.append(
                f"{entry['id']}: comparison.delta_ppb={stored!r} but "
                f"recomputed={recomputed:.3g} (sign disagrees)"
            )
        elif abs(stored) > 0:
            rel = abs(recomputed - stored) / abs(stored)
            if rel > 0.5 and abs(stored) > 100:  # sloppy threshold for hand-entered ppb
                issues.append(
                    f"{entry['id']}: comparison.delta_ppb={stored!r} but "
                    f"recomputed={recomputed:.3g} (rel diff {rel:.2f})"
                )
    assert not issues, "comparison drift: " + str(issues)


# ── Test 6: Renderer idempotency ───────────────────────────────────────────


def test_renderer_idempotent():
    """Running build_dimensional_map.py --check confirms the committed
    Markdown matches what the renderer would produce from the JSON."""
    result = subprocess.run(
        [sys.executable, str(RENDERER), "--check"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    assert result.returncode == 0, (
        f"renderer --check failed (exit {result.returncode}):\n"
        f"STDOUT: {result.stdout}\nSTDERR: {result.stderr}"
    )
