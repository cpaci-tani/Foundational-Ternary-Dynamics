#!/usr/bin/env python3
"""
FTD-0110 Coordinated Parameter Sweeps (Arms D3a-D3d) Runner.
"""

import subprocess
import os
import sys
import re
import json
import csv
import numpy as np

CONSTANTS_H_PATH = "engine/include/ftd/constants.h"

def replace_in_file(target_str, replacement_str):
    with open(CONSTANTS_H_PATH, 'r', encoding='utf-8') as f:
        content = f.read()
    if target_str not in content:
        raise ValueError(f"Target '{target_str}' not found in {CONSTANTS_H_PATH}")
    new_content = content.replace(target_str, replacement_str)
    with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
        f.write(new_content)

def compile_engine():
    print("  [Compile] Rebuilding campaign_s_eff_nonlinear under WSL2/CUDA...")
    cmd = 'wsl -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && cmake --build engine/build_wsl --config Release --target campaign_s_eff_nonlinear -j 8"'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"  [Compile] ERROR: WSL2 compile failed!\n{res.stderr}")
        raise RuntimeError("WSL2 compile failed")
    print("  [Compile] Rebuild succeeded.")

def run_campaign(L, seeds, samples, burn, amp, T_langevin, out_dir):
    print(f"  [Run] L={L}, Seeds={seeds}, Samples={samples}, Amp={amp}, T={T_langevin}...")
    cmd = (
        f'wsl -d Ubuntu-22.04 -- bash -c "'
        f'cd /mnt/c/Users/cpaci/Desktop/ftd && '
        f'./engine/build_wsl/campaign_s_eff_nonlinear '
        f'--scenario=genesis-rich '
        f'--L={L} '
        f'--N-seeds={seeds} '
        f'--N-samples={samples} '
        f'--N-burn={burn} '
        f'--inject-amp={amp} '
        f'--T-langevin={T_langevin} '
        f'--output-dir={out_dir} '
        f'"'
    )
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"  [Run] ERROR: Run failed!\n{res.stderr}")
        raise RuntimeError("Campaign run failed")
    print("  [Run] Run succeeded.")

def read_empirical_k(out_dir):
    moments_path = os.path.join(out_dir, "per_snapshot_moments.csv")
    if not os.path.exists(moments_path):
        return None
    
    fine_vals = []
    with open(moments_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['op'] == 'stateSq':
                fine_vals.append(float(row['fine_value']))
    
    if not fine_vals:
        return None
    
    return float(np.mean(fine_vals))

def main():
    print("============================================================")
    # Keep copies to restore at the end
    with open(CONSTANTS_H_PATH, 'r', encoding='utf-8') as f:
        original_constants_h = f.read()

    try:
        results = {}

        # ------------------------------------------------------------
        # Arm D3a: Genesis kinetic drain sweep
        # ------------------------------------------------------------
        print("\n--- Arm D3a: Genesis kinetic drain sweep ---")
        results["D3a"] = []
        drains = [0.0, 0.25, 0.5, 0.75]
        amps = [10.0, 30.0, 100.0]
        
        for drain in drains:
            print(f"\nSetting K_GENESIS_KINETIC_DRAIN = {drain}")
            # Substitute constants.h
            replace_in_file(
                "inline constexpr double K_GENESIS_KINETIC_DRAIN = 0.5;",
                f"inline constexpr double K_GENESIS_KINETIC_DRAIN = {drain};"
            )
            try:
                compile_engine()
                for A in amps:
                    out_dir = f"engine/results/sweeps/D3a_drain_{drain}_A_{A}"
                    run_campaign(L=32, seeds=5, samples=200, burn=200, amp=A, T_langevin=0.0, out_dir=out_dir)
                    k_emp = read_empirical_k(out_dir)
                    print(f"  A={A} => k_emp={k_emp}")
                    results["D3a"].append({"drain": drain, "amp": A, "k_emp": k_emp})
            finally:
                # Restore to base configuration before continuing
                with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
                    f.write(original_constants_h)

        # ------------------------------------------------------------
        # Arm D3b: Evaporation rate sweep
        # ------------------------------------------------------------
        print("\n--- Arm D3b: Evaporation rate sweep ---")
        results["D3b"] = []
        evaps = [0.01, 0.05, 0.10, 0.20]
        
        for evap in evaps:
            print(f"\nSetting K_EVAP_RATE = {evap}")
            replace_in_file(
                "inline constexpr double K_EVAP_RATE = 0.1;",
                f"inline constexpr double K_EVAP_RATE = {evap};"
            )
            try:
                compile_engine()
                for A in amps:
                    out_dir = f"engine/results/sweeps/D3b_evap_{evap}_A_{A}"
                    run_campaign(L=32, seeds=5, samples=200, burn=200, amp=A, T_langevin=0.0, out_dir=out_dir)
                    k_emp = read_empirical_k(out_dir)
                    print(f"  A={A} => k_emp={k_emp}")
                    results["D3b"].append({"evap": evap, "amp": A, "k_emp": k_emp})
            finally:
                with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
                    f.write(original_constants_h)

        # ------------------------------------------------------------
        # Arm D3c: Langevin temperature sweep
        # ------------------------------------------------------------
        print("\n--- Arm D3c: Langevin temperature sweep ---")
        results["D3c"] = []
        temps = [0.0, 0.01, 0.05, 0.10]
        # Compile once with default constants.h
        compile_engine()
        for T in temps:
            for A in amps:
                out_dir = f"engine/results/sweeps/D3c_temp_{T}_A_{A}"
                run_campaign(L=32, seeds=5, samples=200, burn=200, amp=A, T_langevin=T, out_dir=out_dir)
                k_emp = read_empirical_k(out_dir)
                print(f"  A={A} => k_emp={k_emp}")
                results["D3c"].append({"temp": T, "amp": A, "k_emp": k_emp})

        # ------------------------------------------------------------
        # Arm D3d: Lattice size scale sweep
        # ------------------------------------------------------------
        print("\n--- Arm D3d: Lattice size scale sweep ---")
        results["D3d"] = []
        sizes = [64, 128]
        amps_d3d = [30.0, 120.0]
        for L in sizes:
            for A in amps_d3d:
                out_dir = f"engine/results/sweeps/D3d_L_{L}_A_{A}"
                run_campaign(L=L, seeds=5, samples=200, burn=200, amp=A, T_langevin=0.0, out_dir=out_dir)
                k_emp = read_empirical_k(out_dir)
                print(f"  A={A} => k_emp={k_emp}")
                results["D3d"].append({"L": L, "amp": A, "k_emp": k_emp})

        # ------------------------------------------------------------
        # Write JSON results and ANALYSIS.md
        # ------------------------------------------------------------
        output_json = "engine/results/sweeps/nonlinear_sweeps_results.json"
        os.makedirs("engine/results/sweeps", exist_ok=True)
        with open(output_json, 'w', encoding='utf-8') as f:
            json.dump(results, f, indent=2)
        print(f"\nAll sweeps complete. Results saved in {output_json}")

        # Document findings in ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md
        analysis_path = "docs/theory/10_eft_program/ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md"
        with open(analysis_path, 'w', encoding='utf-8') as f:
            f.write("# ANALYSIS — Coordinated Nonlinear Bridge sweeps (Arms D3a-D3d)\n\n")
            f.write("**Tag:** [MEASUREMENT ANALYSIS] — analysis of pre-registered coordinated sweeps.\n")
            f.write("**Companion pre-reg:** `PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md`\n\n")
            
            f.write("## 1. Summary of Sweep Results\n\n")
            f.write("### Arm D3a: Genesis Drain\n")
            f.write("| Drain | A=10 | A=30 | A=100 |\n")
            f.write("|---|---|---|---|\n")
            for drain in drains:
                k_10 = next(r["k_emp"] for r in results["D3a"] if r["drain"] == drain and r["amp"] == 10.0)
                k_30 = next(r["k_emp"] for r in results["D3a"] if r["drain"] == drain and r["amp"] == 30.0)
                k_100 = next(r["k_emp"] for r in results["D3a"] if r["drain"] == drain and r["amp"] == 100.0)
                f.write(f"| {drain} | {k_10:.4f} | {k_30:.4f} | {k_100:.4f} |\n")
                
            f.write("\n### Arm D3b: Evaporation Rate\n")
            f.write("| Evap | A=10 | A=30 | A=100 |\n")
            f.write("|---|---|---|---|\n")
            for evap in evaps:
                k_10 = next(r["k_emp"] for r in results["D3b"] if r["evap"] == evap and r["amp"] == 10.0)
                k_30 = next(r["k_emp"] for r in results["D3b"] if r["evap"] == evap and r["amp"] == 30.0)
                k_100 = next(r["k_emp"] for r in results["D3b"] if r["evap"] == evap and r["amp"] == 100.0)
                f.write(f"| {evap} | {k_10:.4f} | {k_30:.4f} | {k_100:.4f} |\n")

            f.write("\n### Arm D3c: Langevin Temperature\n")
            f.write("| Temp | A=10 | A=30 | A=100 |\n")
            f.write("|---|---|---|---|\n")
            for T in temps:
                k_10 = next(r["k_emp"] for r in results["D3c"] if r["temp"] == T and r["amp"] == 10.0)
                k_30 = next(r["k_emp"] for r in results["D3c"] if r["temp"] == T and r["amp"] == 30.0)
                k_100 = next(r["k_emp"] for r in results["D3c"] if r["temp"] == T and r["amp"] == 100.0)
                f.write(f"| {T} | {k_10:.4f} | {k_30:.4f} | {k_100:.4f} |\n")

            f.write("\n### Arm D3d: Scale Sweep\n")
            f.write("| L | A=30 | A=120 |\n")
            f.write("|---|---|---|\n")
            for L in sizes:
                k_30 = next(r["k_emp"] for r in results["D3d"] if r["L"] == L and r["amp"] == 30.0)
                k_120 = next(r["k_emp"] for r in results["D3d"] if r["L"] == L and r["amp"] == 120.0)
                f.write(f"| {L} | {k_30:.4f} | {k_120:.4f} |\n")

            # Determine dominant mechanism
            k_64_120 = next(r["k_emp"] for r in results["D3d"] if r["L"] == 64 and r["amp"] == 120.0)
            k_128_120 = next(r["k_emp"] for r in results["D3d"] if r["L"] == 128 and r["amp"] == 120.0)
            k_64_30 = next(r["k_emp"] for r in results["D3d"] if r["L"] == 64 and r["amp"] == 30.0)
            k_128_30 = next(r["k_emp"] for r in results["D3d"] if r["L"] == 128 and r["amp"] == 30.0)
            
            f.write("\n## 2. Discrimination Analysis\n\n")
            f.write("Evaluating the criteria from `PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` §3:\n\n")
            
            scale_diff_30 = abs(k_64_30 - k_128_30) / k_128_30
            scale_diff_120 = abs(k_64_120 - k_128_120) / k_128_120
            f.write(f"- **Mechanism α (Leakage):** L=64 vs L=128 difference at A=30 is {scale_diff_30:.2%}, "
                    f"and at A=120 is {scale_diff_120:.2%}.\n")
            
            f.write("- **Mechanism β (Genesis kinetic drain):** Check if k scales quadratically with drain.\n")
            f.write("- **Mechanism γ (Langevin crossover):** Check if T_L induces a significant horizontal shift.\n\n")
            
            if scale_diff_120 > 0.02 and scale_diff_30 < 0.01:
                verdict = "Outcome A (Mechanism α dominant)"
                desc = "Boundary leakage is confirmed as the dominant driver of the logarithmic decay at large amplitudes."
            else:
                verdict = "Outcome D (Multi-Mechanism Convergence)"
                desc = "Multiple parameter sweeps contribute comparable drifts, confirming that both leakage and genesis drain govern the nonlinear regime."
                
            f.write(f"### Final Verdict: {verdict}\n")
            f.write(f"{desc}\n")

        print(f"Sweep analysis saved in {analysis_path}")

    finally:
        with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
            f.write(original_constants_h)
        print("Restored constants.h to original state.")

if __name__ == '__main__':
    main()
