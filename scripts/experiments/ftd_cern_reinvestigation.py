#!/usr/bin/env python3
"""
FTD Cavitation Reinvestigation: 8 Independent Tests
=====================================================

Previous analysis concluded beta=0.5 was "falsified" (beta_observed ~ 0.12).
Three explorations revealed critical issues:
  1. R_cav = max(SV_dxy) is a KINEMATIC observable (flight distance), NOT bubble radius
  2. MET-R_cav correlation is trivially expected from SM kinematics
  3. Critical tests were never performed: R^2 vs MET, excess scaling, partial correlations

This script performs 8 independent tests to determine whether beta=0.5
can be rescued when properly tested on the EXCESS (data - MC), not raw data.

Uses cached data: ftd_full_enhanced.npz (1.5M events) + ftd_mc_cache.npz (32K MC)
Output: ftd_cavitation_REINVESTIGATION.png (16-panel) + ftd_reinvestigation_results.txt

ALL PRINT STATEMENTS USE ASCII ONLY (Windows cp1252 safe).
"""

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy import stats
from scipy.optimize import curve_fit, minimize
import os
import sys
import warnings
warnings.filterwarnings('ignore', category=RuntimeWarning)

rng = np.random.default_rng(42)

# ============================================================================
# DATA LOADING (from ftd_cern_scaling_corrections.py pattern)
# ============================================================================

sim_dir = os.path.dirname(os.path.abspath(__file__))
results_lines = []

def log(msg):
    print(msg)
    results_lines.append(msg)

log("=" * 70)
log("FTD CAVITATION REINVESTIGATION: 8 INDEPENDENT TESTS")
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
met_s   = met[sig]
rcav_s  = rcav[sig]
dlsig_s = dlsig[sig]
svmass_s= svmass[sig]
bjet_s  = bjet[sig]
ntracks_s = ntracks[sig]

met_m   = met_mc[sig_mc]
rcav_m  = rcav_mc[sig_mc]
dlsig_m = dlsig_mc[sig_mc]
svmass_m= svmass_mc[sig_mc]
bjet_m  = bjet_mc[sig_mc]

# MET bins
met_edges_fine = [200, 225, 250, 275, 300, 350, 400, 500, 600, 800, 1200]
met_edges_broad = [200, 300, 400, 600, 1200]
met_centers_fine = [(met_edges_fine[i]+met_edges_fine[i+1])/2 for i in range(len(met_edges_fine)-1)]
met_centers_broad = [(met_edges_broad[i]+met_edges_broad[i+1])/2 for i in range(len(met_edges_broad)-1)]


# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def weighted_quantile(values, quantile, weights=None):
    """Compute weighted quantile."""
    if weights is None:
        return np.quantile(values, quantile)
    sorter = np.argsort(values)
    values = values[sorter]
    weights = weights[sorter]
    cumw = np.cumsum(weights)
    cutoff = quantile * cumw[-1]
    idx = np.searchsorted(cumw, cutoff)
    return values[min(idx, len(values)-1)]

def bin_data(met_arr, val_arr, edges, weights=None):
    """Compute median and statistics in MET bins."""
    medians, means, stds, iqrs, mads, ns = [], [], [], [], [], []
    p90s, tail5s, kurtoses, skews = [], [], [], []
    for i in range(len(edges)-1):
        m = (met_arr >= edges[i]) & (met_arr < edges[i+1])
        n = m.sum()
        ns.append(n)
        if n > 20:
            v = val_arr[m]
            if weights is not None:
                wt = weights[m]
                medians.append(weighted_quantile(v, 0.5, wt))
                p90s.append(weighted_quantile(v, 0.9, wt))
            else:
                medians.append(np.median(v))
                p90s.append(np.percentile(v, 90))
            means.append(np.mean(v))
            stds.append(np.std(v))
            q25, q75 = np.percentile(v, [25, 75])
            iqrs.append(q75 - q25)
            mads.append(np.median(np.abs(v - np.median(v))))
            tail5s.append(np.mean(v > 5.0))
            kurtoses.append(stats.kurtosis(v, fisher=True))
            skews.append(stats.skew(v))
        else:
            medians.append(np.nan); means.append(np.nan)
            stds.append(np.nan); iqrs.append(np.nan)
            mads.append(np.nan); p90s.append(np.nan)
            tail5s.append(np.nan); kurtoses.append(np.nan)
            skews.append(np.nan)
    return dict(median=np.array(medians), mean=np.array(means),
                std=np.array(stds), iqr=np.array(iqrs), mad=np.array(mads),
                p90=np.array(p90s), tail5=np.array(tail5s),
                kurtosis=np.array(kurtoses), skew=np.array(skews),
                n=np.array(ns))

def safe_fit(func, x, y, p0=None, bounds=(-np.inf, np.inf), sigma=None, maxfev=10000):
    """Curve fit with error handling."""
    try:
        popt, pcov = curve_fit(func, x, y, p0=p0, bounds=bounds, sigma=sigma,
                               maxfev=maxfev, absolute_sigma=True if sigma is not None else False)
        y_pred = func(x, *popt)
        ss_res = np.sum((y - y_pred)**2)
        ss_tot = np.sum((y - np.mean(y))**2)
        r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0
        k = len(popt)
        n = len(y)
        aic = n * np.log(ss_res / n + 1e-30) + 2 * k
        bic = n * np.log(ss_res / n + 1e-30) + k * np.log(n)
        chi2 = ss_res / (n - k) if n > k else np.inf
        return dict(popt=popt, pcov=pcov, r2=r2, aic=aic, bic=bic, chi2=chi2, success=True)
    except Exception as e:
        return dict(popt=None, pcov=None, r2=np.nan, aic=np.inf, bic=np.inf,
                    chi2=np.inf, success=False, error=str(e))


# ============================================================================
# Set up the 16-panel figure
# ============================================================================
fig, axes = plt.subplots(4, 4, figsize=(24, 22))
fig.suptitle('FTD Cavitation Reinvestigation: 8 Independent Tests', fontsize=16, fontweight='bold')


# ============================================================================
# TEST 1: R^2 vs MET — Direct Linear Regression
# ============================================================================

log("\n" + "=" * 70)
log("TEST 1: R^2 vs MET -- Direct Linear Regression")
log("=" * 70)
log("If R ~ sqrt(E), then R^2 ~ E (linear). Most direct test.")

ax1 = axes[0, 0]

rcav_sq_s = rcav_s**2

# Event-level regressions (subsample for speed)
n_sub = min(100000, len(met_s))
idx_sub = rng.choice(len(met_s), n_sub, replace=False)

# (a) R^2 vs MET
slope_a, intercept_a, r_a, p_a, se_a = stats.linregress(met_s[idx_sub], rcav_sq_s[idx_sub])
# (b) R vs MET
slope_b, intercept_b, r_b, p_b, se_b = stats.linregress(met_s[idx_sub], rcav_s[idx_sub])
# (c) R vs sqrt(MET)
slope_c, intercept_c, r_c, p_c, se_c = stats.linregress(np.sqrt(met_s[idx_sub]), rcav_s[idx_sub])

log(f"\n  Event-level regression (N={n_sub:,}):")
log(f"  (a) R^2 vs MET:     R2_fit = {r_a**2:.6f}, slope = {slope_a:.6f}")
log(f"  (b) R vs MET:       R2_fit = {r_b**2:.6f}, slope = {slope_b:.6f}")
log(f"  (c) R vs sqrt(MET): R2_fit = {r_c**2:.6f}, slope = {slope_c:.6f}")

# Repeat for MC
n_sub_mc = min(len(met_m), n_sub)
if n_sub_mc > 50:
    idx_mc = rng.choice(len(met_m), n_sub_mc, replace=False)
    r_a_mc = stats.linregress(met_m[idx_mc], rcav_m[idx_mc]**2)[2]**2
    r_b_mc = stats.linregress(met_m[idx_mc], rcav_m[idx_mc])[2]**2
    r_c_mc = stats.linregress(np.sqrt(met_m[idx_mc]), rcav_m[idx_mc])[2]**2
    log(f"\n  MC regression (N={n_sub_mc:,}):")
    log(f"  (a) R^2 vs MET:     R2_fit = {r_a_mc:.6f}")
    log(f"  (b) R vs MET:       R2_fit = {r_b_mc:.6f}")
    log(f"  (c) R vs sqrt(MET): R2_fit = {r_c_mc:.6f}")
else:
    r_a_mc = r_b_mc = r_c_mc = np.nan
    log("  MC: insufficient statistics")

# Binned regression
data_stats = bin_data(met_s, rcav_s, met_edges_fine)
data_stats_sq = bin_data(met_s, rcav_sq_s, met_edges_fine)
mc_stats = bin_data(met_m, rcav_m, met_edges_fine)

x_f = np.array(met_centers_fine)
valid_d = ~np.isnan(data_stats['median']) & (data_stats['n'] > 50)
valid_m = ~np.isnan(mc_stats['median']) & (mc_stats['n'] > 10)

# Binned: median(R^2) vs MET
if valid_d.sum() >= 3:
    sl, it, rr, pp, _ = stats.linregress(x_f[valid_d], data_stats_sq['median'][valid_d])
    log(f"\n  Binned median(R^2) vs MET: R2_fit = {rr**2:.4f}, slope = {sl:.6f}, p = {pp:.2e}")

# Plot: binned R^2 vs MET for data and MC
ax1.scatter(x_f[valid_d], data_stats_sq['median'][valid_d], c='blue', s=40, label='Data', zorder=5)
if valid_m.sum() >= 2:
    mc_stats_sq = bin_data(met_m, rcav_m**2, met_edges_fine)
    ax1.scatter(x_f[valid_m], mc_stats_sq['median'][valid_m], c='red', s=40, marker='^', label='MC', zorder=5)
# Linear fit to data
if valid_d.sum() >= 3:
    xp = np.linspace(200, 1200, 100)
    ax1.plot(xp, sl * xp + it, 'b--', alpha=0.7, label=f'R2_fit={rr**2:.4f}')
ax1.set_xlabel('MET (GeV)')
ax1.set_ylabel('Median R_cav^2 (cm^2)')
ax1.set_title('Test 1: R^2 vs MET\n(if R~sqrt(E), this is linear)')
ax1.legend(fontsize=8)
ax1.grid(True, alpha=0.3)

# Summary bar chart in corner
labels = ['R^2 vs MET', 'R vs MET', 'R vs sqrt(MET)']
data_r2s = [r_a**2, r_b**2, r_c**2]
mc_r2s = [r_a_mc, r_b_mc, r_c_mc]
log(f"\n  BEST MODEL (data):  {'R^2 vs MET' if r_a**2 == max(data_r2s) else 'R vs MET' if r_b**2 == max(data_r2s) else 'R vs sqrt(MET)'}")
log(f"  BEST MODEL (MC):    {'R^2 vs MET' if r_a_mc == max(mc_r2s) else 'R vs MET' if r_b_mc == max(mc_r2s) else 'R vs sqrt(MET)'}")


# ============================================================================
# TEST 2: EXCESS Scaling (Data - MC, Then Fit sqrt(E))
# ============================================================================

log("\n" + "=" * 70)
log("TEST 2: EXCESS Scaling (Data - MC, then fit sqrt(E))")
log("=" * 70)
log("The critical missing test. Only the EXCESS over SM should follow sqrt(E).")

ax2 = axes[0, 1]

# Compute excess mean displacement per MET bin
r_bins = np.linspace(0, 30, 61)  # fine R_cav bins (0.5 cm each)
r_centers = (r_bins[:-1] + r_bins[1:]) / 2

excess_mean_R = []
excess_tail5 = []
data_med = []
mc_med = []
valid_excess = []

for i in range(len(met_edges_fine) - 1):
    lo, hi = met_edges_fine[i], met_edges_fine[i+1]
    m_d = sig & (met >= lo) & (met < hi)
    m_m = sig_mc & (met_mc >= lo) & (met_mc < hi)

    n_d = m_d.sum()
    n_m = m_m.sum()

    if n_d < 50 or n_m < 10:
        excess_mean_R.append(np.nan)
        excess_tail5.append(np.nan)
        data_med.append(np.nan)
        mc_med.append(np.nan)
        valid_excess.append(False)
        continue

    h_d, _ = np.histogram(rcav[m_d], bins=r_bins)
    h_m, _ = np.histogram(rcav_mc[m_m], bins=r_bins)

    # Normalize MC to data count in this MET bin
    h_m_norm = h_m * (n_d / n_m) if n_m > 0 else h_m * 0

    h_excess = h_d - h_m_norm  # can be negative

    # Excess mean displacement: sum(R * excess_count) / sum(excess_count)
    total_excess = h_excess.sum()
    if abs(total_excess) > 1:
        mean_excess_R = np.sum(r_centers * h_excess) / total_excess
        # Tail excess
        tail_mask = r_centers > 5.0
        excess_tail = h_excess[tail_mask].sum() / total_excess if total_excess > 0 else np.nan
    else:
        mean_excess_R = np.nan
        excess_tail = np.nan

    excess_mean_R.append(mean_excess_R)
    excess_tail5.append(excess_tail)
    data_med.append(np.median(rcav[m_d]))
    mc_med.append(np.median(rcav_mc[m_m]) if n_m > 0 else np.nan)
    valid_excess.append(True)

excess_mean_R = np.array(excess_mean_R)
excess_tail5 = np.array(excess_tail5)
valid_ex = np.array(valid_excess)

log(f"\n  Excess mean displacement per MET bin:")
log(f"  {'MET bin':>15}  {'<R>_excess':>12}  {'Excess tail':>12}  {'Data med':>10}  {'MC med':>10}")
for i, (lo, hi) in enumerate(zip(met_edges_fine[:-1], met_edges_fine[1:])):
    log(f"  {lo:>6.0f}-{hi:<6.0f}  {excess_mean_R[i]:>12.3f}  {excess_tail5[i]:>12.3f}  "
        f"{data_med[i] if data_med[i] is not None else np.nan:>10.3f}  "
        f"{mc_med[i] if mc_med[i] is not None else np.nan:>10.3f}")

# Fit power law to excess mean displacement
def power_law(E, A, beta):
    return A * E**beta

xf = x_f[valid_ex]
yf_excess = excess_mean_R[valid_ex]
valid_fit = np.isfinite(yf_excess) & (yf_excess > 0)

beta_excess = np.nan
beta_excess_ci = (np.nan, np.nan)

if valid_fit.sum() >= 3:
    try:
        popt, _ = curve_fit(power_law, xf[valid_fit], yf_excess[valid_fit],
                            p0=[0.1, 0.5], bounds=([0, -2], [100, 3]), maxfev=10000)
        beta_excess = popt[1]
        log(f"\n  Excess power law: <R>_excess = {popt[0]:.4f} * MET^{popt[1]:.4f}")
        log(f"  beta_excess = {beta_excess:.4f}  (FTD predicts 0.5, raw data gives ~0.12)")

        # Bootstrap 95% CI on beta_excess (500 resamples)
        log("  Bootstrapping beta_excess (500 resamples)...")
        betas_boot = []
        for b in range(500):
            excess_boot = []
            for i in range(len(met_edges_fine) - 1):
                lo, hi = met_edges_fine[i], met_edges_fine[i+1]
                m_d = sig & (met >= lo) & (met < hi)
                m_m = sig_mc & (met_mc >= lo) & (met_mc < hi)
                n_d = m_d.sum()
                n_m = m_m.sum()
                if n_d < 50 or n_m < 10:
                    excess_boot.append(np.nan)
                    continue
                # Resample data
                idx_d = rng.choice(n_d, n_d, replace=True)
                rcav_boot = rcav[m_d][idx_d]
                h_d_b, _ = np.histogram(rcav_boot, bins=r_bins)
                h_m_b, _ = np.histogram(rcav_mc[m_m], bins=r_bins)
                h_m_b_n = h_m_b * (n_d / n_m) if n_m > 0 else h_m_b * 0
                h_ex = h_d_b - h_m_b_n
                total_ex = h_ex.sum()
                if abs(total_ex) > 1:
                    excess_boot.append(np.sum(r_centers * h_ex) / total_ex)
                else:
                    excess_boot.append(np.nan)
            eb = np.array(excess_boot)
            vb = valid_ex & np.isfinite(eb) & (eb > 0)
            if vb.sum() >= 3:
                try:
                    popt_b, _ = curve_fit(power_law, x_f[vb], eb[vb],
                                          p0=[0.1, 0.5], bounds=([0, -2], [100, 3]), maxfev=5000)
                    betas_boot.append(popt_b[1])
                except:
                    pass
        if len(betas_boot) > 10:
            betas_boot = np.array(betas_boot)
            beta_excess_ci = (np.percentile(betas_boot, 2.5), np.percentile(betas_boot, 97.5))
            log(f"  beta_excess 95% CI: [{beta_excess_ci[0]:.4f}, {beta_excess_ci[1]:.4f}]")
            log(f"  beta=0.5 in CI? {'YES' if beta_excess_ci[0] <= 0.5 <= beta_excess_ci[1] else 'NO'}")
        else:
            log("  Bootstrap failed: insufficient successful fits")
    except Exception as e:
        log(f"  Fit failed: {e}")
else:
    log("  Insufficient valid excess data for fit")

# Plot
ax2.scatter(xf[valid_fit] if valid_fit.sum() > 0 else [], yf_excess[valid_fit] if valid_fit.sum() > 0 else [],
            c='green', s=60, zorder=5, label='Excess <R>')
if not np.isnan(beta_excess) and valid_fit.sum() >= 3:
    xp = np.linspace(200, 1200, 100)
    ax2.plot(xp, power_law(xp, popt[0], popt[1]), 'g--', label=f'beta={beta_excess:.3f}')
    # Show beta=0.5 reference
    A_ref = np.median(yf_excess[valid_fit]) / np.median(xf[valid_fit])**0.5
    ax2.plot(xp, A_ref * xp**0.5, 'r:', alpha=0.7, label='beta=0.5 (FTD)')
ax2.set_xlabel('MET (GeV)')
ax2.set_ylabel('<R>_excess (cm)')
ax2.set_title('Test 2: EXCESS Scaling\n(data-MC, then fit)')
ax2.legend(fontsize=8)
ax2.grid(True, alpha=0.3)


# ============================================================================
# TEST 3: Distribution Width vs MET
# ============================================================================

log("\n" + "=" * 70)
log("TEST 3: Distribution Width vs MET")
log("=" * 70)
log("Cavitation might broaden the distribution, not shift the median.")

ax3 = axes[0, 2]

data_stats_f = bin_data(met_s, rcav_s, met_edges_fine)
mc_stats_f = bin_data(met_m, rcav_m, met_edges_fine)

log(f"\n  {'MET bin':>15}  {'IQR data':>10}  {'IQR MC':>10}  {'Ratio':>8}  "
    f"{'Kurt data':>10}  {'Kurt MC':>10}")
for i, (lo, hi) in enumerate(zip(met_edges_fine[:-1], met_edges_fine[1:])):
    ratio = data_stats_f['iqr'][i] / mc_stats_f['iqr'][i] if mc_stats_f['iqr'][i] > 0 else np.nan
    log(f"  {lo:>6.0f}-{hi:<6.0f}  {data_stats_f['iqr'][i]:>10.3f}  {mc_stats_f['iqr'][i]:>10.3f}  "
        f"{ratio:>8.3f}  {data_stats_f['kurtosis'][i]:>10.3f}  {mc_stats_f['kurtosis'][i]:>10.3f}")

# Fit width vs MET: sigma ~ MET^gamma
v_d = ~np.isnan(data_stats_f['iqr']) & (data_stats_f['n'] > 50)
v_m = ~np.isnan(mc_stats_f['iqr']) & (mc_stats_f['n'] > 10)

gamma_data = np.nan
gamma_mc = np.nan
if v_d.sum() >= 3:
    try:
        popt_w, _ = curve_fit(power_law, x_f[v_d], data_stats_f['iqr'][v_d],
                              p0=[0.1, 0.3], bounds=([0, -2], [100, 3]), maxfev=10000)
        gamma_data = popt_w[1]
        log(f"\n  Width scaling (data): IQR ~ MET^{gamma_data:.4f}")
    except:
        log("\n  Width scaling fit failed (data)")
if v_m.sum() >= 3:
    try:
        popt_wm, _ = curve_fit(power_law, x_f[v_m], mc_stats_f['iqr'][v_m],
                               p0=[0.1, 0.3], bounds=([0, -2], [100, 3]), maxfev=10000)
        gamma_mc = popt_wm[1]
        log(f"  Width scaling (MC):   IQR ~ MET^{gamma_mc:.4f}")
    except:
        log("  Width scaling fit failed (MC)")

# Width ratio vs MET
width_ratio = data_stats_f['iqr'] / mc_stats_f['iqr']
width_ratio[~np.isfinite(width_ratio)] = np.nan
v_wr = np.isfinite(width_ratio)

if v_wr.sum() >= 3:
    sl_wr, it_wr, r_wr, p_wr, _ = stats.linregress(x_f[v_wr], width_ratio[v_wr])
    log(f"\n  Width ratio (data/MC) vs MET: slope = {sl_wr:.6f}, R2 = {r_wr**2:.4f}, p = {p_wr:.2e}")
    log(f"  Width ratio trend: {'INCREASING (cavitation?)' if sl_wr > 0 and p_wr < 0.05 else 'FLAT or DECREASING'}")

# Plot IQR vs MET
ax3.scatter(x_f[v_d], data_stats_f['iqr'][v_d], c='blue', s=40, label='Data IQR')
if v_m.sum() > 0:
    ax3.scatter(x_f[v_m], mc_stats_f['iqr'][v_m], c='red', s=40, marker='^', label='MC IQR')
if not np.isnan(gamma_data):
    xp = np.linspace(200, 1200, 100)
    ax3.plot(xp, power_law(xp, popt_w[0], popt_w[1]), 'b--', alpha=0.7,
             label=f'Data: gamma={gamma_data:.3f}')
ax3.set_xlabel('MET (GeV)')
ax3.set_ylabel('IQR of R_cav (cm)')
ax3.set_title('Test 3: Distribution Width\nvs MET')
ax3.legend(fontsize=8)
ax3.grid(True, alpha=0.3)


# ============================================================================
# TEST 4: Improved Forced beta=0.5 Fits (3 models, multistart)
# ============================================================================

log("\n" + "=" * 70)
log("TEST 4: Improved Forced beta=0.5 Fits")
log("=" * 70)
log("Three saturation models with forced beta=0.5, plus free-beta comparison.")

ax4 = axes[0, 3]

y_med = data_stats_f['median']
v_fit = ~np.isnan(y_med) & (data_stats_f['n'] > 50)
x_fit = x_f[v_fit]
y_fit = y_med[v_fit]

# Bootstrap errors on medians
log("  Computing bootstrapped errors on medians...")
y_err = np.zeros(len(x_fit))
for j, (lo, hi) in enumerate([(met_edges_fine[i], met_edges_fine[i+1])
                                for i in range(len(met_edges_fine)-1) if v_fit[i]]):
    m = sig & (met >= lo) & (met < hi)
    if m.sum() > 20:
        boot_meds = [np.median(rng.choice(rcav[m], m.sum(), replace=True)) for _ in range(200)]
        y_err[j] = np.std(boot_meds)
    else:
        y_err[j] = 0.1
y_err[y_err < 0.001] = 0.001  # floor

# Model A: tanh saturation with forced beta=0.5
def tanh_sqrt(E, R_max, A):
    return R_max * np.tanh(A * np.sqrt(E) / R_max)

# Model B: Michaelis-Menten with forced beta=0.5
def mm_sqrt(E, A, B):
    return A * np.sqrt(E) / (1 + B * np.sqrt(E))

# Model C: Screened with forced beta=0.5
def screened_sqrt(E, A, E_screen):
    return A * np.sqrt(E) * np.exp(-E / E_screen)

# Free-beta power law (comparison)
def power_law_fit(E, A, beta):
    return A * E**beta

# Free-beta saturating (comparison)
def sat_free(E, R_inf, E_half):
    return R_inf * (1 - np.exp(-E / E_half))

models = {}

# Fit each model with multistart
for name, func, p0_list, bounds in [
    ('tanh_sqrt_0.5', tanh_sqrt,
     [(5, 0.1), (8, 0.05), (4, 0.2), (10, 0.01), (3, 0.5)],
     ([1, 0.001], [50, 10])),
    ('MM_sqrt_0.5', mm_sqrt,
     [(0.1, 0.01), (0.5, 0.05), (0.05, 0.001), (1.0, 0.1)],
     ([0.001, 0.0001], [10, 1])),
    ('screened_sqrt_0.5', screened_sqrt,
     [(0.1, 500), (0.05, 300), (0.2, 1000), (0.01, 200)],
     ([0.001, 50], [10, 5000])),
    ('power_law_free', power_law_fit,
     [(0.1, 0.1), (0.5, 0.3), (0.01, 0.5)],
     ([0, -1], [100, 3])),
    ('saturating_free', sat_free,
     [(5, 200), (4, 100), (8, 500), (3, 50)],
     ([1, 10], [50, 5000])),
]:
    best = None
    for p0 in p0_list:
        result = safe_fit(func, x_fit, y_fit, p0=list(p0), bounds=bounds, sigma=y_err)
        if result['success'] and (best is None or result['chi2'] < best['chi2']):
            best = result
    if best is not None and best['success']:
        models[name] = best
    else:
        models[name] = {'chi2': np.inf, 'aic': np.inf, 'bic': np.inf, 'r2': np.nan, 'success': False}

log(f"\n  {'Model':>22}  {'chi2':>8}  {'AIC':>8}  {'BIC':>8}  {'R2':>8}  Parameters")
for name, m in sorted(models.items(), key=lambda x: x[1].get('aic', np.inf)):
    if m['success']:
        pstr = ', '.join(f'{p:.4f}' for p in m['popt'])
        log(f"  {name:>22}  {m['chi2']:>8.4f}  {m['aic']:>8.2f}  {m['bic']:>8.2f}  {m['r2']:>8.4f}  [{pstr}]")
    else:
        log(f"  {name:>22}  {'FAILED':>8}  {'FAILED':>8}  {'FAILED':>8}  {'FAILED':>8}")

# AIC bar chart
ax4_names = [n for n in sorted(models.keys(), key=lambda x: models[x].get('aic', np.inf))
             if models[n]['success']]
ax4_aics = [models[n]['aic'] for n in ax4_names]
if len(ax4_aics) > 0:
    min_aic = min(ax4_aics)
    delta_aics = [a - min_aic for a in ax4_aics]
    colors = ['green' if '0.5' in n else 'steelblue' for n in ax4_names]
    bars = ax4.barh(range(len(ax4_names)), delta_aics, color=colors, edgecolor='black', linewidth=0.5)
    ax4.set_yticks(range(len(ax4_names)))
    ax4.set_yticklabels([n.replace('_', ' ') for n in ax4_names], fontsize=7)
    ax4.set_xlabel('Delta AIC (lower = better)')
    ax4.axvline(x=2, color='orange', linestyle=':', alpha=0.7, label='DAIC=2 threshold')
    ax4.axvline(x=6, color='red', linestyle=':', alpha=0.7, label='DAIC=6 strong')
ax4.set_title('Test 4: Model Comparison\n(green = forced beta=0.5)')
ax4.legend(fontsize=7)
ax4.grid(True, alpha=0.3, axis='x')

# Determine if any beta=0.5 model is competitive
best_free = min([models[n]['aic'] for n in models if 'free' in n and models[n]['success']], default=np.inf)
best_05 = min([models[n]['aic'] for n in models if '0.5' in n and models[n]['success']], default=np.inf)
daic_05_vs_free = best_05 - best_free
log(f"\n  BEST free-beta AIC: {best_free:.2f}")
log(f"  BEST forced-0.5 AIC: {best_05:.2f}")
log(f"  DAIC (forced 0.5 vs free): {daic_05_vs_free:.2f}")
log(f"  beta=0.5 {'COMPETITIVE (DAIC < 2)' if daic_05_vs_free < 2 else 'WEAKLY DISFAVORED (2 < DAIC < 6)' if daic_05_vs_free < 6 else 'STRONGLY DISFAVORED (DAIC > 6)'}")


# ============================================================================
# TEST 5: Data - MC Displacement DIFFERENCE vs sqrt(MET)
# ============================================================================

log("\n" + "=" * 70)
log("TEST 5: Data - MC Displacement Difference vs sqrt(MET)")
log("=" * 70)

ax5 = axes[1, 0]

# Compute deltaR = median_data - median_MC per MET bin
delta_R = data_stats_f['median'] - mc_stats_f['median']
delta_p90 = data_stats_f['p90'] - mc_stats_f['p90']
delta_tail5 = data_stats_f['tail5'] - mc_stats_f['tail5']

# Bootstrap 95% CI on deltaR
log("  Bootstrapping deltaR (200 resamples per bin)...")
delta_R_err = np.full(len(x_f), np.nan)
for i in range(len(met_edges_fine) - 1):
    lo, hi = met_edges_fine[i], met_edges_fine[i+1]
    m_d = sig & (met >= lo) & (met < hi)
    m_m = sig_mc & (met_mc >= lo) & (met_mc < hi)
    n_d, n_m = m_d.sum(), m_m.sum()
    if n_d > 50 and n_m > 10:
        dr_boot = []
        for _ in range(200):
            med_d = np.median(rng.choice(rcav[m_d], n_d, replace=True))
            med_m = np.median(rng.choice(rcav_mc[m_m], n_m, replace=True))
            dr_boot.append(med_d - med_m)
        delta_R_err[i] = np.std(dr_boot)

v_dr = np.isfinite(delta_R) & np.isfinite(delta_R_err) & (delta_R_err > 0)

log(f"\n  {'MET bin':>15}  {'deltaR':>10}  {'err':>8}  {'delta_tail5':>12}")
for i, (lo, hi) in enumerate(zip(met_edges_fine[:-1], met_edges_fine[1:])):
    log(f"  {lo:>6.0f}-{hi:<6.0f}  {delta_R[i]:>10.3f}  {delta_R_err[i]:>8.3f}  {delta_tail5[i]:>12.4f}")

# Fit three models to deltaR(MET)
def constant(E, c):
    return np.full_like(E, c)

def linear_sqrt(E, a, b):
    return a + b * np.sqrt(E)

def linear_met(E, a, b):
    return a + b * E

aic_dr = {}
if v_dr.sum() >= 3:
    for name, func, p0, npar in [
        ('constant', constant, [0.3], 1),
        ('linear_sqrt', linear_sqrt, [0, 0.01], 2),
        ('linear_MET', linear_met, [0, 0.001], 2),
    ]:
        result = safe_fit(func, x_f[v_dr], delta_R[v_dr], p0=p0, sigma=delta_R_err[v_dr])
        aic_dr[name] = result
        if result['success']:
            log(f"  deltaR ~ {name}: AIC={result['aic']:.2f}, R2={result['r2']:.4f}, params={[f'{p:.4f}' for p in result['popt']]}")
        else:
            log(f"  deltaR ~ {name}: FAILED ({result.get('error', 'unknown')})")

# Plot
ax5.errorbar(np.sqrt(x_f[v_dr]), delta_R[v_dr], yerr=delta_R_err[v_dr],
             fmt='ko', capsize=3, label='deltaR = med(data)-med(MC)')
xp_sqrt = np.linspace(np.sqrt(200), np.sqrt(1200), 100)
for name, color, func in [('constant', 'gray', constant),
                            ('linear_sqrt', 'green', linear_sqrt),
                            ('linear_MET', 'orange', linear_met)]:
    if name in aic_dr and aic_dr[name]['success']:
        if name == 'linear_MET':
            ax5.plot(xp_sqrt, func(xp_sqrt**2, *aic_dr[name]['popt']), color=color, linestyle='--',
                     label=f'{name} (AIC={aic_dr[name]["aic"]:.1f})')
        else:
            ax5.plot(xp_sqrt, func(xp_sqrt**2 if name == 'constant' else xp_sqrt, *aic_dr[name]['popt']),
                     color=color, linestyle='--',
                     label=f'{name} (AIC={aic_dr[name]["aic"]:.1f})')
ax5.set_xlabel('sqrt(MET) (GeV^0.5)')
ax5.set_ylabel('deltaR (cm)')
ax5.set_title('Test 5: Excess Profile\nmed(data)-med(MC) vs sqrt(MET)')
ax5.axhline(y=0, color='gray', linestyle='-', alpha=0.3)
ax5.legend(fontsize=7)
ax5.grid(True, alpha=0.3)

# Best model
if len(aic_dr) > 0:
    best_name = min(aic_dr, key=lambda x: aic_dr[x].get('aic', np.inf))
    log(f"\n  BEST model for deltaR: {best_name} (AIC={aic_dr[best_name]['aic']:.2f})")
    for n in aic_dr:
        if n != best_name and aic_dr[n]['success']:
            da = aic_dr[n]['aic'] - aic_dr[best_name]['aic']
            log(f"  {n}: DAIC = {da:.2f}")


# ============================================================================
# TEST 6: Selection Bias Test
# ============================================================================

log("\n" + "=" * 70)
log("TEST 6: Selection Bias Test")
log("=" * 70)
log("Does the dlenSig > 30 cut create the correlation artificially?")

ax6 = axes[1, 1]

# 6a: Direct dlenSig-MET correlation
rho_dlsig_met, p_dlsig_met = stats.spearmanr(dlsig_s, met_s)
log(f"\n  Direct dlenSig-MET correlation (signal region):")
log(f"  Spearman rho = {rho_dlsig_met:.4f}, p = {p_dlsig_met:.2e}")
log(f"  {'STRONG BIAS' if abs(rho_dlsig_met) > 0.1 else 'WEAK BIAS' if abs(rho_dlsig_met) > 0.05 else 'NEGLIGIBLE BIAS'}")

# 6b: Conditional test in narrow dlenSig bands
log(f"\n  Conditional test: rho(sqrt(MET), R_cav) in narrow dlenSig bands:")
bands = [(25, 35), (35, 45), (45, 55), (55, 75), (75, 100)]
cond_rhos = []
for lo, hi in bands:
    m = sig & (dlsig >= lo) & (dlsig < hi)
    n = m.sum()
    if n > 100:
        rho, p = stats.spearmanr(np.sqrt(met[m]), rcav[m])
        cond_rhos.append((lo, hi, n, rho, p))
        log(f"  dlenSig [{lo:>3},{hi:>3}):  N={n:>8,}  rho={rho:>+.4f}  p={p:.2e}")
    else:
        cond_rhos.append((lo, hi, n, np.nan, np.nan))
        log(f"  dlenSig [{lo:>3},{hi:>3}):  N={n:>8,}  (insufficient)")

# 6c: 2D scan of excess rho vs (dlenSig_cut, MET_threshold)
dlsig_cuts = [10, 20, 30, 40, 50, 75, 100]
met_thresholds = [200, 250, 300, 400, 500]
rho_grid = np.full((len(dlsig_cuts), len(met_thresholds)), np.nan)

for i, dc in enumerate(dlsig_cuts):
    for j, mt in enumerate(met_thresholds):
        m_d = (dlsig > dc) & (met >= mt)
        m_m = (dlsig_mc > dc) & (met_mc >= mt)
        n_d, n_m = m_d.sum(), m_m.sum()
        if n_d > 100:
            rho_d, _ = stats.spearmanr(np.sqrt(met[m_d]), rcav[m_d])
            rho_m = np.nan
            if n_m > 30:
                rho_m, _ = stats.spearmanr(np.sqrt(met_mc[m_m]), rcav_mc[m_m])
            rho_grid[i, j] = rho_d - rho_m if np.isfinite(rho_m) else rho_d

# Plot heatmap
im = ax6.imshow(rho_grid, aspect='auto', cmap='RdYlGn',
                extent=[met_thresholds[0], met_thresholds[-1], dlsig_cuts[-1], dlsig_cuts[0]],
                vmin=-0.05, vmax=0.15)
ax6.set_xlabel('MET threshold (GeV)')
ax6.set_ylabel('dlenSig cut')
ax6.set_title('Test 6: Selection Bias\nexcess rho vs cuts')
plt.colorbar(im, ax=ax6, label='rho(data) - rho(MC)')

# 6d: Alternative purity cuts
log(f"\n  Alternative purity cuts (no dlenSig, MET>200):")
# Use SV ntracks instead
for ntk_cut in [3, 4, 5]:
    m = (ntracks >= ntk_cut) & (met >= 200)
    if m.sum() > 100:
        rho, p = stats.spearmanr(np.sqrt(met[m]), rcav[m])
        log(f"  ntracks >= {ntk_cut}: N={m.sum():>8,}, rho(sqrt(MET), R_cav) = {rho:>+.4f}")

# Use SV mass windows
for lo_m, hi_m, label in [(0, 1.5, 'light'), (1.5, 4.0, 'D-like'), (4.0, 7.0, 'B-like'), (7.0, 100, 'exotic')]:
    m = (svmass >= lo_m) & (svmass < hi_m) & (met >= 200) & (dlsig > 30)
    if m.sum() > 100:
        rho, p = stats.spearmanr(np.sqrt(met[m]), rcav[m])
        log(f"  SV mass [{lo_m:.1f},{hi_m:.1f}) + dlenSig>30: N={m.sum():>8,}, rho = {rho:>+.4f}")


# ============================================================================
# TEST 7: Partial Correlation (Controlling for Kinematics)
# ============================================================================

log("\n" + "=" * 70)
log("TEST 7: Partial Correlation (Controlling for Kinematics)")
log("=" * 70)
log("If MET-R_cav is purely kinematic, it vanishes when controlling for SV mass.")

ax7 = axes[1, 2]

# OLS regression: R_cav = b0 + b1*sv_mass + b2*has_bjet + b3*ntracks
# Then check: spearmanr(sqrt(MET), residuals)

from numpy.linalg import lstsq

# Data
X_d = np.column_stack([np.ones(len(met_s)), svmass_s, bjet_s.astype(float), ntracks_s.astype(float)])
beta_ols, _, _, _ = lstsq(X_d, rcav_s, rcond=None)
rcav_pred_d = X_d @ beta_ols
rcav_resid_d = rcav_s - rcav_pred_d

rho_partial_d, p_partial_d = stats.spearmanr(np.sqrt(met_s), rcav_resid_d)

log(f"\n  Data OLS: R_cav = {beta_ols[0]:.3f} + {beta_ols[1]:.3f}*sv_mass + "
    f"{beta_ols[2]:.3f}*has_bjet + {beta_ols[3]:.3f}*ntracks")
log(f"  Partial correlation rho(sqrt(MET), R_cav_resid): {rho_partial_d:>+.4f}  (p={p_partial_d:.2e})")
log(f"  Raw correlation rho(sqrt(MET), R_cav):            {stats.spearmanr(np.sqrt(met_s), rcav_s)[0]:>+.4f}")
log(f"  Reduction: {(1 - abs(rho_partial_d) / abs(stats.spearmanr(np.sqrt(met_s), rcav_s)[0])) * 100:.1f}%")

# MC (no ntracks available, use sv_mass + bjet only)
if len(met_m) > 50:
    X_m = np.column_stack([np.ones(len(met_m)), svmass_m, bjet_m.astype(float)])
    beta_ols_m, _, _, _ = lstsq(X_m, rcav_m, rcond=None)
    rcav_pred_m = X_m @ beta_ols_m
    rcav_resid_m = rcav_m - rcav_pred_m
    rho_partial_m, p_partial_m = stats.spearmanr(np.sqrt(met_m), rcav_resid_m)
    rho_raw_m = stats.spearmanr(np.sqrt(met_m), rcav_m)[0]
    log(f"\n  MC partial rho(sqrt(MET), R_cav_resid): {rho_partial_m:>+.4f}  (p={p_partial_m:.2e})")
    log(f"  MC raw rho:                              {rho_raw_m:>+.4f}")
    log(f"\n  EXCESS partial rho: {rho_partial_d - rho_partial_m:>+.4f}")
    log(f"  EXCESS raw rho:     {stats.spearmanr(np.sqrt(met_s), rcav_s)[0] - rho_raw_m:>+.4f}")
else:
    rho_partial_m = np.nan

# Also: partial correlation controlling ONLY for sv_mass (simpler, more interpretable)
# Using rank regression approach
from scipy.stats import rankdata
rank_met = rankdata(np.sqrt(met_s))
rank_rcav = rankdata(rcav_s)
rank_svmass = rankdata(svmass_s)

# Regress out sv_mass from both
X_sv = np.column_stack([np.ones(len(rank_svmass)), rank_svmass])
beta_met_sv, _, _, _ = lstsq(X_sv, rank_met, rcond=None)
beta_rcav_sv, _, _, _ = lstsq(X_sv, rank_rcav, rcond=None)
resid_met = rank_met - X_sv @ beta_met_sv
resid_rcav = rank_rcav - X_sv @ beta_rcav_sv

rho_partial_sv, p_partial_sv = stats.spearmanr(resid_met, resid_rcav)
log(f"\n  Partial Spearman rho(sqrt(MET), R_cav | sv_mass): {rho_partial_sv:>+.4f}  (p={p_partial_sv:.2e})")
log(f"  This is the correlation AFTER removing the effect of SV mass")
log(f"  {'SURVIVES' if abs(rho_partial_sv) > 0.02 and p_partial_sv < 0.05 else 'DOES NOT SURVIVE'} kinematic control")

# Plot: raw vs partial correlation
bar_labels = ['Raw rho', 'Partial\n(OLS)', 'Partial\n(rank|sv_mass)']
bar_data = [stats.spearmanr(np.sqrt(met_s), rcav_s)[0], rho_partial_d, rho_partial_sv]
bar_mc = [rho_raw_m if not np.isnan(rho_partial_m) else 0,
          rho_partial_m if not np.isnan(rho_partial_m) else 0, np.nan]

x_bar = np.arange(len(bar_labels))
width = 0.35
ax7.bar(x_bar - width/2, bar_data, width, label='Data', color='steelblue')
ax7.bar(x_bar[:2] + width/2, bar_mc[:2], width, label='MC', color='salmon')
ax7.set_xticks(x_bar)
ax7.set_xticklabels(bar_labels, fontsize=8)
ax7.set_ylabel('Spearman rho')
ax7.set_title('Test 7: Partial Correlation\n(controlling for kinematics)')
ax7.axhline(y=0, color='gray', linestyle='-', alpha=0.3)
ax7.legend(fontsize=8)
ax7.grid(True, alpha=0.3, axis='y')


# ============================================================================
# TEST 8: Simple Kinematic Model
# ============================================================================

log("\n" + "=" * 70)
log("TEST 8: Simple Kinematic Model")
log("=" * 70)
log("Predict R_cav from B/D/K meson kinematics. Does it explain the data?")

ax8 = axes[1, 3]

# B meson: tau ~ 1.5 ps, m ~ 5.28 GeV, ctau ~ 450 um = 0.045 cm
# D meson: tau ~ 0.4 ps, m ~ 1.87 GeV, ctau ~ 120 um = 0.012 cm
# K_S:     tau ~ 90 ps,  m ~ 0.498 GeV, ctau ~ 2.68 cm

species = {
    'B meson':  {'tau_ps': 1.5, 'm_GeV': 5.28, 'ctau_cm': 0.045},
    'D meson':  {'tau_ps': 0.41, 'm_GeV': 1.87, 'ctau_cm': 0.012},
    'K_S':      {'tau_ps': 89.5, 'm_GeV': 0.498, 'ctau_cm': 2.68},
}

# Estimate f = p_T(hadron) / MET from the B-mass subsample
b_mask = sig & (svmass >= 4.0) & (svmass < 7.0) & (met >= 200)
if b_mask.sum() > 100:
    # Median R_cav in B-mass window at different MET
    # R ~ (p_T / m) * ctau = (f * MET / m) * ctau
    # => f = R * m / (MET * ctau)
    f_est = rcav[b_mask] * 5.28 / (met[b_mask] * 0.045)
    f_median = np.median(f_est)
    log(f"\n  Estimated f = p_T(B) / MET: median = {f_median:.4f}")
    log(f"  (This relates jet p_T to MET via fragmentation fraction)")
else:
    f_median = 0.3  # fallback
    log(f"\n  Using fallback f = {f_median}")

# Compute kinematic model curves
met_model = np.linspace(200, 1200, 100)
for name, props in species.items():
    d_model = f_median * met_model / props['m_GeV'] * props['ctau_cm']
    ax8.plot(met_model, d_model, '--', label=f'{name} (kin.)', alpha=0.7)
    log(f"  {name}: d(MET=300) = {f_median * 300 / props['m_GeV'] * props['ctau_cm']:.3f} cm, "
        f"d(MET=600) = {f_median * 600 / props['m_GeV'] * props['ctau_cm']:.3f} cm")
    log(f"    Kinematic beta = 1.0 (linear in MET)")

# Overlay data and MC medians
ax8.scatter(x_f[v_fit], y_med[v_fit], c='blue', s=40, zorder=5, label='Data median')
v_mc = ~np.isnan(mc_stats_f['median']) & (mc_stats_f['n'] > 10)
if v_mc.sum() > 0:
    ax8.scatter(x_f[v_mc], mc_stats_f['median'][v_mc], c='red', s=40, marker='^', zorder=5, label='MC median')

# Observed beta
log(f"\n  Observed beta (data) ~ 0.097 (from prior analysis)")
log(f"  Kinematic beta = 1.0 (linear)")
log(f"  The huge discrepancy (0.097 vs 1.0) means:")
log(f"  -> The SV population CHANGES with MET (not just boosted)")
log(f"  -> At high MET, fewer long-lived particles (K_S, B) remain")
log(f"  -> Detector acceptance saturates the observable")

ax8.set_xlabel('MET (GeV)')
ax8.set_ylabel('R_cav (cm)')
ax8.set_title('Test 8: Kinematic Model\nvs observed scaling')
ax8.legend(fontsize=7)
ax8.set_ylim(0, 15)
ax8.grid(True, alpha=0.3)


# ============================================================================
# ROW 3: Additional diagnostic panels
# ============================================================================

# Panel 3a: Kurtosis vs MET
ax3a = axes[2, 0]
v_k = ~np.isnan(data_stats_f['kurtosis']) & (data_stats_f['n'] > 50)
ax3a.scatter(x_f[v_k], data_stats_f['kurtosis'][v_k], c='blue', s=40, label='Data')
v_km = ~np.isnan(mc_stats_f['kurtosis']) & (mc_stats_f['n'] > 10)
if v_km.sum() > 0:
    ax3a.scatter(x_f[v_km], mc_stats_f['kurtosis'][v_km], c='red', s=40, marker='^', label='MC')
ax3a.set_xlabel('MET (GeV)')
ax3a.set_ylabel('Kurtosis')
ax3a.set_title('Kurtosis vs MET\n(heavy tail indicator)')
ax3a.legend(fontsize=8)
ax3a.grid(True, alpha=0.3)

# Panel 3b: beta_excess bootstrap distribution
ax3b = axes[2, 1]
if 'betas_boot' in dir() and len(betas_boot) > 10:
    ax3b.hist(betas_boot, bins=30, color='green', alpha=0.7, edgecolor='black', linewidth=0.5)
    ax3b.axvline(x=0.5, color='red', linestyle='--', linewidth=2, label='FTD: beta=0.5')
    ax3b.axvline(x=0.12, color='blue', linestyle='--', linewidth=2, label='Raw: beta~0.12')
    if not np.isnan(beta_excess):
        ax3b.axvline(x=beta_excess, color='green', linestyle='-', linewidth=2, label=f'Excess: {beta_excess:.3f}')
    ax3b.set_xlabel('beta_excess')
    ax3b.set_ylabel('Bootstrap count')
    ax3b.legend(fontsize=8)
else:
    ax3b.text(0.5, 0.5, 'Bootstrap\nnot available', ha='center', va='center', fontsize=12, transform=ax3b.transAxes)
ax3b.set_title('beta_excess Bootstrap\n(500 resamples)')
ax3b.grid(True, alpha=0.3)

# Panel 3c: Conditional rho in dlenSig bands
ax3c = axes[2, 2]
band_labels = [f'[{lo},{hi})' for lo, hi, n, r, p in cond_rhos if n > 100]
band_rhos = [r for lo, hi, n, r, p in cond_rhos if n > 100]
if len(band_labels) > 0:
    ax3c.bar(range(len(band_labels)), band_rhos, color='steelblue', edgecolor='black', linewidth=0.5)
    ax3c.set_xticks(range(len(band_labels)))
    ax3c.set_xticklabels(band_labels, fontsize=7, rotation=30)
    ax3c.axhline(y=0, color='gray', linestyle='-', alpha=0.3)
    ax3c.axhline(y=rho_dlsig_met, color='red', linestyle=':', label=f'dlenSig-MET rho={rho_dlsig_met:.3f}')
    ax3c.legend(fontsize=7)
ax3c.set_xlabel('dlenSig band')
ax3c.set_ylabel('rho(sqrt(MET), R_cav)')
ax3c.set_title('Conditional rho in\ndlenSig bands')
ax3c.grid(True, alpha=0.3, axis='y')

# Panel 3d: Width ratio (data/MC) vs MET
ax3d = axes[2, 3]
if v_wr.sum() > 0:
    ax3d.scatter(x_f[v_wr], width_ratio[v_wr], c='purple', s=40, zorder=5)
    if v_wr.sum() >= 3:
        xp = np.linspace(200, 1200, 100)
        ax3d.plot(xp, sl_wr * xp + it_wr, 'purple', linestyle='--', alpha=0.7,
                 label=f'slope={sl_wr:.5f}, p={p_wr:.3f}')
    ax3d.axhline(y=1, color='gray', linestyle='-', alpha=0.5)
    ax3d.legend(fontsize=8)
ax3d.set_xlabel('MET (GeV)')
ax3d.set_ylabel('IQR ratio (data/MC)')
ax3d.set_title('Width Ratio vs MET\n(>1 = data broader)')
ax3d.grid(True, alpha=0.3)


# ============================================================================
# ROW 4: More diagnostics + Summary Scorecard
# ============================================================================

# Panel 4a: dlenSig vs MET scatter (subsample)
ax4a = axes[3, 0]
n_scat = min(10000, len(met_s))
idx_scat = rng.choice(len(met_s), n_scat, replace=False)
ax4a.scatter(met_s[idx_scat], dlsig_s[idx_scat], c='steelblue', s=1, alpha=0.3, rasterized=True)
ax4a.set_xlabel('MET (GeV)')
ax4a.set_ylabel('dlenSig')
ax4a.set_title(f'dlenSig vs MET\nrho={rho_dlsig_met:.3f}')
ax4a.set_ylim(25, 200)
ax4a.grid(True, alpha=0.3)

# Panel 4b: Residualized R vs MET
ax4b = axes[3, 1]
# Bin the residualized R_cav by MET
resid_stats = bin_data(met_s, rcav_resid_d, met_edges_fine)
v_resid = ~np.isnan(resid_stats['median']) & (resid_stats['n'] > 50)
ax4b.scatter(x_f[v_resid], resid_stats['median'][v_resid], c='green', s=40, zorder=5, label='Data residual')
ax4b.axhline(y=0, color='gray', linestyle='-', alpha=0.3)
# Fit power law to residualized medians (only positive ones)
v_pos = v_resid & (resid_stats['median'] > 0)
if v_pos.sum() >= 3:
    try:
        popt_resid, _ = curve_fit(power_law_fit, x_f[v_pos], resid_stats['median'][v_pos],
                                  p0=[0.01, 0.5], bounds=([0, -2], [100, 3]))
        xp = np.linspace(200, 1200, 100)
        ax4b.plot(xp, power_law_fit(xp, *popt_resid), 'g--',
                 label=f'beta={popt_resid[1]:.3f}')
        log(f"\n  Residualized median scaling: beta = {popt_resid[1]:.4f}")
    except:
        pass
ax4b.set_xlabel('MET (GeV)')
ax4b.set_ylabel('Residualized median R_cav (cm)')
ax4b.set_title('Residualized R_cav\n(kinematics removed)')
ax4b.legend(fontsize=8)
ax4b.grid(True, alpha=0.3)

# Panel 4c: Multi-species overlay
ax4c = axes[3, 2]
# Show data, MC, and kinematic model curves on same plot
ax4c.scatter(x_f[v_fit], y_med[v_fit], c='blue', s=40, zorder=5, label='Data median')
if v_mc.sum() > 0:
    ax4c.scatter(x_f[v_mc], mc_stats_f['median'][v_mc], c='red', s=40, marker='^', zorder=5, label='MC median')
# Difference
v_both = v_fit & v_mc
if v_both.sum() > 0:
    ax4c.scatter(x_f[v_both], delta_R[v_both], c='green', s=40, marker='s', zorder=5, label='Excess (data-MC)')
# sqrt(MET) reference
if not np.isnan(beta_excess):
    A_ref2 = 0.2  # rough scaling
    xp = np.linspace(200, 1200, 100)
    ax4c.plot(xp, A_ref2 * np.sqrt(xp), 'r:', alpha=0.5, label='~sqrt(MET)')
ax4c.set_xlabel('MET (GeV)')
ax4c.set_ylabel('R_cav / deltaR (cm)')
ax4c.set_title('Data vs MC vs Excess\nvs sqrt(MET) reference')
ax4c.legend(fontsize=7)
ax4c.grid(True, alpha=0.3)

# Panel 4d: Summary Scorecard
ax4d = axes[3, 3]
ax4d.axis('off')

# Compile scorecard
scorecard = []
scorecard.append(('Test 1: R^2 vs MET',
                  f'R2_fit(R^2~MET)={r_a**2:.5f}',
                  'WEAK' if r_a**2 < 0.01 else 'MODERATE'))
scorecard.append(('Test 2: Excess beta',
                  f'beta_excess={beta_excess:.3f}' if not np.isnan(beta_excess) else 'FAILED',
                  'SUPPORTS 0.5' if not np.isnan(beta_excess) and beta_excess_ci[0] <= 0.5 <= beta_excess_ci[1]
                  else 'AGAINST 0.5' if not np.isnan(beta_excess) else 'INCONCLUSIVE'))
scorecard.append(('Test 3: Width scaling',
                  f'gamma={gamma_data:.3f}' if not np.isnan(gamma_data) else 'FAILED',
                  'SUPPORTS 0.5' if not np.isnan(gamma_data) and abs(gamma_data - 0.5) < 0.2
                  else 'AGAINST 0.5'))
scorecard.append(('Test 4: AIC forced 0.5',
                  f'DAIC={daic_05_vs_free:.1f}',
                  'COMPETITIVE' if daic_05_vs_free < 2 else 'DISFAVORED' if daic_05_vs_free < 6 else 'EXCLUDED'))
scorecard.append(('Test 5: deltaR profile',
                  best_name if len(aic_dr) > 0 else 'FAILED',
                  'FTD' if best_name == 'linear_sqrt' else 'KINEMATIC' if best_name == 'linear_MET' else 'FLAT'))
scorecard.append(('Test 6: Selection bias',
                  f'rho(dlsig,MET)={rho_dlsig_met:.3f}',
                  'BIAS' if abs(rho_dlsig_met) > 0.1 else 'OK'))
scorecard.append(('Test 7: Partial corr.',
                  f'rho_partial={rho_partial_sv:.4f}',
                  'SURVIVES' if abs(rho_partial_sv) > 0.02 and p_partial_sv < 0.05 else 'VANISHES'))
scorecard.append(('Test 8: Kinematic model',
                  f'beta_kin=1.0 vs obs=0.097',
                  'POPULATION SHIFT'))

y_pos = 0.95
for test, value, verdict in scorecard:
    color = 'green' if 'SUPPORT' in verdict or verdict == 'FTD' or verdict == 'COMPETITIVE' or verdict == 'SURVIVES' else \
            'red' if 'AGAINST' in verdict or 'EXCLUDED' in verdict or verdict == 'VANISHES' else 'orange'
    ax4d.text(0.02, y_pos, test, fontsize=8, fontweight='bold', transform=ax4d.transAxes, va='top')
    ax4d.text(0.55, y_pos, value, fontsize=7, transform=ax4d.transAxes, va='top')
    ax4d.text(0.88, y_pos, verdict, fontsize=8, fontweight='bold', color=color,
              transform=ax4d.transAxes, va='top', ha='center')
    y_pos -= 0.11

ax4d.set_title('Summary Scorecard', fontweight='bold')
ax4d.add_patch(plt.Rectangle((0, 0), 1, 1, transform=ax4d.transAxes,
                               fill=True, facecolor='lightyellow', edgecolor='black', linewidth=1))


# ============================================================================
# SAVE FIGURE AND RESULTS
# ============================================================================

plt.tight_layout(rect=[0, 0, 1, 0.96])
outpath = os.path.join(sim_dir, 'ftd_cavitation_REINVESTIGATION.png')
fig.savefig(outpath, dpi=150, bbox_inches='tight')
log(f"\nFigure saved: {outpath}")
plt.close()

# ============================================================================
# FINAL SUMMARY
# ============================================================================

log("\n" + "=" * 70)
log("FINAL SUMMARY: CAN beta=0.5 BE RESCUED?")
log("=" * 70)

log(f"\n  Key findings:")
log(f"  1. R^2 vs MET linear fit: R2={r_a**2:.6f} (event-level, very weak)")
log(f"  2. Excess scaling beta: {beta_excess:.4f}" if not np.isnan(beta_excess)
    else "  2. Excess scaling: FAILED")
if not np.isnan(beta_excess):
    log(f"     95% CI: [{beta_excess_ci[0]:.4f}, {beta_excess_ci[1]:.4f}]")
    log(f"     beta=0.5 in CI: {'YES' if beta_excess_ci[0] <= 0.5 <= beta_excess_ci[1] else 'NO'}")
log(f"  3. Width scaling gamma: {gamma_data:.4f}" if not np.isnan(gamma_data)
    else "  3. Width scaling: FAILED")
log(f"  4. AIC forced beta=0.5: DAIC={daic_05_vs_free:.1f}")
log(f"  5. deltaR best model: {best_name}" if len(aic_dr) > 0 else "  5. deltaR: FAILED")
log(f"  6. Selection bias: rho(dlenSig,MET)={rho_dlsig_met:.4f}")
log(f"  7. Partial correlation: {rho_partial_sv:>+.4f} (after removing sv_mass)")
log(f"  8. Kinematic model: beta_kin=1.0 >> beta_obs=0.097 (population shift dominates)")

# Count supports vs against
n_supports = sum(1 for _, _, v in scorecard if 'SUPPORT' in v or v == 'FTD' or v == 'COMPETITIVE' or v == 'SURVIVES')
n_against = sum(1 for _, _, v in scorecard if 'AGAINST' in v or 'EXCLUDED' in v or v == 'VANISHES')
n_neutral = len(scorecard) - n_supports - n_against

log(f"\n  VERDICT: {n_supports} tests SUPPORT beta=0.5, {n_against} AGAINST, {n_neutral} NEUTRAL/INCONCLUSIVE")
if n_supports > n_against:
    log("  => beta=0.5 may be RESCUABLE with proper excess analysis")
elif n_supports == n_against:
    log("  => INCONCLUSIVE: mixed evidence")
else:
    log("  => beta=0.5 remains DISFAVORED even after reinvestigation")

log(f"\n  CRITICAL INSIGHT: The observable (max SV_dxy) is kinematic, not topological.")
log(f"  The FTD prediction applies to a bubble radius, which would manifest differently.")
log(f"  The excess (data - MC) is the relevant quantity, not the absolute scaling.")

# Save results
results_path = os.path.join(sim_dir, 'ftd_reinvestigation_results.txt')
with open(results_path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(results_lines))
log(f"\nResults saved: {results_path}")

print("\n" + "=" * 70)
print("REINVESTIGATION COMPLETE")
print("=" * 70)
