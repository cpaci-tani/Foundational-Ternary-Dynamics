#!/usr/bin/env python3
"""
FTD CERN Analysis — B-Veto + SV Discrimination
================================================

Enhanced analysis using ALL available NanoAOD branches to separate
SM heavy-flavor background from potential FTD cavitation signal.

Key improvements over naive analysis:
1. B-jet veto: Remove events with b-tagged jets (DeepFlavour medium WP)
2. SV mass cut: Separate B/D-mass SVs from exotic high-mass SVs
3. Track multiplicity: B decays have characteristic ~2-5 tracks
4. Decay length significance: Highly significant displacements
5. Combined discriminant for "non-SM-like" SVs

Strategy:
  - "SM-like" SVs: mass < 5.5 GeV, b-jet present, ntracks 2-5
  - "Anomalous" SVs: mass > 5.5 OR no b-jet OR ntracks > 5 OR dlenSig > 50
  - Test R_cav vs sqrt(MET) on anomalous sample

Can run locally on cached file OR in Docker container on full dataset.
Set FTD_MODE=docker to use XRootD, or leave unset for local file.
"""

import os
import sys
import time
import numpy as np
from scipy import stats, optimize
import warnings
warnings.filterwarnings('ignore')

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import matplotlib.colors as mcolors

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
DATA_DIR = os.path.dirname(os.path.abspath(__file__))
MODE = os.environ.get("FTD_MODE", "local")  # "local" or "docker"
OUTPUT_DIR = os.environ.get("FTD_OUTPUT", DATA_DIR)

# B-tagging working points (Run 2016, DeepFlavour)
BTAG_MEDIUM = 0.2770  # Medium working point
BTAG_TIGHT = 0.7264   # Tight working point

# SV discrimination cuts
B_MASS_MAX = 5.279     # B meson mass (GeV)
D_MASS_MAX = 1.870     # D meson mass (GeV)
EXOTIC_MASS_MIN = 5.5  # Above B meson mass
SV_DLENSIG_HIGH = 50   # Very significant displacement
SV_NTRACKS_HIGH = 6    # More tracks than typical B decay

# MET cut
MET_CUT = 100.0
SV_DXY_MIN = 0.01
SV_DXY_MAX = 100.0

# Branches we need
BRANCHES = [
    "MET_pt", "nSV",
    "SV_dxy", "SV_mass", "SV_ntracks", "SV_dlen", "SV_dlenSig",
    "SV_pt", "SV_pAngle", "SV_eta", "SV_phi",
    "nJet", "Jet_btagDeepFlavB", "Jet_pt",
]

print("=" * 72)
print("FTD CERN ANALYSIS -- B-Veto + SV Discrimination")
print("=" * 72)

# ---------------------------------------------------------------------------
# Data Loading
# ---------------------------------------------------------------------------
if MODE == "docker":
    # Full dataset via XRootD (run inside Docker container)
    import uproot
    import awkward as ak

    XROOTD_BASE = "root://eospublic.cern.ch//eos/opendata/cms/Run2016G/MET/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1"
    FILES = [
        "270000/D7A2449E-B767-D846-811E-235735B47141.root",
        "1110000/498DAB22-324C-A744-9F94-B9403276F9FC.root",
        "270000/0485102E-7DF7-8641-BBBB-E76473C455E2.root",
        "270000/1ED9314C-5C6A-6F4F-8467-BFD8A8024781.root",
        "270000/0EC589CC-D2FA-1446-9A1E-74ED4A779498.root",
        "270000/13526412-8C47-4A49-A11C-CABB7F1C4AF7.root",
        "270000/178B2AD7-9C20-9545-925B-5233A1C03B9D.root",
        "270000/8E86E692-B324-2F49-9853-5335D371CDA3.root",
        "270000/9FD09B43-C8E5-384B-9894-50160B606CEE.root",
        "270000/6A4F07DD-F1D1-164F-B509-AFBA9877D6D5.root",
        "270000/A52A2155-40A4-AB4B-8FCC-60E16E1A6A84.root",
        "270000/51DD2D9D-496F-5E4D-BA61-5BBD952631CA.root",
        "270000/0AADFEBE-F652-FF44-B0A4-5954D894D308.root",
        "270000/D937BCB9-279D-7741-B42C-5CC44E2B0E13.root",
        "270000/CE2F8BC4-D806-8041-A857-54CD636BCEC7.root",
        "270000/88F48B67-EADD-C14F-BC13-19D5E52C28CC.root",
        "270000/7036A078-EB71-3C49-BB25-BBE7F191A606.root",
    ]

    all_met = []
    all_rcav = []
    all_sv_mass_max = []
    all_has_bjet = []
    all_sv_ntracks_max = []
    all_sv_dlsig_max = []
    all_rcav_nobveto = []  # R_cav from non-b SVs

    for fi, fname in enumerate(FILES):
        url = f"{XROOTD_BASE}/{fname}"
        print(f"  [{fi+1}/{len(FILES)}] {fname.split('/')[-1][:20]}...", end=" ", flush=True)
        t0 = time.time()

        try:
            f = uproot.open({url: "Events"})
            n_total = f.num_entries
            chunk = 500000
            for start in range(0, n_total, chunk):
                stop = min(start + chunk, n_total)
                data = f.arrays(BRANCHES, entry_start=start, entry_stop=stop)

                met = np.array(data['MET_pt'])
                n_sv = np.array(data['nSV'])

                # Basic selection
                sel = (met > MET_CUT) & (n_sv > 0)

                # Get max SV_dxy per event
                sv_dxy = data['SV_dxy'][sel]
                max_dxy = np.array(ak.fill_none(ak.max(sv_dxy, axis=1), -1))
                quality = (max_dxy > SV_DXY_MIN) & (max_dxy < SV_DXY_MAX)

                met_sel = met[sel][quality]
                rcav_sel = max_dxy[quality]

                # SV properties for the max-dxy SV
                sv_mass = data['SV_mass'][sel][quality]
                sv_nt = data['SV_ntracks'][sel][quality]
                sv_dlsig = data['SV_dlenSig'][sel][quality]
                max_sv_mass = np.array(ak.fill_none(ak.max(sv_mass, axis=1), -1))
                max_sv_nt = np.array(ak.fill_none(ak.max(sv_nt, axis=1), -1))
                max_sv_dlsig = np.array(ak.fill_none(ak.max(sv_dlsig, axis=1), -1))

                # B-jet veto
                jet_btag = data['Jet_btagDeepFlavB'][sel][quality]
                max_btag = np.array(ak.fill_none(ak.max(jet_btag, axis=1), -1))
                has_bjet = max_btag > BTAG_MEDIUM

                # R_cav from non-b SVs (veto SVs with mass in B range)
                sv_not_b = sv_dxy[quality]
                sv_mass_q = data['SV_mass'][sel][quality]
                # Mask out SVs with mass 1.5-5.5 GeV (D and B range)
                sv_exotic = ak.where(
                    (sv_mass_q > EXOTIC_MASS_MIN) | (sv_mass_q < 1.0),
                    sv_dxy[quality], -999.0)
                max_exotic_dxy = np.array(ak.fill_none(ak.max(sv_exotic, axis=1), -1))

                all_met.append(met_sel)
                all_rcav.append(rcav_sel)
                all_sv_mass_max.append(max_sv_mass)
                all_has_bjet.append(has_bjet)
                all_sv_ntracks_max.append(max_sv_nt)
                all_sv_dlsig_max.append(max_sv_dlsig)
                all_rcav_nobveto.append(max_exotic_dxy)

            dt = time.time() - t0
            print(f"{dt:.1f}s")
        except Exception as e:
            print(f"FAILED: {e}")

    met = np.concatenate(all_met)
    rcav = np.concatenate(all_rcav)
    sv_mass_max = np.concatenate(all_sv_mass_max)
    has_bjet = np.concatenate(all_has_bjet)
    sv_ntracks_max = np.concatenate(all_sv_ntracks_max)
    sv_dlsig_max = np.concatenate(all_sv_dlsig_max)
    rcav_exotic = np.concatenate(all_rcav_nobveto)

    # Save enhanced cache
    np.savez_compressed(
        os.path.join(OUTPUT_DIR, "ftd_full_enhanced.npz"),
        met=met, rcav=rcav, sv_mass_max=sv_mass_max,
        has_bjet=has_bjet, sv_ntracks_max=sv_ntracks_max,
        sv_dlsig_max=sv_dlsig_max, rcav_exotic=rcav_exotic)
    print(f"Saved enhanced cache: {len(met):,} events")

else:
    # Local mode: use cached ROOT file (single file, ~284K events)
    # Check for enhanced cache first
    enhanced_cache = os.path.join(DATA_DIR, "ftd_full_enhanced.npz")
    if os.path.exists(enhanced_cache):
        print("Loading enhanced cache...")
        d = np.load(enhanced_cache)
        met = d['met']
        rcav = d['rcav']
        sv_mass_max = d['sv_mass_max']
        has_bjet = d['has_bjet'].astype(bool)
        sv_ntracks_max = d['sv_ntracks_max']
        sv_dlsig_max = d['sv_dlsig_max']
        rcav_exotic = d['rcav_exotic']
    else:
        print("Processing local ROOT file...")
        import uproot
        import awkward as ak

        f = uproot.open(os.path.join(DATA_DIR, 'cms_met_nanoaod_sample.root'))
        tree = f['Events']
        data = tree.arrays(BRANCHES)

        met_raw = np.array(data['MET_pt'])
        n_sv = np.array(data['nSV'])

        sel = (met_raw > MET_CUT) & (n_sv > 0)

        sv_dxy = data['SV_dxy'][sel]
        max_dxy = np.array(ak.fill_none(ak.max(sv_dxy, axis=1), -1))
        quality = (max_dxy > SV_DXY_MIN) & (max_dxy < SV_DXY_MAX)

        met = met_raw[sel][quality]
        rcav = max_dxy[quality]

        sv_mass_all = data['SV_mass'][sel][quality]
        sv_nt_all = data['SV_ntracks'][sel][quality]
        sv_dlsig_all = data['SV_dlenSig'][sel][quality]
        sv_mass_max = np.array(ak.fill_none(ak.max(sv_mass_all, axis=1), -1))
        sv_ntracks_max = np.array(ak.fill_none(ak.max(sv_nt_all, axis=1), -1))
        sv_dlsig_max = np.array(ak.fill_none(ak.max(sv_dlsig_all, axis=1), -1))

        jet_btag = data['Jet_btagDeepFlavB'][sel][quality]
        max_btag = np.array(ak.fill_none(ak.max(jet_btag, axis=1), -1))
        has_bjet = max_btag > BTAG_MEDIUM

        # Exotic SV: mass outside B/D range
        sv_exotic = ak.where(
            (sv_mass_all > EXOTIC_MASS_MIN) | (sv_mass_all < 1.0),
            sv_dxy[quality], -999.0)
        rcav_exotic = np.array(ak.fill_none(ak.max(sv_exotic, axis=1), -1))

        # Save enhanced cache
        np.savez_compressed(
            os.path.join(DATA_DIR, "ftd_local_enhanced.npz"),
            met=met, rcav=rcav, sv_mass_max=sv_mass_max,
            has_bjet=has_bjet, sv_ntracks_max=sv_ntracks_max,
            sv_dlsig_max=sv_dlsig_max, rcav_exotic=rcav_exotic)

N = len(met)
sqrt_met = np.sqrt(met)
print(f"\nSelected {N:,} events for analysis")

# ---------------------------------------------------------------------------
# DEFINE CATEGORIES
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("EVENT CATEGORIZATION")
print("=" * 72)

# Category 1: SM-like (has b-jet AND SV mass < 5.5 GeV)
sm_like = has_bjet & (sv_mass_max < EXOTIC_MASS_MIN) & (sv_mass_max > 0)

# Category 2: No b-jet (cleaner for exotic search)
no_bjet = ~has_bjet

# Category 3: High-mass SV (mass > 5.5 GeV — NOT a B/D meson)
high_mass_sv = sv_mass_max > EXOTIC_MASS_MIN

# Category 4: Highly significant displacement
high_sig = sv_dlsig_max > SV_DLENSIG_HIGH

# Category 5: "Anomalous" = any non-SM indicator
anomalous = no_bjet | high_mass_sv | high_sig | (sv_ntracks_max > SV_NTRACKS_HIGH)

# Category 6: "Clean exotic" = no b-jet AND (high mass OR high sig)
clean_exotic = no_bjet & (high_mass_sv | high_sig)

# Category 7: Outer tracker + no b-jet
outer_nobjet = (rcav > 2.9) & no_bjet

# Category 8: Has exotic R_cav (SV not in B/D mass window)
has_exotic_rcav = rcav_exotic > SV_DXY_MIN

categories = [
    ("All events", np.ones(N, bool)),
    ("SM-like (b-jet + low-mass SV)", sm_like),
    ("No b-jet (medium WP)", no_bjet),
    ("High-mass SV (>5.5 GeV)", high_mass_sv),
    ("High dlenSig (>50)", high_sig),
    ("Any anomalous indicator", anomalous),
    ("Clean exotic (no-b AND high-mass/sig)", clean_exotic),
    ("Outer tracker + no b-jet", outer_nobjet),
    ("Exotic R_cav (non-B/D SV)", has_exotic_rcav),
]

print(f"\n  {'Category':45s} {'N':>8s}  {'%':>6s}  rho(sqrtE,R)  p-value")
print("  " + "-" * 90)

cat_results = []
for label, mask in categories:
    n_cat = mask.sum()
    frac = n_cat / N * 100 if N > 0 else 0

    if n_cat >= 30:
        # Use the appropriate R_cav
        if "Exotic R_cav" in label:
            r_cat = rcav_exotic[mask]
        else:
            r_cat = rcav[mask]
        rho, p = stats.spearmanr(sqrt_met[mask], r_cat)
        rho_lin = stats.spearmanr(met[mask], r_cat)[0]
        winner = "sqrt" if abs(rho) > abs(rho_lin) else "lin"
        print(f"  {label:45s} {n_cat:>8,}  {frac:>5.1f}%  {rho:+.4f}       {p:.2e} [{winner}]")
        cat_results.append({'label': label, 'n': n_cat, 'rho': rho, 'p': p,
                           'rho_lin': rho_lin, 'winner': winner, 'mask': mask})
    else:
        print(f"  {label:45s} {n_cat:>8,}  {frac:>5.1f}%  (too few)")
        cat_results.append({'label': label, 'n': n_cat, 'rho': 0, 'p': 1,
                           'rho_lin': 0, 'winner': '-', 'mask': mask})

# ---------------------------------------------------------------------------
# TAIL ANALYSIS WITH B-VETO
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("TAIL ANALYSIS WITH B-VETO")
print("=" * 72)

for r_cut in [3.0, 5.0, 10.0]:
    print(f"\n  R > {r_cut} cm:")
    for label, mask in [("All", np.ones(N, bool)),
                         ("No b-jet", no_bjet),
                         ("Anomalous", anomalous)]:
        r_sub = rcav[mask]
        met_sub = met[mask]
        n_above = (r_sub > r_cut).sum()
        n_total = mask.sum()
        frac = n_above / n_total * 100 if n_total > 0 else 0

        if n_above >= 10:
            rho = stats.spearmanr(sqrt_met[mask & (rcav > r_cut)],
                                   rcav[mask & (rcav > r_cut)])[0]
            print(f"    {label:15s}: {n_above:>7,}/{n_total:>8,} ({frac:.2f}%)  "
                  f"rho(tail)={rho:+.4f}")
        else:
            print(f"    {label:15s}: {n_above:>7,}/{n_total:>8,} ({frac:.2f}%)")

# ---------------------------------------------------------------------------
# CONDITIONAL TAIL WITH B-VETO
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("CONDITIONAL TAIL PROBABILITY (B-VETO APPLIED)")
print("=" * 72)

n_bins = 15
for cat_label, cat_mask in [("No b-jet", no_bjet), ("Anomalous", anomalous)]:
    met_cat = met[cat_mask]
    rcav_cat = rcav[cat_mask]

    if len(met_cat) < 100:
        continue

    met_pct = np.percentile(met_cat, np.linspace(0, 100, n_bins + 1))

    for r_cut in [3.0, 5.0, 10.0]:
        centers = []
        probs = []
        for i in range(n_bins):
            lo, hi = met_pct[i], met_pct[i+1]
            bin_mask = (met_cat >= lo) & (met_cat < hi)
            n_bin = bin_mask.sum()
            if n_bin < 30:
                continue
            n_above = (rcav_cat[bin_mask] > r_cut).sum()
            centers.append(np.sqrt(np.median(met_cat[bin_mask])))
            probs.append(n_above / n_bin)

        if len(centers) > 3 and any(p > 0 for p in probs):
            c = np.array(centers)
            p_arr = np.array(probs)
            valid = p_arr > 0
            if valid.sum() > 3:
                rho, pval = stats.spearmanr(c[valid], p_arr[valid])
                print(f"  {cat_label:15s} P(R>{r_cut:.0f}cm|MET): "
                      f"rho={rho:+.4f} (p={pval:.2e})")

# ---------------------------------------------------------------------------
# POWER LAW FITS BY CATEGORY
# ---------------------------------------------------------------------------
print("\n" + "=" * 72)
print("POWER LAW FITS BY CATEGORY")
print("=" * 72)

for cr in cat_results:
    if cr['n'] < 100:
        continue
    mask = cr['mask']
    if "Exotic R_cav" in cr['label']:
        r = rcav_exotic[mask]
    else:
        r = rcav[mask]
    m = met[mask]

    valid = (r > 0.01) & (m > 0)
    if valid.sum() < 50:
        continue

    sl, ic, rval, _, _ = stats.linregress(np.log10(m[valid]), np.log10(r[valid]))
    print(f"  {cr['label']:45s}: R ~ MET^{sl:.4f} (R2={rval**2:.4f}) "
          f"[FTD: 0.5]")

# ---------------------------------------------------------------------------
# PLOTTING
# ---------------------------------------------------------------------------
print("\nGenerating 9-panel figure...")

fig = plt.figure(figsize=(20, 18))
gs = gridspec.GridSpec(3, 3, hspace=0.35, wspace=0.3)

# Panel 1: Category bar chart
ax1 = fig.add_subplot(gs[0, 0])
labels = [cr['label'][:30] for cr in cat_results if cr['n'] > 0]
rhos = [cr['rho'] for cr in cat_results if cr['n'] > 0]
colors = ['green' if r > 0.02 else 'red' if r < -0.02 else 'gray' for r in rhos]
y_pos = range(len(labels))
ax1.barh(y_pos, rhos, color=colors, alpha=0.7)
ax1.axvline(0, color='black', lw=0.5)
ax1.set_yticks(y_pos)
ax1.set_yticklabels(labels, fontsize=7)
ax1.set_xlabel("Spearman rho(sqrt(MET), R_cav)")
ax1.set_title("1. Correlations by category")

# Panel 2: SV mass distribution
ax2 = fig.add_subplot(gs[0, 1])
valid_mass = sv_mass_max > 0
ax2.hist(sv_mass_max[valid_mass], bins=200, range=(0, 15), color='steelblue',
         alpha=0.7, density=True, label='All')
ax2.hist(sv_mass_max[valid_mass & no_bjet], bins=200, range=(0, 15),
         color='red', alpha=0.5, density=True, label='No b-jet')
ax2.axvline(B_MASS_MAX, color='orange', ls='--', label=f'B mass ({B_MASS_MAX})')
ax2.axvline(D_MASS_MAX, color='green', ls='--', label=f'D mass ({D_MASS_MAX})')
ax2.axvline(EXOTIC_MASS_MIN, color='red', ls='--', label=f'Exotic cut ({EXOTIC_MASS_MIN})')
ax2.set_xlabel("max(SV_mass) per event (GeV)")
ax2.set_ylabel("Density")
ax2.set_title("2. SV mass distribution")
ax2.legend(fontsize=7)
ax2.set_xlim(0, 15)

# Panel 3: Scatter — No b-jet events
ax3 = fig.add_subplot(gs[0, 2])
if no_bjet.sum() > 100:
    n_plot = min(30000, no_bjet.sum())
    idx = np.random.choice(np.where(no_bjet)[0], n_plot, replace=False)
    ax3.scatter(sqrt_met[idx], rcav[idx], s=0.5, alpha=0.15, c='navy')
    rho_nb = stats.spearmanr(sqrt_met[no_bjet], rcav[no_bjet])[0]
    ax3.set_title(f"3. No b-jet scatter (rho={rho_nb:.4f})")
ax3.set_xlabel("sqrt(MET)")
ax3.set_ylabel("R_cav (cm)")
ax3.set_ylim(0, 30)

# Panel 4: Scatter — Anomalous events
ax4 = fig.add_subplot(gs[1, 0])
if anomalous.sum() > 100:
    n_plot = min(30000, anomalous.sum())
    idx = np.random.choice(np.where(anomalous)[0], n_plot, replace=False)
    ax4.scatter(sqrt_met[idx], rcav[idx], s=0.5, alpha=0.15, c='darkred')
    rho_an = stats.spearmanr(sqrt_met[anomalous], rcav[anomalous])[0]
    ax4.set_title(f"4. Anomalous events (rho={rho_an:.4f})")
ax4.set_xlabel("sqrt(MET)")
ax4.set_ylabel("R_cav (cm)")
ax4.set_ylim(0, 30)

# Panel 5: Comparison — SM-like vs anomalous R_cav distributions
ax5 = fig.add_subplot(gs[1, 1])
if sm_like.sum() > 100 and anomalous.sum() > 100:
    ax5.hist(rcav[sm_like], bins=200, range=(0, 20), density=True,
             alpha=0.6, color='blue', label=f'SM-like (N={sm_like.sum():,})')
    ax5.hist(rcav[anomalous], bins=200, range=(0, 20), density=True,
             alpha=0.6, color='red', label=f'Anomalous (N={anomalous.sum():,})')
ax5.set_xlabel("R_cav (cm)")
ax5.set_ylabel("Density")
ax5.set_title("5. SM-like vs Anomalous R_cav")
ax5.legend(fontsize=7)
ax5.set_xlim(0, 20)

# Panel 6: Tail probability comparison
ax6 = fig.add_subplot(gs[1, 2])
r_range = np.logspace(-1.5, 2, 100)
for label, mask, color in [("All", np.ones(N, bool), 'black'),
                             ("SM-like", sm_like, 'blue'),
                             ("No b-jet", no_bjet, 'red'),
                             ("Anomalous", anomalous, 'green')]:
    r_sub = rcav[mask]
    survival = np.array([np.mean(r_sub > r) for r in r_range])
    ax6.semilogy(r_range, survival, label=label, color=color, lw=1.5)
ax6.set_xlabel("R_cav threshold (cm)")
ax6.set_ylabel("P(R > threshold)")
ax6.set_title("6. Survival functions by category")
ax6.legend(fontsize=7)
ax6.set_xlim(0.01, 50)
ax6.set_ylim(1e-5, 1)

# Panel 7: High-MET focus (no b-jet)
ax7 = fig.add_subplot(gs[2, 0])
he_nobjet = (met > 300) & no_bjet
if he_nobjet.sum() > 20:
    ax7.scatter(sqrt_met[he_nobjet], rcav[he_nobjet], s=2, alpha=0.3, c='darkgreen')
    if he_nobjet.sum() > 30:
        rho_he = stats.spearmanr(sqrt_met[he_nobjet], rcav[he_nobjet])[0]
        ax7.set_title(f"7. MET>300 + no b-jet (N={he_nobjet.sum():,}, rho={rho_he:.3f})")
    else:
        ax7.set_title(f"7. MET>300 + no b-jet (N={he_nobjet.sum():,})")
ax7.set_xlabel("sqrt(MET)")
ax7.set_ylabel("R_cav (cm)")

# Panel 8: dlenSig distribution
ax8 = fig.add_subplot(gs[2, 1])
valid_sig = sv_dlsig_max > 0
ax8.hist(sv_dlsig_max[valid_sig], bins=200, range=(0, 200),
         color='steelblue', alpha=0.7, density=True, label='All')
ax8.hist(sv_dlsig_max[valid_sig & no_bjet], bins=200, range=(0, 200),
         color='red', alpha=0.5, density=True, label='No b-jet')
ax8.axvline(SV_DLENSIG_HIGH, color='green', ls='--', label=f'Cut at {SV_DLENSIG_HIGH}')
ax8.set_xlabel("max(SV_dlenSig)")
ax8.set_ylabel("Density")
ax8.set_title("8. Decay length significance")
ax8.legend(fontsize=7)

# Panel 9: Summary
ax9 = fig.add_subplot(gs[2, 2])
ax9.axis('off')

# Find best and worst categories
best = max(cat_results, key=lambda c: c['rho'] if c['n'] > 30 else -999)
worst = min(cat_results, key=lambda c: c['rho'] if c['n'] > 30 else 999)

summary = f"""B-VETO ANALYSIS SUMMARY
{'='*40}
Events: {N:,}
B-tagged (medium): {has_bjet.sum():,} ({has_bjet.sum()/N*100:.1f}%)
No b-jet: {no_bjet.sum():,} ({no_bjet.sum()/N*100:.1f}%)
Anomalous: {anomalous.sum():,} ({anomalous.sum()/N*100:.1f}%)

CORRELATIONS rho(sqrt(MET), R_cav):
  All events:    {cat_results[0]['rho']:+.4f}
  SM-like:       {cat_results[1]['rho']:+.4f}
  No b-jet:      {cat_results[2]['rho']:+.4f}
  Anomalous:     {cat_results[5]['rho']:+.4f}

Best category:  {best['label'][:30]}
  rho = {best['rho']:+.4f}

KEY FINDING:
  B-veto {'IMPROVES' if abs(cat_results[2]['rho']) > abs(cat_results[0]['rho']) else 'does NOT improve'}
  correlation vs all-events.

  sqrt(MET) wins over linear in:
  {sum(1 for c in cat_results if c['winner']=='sqrt' and c['n']>30)}/{sum(1 for c in cat_results if c['n']>30)} categories"""

ax9.text(0.05, 0.95, summary, transform=ax9.transAxes,
         fontsize=8, verticalalignment='top', fontfamily='monospace',
         bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

fig.suptitle("FTD Cavitation: B-Veto + SV Discrimination Analysis",
             fontsize=14, fontweight='bold', y=0.98)

plot_path = os.path.join(OUTPUT_DIR, "ftd_cavitation_BVETO.png")
fig.savefig(plot_path, dpi=150, bbox_inches='tight')
print(f"\nSaved: {plot_path}")

print("\n" + "=" * 72)
print("B-VETO ANALYSIS COMPLETE")
print("=" * 72)
