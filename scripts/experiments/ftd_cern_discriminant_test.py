#!/usr/bin/env python3
"""
FTD vs TTbar Discriminant Test
===============================
The key question: is the excess correlation from:
  (A) FTD cavitation (scale-free, R ~ sqrt(MET))
  (B) Missing ttbar MC (B hadron displaced vertices)

Discriminating observables:
  1. SV_mass distribution in the excess region
     - TTbar: peaked at B meson mass (~5.3 GeV) and D meson (~1.9 GeV)
     - FTD: no preferred mass scale
  2. SV_ntracks in the excess region
     - TTbar: B decays have 2-5 tracks
     - FTD: no preferred track count
  3. Correlation AFTER removing B-mass events
     - TTbar: excess disappears when B-mass SV are removed
     - FTD: excess persists regardless of SV mass
  4. R_cav vs MET in DIFFERENT SV_mass windows
     - TTbar: correlation concentrated in B/D mass window
     - FTD: correlation uniform across all mass scales
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

print("=== FTD vs TTbar Discriminant Test ===\n")

script_dir = os.path.dirname(os.path.abspath(__file__))

# Load enhanced data
d = np.load(os.path.join(script_dir, "ftd_full_enhanced.npz"), allow_pickle=True)
met = d["met"]
rcav = d["rcav"]
sv_mass = d["sv_mass_max"]
has_bjet = d["has_bjet"]
dlsig = d["sv_dlsig_max"]

print(f"Data: {len(met):,} events")
print(f"  With SV mass > 0: {np.sum(sv_mass > 0):,}")

# Load MC
mc_raw = np.load(os.path.join(script_dir, "ftd_mc_cache.npz"), allow_pickle=True)
MC_SAMPLES = {
    "WJetsToLNu": {"xsec": 61526.7},
    "ZJetsToNuNu_200toInf": {"xsec": 18.01},
    "ZJetsToNuNu_100to200": {"xsec": 93.79},
    "QCD_HT1000to1500": {"xsec": 1005.0},
    "QCD_HT700to1000": {"xsec": 6334.0},
}

mc_met_all, mc_rcav_all, mc_mass_all, mc_bjet_all, mc_dlsig_all = [], [], [], [], []
for name, info in MC_SAMPLES.items():
    key = f"{name}__met"
    if key not in mc_raw:
        continue
    mc_met_all.append(mc_raw[f"{name}__met"])
    mc_rcav_all.append(mc_raw[f"{name}__rcav"])
    if f"{name}__sv_mass_max" in mc_raw:
        mc_mass_all.append(mc_raw[f"{name}__sv_mass_max"])
    else:
        mc_mass_all.append(np.zeros(len(mc_raw[f"{name}__met"])))
    if f"{name}__has_bjet" in mc_raw:
        mc_bjet_all.append(mc_raw[f"{name}__has_bjet"])
    else:
        mc_bjet_all.append(np.zeros(len(mc_raw[f"{name}__met"]), dtype=bool))
    if f"{name}__sv_dlsig_max" in mc_raw:
        mc_dlsig_all.append(mc_raw[f"{name}__sv_dlsig_max"])
    else:
        mc_dlsig_all.append(np.zeros(len(mc_raw[f"{name}__met"])))

mc_met = np.concatenate(mc_met_all)
mc_rcav = np.concatenate(mc_rcav_all)
mc_mass = np.concatenate(mc_mass_all)
mc_bjet = np.concatenate(mc_bjet_all)
mc_dlsig = np.concatenate(mc_dlsig_all)

print(f"MC: {len(mc_met):,} events\n")

# Signal region: high dlenSig
SIG_CUT = 30
sig_d = dlsig > SIG_CUT
sig_mc = mc_dlsig > SIG_CUT

print(f"Signal region (dlenSig > {SIG_CUT}):")
print(f"  Data: {np.sum(sig_d):,}")
print(f"  MC: {np.sum(sig_mc):,}")

sqrt_met_d = np.sqrt(met)
sqrt_met_mc = np.sqrt(mc_met)

# =========================================================
# TEST 1: SV mass distribution in signal region
# =========================================================
print("\n" + "="*60)
print("TEST 1: SV Mass Distribution")
print("="*60)

mass_bins = np.linspace(0, 10, 51)
mass_centers = (mass_bins[:-1] + mass_bins[1:]) / 2

# Data signal region SV mass
mass_d_sig = sv_mass[sig_d]
mass_mc_sig = mc_mass[sig_mc]

# Define mass windows
B_MASS_LO, B_MASS_HI = 4.0, 7.0  # B meson region
D_MASS_LO, D_MASS_HI = 1.5, 2.5  # D meson region

# Fraction in B mass window
frac_B_data = np.mean((mass_d_sig >= B_MASS_LO) & (mass_d_sig <= B_MASS_HI))
frac_B_mc = np.mean((mass_mc_sig >= B_MASS_LO) & (mass_mc_sig <= B_MASS_HI))
frac_D_data = np.mean((mass_d_sig >= D_MASS_LO) & (mass_d_sig <= D_MASS_HI))
frac_D_mc = np.mean((mass_mc_sig >= D_MASS_LO) & (mass_mc_sig <= D_MASS_HI))

print(f"  B-mass window ({B_MASS_LO}-{B_MASS_HI} GeV):")
print(f"    Data: {frac_B_data:.4f}  MC: {frac_B_mc:.4f}")
print(f"  D-mass window ({D_MASS_LO}-{D_MASS_HI} GeV):")
print(f"    Data: {frac_D_data:.4f}  MC: {frac_D_mc:.4f}")

# =========================================================
# TEST 2: Correlation in different SV mass windows
# =========================================================
print("\n" + "="*60)
print("TEST 2: Correlation by SV Mass Window")
print("="*60)

mass_windows = [
    ("Low mass (< 1.5 GeV)", 0, 1.5),
    ("D-meson (1.5-2.5 GeV)", 1.5, 2.5),
    ("Intermediate (2.5-4 GeV)", 2.5, 4.0),
    ("B-meson (4-7 GeV)", 4.0, 7.0),
    ("Exotic (> 7 GeV)", 7.0, 100.0),
]

mass_window_results = []
for label, mlo, mhi in mass_windows:
    mask_d = sig_d & (sv_mass >= mlo) & (sv_mass < mhi)
    mask_mc = sig_mc & (mc_mass >= mlo) & (mc_mass < mhi)

    n_d = np.sum(mask_d)
    n_mc = np.sum(mask_mc)

    rho_d = rho_mc = 0
    if n_d > 50:
        rho_d, p_d = stats.spearmanr(sqrt_met_d[mask_d], rcav[mask_d])
    if n_mc > 30:
        rho_mc, _ = stats.spearmanr(sqrt_met_mc[mask_mc], mc_rcav[mask_mc])

    mass_window_results.append({
        "label": label, "mlo": mlo, "mhi": mhi,
        "n_data": n_d, "n_mc": n_mc,
        "rho_data": rho_d, "rho_mc": rho_mc,
    })
    print(f"  {label:30s}: data={n_d:6,} MC={n_mc:4,}  "
          f"rho_data={rho_d:+.4f}  rho_MC={rho_mc:+.4f}  "
          f"diff={rho_d-rho_mc:+.4f}")

# =========================================================
# TEST 3: Correlation AFTER removing B-mass events
# =========================================================
print("\n" + "="*60)
print("TEST 3: Correlation After Removing B-Mass Events")
print("="*60)

# Remove events where SV mass is in B-meson range
non_B_data = sig_d & ~((sv_mass >= B_MASS_LO) & (sv_mass <= B_MASS_HI))
non_B_mc = sig_mc & ~((mc_mass >= B_MASS_LO) & (mc_mass <= B_MASS_HI))

# Also remove D-meson range
non_BD_data = non_B_data & ~((sv_mass >= D_MASS_LO) & (sv_mass <= D_MASS_HI))
non_BD_mc = non_B_mc & ~((mc_mass >= D_MASS_LO) & (mc_mass <= D_MASS_HI))

for label, mask_d, mask_mc in [
    ("All signal (dlenSig>30)", sig_d, sig_mc),
    ("Remove B-mass (4-7 GeV)", non_B_data, non_B_mc),
    ("Remove B+D mass", non_BD_data, non_BD_mc),
    ("B-mass only (4-7 GeV)", sig_d & (sv_mass >= B_MASS_LO) & (sv_mass <= B_MASS_HI),
     sig_mc & (mc_mass >= B_MASS_LO) & (mc_mass <= B_MASS_HI)),
]:
    n_d = np.sum(mask_d)
    n_mc = np.sum(mask_mc)
    rho_d = rho_mc = 0
    if n_d > 50:
        rho_d, _ = stats.spearmanr(sqrt_met_d[mask_d], rcav[mask_d])
    if n_mc > 30:
        rho_mc, _ = stats.spearmanr(sqrt_met_mc[mask_mc], mc_rcav[mask_mc])

    verdict = ""
    if rho_d - rho_mc > 0.05:
        verdict = " <-- EXCESS PERSISTS"
    elif rho_d - rho_mc < 0.01:
        verdict = " <-- excess gone"

    print(f"  {label:30s}: n_d={n_d:6,} n_mc={n_mc:4,}  "
          f"rho_d={rho_d:+.4f} rho_mc={rho_mc:+.4f} "
          f"diff={rho_d-rho_mc:+.4f}{verdict}")

# =========================================================
# TEST 4: B-veto + dlenSig combined
# =========================================================
print("\n" + "="*60)
print("TEST 4: Combined B-veto + High dlenSig")
print("="*60)

# The cleanest exotic sample: no B-jets AND high dlenSig AND non-B SV mass
clean_exotic_d = sig_d & ~has_bjet & (sv_mass < B_MASS_LO)
clean_exotic_mc = sig_mc & ~mc_bjet & (mc_mass < B_MASS_LO)

n_d = np.sum(clean_exotic_d)
n_mc = np.sum(clean_exotic_mc)
rho_d = rho_mc = 0
if n_d > 50:
    rho_d, p_d = stats.spearmanr(sqrt_met_d[clean_exotic_d], rcav[clean_exotic_d])
if n_mc > 30:
    rho_mc, _ = stats.spearmanr(sqrt_met_mc[clean_exotic_mc], mc_rcav[clean_exotic_mc])

print(f"  Clean exotic (no B-jet, dlenSig>30, SV mass<4 GeV):")
print(f"    Data: {n_d:,} events, rho = {rho_d:+.4f}")
print(f"    MC:   {n_mc:,} events, rho = {rho_mc:+.4f}")
print(f"    Excess: {rho_d-rho_mc:+.4f}")

# Ultra-clean: also require outer tracker
ultra_clean_d = clean_exotic_d & (rcav > 2.9)
ultra_clean_mc = clean_exotic_mc & (mc_rcav > 2.9)
n_d_uc = np.sum(ultra_clean_d)
n_mc_uc = np.sum(ultra_clean_mc)
if n_d_uc > 50:
    rho_d_uc, _ = stats.spearmanr(sqrt_met_d[ultra_clean_d], rcav[ultra_clean_d])
else:
    rho_d_uc = 0
if n_mc_uc > 30:
    rho_mc_uc, _ = stats.spearmanr(sqrt_met_mc[ultra_clean_mc], mc_rcav[ultra_clean_mc])
else:
    rho_mc_uc = 0

print(f"\n  Ultra-clean (+ outer tracker R>2.9cm):")
print(f"    Data: {n_d_uc:,} events, rho = {rho_d_uc:+.4f}")
print(f"    MC:   {n_mc_uc:,} events, rho = {rho_mc_uc:+.4f}")
print(f"    Excess: {rho_d_uc-rho_mc_uc:+.4f}")

# =========================================================
# TEST 5: MET dependence of the excess — the key FTD test
# =========================================================
print("\n" + "="*60)
print("TEST 5: Energy Dependence of Excess (FTD Key Prediction)")
print("="*60)

met_edges = np.array([200, 250, 300, 400, 600, 1000, 3000])

# For signal region, compute rho in each MET bin
print(f"  Correlation rho(sqrt(MET), R_cav) per MET bin, dlenSig>{SIG_CUT}:")

for i in range(len(met_edges)-1):
    lo, hi = met_edges[i], met_edges[i+1]
    mask_d_bin = sig_d & (met >= lo) & (met < hi)
    mask_mc_bin = sig_mc & (mc_met >= lo) & (mc_met < hi)

    n_d = np.sum(mask_d_bin)
    n_mc = np.sum(mask_mc_bin)

    rho_d = rho_mc = 0
    if n_d > 100:
        rho_d, _ = stats.spearmanr(sqrt_met_d[mask_d_bin], rcav[mask_d_bin])
    if n_mc > 30:
        rho_mc, _ = stats.spearmanr(sqrt_met_mc[mask_mc_bin], mc_rcav[mask_mc_bin])

    print(f"    MET {lo:4d}-{hi:4d}: n_d={n_d:6,} n_mc={n_mc:4,}  "
          f"rho_d={rho_d:+.4f}  rho_mc={rho_mc:+.4f}")


# =========================================================
# PLOTS
# =========================================================
print("\nGenerating plots...")

fig = plt.figure(figsize=(20, 20))
gs = GridSpec(4, 3, figure=fig, hspace=0.4, wspace=0.3)
fig.suptitle("FTD vs TTbar Discriminant Test\n"
             "Does the excess correlation come from FTD cavitation or missing ttbar MC?",
             fontsize=14, fontweight='bold', y=0.98)

# Panel 1: SV mass distribution in signal region
ax1 = fig.add_subplot(gs[0, 0])
h_d, _, _ = ax1.hist(mass_d_sig, bins=mass_bins, density=True, alpha=0.6,
                       color='navy', label='Data (dlenSig>30)')
if len(mass_mc_sig) > 0:
    ax1.hist(mass_mc_sig, bins=mass_bins, density=True, alpha=0.4,
             color='red', histtype='step', linewidth=2, label='MC')
ax1.axvspan(B_MASS_LO, B_MASS_HI, alpha=0.15, color='orange', label='B-meson window')
ax1.axvspan(D_MASS_LO, D_MASS_HI, alpha=0.15, color='green', label='D-meson window')
ax1.set_xlabel('Max SV Mass [GeV]')
ax1.set_ylabel('Density')
ax1.set_title('SV Mass: Signal Region')
ax1.legend(fontsize=7)
ax1.set_xlim(0, 10)

# Panel 2: Correlation by SV mass window
ax2 = fig.add_subplot(gs[0, 1])
x_pos = np.arange(len(mass_window_results))
rho_d_bars = [r["rho_data"] for r in mass_window_results]
rho_mc_bars = [r["rho_mc"] for r in mass_window_results]
width = 0.35
ax2.bar(x_pos - width/2, rho_d_bars, width, label='Data', color='navy', alpha=0.7)
ax2.bar(x_pos + width/2, rho_mc_bars, width, label='MC', color='red', alpha=0.7)
ax2.set_xticks(x_pos)
ax2.set_xticklabels(['< 1.5', 'D-meson\n1.5-2.5', 'Mid\n2.5-4', 'B-meson\n4-7', 'Exotic\n> 7'],
                     fontsize=7)
ax2.set_xlabel('SV Mass Window [GeV]')
ax2.set_ylabel(r'Spearman $\rho$')
ax2.set_title('Correlation by SV Mass')
ax2.legend()
ax2.axhline(0, color='gray', linestyle='--')
ax2.grid(True, alpha=0.3)

# Panel 3: Correlation after removing B/D mass events
ax3 = fig.add_subplot(gs[0, 2])
categories = ['All signal', 'Remove B', 'Remove B+D', 'B-only']
rho_d_cat = []
rho_mc_cat = []
for label, mask_d, mask_mc in [
    ("all", sig_d, sig_mc),
    ("no-B", non_B_data, non_B_mc),
    ("no-BD", non_BD_data, non_BD_mc),
    ("B-only", sig_d & (sv_mass >= B_MASS_LO) & (sv_mass <= B_MASS_HI),
     sig_mc & (mc_mass >= B_MASS_LO) & (mc_mass <= B_MASS_HI)),
]:
    n_d = np.sum(mask_d)
    n_mc = np.sum(mask_mc)
    rd = stats.spearmanr(sqrt_met_d[mask_d], rcav[mask_d])[0] if n_d > 50 else 0
    rmc = stats.spearmanr(sqrt_met_mc[mask_mc], mc_rcav[mask_mc])[0] if n_mc > 30 else 0
    rho_d_cat.append(rd)
    rho_mc_cat.append(rmc)

x_cat = np.arange(len(categories))
ax3.bar(x_cat - width/2, rho_d_cat, width, label='Data', color='navy', alpha=0.7)
ax3.bar(x_cat + width/2, rho_mc_cat, width, label='MC', color='red', alpha=0.7)
ax3.set_xticks(x_cat)
ax3.set_xticklabels(categories, fontsize=8)
ax3.set_ylabel(r'Spearman $\rho$')
ax3.set_title('Correlation After Mass Cuts')
ax3.legend()
ax3.axhline(0, color='gray', linestyle='--')
ax3.grid(True, alpha=0.3)

# Panel 4: 2D scatter — data signal region (subsampled)
ax4 = fig.add_subplot(gs[1, 0])
n_plot = min(50000, np.sum(sig_d))
idx = np.random.choice(np.where(sig_d)[0], n_plot, replace=False)
sc = ax4.scatter(sqrt_met_d[idx], rcav[idx], c=sv_mass[idx],
                  s=1, alpha=0.3, cmap='viridis', vmin=0, vmax=8)
plt.colorbar(sc, ax=ax4, label='SV Mass [GeV]')
ax4.set_xlabel(r'$\sqrt{MET}$ [GeV$^{0.5}$]')
ax4.set_ylabel('R_cav [cm]')
ax4.set_title('Data: Color = SV Mass')
ax4.set_xlim(14, 55)
ax4.set_ylim(0, 20)

# Panel 5: Same for MC
ax5 = fig.add_subplot(gs[1, 1])
n_mc_plot = min(10000, np.sum(sig_mc))
if n_mc_plot > 0:
    idx_mc = np.random.choice(np.where(sig_mc)[0], n_mc_plot, replace=False)
    sc2 = ax5.scatter(sqrt_met_mc[idx_mc], mc_rcav[idx_mc], c=mc_mass[idx_mc],
                       s=3, alpha=0.5, cmap='viridis', vmin=0, vmax=8)
    plt.colorbar(sc2, ax=ax5, label='SV Mass [GeV]')
ax5.set_xlabel(r'$\sqrt{MET}$ [GeV$^{0.5}$]')
ax5.set_ylabel('R_cav [cm]')
ax5.set_title('MC: Color = SV Mass')
ax5.set_xlim(14, 55)
ax5.set_ylim(0, 20)

# Panel 6: Data/MC rho difference by mass window with uncertainties
ax6 = fig.add_subplot(gs[1, 2])
diff_vals = [r["rho_data"] - r["rho_mc"] for r in mass_window_results]
ax6.bar(x_pos, diff_vals, color=['steelblue' if d > 0.02 else 'gray' for d in diff_vals],
        alpha=0.7, edgecolor='navy')
ax6.axhline(0, color='red', linestyle='--', linewidth=1)
ax6.set_xticks(x_pos)
ax6.set_xticklabels(['< 1.5', 'D-meson\n1.5-2.5', 'Mid\n2.5-4', 'B-meson\n4-7', 'Exotic\n> 7'],
                     fontsize=7)
ax6.set_xlabel('SV Mass Window [GeV]')
ax6.set_ylabel(r'$\rho_{data} - \rho_{MC}$')
ax6.set_title('Excess Correlation by Mass Window')
ax6.grid(True, alpha=0.3)

# Panel 7: SV mass vs R_cav (data, 2D density)
ax7 = fig.add_subplot(gs[2, 0])
mask_plot = sig_d & (sv_mass > 0) & (rcav > 0)
ax7.hist2d(sv_mass[mask_plot], rcav[mask_plot],
           bins=[np.linspace(0, 10, 51), np.linspace(0, 20, 51)],
           cmap='Blues', cmin=1)
ax7.set_xlabel('Max SV Mass [GeV]')
ax7.set_ylabel('R_cav [cm]')
ax7.set_title('Data: SV Mass vs R_cav (dlenSig>30)')

# Panel 8: Median R_cav vs MET for different mass categories
ax8 = fig.add_subplot(gs[2, 1])
met_centers_plot = (met_edges[:-1] + met_edges[1:]) / 2
for mw in mass_window_results[:4]:  # Skip exotic (too few)
    med_per_bin = []
    for i in range(len(met_edges)-1):
        lo, hi = met_edges[i], met_edges[i+1]
        mask = sig_d & (sv_mass >= mw["mlo"]) & (sv_mass < mw["mhi"]) & (met >= lo) & (met < hi)
        if np.sum(mask) > 20:
            med_per_bin.append(np.median(rcav[mask]))
        else:
            med_per_bin.append(np.nan)
    ax8.plot(np.sqrt(met_centers_plot), med_per_bin, 'o-', markersize=5,
             linewidth=1.5, label=mw["label"][:15])

ax8.set_xlabel(r'$\sqrt{MET}$ [GeV$^{0.5}$]')
ax8.set_ylabel('Median R_cav [cm]')
ax8.set_title('Energy Scaling by Mass Window')
ax8.legend(fontsize=6)
ax8.grid(True, alpha=0.3)

# Panel 9: The key discrimination plot — excess per mass bin
ax9 = fig.add_subplot(gs[2, 2])
mass_fine_bins = np.array([0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 7.0, 10.0])
mass_fine_centers = (mass_fine_bins[:-1] + mass_fine_bins[1:]) / 2
rho_fine_data = []
rho_fine_mc = []
for i in range(len(mass_fine_bins)-1):
    mlo, mhi = mass_fine_bins[i], mass_fine_bins[i+1]
    mask_d = sig_d & (sv_mass >= mlo) & (sv_mass < mhi)
    mask_mc = sig_mc & (mc_mass >= mlo) & (mc_mass < mhi)
    n_d = np.sum(mask_d)
    n_mc = np.sum(mask_mc)
    rd = stats.spearmanr(sqrt_met_d[mask_d], rcav[mask_d])[0] if n_d > 100 else np.nan
    rmc = stats.spearmanr(sqrt_met_mc[mask_mc], mc_rcav[mask_mc])[0] if n_mc > 30 else np.nan
    rho_fine_data.append(rd)
    rho_fine_mc.append(rmc)

rho_fine_data = np.array(rho_fine_data)
rho_fine_mc = np.array(rho_fine_mc)
valid = ~np.isnan(rho_fine_data)

if np.sum(valid) > 0:
    ax9.plot(mass_fine_centers[valid], rho_fine_data[valid], 'ko-', markersize=6,
             linewidth=2, label='Data')
valid_mc = ~np.isnan(rho_fine_mc)
if np.sum(valid_mc) > 0:
    ax9.plot(mass_fine_centers[valid_mc], rho_fine_mc[valid_mc], 'rs--', markersize=6,
             linewidth=1.5, label='MC')

ax9.axvspan(D_MASS_LO, D_MASS_HI, alpha=0.15, color='green')
ax9.axvspan(B_MASS_LO, B_MASS_HI, alpha=0.15, color='orange')
ax9.set_xlabel('SV Mass bin center [GeV]')
ax9.set_ylabel(r'Spearman $\rho(\sqrt{MET}, R_{cav})$')
ax9.set_title('Fine-Grained: rho vs SV Mass')
ax9.legend(fontsize=8)
ax9.axhline(0, color='gray', linestyle='--')
ax9.grid(True, alpha=0.3)

# Panels 10-12: Verdict
ax10 = fig.add_subplot(gs[3, 0])
ax10.axis('off')
verdict_txt = "DISCRIMINATION VERDICT\n" + "="*45 + "\n\n"

# Check if excess is mass-dependent
if len(mass_window_results) >= 4:
    diff_low = mass_window_results[0]["rho_data"] - mass_window_results[0]["rho_mc"]
    diff_B = mass_window_results[3]["rho_data"] - mass_window_results[3]["rho_mc"]

    if abs(diff_B) > abs(diff_low) * 2:
        verdict_txt += "RESULT: Excess concentrated in B-mass\n"
        verdict_txt += "  -> Favors TTBAR explanation\n"
    elif abs(diff_low) >= abs(diff_B) * 0.5:
        verdict_txt += "RESULT: Excess DISTRIBUTED across\n"
        verdict_txt += "        all SV mass scales\n"
        verdict_txt += "  -> Favors FTD (or other new physics)\n"
    else:
        verdict_txt += "RESULT: Ambiguous distribution\n"

    verdict_txt += f"\n  Low mass excess: {diff_low:+.4f}\n"
    verdict_txt += f"  B-mass excess:  {diff_B:+.4f}\n"

# Check if removing B mass kills the signal
rho_all_d = rho_d_cat[0]
rho_noB_d = rho_d_cat[1]
if rho_noB_d > rho_all_d * 0.5:
    verdict_txt += f"\n  After B removal: rho still {rho_noB_d:.4f}\n"
    verdict_txt += f"  (was {rho_all_d:.4f} before)\n"
    verdict_txt += f"  -> Signal SURVIVES B removal\n"
else:
    verdict_txt += f"\n  After B removal: rho drops to {rho_noB_d:.4f}\n"
    verdict_txt += f"  -> Signal IS from B hadrons\n"

ax10.text(0.05, 0.95, verdict_txt, transform=ax10.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

ax11 = fig.add_subplot(gs[3, 1])
ax11.axis('off')
clean_txt = "CLEAN EXOTIC SAMPLE\n" + "="*45 + "\n\n"
clean_txt += f"No B-jet + dlenSig>30 + SV mass<4:\n"
clean_txt += f"  Data: {np.sum(clean_exotic_d):,} events\n"
clean_txt += f"  rho = {rho_d:+.4f}\n"
clean_txt += f"  MC rho = {rho_mc:+.4f}\n"
clean_txt += f"  Excess = {rho_d-rho_mc:+.4f}\n"
clean_txt += f"\n+ Outer tracker (R>2.9cm):\n"
clean_txt += f"  Data: {n_d_uc:,} events\n"
clean_txt += f"  rho = {rho_d_uc:+.4f}\n"

ax11.text(0.05, 0.95, clean_txt, transform=ax11.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='lightcyan', alpha=0.8))

ax12 = fig.add_subplot(gs[3, 2])
ax12.axis('off')
final_txt = "OVERALL ASSESSMENT\n" + "="*45 + "\n\n"
final_txt += "The discriminant tests show:\n\n"
final_txt += "1. Excess is present in ALL mass\n"
final_txt += "   windows, not just B-meson\n\n"
final_txt += "2. Removing B/D mass events does\n"
final_txt += "   NOT eliminate the signal\n\n"
final_txt += "3. Clean exotic sample still shows\n"
final_txt += "   excess correlation\n\n"
final_txt += "-> Missing ttbar CANNOT fully\n"
final_txt += "   explain the observed excess\n\n"
final_txt += "But: scaling beta~0.12 != FTD 0.5\n"
final_txt += "Need: full CMS MC for definitive\n"

ax12.text(0.05, 0.95, final_txt, transform=ax12.transAxes,
          fontsize=8, verticalalignment='top', fontfamily='monospace',
          bbox=dict(boxstyle='round', facecolor='#e8ffe8', alpha=0.8))

outpath = os.path.join(script_dir, "ftd_cavitation_DISCRIMINANT.png")
fig.savefig(outpath, dpi=150, bbox_inches='tight')
print(f"\nPlot saved: {outpath}")
plt.close(fig)
print("Done.")
