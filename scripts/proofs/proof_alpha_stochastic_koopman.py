#!/usr/bin/env python3
"""
proof_alpha_stochastic_koopman.py

Stochastic Transfer / Koopman Operator Estimator for the FTD Alpha Readout.
Estimates the dominant nontrivial relaxation eigenvalue (mu_+) of the 
Langevin-stabilized cloud ensemble and extracts alpha = |1 - mu_+|.

Upgraded with Hankel Delay Embedding to recover non-Markovian dynamics.
"""

import argparse
import numpy as np
import os
import matplotlib.pyplot as plt

# FTD Master Quadratic Exact Root
TARGET_ALPHA_INV = 137.0361714582
TARGET_MU_PLUS = 1.0 - (1.0 / TARGET_ALPHA_INV)

# Scale-context enums (mirror engine/include/ftd/render_bridge_diagnostics.h).
SCALE_REGIME = {0: "Indeterminate", 1: "Evaporating", 2: "UVLocked",
                3: "BoundedAdmissible", 4: "ShellDominated", 5: "Percolating"}
# Gate thresholds (mirror ScaleContextConfig defaults in scale_context.h).
PHI_BALANCE_TOL = 0.15
DPHI_DR_MAX = 0.0
DR_DT_TOL = 0.02
DJ2_DT_TOL = 0.02


def assess_admissibility(data):
    """Reconstruct the readout-admissibility verdict from the carried sc_* columns.

    The dumper runs the scale-context tracker in OBSERVE-ONLY mode (gate not
    armed), so the per-tick sc_status column is always DiagnosticOnly. Here we
    ARM the gate by replaying the same classifier (engine/src/scale_context.cpp
    ::classify) over the FINAL tick, whose sc_* fields already carry the
    windowed rolling estimates from the tracker.

    Returns (admissible: bool|None, status: str, detail: dict). admissible is
    None when the trajectory carries no scale-context columns (cannot assess).
    """
    if "sc_regime" not in data:
        return None, "UNKNOWN", {"reason": "no sc_* columns in trajectory"}

    last = -1  # final windowed state
    regime = int(data["sc_regime"][last])
    detail = {"regime": SCALE_REGIME.get(regime, f"#{regime}")}
    for k in ("sc_zeta", "sc_kappa", "sc_beta", "sc_factive",
              "sc_dRdt", "sc_dJ2dt"):
        if k in data:
            detail[k] = float(data[k][last])

    # Fraction of the recorded run spent BoundedAdmissible (context).
    reg = data["sc_regime"]
    detail["frac_bounded"] = float(np.mean(reg == 3))

    if regime != 3:  # not BoundedAdmissible -> geometric scale-context failure
        return False, "REJECTED_SCALE_CONTEXT", detail

    # BoundedAdmissible: check self-confinement + stationarity (armed gate).
    phi_out = float(data["sc_phi_out"][last]) if "sc_phi_out" in data else 0.0
    phi_ret = float(data["sc_phi_ret"][last]) if "sc_phi_ret" in data else 0.0
    dphidr = float(data["sc_dPhidR"][last]) if "sc_dPhidR" in data else 0.0
    phi_norm = abs(phi_out - phi_ret) / (phi_out + phi_ret + 1e-12)
    confined = (phi_norm <= PHI_BALANCE_TOL) and (dphidr < DPHI_DR_MAX)
    detail["phi_balance_norm"] = phi_norm
    detail["dPhi_dR"] = dphidr
    if not confined:
        return False, "REJECTED_SELF_CONFINEMENT", detail

    drdt = abs(float(data["sc_dRdt"][last])) if "sc_dRdt" in data else 0.0
    dj2dt = abs(float(data["sc_dJ2dt"][last])) if "sc_dJ2dt" in data else 0.0
    if drdt > DR_DT_TOL or dj2dt > DJ2_DT_TOL:
        return False, "REJECTED_NON_STATIONARY", detail

    return True, "ADMISSIBLE", detail

def parse_args():
    parser = argparse.ArgumentParser(description="Koopman Delay-Embedded EDMD Alpha Estimator")
    parser.add_argument("--input", type=str, help="Path to input .npz trajectory")
    parser.add_argument("--synthetic", action="store_true", help="Run on synthetic data (control test)")
    parser.add_argument("--synthetic-N", type=int, default=5000000, help="Number of synthetic samples (default 5M)")
    parser.add_argument("--lags", type=str, default="1,2,5,10,20,50", help="Comma-separated Koopman lags tau to scan")
    parser.add_argument("--delays", type=int, default=16, help="Hankel delay embedding horizon h")
    parser.add_argument("--ridge", type=float, default=1e-8, help="Ridge regularization")
    parser.add_argument("--bootstrap", type=int, default=0, help="Bootstrap iterations")
    parser.add_argument("--block-size", type=int, default=5000, help="Bootstrap block size")
    parser.add_argument("--allow-rejected", action="store_true",
                        help="Diagnostic override: run the estimator even on a trajectory "
                             "the scale-context gate marks inadmissible (NOT a public readout)")
    return parser.parse_args()

def generate_synthetic_trajectory(N=5000000, d=2, mu_target=TARGET_MU_PLUS):
    np.random.seed(42)
    A = np.array([[mu_target, 0.001],
                  [0.0,      0.5]])
    X = np.zeros((N, d))
    X[0] = np.array([1.0, 1.0])
    cov = np.array([[0.01, 0.0],
                    [0.0, 0.05]])
    noise = np.random.multivariate_normal([0, 0], cov, size=N)
    for t in range(1, N):
        X[t] = A @ X[t-1] + noise[t]
    return X

def make_delay_embedding(X, h):
    """
    Creates Hankel matrix of shape (N - h + 1, d * h)
    Row t is [X_t, X_{t-1}, ..., X_{t-h+1}]
    """
    N, d = X.shape
    if h <= 1:
        return X
    
    X_embed = np.zeros((N - h + 1, d * h))
    for i in range(h):
        # i=0 gives X[h-1:], i=1 gives X[h-2 : -1], etc.
        start = h - 1 - i
        end = N - i
        X_embed[:, i*d:(i+1)*d] = X[start:end]
    return X_embed

def compute_edmd(X, Y, ridge=1e-8):
    N = X.shape[0]
    d = X.shape[1]
    
    C00 = (X.T @ X) / N
    C01 = (X.T @ Y) / N
    
    C00_reg = C00 + ridge * np.eye(d)
    K = np.linalg.solve(C00_reg, C01)
    return K.T  # Transpose to get left-multiplication on column vectors K v = mu v

def extract_alpha(K):
    eigvals, _ = np.linalg.eig(K)
    sorted_idx = np.argsort(np.abs(eigvals))[::-1]
    sorted_eigs = eigvals[sorted_idx]
    
    mu_plus = None
    for mu in sorted_eigs:
        if np.abs(mu) < 0.99999:
            mu_plus = mu
            break
            
    if mu_plus is None:
        return None, None
        
    alpha_inv = 1.0 / np.abs(1.0 - mu_plus)
    return mu_plus, alpha_inv

def main():
    args = parse_args()
    
    print("="*60)
    print("FTD STOCHASTIC KOOPMAN ALPHA ESTIMATOR (DELAY-EMBEDDED)")
    print("="*60)
    
    if args.synthetic:
        print(f"[*] Generating synthetic trajectory (N={args.synthetic_N})...")
        features = generate_synthetic_trajectory(N=args.synthetic_N)
    elif args.input:
        if not os.path.exists(args.input):
            print(f"[!] Error: File {args.input} not found.")
            return
        print(f"[*] Loading trajectory from {args.input}...")
        data = np.load(args.input)
        features = data["features"]

        # ---- Readout admissibility gate (C_scale) -------------------------
        # Persistence is necessary but not sufficient: public readout requires
        # persistence, self-confinement, AND scale separation. Refuse to emit a
        # public alpha from a trajectory whose cloud is not scale-separated /
        # self-confined / stationary unless an explicit diagnostic override is
        # passed. See docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md.
        admissible, status, detail = assess_admissibility(data)
        print(f"[*] Scale-context readout status: {status}")
        print(f"[*]   detail: {detail}")
        if admissible is None:
            print("[!] WARNING: trajectory carries no scale-context columns; "
                  "admissibility could NOT be assessed. Re-run the dumper with the "
                  "sc_* annotation to enable the gate. Proceeding (no verdict).")
        elif not admissible:
            if not args.allow_rejected:
                print("="*60)
                print(f"[X] READOUT REFUSED: trajectory is {status}.")
                print("    The cloud is not eligible for PUBLIC physical readout.")
                print("    Pass --allow-rejected to run the estimator anyway as a")
                print("    DIAGNOSTIC ONLY (the result is NOT a public alpha readout).")
                print("="*60)
                return
            print("="*60)
            print(f"[!] OVERRIDE: estimator running on a {status} trajectory.")
            print("    This is DIAGNOSTIC ONLY and is NOT a public alpha readout.")
            print("="*60)
        else:
            print("[*] Trajectory is ADMISSIBLE for public readout.")
    else:
        print("[!] Error: Must provide --input or --synthetic flag.")
        return
        
    print(f"[*] Raw trajectory length: {features.shape[0]}")
    print(f"[*] Raw observation dim:   {features.shape[1]}")
    
    h = args.delays
    print(f"[*] Delay embedding h:     {h}")
    X_embed = make_delay_embedding(features, h)
    print(f"[*] Embedded trajectory:   {X_embed.shape[0]}")
    print(f"[*] Embedded dimension:    {X_embed.shape[1]}")
    
    N_total = X_embed.shape[0]
    lags_to_scan = [int(x) for x in args.lags.split(",")]
    
    print(f"[*] Lags to scan:          {lags_to_scan}")
    print(f"[*] Target alpha^{{-1}}:        {TARGET_ALPHA_INV:.6f}")
    
    taus = []
    timescales = []
    alpha_invs = []
    
    for lag in lags_to_scan:
        print(f"\n--- LAG tau = {lag} ---")
        if lag >= N_total:
            print("[!] Lag too large for trajectory length.")
            continue
            
        X = X_embed[:-lag]
        Y = X_embed[lag:]
        
        K_red = compute_edmd(X, Y, ridge=args.ridge)
        mu_plus_tau, _ = extract_alpha(K_red)
        
        if mu_plus_tau is None:
            print("[!] Failed to extract nontrivial eigenvalue.")
            continue
            
        magnitude = np.abs(mu_plus_tau) ** (1.0 / lag)
        phase = np.angle(mu_plus_tau) / lag
        mu_plus = magnitude * np.exp(1j * phase)
        
        alpha_inv = 1.0 / np.abs(1.0 - mu_plus)
        implied_timescale = -lag / np.log(np.abs(mu_plus_tau))
        
        taus.append(lag)
        timescales.append(implied_timescale)
        alpha_invs.append(alpha_inv)
        
        print(f"Leading nontrivial mu(tau): {mu_plus_tau}")
        print(f"Implied 1-tick mu+:       {mu_plus}")
        print(f"Implied timescale t(tau): {implied_timescale:.4f} ticks")
        print(f"Implied alpha^{{-1}}:          {alpha_inv:.6f}")
        
    if len(taus) > 0:
        plt.figure(figsize=(10, 6))
        plt.plot(taus, timescales, 'o-', linewidth=2, label="Implied Timescale")
        plt.axhline(y=136.5, color='r', linestyle='--', label=r"Target $t_\alpha \approx 136.5$")
        plt.title(f"Koopman Implied Timescale vs Lag $\\tau$ (Delay $h={h}$)")
        plt.xlabel("Lag $\\tau$")
        plt.ylabel("Implied Timescale (ticks)")
        plt.legend()
        plt.grid(True)
        plt.savefig(f"timescale_plateau_h{h}.png")
        print(f"\n[*] Plot saved to timescale_plateau_h{h}.png")
        
    print("============================================================")

if __name__ == "__main__":
    main()