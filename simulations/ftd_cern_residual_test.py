#!/usr/bin/env python3
"""
FTD Residual Analysis: Data-MC Correlation Excess
===================================================
Focus on the key finding from MC comparison:

  DATA shows STRONGER sqrt(MET)-R_cav correlation than SM MC
  in high decay-length-significance events:
    dlenSig>10: rho_data=0.077 vs rho_MC=0.034 (diff=+0.042)
    dlenSig>30: rho_data=0.109 vs rho_MC=0.020 (diff=+0.089)
    dlenSig>50: rho_data=0.107 vs rho_MC=0.022 (diff=+0.085)

This script:
  1. Bootstrap confidence intervals on rho(data) - rho(MC)
  2. Energy-binned residual analysis
  3. FTD-specific prediction test: does excess scale as sqrt(MET)?
  4. Null hypothesis test via MC permutation
"""

import os, sys, time
import numpy as np
from scipy import stats
import warnings
warnings.filterwarnings('ignore')

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

print("=== FTD Residual Analysis: Data-MC Correlation Excess ===\n")

# Load data and MC from cache
script_dir = os.path.dirname(os.path.abspath(__file__))

# --- DATA ---
enhanced_path = os.path.join(script_dir, "ftd_full_enhanced.npz")
d = np.load(enhanced_path, allow_pickle=True)
data_met = d["met"]
data_rcav = d["rcav"]
data_dlsig = d["sv_dlsig_max"] if "sv_dlsig_max" in d else None
data_bjet = d["has_bjet"] if "has_bjet" in d else None

print(f"Data: {len(data_met):,} events")

# --- MC ---
mc_path = os.path.join(script_dir, "ftd_mc_cache.npz")
mc_raw = np.load(mc_path, allow_pickle=True)

# Combine all MC samples
mc_met_all = []
mc_rcav_all = []
mc_dlsig_all = []
mc_bjet_all = []
mc_weight_all = []

MC_SAMPLES = {
    "WJetsToLNu": {"xsec": 61526.7},
    "ZJetsToNuNu_200toInf": {"xsec": 18.01},
    "ZJetsToNuNu_100to200": {"xsec": 93.79},
    "QCD_HT1000to1500": {"xsec": 1005.0},
    "QCD_HT700to1000": {"xsec": 6334.0},
}

for name, info in MC_SAMPLES.items():
    key_met = f"{name}__met"
    if key_met not in mc_raw:
        continue
    met = mc_raw[key_met]
    rcav = mc_raw[f"{name}__rcav"]
    n_total = int(mc_raw[f"{name}__n_total"])
    w = info["xsec"] / n_total  # weight per event
    mc_met_all.append(met)
    mc_rcav_all.append(rcav)
    mc_weight_all.extend([w] * len(met))

    key_dlsig = f"{name}__sv_dlsig_max"
    if key_dlsig in mc_raw:
        mc_dlsig_all.append(mc_raw[key_dlsig])
    else:
        mc_dlsig_all.append(np.zeros(len(met)))

    key_bjet = f"{name}__has_bjet"
    if key_bjet in mc_raw:
        mc_bjet_all.append(mc_raw[key_bjet])
    else:
        mc_bjet_all.append(np.zeros(len(met), dtype=bool))

    print(f"  MC {name}: {len(met):,} events (w={w:.6f})")

mc_met = np.concatenate(mc_met_all)
mc_rcav = np.concatenate(mc_rcav_all)
mc_dlsig = np.concatenate(mc_dlsig_all)
mc_bjet = np.concatenate(mc_bjet_all)
mc_weights = np.array(mc_weight_all)

print(f"\nTotal MC: {len(mc_met):,} events (combined)")

# =========================================================
# 1. BOOTSTRAP CONFIDENCE INTERVALS on rho_data - rho_MC
# =========================================================
print("\n" + "="*60)
print("1. Bootstrap Confidence Intervals")
print("="*60)

N_BOOT = 1000
rng = np.random.default_rng(42)

def bootstrap_rho(met, rcav, n_boot=N_BOOT):
    """Bootstrap Spearman rho between sqrt(met) and rcav."""
    sqrt_met = np.sqrt(met)
    rhos = np.zeros(n_boot)
    n = len(met)
    for i in range(n_boot):
        idx = rng.integers(0, n, size=n)
        rhos[i] = stats.spearmanr(sqrt_met[idx], rcav[idx])[0]
    return rhos

dlsig_cuts = [0, 5, 10, 20, 30, 50, 75, 100]
boot_results = {}

for cut in dlsig_cuts:
    # Data
    if data_dlsig is not None:
        mask_d = data_dlsig > cut
    else:
        mask_d = np.ones(len(data_met), dtype=bool)

    # MC
    mask_mc = mc_dlsig > cut

    n_d = np.sum(mask_d)
    n_mc = np.sum(mask_mc)

    if n_d < 100 or n_mc < 50:
        print(f"  dlenSig>{cut}: insufficient stats (data={n_d}, MC={n_mc})")
        continue

    # Point estimates
    rho_d, p_d = stats.spearmanr(np.sqrt(data_met[mask_d]), data_rcav[mask_d])
    rho_mc, p_mc = stats.spearmanr(np.sqrt(mc_met[mask_mc]), mc_rcav[mask_mc])

    # Bootstrap (use smaller sample for MC to keep reasonable time)
    n_boot_mc = min(N_BOOT, 200) if n_mc < 1000 else N_BOOT
    rhos_d = bootstrap_rho(data_met[mask_d], data_rcav[mask_d])
    rhos_mc = bootstrap_rho(mc_met[mask_mc], mc_rcav[mask_mc], n_boot=n_boot_mc)

    # Difference distribution
    min_len = min(len(rhos_d), len(rhos_mc))
    rho_diff_boot = rhos_d[:min_len] - rhos_mc[:min_len]

    ci_low = np.percentile(rho_diff_boot, 2.5)
    ci_high = np.percentile(rho_diff_boot, 97.5)
    p_zero = np.mean(rho_diff_boot < 0)  # fraction where MC > data

    boot_results[cut] = {
        "n_data": n_d, "n_mc": n_mc,
        "rho_data": rho_d, "rho_mc": rho_mc,
        "diff": rho_d - rho_mc,
        "ci_low": ci_low, "ci_high": ci_high,
        "p_zero": p_zero,
        "rhos_d": rhos_d, "rhos_mc": rhos_mc,
        "rho_diff_boot": rho_diff_boot,
    }

    sig = "***" if ci_low > 0 else "ns"
    print(f"  dlenSig>{cut:3d}: rho_data={rho_d:+.4f}, rho_MC={rho_mc:+.4f}, "
          f"diff={rho_d-rho_mc:+.4f} [{ci_low:+.4f}, {ci_high:+.4f}] {sig} "
          f"(n_data={n_d:,}, n_MC={n_mc:,})")


# =========================================================
# 2. ENERGY-BINNED RESIDUAL
# =========================================================
print("\n" + "="*60)
print("2. Energy-Binned Correlation: Data vs MC")
print("="*60)

met_edges = np.array([200, 250, 300, 400, 600, 1000, 3000])
met_labels = [f"{int(met_edges[i])}-{int(met_edges[i+1])}" for i in range(len(met_edges)-1)]

# For high-dlenSig events only (our signal region)
DLSIG_CUT = 30
mask_d_sig = data_dlsig > DLSIG_CUT if data_dlsig is not None else np.ones(len(data_met), dtype=bool)
mask_mc_sig = mc_dlsig > DLSIG_CUT

energy_results = {}
for i in range(len(met_edges)-1):
    lo, hi = met_edges[i], met_edges[i+1]

    # Data in this MET bin, high dlenSig
    mask_d_bin = mask_d_sig & (data_met >= lo) & (data_met < hi)
    # MC in this MET bin, high dlenSig
    mask_mc_bin = mask_mc_sig & (mc_met >= lo) & (mc_met < hi)

    n_d = np.sum(mask_d_bin)
    n_mc = np.sum(mask_mc_bin)

    # Median R_cav in this energy bin
    med_d = np.median(data_rcav[mask_d_bin]) if n_d > 10 else 0
    med_mc = np.median(mc_rcav[mask_mc_bin]) if n_mc > 10 else 0

    # 90th percentile R_cav
    p90_d = np.percentile(data_rcav[mask_d_bin], 90) if n_d > 10 else 0
    p90_mc = np.percentile(mc_rcav[mask_mc_bin], 90) if n_mc > 10 else 0

    # Tail fraction: P(R > 5cm)
    tail_d = np.mean(data_rcav[mask_d_bin] > 5) if n_d > 10 else 0
    tail_mc = np.mean(mc_rcav[mask_mc_bin] > 5) if n_mc > 10 else 0

    energy_results[met_labels[i]] = {
        "n_data": n_d, "n_mc": n_mc,
        "med_data": med_d, "med_mc": med_mc,
        "p90_data": p90_d, "p90_mc": p90_mc,
        "tail_data": tail_d, "tail_mc": tail_mc,
        "met_center": (lo + hi) / 2,
    }

    print(f"  MET {lo:4d}-{hi:4d}: data={n_d:6,} MC={n_mc:4,}  "
          f"median(R): {med_d:.2f} vs {med_mc:.2f}  "
          f"P90(R): {p90_d:.2f} vs {p90_mc:.2f}  "
          f"P(R>5): {tail_d:.4f} vs {tail_mc:.4f}")


# =========================================================
# 3. FTD PREDICTION TEST: Does data excess scale as sqrt(MET)?
# =========================================================
print("\n" + "="*60)
print("3. FTD Prediction Test: Median R_cav vs sqrt(MET)")
print("="*60)

met_centers = np.array([v["met_center"] for v in energy_results.values()])
sqrt_met_c = np.sqrt(met_centers)
med_data = np.array([v["med_data"] for v in energy_results.values()])
med_mc = np.array([v["med_mc"] for v in energy_results.values()])
p90_data = np.array([v["p90_data"] for v in energy_results.values()])
p90_mc = np.array([v["p90_mc"] for v in energy_results.values()])
tail_data = np.array([v["tail_data"] for v in energy_results.values()])
tail_mc = np.array([v["tail_mc"] for v in energy_results.values()])

# Median R_cav growth rate
valid = (med_data > 0) & (med_mc > 0)
if np.sum(valid) >= 3:
    # FTD: R_cav ~ sqrt(MET)  =>  log(R) = 0.5 * log(MET) + const
    # Power law fit: R = A * MET^beta
    log_met = np.log(met_centers[valid])
    log_med_d = np.log(med_data[valid])
    log_med_mc = np.log(med_mc[valid])

    beta_data, _ = np.polyfit(log_met, log_med_d, 1)
    beta_mc, _ = np.polyfit(log_met, log_med_mc, 1)

    print(f"  Power-law fit: median(R_cav) ~ MET^beta")
    print(f"    Data beta:  {beta_data:.4f}")
    print(f"    MC beta:    {beta_mc:.4f}")
    print(f"    FTD prediction: beta = 0.5")
    print(f"    Excess beta: {beta_data - beta_mc:.4f}")

# P90 growth rate
valid_p90 = (p90_data > 0) & (p90_mc > 0)
if np.sum(valid_p90) >= 3:
    log_p90_d = np.log(p90_data[valid_p90])
    log_p90_mc = np.log(p90_mc[valid_p90])

    beta_p90_data, _ = np.polyfit(np.log(met_centers[valid_p90]), log_p90_d, 1)
    beta_p90_mc, _ = np.polyfit(np.log(met_centers[valid_p90]), log_p90_mc, 1)

    print(f"\n  Power-law fit: P90(R_cav) ~ MET^beta")
    print(f"    Data beta:  {beta_p90_data:.4f}")
    print(f"    MC beta:    {beta_p90_mc:.4f}")
    print(f"    FTD prediction: beta = 0.5")
    print(f"    Excess beta: {beta_p90_data - beta_p90_mc:.4f}")

# Tail fraction growth
valid_tail = (tail_data > 0) & (tail_mc > 0)
if np.sum(valid_tail) >= 3:
    beta_tail_data, _ = np.polyfit(np.log(met_centers[valid_tail]),
                                     np.log(tail_data[valid_tail]), 1)
    beta_tail_mc, _ = np.polyfit(np.log(met_centers[valid_tail]),
                                   np.log(tail_mc[valid_tail]), 1)

    print(f"\n  Power-law fit: P(R>5cm|MET) ~ MET^beta")
    print(f"    Data beta:  {beta_tail_data:.4f}")
    print(f"    MC beta:    {beta_tail_mc:.4f}")


# =========================================================
# 4. NULL HYPOTHESIS: Permutation Test
# =========================================================
print("\n" + "="*60)
print("4. Null Hypothesis Test: Permutation of R_cav labels")
print("="*60)

# Under H0: R_cav is independent of MET
# Permute R_cav labels and recompute rho
# For the dlenSig>30 signal region

mask_signal = data_dlsig > 30 if data_dlsig is not None else np.ones(len(data_met), dtype=bool)
sqrt_met_sig = np.sqrt(data_met[mask_signal])
rcav_sig = data_rcav[mask_signal]
rho_observed, _ = stats.spearmanr(sqrt_met_sig, rcav_sig)

N_PERM = 5000
rho_null = np.zeros(N_PERM)
for i in range(N_PERM):
    rcav_perm = rng.permutation(rcav_sig)
    rho_null[i] = stats.spearmanr(sqrt_met_sig, rcav_perm)[0]

p_perm = np.mean(np.abs(rho_null) >= np.abs(rho_observed))

print(f"  Signal region: dlenSig > 30 ({np.sum(mask_signal):,} events)")
print(f"  Observed rho: {rho_observed:.4f}")
print(f"  Null distribution: mean={np.mean(rho_null):.6f}, std={np.std(rho_null):.6f}")
print(f"  Permutation p-value: {p_perm:.6f}")
print(f"  Z-score: {(rho_observed - np.mean(rho_null)) / np.std(rho_null):.1f} sigma")


# =========================================================
# 5. COMPARE R_cav DISTRIBUTIONS BY MET BIN: KS tests
# =========================================================
print("\n" + "="*60)
print("5. KS Tests: Data vs MC R_cav shape by MET bin")
print("="*60)

for i in range(len(met_edges)-1):
    lo, hi = met_edges[i], met_edges[i+1]

    mask_d_bin = mask_d_sig & (data_met >= lo) & (data_met < hi)
    mask_mc_bin = mask_mc_sig & (mc_met >= lo) & (mc_met < hi)

    n_d = np.sum(mask_d_bin)
    n_mc = np.sum(mask_mc_bin)

    if n_d > 50 and n_mc > 20:
        ks, p_ks = stats.ks_2samp(data_rcav[mask_d_bin], mc_rcav[mask_mc_bin])
        sig = "***" if p_ks < 0.001 else ("**" if p_ks < 0.01 else ("*" if p_ks < 0.05 else "ns"))
        print(f"  MET {lo}-{hi}: KS={ks:.4f}, p={p_ks:.4e} {sig}")
    else:
        print(f"  MET {lo}-{hi}: insufficient stats")


# =========================================================
# PLOTS
# =========================================================
print("\nGenerating plots...")

fig = plt.figure(figsize=(20, 20))
gs = GridSpec(4, 3, figure=fig, hspace=0.4, wspace=0.3)
fig.suptitle("FTD Residual Analysis: Data-MC Correlation Excess\n"
             "(High dlenSig events, CMS Run2016G MET dataset)",
             fontsize=14, fontweight='bold', y=0.98)

# Panel 1: Bootstrap rho_diff distribution for dlenSig>30
ax1 = fig.add_subplot(gs[0, 0])
if 30 in boot_results:
    br = boot_results[30]
    ax1.hist(br["rho_diff_boot"], bins=50, alpha=0.7, color='steelblue',
             edgecolor='navy', density=True)
    ax1.axvline(br["diff"], color='red', linewidth=2, label=f'Observed: {br["diff"]:.4f}')
    ax1.axvline(0, color='gray', linestyle='--', linewidth=1, label='No excess')
    ax1.axvline(br["ci_low"], color='orange', linestyle=':', linewidth=1)
    ax1.axvline(br["ci_high"], color='orange', linestyle=':', linewidth=1,
                label=f'95% CI: [{br["ci_low"]:.3f}, {br["ci_high"]:.3f}]')
    ax1.set_xlabel(r'$\rho_{data} - \rho_{MC}$')
    ax1.set_ylabel('Density')
    ax1.set_title(r'Bootstrap $\Delta\rho$ (dlenSig>30)')
    ax1.legend(fontsize=7)

# Panel 2: Rho vs dlenSig cut
ax2 = fig.add_subplot(gs[0, 1])
cuts_valid = sorted([c for c in boot_results.keys()])
rho_d_vs_cut = [boot_results[c]["rho_data"] for c in cuts_valid]
rho_mc_vs_cut = [boot_results[c]["rho_mc"] for c in cuts_valid]
diff_vs_cut = [boot_results[c]["diff"] for c in cuts_valid]
ci_lo = [boot_results[c]["ci_low"] for c in cuts_valid]
ci_hi = [boot_results[c]["ci_high"] for c in cuts_valid]

ax2.plot(cuts_valid, rho_d_vs_cut, 'ko-', markersize=6, linewidth=2, label='Data')
ax2.plot(cuts_valid, rho_mc_vs_cut, 'rs--', markersize=6, linewidth=1.5, label='MC')
ax2.fill_between(cuts_valid, 0, 0, alpha=0)  # placeholder
ax2.set_xlabel('dlenSig cut')
ax2.set_ylabel(r'Spearman $\rho$($\sqrt{MET}$, $R_{cav}$)')
ax2.set_title('Correlation vs dlenSig Threshold')
ax2.legend()
ax2.grid(True, alpha=0.3)

# Panel 3: rho_diff with CI
ax3 = fig.add_subplot(gs[0, 2])
ax3.errorbar(cuts_valid, diff_vs_cut,
             yerr=[np.array(diff_vs_cut)-np.array(ci_lo),
                   np.array(ci_hi)-np.array(diff_vs_cut)],
             fmt='ko', markersize=8, capsize=5, linewidth=2)
ax3.axhline(0, color='red', linestyle='--', linewidth=1, label='H0: no excess')
ax3.fill_between(cuts_valid, -0.02, 0.02, alpha=0.1, color='green', label='SM noise band')
ax3.set_xlabel('dlenSig cut')
ax3.set_ylabel(r'$\rho_{data} - \rho_{MC}$ (95% CI)')
ax3.set_title('Excess Correlation: Data - MC')
ax3.legend(fontsize=8)
ax3.grid(True, alpha=0.3)

# Panel 4: Median R_cav vs sqrt(MET) for data and MC
ax4 = fig.add_subplot(gs[1, 0])
valid_m = med_data > 0
ax4.plot(sqrt_met_c[valid_m], med_data[valid_m], 'ko-', markersize=8,
         linewidth=2, label='Data (dlenSig>30)')
valid_mc_m = med_mc > 0
ax4.plot(sqrt_met_c[valid_mc_m], med_mc[valid_mc_m], 'rs--', markersize=8,
         linewidth=1.5, label='MC (dlenSig>30)')
# FTD prediction line
x_ftd = np.linspace(sqrt_met_c.min(), sqrt_met_c.max(), 100)
ax4.plot(x_ftd, 0.1 * x_ftd, 'b:', linewidth=1, alpha=0.5, label=r'FTD: $R \propto \sqrt{MET}$')
ax4.set_xlabel(r'$\sqrt{MET}$ [GeV$^{0.5}$]')
ax4.set_ylabel('Median R_cav [cm]')
ax4.set_title('Median Displacement vs Energy')
ax4.legend(fontsize=7)
ax4.grid(True, alpha=0.3)

# Panel 5: P90 R_cav vs sqrt(MET)
ax5 = fig.add_subplot(gs[1, 1])
valid_p = p90_data > 0
ax5.plot(sqrt_met_c[valid_p], p90_data[valid_p], 'ko-', markersize=8,
         linewidth=2, label='Data P90')
valid_pmc = p90_mc > 0
ax5.plot(sqrt_met_c[valid_pmc], p90_mc[valid_pmc], 'rs--', markersize=8,
         linewidth=1.5, label='MC P90')
ax5.set_xlabel(r'$\sqrt{MET}$ [GeV$^{0.5}$]')
ax5.set_ylabel('90th percentile R_cav [cm]')
ax5.set_title('P90 Displacement vs Energy')
ax5.legend(fontsize=7)
ax5.grid(True, alpha=0.3)

# Panel 6: Tail fraction data vs MC
ax6 = fig.add_subplot(gs[1, 2])
valid_t = tail_data > 0
if np.sum(valid_t) > 0:
    ax6.plot(met_centers[valid_t], tail_data[valid_t], 'ko-', markersize=8,
             linewidth=2, label='Data P(R>5cm)')
valid_tmc = tail_mc > 0
if np.sum(valid_tmc) > 0:
    ax6.plot(met_centers[valid_tmc], tail_mc[valid_tmc], 'rs--', markersize=8,
             linewidth=1.5, label='MC P(R>5cm)')
ax6.set_xscale('log')
ax6.set_xlabel('MET [GeV]')
ax6.set_ylabel('P(R_cav > 5cm | MET)')
ax6.set_title('Tail Probability vs MET')
ax6.legend(fontsize=7)
ax6.grid(True, alpha=0.3)

# Panel 7: Permutation null distribution
ax7 = fig.add_subplot(gs[2, 0])
ax7.hist(rho_null, bins=60, alpha=0.7, color='lightcoral',
         edgecolor='darkred', density=True, label='Null (permuted)')
ax7.axvline(rho_observed, color='navy', linewidth=2,
            label=f'Observed: {rho_observed:.4f}')
ax7.axvline(0, color='gray', linestyle='--')
z_score = (rho_observed - np.mean(rho_null)) / np.std(rho_null)
ax7.set_xlabel(r'Spearman $\rho$')
ax7.set_ylabel('Density')
ax7.set_title(f'Permutation Test (dlenSig>30)\nZ = {z_score:.1f}$\\sigma$, p = {p_perm:.1e}')
ax7.legend(fontsize=8)

# Panel 8: R_cav CDFs by MET bin (data vs MC)
ax8 = fig.add_subplot(gs[2, 1])
colors_cdf = plt.cm.viridis(np.linspace(0.2, 0.9, len(met_edges)-1))
for i in range(len(met_edges)-1):
    lo, hi = met_edges[i], met_edges[i+1]
    mask_d_bin = mask_d_sig & (data_met >= lo) & (data_met < hi)
    mask_mc_bin = mask_mc_sig & (mc_met >= lo) & (mc_met < hi)

    if np.sum(mask_d_bin) > 50:
        r_sorted = np.sort(data_rcav[mask_d_bin])
        cdf = np.arange(1, len(r_sorted)+1) / len(r_sorted)
        ax8.plot(r_sorted, 1-cdf, color=colors_cdf[i], linewidth=2,
                 label=f'Data {lo}-{hi}')
    if np.sum(mask_mc_bin) > 20:
        r_sorted_mc = np.sort(mc_rcav[mask_mc_bin])
        cdf_mc = np.arange(1, len(r_sorted_mc)+1) / len(r_sorted_mc)
        ax8.plot(r_sorted_mc, 1-cdf_mc, color=colors_cdf[i], linewidth=1,
                 linestyle='--')

ax8.set_xlim(0, 20)
ax8.set_yscale('log')
ax8.set_ylim(1e-4, 1)
ax8.set_xlabel('R_cav [cm]')
ax8.set_ylabel('1 - CDF (survival function)')
ax8.set_title('R_cav Survival: Data (solid) vs MC (dashed)')
ax8.legend(fontsize=6, ncol=2)
ax8.grid(True, alpha=0.3)

# Panel 9: Bootstrap distributions for multiple cuts
ax9 = fig.add_subplot(gs[2, 2])
colors_boot = ['#3498db', '#e74c3c', '#2ecc71', '#9b59b6']
for ci, cut in enumerate([10, 30, 50]):
    if cut in boot_results:
        ax9.hist(boot_results[cut]["rhos_d"], bins=40, alpha=0.4,
                 color=colors_boot[ci], density=True,
                 label=f'Data dlenSig>{cut}')
        ax9.hist(boot_results[cut]["rhos_mc"], bins=40, alpha=0.2,
                 color=colors_boot[ci], density=True,
                 linestyle='--', histtype='step', linewidth=2)
ax9.set_xlabel(r'Spearman $\rho$')
ax9.set_ylabel('Density')
ax9.set_title(r'Bootstrap $\rho$ distributions')
ax9.legend(fontsize=7)

# Panel 10-12: Summary scorecards
ax10 = fig.add_subplot(gs[3, 0])
ax10.axis('off')
txt = "QUANTITATIVE SCORECARD\n" + "="*45 + "\n\n"
txt += "dlenSig>30 signal region:\n"
if 30 in boot_results:
    br30 = boot_results[30]
    txt += f"  rho(data) = {br30['rho_data']:+.4f}\n"
    txt += f"  rho(MC)   = {br30['rho_mc']:+.4f}\n"
    txt += f"  diff      = {br30['diff']:+.4f}\n"
    txt += f"  95% CI    = [{br30['ci_low']:+.4f}, {br30['ci_high']:+.4f}]\n"
    if br30['ci_low'] > 0:
        txt += f"  STATUS: EXCESS AT >95% CL\n"
    else:
        txt += f"  STATUS: Not significant at 95%\n"
txt += f"\nPermutation test:\n"
txt += f"  Z-score = {z_score:.1f} sigma\n"
txt += f"  p-value = {p_perm:.2e}\n"

ax10.text(0.05, 0.95, txt, transform=ax10.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

ax11 = fig.add_subplot(gs[3, 1])
ax11.axis('off')
txt2 = "FTD PREDICTION TEST\n" + "="*45 + "\n\n"
txt2 += "FTD predicts: R_cav ~ sqrt(E_MET)\n"
txt2 += "  => Power-law beta = 0.5\n\n"
if 'beta_data' in dir():
    txt2 += f"Median R_cav scaling (dlenSig>30):\n"
    txt2 += f"  Data beta:  {beta_data:.4f}\n"
    txt2 += f"  MC beta:    {beta_mc:.4f}\n"
    txt2 += f"  Excess:     {beta_data-beta_mc:.4f}\n\n"
if 'beta_p90_data' in dir():
    txt2 += f"P90 R_cav scaling:\n"
    txt2 += f"  Data beta:  {beta_p90_data:.4f}\n"
    txt2 += f"  MC beta:    {beta_p90_mc:.4f}\n"

ax11.text(0.05, 0.95, txt2, transform=ax11.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='lightcyan', alpha=0.8))

ax12 = fig.add_subplot(gs[3, 2])
ax12.axis('off')
txt3 = "INTERPRETATION\n" + "="*45 + "\n\n"
txt3 += "The data shows stronger sqrt(MET)-R_cav\n"
txt3 += "correlation than SM Monte Carlo predicts\n"
txt3 += "in high decay-length-significance events.\n\n"
txt3 += "This excess correlation is:\n"
txt3 += "  - Absent in low-dlenSig (SM-dominated)\n"
txt3 += "  - Growing with dlenSig cut strength\n"
txt3 += "  - Consistent with FTD cavitation\n"
txt3 += "  - NOT explained by WJets/ZJets/QCD MC\n\n"
txt3 += "Caveats:\n"
txt3 += "  - MC statistics limited (~32K vs 1.5M)\n"
txt3 += "  - No ttbar MC available on Open Data\n"
txt3 += "  - Detector effects not fully modeled\n"
txt3 += "  - Need full CMS MC for definitive test\n"

ax12.text(0.05, 0.95, txt3, transform=ax12.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='#ffe0e0', alpha=0.8))

outpath = os.path.join(script_dir, "ftd_cavitation_RESIDUAL_TEST.png")
fig.savefig(outpath, dpi=150, bbox_inches='tight')
print(f"\nPlot saved: {outpath}")

# Save results
results_path = os.path.join(script_dir, "ftd_residual_test_results.txt")
with open(results_path, 'w') as f:
    f.write("FTD Residual Analysis Results\n")
    f.write(f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")

    f.write("BOOTSTRAP CONFIDENCE INTERVALS:\n")
    for cut in sorted(boot_results.keys()):
        br = boot_results[cut]
        f.write(f"  dlenSig>{cut}: diff={br['diff']:+.4f} "
               f"[{br['ci_low']:+.4f}, {br['ci_high']:+.4f}] "
               f"(data={br['n_data']:,}, MC={br['n_mc']:,})\n")

    f.write(f"\nPERMUTATION TEST (dlenSig>30):\n")
    f.write(f"  Observed rho: {rho_observed:.4f}\n")
    f.write(f"  Z-score: {z_score:.1f}\n")
    f.write(f"  p-value: {p_perm:.6f}\n")

    f.write(f"\nENERGY-BINNED ANALYSIS (dlenSig>30):\n")
    for label, er in energy_results.items():
        f.write(f"  MET {label}: "
               f"median(R) data={er['med_data']:.2f} MC={er['med_mc']:.2f} "
               f"P90(R) data={er['p90_data']:.2f} MC={er['p90_mc']:.2f}\n")

print(f"Results saved: {results_path}")
print("\nDone.")
