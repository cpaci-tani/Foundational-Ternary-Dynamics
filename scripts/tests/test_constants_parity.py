"""
JS↔Python constants parity test.

Asserts that every constant introduced by the 2026-04-26 Wave-1
"surgical attack" batch in engine/web/js/constants.js has a matching
entry in scripts/constants.py (Experimental class).

Catches the failure mode that produced the audit in the first place:
JS gets an empirical PDG value updated, the Python canonical drifts,
and downstream verification scripts compare against the wrong number.

To extend: add new pairs to PARITY_PAIRS as you migrate more sites.
"""
from __future__ import annotations

import re
from pathlib import Path

import pytest


# (js_export_name, python_attr, tolerance_relative_or_None)
PARITY_PAIRS: list[tuple[str, str, float | None]] = [
    # Quark masses
    ("M_U_PHYS", "m_up", 1e-9),
    ("M_D_PHYS", "m_down", 1e-9),
    ("M_S_PHYS", "m_strange", 1e-9),
    ("M_C_PHYS", "m_charm", 1e-9),
    ("M_B_PHYS", "m_bottom", 1e-9),
    ("M_T_PHYS", "m_top", 1e-9),
    # Bosons (MeV variants — both sides should match)
    ("M_W_PHYS", "m_W_phys_mev", 1e-9),
    ("M_Z_PHYS", "m_Z_phys_mev", 1e-9),
    ("M_HIGGS_PHYS", "m_Higgs_phys_mev", 1e-9),
    # Hadrons
    ("M_LAMBDA_PHYS", "m_Lambda", 1e-9),
    ("M_XI_0_PHYS", "m_Xi_0", 1e-9),
    ("M_XI_M_PHYS", "m_Xi_minus", 1e-9),
    ("M_ETA_PHYS", "m_eta", 1e-9),
    ("M_RHO_PHYS", "m_rho", 1e-9),
    ("M_J_PSI_PHYS", "m_J_psi", 1e-9),
    ("M_UPSILON_PHYS", "m_Upsilon", 1e-9),
    # Weak / CKM
    ("V_UD", "V_ud", 1e-9),
    ("G_A", "g_A", 1e-9),
    ("F_N", "f_n", 1e-9),
    ("F_PI", "f_pi", 1e-9),
    # Conversions
    ("AMU_MEV", "amu_mev", 1e-9),
    ("HBAR_MEV_S", "hbar_mev_s", 1e-9),
    ("K_PER_EV", "k_per_ev", 1e-9),
    # Cosmic-lattice
    ("H0_LATTICE", "H0_lattice", 1e-9),
    ("M_CHANDRA_LATTICE", "M_chandra_lattice", 1e-9),
    ("M_TOV_LATTICE", "M_tov_lattice", 1e-9),
    # Theme D
    ("THOMAS_FERMI_PREFACTOR_EV", "thomas_fermi_prefactor_ev", 1e-9),
    # Neutrinos (in MeV; constants.py mirrors the values added to JS)
    ("M_NU_E_PHYS", "m_nu_e", 1e-9),
    ("M_NU_MU_PHYS", "m_nu_mu", 1e-9),
    ("M_NU_TAU_PHYS", "m_nu_tau", 1e-9),
]


def _project_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _parse_js_constant(js_text: str, name: str) -> float | None:
    """Extract a top-level `export const NAME = <numeric expression>;`.

    Only handles numeric literals (no arithmetic) — the canonical
    constants we care about for parity are PDG/empirical scalars, so
    this is sufficient. Returns None if not found.
    """
    pattern = rf"export\s+const\s+{re.escape(name)}\s*=\s*([0-9.eE+\-*\s]+);"
    m = re.search(pattern, js_text)
    if not m:
        return None
    expr = m.group(1).strip()
    # Allow simple `A * B` forms (e.g. K_PER_EV * 1e6)
    try:
        return float(eval(expr, {"__builtins__": {}}, {}))
    except Exception:
        return None


@pytest.fixture(scope="module")
def js_text() -> str:
    path = _project_root() / "engine" / "web" / "js" / "constants.js"
    return path.read_text(encoding="utf-8")


@pytest.fixture(scope="module")
def experimental():
    # Lazy import so the test module can be discovered without numpy
    # being importable, etc.
    from scripts.constants import Experimental  # type: ignore[import-not-found]
    return Experimental


@pytest.mark.parametrize("js_name,py_attr,tol", PARITY_PAIRS)
def test_js_python_constant_parity(js_text, experimental, js_name, py_attr, tol):
    js_value = _parse_js_constant(js_text, js_name)
    assert js_value is not None, (
        f"JS export `{js_name}` not found in constants.js — drift detected"
    )
    assert hasattr(experimental, py_attr), (
        f"Python `Experimental.{py_attr}` missing — JS/Python parity broken "
        f"(JS export `{js_name}` = {js_value})"
    )
    py_value = getattr(experimental, py_attr)
    if tol is None:
        assert js_value == py_value
    else:
        denom = max(abs(js_value), abs(py_value), 1e-30)
        rel_err = abs(js_value - py_value) / denom
        assert rel_err <= tol, (
            f"Constant drift: JS `{js_name}`={js_value} vs "
            f"Py `Experimental.{py_attr}`={py_value} (rel err {rel_err:.3e} > {tol:.3e})"
        )


def test_parity_pairs_are_unique():
    """Each JS name and each Python attr appears at most once."""
    js_names = [p[0] for p in PARITY_PAIRS]
    py_attrs = [p[1] for p in PARITY_PAIRS]
    assert len(set(js_names)) == len(js_names), "duplicate JS names in PARITY_PAIRS"
    assert len(set(py_attrs)) == len(py_attrs), "duplicate Python attrs in PARITY_PAIRS"
