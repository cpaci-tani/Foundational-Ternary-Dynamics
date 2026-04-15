"""
FTD Ultimate Verification -- Shared Test Utilities

Single source of truth for experimental values, FTD computations,
scoring functions, and verdict generation.
"""

import sys
import os as _os
sys.path.insert(0, _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), '..', '..'))
from constants import N_c, N_base, b_3, N_eff, D_CONSTRAINT as _D_CONSTRAINT

from dataclasses import dataclass, field
from typing import Dict, List, Tuple

# =============================================================================
# FRAMEWORK INTEGERS (imported from constants.py)
# =============================================================================

D = _D_CONSTRAINT  # = N_c * N_base**2 - 1 = 47

# =============================================================================
# CODATA 2022 / PDG 2024 EXPERIMENTAL VALUES
# =============================================================================


@dataclass
class ExpValue:
    """An experimental measurement with uncertainty."""

    value: float
    uncertainty: float
    unit: str
    source: str


CODATA = {
    "alpha_inv": ExpValue(137.035999177, 0.000000021, "", "CODATA 2022"),
    "alpha_s_MZ": ExpValue(0.1179, 0.0009, "", "PDG 2024"),
    "sin2_theta_w": ExpValue(0.23122, 0.00003, "", "PDG 2024"),
    # Lepton masses (MeV)
    "m_electron": ExpValue(0.51099895, 0.00000015, "MeV", "CODATA 2022"),
    "m_muon": ExpValue(105.6583755, 0.0000023, "MeV", "PDG 2024"),
    "m_tau": ExpValue(1776.86, 0.12, "MeV", "PDG 2024"),
    # Mass ratios
    "m_mu_over_m_e": ExpValue(206.7682830, 0.0000046, "", "CODATA 2022"),
    "m_tau_over_m_e": ExpValue(3477.23, 0.23, "", "PDG 2024"),
    "m_p_over_m_e": ExpValue(1836.15267343, 0.00000011, "", "CODATA 2022"),
    # Hadron masses (MeV)
    "m_proton": ExpValue(938.27208816, 0.00000029, "MeV", "CODATA 2022"),
    "m_neutron": ExpValue(939.56542052, 0.00000054, "MeV", "CODATA 2022"),
    # Boson masses (GeV)
    "m_W": ExpValue(80.3692, 0.0133, "GeV", "PDG 2024"),
    "m_Z": ExpValue(91.1876, 0.0021, "GeV", "PDG 2024"),
    "m_Higgs": ExpValue(125.25, 0.17, "GeV", "PDG 2024"),
    "v_Higgs": ExpValue(246.22, 0.01, "GeV", "PDG 2024"),
    # CKM phase
    "delta_CKM": ExpValue(68.0, 2.0, "degrees", "PDG 2024"),
    # PMNS angles
    "sin2_theta12_PMNS": ExpValue(0.304, 0.013, "", "PDG 2024"),
    "sin2_theta23_PMNS": ExpValue(0.573, 0.025, "", "PDG 2024"),
    "sin2_theta13_PMNS": ExpValue(0.02220, 0.00068, "", "PDG 2024"),
    # Neutrino mass splitting ratio
    "delta_m2_ratio": ExpValue(33.41, 0.75, "", "PDG 2024"),  # dm2_31/dm2_21
    # Cosmological
    "n_s": ExpValue(0.9649, 0.0042, "", "Planck 2018"),
    # Gravitational hierarchy
    "alpha_G": ExpValue(5.906e-39, 0.001e-39, "", "Derived from G_N"),
    # Planck mass (GeV)
    "M_Planck": ExpValue(1.220890e19, 0.000014e19, "GeV", "CODATA 2022"),
}

# =============================================================================
# ERROR COMPUTATION
# =============================================================================


def percent_error(derived: float, experimental: float) -> float:
    """Percent error."""
    return abs(derived - experimental) / abs(experimental) * 100


def ppm_error(derived: float, experimental: float) -> float:
    """Parts per million error."""
    return abs(derived - experimental) / abs(experimental) * 1e6


def ppt_error(derived: float, experimental: float) -> float:
    """Parts per trillion error."""
    return abs(derived - experimental) / abs(experimental) * 1e12


def sigma_deviation(derived: float, exp: ExpValue) -> float:
    """Number of sigma from experimental value."""
    if exp.uncertainty == 0:
        return float("inf") if derived != exp.value else 0.0
    return abs(derived - exp.value) / exp.uncertainty


# =============================================================================
# PREDICTION REGISTRY
# =============================================================================


@dataclass
class Prediction:
    """A single FTD prediction to be tested."""

    name: str
    ftd_value: float
    exp_key: str  # key into CODATA dict
    formula: str
    classification: str  # GENUINE, PARAMETRIC, NUMEROLOGICAL
    notes: str = ""

    @property
    def exp(self) -> ExpValue:
        return CODATA[self.exp_key]

    @property
    def error_pct(self) -> float:
        return percent_error(self.ftd_value, self.exp.value)

    @property
    def error_ppm(self) -> float:
        return ppm_error(self.ftd_value, self.exp.value)

    @property
    def sigma(self) -> float:
        return sigma_deviation(self.ftd_value, self.exp)


# =============================================================================
# SCORING AND VERDICT
# =============================================================================

TIER_WEIGHTS = {
    "T1_math": 0.15,
    "T2_chain": 0.20,
    "T3_predictions": 0.25,
    "T4_simulation": 0.15,
    "T5_gaps": 0.15,
    "T6_novel": 0.05,
    "T7_falsification": 0.05,
}


@dataclass
class TierResult:
    """Result from a single tier of tests."""

    name: str
    score: float  # 0-100
    passed: int
    failed: int
    total: int
    details: List[str] = field(default_factory=list)
    critical_failures: List[str] = field(default_factory=list)


def compute_verdict(tier_results: Dict[str, TierResult]) -> Tuple[float, str]:
    """Compute weighted final score and verdict band."""
    total_score = 0.0
    for tier_key, weight in TIER_WEIGHTS.items():
        if tier_key in tier_results:
            total_score += weight * tier_results[tier_key].score

    if total_score >= 90:
        verdict = "STRONG"
    elif total_score >= 70:
        verdict = "PROMISING"
    elif total_score >= 50:
        verdict = "MIXED"
    elif total_score >= 30:
        verdict = "WEAK"
    else:
        verdict = "FAILED"

    return total_score, verdict


def format_verdict_report(tier_results: Dict[str, TierResult]) -> str:
    """Generate the final verdict report as a formatted string."""
    lines = []
    lines.append("=" * 72)
    lines.append("  FTD ULTIMATE VERIFICATION -- FINAL VERDICT")
    lines.append("=" * 72)
    lines.append("")

    for tier_key in TIER_WEIGHTS:
        if tier_key in tier_results:
            r = tier_results[tier_key]
            weight = TIER_WEIGHTS[tier_key]
            weighted = weight * r.score
            lines.append(
                f"  {r.name:<40} {r.passed}/{r.total} passed  "
                f"Score: {r.score:5.1f}/100  (x{weight:.2f} = {weighted:5.1f})"
            )
            for cf in r.critical_failures:
                lines.append(f"    !! CRITICAL: {cf}")

    lines.append("")
    total, verdict = compute_verdict(tier_results)
    lines.append(f"  WEIGHTED TOTAL: {total:.1f} / 100")
    lines.append(f"  VERDICT: {verdict}")
    lines.append("")

    verdict_descriptions = {
        "STRONG": "Mathematically sound, physically predictive, falsifiable",
        "PROMISING": "Core math works, physics verified, gaps identified but contained",
        "MIXED": "Interesting mathematical structure, significant physics gaps",
        "WEAK": "More numerology than derivation, critical gaps unaddressed",
        "FAILED": "Internal contradictions, circular reasoning, or unfalsifiable",
    }
    lines.append(f"  -> {verdict_descriptions[verdict]}")
    lines.append("")
    lines.append("=" * 72)

    return "\n".join(lines)
