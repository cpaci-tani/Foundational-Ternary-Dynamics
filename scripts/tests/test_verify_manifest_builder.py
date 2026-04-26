"""Unit tests for build_verify_manifest — tier-assertion enforcement."""
import json
import pytest
from pathlib import Path

from scripts.proofs.build_verify_manifest import (
    build_manifest,
    TierAssertionError,
)


def _measurements(*rows):
    return {"schema_version": 1, "last_checked": "2026-04-18", "measurements": list(rows)}


def _meas(id, value=1.0, sigma=0.01, sector="qed"):
    return {
        "id": id, "quantity": id, "sector": sector,
        "value": value, "sigma": sigma, "units": "",
        "source": "test", "url": "", "date": "2026-04-18",
    }


def test_hard_row_must_list_inputs_used():
    rows = [{
        "id": "alpha_inv", "tier": "hard", "epistemic": "THEOREM",
        "question": "Does alpha follow from D=3 and varpi?",
        "ftd_value": 137.035999177,
        "formula": "7-term expansion",
        # inputs_used missing -> should fail
        "theory_ref": "docs/theory/03_derivations/DERIV_ALPHA_FROM_GSTAR.md",
    }]
    meas = _measurements(_meas("alpha_inv", 137.035999177, 2.1e-8))
    with pytest.raises(TierAssertionError, match="inputs_used"):
        build_manifest(ftd_rows=rows, measurements=meas, build_stamp={})


def test_parametric_row_must_declare_formula_source_sm():
    rows = [{
        "id": "m_higgs", "tier": "parametric", "epistemic": "PARAMETRIC",
        "question": "Is m_H consistent with the EW scale?",
        "ftd_value": 124.8,
        "formula": "(N_eff / alpha^2) * m_e",
        # formula_source missing -> should fail
        "ftd_inputs": ["alpha", "N_eff", "m_e"],
        "theory_ref": "docs/theory/05_particles/DERIV_HIGGS_MASS.md",
    }]
    meas = _measurements(_meas("m_higgs", 125.25, 0.17, sector="ew"))
    with pytest.raises(TierAssertionError, match="formula_source"):
        build_manifest(ftd_rows=rows, measurements=meas, build_stamp={})


def test_unpredicted_row_must_have_no_ftd_value():
    rows = [{
        "id": "v_us", "tier": "unpredicted", "epistemic": "OPEN",
        "question": "Does FTD predict |V_us|?",
        "ftd_value": 0.22,  # <-- illegal for unpredicted tier
    }]
    meas = _measurements(_meas("v_us", 0.2243, 0.0008, sector="flavor"))
    with pytest.raises(TierAssertionError, match="ftd_value"):
        build_manifest(ftd_rows=rows, measurements=meas, build_stamp={})


def test_valid_minimal_manifest_builds():
    rows = [
        {
            "id": "alpha_inv", "tier": "hard", "epistemic": "THEOREM",
            "question": "Does alpha follow from D=3 and varpi?",
            "ftd_value": 137.035999177,
            "formula": "7-term expansion",
            "inputs_used": ["D=3", "varpi"],
            "theory_ref": "docs/theory/03_derivations/DERIV_ALPHA_FROM_GSTAR.md",
        },
        {
            "id": "m_higgs", "tier": "parametric", "epistemic": "PARAMETRIC",
            "question": "Is m_H consistent with the EW scale?",
            "ftd_value": 124.8,
            "formula": "(N_eff / alpha^2) * m_e",
            "formula_source": "SM",
            "ftd_inputs": ["alpha", "N_eff", "m_e"],
            "theory_ref": "docs/theory/05_particles/DERIV_HIGGS_MASS.md",
        },
        {
            "id": "v_us", "tier": "unpredicted", "epistemic": "OPEN",
            "question": "Does FTD predict |V_us|?",
        },
    ]
    meas = _measurements(
        _meas("alpha_inv", 137.035999177, 2.1e-8),
        _meas("m_higgs", 125.25, 0.17, sector="ew"),
        _meas("v_us", 0.2243, 0.0008, sector="flavor"),
    )
    manifest = build_manifest(ftd_rows=rows, measurements=meas, build_stamp={"commit": "abc123"})
    assert manifest["counts"] == {"hard": 1, "parametric": 1, "unpredicted": 1}
    assert manifest["tiers"]["hard"][0]["delta_ppb"] is not None
    # parametric tier must NOT compute a pull; only relative error
    assert "pull" not in manifest["tiers"]["parametric"][0]
    assert "rel_error" in manifest["tiers"]["parametric"][0]
    # unpredicted has no delta at all
    assert manifest["tiers"]["unpredicted"][0].get("delta_ppb") is None


def test_measurement_without_matching_ftd_row_is_rejected():
    rows = []
    meas = _measurements(_meas("orphan", 1.0, 0.01))
    with pytest.raises(TierAssertionError, match="orphan"):
        build_manifest(ftd_rows=rows, measurements=meas, build_stamp={})


def test_ftd_row_without_matching_measurement_is_rejected_for_hard_tier():
    rows = [{
        "id": "alpha_inv", "tier": "hard", "epistemic": "THEOREM",
        "question": "q", "ftd_value": 1.0, "formula": "f",
        "inputs_used": ["x"], "theory_ref": "docs/...",
    }]
    meas = _measurements()  # no measurements at all
    with pytest.raises(TierAssertionError, match="alpha_inv"):
        build_manifest(ftd_rows=rows, measurements=meas, build_stamp={})
