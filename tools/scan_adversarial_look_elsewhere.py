#!/usr/bin/env python3
"""
scan_adversarial_look_elsewhere.py -- FTD-0187 adversarial look-elsewhere scan.

PRE-REGISTRATION: docs/theory/10_eft_program/PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md
  This script must be hash-locked (git commit + tag) BEFORE it is run.
  Running it before the tag exists voids the pre-registration.

WHAT THIS TESTS
  The master quadratic  x^2 - 16 G*^2 x + 16 G*^3  has roots
  x_+ ~ 137.036 (claimed ~ 1/alpha) and x_- ~ 3.024 (claimed ~ N_c = 3).
  FTD's ~4e5:1 Bayes weight for that dual-match comes from a scan
  (proof_polynomial_look_elsewhere_extended.py) over a family every
  member of which is built on G*: every polynomial there has the form
  x^2 - c1*G*^p*x + c2*G*^q.  G* is the ONLY constant in that family.

  A physics-panel review (Pauli, Dirac) flagged that a Bayes factor
  conditioned on a family FTD designed around G* is not evidence that
  G* is special.  This scan removes that conditioning: it runs the
  IDENTICAL polynomial template over a frozen basket of 18 standard
  mathematical constants -- G* among them on identical footing -- and
  counts dual-matchers per constant.

  Outcome A (dual-match survives): the master quadratic is the unique
    (or dramatically rarest) dual-matcher across the adversarial basket.
  Outcome B (look-elsewhere artifact): dual-matchers occur across many
    constants comparably; the ~4e5:1 figure is family-conditioned.
  Outcome C (ambiguous): a small number of non-G* dual-matchers needing
    structural analysis.
  Pre-registered numeric criteria for A/B/C: see the pre-registration §5.

  The script does NOT know which constant FTD cares about; all 18 are
  scanned identically by a mechanical exhaustivity rule.

DETERMINISM: no RNG anywhere.  Iteration order is the declared order of
  the frozen lists.  Output is reproducible on any machine with the
  same Python / numpy / mpmath.

Usage:  python tools/scan_adversarial_look_elsewhere.py
  (No CLI args.  Every parameter is FROZEN below and in the pre-reg.)
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

import numpy as np
import mpmath as mp

mp.mp.dps = 50


# ============================================================
# FROZEN -- the constant basket (18 entries; G* is entry 0,
# on identical footing with the rest).
# ============================================================
def _build_constants():
    g = mp.gamma
    sqrt = mp.sqrt
    basket = [
        ("G_star",      g(mp.mpf(1) / 4) / g(mp.mpf(3) / 4)),     # Gamma(1/4)/Gamma(3/4)
        ("pi",          mp.pi),
        ("e",           mp.e),
        ("sqrt2",       sqrt(2)),
        ("sqrt3",       sqrt(3)),
        ("sqrt5",       sqrt(5)),
        ("golden_phi",  (1 + sqrt(5)) / 2),
        ("euler_gamma", mp.euler),
        ("ln2",         mp.log(2)),
        ("apery_zeta3", mp.zeta(3)),
        ("catalan",     mp.catalan),
        ("varpi_lemn",  g(mp.mpf(1) / 4) ** 2 / (2 * sqrt(2 * mp.pi))),  # lemniscate const
        ("gauss_G",     1 / mp.agm(1, sqrt(2))),
        ("sqrt_pi",     sqrt(mp.pi)),
        ("gamma_1_3",   g(mp.mpf(1) / 3)),
        ("R3_equianh",  g(mp.mpf(1) / 3) / g(mp.mpf(2) / 3)),
        ("khinchin",    mp.khinchin),
        ("glaisher",    mp.glaisher),
    ]
    return [(name, float(val)) for name, val in basket]


CONSTANTS = _build_constants()                  # FROZEN, 18 entries
N_CONSTANTS = len(CONSTANTS)
assert N_CONSTANTS == 18, f"expected 18 constants, got {N_CONSTANTS}"

# FROZEN -- the polynomial templates and coefficient grids.
# Degree-2:  x^2 - c1*K^a*x + c2*K^b      (c in [1,64], exp in [0,5])
DEG2_C = tuple(range(1, 65))
DEG2_EXP = tuple(range(0, 6))
# Degree-3:  x^3 - c1*K^a*x^2 + c2*K^b*x - c3*K^c   (c in [1,12], exp in [0,4])
DEG3_C = tuple(range(1, 13))
DEG3_EXP = tuple(range(0, 5))

# FROZEN -- physical targets (CODATA 2022 / PDG 2024; REF_EXTERNAL_CONSTANTS).
ALPHA_INV = 137.035999177       # 1/alpha
N_C = 3.0                       # colour number (exact integer)

# FROZEN -- the master quadratic, for locating it in the results.
MASTER = ("G_star", 16, 2, 16, 3)   # x^2 - 16*G*^2*x + 16*G*^3

# FROZEN -- the headline dual-matcher definition.
#   resid_plus  = |x_+ - 1/alpha| / (1/alpha)   must be < TOL_PLUS
#   resid_minus = |x_- - N_c|     / N_c          must be < TOL_MINUS
# TOL_PLUS slightly looser than the master quadratic's own 1.26 ppm so
# the master quadratic counts comfortably (not on a knife-edge);
# TOL_MINUS = 1% is the master quadratic's own x_- precision band.
TOL_PLUS = 2.0e-6
TOL_MINUS = 1.0e-2

# FROZEN -- the transparency tolerance grid for x_+ (x_- gate fixed at TOL_MINUS).
TOL_GRID_PLUS = (1e-3, 1e-4, 1e-5, 2e-6, 1e-6)

# FROZEN -- Outcome thresholds (pre-reg §5).
OUTCOME_A_MAX_NONG = 0          # A: zero non-G* dual-matchers
OUTCOME_B_MIN_NONG = 3          # B: >= 3 non-G* dual-matchers

REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = REPO_ROOT / "engine" / "results" / "adversarial_look_elsewhere_2026-05-21"


# ============================================================
# Degree-2 scan
# ============================================================
def scan_degree2():
    """Return a list of dual-matcher records and the global ranking.

    record = dict(constant, c1, a, c2, b, x_plus, x_minus,
                   resid_plus, resid_minus, degree)
    """
    matchers = []
    ranked = []                 # (resid_plus, record) for x_- within TOL_MINUS
    c_arr = np.array(DEG2_C, dtype=float)
    exp_arr = np.array(DEG2_EXP, dtype=float)
    n_c, n_e = len(DEG2_C), len(DEG2_EXP)

    for cname, K in CONSTANTS:
        Kpow = K ** exp_arr                          # K^0 .. K^5
        coeff = np.outer(c_arr, Kpow).ravel()        # n_c*n_e values: c*K^exp
        B = np.repeat(coeff, coeff.size)             # linear coeff  (-B)
        CC = np.tile(coeff, coeff.size)              # constant coeff (+CC)
        disc = B * B - 4.0 * CC
        ok = disc >= 0.0
        sq = np.sqrt(np.where(ok, disc, 0.0))
        x_plus = np.where(ok, (B + sq) / 2.0, np.nan)
        x_minus = np.where(ok, (B - sq) / 2.0, np.nan)
        rp = np.abs(x_plus - ALPHA_INV) / ALPHA_INV
        rm = np.abs(x_minus - N_C) / N_C

        gate = ok & (rm < TOL_MINUS)                 # x_- within 1% of N_c
        hit = gate & (rp < TOL_PLUS)                 # + x_+ within TOL_PLUS

        for flat in np.where(hit)[0]:
            i_b, i_cc = divmod(int(flat), coeff.size)
            c1, a = DEG2_C[i_b // n_e], DEG2_EXP[i_b % n_e]
            c2, b = DEG2_C[i_cc // n_e], DEG2_EXP[i_cc % n_e]
            matchers.append(dict(
                constant=cname, c1=c1, a=a, c2=c2, b=b, degree=2,
                x_plus=float(x_plus[flat]), x_minus=float(x_minus[flat]),
                resid_plus=float(rp[flat]), resid_minus=float(rm[flat])))

        for flat in np.where(gate)[0]:               # ranking pool
            i_b, i_cc = divmod(int(flat), coeff.size)
            c1, a = DEG2_C[i_b // n_e], DEG2_EXP[i_b % n_e]
            c2, b = DEG2_C[i_cc // n_e], DEG2_EXP[i_cc % n_e]
            ranked.append((float(rp[flat]), dict(
                constant=cname, c1=c1, a=a, c2=c2, b=b, degree=2,
                x_plus=float(x_plus[flat]), x_minus=float(x_minus[flat]),
                resid_plus=float(rp[flat]), resid_minus=float(rm[flat]))))

    ranked.sort(key=lambda t: t[0])
    return matchers, ranked


# ============================================================
# Degree-3 scan (secondary; numpy.roots, bounded ranges)
# ============================================================
def scan_degree3():
    """Cubic x^3 - c1*K^a*x^2 + c2*K^b*x - c3*K^c.  Report any cubic two
    of whose real roots dual-match (1/alpha, N_c).  G* cubics whose
    matching pair equals the master-quadratic roots are flagged as
    embeddings (master quadratic times a linear factor)."""
    matchers = []
    n_total = 0
    for cname, K in CONSTANTS:
        Kpow = [K ** e for e in DEG3_EXP]
        for c1 in DEG3_C:
            for a in DEG3_EXP:
                a2 = c1 * Kpow[a]
                for c2 in DEG3_C:
                    for b in DEG3_EXP:
                        a1 = c2 * Kpow[b]
                        for c3 in DEG3_C:
                            for c in DEG3_EXP:
                                n_total += 1
                                a0 = c3 * Kpow[c]
                                roots = np.roots([1.0, -a2, a1, -a0])
                                rr = sorted(
                                    (r.real for r in roots if abs(r.imag) < 1e-9),
                                    reverse=True)
                                for i in range(len(rr)):
                                    for j in range(i + 1, len(rr)):
                                        rp = abs(rr[i] - ALPHA_INV) / ALPHA_INV
                                        rm = abs(rr[j] - N_C) / N_C
                                        if rp < TOL_PLUS and rm < TOL_MINUS:
                                            embed = (cname == "G_star"
                                                     and abs(rr[i] - 137.0361715) < 1e-3
                                                     and abs(rr[j] - 3.0239639) < 1e-3)
                                            matchers.append(dict(
                                                constant=cname, c1=c1, a=a,
                                                c2=c2, b=b, c3=c3, c=c, degree=3,
                                                x_plus=float(rr[i]), x_minus=float(rr[j]),
                                                resid_plus=float(rp), resid_minus=float(rm),
                                                embedding=embed))
    return matchers, n_total


# ============================================================
# Analysis
# ============================================================
def per_constant_counts(matchers):
    counts = {name: 0 for name, _ in CONSTANTS}
    for m in matchers:
        counts[m["constant"]] += 1
    return counts


def classify_outcome(matchers_deg2, matchers_deg3):
    """Pre-registered Outcome classification (pre-reg §5)."""
    non_g = [m for m in matchers_deg2 if m["constant"] != "G_star"]
    # genuine (non-embedding) degree-3 matchers also count toward look-elsewhere
    non_g += [m for m in matchers_deg3
              if m["constant"] != "G_star" or not m.get("embedding", False)]
    n_nonG = len(non_g)
    distinct = sorted({m["constant"] for m in non_g})
    if n_nonG <= OUTCOME_A_MAX_NONG:
        outcome = "A"
    elif n_nonG >= OUTCOME_B_MIN_NONG or len(distinct) >= 3:
        outcome = "B"
    else:
        outcome = "C"
    return outcome, n_nonG, distinct


def main() -> int:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    n_deg2 = N_CONSTANTS * len(DEG2_C) ** 2 * len(DEG2_EXP) ** 2

    print("=" * 74)
    print("  FTD-0187  Adversarial look-elsewhere scan")
    print("=" * 74)
    print(f"  constants in basket : {N_CONSTANTS}  (G* is one of them, equal footing)")
    print(f"  degree-2 family     : {n_deg2:,} polynomials")
    print(f"  dual-matcher def    : resid_+ < {TOL_PLUS:.1e}  AND  resid_- < {TOL_MINUS:.1e}")
    print(f"  targets             : 1/alpha = {ALPHA_INV}, N_c = {N_C}")
    print("-" * 74)

    print("Scanning degree-2 family ...")
    matchers2, ranked2 = scan_degree2()
    print(f"  degree-2 dual-matchers: {len(matchers2)}")
    print("Scanning degree-3 family (secondary) ...")
    matchers3, n_deg3 = scan_degree3()
    genuine3 = [m for m in matchers3 if not m.get("embedding", False)]
    print(f"  degree-3 cubics scanned: {n_deg3:,}")
    print(f"  degree-3 dual-matchers : {len(matchers3)} "
          f"({len(matchers3) - len(genuine3)} G*-embeddings, {len(genuine3)} genuine)")
    print("-" * 74)

    counts2 = per_constant_counts(matchers2)
    print("Degree-2 dual-matchers per constant:")
    for name, _ in CONSTANTS:
        star = "  <-- master quadratic lives here" if name == "G_star" else ""
        print(f"  {name:14s}: {counts2[name]}{star}")
    print("-" * 74)

    print("Global ranking -- top 25 (x_- within 1% of N_c), ranked by x_+ residual:")
    print(f"  {'rank':>4} {'constant':14s} {'c1':>3} {'a':>2} {'c2':>3} {'b':>2}"
          f" {'resid_+':>11} {'resid_-':>10}")
    master_rank = None
    for idx, (rp, rec) in enumerate(ranked2[:25], 1):
        tag = ""
        if (rec["constant"], rec["c1"], rec["a"], rec["c2"], rec["b"]) == MASTER:
            tag = "  <== MASTER QUADRATIC"
        print(f"  {idx:>4} {rec['constant']:14s} {rec['c1']:>3} {rec['a']:>2}"
              f" {rec['c2']:>3} {rec['b']:>2} {rp:>11.3e} {rec['resid_minus']:>10.3e}{tag}")
    for idx, (rp, rec) in enumerate(ranked2, 1):
        if (rec["constant"], rec["c1"], rec["a"], rec["c2"], rec["b"]) == MASTER:
            master_rank = idx
    print(f"  master quadratic global rank: {master_rank} of {len(ranked2)} gated polynomials")
    print("-" * 74)

    # transparency grid
    print("Transparency -- degree-2 dual-matcher count vs x_+ tolerance"
          f" (x_- gate fixed < {TOL_MINUS:.0e}):")
    for tp in TOL_GRID_PLUS:
        n = sum(1 for rp, _ in ranked2 if rp < tp)
        ng = sum(1 for rp, rec in ranked2 if rp < tp and rec["constant"] != "G_star")
        print(f"  resid_+ < {tp:.0e}:  total {n:>4}   non-G* {ng:>4}")
    print("-" * 74)

    outcome, n_nonG, distinct = classify_outcome(matchers2, matchers3)
    print(f"PRE-REGISTERED OUTCOME: {outcome}")
    print(f"  non-G* dual-matchers (deg-2 + genuine deg-3): {n_nonG}")
    print(f"  distinct non-G* constants producing dual-matchers: "
          f"{len(distinct)}  {distinct}")
    if outcome == "A":
        print("  => the master quadratic stands alone across the adversarial basket.")
        print("     FTD-0013 (x_+ = 1/alpha) retains [SMC]; the Bayes basis is")
        print("     upgraded -- the dual-match survives a family FTD did not design.")
    elif outcome == "B":
        print("  => other constants reproduce the dual-match comparably.")
        print("     The dual-match is a look-elsewhere artifact; the ~4e5:1 figure")
        print("     is retracted as family-conditioned; FTD-0013 honestly demoted.")
    else:
        print("  => ambiguous: a small number of non-G* dual-matchers.")
        print("     Each requires structural analysis (pre-reg §5, Outcome C).")
    print("=" * 74)

    # artifacts
    (OUTPUT_DIR / "matchers_degree2.json").write_text(
        json.dumps(matchers2, indent=2), encoding="utf-8")
    (OUTPUT_DIR / "matchers_degree3.json").write_text(
        json.dumps(matchers3, indent=2), encoding="utf-8")
    (OUTPUT_DIR / "summary.json").write_text(json.dumps(dict(
        ledger_row="FTD-0187",
        pre_registration="PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md",
        n_constants=N_CONSTANTS,
        n_degree2=n_deg2,
        n_degree3=n_deg3,
        degree2_matchers=len(matchers2),
        degree3_matchers=len(matchers3),
        per_constant_degree2=counts2,
        master_quadratic_global_rank=master_rank,
        outcome=outcome,
        non_G_dual_matchers=n_nonG,
        distinct_non_G_constants=distinct,
    ), indent=2), encoding="utf-8")
    print(f"Artifacts written to {OUTPUT_DIR}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
