"""FTD-0143 — FQCR quotient-uniqueness scan (pre-registered execution).

Executes PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md (git tag
`preregister-fqcr-quotient-uniqueness-v1` @ 557593e4) exactly as locked:

  search space  (k, d; l, m) in {2..8}^4 = 2401 quadruples
  N = 4096, t = 1, mpmath 50 dps
  Psi_N^{(k,d;l,m)}(1) = prod_{n=1}^{N} (1-Q^{kn})^d / (1-Q^{ln})^m,  Q = e^{-2*pi}
  A_N = (1/3) d/dt log Psi_N = (2*pi/3) * (d*k*T_k - m*l*T_l),
        T_j = sum_{n=1}^{N} n*Q^{jn} / (1 - Q^{jn})
        (reduces to SPEC_FQCR.md section 3.3's 16*pi/4*pi form at (4,6;3,2))
  R_N(1) = 1 + lambda_N(4i) + A_N   (lambda_N: truncated Jacobi theta ratio ^4)
  lambda_max = largest root of x^2 - 16*G_N*^2 x + 16*G_N*^3 R_N = 0
  G_N* = (N+1)^{-1/2} prod_{n=0}^{N} (n+3/4)/(n+1/4)      (SPEC_FQCR Prop 2)
  targets/tolerances imported from tools.scan_look_elsewhere (the locked list)
  hit = relative residual |value - target|/|target| < eps  (FTD-0097 convention)

Outputs (pre-reg section 5, locked directory name; run date lives in meta.json):
  engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/
    meta.json, all_quadruples.csv, ranking_eps_*.csv, alpha_match_quadruples.csv

ANALYSIS.md is authored separately from these artifacts.
"""

from __future__ import annotations

import csv
import hashlib
import json
import subprocess
import sys
import time
from pathlib import Path

from mpmath import mp, mpf, exp, log, sqrt, pi

sys.path.insert(0, str(Path(__file__).resolve().parent))
from scan_look_elsewhere import TARGETS, TOLERANCES  # locked lists (comment-only edits since lock)

mp.dps = 50

N = 4096
T_BASE = 1  # t = 1 per pre-reg
J_RANGE = range(2, 9)  # exponent indices {2..8}
CANONICAL = (4, 6, 3, 2)

REPO = Path(__file__).resolve().parents[1]
OUT_DIR = REPO / "engine" / "results" / "fqcr_quotient_uniqueness_2026-05-06_l_scan"


def compute_primitives():
    """Precompute Q-sums shared by all quadruples (the whole scan's cost)."""
    Q = exp(-2 * pi * T_BASE)
    T = {}  # T_j = sum n Q^{jn}/(1-Q^{jn})
    S = {}  # S_j = sum log(1 - Q^{jn})   (for log Psi artifact column)
    for j in J_RANGE:
        print(f"  precompute j={j} ...", flush=True)
        Qj = Q ** j
        acc_t = mpf(0)
        acc_s = mpf(0)
        Qjn = mpf(1)
        for n in range(1, N + 1):
            Qjn *= Qj  # = Q^{jn}
            if Qjn == 0:
                break  # underflow at 50 dps; remaining terms are exactly 0 there
            one_minus = 1 - Qjn
            acc_t += n * Qjn / one_minus
            acc_s += log(one_minus)
        T[j] = acc_t
        S[j] = acc_s

    # lambda_N(4i) = (theta2_N/theta3_N)^4 at tau = 4i, nome q = e^{i pi tau} = e^{-4 pi}
    q = exp(-4 * pi)
    th2 = mpf(0)
    th3 = mpf(1)
    for n in range(0, N):
        term = 2 * q ** ((n + mpf(1) / 2) ** 2)
        th2 += term
        if term == 0:
            break
    for n in range(1, N + 1):
        term = 2 * q ** (n * n)
        th3 += term
        if term == 0:
            break
    lam = (th2 / th3) ** 4

    # G_N* finite product (Prop 2)
    g = mpf(1)
    for n in range(0, N + 1):
        g *= (n + mpf(3) / 4) / (n + mpf(1) / 4)
    g /= sqrt(N + 1)

    return Q, T, S, lam, g


def lambda_max(gN, R):
    b = 16 * gN ** 2
    c = 16 * gN ** 3 * R
    disc = b * b - 4 * c
    return (b + sqrt(disc)) / 2


def main():
    t0 = time.time()
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    print("FTD-0143 scan: precomputing primitives (N=4096, 50 dps)...", flush=True)
    Q, T, S, lam, gN = compute_primitives()
    print(f"  Q          = {mp.nstr(Q, 12)}")
    print(f"  lambda_N(4i)= {mp.nstr(lam, 12)}")
    print(f"  G_N*       = {mp.nstr(gN, 20)}  (G* = 2.9586751191886389...)", flush=True)

    two_pi_over_3 = 2 * pi / 3
    rows = []
    n_done = 0
    for k in J_RANGE:
        for d in J_RANGE:
            for l in J_RANGE:
                for m in J_RANGE:
                    A = two_pi_over_3 * (d * k * T[k] - m * l * T[l])
                    R = 1 + lam + A
                    val = lambda_max(gN, R)
                    log_psi = d * S[k] - m * S[l]
                    residuals = {}
                    hit_counts = {f"{eps:.0e}": 0 for eps in TOLERANCES}
                    for (tname, tval, _tunc) in TARGETS:
                        rres = abs(val - mpf(repr(tval))) / abs(mpf(repr(tval)))
                        residuals[tname] = rres
                        for eps in TOLERANCES:
                            if rres < eps:
                                hit_counts[f"{eps:.0e}"] += 1
                    rows.append({
                        "k": k, "d": d, "l": l, "m": m,
                        "A_N": A, "R_N": R, "lambda_max": val, "log_psi": log_psi,
                        "hits": hit_counts, "residuals": residuals,
                    })
                    n_done += 1
                    if n_done % 500 == 0:
                        print(f"  {n_done}/2401 quadruples "
                              f"({time.time()-t0:.1f}s)", flush=True)

    print(f"scan complete: {n_done} quadruples in {time.time()-t0:.1f}s", flush=True)

    eps_keys = [f"{eps:.0e}" for eps in TOLERANCES]

    # ---- all_quadruples.csv
    with open(OUT_DIR / "all_quadruples.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["k", "d", "l", "m", "A_N", "R_N", "lambda_max", "log_psi"]
                   + [f"hits_{ek}" for ek in eps_keys]
                   + ["alpha_inv_rel_residual"])
        for r in rows:
            w.writerow([r["k"], r["d"], r["l"], r["m"],
                        mp.nstr(r["A_N"], 17), mp.nstr(r["R_N"], 25),
                        mp.nstr(r["lambda_max"], 25), mp.nstr(r["log_psi"], 17)]
                       + [r["hits"][ek] for ek in eps_keys]
                       + [mp.nstr(r["residuals"]["alpha_inv"], 10)])

    # ---- ranking_eps_*.csv (top 20 per tolerance) + criterion 3.1 (tie-inclusive)
    crit31_pass_tols = []
    canonical_rank_report = {}
    for eps, ek in zip(TOLERANCES, eps_keys):
        by_hits = sorted(rows, key=lambda r: (-r["hits"][ek], r["residuals"]["alpha_inv"]))
        with open(OUT_DIR / f"ranking_eps_{ek}.csv", "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(["rank_by_sort", "k", "d", "l", "m", f"hits_{ek}",
                        "alpha_inv_rel_residual"])
            for i, r in enumerate(by_hits[:20], 1):
                w.writerow([i, r["k"], r["d"], r["l"], r["m"], r["hits"][ek],
                            mp.nstr(r["residuals"]["alpha_inv"], 10)])
        # tie-inclusive top-3: hit-count >= 3rd-highest DISTINCT nonneg hit count
        distinct = sorted({r["hits"][ek] for r in rows}, reverse=True)
        third_highest = distinct[min(2, len(distinct) - 1)]
        canon = next(r for r in rows if (r["k"], r["d"], r["l"], r["m"]) == CANONICAL)
        tie_inclusive_top3 = canon["hits"][ek] >= third_highest and canon["hits"][ek] > 0
        n_at_or_above = sum(1 for r in rows if r["hits"][ek] >= canon["hits"][ek])
        canonical_rank_report[ek] = {
            "canonical_hits": canon["hits"][ek],
            "distinct_hit_counts_desc": distinct[:6],
            "third_highest_distinct": third_highest,
            "tie_inclusive_top3": tie_inclusive_top3,
            "n_quadruples_with_hits_geq_canonical": n_at_or_above,
        }
        if tie_inclusive_top3:
            crit31_pass_tols.append(ek)

    crit31 = len(crit31_pass_tols) >= 3

    # ---- criterion 3.2: no competitor matches > 1 target at eps <= 1e-4
    viol32 = []
    for r in rows:
        if (r["k"], r["d"], r["l"], r["m"]) == CANONICAL:
            continue
        if r["hits"]["1e-04"] > 1:
            viol32.append((r["k"], r["d"], r["l"], r["m"], r["hits"]["1e-04"]))
    crit32 = len(viol32) == 0

    # ---- criterion 3.3: canonical is the UNIQUE quadruple matching alpha_inv at <= 1e-5
    alpha_matchers = {ek: [] for ek in eps_keys}
    for r in rows:
        for eps, ek in zip(TOLERANCES, eps_keys):
            if r["residuals"]["alpha_inv"] < eps:
                alpha_matchers[ek].append((r["k"], r["d"], r["l"], r["m"]))
    with open(OUT_DIR / "alpha_match_quadruples.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["eps", "n_matching", "quadruples_first_50"])
        for ek in eps_keys:
            qs = alpha_matchers[ek]
            w.writerow([ek, len(qs), "; ".join(f"({k},{d};{l},{m})" for k, d, l, m in qs[:50])])
    m5 = alpha_matchers["1e-05"]
    crit33 = (len(m5) == 1 and m5[0] == CANONICAL)

    verdict = "A" if (crit31 and crit32 and crit33) else (
        "B" if not (crit31 or crit32 or crit33) else
        ("B" if not (crit31 and crit32 and crit33) and not any([crit31, crit32, crit33]) else "C"))
    # Pre-reg section 4: A = all three; B = "one or more fail"; C = "some met, others fail".
    # A strict reading makes B and C overlap; report the mechanical split:
    n_met = sum([crit31, crit32, crit33])
    verdict = "A" if n_met == 3 else ("B" if n_met == 0 else "C")

    # ---- meta.json
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=REPO,
                          capture_output=True, text=True).stdout.strip()
    tag_commit = subprocess.run(
        ["git", "rev-list", "-n1", "preregister-fqcr-quotient-uniqueness-v1"],
        cwd=REPO, capture_output=True, text=True).stdout.strip()
    self_sha = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    meta = {
        "prereg": "docs/theory/10_eft_program/preregistrations/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md",
        "prereg_tag": "preregister-fqcr-quotient-uniqueness-v1",
        "prereg_tag_commit": tag_commit,
        "run_date": "2026-07-12",
        "head_commit": head,
        "runner": "tools/scan_fqcr_quotient_uniqueness.py",
        "runner_sha256": self_sha,
        "config": {"N": N, "t": T_BASE, "dps": mp.dps,
                   "search_space": "(k,d;l,m) in {2..8}^4 = 2401",
                   "targets": "20 locked (tools/scan_look_elsewhere.py TARGETS)",
                   "tolerances": eps_keys,
                   "hit_convention": "relative residual |v-t|/|t| < eps (FTD-0097)"},
        "primitives": {"Q": mp.nstr(Q, 30), "lambda_N_4i": mp.nstr(lam, 30),
                       "G_N_star": mp.nstr(gN, 30)},
        "criteria": {
            "3.1_top3_at_geq3_tolerances": {
                "pass": crit31, "tie_inclusive_pass_tolerances": crit31_pass_tols,
                "per_tolerance": canonical_rank_report,
                "note": "pre-reg defines no tie-break; tie-inclusive reading used, "
                        "both readings reported in ANALYSIS.md"},
            "3.2_no_competitor_gt1_target_at_1e-4": {
                "pass": crit32, "n_violators": len(viol32),
                "violators_first_20": viol32[:20]},
            "3.3_unique_alpha_match_at_1e-5": {
                "pass": crit33, "n_alpha_matchers_at_1e-5": len(m5),
                "canonical_among_them": CANONICAL in m5},
        },
        "verdict": verdict,
        "wall_seconds": round(time.time() - t0, 1),
    }
    with open(OUT_DIR / "meta.json", "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    print(json.dumps(meta["criteria"], indent=2))
    print(f"VERDICT: Outcome {verdict}")
    print(f"artifacts -> {OUT_DIR}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
