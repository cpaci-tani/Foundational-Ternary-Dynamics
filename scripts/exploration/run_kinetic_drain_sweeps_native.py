import subprocess
import os
import sys
import csv
import re

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
    print("  [Compile] Rebuilding campaign_thermostat_off_sweep...")
    cmd = 'cmake --build engine/build --config Release --target campaign_thermostat_off_sweep --parallel 24'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"  [Compile] ERROR: MSVC compile failed!\n{res.stderr}")
        raise RuntimeError("MSVC compile failed")
    print("  [Compile] Rebuild succeeded.")

def run_campaign(L, A, seeds, thermostat_on, output_dir):
    print(f"  [Run] L={L}, A={A}, seeds={seeds}, thermostat={thermostat_on}...")
    thermostat_flag = "--thermostat=on" if thermostat_on else "--thermostat=off"
    cmd = f'.\\engine\\build\\Release\\campaign_thermostat_off_sweep.exe --L={L} --A={A} --seeds={seeds} {thermostat_flag} --coupling=on --output-dir={output_dir}'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"  [Run] ERROR: Run failed!\n{res.stderr}")
        raise RuntimeError("Campaign run failed")
    print("  [Run] Run succeeded.")

def read_k_mean(output_dir, A):
    char_A = f"{A:.2f}"
    csv_path = os.path.join(output_dir, f"sweep_run_A{char_A}.csv")
    if not os.path.exists(csv_path):
        return None
    
    k_vals = []
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            k_vals.append(float(row['k_mean']))
    
    if not k_vals:
        return None
    return sum(k_vals) / len(k_vals)

def main():
    print("=== run_kinetic_drain_sweeps_native.py ===")
    with open(CONSTANTS_H_PATH, 'r', encoding='utf-8') as f:
        original_constants_h = f.read()

    try:
        drains = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5]
        amps = [10.0, 30.0, 50.0]
        results = []

        output_dir = "engine/results/sweeps_native/"
        os.makedirs(output_dir, exist_ok=True)

        for drain in drains:
            print(f"\nSetting K_GENESIS_KINETIC_DRAIN = {drain}")
            replace_in_file(
                "inline constexpr double K_GENESIS_KINETIC_DRAIN = 0.5;",
                f"inline constexpr double K_GENESIS_KINETIC_DRAIN = {drain};"
            )
            try:
                compile_engine()
                for A in amps:
                    # Clean up old CSV if exists
                    csv_path = os.path.join(output_dir, f"sweep_run_A{A:.2f}.csv")
                    if os.path.exists(csv_path):
                        os.remove(csv_path)

                    run_campaign(L=32, A=A, seeds=5, thermostat_on=True, output_dir=output_dir)
                    k_mean = read_k_mean(output_dir, A)
                    print(f"  drain={drain}, A={A} => k_mean={k_mean}")
                    results.append({"drain": drain, "amp": A, "k_mean": k_mean})
            finally:
                with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
                    f.write(original_constants_h)

        print("\n=== SWEEP RESULTS TABLE ===")
        print(f"{'Drain':<8} | {'A=10.0':<8} | {'A=30.0':<8} | {'A=50.0':<8}")
        print("-" * 38)
        for drain in drains:
            k_10 = next((r["k_mean"] for r in results if r["drain"] == drain and r["amp"] == 10.0), None)
            k_30 = next((r["k_mean"] for r in results if r["drain"] == drain and r["amp"] == 30.0), None)
            k_50 = next((r["k_mean"] for r in results if r["drain"] == drain and r["amp"] == 50.0), None)
            
            def fmt(val):
                return f"{val:.4f}" if val is not None else "N/A"
            
            print(f"{drain:<8} | {fmt(k_10):<8} | {fmt(k_30):<8} | {fmt(k_50):<8}")

    except Exception as e:
        print(f"Error occurred: {e}")
    finally:
        with open(CONSTANTS_H_PATH, 'w', encoding='utf-8') as f:
            f.write(original_constants_h)
        print("\nRestored constants.h to original state.")

if __name__ == '__main__':
    main()
