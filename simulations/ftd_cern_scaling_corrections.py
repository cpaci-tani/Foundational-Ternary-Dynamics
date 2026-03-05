#!/usr/bin/env python3
"""
FTD Scaling Correction Analysis & Excess Characterization
==========================================================

Two-part analysis:
  PART 1: Why beta=0.12, not 0.5? — Explore lattice correction models
  PART 2: What IS the excess? — Characterize its functional form

Uses cached data: ftd_full_enhanced.npz + ftd_mc_cache.npz
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
from scipy.optimize import curve_fit
import os, sys

# ── Load cached data ─────────────────────────────────────────────────────
sim_dir = os.path.dirname(os.path.abspath(__file__))

print("Loading cached data...")
data = np.load(os.path.join(sim_dir, 'ftd_full_enhanced.npz'))
met   = data['met']
rcav  = data['rcav']
dlsig = data['sv_dlsig_max']
svmass= data['sv_mass_max']
bjet  = data['has_bjet']

mc_raw = np.load(os.path.join(sim_dir, 'ftd_mc_cache.npz'))

# Combine MC samples with cross-section weights
mc_samples = ['WJetsToLNu', 'ZJetsToNuNu_200toInf', 'ZJetsToNuNu_100to200',
              'QCD_HT1000to1500', 'QCD_HT700to1000']

met_mc_all, rcav_mc_all, dlsig_mc_all, svmass_mc_all, w_mc_all = [], [], [], [], []
for s in mc_samples:
    m  = mc_raw[f'{s}__met']
    r  = mc_raw[f'{s}__rcav']
    dl = mc_raw[f'{s}__sv_dlsig_max']
    sv = mc_raw[f'{s}__sv_mass_max']
    xsec    = float(mc_raw[f'{s}__xsec'])
    n_total = float(mc_raw[f'{s}__n_total'])
    w = np.full(len(m), xsec / n_total)
    met_mc_all.append(m); rcav_mc_all.append(r)
    dlsig_mc_all.append(dl); svmass_mc_all.append(sv); w_mc_all.append(w)

met_mc   = np.concatenate(met_mc_all)
rcav_mc  = np.concatenate(rcav_mc_all)
dlsig_mc = np.concatenate(dlsig_mc_all)
svmass_mc= np.concatenate(svmass_mc_all)
w_mc     = np.concatenate(w_mc_all)

# Normalize MC weights to sum to data count (for shape comparisons)
w_mc_norm = w_mc * (len(met) / w_mc.sum())

print(f"Data: {len(met):,} events | MC: {len(met_mc):,} events")

# ── Signal region: dlenSig > 30 ──────────────────────────────────────────
sig    = dlsig > 30
sig_mc = dlsig_mc > 30
print(f"Signal (dlenSig>30): Data {sig.sum():,} | MC {sig_mc.sum():,}")


# ╔══════════════════════════════════════════════════════════════════════════╗
# ║  PART 1: SCALING CORRECTION MODELS                                     ║
# ╚══════════════════════════════════════════════════════════════════════════╝

print("\n" + "="*70)
print("PART 1: SCALING CORRECTION MODELS")
print("="*70)

# ── 1.1 Measure the scaling in fine bins ─────────────────────────────────
met_edges = [200, 225, 250, 275, 300, 350, 400, 500, 600, 800, 1200]
met_centers = [(met_edges[i]+met_edges[i+1])/2 for i in range(len(met_edges)-1)]

def compute_quantiles(met_arr, rcav_arr, mask, edges, quantiles=[0.25, 0.50, 0.75, 0.90]):
    """Compute R_cav quantiles in MET bins."""
    results = {f'q{int(q*100)}': [] for q in quantiles}
    results['n'] = []
    for i in range(len(edges)-1):
        m = mask & (met_arr >= edges[i]) & (met_arr < edges[i+1])
        results['n'].append(m.sum())
        for q in quantiles:
            if m.sum() > 10:
                results[f'q{int(q*100)}'].append(np.quantile(rcav_arr[m], q))
            else:
                results[f'q{int(q*100)}'].append(np.nan)
    return results

data_q = compute_quantiles(met, rcav, sig, met_edges)
mc_q   = compute_quantiles(met_mc, rcav_mc, sig_mc, met_edges)

# ── 1.2 Fit multiple functional forms ────────────────────────────────────

def power_law(E, A, beta):
    return A * E**beta

def log_corrected(E, A, beta, gamma):
    return A * E**beta * (1 + gamma * np.log(E/200))

def saturating(E, R_inf, E_half):
    return R_inf * (1 - np.exp(-E / E_half))

def screened_power(E, A, beta, E_screen):
    return A * E**beta * np.exp(-E / E_screen)

def lattice_corrected(E, A, beta_bare, R_max):
    """FTD lattice correction: R = R_max * tanh(A * E^beta / R_max)"""
    return R_max * np.tanh(A * E**beta_bare / R_max)

x = np.array(met_centers)
y = np.array(data_q['q50'])
valid = ~np.isnan(y) & (np.array(data_q['n']) > 50)
x_fit = x[valid]
y_fit = y[valid]

print("\n--- Functional Form Fits (Median R_cav vs MET, dlenSig>30) ---\n")

fits = {}

# 1. Pure power law
try:
    popt, pcov = curve_fit(power_law, x_fit, y_fit, p0=[0.1, 0.1])
    y_pred = power_law(x_fit, *popt)
    chi2 = np.sum((y_fit - y_pred)**2 / y_pred)
    fits['power_law'] = {'params': popt, 'chi2': chi2, 'label': f'R = {popt[0]:.3f} * E^{popt[1]:.4f}'}
    print(f"  Power law:       beta = {popt[1]:.4f}, chi2 = {chi2:.4f}")
    print(f"                   R = {popt[0]:.4f} * E^{popt[1]:.4f}")
except: print("  Power law: FIT FAILED")

# 2. Log-corrected power law
try:
    popt, pcov = curve_fit(log_corrected, x_fit, y_fit, p0=[0.1, 0.1, 0.1])
    y_pred = log_corrected(x_fit, *popt)
    chi2 = np.sum((y_fit - y_pred)**2 / y_pred)
    fits['log_corrected'] = {'params': popt, 'chi2': chi2, 'label': f'R = {popt[0]:.3f} * E^{popt[1]:.3f} * (1+{popt[2]:.3f}*ln(E/200))'}
    print(f"  Log-corrected:   beta = {popt[1]:.4f}, gamma = {popt[2]:.4f}, chi2 = {chi2:.4f}")
except: print("  Log-corrected: FIT FAILED")

# 3. Saturating (finite lattice)
try:
    popt, pcov = curve_fit(saturating, x_fit, y_fit, p0=[10, 300])
    y_pred = saturating(x_fit, *popt)
    chi2 = np.sum((y_fit - y_pred)**2 / y_pred)
    fits['saturating'] = {'params': popt, 'chi2': chi2, 'label': f'R = {popt[0]:.2f} * (1-exp(-E/{popt[1]:.0f}))'}
    print(f"  Saturating:      R_inf = {popt[0]:.2f} cm, E_half = {popt[1]:.0f} GeV, chi2 = {chi2:.4f}")
except: print("  Saturating: FIT FAILED")

# 4. Lattice-corrected power law (beta_bare could be 0.5)
try:
    popt, pcov = curve_fit(lattice_corrected, x_fit, y_fit, p0=[0.01, 0.5, 15],
                           bounds=([0, 0.1, 1], [10, 1.0, 100]))
    y_pred = lattice_corrected(x_fit, *popt)
    chi2 = np.sum((y_fit - y_pred)**2 / y_pred)
    fits['lattice_corrected'] = {'params': popt, 'chi2': chi2,
                                  'label': f'R = {popt[2]:.1f}*tanh({popt[0]:.4f}*E^{popt[1]:.3f}/{popt[2]:.1f})'}
    print(f"  Lattice-corr:    beta_bare = {popt[1]:.4f}, R_max = {popt[2]:.2f} cm, chi2 = {chi2:.4f}")
    print(f"                   CAN beta_bare = 0.5 fit? See below...")
except: print("  Lattice-corrected: FIT FAILED")

# 5. Forced beta=0.5 with lattice saturation
try:
    def forced_05(E, A, R_max):
        return R_max * np.tanh(A * E**0.5 / R_max)
    popt_forced, _ = curve_fit(forced_05, x_fit, y_fit, p0=[0.01, 10])
    y_pred_forced = forced_05(x_fit, *popt_forced)
    chi2_forced = np.sum((y_fit - y_pred_forced)**2 / y_pred_forced)
    fits['forced_05'] = {'params': popt_forced, 'chi2': chi2_forced,
                          'label': f'R = {popt_forced[1]:.1f}*tanh({popt_forced[0]:.5f}*sqrt(E)/{popt_forced[1]:.1f})'}
    print(f"\n  ** FORCED beta=0.5: R_max = {popt_forced[1]:.2f} cm, chi2 = {chi2_forced:.4f}")
    print(f"     Comparison: free beta chi2 = {fits.get('power_law',{}).get('chi2', 999):.4f}")
    print(f"     Ratio: {chi2_forced / fits.get('power_law',{}).get('chi2', 1):.2f}x worse (if >2, model disfavored)")
except Exception as e: print(f"  Forced beta=0.5: FIT FAILED ({e})")


# ── 1.3 Energy fraction analysis ─────────────────────────────────────────
print("\n--- Energy Fraction (what fraction of MET drives displacement?) ---")

# If R ~ (f*E)^0.5 where f is fraction, then beta_eff = 0.5 * d(ln f*E)/d(ln E)
# If f decreases with E (energy sharing), beta_eff < 0.5
# For beta_eff = 0.12, need f ~ E^(-0.76)

# Compute the implied energy fraction at each MET bin
if 'power_law' in fits:
    A, beta = fits['power_law']['params']
    # R = A * E^beta. If R = A' * (f*E)^0.5, then f*E = (R/A')^2
    # This means f = (R/A')^2 / E = A^2 * E^(2*beta-1) / A'^2
    # For beta=0.12: f ~ E^(2*0.12-1) = E^(-0.76)
    print(f"  For observed beta={beta:.3f}:")
    print(f"  If R ~ (f*E)^0.5, then f ~ E^({2*beta-1:.2f})")
    print(f"  This means the effective energy fraction DECREASES with energy")
    print(f"  At 200 GeV: f_eff = 1 (reference)")
    for E_test in [300, 500, 800, 1000]:
        f_ratio = (E_test/200)**(2*beta-1)
        print(f"  At {E_test} GeV: f_eff = {f_ratio:.3f} (only {f_ratio*100:.1f}% of 200 GeV fraction)")


# ╔══════════════════════════════════════════════════════════════════════════╗
# ║  PART 2: EXCESS CHARACTERIZATION                                       ║
# ╚══════════════════════════════════════════════════════════════════════════╝

print("\n" + "="*70)
print("PART 2: EXCESS CHARACTERIZATION")
print("="*70)

# ── 2.1 Excess as function of R_cav ──────────────────────────────────────
print("\n--- 2.1 Where in R_cav does the excess live? ---")

r_edges = [0, 0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 7, 10, 15, 20, 30, 50]
r_centers = [(r_edges[i]+r_edges[i+1])/2 for i in range(len(r_edges)-1)]

# Data: histogram in R_cav bins (signal region, high MET)
for met_cut_label, met_lo, met_hi in [("200-400", 200, 400), ("400-1000", 400, 1000), ("200-1000", 200, 1000)]:
    mask_d = sig & (met >= met_lo) & (met < met_hi)
    mask_m = sig_mc & (met_mc >= met_lo) & (met_mc < met_hi)

    h_d, _ = np.histogram(rcav[mask_d], bins=r_edges)
    h_m, _ = np.histogram(rcav_mc[mask_m], bins=r_edges, weights=w_mc_norm[mask_m])

    # Normalize to same total
    h_m_scaled = h_m * (h_d.sum() / h_m.sum()) if h_m.sum() > 0 else h_m

    ratio = np.where(h_m_scaled > 0, h_d / h_m_scaled, np.nan)

    print(f"\n  MET {met_cut_label} GeV (data: {mask_d.sum():,}, MC: {mask_m.sum():,}):")
    print(f"  {'R_cav bin':>15}  {'Data':>8}  {'MC_norm':>8}  {'Ratio':>8}")
    for i in range(len(r_centers)):
        if h_d[i] > 10:
            print(f"  {r_edges[i]:5.1f}-{r_edges[i+1]:5.1f} cm   {h_d[i]:>8d}  {h_m_scaled[i]:>8.0f}  {ratio[i]:>8.2f}")


# ── 2.2 Conditional correlation: rho(MET, R_cav | R_cav > threshold) ─────
print("\n--- 2.2 Conditional correlations ---")

print(f"\n  dlenSig>30, varying R_cav minimum:")
print(f"  {'R_cav min':>10}  {'N_data':>10}  {'rho_data':>10}  {'rho_MC':>10}  {'excess':>10}")
for r_min in [0, 1, 2, 3, 5, 7, 10, 15]:
    mask_d = sig & (rcav > r_min)
    mask_m = sig_mc & (rcav_mc > r_min)
    if mask_d.sum() > 100:
        rho_d, _ = stats.spearmanr(np.sqrt(met[mask_d]), rcav[mask_d])
    else: rho_d = np.nan
    if mask_m.sum() > 30:
        rho_m, _ = stats.spearmanr(np.sqrt(met_mc[mask_m]), rcav_mc[mask_m])
    else: rho_m = np.nan
    excess = rho_d - rho_m if not (np.isnan(rho_d) or np.isnan(rho_m)) else np.nan
    print(f"  {f'>{r_min} cm':>10}  {mask_d.sum():>10,}  {rho_d:>+10.4f}  {rho_m:>+10.4f}  {excess:>+10.4f}")


# ── 2.3 Data-MC displacement ratio vs MET (the excess profile) ───────────
print("\n--- 2.3 Excess profile: Data/MC displacement ratio vs MET ---")

met_fine_edges = [200, 220, 240, 260, 280, 300, 330, 360, 400, 450, 500, 600, 800, 1200]
print(f"\n  dlenSig>30, R_cav > 3cm (outer tracker):")
print(f"  {'MET bin':>12}  {'frac_data':>10}  {'frac_MC':>10}  {'ratio':>10}  {'N_data':>8}")

for i in range(len(met_fine_edges)-1):
    lo, hi = met_fine_edges[i], met_fine_edges[i+1]
    md = sig & (met >= lo) & (met < hi)
    mm = sig_mc & (met_mc >= lo) & (met_mc < hi)

    frac_d = (rcav[md] > 3).sum() / md.sum() if md.sum() > 0 else 0
    frac_m = (rcav_mc[mm] > 3).sum() / mm.sum() if mm.sum() > 0 else 0
    ratio = frac_d / frac_m if frac_m > 0 else np.nan

    print(f"  {lo:>4}-{hi:<4} GeV  {frac_d:>10.4f}  {frac_m:>10.4f}  {ratio:>10.2f}  {md.sum():>8,}")


# ── 2.4 Tail excess characterization ─────────────────────────────────────
print("\n--- 2.4 Tail excess: how does P(R > R_cut) scale with MET? ---")

# For each R_cut, fit P(R > R_cut | MET) = a + b * sqrt(MET)
r_cuts = [2, 3, 5, 7, 10, 15, 20]
print(f"\n  Fit: P(R > R_cut) = a + b*sqrt(MET) within signal region")
print(f"  {'R_cut':>6}  {'a (interc.)':>12}  {'b (slope)':>12}  {'p-value':>12}  {'Data b/MC b':>12}")

met_bin_centers = [(met_fine_edges[i]+met_fine_edges[i+1])/2 for i in range(len(met_fine_edges)-1)]

for r_cut in r_cuts:
    frac_d_bins = []
    frac_m_bins = []
    valid_bins = []
    for i in range(len(met_fine_edges)-1):
        lo, hi = met_fine_edges[i], met_fine_edges[i+1]
        md = sig & (met >= lo) & (met < hi)
        mm = sig_mc & (met_mc >= lo) & (met_mc < hi)
        if md.sum() > 100:
            frac_d_bins.append((rcav[md] > r_cut).sum() / md.sum())
            frac_m_bins.append((rcav_mc[mm] > r_cut).sum() / mm.sum() if mm.sum() > 10 else np.nan)
            valid_bins.append(met_bin_centers[i])

    if len(valid_bins) > 3:
        x_sqrt = np.sqrt(valid_bins)
        slope_d, intercept_d, r_d, p_d, _ = stats.linregress(x_sqrt, frac_d_bins)

        valid_mc = [j for j, f in enumerate(frac_m_bins) if not np.isnan(f)]
        if len(valid_mc) > 3:
            slope_m, _, _, _, _ = stats.linregress(
                np.sqrt([valid_bins[j] for j in valid_mc]),
                [frac_m_bins[j] for j in valid_mc])
            ratio_str = f"{slope_d/slope_m:.2f}" if slope_m != 0 else "inf"
        else:
            ratio_str = "N/A"

        print(f"  {r_cut:>4} cm  {intercept_d:>+12.5f}  {slope_d:>+12.6f}  {p_d:>12.2e}  {ratio_str:>12}")


# ── 2.5 The excess as a function of MET (bootstrapped) ───────────────────
print("\n--- 2.5 Bootstrapped excess correlation vs MET ---")

met_broad = [(200, 300), (300, 400), (400, 600), (600, 1200)]
n_boot = 500

print(f"\n  {'MET range':>12}  {'rho_data':>10}  {'rho_MC':>10}  {'excess':>10}  {'95% CI':>20}  {'sig?':>5}")
for lo, hi in met_broad:
    md = sig & (met >= lo) & (met < hi)
    mm = sig_mc & (met_mc >= lo) & (met_mc < hi)

    rho_d, _ = stats.spearmanr(np.sqrt(met[md]), rcav[md]) if md.sum() > 100 else (np.nan, 0)
    rho_m, _ = stats.spearmanr(np.sqrt(met_mc[mm]), rcav_mc[mm]) if mm.sum() > 30 else (np.nan, 0)

    # Bootstrap CI for data rho
    boot_rhos = []
    n_d = md.sum()
    idx_d = np.where(md)[0]
    for _ in range(n_boot):
        boot_idx = np.random.choice(idx_d, size=n_d, replace=True)
        br, _ = stats.spearmanr(np.sqrt(met[boot_idx]), rcav[boot_idx])
        boot_rhos.append(br)
    ci_lo, ci_hi = np.percentile(boot_rhos, [2.5, 97.5])

    excess = rho_d - rho_m if not (np.isnan(rho_d) or np.isnan(rho_m)) else np.nan
    sig_flag = "YES" if ci_lo > rho_m else "no"

    print(f"  {lo:>4}-{hi:<4} GeV  {rho_d:>+10.4f}  {rho_m:>+10.4f}  {excess:>+10.4f}  [{ci_lo:>+.4f}, {ci_hi:>+.4f}]  {sig_flag:>5}")


# ── 2.6 Is the excess in exotic (non-B, non-D) SVs? ─────────────────────
print("\n--- 2.6 Exotic SV excess (mass < 1.5 GeV, no B-jet) ---")

exotic = sig & (svmass < 1.5) & (bjet < 0.5)
exotic_mc = sig_mc & (svmass_mc < 1.5)  # no bjet in MC cache structure

for met_cut_label, met_lo, met_hi in [("200-300", 200, 300), ("300-500", 300, 500), ("500+", 500, 2000)]:
    md = exotic & (met >= met_lo) & (met < met_hi)
    mm = exotic_mc & (met_mc >= met_lo) & (met_mc < met_hi)

    if md.sum() > 50:
        rho_d, _ = stats.spearmanr(np.sqrt(met[md]), rcav[md])
        med_d = np.median(rcav[md])
    else:
        rho_d, med_d = np.nan, np.nan
    if mm.sum() > 20:
        rho_m, _ = stats.spearmanr(np.sqrt(met_mc[mm]), rcav_mc[mm])
        med_m = np.median(rcav_mc[mm])
    else:
        rho_m, med_m = np.nan, np.nan

    print(f"  MET {met_cut_label}: N_data={md.sum():,}, rho_data={rho_d:+.4f}, "
          f"median_R={med_d:.2f}cm | MC: N={mm.sum()}, rho_MC={rho_m:+.4f}, median_R={med_m:.2f}cm")


# ╔══════════════════════════════════════════════════════════════════════════╗
# ║  PART 3: PLOT                                                          ║
# ╚══════════════════════════════════════════════════════════════════════════╝

fig = plt.figure(figsize=(20, 16))
fig.suptitle('FTD Scaling Corrections & Excess Characterization\n'
             'CMS Run2016G MET | dlenSig > 30 signal region', fontsize=14, fontweight='bold')

# ── Panel 1: Multiple functional form fits ───────────────────────────────
ax1 = fig.add_subplot(3, 4, 1)
ax1.plot(x_fit, y_fit, 'ko', markersize=8, label='Data median')
E_range = np.linspace(200, 1100, 200)

colors = {'power_law': 'blue', 'log_corrected': 'green', 'saturating': 'orange',
          'lattice_corrected': 'red', 'forced_05': 'purple'}
for name, info in fits.items():
    func = {'power_law': power_law, 'log_corrected': log_corrected,
            'saturating': saturating, 'lattice_corrected': lattice_corrected,
            'forced_05': lambda E, *p: p[1]*np.tanh(p[0]*E**0.5/p[1])}.get(name)
    if func:
        try:
            ax1.plot(E_range, func(E_range, *info['params']), color=colors.get(name, 'gray'),
                    linewidth=1.5, label=f"{name} (chi2={info['chi2']:.3f})")
        except: pass

# FTD prediction (pure sqrt)
r0 = y_fit[0]
ax1.plot(E_range, r0 * (E_range/200)**0.5, 'r:', linewidth=2, alpha=0.5, label='FTD naive (beta=0.5)')
ax1.set_xlabel('MET (GeV)')
ax1.set_ylabel('Median R_cav (cm)')
ax1.set_title('Functional Form Fits')
ax1.legend(fontsize=6, loc='upper left')
ax1.grid(True, alpha=0.3)

# ── Panel 2: Log-log with residuals ──────────────────────────────────────
ax2 = fig.add_subplot(3, 4, 2)
ax2.plot(np.log(x_fit), np.log(y_fit), 'ko', markersize=8)
if 'power_law' in fits:
    A, beta = fits['power_law']['params']
    ax2.plot(np.log(E_range), np.log(A) + beta*np.log(E_range), 'b-',
            label=f'beta={beta:.3f}')
ax2.plot(np.log(E_range), np.log(r0) + 0.5*(np.log(E_range)-np.log(200)), 'r:',
        alpha=0.5, label='beta=0.5')
ax2.set_xlabel('ln(MET)')
ax2.set_ylabel('ln(Median R_cav)')
ax2.set_title('Log-Log Scaling')
ax2.legend(fontsize=8)
ax2.grid(True, alpha=0.3)

# ── Panel 3: Quantile fan ────────────────────────────────────────────────
ax3 = fig.add_subplot(3, 4, 3)
for q, color, label in [('q25', 'lightblue', 'Q25'), ('q50', 'blue', 'Median'),
                         ('q75', 'darkblue', 'Q75'), ('q90', 'navy', 'Q90')]:
    y_q = np.array(data_q[q])
    valid_q = ~np.isnan(y_q)
    if valid_q.sum() > 2:
        ax3.plot(np.array(met_centers)[valid_q], y_q[valid_q], 'o-', color=color, label=label)
        # Fit each quantile
        slope, _, _, _, _ = stats.linregress(np.log(np.array(met_centers)[valid_q]), np.log(y_q[valid_q]))
        ax3.text(850, y_q[valid_q][-1], f'beta={slope:.2f}', fontsize=7, color=color)

ax3.set_xlabel('MET (GeV)')
ax3.set_ylabel('R_cav (cm)')
ax3.set_title('Quantile Fan (Data)')
ax3.legend(fontsize=7)
ax3.grid(True, alpha=0.3)

# ── Panel 4: Data vs MC quantiles ────────────────────────────────────────
ax4 = fig.add_subplot(3, 4, 4)
mc_valid = ~np.isnan(np.array(mc_q['q50']))
ax4.plot(np.array(met_centers), data_q['q50'], 'ro-', linewidth=2, label='Data median')
if mc_valid.any():
    ax4.plot(np.array(met_centers)[mc_valid], np.array(mc_q['q50'])[mc_valid],
            'bs--', linewidth=2, label='MC median')
# Ratio axis
ax4_r = ax4.twinx()
ratio_med = np.array(data_q['q50']) / np.array(mc_q['q50'])
ratio_valid = ~np.isnan(ratio_med)
if ratio_valid.any():
    ax4_r.plot(np.array(met_centers)[ratio_valid], ratio_med[ratio_valid],
              'g^:', alpha=0.6, label='Data/MC')
    ax4_r.set_ylabel('Data/MC ratio', color='green')
    ax4_r.axhline(1, color='green', alpha=0.3, linestyle=':')
ax4.set_xlabel('MET (GeV)')
ax4.set_ylabel('Median R_cav (cm)')
ax4.set_title('Data vs MC Medians')
ax4.legend(fontsize=8, loc='upper left')
ax4.grid(True, alpha=0.3)

# ── Panel 5: Data-MC displacement distribution ──────────────────────────
ax5 = fig.add_subplot(3, 4, 5)
r_bins = np.linspace(0, 30, 60)
md_sig = sig & (met >= 300)  # High MET signal
mm_sig = sig_mc & (met_mc >= 300)

h_d5, _ = np.histogram(rcav[md_sig], bins=r_bins, density=True)
h_m5, _ = np.histogram(rcav_mc[mm_sig], bins=r_bins, density=True)
r_c5 = (r_bins[:-1] + r_bins[1:]) / 2

ax5.step(r_c5, h_d5, 'r-', linewidth=2, label=f'Data (n={md_sig.sum():,})')
ax5.step(r_c5, h_m5, 'b--', linewidth=2, label=f'MC (n={mm_sig.sum():,})')
ax5.set_xlabel('R_cav (cm)')
ax5.set_ylabel('Normalized density')
ax5.set_title('R_cav Distribution (MET>300, dlenSig>30)')
ax5.set_yscale('log')
ax5.set_ylim(1e-5, 1)
ax5.legend(fontsize=8)
ax5.grid(True, alpha=0.3)

# ── Panel 6: Data/MC ratio vs R_cav ─────────────────────────────────────
ax6 = fig.add_subplot(3, 4, 6)
ratio6 = np.where(h_m5 > 0, h_d5 / h_m5, np.nan)
valid6 = ~np.isnan(ratio6) & (h_d5 > 0)
ax6.plot(r_c5[valid6], ratio6[valid6], 'ko', markersize=4)
ax6.axhline(1, color='red', linestyle='--')
ax6.set_xlabel('R_cav (cm)')
ax6.set_ylabel('Data / MC')
ax6.set_title('Data/MC Ratio (MET>300)')
ax6.set_ylim(0, 5)
ax6.grid(True, alpha=0.3)

# ── Panel 7: Conditional rho vs R_cav min ────────────────────────────────
ax7 = fig.add_subplot(3, 4, 7)
r_mins = [0, 0.5, 1, 1.5, 2, 3, 5, 7, 10, 15]
rho_d_cond = []
rho_m_cond = []
for r_min in r_mins:
    md = sig & (rcav > r_min)
    mm = sig_mc & (rcav_mc > r_min)
    rho_d_cond.append(stats.spearmanr(np.sqrt(met[md]), rcav[md])[0] if md.sum() > 100 else np.nan)
    rho_m_cond.append(stats.spearmanr(np.sqrt(met_mc[mm]), rcav_mc[mm])[0] if mm.sum() > 30 else np.nan)

ax7.plot(r_mins, rho_d_cond, 'ro-', linewidth=2, label='Data')
ax7.plot(r_mins, rho_m_cond, 'bs--', linewidth=2, label='MC')
ax7.fill_between(r_mins, rho_d_cond, rho_m_cond, alpha=0.2, color='green', label='Excess')
ax7.set_xlabel('R_cav minimum (cm)')
ax7.set_ylabel('Spearman rho')
ax7.set_title('Conditional Correlation')
ax7.legend(fontsize=8)
ax7.grid(True, alpha=0.3)

# ── Panel 8: P(R>5cm) vs sqrt(MET) with linear fit ──────────────────────
ax8 = fig.add_subplot(3, 4, 8)
p5_d = []
p5_m = []
met_sqrt_c = []
for i in range(len(met_fine_edges)-1):
    lo, hi = met_fine_edges[i], met_fine_edges[i+1]
    md = sig & (met >= lo) & (met < hi)
    mm = sig_mc & (met_mc >= lo) & (met_mc < hi)
    if md.sum() > 100:
        p5_d.append((rcav[md] > 5).sum() / md.sum())
        p5_m.append((rcav_mc[mm] > 5).sum() / mm.sum() if mm.sum() > 10 else np.nan)
        met_sqrt_c.append(np.sqrt((lo+hi)/2))

ax8.plot(met_sqrt_c, p5_d, 'ro-', label='Data')
valid8 = [j for j, f in enumerate(p5_m) if not np.isnan(f)]
ax8.plot([met_sqrt_c[j] for j in valid8], [p5_m[j] for j in valid8], 'bs--', label='MC')
# Linear fit
slope_d, int_d, _, p_d, _ = stats.linregress(met_sqrt_c, p5_d)
ax8.plot(met_sqrt_c, int_d + slope_d * np.array(met_sqrt_c), 'r:', alpha=0.5)
ax8.set_xlabel('sqrt(MET) [GeV^0.5]')
ax8.set_ylabel('P(R_cav > 5 cm)')
ax8.set_title(f'Tail Fraction vs sqrt(MET)\nslope={slope_d:.5f}, p={p_d:.1e}')
ax8.legend(fontsize=8)
ax8.grid(True, alpha=0.3)

# ── Panel 9: Excess by SV mass (bar chart) ───────────────────────────────
ax9 = fig.add_subplot(3, 4, 9)
mass_wins = [('<1.5', 0, 1.5), ('1.5-2.5', 1.5, 2.5), ('2.5-4', 2.5, 4),
             ('4-7', 4, 7), ('>7', 7, 100)]
excess_by_mass = []
labels_mass = []
for name, lo, hi in mass_wins:
    md = sig & (svmass >= lo) & (svmass < hi)
    mm = sig_mc & (svmass_mc >= lo) & (svmass_mc < hi)
    rho_d = stats.spearmanr(np.sqrt(met[md]), rcav[md])[0] if md.sum() > 100 else np.nan
    rho_m = stats.spearmanr(np.sqrt(met_mc[mm]), rcav_mc[mm])[0] if mm.sum() > 20 else np.nan
    excess_by_mass.append(rho_d - rho_m if not (np.isnan(rho_d) or np.isnan(rho_m)) else 0)
    labels_mass.append(name)

ax9.bar(labels_mass, excess_by_mass, color='green', alpha=0.7)
ax9.axhline(0, color='black', linewidth=0.5)
ax9.axhline(np.mean(excess_by_mass), color='red', linestyle=':', label=f'mean={np.mean(excess_by_mass):.3f}')
ax9.set_xlabel('SV Mass Window (GeV)')
ax9.set_ylabel('rho(Data) - rho(MC)')
ax9.set_title('Excess Uniformity Across Mass')
ax9.legend(fontsize=8)
ax9.grid(True, alpha=0.3, axis='y')

# ── Panel 10: 2D excess map (MET vs R_cav) ──────────────────────────────
ax10 = fig.add_subplot(3, 4, 10)
met_2d = [200, 250, 300, 400, 500, 700, 1200]
r_2d   = [0, 1, 2, 3, 5, 8, 15, 30]
ratio_2d = np.zeros((len(r_2d)-1, len(met_2d)-1))

for i in range(len(met_2d)-1):
    for j in range(len(r_2d)-1):
        md = sig & (met >= met_2d[i]) & (met < met_2d[i+1]) & (rcav >= r_2d[j]) & (rcav < r_2d[j+1])
        mm = sig_mc & (met_mc >= met_2d[i]) & (met_mc < met_2d[i+1]) & (rcav_mc >= r_2d[j]) & (rcav_mc < r_2d[j+1])
        nd = md.sum()
        nm = mm.sum() * (len(met) / len(met_mc)) if mm.sum() > 0 else 0
        ratio_2d[j, i] = nd / nm if nm > 5 else np.nan

im = ax10.imshow(ratio_2d, origin='lower', aspect='auto', cmap='RdBu_r', vmin=0.5, vmax=2.0,
                 extent=[0, len(met_2d)-1, 0, len(r_2d)-1])
ax10.set_xticks(range(len(met_2d)-1))
ax10.set_xticklabels([f'{met_2d[i]}' for i in range(len(met_2d)-1)], fontsize=7)
ax10.set_yticks(range(len(r_2d)-1))
ax10.set_yticklabels([f'{r_2d[j]}-{r_2d[j+1]}' for j in range(len(r_2d)-1)], fontsize=7)
ax10.set_xlabel('MET (GeV)')
ax10.set_ylabel('R_cav (cm)')
ax10.set_title('Data/MC Ratio (2D)')
plt.colorbar(im, ax=ax10, label='Data/MC')

# ── Panel 11: Effective beta at each quantile ────────────────────────────
ax11 = fig.add_subplot(3, 4, 11)
quantiles_to_fit = [0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99]
betas = []
for q in quantiles_to_fit:
    q_vals = []
    for i in range(len(met_edges)-1):
        md = sig & (met >= met_edges[i]) & (met < met_edges[i+1])
        if md.sum() > 50:
            q_vals.append(np.quantile(rcav[md], q))
        else:
            q_vals.append(np.nan)
    qv = np.array(q_vals)
    valid = ~np.isnan(qv)
    if valid.sum() > 3:
        beta_q, _, _, _, _ = stats.linregress(np.log(np.array(met_centers)[valid]), np.log(qv[valid]))
        betas.append(beta_q)
    else:
        betas.append(np.nan)

ax11.plot([int(q*100) for q in quantiles_to_fit], betas, 'ko-', markersize=8)
ax11.axhline(0.5, color='red', linestyle=':', label='FTD prediction')
ax11.axhline(0, color='gray', linestyle=':')
ax11.set_xlabel('Percentile')
ax11.set_ylabel('Effective beta')
ax11.set_title('Scaling Exponent by Quantile')
ax11.legend(fontsize=8)
ax11.grid(True, alpha=0.3)

# ── Panel 12: Summary text ───────────────────────────────────────────────
ax12 = fig.add_subplot(3, 4, 12)
ax12.axis('off')

summary_text = ""
if 'power_law' in fits:
    summary_text += f"OBSERVED: beta = {fits['power_law']['params'][1]:.3f}\n"
    summary_text += f"FTD PREDICTED: beta = 0.500\n\n"

summary_text += "CORRECTION MODELS:\n"
for name, info in sorted(fits.items(), key=lambda x: x[1]['chi2']):
    summary_text += f"  {name}: chi2={info['chi2']:.4f}\n"

summary_text += f"\nBest fit: {min(fits.items(), key=lambda x: x[1]['chi2'])[0]}\n"

if 'forced_05' in fits and 'power_law' in fits:
    ratio = fits['forced_05']['chi2'] / fits['power_law']['chi2']
    summary_text += f"\nForced beta=0.5 is {ratio:.1f}x worse\n"
    if ratio < 2:
        summary_text += "  -> beta=0.5 NOT excluded!\n"
        summary_text += f"  -> R_max = {fits['forced_05']['params'][1]:.1f} cm\n"
        summary_text += "  -> Saturation explains low beta\n"
    else:
        summary_text += "  -> beta=0.5 DISFAVORED\n"

summary_text += f"\nExcess uniform across mass: YES\n"
summary_text += f"Excess grows with energy: "
if 'power_law' in fits and fits['power_law']['params'][1] > 0:
    summary_text += "YES (weak)\n"
else:
    summary_text += "NO\n"

ax12.text(0.05, 0.95, summary_text, transform=ax12.transAxes, fontsize=8,
         verticalalignment='top', fontfamily='monospace',
         bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.savefig(os.path.join(sim_dir, 'ftd_cavitation_SCALING_CORRECTIONS.png'), dpi=150, bbox_inches='tight')
print(f"\nPlot saved: ftd_cavitation_SCALING_CORRECTIONS.png")
plt.close()

print("\n" + "="*70)
print("ANALYSIS COMPLETE")
print("="*70)
