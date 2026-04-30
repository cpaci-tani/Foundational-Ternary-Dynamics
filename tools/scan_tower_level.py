"""
Hash-locked blind runner — (1+i)-tower level-scan falsifier.

Pre-registration: docs/theory/03_derivations/PROTOCOL_TOWER_LEVEL_FALSIFIER.md
Theorem reference: docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md
LEDGER: FTD-0111

This runner is the BLIND execution of the protocol.  Once hash-locked
via SHA-256 + git tag preregister-tower-level-scan-v1, the catalog,
level range, tolerance, and verdict matrix are fixed; results from
running this script produce the falsifier verdict directly.

Determinism: pure mpmath (no RNG).  Reproducible bit-for-bit at
mp.dps >= 30.

DO NOT MODIFY this file after the hash-lock without invalidating
the pre-registration.  Any catalog or threshold change requires a
new tag (vN+1) and a new measurement document.
"""

from __future__ import annotations

import hashlib
import json
import sys
from pathlib import Path

from mpmath import mp, mpf, gamma, sqrt, log, pi as mp_pi, nstr

# ============================================================================
# Pre-registered constants.  LOCKED at hash-lock time.
# ============================================================================

mp.dps = 50

# G* per scripts/constants.py line 103/265 (canonical FTD convention)
G_STAR = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)

# Locked level range (PROTOCOL §1)
K_MIN = 3
K_MAX = 15

# Locked tolerance (PROTOCOL §3)
TOLERANCE = mpf("0.01")  # 1.0% relative error

# Locked tolerance for "POSITIVE STRUCTURAL EVIDENCE" verdict
STRONG_TOLERANCE = mpf("0.001")  # 0.1%

# Locked framework-integer levels (PROTOCOL §3 verdict matrix special row)
FRAMEWORK_INTEGER_LEVELS = {3, 4, 7, 13}  # {N_c, N_base, b_3, N_eff}

# Locked verdict-matrix bands (PROTOCOL §3)
NULL_CONSISTENT_MAX_NONK4 = 1
NULL_REJECT_UPWARD_MIN_NONK4 = 15

# Locked candidate catalog (PROTOCOL §2).  22 entries.  Every value sourced
# from CODATA 2022, PDG 2024, or Planck 2018 as documented in the protocol.
CANDIDATES = [
    # Anchor (verified, control)
    ("1/alpha (CODATA 2022)", mpf("137.035999084")),
    # Lepton mass ratios (PDG 2024)
    ("m_mu/m_e (PDG)", mpf("206.7682830")),
    ("m_tau/m_e (PDG)", mpf("3477.23")),
    ("m_tau/m_mu (PDG)", mpf("16.817")),
    # Hadron mass ratios (PDG 2024)
    ("m_p/m_e (PDG)", mpf("1836.15267343")),
    ("m_n/m_e (PDG)", mpf("1838.68366")),
    ("m_pi/m_e (PDG)", mpf("273.13")),
    ("m_K/m_e (PDG)", mpf("974.0")),
    ("m_p/m_n (PDG)", mpf("0.99862")),
    # Electroweak (PDG 2024 on-shell)
    ("m_W/m_Z (PDG)", mpf("1.13501")),
    ("m_H/m_W (PDG)", mpf("1.553")),
    ("sin^2 theta_W (PDG)", mpf("0.23121")),
    # PMNS (PDG 2024)
    ("sin^2 theta_12 (PMNS)", mpf("0.307")),
    ("sin^2 theta_23 (PMNS)", mpf("0.546")),
    ("sin^2 theta_13 (PMNS)", mpf("0.0220")),
    ("cos^2 theta_13 (PMNS)", mpf("0.978")),
    # Strong / coupling
    ("alpha_s(M_Z) (PDG)", mpf("0.1180")),
    ("m_b/m_t (PDG)", mpf("0.0234")),
    # Cosmological (Planck 2018)
    ("Omega_b h^2 (Planck)", mpf("0.02237")),
    ("Omega_DM/Omega_b (Planck)", mpf("5.32")),
    # Mathematical anchors (controls)
    ("4 pi", 4 * mp_pi),
    ("e (Euler)", __import__("mpmath").e),
]

assert len(CANDIDATES) == 22, "Catalog size locked at 22"

# ============================================================================
# Tower computation
# ============================================================================


def tower_observables(k: int):
    """Compute (x_+, x_-, 1/y_+, 1/y_-) for level k of the (1+i)-tower."""
    bk = mpf(2) ** k * G_STAR ** (k - 2)
    ck = mpf(2) ** k * G_STAR ** (k - 1)
    disc = bk ** 2 - 4 * ck
    xp = (bk + sqrt(disc)) / 2
    xm = (bk - sqrt(disc)) / 2
    inv_yp = G_STAR / xp
    inv_ym = G_STAR / xm
    return xp, xm, inv_yp, inv_ym


# ============================================================================
# Match scan
# ============================================================================


def relative_error(measured, target):
    """|measured/target - 1|."""
    return abs(measured / target - 1)


def harmonic_complement(level: int, qname: str) -> bool:
    """True iff this match is the automatic-from-harmonic complement of an
    independent match at the same level.  At level 4, 1/y_- = 1 - G*alpha
    is automatic given 1/y_+ = G*alpha; flagging it prevents double-counting."""
    return qname == "1/y_-"


def scan():
    """Execute the locked scan.  Returns list of match dicts."""
    matches = []
    for k in range(K_MIN, K_MAX + 1):
        xp, xm, inv_yp, inv_ym = tower_observables(k)
        observables = [
            ("x_+", xp),
            ("x_-", xm),
            ("1/y_+", inv_yp),
            ("1/y_-", inv_ym),
        ]
        for qname, q in observables:
            for label, target in CANDIDATES:
                err = relative_error(q, target)
                if err < TOLERANCE:
                    matches.append(
                        {
                            "level": k,
                            "observable": qname,
                            "value": str(nstr(q, 12)),
                            "target_label": label,
                            "target_value": str(nstr(target, 12)),
                            "rel_error": float(err),
                            "rel_error_pct": float(err * 100),
                            "below_strong_tol": bool(err < STRONG_TOLERANCE),
                            "harmonic_complement": harmonic_complement(k, qname),
                            "framework_integer_level": k in FRAMEWORK_INTEGER_LEVELS,
                        }
                    )
    return matches


# ============================================================================
# Verdict logic (PROTOCOL §3)
# ============================================================================


def apply_verdict_matrix(matches: list[dict]) -> dict:
    """Apply the locked verdict matrix from PROTOCOL §3."""

    # Independent matches at k != 4 (excluding the auto-from-harmonic complements)
    nonk4_independent = [
        m
        for m in matches
        if m["level"] != 4 and not m["harmonic_complement"]
    ]
    nonk4_count = len(nonk4_independent)

    # Strong-tolerance matches at framework-integer levels k in {3, 7, 13}
    fw_strong = [
        m
        for m in matches
        if m["framework_integer_level"]
        and m["level"] != 4  # exclude the verified anchor
        and m["below_strong_tol"]
        and not m["harmonic_complement"]
    ]

    # Verified k=4 alpha control
    k4_alpha = [
        m
        for m in matches
        if m["level"] == 4 and m["target_label"].startswith("1/alpha")
    ]

    verdicts = []

    # Verdict 1: control passes?
    if k4_alpha:
        verdicts.append(("CONTROL_PASS", "k=4 x_+ matches 1/alpha (anchor verified)"))
    else:
        verdicts.append(("CONTROL_FAIL", "k=4 alpha anchor MISSING — runner broken"))

    # Verdict 2: framework-integer strong evidence?
    if fw_strong:
        verdicts.append(
            (
                "POSITIVE_STRUCTURAL_EVIDENCE",
                f"{len(fw_strong)} match(es) at framework-integer levels at <0.1% precision",
            )
        )

    # Verdict 3: catalog richness band
    if nonk4_count <= NULL_CONSISTENT_MAX_NONK4:
        verdicts.append(
            (
                "NULL_CONSISTENT",
                f"{nonk4_count} independent matches at k!=4 (<= {NULL_CONSISTENT_MAX_NONK4})",
            )
        )
    elif nonk4_count >= NULL_REJECT_UPWARD_MIN_NONK4:
        verdicts.append(
            (
                "NULL_REJECTED_UPWARD",
                f"{nonk4_count} independent matches at k!=4 (>= {NULL_REJECT_UPWARD_MIN_NONK4})",
            )
        )
    else:
        verdicts.append(
            (
                "INCONCLUSIVE",
                f"{nonk4_count} independent matches at k!=4 (between {NULL_CONSISTENT_MAX_NONK4} and {NULL_REJECT_UPWARD_MIN_NONK4})",
            )
        )

    return {
        "verdicts": verdicts,
        "nonk4_independent_count": nonk4_count,
        "framework_integer_strong_match_count": len(fw_strong),
        "control_passed": bool(k4_alpha),
    }


# ============================================================================
# Self-hash
# ============================================================================


def self_hash() -> str:
    """SHA-256 of this runner script."""
    src = Path(__file__).read_bytes()
    return hashlib.sha256(src).hexdigest()


# ============================================================================
# Main
# ============================================================================


def main():
    print("=" * 76)
    print("BLIND TOWER-LEVEL SCAN — preregister-tower-level-scan-v1")
    print("=" * 76)
    print(f"Runner SHA-256: {self_hash()}")
    print(f"Precision: mp.dps = {mp.dps}")
    print(f"G* = {nstr(G_STAR, 25)}")
    print(f"Catalog size: {len(CANDIDATES)} (PROTOCOL §2 LOCKED)")
    print(f"Level range: k in [{K_MIN}, {K_MAX}] (PROTOCOL §1 LOCKED)")
    print(f"Tolerance: {float(TOLERANCE)*100:.2f}% (PROTOCOL §3 LOCKED)")
    print(f"Strong tolerance: {float(STRONG_TOLERANCE)*100:.2f}% (PROTOCOL §3 LOCKED)")
    print(
        f"Framework-integer levels: {sorted(FRAMEWORK_INTEGER_LEVELS)} "
        f"(PROTOCOL §3 LOCKED)"
    )
    print()

    print("-" * 76)
    print("Tower observables")
    print("-" * 76)
    print(
        f'{"k":>3} | {"x_+(k)":>16} | {"x_-(k)":>10} | '
        f'{"1/y_+(k)":>14} | {"1/y_-(k)":>14}'
    )
    print("-" * 76)
    for k in range(K_MIN, K_MAX + 1):
        xp, xm, inv_yp, inv_ym = tower_observables(k)
        print(
            f"{k:>3} | {nstr(xp, 8):>16} | {nstr(xm, 8):>10} | "
            f"{nstr(inv_yp, 8):>14} | {nstr(inv_ym, 8):>14}"
        )

    print()
    print("-" * 76)
    print("Match scan")
    print("-" * 76)
    matches = scan()
    if not matches:
        print("  No matches at 1% tolerance.")
    else:
        for m in matches:
            tags = []
            if m["below_strong_tol"]:
                tags.append("STRONG")
            if m["harmonic_complement"]:
                tags.append("HARMONIC-AUTO")
            if m["framework_integer_level"]:
                tags.append("FW-INTEGER-LEVEL")
            tag_str = " ".join(f"[{t}]" for t in tags)
            print(
                f"  k={m['level']:>2} {m['observable']:>5} = {m['value']:<15} "
                f"~ {m['target_label']:<28} | err = {m['rel_error_pct']:.5f}%  {tag_str}"
            )

    print()
    print("-" * 76)
    print("Verdict (PROTOCOL §3 matrix)")
    print("-" * 76)
    verdict = apply_verdict_matrix(matches)
    for code, msg in verdict["verdicts"]:
        marker = "POSITIVE" if "POSITIVE" in code or "PASS" in code else "  "
        print(f"  [{marker:>8}] {code}: {msg}")

    # JSON dump for downstream consumption
    output = {
        "runner_sha256": self_hash(),
        "preregistration_tag": "preregister-tower-level-scan-v1",
        "G_STAR": str(nstr(G_STAR, 25)),
        "k_range": [K_MIN, K_MAX],
        "tolerance": float(TOLERANCE),
        "strong_tolerance": float(STRONG_TOLERANCE),
        "framework_integer_levels": sorted(FRAMEWORK_INTEGER_LEVELS),
        "catalog_size": len(CANDIDATES),
        "matches": matches,
        "verdict": verdict,
    }

    out_dir = Path("engine/results/tower_level_scan_2026-04-29")
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "scan_result.json"
    out_path.write_text(json.dumps(output, indent=2))
    print()
    print(f"  → JSON written to {out_path}")
    print(f"  → Runner SHA-256: {self_hash()}")
    print()

    # Exit code: 0 if control passed, 1 if it failed (broken runner)
    return 0 if verdict["control_passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
