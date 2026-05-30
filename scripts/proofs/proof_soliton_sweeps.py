#!/usr/bin/env python3
import pandas as pd
import numpy as np
import sys
import os

def main():
    print("================================================================")
    print("  VERIFICATION: Emergent Soliton Sweeps Analysis")
    print("================================================================\n")

    csv_path = "engine/results/soliton_sweeps.csv"
    if not os.path.exists(csv_path):
        print(f"Error: CSV results file not found at {csv_path}")
        print("Please run the C++ sweeps program under WSL2 first.")
        sys.exit(1)

    df = pd.read_csv(csv_path)

    # Calculate statistics grouped by config and amplitude
    summary = df.groupby(["config", "amplitude"]).agg(
        n_mean=("n_total", "mean"),
        n_std=("n_total", "std"),
        drift_mean=("centroid_drift", "mean"),
        rms_mean=("rms_radius", "mean"),
        tau_mean=("tau_bind", "mean"),
        solitons=("regime", lambda x: (x == "SOLITON").sum()),
        floods=("regime", lambda x: (x == "FLOODING").sum()),
        bounds=("regime", lambda x: (x == "BOUND").sum()),
        decays=("regime", lambda x: (x == "DECAY").sum())
    ).reset_index()

    # Determine major regime
    def get_major_regime(row):
        counts = {
            "SOLITON": row["solitons"],
            "FLOODING": row["floods"],
            "BOUND": row["bounds"],
            "DECAY": row["decays"]
        }
        return max(counts, key=counts.get)

    summary["major_regime"] = summary.apply(get_major_regime, axis=1)

    print("Aggregated Sweep Results:")
    print("-" * 110)
    print(f"{'Config':<15} | {'Amp':<5} | {'N (mean±std)':<14} | {'Drift (mean)':<12} | {'RMS Rad (mean)':<14} | {'tau_bind':<8} | {'Regime':<10}")
    print("-" * 110)
    for _, row in summary.iterrows():
        n_str = f"{row['n_mean']:.1f} ± {row['n_std']:.1f}" if not pd.isna(row['n_std']) else f"{row['n_mean']:.1f} ± 0.0"
        print(f"{row['config']:<15} | {row['amplitude']:<5.1f} | {n_str:<14} | {row['drift_mean']:<12.2f} | {row['rms_mean']:<14.2f} | {row['tau_mean']:<8.1f} | {row['major_regime']:<10}")
    print("-" * 110 + "\n")

    # ----- Strict Verification Assertions -----
    failures = 0

    # 1. Verify stable soliton at A=10.0 under Default has n_total in expected range [3, 6]
    default_a10 = df[(df["config"] == "Default") & (df["amplitude"] == 10.0)]
    if not default_a10.empty:
        n_a10 = default_a10["n_total"].mean()
        is_stable = n_a10 >= 3 and n_a10 <= 6
        if is_stable:
            print(f"  [PASS] A=10.0 soliton matter content is stable: {n_a10:.2f} (Expected: ~4)")
        else:
            print(f"  [FAIL] A=10.0 soliton matter content is out of expected range [3, 6]: {n_a10:.2f}")
            failures += 1
    else:
        print("  [FAIL] No data found for Default at A=10.0.")
        failures += 1

    # 2. Verify flooding onset at large amplitudes (A >= 30.0) under Langevin configs
    large_langevin = df[(df["config"].isin(["Color+Triad", "Full Physics"])) & (df["amplitude"] >= 30.0)]
    if not large_langevin.empty:
        floods_or_decays = np.all((large_langevin["regime"] == "FLOODING") | (large_langevin["regime"] == "DECAY"))
        if floods_or_decays:
            print("  [PASS] Large amplitude excitations (A >= 30.0) under Langevin toggles correctly flood or decay.")
        else:
            print(f"  [FAIL] Langevin excitations did not flood or decay as expected: {large_langevin['regime'].values}")
            failures += 1
    else:
        print("  [FAIL] No data found for Langevin configs at large amplitudes.")
        failures += 1

    # 3. Verify stable soliton localization at A=10.0 under Default
    if not default_a10.empty:
        drift_a10 = default_a10["centroid_drift"].mean()
        rms_a10 = default_a10["rms_radius"].mean()
        if drift_a10 < 1.0 and rms_a10 < 2.0:
            print(f"  [PASS] Bounded propagating soliton confirmed (drift={drift_a10:.2f}, rms={rms_a10:.2f}).")
        else:
            print(f"  [FAIL] Soliton drift or rms radius out of bounds: drift={drift_a10:.2f}, rms={rms_a10:.2f}")
            failures += 1

    print("\n================================================================")
    if failures == 0:
        print("  RESULT: ALL SWEEP VERIFICATIONS PASSED")
        sys.exit(0)
    else:
        print(f"  RESULT: {failures} VERIFICATION FAILURE(S)")
        sys.exit(1)

if __name__ == "__main__":
    main()
