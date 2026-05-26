#!/usr/bin/env python3
"""
FQCR Quotient Uniqueness Scan (FTD-0143).
"""

import os
import json
import csv
import math
from mpmath import mp, mpf, exp as mp_exp, log as mp_log, sqrt as mp_sqrt, pi as mp_pi, gamma as mp_gamma

mp.dps = 50

# ----------------------------------------------------------------------------
# 20 Look-Elsewhere dimensionless targets
# ----------------------------------------------------------------------------
TARGETS = [
    ("alpha_inv",         137.035999084,    1.5e-10),
    ("m_e_in_MeV",        0.51099895069,    3e-10),
    ("m_p_over_m_e",      1836.15267343,    6e-11),
    ("m_n_over_m_e",      1838.68366173,    9e-10),
    ("m_mu_over_m_e",     206.7682830,      1.6e-8),
    ("m_tau_over_m_e",    3477.23,          5e-5),
    ("m_p_over_m_n",      0.99862347796,    7e-10),
    ("g_e_minus_2",       0.00231930437,    8e-13),
    ("a_mu",              0.0011659184,     6e-10),
    ("alpha_s_MZ",        0.1179,           1e-3),
    ("sin2_theta_W",      0.22290,          3e-5),
    ("Vud_squared",       0.94888,          5e-5),
    ("m_W_over_m_Z",      0.88147,          2e-5),
    ("m_b_over_m_c",      4.18,             0.05),
    ("m_t_over_v_higgs",  0.991,            0.001),
    ("Omega_b",           0.0493,           0.0006),
    ("Omega_dm",          0.265,            0.007),
    ("h_Hubble",          0.674,            0.005),
    ("Theta_13",          0.150,            0.001),
    ("delta_CP",          1.36,             0.17),
]

TOLERANCES = [1e-3, 1e-4, 1e-5, 1e-6]

# ----------------------------------------------------------------------------
# FQCR Mathematical functions (high precision)
# ----------------------------------------------------------------------------

def G_N_star(N):
    one = mpf(1)
    N_mp = mpf(N)
    return (
        (N_mp + 1) ** (-one / 2)
        * mp_gamma(N_mp + mpf(7) / 4)
        * mp_gamma(one / 4)
        / (mp_gamma(N_mp + mpf(5) / 4) * mp_gamma(mpf(3) / 4))
    )

def theta2_N(t, N):
    q = mp_exp(-4.0 * mp_pi * t)
    s = mpf(0)
    for n in range(N + 1):
        s += q ** ((mpf(n) + mpf(0.5)) ** 2)
    return 2.0 * s

def theta3_N(t, N):
    q = mp_exp(-4.0 * mp_pi * t)
    s = mpf(0)
    for n in range(1, N + 1):
        s += q ** (n * n)
    return 1.0 + 2.0 * s

def lambda_N(t, N):
    return (theta2_N(t, N) / theta3_N(t, N)) ** 4

def A_N(t, N, k, d, l, m):
    Q = mp_exp(-2.0 * mp_pi * t)
    s1 = mpf(0)
    s2 = mpf(0)
    for n in range(1, N + 1):
        Qkn = Q ** (k * n)
        Qln = Q ** (l * n)
        if Qkn < 1.0:
            s1 += n * Qkn / (1.0 - Qkn)
        if Qln < 1.0:
            s2 += n * Qln / (1.0 - Qln)
    
    coeff1 = 2.0 * mp_pi * k * d / 3.0
    coeff2 = 2.0 * mp_pi * l * m / 3.0
    return coeff1 * s1 - coeff2 * s2

def main():
    print("============================================================")
    print("  FQCR Quotient Uniqueness Scan (FTD-0143)")
    print("============================================================")
    
    N = 4096
    print(f"Pre-computing G_N* at N={N}...")
    g_N = G_N_star(N)
    print(f"  G_N* = {str(g_N)[:32]}")
    
    print(f"Pre-computing lambda_N at N={N}, t=1.0...")
    lam = lambda_N(mpf(1.0), N)
    print(f"  lambda_N = {str(lam)[:32]}")
    
    out_dir = "engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan"
    os.makedirs(out_dir, exist_ok=True)
    
    search_space = list(range(2, 9))  # {2, ..., 8}
    quadruples = []
    
    print("Starting uniqueness scan over 2401 quadruples...")
    total_quads = len(search_space) ** 4
    count = 0
    
    all_quads_data = []
    
    for k in search_space:
        for d in search_space:
            for l in search_space:
                for m in search_space:
                    count += 1
                    if count % 200 == 0:
                        print(f"  Processed {count}/{total_quads}...")
                    
                    a_val = A_N(mpf(1.0), N, k, d, l, m)
                    R_N = 1.0 + lam + a_val
                    
                    disc = 4.0 * g_N - R_N
                    if disc < 0:
                        x_plus = None
                    else:
                        x_plus = 8.0 * g_N * g_N + 4.0 * (g_n := g_N) ** 1.5 * mp_sqrt(disc)
                    
                    # Compute hits against targets
                    hits = {eps: [] for eps in TOLERANCES}
                    match_details = {}
                    
                    if x_plus is not None:
                        x_float = float(x_plus)
                        for tname, tval, _ in TARGETS:
                            res = abs(x_float - tval) / tval
                            for eps in TOLERANCES:
                                if res < eps:
                                    hits[eps].append(tname)
                            if res < 1e-3:
                                match_details[tname] = float(res)
                    
                    all_quads_data.append({
                        "k": k, "d": d, "l": l, "m": m,
                        "x_plus": float(x_plus) if x_plus is not None else None,
                        "hits": hits,
                        "match_details": match_details
                    })
                    
    print("Scan finished.")
    
    # ----------------------------------------------------------------------------
    # Write output CSV: all_quadruples.csv
    # ----------------------------------------------------------------------------
    csv_path = os.path.join(out_dir, "all_quadruples.csv")
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(["k", "d", "l", "m", "x_plus", "hits_1e-3", "hits_1e-4", "hits_1e-5", "hits_1e-6", "matched_targets"])
        for q in all_quads_data:
            matched_str = ";".join(f"{t}:{r:.2e}" for t, r in q["match_details"].items())
            writer.writerow([
                q["k"], q["d"], q["l"], q["m"],
                q["x_plus"] if q["x_plus"] is not None else "complex",
                len(q["hits"][1e-3]),
                len(q["hits"][1e-4]),
                len(q["hits"][1e-5]),
                len(q["hits"][1e-6]),
                matched_str
            ])
            
    # ----------------------------------------------------------------------------
    # Write ranking CSVs: ranking_eps_*.csv
    # ----------------------------------------------------------------------------
    for eps in TOLERANCES:
        # Sort by total hits at this tolerance
        sorted_quads = sorted(all_quads_data, key=lambda q: len(q["hits"][eps]), reverse=True)
        rank_path = os.path.join(out_dir, f"ranking_eps_{eps:.0e}.csv")
        with open(rank_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["rank", "k", "d", "l", "m", "x_plus", "total_hits", "matched_targets"])
            for rank, q in enumerate(sorted_quads[:20], 1):
                matched_str = ";".join(q["hits"][eps])
                writer.writerow([
                    rank, q["k"], q["d"], q["l"], q["m"],
                    q["x_plus"] if q["x_plus"] is not None else "complex",
                    len(q["hits"][eps]),
                    matched_str
                ])
                
    # ----------------------------------------------------------------------------
    # Write alpha match CSV: alpha_match_quadruples.csv
    # ----------------------------------------------------------------------------
    alpha_path = os.path.join(out_dir, "alpha_match_quadruples.csv")
    with open(alpha_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(["epsilon", "k", "d", "l", "m", "x_plus", "residual"])
        for eps in TOLERANCES:
            for q in all_quads_data:
                if "alpha_inv" in q["match_details"] and q["match_details"]["alpha_inv"] < eps:
                    writer.writerow([
                        f"{eps:.0e}", q["k"], q["d"], q["l"], q["m"],
                        q["x_plus"], q["match_details"]["alpha_inv"]
                    ])
                    
    # ----------------------------------------------------------------------------
    # Evaluate pre-registered uniqueness criteria (§3)
    # ----------------------------------------------------------------------------
    print("\nEvaluating pre-registered uniqueness criteria...")
    canonical = (4, 6, 3, 2)
    canonical_data = next(q for q in all_quads_data if (q["k"], q["d"], q["l"], q["m"]) == canonical)
    
    # 1. Top-3 across multiple tolerances
    top_3_count = 0
    for eps in TOLERANCES:
        sorted_quads = sorted(all_quads_data, key=lambda q: len(q["hits"][eps]), reverse=True)
        top_3_quads = [(q["k"], q["d"], q["l"], q["m"]) for q in sorted_quads[:3]]
        if canonical in top_3_quads:
            top_3_count += 1
            print(f"  - Top-3 at tolerance {eps:.0e}: PASS")
        else:
            print(f"  - Top-3 at tolerance {eps:.0e}: FAIL")
            
    crit_1_pass = top_3_count >= 3
    
    # 2. No competing quadruple matches more than 1 target at <= 10^-4
    competing_violations = 0
    for q in all_quads_data:
        if (q["k"], q["d"], q["l"], q["m"]) != canonical:
            hits_1e4 = len(q["hits"][1e-4])
            if hits_1e4 > 1:
                competing_violations += 1
                
    crit_2_pass = competing_violations == 0
    print(f"  - No competing quadruple matches >1 target at 1e-4: {'PASS' if crit_2_pass else 'FAIL'} ({competing_violations} violations)")
    
    # 3. alpha-target at <= 10^-5 exclusively
    alpha_matches_1e5 = []
    for q in all_quads_data:
        if "alpha_inv" in q["match_details"] and q["match_details"]["alpha_inv"] < 1e-5:
            alpha_matches_1e5.append((q["k"], q["d"], q["l"], q["m"]))
            
    crit_3_pass = (alpha_matches_1e5 == [canonical])
    print(f"  - alpha_inv unique match at 1e-5: {'PASS' if crit_3_pass else 'FAIL'} (matches: {alpha_matches_1e5})")
    
    # ----------------------------------------------------------------------------
    # Determine and write Verdict to ANALYSIS.md
    # ----------------------------------------------------------------------------
    if crit_1_pass and crit_2_pass and crit_3_pass:
        verdict = "Outcome A — uniqueness confirmed"
        verdict_desc = "All three criteria met. Model IV is upgraded to [SELECTION with uniqueness backing]."
    elif (not crit_1_pass) and (not crit_2_pass) and (not crit_3_pass):
        verdict = "Outcome B — uniqueness rejected"
        verdict_desc = "All criteria failed. Model IV remains [SELECTION] with no uniqueness backing; the exponent quadruple is one of many near-misses."
    else:
        verdict = "Outcome C — partial / inconclusive"
        verdict_desc = "Some criteria were met while others failed. Model IV remains [SELECTION] without uniqueness backing."
        
    print(f"\nVerdict: {verdict}")
    print(verdict_desc)
    
    analysis_path = os.path.join(out_dir, "ANALYSIS.md")
    with open(analysis_path, 'w', encoding='utf-8') as f:
        f.write("# ANALYSIS — FQCR Quotient Uniqueness Scan (FTD-0143)\n\n")
        f.write("**Tag:** [MEASUREMENT ANALYSIS] — records the result of the quotient uniqueness scan.\n")
        f.write(f"**Date:** 2026-05-26\n")
        f.write(f"**Verdict:** {verdict}\n\n")
        f.write(f"{verdict_desc}\n\n")
        
        f.write("## 1. Criterion Evaluation Summary\n\n")
        f.write(f"- **§3.1 — Top-3 across multiple tolerances:** {'PASS' if crit_1_pass else 'FAIL'} (top-3 in {top_3_count}/4 tolerances)\n")
        f.write(f"- **§3.2 — No competing quadruple matches >1 target at 1e-4:** {'PASS' if crit_2_pass else 'FAIL'} ({competing_violations} violations)\n")
        f.write(f"- **§3.3 — α-target at 1e-5 exclusively:** {'PASS' if crit_3_pass else 'FAIL'} (matched by {len(alpha_matches_1e5)} quadruples: {alpha_matches_1e5})\n\n")
        
        f.write("## 2. Canonical (4, 6; 3, 2) Performance\n\n")
        f.write(f"- **x_plus value:** {canonical_data['x_plus']:.9f}\n")
        f.write("- **Matched targets at 1e-3:** " + ", ".join(canonical_data["hits"][1e-3]) + "\n")
        f.write("- **Matched targets at 1e-4:** " + ", ".join(canonical_data["hits"][1e-4]) + "\n")
        f.write("- **Matched targets at 1e-5:** " + ", ".join(canonical_data["hits"][1e-5]) + "\n")
        f.write("- **Matched targets at 1e-6:** " + ", ".join(canonical_data["hits"][1e-6]) + "\n\n")
        
    # Write meta.json
    meta_path = os.path.join(out_dir, "meta.json")
    with open(meta_path, 'w', encoding='utf-8') as f:
        json.dump({
            "campaign": "FQCR quotient uniqueness scan",
            "ledger_row": "FTD-0143",
            "protocol": "docs/theory/10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md",
            "hash_lock_tag": "preregister-fqcr-quotient-uniqueness-v1",
            "N": N,
            "verdict": verdict,
            "canonical_x_plus": float(canonical_data["x_plus"]) if canonical_data["x_plus"] is not None else None,
            "criteria_passed": {
                "top_3_tolerances": crit_1_pass,
                "no_competing_overlap": crit_2_pass,
                "exclusive_alpha_1e5": crit_3_pass
            }
        }, f, indent=2)
        
    print(f"Quotient scan complete. Results saved in {out_dir}/")

if __name__ == "__main__":
    main()
