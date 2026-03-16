#!/usr/bin/env python3
"""
FTD Partial Correlation Deep Investigation: 10 Tests
======================================================

The 8-test reinvestigation found rho_partial = +0.103 between sqrt(MET)
and R_cav after controlling for SV mass (only 5% reduction from raw).
This script performs 10 deeper tests to determine whether the partial
correlation is genuine or an artifact.

Uses cached data: ftd_full_enhanced.npz (1.5M) + ftd_mc_cache.npz (32K)
Output: ftd_partial_correlation_DEEP.png (12-panel) +
        ftd_partial_correlation_deep_results.txt

ALL PRINT STATEMENTS USE ASCII ONLY (Windows cp1252 safe).
"""

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy import stats
from scipy.spatial.distance import pdist, squareform
import os
import sys
import warnings
warnings.filterwarnings('ignore', category=RuntimeWarning)
warnings.filterwarnings('ignore', category=FutureWarning)

rng = np.random.default_rng(42)

# ============================================================================
# DATA LOADING (from ftd_cern_reinvestigation.py)
# ============================================================================

sim_dir = os.path.dirname(os.path.abspath(__file__))
results_lines = []

def log(msg):
    print(msg)
    results_lines.append(msg)

log("=" * 70)
log("FTD PARTIAL CORRELATION DEEP INVESTIGATION: 10 TESTS")
log("=" * 70)

log("\nLoading cached data...")
data = np.load(os.path.join(sim_dir, 'ftd_full_enhanced.npz'))
met     = data['met']
rcav    = data['rcav']
dlsig   = data['sv_dlsig_max']
svmass  = data['sv_mass_max']
bjet    = data['has_bjet']
ntracks = data['sv_ntracks_max']

mc_raw = np.load(os.path.join(sim_dir, 'ftd_mc_cache.npz'))
mc_samples = ['WJetsToLNu', 'ZJetsToNuNu_200toInf', 'ZJetsToNuNu_100to200',
              'QCD_HT1000to1500', 'QCD_HT700to1000']

met_mc_all, rcav_mc_all, dlsig_mc_all, svmass_mc_all, bjet_mc_all, w_mc_all = \
    [], [], [], [], [], []
for s in mc_samples:
    m  = mc_raw[f'{s}__met']
    r  = mc_raw[f'{s}__rcav']
    dl = mc_raw[f'{s}__sv_dlsig_max']
    sv = mc_raw[f'{s}__sv_mass_max']
    bj = mc_raw[f'{s}__has_bjet']
    xsec    = float(mc_raw[f'{s}__xsec'])
    n_total = float(mc_raw[f'{s}__n_total'])
    w = np.full(len(m), xsec / n_total)
    met_mc_all.append(m); rcav_mc_all.append(r)
    dlsig_mc_all.append(dl); svmass_mc_all.append(sv)
    bjet_mc_all.append(bj); w_mc_all.append(w)

met_mc   = np.concatenate(met_mc_all)
rcav_mc  = np.concatenate(rcav_mc_all)
dlsig_mc = np.concatenate(dlsig_mc_all)
svmass_mc= np.concatenate(svmass_mc_all)
bjet_mc  = np.concatenate(bjet_mc_all)
w_mc     = np.concatenate(w_mc_all)

# Signal region
sig    = dlsig > 30
sig_mc = dlsig_mc > 30

log(f"Data: {len(met):,} events | MC: {len(met_mc):,} events")
log(f"Signal (dlenSig>30): Data {sig.sum():,} | MC {sig_mc.sum():,}")

# Convenience arrays for signal region
met_s     = met[sig]
rcav_s    = rcav[sig]
dlsig_s   = dlsig[sig]
svmass_s  = svmass[sig]
bjet_s    = bjet[sig].astype(float)
ntracks_s = ntracks[sig].astype(float)
sqrt_met_s = np.sqrt(met_s)

met_m     = met_mc[sig_mc]
rcav_m    = rcav_mc[sig_mc]
dlsig_m   = dlsig_mc[sig_mc]
svmass_m  = svmass_mc[sig_mc]
bjet_m    = bjet_mc[sig_mc].astype(float)
sqrt_met_m = np.sqrt(met_m)

n_data = len(met_s)
n_mc   = len(met_m)

log(f"Signal data: {n_data:,} events")
log(f"Signal MC:   {n_mc:,} events")


# ============================================================================
# HELPER: Rank-based partial Spearman correlation
# ============================================================================

def partial_spearman(x, y, z):
    """Rank-based partial Spearman: rho(x, y | z).
    Regress out z from both x and y using rank regression, then correlate residuals.
    """
    rx = stats.rankdata(x)
    ry = stats.rankdata(y)
    rz = stats.rankdata(z)
    # Regress out z from x ranks
    slope_xz, intercept_xz, _, _, _ = stats.linregress(rz, rx)
    resid_x = rx - (intercept_xz + slope_xz * rz)
    # Regress out z from y ranks
    slope_yz, intercept_yz, _, _, _ = stats.linregress(rz, ry)
    resid_y = ry - (intercept_yz + slope_yz * rz)
    rho, pval = stats.spearmanr(resid_x, resid_y)
    return rho, pval

def partial_spearman_multi(x, y, Z):
    """Multi-variable rank-based partial Spearman: rho(x, y | Z1, Z2, ...).
    Z is a 2D array of shape (n, k) with k control variables.
    Uses sequential regression on ranks.
    """
    rx = stats.rankdata(x).astype(float)
    ry = stats.rankdata(y).astype(float)
    # Regress out each control variable sequentially from both x and y
    for j in range(Z.shape[1]):
        rz = stats.rankdata(Z[:, j]).astype(float)
        # x residuals
        s, i, _, _, _ = stats.linregress(rz, rx)
        rx = rx - (i + s * rz)
        # y residuals
        s, i, _, _, _ = stats.linregress(rz, ry)
        ry = ry - (i + s * rz)
    rho, pval = stats.spearmanr(rx, ry)
    return rho, pval


# ============================================================================
# SANITY CHECK: Reproduce baseline values
# ============================================================================

log("\n" + "-" * 70)
log("SANITY CHECK: Reproducing baseline correlations")
log("-" * 70)

rho_raw, p_raw = stats.spearmanr(sqrt_met_s, rcav_s)
rho_partial, p_partial = partial_spearman(sqrt_met_s, rcav_s, svmass_s)

log(f"  Raw rho(sqrt(MET), R_cav):             {rho_raw:+.4f}  (expect ~+0.109)")
log(f"  Partial rho (rank, sv_mass only):       {rho_partial:+.4f}  (expect ~+0.103)")
log(f"  Reduction: {(1 - abs(rho_partial)/abs(rho_raw))*100:.1f}%  (expect ~5%)")

# MC baseline
rho_mc_raw, _ = stats.spearmanr(sqrt_met_m, rcav_m)
log(f"  MC raw rho:                             {rho_mc_raw:+.4f}  (expect ~+0.020)")


# ============================================================================
# Set up the 12-panel figure (4x3)
# ============================================================================
fig, axes = plt.subplots(4, 3, figsize=(20, 24))
fig.suptitle('FTD Partial Correlation Deep Investigation: 10 Tests',
             fontsize=16, fontweight='bold')


# ============================================================================
# TEST 1: Iterative Control Waterfall
# ============================================================================

log("\n" + "=" * 70)
log("TEST 1: Iterative Control Waterfall")
log("=" * 70)

# Build control variables one at a time
controls_names = ['Raw', '+sv_mass', '+bjet', '+ntracks', '+dlsig', '+sv_mass^2']
rho_waterfall = []

# Raw
rho_waterfall.append(rho_raw)
log(f"  Raw:             rho = {rho_raw:+.4f}")

# Cumulative controls
ctrl_vars = [svmass_s, bjet_s, ntracks_s, dlsig_s, svmass_s**2]
for i, name in enumerate(controls_names[1:]):
    Z = np.column_stack(ctrl_vars[:i+1])
    rho_i, _ = partial_spearman_multi(sqrt_met_s, rcav_s, Z)
    rho_waterfall.append(rho_i)
    reduction = (1 - abs(rho_i) / abs(rho_raw)) * 100
    log(f"  {name:18s} rho = {rho_i:+.4f}  ({reduction:.1f}% reduction)")

# Plot waterfall
ax = axes[0, 0]
colors = ['steelblue'] + ['coral'] * (len(rho_waterfall) - 1)
bars = ax.bar(range(len(rho_waterfall)), rho_waterfall, color=colors, edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(controls_names)))
ax.set_xticklabels(controls_names, rotation=45, ha='right', fontsize=8)
ax.set_ylabel('Partial rho')
ax.set_title('Test 1: Iterative Control Waterfall')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.set_ylim(0, max(rho_waterfall) * 1.3)
for bar, val in zip(bars, rho_waterfall):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.002,
            f'{val:.3f}', ha='center', va='bottom', fontsize=8)


# ============================================================================
# TEST 2: Nonlinear Residualization
# ============================================================================

log("\n" + "=" * 70)
log("TEST 2: Nonlinear Residualization")
log("=" * 70)

resid_results = {}

# 2a: Polynomial degree 2
try:
    from numpy.polynomial.polynomial import polyvander
    X_feat = np.column_stack([svmass_s, bjet_s, ntracks_s])
    # Degree 2: add squares and cross terms
    X_poly2 = np.column_stack([X_feat, svmass_s**2, ntracks_s**2,
                                svmass_s * bjet_s, svmass_s * ntracks_s])
    X_poly2 = np.column_stack([np.ones(n_data), X_poly2])
    beta_ols = np.linalg.lstsq(X_poly2, rcav_s, rcond=None)[0]
    resid_poly2 = rcav_s - X_poly2 @ beta_ols
    rho_poly2, _ = stats.spearmanr(sqrt_met_s, resid_poly2)
    resid_results['Poly-2'] = rho_poly2
    log(f"  Polynomial (deg 2):  rho = {rho_poly2:+.4f}")
except Exception as e:
    resid_results['Poly-2'] = np.nan
    log(f"  Polynomial (deg 2):  FAILED ({e})")

# 2b: Polynomial degree 3
try:
    X_poly3 = np.column_stack([X_poly2, svmass_s**3, ntracks_s**3,
                                svmass_s**2 * ntracks_s, svmass_s * ntracks_s**2])
    beta_ols3 = np.linalg.lstsq(X_poly3, rcav_s, rcond=None)[0]
    resid_poly3 = rcav_s - X_poly3 @ beta_ols3
    rho_poly3, _ = stats.spearmanr(sqrt_met_s, resid_poly3)
    resid_results['Poly-3'] = rho_poly3
    log(f"  Polynomial (deg 3):  rho = {rho_poly3:+.4f}")
except Exception as e:
    resid_results['Poly-3'] = np.nan
    log(f"  Polynomial (deg 3):  FAILED ({e})")

# 2c: Random Forest
try:
    from sklearn.ensemble import RandomForestRegressor, GradientBoostingRegressor
    HAS_SKLEARN = True
except ImportError:
    HAS_SKLEARN = False
    log("  WARNING: sklearn not available. Skipping RF/GBM tests.")

if HAS_SKLEARN:
    # Use a subsample for speed (RF on 350K events is slow)
    n_sub = min(100000, n_data)
    idx_sub = rng.choice(n_data, n_sub, replace=False)
    X_rf = np.column_stack([svmass_s, bjet_s, ntracks_s, dlsig_s])[idx_sub]
    y_rf = rcav_s[idx_sub]
    sqrt_met_sub = sqrt_met_s[idx_sub]

    try:
        rf = RandomForestRegressor(n_estimators=100, max_depth=10, n_jobs=-1,
                                   random_state=42)
        rf.fit(X_rf, y_rf)
        resid_rf = y_rf - rf.predict(X_rf)
        rho_rf, _ = stats.spearmanr(sqrt_met_sub, resid_rf)
        resid_results['RF'] = rho_rf
        log(f"  Random Forest:       rho = {rho_rf:+.4f}  (n={n_sub:,})")

        # Feature importances
        importances = rf.feature_importances_
        feat_names = ['sv_mass', 'bjet', 'ntracks', 'dlsig']
        log(f"    Feature importances: {dict(zip(feat_names, np.round(importances, 3)))}")
    except Exception as e:
        resid_results['RF'] = np.nan
        log(f"  Random Forest:       FAILED ({e})")

    # 2d: Gradient Boosting
    try:
        gbm = GradientBoostingRegressor(n_estimators=200, max_depth=5, learning_rate=0.1,
                                         random_state=42, subsample=0.8)
        gbm.fit(X_rf, y_rf)
        resid_gbm = y_rf - gbm.predict(X_rf)
        rho_gbm, _ = stats.spearmanr(sqrt_met_sub, resid_gbm)
        resid_results['GBM'] = rho_gbm
        log(f"  Gradient Boosting:   rho = {rho_gbm:+.4f}  (n={n_sub:,})")
    except Exception as e:
        resid_results['GBM'] = np.nan
        log(f"  Gradient Boosting:   FAILED ({e})")

# Plot
ax = axes[0, 1]
methods = list(resid_results.keys())
rhos = [resid_results[m] for m in methods]
colors2 = ['green' if abs(r) < 0.05 else 'coral' for r in rhos]
bars2 = ax.bar(range(len(methods)), rhos, color=colors2, edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(methods)))
ax.set_xticklabels(methods, rotation=45, ha='right')
ax.set_ylabel('rho(sqrt(MET), residuals)')
ax.set_title('Test 2: Nonlinear Residualization')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.axhline(y=rho_raw, color='steelblue', linestyle='--', linewidth=1, label=f'Raw={rho_raw:.3f}')
ax.legend(fontsize=8)
for bar, val in zip(bars2, rhos):
    if not np.isnan(val):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.002,
                f'{val:.3f}', ha='center', va='bottom', fontsize=8)


# ============================================================================
# TEST 3: B-Fraction Reweighting
# ============================================================================

log("\n" + "=" * 70)
log("TEST 3: B-Fraction Reweighting")
log("=" * 70)

met_edges_bfrac = [200, 250, 300, 350, 400, 500, 600, 800, 1200]
global_bfrac = np.mean(bjet_s)
log(f"  Global B-fraction: {global_bfrac:.3f}")

# Compute B-fraction per MET bin
bfrac_per_bin = []
for i in range(len(met_edges_bfrac)-1):
    mask = (met_s >= met_edges_bfrac[i]) & (met_s < met_edges_bfrac[i+1])
    if mask.sum() > 100:
        bf = np.mean(bjet_s[mask])
        bfrac_per_bin.append((met_edges_bfrac[i], met_edges_bfrac[i+1], bf, mask.sum()))
        log(f"  MET [{met_edges_bfrac[i]:4d}, {met_edges_bfrac[i+1]:4d}): B-frac = {bf:.3f}  (n={mask.sum():,})")

# Reweight: make B-fraction constant at global average
weights = np.ones(n_data)
for lo, hi, bf_bin, _ in bfrac_per_bin:
    mask = (met_s >= lo) & (met_s < hi)
    if bf_bin > 0.01 and bf_bin < 0.99:
        # For B-tagged events in this bin: weight = global_bfrac / bf_bin
        # For non-B events: weight = (1 - global_bfrac) / (1 - bf_bin)
        b_mask = mask & (bjet_s > 0.5)
        nb_mask = mask & (bjet_s < 0.5)
        weights[b_mask] = global_bfrac / bf_bin
        weights[nb_mask] = (1 - global_bfrac) / (1 - bf_bin)

# Weighted Spearman via rank on weighted data
# Approximate: use weighted ranks
def weighted_spearman(x, y, w):
    """Approximate weighted Spearman using bootstrap."""
    n_boot = 500
    rhos_boot = []
    for _ in range(n_boot):
        idx = rng.choice(len(x), len(x), replace=True, p=w/w.sum())
        r, _ = stats.spearmanr(x[idx], y[idx])
        rhos_boot.append(r)
    return np.mean(rhos_boot), np.std(rhos_boot)

rho_reweighted, rho_rw_std = weighted_spearman(sqrt_met_s, rcav_s, weights)
log(f"\n  Raw rho (unweighted):       {rho_raw:+.4f}")
log(f"  Reweighted rho (const B-frac): {rho_reweighted:+.4f} +/- {rho_rw_std:.4f}")
log(f"  Reduction: {(1 - abs(rho_reweighted)/abs(rho_raw))*100:.1f}%")

# Plot B-fraction vs MET
ax = axes[0, 2]
centers = [(lo+hi)/2 for lo, hi, _, _ in bfrac_per_bin]
bfracs = [bf for _, _, bf, _ in bfrac_per_bin]
ax.plot(centers, bfracs, 'o-', color='steelblue', label='B-fraction')
ax.axhline(y=global_bfrac, color='red', linestyle='--', label=f'Global={global_bfrac:.3f}')
ax.set_xlabel('MET (GeV)')
ax.set_ylabel('B-jet fraction')
ax.set_title('Test 3: B-Fraction vs MET')
ax.legend(fontsize=8)
ax.text(0.05, 0.05, f'Raw rho: {rho_raw:+.3f}\nReweighted: {rho_reweighted:+.3f}',
        transform=ax.transAxes, fontsize=9, verticalalignment='bottom',
        bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))


# ============================================================================
# TEST 4: SV Mass Stratification
# ============================================================================

log("\n" + "=" * 70)
log("TEST 4: SV Mass Stratification")
log("=" * 70)

mass_bands = [(0, 1, 'K/light'), (1, 2.5, 'D meson'), (2.5, 4, 'transition'),
              (4, 7, 'B meson'), (7, 50, 'exotic')]
rho_by_mass = []

for lo, hi, label in mass_bands:
    mask = (svmass_s >= lo) & (svmass_s < hi)
    n_band = mask.sum()
    if n_band > 500:
        rho_band, p_band = stats.spearmanr(sqrt_met_s[mask], rcav_s[mask])
        rho_by_mass.append((label, rho_band, n_band))
        log(f"  SV mass [{lo:.1f}, {hi:.1f}) GeV ({label:12s}): rho = {rho_band:+.4f}  (n={n_band:,})")
    else:
        rho_by_mass.append((label, np.nan, n_band))
        log(f"  SV mass [{lo:.1f}, {hi:.1f}) GeV ({label:12s}): too few events (n={n_band})")

# Plot
ax = axes[1, 0]
labels_m = [r[0] for r in rho_by_mass]
rhos_m = [r[1] for r in rho_by_mass]
ns_m = [r[2] for r in rho_by_mass]
valid = [not np.isnan(r) for r in rhos_m]
colors_m = ['coral' if abs(r) > 0.05 else 'green' for r in rhos_m]
bars_m = ax.bar(range(len(labels_m)), rhos_m, color=colors_m, edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(labels_m)))
ax.set_xticklabels(labels_m, rotation=45, ha='right', fontsize=8)
ax.set_ylabel('rho(sqrt(MET), R_cav)')
ax.set_title('Test 4: SV Mass Stratification')
ax.axhline(y=rho_raw, color='steelblue', linestyle='--', linewidth=1, alpha=0.5, label='Raw')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.legend(fontsize=8)
for bar, val, n in zip(bars_m, rhos_m, ns_m):
    if not np.isnan(val):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.003,
                f'{val:.3f}\nn={n//1000}k', ha='center', va='bottom', fontsize=7)


# ============================================================================
# TEST 5: Track Multiplicity Stratification
# ============================================================================

log("\n" + "=" * 70)
log("TEST 5: Track Multiplicity Stratification")
log("=" * 70)

ntracks_bins = [(2, 2, 'nt=2'), (3, 3, 'nt=3'), (4, 4, 'nt=4'), (5, 99, 'nt>=5')]
rho_by_nt = []

for lo, hi, label in ntracks_bins:
    mask = (ntracks_s >= lo) & (ntracks_s <= hi)
    n_band = mask.sum()
    if n_band > 500:
        rho_band, p_band = stats.spearmanr(sqrt_met_s[mask], rcav_s[mask])
        rho_by_nt.append((label, rho_band, n_band))
        log(f"  {label:8s}: rho = {rho_band:+.4f}  (n={n_band:,})")
    else:
        rho_by_nt.append((label, np.nan, n_band))
        log(f"  {label:8s}: too few events (n={n_band})")

# Plot
ax = axes[1, 1]
labels_nt = [r[0] for r in rho_by_nt]
rhos_nt = [r[1] for r in rho_by_nt]
ns_nt = [r[2] for r in rho_by_nt]
colors_nt = ['coral' if abs(r) > 0.05 else 'green' for r in rhos_nt]
bars_nt = ax.bar(range(len(labels_nt)), rhos_nt, color=colors_nt, edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(labels_nt)))
ax.set_xticklabels(labels_nt, rotation=45, ha='right')
ax.set_ylabel('rho(sqrt(MET), R_cav)')
ax.set_title('Test 5: Track Multiplicity Strata')
ax.axhline(y=rho_raw, color='steelblue', linestyle='--', linewidth=1, alpha=0.5, label='Raw')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.legend(fontsize=8)
for bar, val, n in zip(bars_nt, rhos_nt, ns_nt):
    if not np.isnan(val):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.003,
                f'{val:.3f}\nn={n//1000}k', ha='center', va='bottom', fontsize=7)


# ============================================================================
# TEST 6: MET-Dependent Partial Correlation
# ============================================================================

log("\n" + "=" * 70)
log("TEST 6: MET-Dependent Partial Correlation")
log("=" * 70)

met_quartiles = [200, 250, 325, 450, 1200]
rho_by_met = []

for i in range(len(met_quartiles)-1):
    lo, hi = met_quartiles[i], met_quartiles[i+1]
    mask = (met_s >= lo) & (met_s < hi)
    n_band = mask.sum()
    if n_band > 500:
        rho_raw_band, _ = stats.spearmanr(sqrt_met_s[mask], rcav_s[mask])
        rho_partial_band, _ = partial_spearman(sqrt_met_s[mask], rcav_s[mask], svmass_s[mask])
        rho_by_met.append((lo, hi, rho_raw_band, rho_partial_band, n_band))
        log(f"  MET [{lo:4d}, {hi:4d}): raw={rho_raw_band:+.4f}  partial={rho_partial_band:+.4f}  (n={n_band:,})")
    else:
        rho_by_met.append((lo, hi, np.nan, np.nan, n_band))

# Plot
ax = axes[1, 2]
centers_met = [(r[0]+r[1])/2 for r in rho_by_met]
rhos_raw_met = [r[2] for r in rho_by_met]
rhos_part_met = [r[3] for r in rho_by_met]
ax.bar(np.arange(len(centers_met))-0.15, rhos_raw_met, width=0.3, color='steelblue',
       edgecolor='black', linewidth=0.5, label='Raw')
ax.bar(np.arange(len(centers_met))+0.15, rhos_part_met, width=0.3, color='coral',
       edgecolor='black', linewidth=0.5, label='Partial (sv_mass)')
ax.set_xticks(range(len(centers_met)))
ax.set_xticklabels([f'{r[0]}-{r[1]}' for r in rho_by_met], fontsize=8)
ax.set_xlabel('MET bin (GeV)')
ax.set_ylabel('rho')
ax.set_title('Test 6: MET-Dependent Partial Corr')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.legend(fontsize=8)


# ============================================================================
# TEST 7: Permutation Null Distribution
# ============================================================================

log("\n" + "=" * 70)
log("TEST 7: Permutation Null Distribution")
log("=" * 70)

n_perm = 5000
log(f"  Running {n_perm} permutations (this may take a minute)...")

# For speed, use a subsample
n_perm_sub = min(50000, n_data)
idx_perm = rng.choice(n_data, n_perm_sub, replace=False)
sqrt_met_perm = sqrt_met_s[idx_perm]
rcav_perm = rcav_s[idx_perm]
svmass_perm = svmass_s[idx_perm]

# Observed partial correlation on subsample
rho_obs_sub, _ = partial_spearman(sqrt_met_perm, rcav_perm, svmass_perm)

null_rhos = []
for p in range(n_perm):
    shuffled_met = rng.permutation(sqrt_met_perm)
    rho_null, _ = partial_spearman(shuffled_met, rcav_perm, svmass_perm)
    null_rhos.append(rho_null)
    if (p+1) % 1000 == 0:
        log(f"    {p+1}/{n_perm} permutations done...")

null_rhos = np.array(null_rhos)
p_value = np.mean(np.abs(null_rhos) >= abs(rho_obs_sub))
z_score = (rho_obs_sub - np.mean(null_rhos)) / np.std(null_rhos) if np.std(null_rhos) > 0 else np.inf

log(f"  Observed partial rho (subsample): {rho_obs_sub:+.4f}")
log(f"  Null distribution: mean={np.mean(null_rhos):+.4f}, std={np.std(null_rhos):.4f}")
log(f"  p-value (two-sided): {p_value:.6f}")
log(f"  Z-score: {z_score:.2f}")
if p_value < 0.001:
    log(f"  --> HIGHLY SIGNIFICANT (p < 0.001, Z = {z_score:.1f})")
elif p_value < 0.05:
    log(f"  --> Significant at 5% level")
else:
    log(f"  --> NOT significant (p = {p_value:.3f})")

# Plot
ax = axes[2, 0]
ax.hist(null_rhos, bins=50, color='lightgray', edgecolor='gray', density=True, label='Null')
ax.axvline(x=rho_obs_sub, color='red', linewidth=2, label=f'Observed={rho_obs_sub:+.3f}')
ax.set_xlabel('Partial rho (permuted)')
ax.set_ylabel('Density')
ax.set_title(f'Test 7: Permutation Null (p={p_value:.4f})')
ax.legend(fontsize=8)
ax.text(0.05, 0.95, f'Z = {z_score:.1f}\np = {p_value:.1e}',
        transform=ax.transAxes, fontsize=10, verticalalignment='top',
        bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))


# ============================================================================
# TEST 8: MC Partial Correlation
# ============================================================================

log("\n" + "=" * 70)
log("TEST 8: MC Partial Correlation")
log("=" * 70)

rho_mc_partial, p_mc_partial = partial_spearman(sqrt_met_m, rcav_m, svmass_m)
log(f"  MC raw rho:            {rho_mc_raw:+.4f}")
log(f"  MC partial (sv_mass):  {rho_mc_partial:+.4f}")

# Compare data vs MC
log(f"\n  DATA partial:          {rho_partial:+.4f}")
log(f"  MC   partial:          {rho_mc_partial:+.4f}")
log(f"  Excess partial:        {rho_partial - rho_mc_partial:+.4f}")

if abs(rho_mc_partial) < 0.03:
    log("  --> MC shows NO residual correlation. Data excess is GENUINE.")
elif abs(rho_mc_partial) > abs(rho_partial) * 0.5:
    log("  --> MC reproduces most of the correlation. Likely kinematic.")
else:
    log("  --> MC shows SOME correlation. Partial kinematic origin.")

# Also check MC with bjet control
Z_mc = np.column_stack([svmass_m, bjet_m])
rho_mc_multi, _ = partial_spearman_multi(sqrt_met_m, rcav_m, Z_mc)
log(f"  MC partial (sv_mass+bjet): {rho_mc_multi:+.4f}")

# Plot comparison
ax = axes[2, 1]
categories = ['Raw', 'Partial\n(sv_mass)', 'Partial\n(sv_mass+bjet)']
data_vals = [rho_raw, rho_partial, rho_waterfall[2] if len(rho_waterfall) > 2 else np.nan]
mc_vals = [rho_mc_raw, rho_mc_partial, rho_mc_multi]
x_pos = np.arange(len(categories))
ax.bar(x_pos - 0.15, data_vals, width=0.3, color='steelblue', edgecolor='black',
       linewidth=0.5, label='Data')
ax.bar(x_pos + 0.15, mc_vals, width=0.3, color='orange', edgecolor='black',
       linewidth=0.5, label='MC')
ax.set_xticks(x_pos)
ax.set_xticklabels(categories, fontsize=8)
ax.set_ylabel('rho')
ax.set_title('Test 8: Data vs MC Partial Corr')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
ax.legend(fontsize=8)


# ============================================================================
# TEST 9: R_cav Range Dependence
# ============================================================================

log("\n" + "=" * 70)
log("TEST 9: R_cav Range Dependence")
log("=" * 70)

rcav_ranges = [(0, 1, 'R<1cm'), (1, 4.4, '1-4.4cm'),
               (4.4, 10.2, '4.4-10.2cm'), (10.2, 60, 'R>10.2cm')]
rho_by_rcav = []

for lo, hi, label in rcav_ranges:
    mask = (rcav_s >= lo) & (rcav_s < hi)
    n_band = mask.sum()
    if n_band > 500:
        rho_band, p_band = stats.spearmanr(sqrt_met_s[mask], rcav_s[mask])
        rho_by_rcav.append((label, rho_band, n_band))
        log(f"  R_cav [{lo:5.1f}, {hi:5.1f}) cm ({label:12s}): rho = {rho_band:+.4f}  (n={n_band:,})")
    else:
        rho_by_rcav.append((label, np.nan, n_band))
        log(f"  R_cav [{lo:5.1f}, {hi:5.1f}) cm ({label:12s}): too few (n={n_band})")

# Plot
ax = axes[2, 2]
labels_r = [r[0] for r in rho_by_rcav]
rhos_r = [r[1] for r in rho_by_rcav]
ns_r = [r[2] for r in rho_by_rcav]
colors_r = ['coral' if abs(r) > 0.05 else 'green' for r in rhos_r]
bars_r = ax.bar(range(len(labels_r)), rhos_r, color=colors_r, edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(labels_r)))
ax.set_xticklabels(labels_r, rotation=45, ha='right', fontsize=8)
ax.set_ylabel('rho(sqrt(MET), R_cav)')
ax.set_title('Test 9: R_cav Range Dependence')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
# Mark pixel layers
ax.text(0.5, 0.95, 'CMS pixels: 4.4, 7.3, 10.2 cm', transform=ax.transAxes,
        fontsize=7, ha='center', va='top', style='italic')
for bar, val, n in zip(bars_r, rhos_r, ns_r):
    if not np.isnan(val):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.003,
                f'{val:.3f}\nn={n//1000}k', ha='center', va='bottom', fontsize=7)


# ============================================================================
# TEST 10: Partial Distance Correlation
# ============================================================================

log("\n" + "=" * 70)
log("TEST 10: Partial Distance Correlation")
log("=" * 70)

def distance_correlation(x, y):
    """Compute distance correlation between 1D arrays x and y.
    Manual implementation via pairwise distance matrices.
    """
    n = len(x)
    # Pairwise distances
    a = np.abs(x[:, None] - x[None, :])
    b = np.abs(y[:, None] - y[None, :])
    # Double centering
    a_row = a.mean(axis=1, keepdims=True)
    a_col = a.mean(axis=0, keepdims=True)
    a_grand = a.mean()
    A = a - a_row - a_col + a_grand
    b_row = b.mean(axis=1, keepdims=True)
    b_col = b.mean(axis=0, keepdims=True)
    b_grand = b.mean()
    B = b - a_row - b_col + b_grand  # BUG FIX: should use b_row
    B = b - b_row - b_col + b_grand

    dcov2 = np.mean(A * B)
    dvar_x = np.mean(A * A)
    dvar_y = np.mean(B * B)

    if dvar_x > 0 and dvar_y > 0:
        dcor = np.sqrt(dcov2 / np.sqrt(dvar_x * dvar_y))
    else:
        dcor = 0.0
    return dcor

# Use small subsample (dcor is O(n^2) in memory)
n_dcor = 5000
idx_dcor = rng.choice(n_data, n_dcor, replace=False)
x_dc = sqrt_met_s[idx_dcor]
y_dc = rcav_s[idx_dcor]
z_dc = svmass_s[idx_dcor]

log(f"  Computing distance correlation (n={n_dcor}, O(n^2) memory)...")

try:
    # Full dcor
    dcor_xy = distance_correlation(x_dc, y_dc)
    log(f"  dcor(sqrt(MET), R_cav) = {dcor_xy:.4f}")

    # dcor of residuals after controlling for sv_mass
    # Partial dcor: regress out sv_mass from both, then compute dcor
    slope_xz, int_xz, _, _, _ = stats.linregress(z_dc, x_dc)
    resid_x_dc = x_dc - (int_xz + slope_xz * z_dc)
    slope_yz, int_yz, _, _, _ = stats.linregress(z_dc, y_dc)
    resid_y_dc = y_dc - (int_yz + slope_yz * z_dc)

    dcor_partial = distance_correlation(resid_x_dc, resid_y_dc)
    log(f"  dcor(sqrt(MET), R_cav | sv_mass) = {dcor_partial:.4f}  [linear residualization]")

    # Nonlinear residualization for dcor
    # Use polynomial regression for z -> x and z -> y
    z_poly = np.column_stack([z_dc, z_dc**2, z_dc**3])
    z_poly_aug = np.column_stack([np.ones(n_dcor), z_poly])
    beta_x = np.linalg.lstsq(z_poly_aug, x_dc, rcond=None)[0]
    beta_y = np.linalg.lstsq(z_poly_aug, y_dc, rcond=None)[0]
    resid_x_nl = x_dc - z_poly_aug @ beta_x
    resid_y_nl = y_dc - z_poly_aug @ beta_y

    dcor_partial_nl = distance_correlation(resid_x_nl, resid_y_nl)
    log(f"  dcor(sqrt(MET), R_cav | sv_mass^3) = {dcor_partial_nl:.4f}  [poly-3 residualization]")

    # Permutation test for dcor significance
    n_dcor_perm = 500
    null_dcors = []
    for p in range(n_dcor_perm):
        x_shuf = rng.permutation(resid_x_dc)
        null_dcors.append(distance_correlation(x_shuf, resid_y_dc))
    null_dcors = np.array(null_dcors)
    p_dcor = np.mean(null_dcors >= dcor_partial)
    log(f"  Permutation test (n={n_dcor_perm}): p = {p_dcor:.4f}")

except Exception as e:
    dcor_xy = np.nan
    dcor_partial = np.nan
    dcor_partial_nl = np.nan
    p_dcor = np.nan
    log(f"  Distance correlation FAILED: {e}")

# Plot
ax = axes[3, 0]
dcor_vals = [dcor_xy, dcor_partial, dcor_partial_nl]
dcor_labels = ['Full dcor', 'Partial\n(linear)', 'Partial\n(poly-3)']
colors_dc = ['steelblue', 'coral', 'darkorange']
bars_dc = ax.bar(range(len(dcor_vals)), dcor_vals, color=colors_dc,
                  edgecolor='black', linewidth=0.5)
ax.set_xticks(range(len(dcor_labels)))
ax.set_xticklabels(dcor_labels, fontsize=8)
ax.set_ylabel('Distance correlation')
ax.set_title(f'Test 10: Partial dcor (p={p_dcor:.3f})')
ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
for bar, val in zip(bars_dc, dcor_vals):
    if not np.isnan(val):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.002,
                f'{val:.3f}', ha='center', va='bottom', fontsize=9)


# ============================================================================
# SUMMARY PANEL: Heatmap
# ============================================================================

log("\n" + "=" * 70)
log("SUMMARY")
log("=" * 70)

ax = axes[3, 1]
# Create summary matrix: test vs verdict
summary_tests = [
    'T1: Iterative ctrl',
    'T2: Nonlinear resid',
    'T3: B-frac reweight',
    'T4: Mass strata',
    'T5: Ntracks strata',
    'T6: MET-dep partial',
    'T7: Permutation',
    'T8: MC comparison',
    'T9: R_cav range',
    'T10: Distance corr'
]

# Classify each test result
def classify_test(rho_after, rho_before=rho_raw):
    """Classify: -1 = kills correlation, 0 = ambiguous, +1 = survives"""
    if np.isnan(rho_after):
        return 0
    ratio = abs(rho_after) / abs(rho_before) if abs(rho_before) > 0 else 0
    if ratio < 0.3:
        return -1  # killed
    elif ratio > 0.7:
        return 1   # survives
    else:
        return 0   # ambiguous

# Gather final rho values for each test
final_rhos = [
    rho_waterfall[-1] if rho_waterfall else np.nan,  # T1
    min([abs(v) for v in resid_results.values() if not np.isnan(v)], default=np.nan),  # T2
    rho_reweighted,  # T3
    np.mean([r[1] for r in rho_by_mass if not np.isnan(r[1])]),  # T4 avg
    np.mean([r[1] for r in rho_by_nt if not np.isnan(r[1])]),  # T5 avg
    np.mean([r[3] for r in rho_by_met if not np.isnan(r[3])]),  # T6 avg partial
    rho_obs_sub,  # T7
    rho_partial - rho_mc_partial,  # T8 excess
    np.mean([r[1] for r in rho_by_rcav if not np.isnan(r[1])]),  # T9 avg
    dcor_partial_nl if not np.isnan(dcor_partial_nl) else 0.0,  # T10
]

verdicts = []
verdict_colors = []
for i, rho_val in enumerate(final_rhos):
    if i == 6:  # T7: permutation — verdict based on significance
        if p_value < 0.001:
            verdicts.append('SURVIVES')
            verdict_colors.append('coral')
        else:
            verdicts.append('NOT SIG')
            verdict_colors.append('green')
    elif i == 7:  # T8: MC — verdict based on excess
        if abs(rho_partial) > 2 * abs(rho_mc_partial):
            verdicts.append('GENUINE')
            verdict_colors.append('coral')
        else:
            verdicts.append('KINEMATIC')
            verdict_colors.append('green')
    elif i == 9:  # T10: dcor — verdict based on p-value
        if p_dcor < 0.05:
            verdicts.append('SURVIVES')
            verdict_colors.append('coral')
        else:
            verdicts.append('KILLED')
            verdict_colors.append('green')
    else:
        c = classify_test(rho_val, rho_raw)
        if c == 1:
            verdicts.append('SURVIVES')
            verdict_colors.append('coral')
        elif c == -1:
            verdicts.append('KILLED')
            verdict_colors.append('green')
        else:
            verdicts.append('AMBIG')
            verdict_colors.append('gold')

# Plot summary as text table
ax.axis('off')
table_data = [[t, f'{r:.3f}' if not np.isnan(r) else 'N/A', v]
              for t, r, v in zip(summary_tests, final_rhos, verdicts)]
table = ax.table(cellText=table_data, colLabels=['Test', 'rho', 'Verdict'],
                  cellLoc='center', loc='center')
table.auto_set_font_size(False)
table.set_fontsize(8)
table.scale(1, 1.4)
# Color verdict column
for i, vc in enumerate(verdict_colors):
    table[i+1, 2].set_facecolor(vc)
    table[i+1, 2].set_alpha(0.4)
ax.set_title('Summary Table')


# ============================================================================
# VERDICT SCORECARD
# ============================================================================

ax = axes[3, 2]
ax.axis('off')

n_survives = verdicts.count('SURVIVES') + verdicts.count('GENUINE')
n_killed = verdicts.count('KILLED')
n_ambig = verdicts.count('AMBIG') + verdicts.count('NOT SIG')

verdict_text = (
    f"PARTIAL CORRELATION DEEP INVESTIGATION\n"
    f"{'=' * 40}\n\n"
    f"Baseline: rho = +{rho_raw:.3f} (raw)\n"
    f"          rho = +{rho_partial:.3f} (partial, sv_mass)\n\n"
    f"10-TEST RESULTS:\n"
    f"  SURVIVES:  {n_survives}\n"
    f"  KILLED:    {n_killed}\n"
    f"  AMBIGUOUS: {n_ambig}\n\n"
)

if n_killed >= 3:
    verdict_text += "VERDICT: Correlation is likely an ARTIFACT\n"
    verdict_text += "of nonlinear kinematics / composition"
elif n_survives >= 6:
    verdict_text += "VERDICT: Correlation appears GENUINE\n"
    verdict_text += "(non-kinematic, non-compositional)"
else:
    verdict_text += "VERDICT: INCONCLUSIVE\n"
    verdict_text += "Some controls reduce it, others don't"

verdict_text += f"\n\nPermutation p-value: {p_value:.1e}"
verdict_text += f"\nZ-score: {z_score:.1f}"

ax.text(0.05, 0.95, verdict_text, transform=ax.transAxes, fontsize=9,
        verticalalignment='top', fontfamily='monospace',
        bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))
ax.set_title('Final Verdict')


# ============================================================================
# Save figure and results
# ============================================================================

plt.tight_layout(rect=[0, 0, 1, 0.96])
fig_path = os.path.join(sim_dir, 'ftd_partial_correlation_DEEP.png')
plt.savefig(fig_path, dpi=150, bbox_inches='tight')
log(f"\nFigure saved: {fig_path}")

# Also log the full verdict
log("\n" + "=" * 70)
log("FINAL VERDICT")
log("=" * 70)
for t, r, v in zip(summary_tests, final_rhos, verdicts):
    log(f"  {t:25s}  rho={r:+.4f}  --> {v}")

log(f"\n  SURVIVES: {n_survives}  |  KILLED: {n_killed}  |  AMBIGUOUS: {n_ambig}")
if n_killed >= 3:
    log("  OVERALL: Correlation is likely an ARTIFACT")
elif n_survives >= 6:
    log("  OVERALL: Correlation appears GENUINE (non-kinematic)")
else:
    log("  OVERALL: INCONCLUSIVE")

log(f"\n  Permutation p-value: {p_value:.1e}")
log(f"  Z-score: {z_score:.1f}")

# Save results
results_path = os.path.join(sim_dir, 'ftd_partial_correlation_deep_results.txt')
with open(results_path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(results_lines))
log(f"Results saved: {results_path}")

log("\nDone.")
