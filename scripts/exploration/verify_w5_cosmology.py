"""
verify_w5_cosmology.py — W5 Moore-Shell DM Weighting independent cosmological confirmation.

Pre-registered campaign ID: FTD-0211.
This script implements the numerical verification of the W5 per-site weighting
scheme against BBN Helium abundance (Y_p) and CMB acoustic peak scale (l_1).
"""

import sys
from mpmath import mp, mpf

# Prevent Windows console encoding issues when printing symbols
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

# Set high precision
mp.dps = 50

# ── Step 1: Define Observables & Targets (Definition D3, D4, D5) ──────
# Planck 2018 Observables from pre-registration:
h = mpf('0.674')              # Hubble parameter h = 0.674
omega_m_val = mpf('1') / 3    # Omega_m = 1/3 (based on Lambda = 2/3 selection)
omega_m_h2 = omega_m_val * h**2

# Observed values:
Y_p_obs = mpf('0.245')
sigma_Y_p = mpf('0.003')

l1_obs = mpf('220.0')
sigma_l1 = mpf('1.0')

def run_cosmology_calculation(name, ratio_b_m):
    """
    Run the BBN and CMB calculations for a given baryon-to-matter ratio.
    """
    # Definition D3: physical baryon density Omega_b h^2
    omega_b = ratio_b_m * omega_m_val
    omega_b_h2 = ratio_b_m * omega_m_h2
    
    # Definition D4: BBN helium mass fraction Y_p
    eta_10 = mpf('273.9') * omega_b_h2
    Y_p_pred = mpf('0.2467') + mpf('0.009') * (eta_10 - mpf('6.0'))
    
    # Definition D5: CMB acoustic peak position l_1
    l1_pred = mpf('220.0') * (omega_b_h2 / mpf('0.0224'))**mpf('-0.1') * (omega_m_h2 / mpf('0.142'))**mpf('0.25')
    
    # Residuals
    Y_p_resid = abs(Y_p_pred - Y_p_obs) / Y_p_obs
    l1_resid = abs(l1_pred - l1_obs) / l1_obs
    
    # Sigma exclusions
    Y_p_sigma = (Y_p_pred - Y_p_obs) / sigma_Y_p
    l1_sigma = (l1_pred - l1_obs) / sigma_l1
    
    return {
        'omega_b': omega_b,
        'omega_b_h2': omega_b_h2,
        'eta_10': eta_10,
        'Y_p_pred': Y_p_pred,
        'Y_p_resid': Y_p_resid,
        'Y_p_sigma': Y_p_sigma,
        'l1_pred': l1_pred,
        'l1_resid': l1_resid,
        'l1_sigma': l1_sigma
    }

# ── Step 2: Compute W1 Baseline (Definition D1) ──────────────────────
# W1 baryon fraction of matter is 10/27
w1_results = run_cosmology_calculation("W1 Uniform", mpf('10') / mpf('27'))

# ── Step 3: Compute W5 Active Hypothesis (Definition D2) ──────────────
# W5 baryon fraction of matter is 10/63
w5_results = run_cosmology_calculation("W5 Cuboctahedron-weighted", mpf('10') / mpf('63'))

# ── Step 4: Print Detailed Results ───────────────────────────────────
print("==========================================================================")
print("FTD-0211: W5 MOORE-SHELL DM WEIGHTING INDEPENDENT COSMOLOGICAL VERIFICATION")
print("==========================================================================")
print(f"Planck 2018 Parameters: h = {float(h):.4f}, Omega_m = {float(omega_m_val):.6f}")
print(f"Observed Targets:       Y_p = {float(Y_p_obs):.3f} +/- {float(sigma_Y_p):.3f}")
print(f"                        l_1 = {float(l1_obs):.1f} +/- {float(sigma_l1):.1f}")
print("--------------------------------------------------------------------------")

print("\n--- W1: Uniform voxel-counting (baseline) ---")
print(f"  Baryon fraction of matter (Omega_b/Omega_m): {10}/{27} ≈ {float(mpf('10')/27):.6f}")
print(f"  Omega_b:                                     {float(w1_results['omega_b']):.6f}")
print(f"  Omega_b h^2:                                 {float(w1_results['omega_b_h2']):.6f}")
print(f"  eta_10:                                      {float(w1_results['eta_10']):.4f}")
print(f"  Predicted Y_p:                               {float(w1_results['Y_p_pred']):.6f}")
print(f"    Residual deviation:                        {float(w1_results['Y_p_resid'])*100:.4f}%")
print(f"    Sigma deviation:                           {float(w1_results['Y_p_sigma']):+.2f} sigma")
print(f"  Predicted l_1:                               {float(w1_results['l1_pred']):.4f}")
print(f"    Residual deviation:                        {float(w1_results['l1_resid'])*100:.4f}%")
print(f"    Sigma deviation:                           {float(w1_results['l1_sigma']):+.2f} sigma")

print("\n--- W5: Cuboctahedron-weighted (active) ---")
print(f"  Baryon fraction of matter (Omega_b/Omega_m): {10}/{63} ≈ {float(mpf('10')/63):.6f}")
print(f"  Omega_b:                                     {float(w5_results['omega_b']):.6f}")
print(f"  Omega_b h^2:                                 {float(w5_results['omega_b_h2']):.6f}")
print(f"  eta_10:                                      {float(w5_results['eta_10']):.4f}")
print(f"  Predicted Y_p:                               {float(w5_results['Y_p_pred']):.6f}")
print(f"    Residual deviation:                        {float(w5_results['Y_p_resid'])*100:.4f}%")
print(f"    Sigma deviation:                           {float(w5_results['Y_p_sigma']):+.2f} sigma")
print(f"  Predicted l_1:                               {float(w5_results['l1_pred']):.4f}")
print(f"    Residual deviation:                        {float(w5_results['l1_resid'])*100:.4f}%")
print(f"    Sigma deviation:                           {float(w5_results['l1_sigma']):+.2f} sigma")

print("\n--------------------------------------------------------------------------")

# Evaluate thresholds
w5_Y_p_resid_pct = float(w5_results['Y_p_resid']) * 100
w5_l1_resid_pct = float(w5_results['l1_resid']) * 100

w1_Y_p_sigma_abs = abs(float(w1_results['Y_p_sigma']))
w1_l1_sigma_abs = abs(float(w1_results['l1_sigma']))

print("EVALUATING PRE-REGISTERED OUTCOMES:")
print(f"  W5 Y_p Residual: {w5_Y_p_resid_pct:.4f}% (threshold: <= 1.5% for Outcome A, <= 5.0% for Outcome B)")
print(f"  W5 l_1 Residual: {w5_l1_resid_pct:.4f}% (threshold: <= 1.5% for Outcome A, <= 5.0% for Outcome B)")
print(f"  W1 Y_p Exclusion: {w1_Y_p_sigma_abs:.2f} sigma (threshold: > 5 sigma exclusion)")
print(f"  W1 l_1 Exclusion: {w1_l1_sigma_abs:.2f} sigma (threshold: > 5 sigma exclusion)")

if w5_Y_p_resid_pct <= 1.5 and w5_l1_resid_pct <= 1.5 and w1_Y_p_sigma_abs > 5.0 and w1_l1_sigma_abs > 5.0:
    verdict = "Outcome A (FOUND)"
    desc = "The W5 weighting predicts both Y_p and l_1 within a 1.5% deviation threshold, while W1 is strongly excluded."
elif w5_Y_p_resid_pct <= 5.0 and w5_l1_resid_pct <= 5.0 and (w1_Y_p_sigma_abs > 5.0 or w1_l1_sigma_abs > 5.0):
    verdict = "Outcome B (UNDERDETERMINED)"
    desc = "The W5 weighting predicts both observables within a 5.0% deviation, but does not sharply satisfy the 1.5% threshold for both, or W1 is not strongly excluded."
else:
    verdict = "Outcome C (CLOSED-NEGATIVE)"
    desc = "The W5 weighting deviates by > 5% on either observable. W5 is retired as a post-hoc coincidence."

print(f"\nFINAL VERDICT: {verdict}")
print(f"DESCRIPTION:   {desc}")
print("==========================================================================")
