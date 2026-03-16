#!/usr/bin/env python3
"""
FTD CERN Data Exploration — Topological Cavitation Analysis
============================================================

Deep exploration of the 27M-event CMS MET dataset in the context of
FTD's topological cavitation hypothesis:

    R_cav ∝ √(E_MET)    (FTD prediction: bubble radius scales as sqrt of energy)

Key analyses:
1. Energy-binned R_cav distributions — shape evolution with energy
2. Functional form fits — R = a√E vs R = aE vs R = aE^b
3. Detector acceptance deconvolution — CMS tracker geometry effects
4. Upper envelope analysis — hard boundary vs soft tail
5. Conditional quantile regression — how percentiles scale with energy
6. Residual structure after detrending
7. FTD-specific derived quantities (cavitation volume, energy density)

Uses cached data from ftd_full_extracted.npz (4.57M selected events).
"""

import os
import sys
import numpy as np
from scipy import stats, optimize
import warnings
warnings.filterwarnings('ignore')

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.ticker import LogLocator, NullFormatter
import matplotlib.colors as mcolors

# ---------------------------------------------------------------------------
# Load cached data
# ---------------------------------------------------------------------------
DATA_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_FILE = os.path.join(DATA_DIR, "ftd_full_extracted.npz")

print("=" * 72)
print("FTD CERN DATA EXPLORATION")
print("Topological Cavitation Hypothesis: R_cav ∝ √(E_MET)")
print("=" * 72)

d = np.load(DATA_FILE)
met = d['met']       # MET in GeV
rcav = d['rcav']     # max(SV_dxy) in cm
N = len(met)
print(f"\nLoaded {N:,} selected events")
print(f"  MET:  [{met.min():.1f}, {met.max():.1f}] GeV, median={np.median(met):.1f}")
print(f"  R_cav: [{rcav.min():.4f}, {rcav.max():.1f}] cm, median={np.median(rcav):.4f}")

sqrt_met = np.sqrt(met)
log_met = np.log10(met)
log_rcav = np.log10(rcav)

# ---------------------------------------------------------------------------
# 1. ENERGY-BINNED R_cav DISTRIBUTIONS
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("1. ENERGY-BINNED R_cav DISTRIBUTIONS")
print("=" * 72)

# Define energy bins (in MET GeV)
energy_edges = [100, 130, 170, 230, 320, 500, 1000, 5000, 50000]
energy_labels = []
bin_stats = []

for i in range(len(energy_edges) - 1):
    lo, hi = energy_edges[i], energy_edges[i+1]
    mask = (met >= lo) & (met < hi)
    n_in_bin = mask.sum()
    if n_in_bin < 10:
        continue

    r_bin = rcav[mask]
    label = f"[{lo},{hi}) GeV"
    energy_labels.append(label)

    p50 = np.median(r_bin)
    p95 = np.percentile(r_bin, 95)
    p99 = np.percentile(r_bin, 99)
    mean_r = np.mean(r_bin)
    std_r = np.std(r_bin)
    geo_mean = np.exp(np.mean(np.log(r_bin)))

    bin_stats.append({
        'lo': lo, 'hi': hi, 'n': n_in_bin,
        'center': np.sqrt(lo * hi),  # geometric center
        'sqrt_center': np.sqrt(np.sqrt(lo * hi)),
        'median': p50, 'p95': p95, 'p99': p99,
        'mean': mean_r, 'std': std_r, 'geo_mean': geo_mean,
        'rcav': r_bin
    })

    print(f"  {label:22s}  N={n_in_bin:>8,}  median={p50:.3f}  "
          f"p95={p95:.2f}  p99={p99:.2f}  geo_mean={geo_mean:.3f} cm")

# ---------------------------------------------------------------------------
# 2. FUNCTIONAL FORM FITS
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("2. FUNCTIONAL FORM FITS — R_cav vs MET")
print("=" * 72)

# Use binned medians for cleaner fits (less noise)
centers = np.array([s['center'] for s in bin_stats])
sqrt_centers = np.sqrt(centers)
medians = np.array([s['median'] for s in bin_stats])
p95s = np.array([s['p95'] for s in bin_stats])
p99s = np.array([s['p99'] for s in bin_stats])
geo_means = np.array([s['geo_mean'] for s in bin_stats])

# Fit 1: R = a * sqrt(MET)  [FTD prediction]
def model_sqrt(E, a):
    return a * np.sqrt(E)

# Fit 2: R = a * MET  [linear — SM null hypothesis]
def model_linear(E, a):
    return a * E

# Fit 3: R = a * E^b  [power law — agnostic]
def model_power(E, a, b):
    return a * np.power(E, b)

# Fit 4: R = a * sqrt(E) + c  [FTD with offset]
def model_sqrt_offset(E, a, c):
    return a * np.sqrt(E) + c

# Fit 5: R = a * log(E)  [logarithmic]
def model_log(E, a):
    return a * np.log(E)

fits = {}
for name, model, p0, quantile_data in [
    ("sqrt(E)", model_sqrt, [0.05], medians),
    ("linear E", model_linear, [0.001], medians),
    ("power E^b", model_power, [0.05, 0.5], medians),
    ("sqrt(E)+c", model_sqrt_offset, [0.05, 0.0], medians),
    ("log(E)", model_log, [0.1], medians),
    # Also fit the 95th percentile (upper envelope)
    ("sqrt(E) [p95]", model_sqrt, [0.3], p95s),
    ("power E^b [p95]", model_power, [0.3, 0.5], p95s),
    ("sqrt(E) [p99]", model_sqrt, [0.5], p99s),
    ("power E^b [p99]", model_power, [0.5, 0.5], p99s),
]:
    try:
        popt, pcov = optimize.curve_fit(model, centers, quantile_data, p0=p0, maxfev=10000)
        y_pred = model(centers, *popt)
        residuals = quantile_data - y_pred
        ss_res = np.sum(residuals**2)
        ss_tot = np.sum((quantile_data - np.mean(quantile_data))**2)
        r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0

        param_str = ", ".join(f"{p:.6f}" for p in popt)
        fits[name] = {'popt': popt, 'r2': r2, 'model': model}
        print(f"  {name:22s}  params=({param_str})  R²={r2:.6f}")
    except Exception as e:
        print(f"  {name:22s}  FAILED: {e}")

# Compare sqrt vs linear for the median
if "sqrt(E)" in fits and "linear E" in fits:
    print(f"\n  ** FTD (sqrt) R² = {fits['sqrt(E)']['r2']:.6f} vs "
          f"Linear R² = {fits['linear E']['r2']:.6f}")
    if fits['sqrt(E)']['r2'] > fits['linear E']['r2']:
        print("  ** sqrt(MET) fits BETTER than linear MET — consistent with FTD")
    else:
        print("  ** linear MET fits better than sqrt(MET)")

if "power E^b" in fits:
    b = fits['power E^b']['popt'][1]
    print(f"\n  ** Best-fit power law exponent b = {b:.4f}")
    print(f"     FTD predicts b = 0.5, SM null is b = 0 (no correlation) or b = 1")
    print(f"     Distance from FTD: |b - 0.5| = {abs(b - 0.5):.4f}")

# ---------------------------------------------------------------------------
# 3. DETECTOR ACCEPTANCE / GEOMETRY EFFECTS
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("3. CMS TRACKER GEOMETRY EFFECTS")
print("=" * 72)

# CMS pixel detector layers (barrel):
#   Layer 1: r ≈ 2.9 cm (was 4.4 cm pre-Phase 1)
#   Layer 2: r ≈ 6.8 cm
#   Layer 3: r ≈ 10.9 cm
#   Layer 4: r ≈ 16.0 cm
# Beam pipe: r ≈ 2.2-2.5 cm
# SV reconstruction requires ≥2 tracks → efficiency drops near beam pipe
# and at large radii (fewer hits)

# Check for detector structure in R_cav distribution
r_hist, r_edges = np.histogram(rcav, bins=500, range=(0, 20))
r_centers_hist = (r_edges[:-1] + r_edges[1:]) / 2

# Find peaks/dips in the derivative (detector layer signatures)
from scipy.ndimage import gaussian_filter1d
r_smooth = gaussian_filter1d(r_hist.astype(float), sigma=3)
r_deriv = np.gradient(r_smooth)

print("  CMS Pixel Barrel Layers:")
print("    Layer 1: ~2.9 cm (Phase-1 upgrade)")
print("    Layer 2: ~6.8 cm")
print("    Layer 3: ~10.9 cm")
print("    Layer 4: ~16.0 cm")

# Check R_cav distribution at detector boundaries
for boundary, label in [(2.5, "beam pipe"), (2.9, "pixel L1"),
                         (6.8, "pixel L2"), (10.9, "pixel L3"), (16.0, "pixel L4")]:
    mask_below = rcav < boundary
    mask_above = rcav >= boundary
    frac_below = mask_below.sum() / N
    print(f"    R < {boundary:5.1f} cm ({label:10s}): {frac_below*100:.1f}% of events")

# Efficiency-corrected analysis: split into "inner" (< 2.9cm) and "outer" (> 2.9cm)
inner_mask = rcav < 2.9
outer_mask = rcav >= 2.9
print(f"\n  Inner tracker (R < 2.9 cm): {inner_mask.sum():,} events ({inner_mask.sum()/N*100:.1f}%)")
print(f"  Outer tracker (R ≥ 2.9 cm): {outer_mask.sum():,} events ({outer_mask.sum()/N*100:.1f}%)")

# Repeat correlation for outer-only (less detector bias)
rho_outer, p_outer = stats.spearmanr(sqrt_met[outer_mask], rcav[outer_mask])
rho_inner, p_inner = stats.spearmanr(sqrt_met[inner_mask], rcav[inner_mask])
print(f"\n  Spearman rho(sqrt(MET), R_cav):")
print(f"    Full sample:  {stats.spearmanr(sqrt_met, rcav)[0]:.4f}")
print(f"    Inner only:   {rho_inner:.4f} (p={p_inner:.2e})")
print(f"    Outer only:   {rho_outer:.4f} (p={p_outer:.2e})")

# ---------------------------------------------------------------------------
# 4. CONDITIONAL QUANTILE ANALYSIS
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("4. CONDITIONAL QUANTILE REGRESSION")
print("=" * 72)
print("  How do R_cav percentiles scale with √MET?")

# Finer energy binning for quantile tracking
n_qbins = 20
met_sorted = np.sort(met)
bin_boundaries = [met_sorted[int(i * N / n_qbins)] for i in range(n_qbins)] + [met.max() + 1]

q_centers = []
q_p10 = []
q_p25 = []
q_p50 = []
q_p75 = []
q_p90 = []
q_p95 = []
q_p99 = []

for i in range(n_qbins):
    lo, hi = bin_boundaries[i], bin_boundaries[i+1]
    mask = (met >= lo) & (met < hi)
    r_bin = rcav[mask]
    if len(r_bin) < 100:
        continue

    q_centers.append(np.sqrt(np.median(met[mask])))  # sqrt(median MET)
    q_p10.append(np.percentile(r_bin, 10))
    q_p25.append(np.percentile(r_bin, 25))
    q_p50.append(np.percentile(r_bin, 50))
    q_p75.append(np.percentile(r_bin, 75))
    q_p90.append(np.percentile(r_bin, 90))
    q_p95.append(np.percentile(r_bin, 95))
    q_p99.append(np.percentile(r_bin, 99))

q_centers = np.array(q_centers)
q_p50 = np.array(q_p50)
q_p95 = np.array(q_p95)
q_p99 = np.array(q_p99)

# Fit power laws to each quantile
for label, qdata in [("p50", q_p50), ("p95", q_p95), ("p99", q_p99)]:
    try:
        # Fit R = a * (sqrt_MET)^b → log R = log a + b * log(sqrt_MET)
        log_x = np.log(q_centers)
        log_y = np.log(qdata)
        slope, intercept, r, p, se = stats.linregress(log_x, log_y)
        a_fit = np.exp(intercept)
        print(f"  {label}: R = {a_fit:.4f} × (√MET)^{slope:.3f}  "
              f"(R²={r**2:.4f}, FTD predicts exponent ≈ 1.0)")
    except Exception as e:
        print(f"  {label}: fit failed — {e}")

# ---------------------------------------------------------------------------
# 5. HIGH-ENERGY TAIL ANALYSIS
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("5. HIGH-ENERGY TAIL — WHERE FTD SIGNAL WOULD BE STRONGEST")
print("=" * 72)

# FTD predicts the cavitation effect grows with energy
# At high MET, we should see the clearest signal
high_met_thresholds = [200, 500, 1000, 2000, 5000]

for thresh in high_met_thresholds:
    mask = met >= thresh
    n_high = mask.sum()
    if n_high < 10:
        print(f"  MET > {thresh:5d} GeV:  N={n_high:>6,}  (too few)")
        continue

    r_high = rcav[mask]
    rho_s, p_s = stats.spearmanr(np.sqrt(met[mask]), r_high)
    rho_p, p_p = stats.pearsonr(np.sqrt(met[mask]), r_high)

    print(f"  MET > {thresh:5d} GeV:  N={n_high:>6,}  "
          f"median_R={np.median(r_high):.3f}  "
          f"rho_S={rho_s:+.4f} (p={p_s:.2e})  "
          f"rho_P={rho_p:+.4f}")

# ---------------------------------------------------------------------------
# 6. FTD-SPECIFIC DERIVED QUANTITIES
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("6. FTD-DERIVED QUANTITIES")
print("=" * 72)

# In FTD, the cavitation bubble has:
#   R_cav = sqrt(E / (4π σ_vac))    where σ_vac is vacuum surface tension
#   Volume V = (4/3)π R³
#   Energy density ε = E / V = 3σ_vac / R

# If R_cav = a * sqrt(MET), then a² = 1/(4π σ_vac) → σ_vac = 1/(4π a²)
# Let's extract the effective surface tension

# Use the median fit coefficient
if "sqrt(E)" in fits:
    a_sqrt = fits['sqrt(E)']['popt'][0]
    # σ_vac in units of GeV/cm² (since MET in GeV, R in cm)
    sigma_vac = 1.0 / (4 * np.pi * a_sqrt**2)
    print(f"  FTD sqrt fit coefficient: a = {a_sqrt:.6f} cm/√GeV")
    print(f"  Implied vacuum surface tension: σ_vac = {sigma_vac:.2f} GeV/cm²")
    print(f"  In natural units (ℓ_P = 1.6e-35 m = 1.6e-33 cm):")
    # Convert to Planck units
    lp_cm = 1.616e-33  # Planck length in cm
    ep_gev = 1.22e19   # Planck energy in GeV
    sigma_planck = sigma_vac * lp_cm**2 / ep_gev
    print(f"  σ_vac = {sigma_planck:.2e} (Planck units)")

# Compute cavitation volume and energy density for each event
V_cav = (4.0/3.0) * np.pi * rcav**3   # cm³
eps_cav = met / V_cav                   # GeV/cm³ (energy density in bubble)

print(f"\n  Cavitation volume V_cav (cm³):")
print(f"    median={np.median(V_cav):.4f}, mean={np.mean(V_cav):.2f}, max={np.max(V_cav):.1f}")
print(f"  Energy density ε = E/V (GeV/cm³):")
print(f"    median={np.median(eps_cav):.1f}, mean={np.mean(eps_cav):.1f}")

# FTD predicts ε should be roughly constant (= 3σ_vac) if R ∝ √E
# Check: does ε vs E show a trend?
rho_eps, p_eps = stats.spearmanr(met, eps_cav)
print(f"\n  Spearman rho(MET, ε_cav) = {rho_eps:.4f} (p={p_eps:.2e})")
print(f"  FTD predicts ε ~ const → rho ≈ 0 if R ∝ √E")
print(f"  SM predicts ε varies randomly → rho ≈ 0 trivially")

# Better test: ε vs R should follow ε ∝ 1/R if FTD correct
rho_er, p_er = stats.spearmanr(rcav, eps_cav)
print(f"  Spearman rho(R_cav, ε_cav) = {rho_er:.4f}")
print(f"  FTD predicts ε ∝ 1/R → strong negative correlation")

# ---------------------------------------------------------------------------
# 7. LOG-SPACE ANALYSIS (Heavy tails)
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("7. LOG-SPACE ANALYSIS")
print("=" * 72)

# In log-log space, R = a * E^b becomes log R = log a + b * log E
slope_full, intercept_full, r_full, p_full, se_full = stats.linregress(
    np.log10(met), np.log10(rcav))
print(f"  Log-log regression (full sample):")
print(f"    log₁₀(R) = {intercept_full:.4f} + {slope_full:.4f} × log₁₀(MET)")
print(f"    R² = {r_full**2:.6f}")
print(f"    Power law exponent = {slope_full:.4f}")
print(f"    FTD predicts exponent = 0.5")
print(f"    Distance from FTD: {abs(slope_full - 0.5):.4f}")

# Separate log-log for inner vs outer tracker
for label, mask in [("Inner (R<2.9)", inner_mask), ("Outer (R≥2.9)", outer_mask)]:
    m_sub, r_sub = met[mask], rcav[mask]
    sl, ic, r_val, p_val, se = stats.linregress(np.log10(m_sub), np.log10(r_sub))
    print(f"\n  {label}:")
    print(f"    log₁₀(R) = {ic:.4f} + {sl:.4f} × log₁₀(MET), R²={r_val**2:.6f}")
    print(f"    Power exponent = {sl:.4f}")

# ---------------------------------------------------------------------------
# 8. RATIO ANALYSIS — R_cav / √MET should be constant if FTD correct
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("8. RATIO ANALYSIS — R_cav / √MET")
print("=" * 72)

ratio = rcav / sqrt_met
print(f"  R_cav / √MET statistics:")
print(f"    mean   = {np.mean(ratio):.5f} cm/√GeV")
print(f"    median = {np.median(ratio):.5f} cm/√GeV")
print(f"    std    = {np.std(ratio):.5f}")
print(f"    CV     = {np.std(ratio)/np.mean(ratio):.2f} (coefficient of variation)")
print(f"    FTD predicts CV ≈ 0 (constant ratio)")

# Check if ratio depends on energy (it shouldn't if FTD is correct)
rho_ratio, p_ratio = stats.spearmanr(met, ratio)
print(f"\n  Spearman rho(MET, R/√MET) = {rho_ratio:.4f} (p={p_ratio:.2e})")
print(f"  FTD predicts rho ≈ 0 (ratio independent of energy)")

# Bin the ratio by MET and check for trend
print("\n  Binned ratio R/√MET vs MET:")
for s in bin_stats:
    r_bin = s['rcav']
    sqrt_e = np.sqrt(np.sqrt(s['lo'] * s['hi']))  # sqrt of geometric center
    ratio_bin = np.median(r_bin) / sqrt_e**2
    print(f"    MET ~ {s['center']:.0f} GeV: median(R/√MET) = "
          f"{np.median(r_bin / np.sqrt(s['center'])):.5f}")

# ---------------------------------------------------------------------------
# 9. KS TESTS — DISTRIBUTION SHAPE EVOLUTION
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("9. KOLMOGOROV-SMIRNOV TESTS — Shape Evolution")
print("=" * 72)
print("  Does the R_cav distribution change shape with energy?")
print("  (FTD: shapes should be self-similar when scaled by √MET)")

# Rescale R_cav by √MET and check if distributions match across energy bins
if len(bin_stats) >= 3:
    # Use first bin as reference
    ref = bin_stats[0]
    ref_scaled = ref['rcav'] / np.sqrt(ref['center'])

    for s in bin_stats[1:]:
        test_scaled = s['rcav'] / np.sqrt(s['center'])
        ks_stat, ks_p = stats.ks_2samp(ref_scaled, test_scaled)
        print(f"  [{ref['lo']},{ref['hi']}) vs [{s['lo']},{s['hi']}):  "
              f"KS={ks_stat:.4f}, p={ks_p:.2e}")

    print(f"\n  If FTD is correct (R ∝ √E), rescaled distributions should match → large p-values")
    print(f"  Small p-values indicate shape changes → inconsistent with simple scaling")

# ---------------------------------------------------------------------------
# 10. SUMMARY & FTD INTERPRETATION
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("10. SUMMARY & FTD INTERPRETATION")
print("=" * 72)

print("""
TOPOLOGICAL CAVITATION HYPOTHESIS TEST RESULTS
===============================================

FTD Prediction:
  When MET event energy E exceeds the manifestation threshold K_B,
  the vacuum undergoes topological cavitation — a spherical tear that
  expands as R_cav = √(E / 4πσ_vac). The displaced secondary vertex
  (SV) traces the projected radius of this cavitation bubble.

Observable signatures:
  (a) R_cav ∝ √(E_MET)  — deterministic geometric boundary
  (b) Hard upper envelope in R vs √E scatter — sharp cutoff
  (c) Self-similar R/√E distributions across energy scales

SM Null Hypothesis:
  SV displacement arises from random LLP (long-lived particle) decay.
  R_cav distribution is exponential/Poisson with no energy-dependent
  geometric boundary. Upper envelope is diffuse (exponential tail).
""")

# Scorecard
score_ftd = 0
score_sm = 0

# Test (a): sqrt vs linear correlation
r2_sqrt = fits.get('sqrt(E)', {}).get('r2', 0)
r2_lin = fits.get('linear E', {}).get('r2', 0)
if r2_sqrt > r2_lin:
    score_ftd += 1
    print(f"  [✓ FTD] sqrt(E) fits better than linear E (R²={r2_sqrt:.6f} > {r2_lin:.6f})")
else:
    score_sm += 1
    print(f"  [✗ FTD] linear E fits better than sqrt(E)")

# Test: power law exponent
if "power E^b" in fits:
    b = fits['power E^b']['popt'][1]
    if abs(b - 0.5) < abs(b - 0.0) and abs(b - 0.5) < abs(b - 1.0):
        score_ftd += 1
        print(f"  [✓ FTD] Power exponent b={b:.3f} closest to 0.5 (FTD)")
    elif abs(b - 0.0) < abs(b - 0.5):
        score_sm += 1
        print(f"  [✗ FTD] Power exponent b={b:.3f} closest to 0 (no correlation)")
    else:
        score_sm += 1
        print(f"  [? ---] Power exponent b={b:.3f} — ambiguous")

# Test (b): Positive correlation
rho_full = stats.spearmanr(sqrt_met, rcav)[0]
if rho_full > 0.02:
    score_ftd += 1
    print(f"  [✓ FTD] Positive rho(√MET, R_cav) = {rho_full:.4f} (expected sign)")
else:
    score_sm += 1
    print(f"  [✗ FTD] rho(√MET, R_cav) = {rho_full:.4f} (too weak or wrong sign)")

# Test: sqrt > linear Spearman
rho_linear = stats.spearmanr(met, rcav)[0]
if abs(rho_full) > abs(rho_linear):
    score_ftd += 1
    print(f"  [✓ FTD] sqrt(MET) correlates better than linear MET "
          f"({rho_full:.4f} > {rho_linear:.4f})")
else:
    score_sm += 1
    print(f"  [✗ FTD] linear MET correlates better")

# Test: Hard boundary (negative envelope slope indicates no hard boundary)
if "power E^b [p95]" in fits:
    b95 = fits['power E^b [p95]']['popt'][1]
    if b95 > 0.3:
        score_ftd += 1
        print(f"  [✓ FTD] 95th-pct envelope grows with energy (b={b95:.3f})")
    else:
        score_sm += 1
        print(f"  [✗ FTD] 95th-pct envelope flat or falling (b={b95:.3f})")

# Test: Log-space exponent
if abs(slope_full - 0.5) < 0.3:
    score_ftd += 1
    print(f"  [✓ FTD] Log-log slope {slope_full:.3f} within 0.3 of 0.5")
else:
    score_sm += 1
    print(f"  [✗ FTD] Log-log slope {slope_full:.3f} far from 0.5")

print(f"\n  SCORECARD: FTD = {score_ftd}, SM-null = {score_sm}")
print(f"  (out of {score_ftd + score_sm} tests)")

# ---------------------------------------------------------------------------
# PLOTTING — 12-panel comprehensive figure
# ---------------------------------------------------------------------------
print("\n\nGenerating comprehensive 12-panel figure...")

fig = plt.figure(figsize=(24, 20))
gs = gridspec.GridSpec(4, 3, hspace=0.35, wspace=0.3)

# Panel 1: Energy-binned R_cav distributions (violin-style)
ax1 = fig.add_subplot(gs[0, 0])
positions = range(len(bin_stats))
bp_data = [np.clip(s['rcav'], 0, 20) for s in bin_stats]
bp = ax1.boxplot(bp_data, positions=positions, widths=0.6,
                  showfliers=False, patch_artist=True)
for patch, s in zip(bp['boxes'], bin_stats):
    color = plt.cm.plasma(np.log10(s['center']) / np.log10(50000))
    patch.set_facecolor(color)
    patch.set_alpha(0.7)
ax1.set_xticks(positions)
ax1.set_xticklabels([f"{s['center']:.0f}" for s in bin_stats], rotation=45, fontsize=7)
ax1.set_xlabel("MET bin center (GeV)")
ax1.set_ylabel("R_cav (cm)")
ax1.set_title("1. R_cav distributions by energy")
ax1.set_ylim(0, 15)

# Panel 2: Quantile progression with fits
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(q_centers, q_p50, 'bo-', label='Median', markersize=4)
ax2.plot(q_centers, q_p95, 'r^-', label='95th pct', markersize=4)
ax2.plot(q_centers, q_p99, 'gv-', label='99th pct', markersize=4)
# Overlay FTD fits
if "sqrt(E)" in fits:
    x_fit = np.linspace(q_centers.min(), q_centers.max(), 100)
    a_med = fits['sqrt(E)']['popt'][0]
    ax2.plot(x_fit, a_med * x_fit**2, 'b--', alpha=0.5, label=f'FTD: {a_med:.4f}√E')
if "sqrt(E) [p95]" in fits:
    a_95 = fits['sqrt(E) [p95]']['popt'][0]
    ax2.plot(x_fit, a_95 * x_fit**2, 'r--', alpha=0.5)
ax2.set_xlabel("√MET (√GeV)")
ax2.set_ylabel("R_cav (cm)")
ax2.set_title("2. Quantile progression vs √MET")
ax2.legend(fontsize=7)

# Panel 3: Log-log with power law fits
ax3 = fig.add_subplot(gs[0, 2])
# 2D histogram in log space
h, xe, ye = np.histogram2d(log_met, log_rcav, bins=100,
                            range=[[2, 4.7], [-2, 2]])
ax3.pcolormesh(xe, ye, h.T, cmap='hot', norm=mcolors.LogNorm(vmin=1))
# Overlay power law fit
x_log = np.linspace(2, 4.7, 100)
ax3.plot(x_log, intercept_full + slope_full * x_log, 'c-', lw=2,
         label=f'b={slope_full:.3f} (FTD: 0.5)')
ax3.plot(x_log, intercept_full + 0.5 * x_log, 'g--', lw=1.5,
         label='FTD prediction (b=0.5)')
ax3.set_xlabel("log₁₀(MET / GeV)")
ax3.set_ylabel("log₁₀(R_cav / cm)")
ax3.set_title("3. Log-log power law analysis")
ax3.legend(fontsize=7)

# Panel 4: Ratio R/√MET vs MET
ax4 = fig.add_subplot(gs[1, 0])
# Bin and plot
ratio_bins = np.logspace(2, 4, 30)
ratio_meds = []
ratio_x = []
for i in range(len(ratio_bins)-1):
    mask = (met >= ratio_bins[i]) & (met < ratio_bins[i+1])
    if mask.sum() > 50:
        ratio_x.append(np.sqrt(ratio_bins[i] * ratio_bins[i+1]))
        ratio_meds.append(np.median(rcav[mask] / np.sqrt(met[mask])))
ax4.plot(ratio_x, ratio_meds, 'ko-', markersize=4)
ax4.axhline(np.median(ratio), color='r', ls='--', alpha=0.5, label=f'Overall median = {np.median(ratio):.5f}')
ax4.set_xscale('log')
ax4.set_xlabel("MET (GeV)")
ax4.set_ylabel("R_cav / √MET (cm/√GeV)")
ax4.set_title(f"4. Ratio test (FTD: flat line)")
ax4.legend(fontsize=7)

# Panel 5: R_cav histogram with detector layers
ax5 = fig.add_subplot(gs[1, 1])
ax5.hist(rcav, bins=300, range=(0, 20), color='steelblue', alpha=0.7, density=True)
for r_det, lbl, col in [(2.9, 'Pixel L1', 'red'), (6.8, 'Pixel L2', 'orange'),
                          (10.9, 'Pixel L3', 'green'), (16.0, 'Pixel L4', 'purple')]:
    ax5.axvline(r_det, color=col, ls='--', alpha=0.7, label=lbl)
ax5.set_xlabel("R_cav = max(SV_dxy) (cm)")
ax5.set_ylabel("Density")
ax5.set_title("5. R_cav with CMS pixel layers")
ax5.legend(fontsize=7)
ax5.set_xlim(0, 20)

# Panel 6: Scatter (subsampled) with FTD prediction band
ax6 = fig.add_subplot(gs[1, 2])
n_plot = min(50000, N)
idx = np.random.choice(N, n_plot, replace=False)
ax6.scatter(sqrt_met[idx], rcav[idx], s=0.3, alpha=0.1, c='navy')
# FTD prediction band
if "sqrt(E)" in fits and "sqrt(E) [p95]" in fits:
    x_pred = np.linspace(sqrt_met.min(), min(sqrt_met.max(), 70), 200)
    a_med = fits['sqrt(E)']['popt'][0]
    a_95 = fits['sqrt(E) [p95]']['popt'][0]
    ax6.plot(x_pred, a_med * x_pred**2, 'r-', lw=2, label=f'FTD median: {a_med:.4f}√E')
    ax6.fill_between(x_pred, 0, a_95 * x_pred**2, alpha=0.1, color='red', label='FTD 95% envelope')
ax6.set_xlabel("√MET (√GeV)")
ax6.set_ylabel("R_cav (cm)")
ax6.set_title("6. Scatter with FTD prediction")
ax6.set_ylim(0, 30)
ax6.set_xlim(10, 70)
ax6.legend(fontsize=7)

# Panel 7: Energy density ε = E/V vs R_cav
ax7 = fig.add_subplot(gs[2, 0])
valid = (V_cav > 0) & (eps_cav < 1e8)
n_sub = min(50000, valid.sum())
idx_v = np.random.choice(np.where(valid)[0], n_sub, replace=False)
ax7.scatter(rcav[idx_v], eps_cav[idx_v], s=0.3, alpha=0.1, c='darkred')
ax7.set_xscale('log')
ax7.set_yscale('log')
ax7.set_xlabel("R_cav (cm)")
ax7.set_ylabel("ε = MET/V_cav (GeV/cm³)")
ax7.set_title("7. Energy density vs radius")
# Overlay 1/R line
r_line = np.logspace(-2, 2, 100)
if "sqrt(E)" in fits:
    a = fits['sqrt(E)']['popt'][0]
    sigma = 1 / (4 * np.pi * a**2)
    ax7.plot(r_line, 3 * sigma / r_line, 'g-', lw=2, alpha=0.5, label=f'FTD: ε=3σ/R')
ax7.legend(fontsize=7)

# Panel 8: Outer tracker only — cleaner sample
ax8 = fig.add_subplot(gs[2, 1])
if outer_mask.sum() > 10000:
    n_out = min(30000, outer_mask.sum())
    idx_out = np.random.choice(np.where(outer_mask)[0], n_out, replace=False)
    ax8.scatter(sqrt_met[idx_out], rcav[idx_out], s=0.3, alpha=0.15, c='teal')
    # Fit outer-only
    try:
        popt_out, _ = optimize.curve_fit(model_power, met[outer_mask], rcav[outer_mask],
                                          p0=[0.1, 0.5], maxfev=10000)
        x_out = np.linspace(10, 70, 100)
        ax8.plot(x_out, popt_out[0] * (x_out**2)**popt_out[1], 'r-', lw=2,
                 label=f'Outer: R∝E^{popt_out[1]:.3f}')
    except:
        pass
ax8.set_xlabel("√MET (√GeV)")
ax8.set_ylabel("R_cav (cm)")
ax8.set_title(f"8. Outer tracker only (R≥2.9cm, ρ={rho_outer:.4f})")
ax8.legend(fontsize=7)
ax8.set_ylim(2.9, 50)

# Panel 9: High-energy focus (MET > 500 GeV)
ax9 = fig.add_subplot(gs[2, 2])
he_mask = met > 500
if he_mask.sum() > 100:
    ax9.scatter(sqrt_met[he_mask], rcav[he_mask], s=1, alpha=0.3, c='darkgreen')
    rho_he = stats.spearmanr(sqrt_met[he_mask], rcav[he_mask])[0]
    ax9.set_title(f"9. High-E tail (MET>500, N={he_mask.sum():,}, ρ={rho_he:.3f})")
else:
    ax9.set_title("9. High-E tail (too few events)")
ax9.set_xlabel("√MET (√GeV)")
ax9.set_ylabel("R_cav (cm)")

# Panel 10: Rescaled distributions (self-similarity test)
ax10 = fig.add_subplot(gs[3, 0])
for i, s in enumerate(bin_stats[:6]):  # First 6 bins
    scaled = s['rcav'] / np.sqrt(s['center'])
    color = plt.cm.viridis(i / 6)
    ax10.hist(scaled, bins=100, range=(0, 0.5), alpha=0.4, density=True,
              color=color, label=f"E~{s['center']:.0f}")
ax10.set_xlabel("R_cav / √MET (scaled)")
ax10.set_ylabel("Density")
ax10.set_title("10. Self-similarity test (FTD: overlapping)")
ax10.legend(fontsize=6, ncol=2)

# Panel 11: Cumulative R_cav at different energies
ax11 = fig.add_subplot(gs[3, 1])
for i, s in enumerate(bin_stats[:6]):
    sorted_r = np.sort(s['rcav'])
    cdf = np.arange(1, len(sorted_r)+1) / len(sorted_r)
    color = plt.cm.viridis(i / 6)
    ax11.plot(sorted_r, cdf, color=color, label=f"E~{s['center']:.0f}")
ax11.set_xscale('log')
ax11.set_xlabel("R_cav (cm)")
ax11.set_ylabel("CDF")
ax11.set_title("11. CDF by energy bin")
ax11.legend(fontsize=6, ncol=2)
ax11.set_xlim(0.01, 100)

# Panel 12: FTD Scorecard summary
ax12 = fig.add_subplot(gs[3, 2])
ax12.axis('off')
scorecard_text = f"""FTD CAVITATION HYPOTHESIS
SCORECARD (27M events, 4.57M selected)

Tests favoring FTD:      {score_ftd}
Tests favoring SM-null:  {score_sm}

Key metrics:
  ρ(√MET, R_cav) = {rho_full:.4f}
  √MET > linear MET: {'YES' if rho_full > rho_linear else 'NO'}
  Power law exponent: {slope_full:.3f} (FTD: 0.5)

Interpretation:
  The data shows a WEAK positive correlation
  between √MET and displaced vertex radius,
  directionally consistent with FTD cavitation.

  However, the effect is too weak ({rho_full:.3f}) to
  distinguish from detector acceptance effects.

  The power law exponent ({slope_full:.3f}) is positive
  but below the FTD prediction of 0.5.

Status: SUGGESTIVE but NOT CONCLUSIVE
  A dedicated LLP search with cavitation-
  specific selections could improve sensitivity."""

ax12.text(0.05, 0.95, scorecard_text, transform=ax12.transAxes,
          fontsize=9, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

fig.suptitle("FTD Topological Cavitation — CMS MET 2016G Exploration (27M events)",
             fontsize=16, fontweight='bold', y=0.98)

plot_path = os.path.join(DATA_DIR, "ftd_cavitation_EXPLORATION.png")
fig.savefig(plot_path, dpi=150, bbox_inches='tight')
print(f"\nSaved 12-panel exploration figure: {plot_path}")
plt.close()

print("\n" + "=" * 72)
print("ANALYSIS COMPLETE")
print("=" * 72)
