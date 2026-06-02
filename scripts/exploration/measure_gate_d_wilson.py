import os
import sys
import subprocess
import csv
import numpy as np

ENGINE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../engine'))
EXE_PATH = os.path.join(ENGINE_DIR, 'build', 'Release', 'campaign_s_eff_nonlinear.exe')
OUTPUT_BASE = os.path.join(ENGINE_DIR, 'results', 'gate_d_wilson')

def run_campaign(scenario, l_size, n_seeds, n_samples, n_burn, wilson_arg, out_dir):
    cmd = [
        EXE_PATH,
        f"--scenario={scenario}",
        f"--L={l_size}",
        f"--N-seeds={n_seeds}",
        f"--N-samples={n_samples}",
        f"--N-burn={n_burn}",
        f"--output-dir={out_dir}"
    ]
    if wilson_arg:
        cmd.append(f"--wilson-coefficient={wilson_arg}")
    
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=ENGINE_DIR, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running campaign: {result.stderr}")
        print(f"Stdout: {result.stdout}")
        sys.exit(1)
    
    csv_path = os.path.join(out_dir, "M_ab.csv")
    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not generated. Check engine logs.")
        sys.exit(1)
        
    data = np.zeros((10, 10))
    op_names = ["JJ", "divJ2", "curlJ2", "JdotDivJ", "J4", "stateSq", "reactionDensity", "genesisFlux", "evapFlux", "JdotDeltaS"]
    op_idx = {name: i for i, name in enumerate(op_names)}
    
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or row[0] == 'op_a': continue
            op_a, op_b, val_str = row[0], row[1], row[2]
            i = op_idx.get(op_a)
            j = op_idx.get(op_b)
            if i is not None and j is not None:
                try:
                    val = float(val_str)
                except ValueError:
                    val = float('nan')
                data[i, j] = val
                
    return data

def main():
    smoke = '--smoke' in sys.argv
    scenario = "mixed-balanced"
    
    if smoke:
        l_size = 8
        n_seeds = 1
        n_samples = 8
        n_burn = 50
    else:
        l_size = 16  # Using 16 instead of 32 to keep times tractable for the local run
        n_seeds = 3
        n_samples = 50
        n_burn = 100

    print(f"--- Starting Gate D Wilson Coefficient Campaign ---")
    print(f"L={l_size}, N_seeds={n_seeds}, N_samples={n_samples}")
    
    # 1. Unperturbed
    out_dir_unpert = os.path.join(OUTPUT_BASE, f"L{l_size}_unperturbed")
    M_unpert = run_campaign(scenario, l_size, n_seeds, n_samples, n_burn, None, out_dir_unpert)
    
    # 2. Perturbed (g_JJ = 0.01)
    g_val = 0.01
    out_dir_pert = os.path.join(OUTPUT_BASE, f"L{l_size}_gJJ_{g_val}")
    M_pert = run_campaign(scenario, l_size, n_seeds, n_samples, n_burn, f"g_JJ:{g_val}", out_dir_pert)
    
    # 3. Calculate Derivative
    dM_dg = (M_pert - M_unpert) / g_val
    
    print("\n--- Results ---")
    print("M (Unperturbed) diagonal:")
    print(np.diag(M_unpert))
    print("\ndM/dg_JJ diagonal (Screening Self-Energy Matrix):")
    print(np.diag(dM_dg))
    
    # The crucial signature is that dM/dg creates a mass gap (screening)
    print("\nExtracting screening self-energy Pi(0) from dM/dg...")
    
    # J^2 is index 0
    pi_0 = dM_dg[0, 0]
    print(f"Pi_J(0) = {pi_0:.6f}")
    
    # Compare against analytical expectation 16 G*^2 (1 - G*/x_+)
    G_star = 2.958675119
    x_plus = 137.0361715
    analytical = 16 * (G_star**2) * (1 - G_star / x_plus)
    print(f"Analytical Prediction = {analytical:.6f}")
    print(f"Ratio (Measured / Analytical) = {pi_0 / analytical:.6f}")

if __name__ == '__main__':
    main()
