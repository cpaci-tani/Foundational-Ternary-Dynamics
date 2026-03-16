#!/usr/bin/env python3
"""
FTD CERN Monte Carlo Comparison Analysis
=========================================
Compare CMS MET data against Standard Model MC predictions.

MC samples (NanoAODv9, 2016 UL):
  - WJetsToLNu    (record 69747) — dominant MET background
  - ZJetsToNuNu   (records 74908/74910) — irreducible MET background
  - QCD_HT1000to1500 (record 63081) — fake MET from mismeasurement

Strategy:
  1. Load MC and data from XRootD (or local cache)
  2. Apply identical MET>200 + SV selection
  3. Build combined SM prediction (shape-only, normalize to data)
  4. Compute data-MC residual in (MET, R_cav) space
  5. Test whether residual correlates with FTD cavitation prediction

Usage:
  # In CERN Docker container:
  FTD_MODE=docker python3 ftd_cern_mc_comparison.py

  # Local (uses cached .npz if available):
  python3 ftd_cern_mc_comparison.py
"""

import os, sys, time
import numpy as np
import warnings
warnings.filterwarnings('ignore')

# --------------- MODE ---------------
MODE = os.environ.get("FTD_MODE", "local")
print(f"=== FTD CERN MC Comparison Analysis ===")
print(f"Mode: {MODE}")

# --------------- MC file lists ---------------
MC_SAMPLES = {
    "WJetsToLNu": {
        "label": r"W+jets $\rightarrow l\nu$",
        "color": "#3498db",
        "xsec": 61526.7,  # pb (NLO), CMS standard
        "files": [
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/00702195-E707-3743-8BBA-57EB9DEE1DBA.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/0A1A30A2-740B-2A48-8D3B-A458026E93EA.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/DDCE98B4-9142-D041-A3BF-1F81DECC09D2.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/1F96440D-3010-734C-AB94-E0D52DEB0730.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/AD7B7E81-0901-3346-89EB-6B808E2D8B56.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/270000/CC80A34F-10EC-9B43-A56B-3DEE96459E7F.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/06169F6B-C5AE-1646-87E4-F6613C6046C7.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/0B81B07D-9A8E-1348-B71A-DEFDFFDDB3AC.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/427C2439-1B97-5D41-9A6F-EA921E59BA86.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/6392ADA8-07B2-EB47-9AC3-A6720587D4F0.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/71CBA5AA-9925-1A43-AFDA-98B26632B95D.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/280000/BB2D42DC-ECA1-6B46-8A74-F597A314E051.root",
        ],
    },
    "ZJetsToNuNu_200toInf": {
        "label": r"Z$\rightarrow\nu\nu$ (pT>200)",
        "color": "#e74c3c",
        "xsec": 18.01,  # pb (NLO for Zpt>200)
        "files": [
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/ZJetsToNuNu_Zpt-200toInf_BPSFilter_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2530000/0449F39D-84E0-594F-B099-C0BCD5DA460E.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/ZJetsToNuNu_Zpt-200toInf_BPSFilter_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2530000/D67ED84E-76F2-8647-BAA8-E0D722B6BB08.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/ZJetsToNuNu_Zpt-200toInf_BPSFilter_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2530000/DF7597EF-EF61-DE46-8D91-9FD962E28B83.root",
        ],
    },
    "ZJetsToNuNu_100to200": {
        "label": r"Z$\rightarrow\nu\nu$ (100<pT<200)",
        "color": "#e67e22",
        "xsec": 93.79,  # pb (NLO for Zpt 100-200)
        "files": [
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/ZJetsToNuNu_Zpt-100to200_BPSFilter_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2530000/028D3BA4-4D05-D24A-8DEA-23D972301254.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/ZJetsToNuNu_Zpt-100to200_BPSFilter_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2530000/1B6363A2-6C90-0546-AF10-6AF33F24B0F6.root",
        ],
    },
    "QCD_HT1000to1500": {
        "label": "QCD multijet",
        "color": "#2ecc71",
        "xsec": 1005.0,  # pb (LO)
        "files": [
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2520000/47D78D8D-BE86-4444-89E0-A58088C6C553.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/2520000/79AE80B9-E1C8-CB45-B9C1-B8022F9349D1.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/260000/0E459E78-3787-9D46-B969-B52DB5512DDB.root",
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/260000/0F811E5D-50F6-C641-BC03-9BE2574CC541.root",
        ],
    },
    "QCD_HT700to1000": {
        "label": "QCD HT700-1000",
        "color": "#27ae60",
        "xsec": 6334.0,  # pb (LO)
        "files": [
            "root://eospublic.cern.ch//eos/opendata/cms/mc/RunIISummer20UL16NanoAODv9/QCD_HT700to1000_TuneCH3_13TeV-madgraphMLM-herwig7/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/80000/64FAD11D-E6E7-574E-9F11-A5C82A832B90.root",
        ],
    },
}

# Branches to read
BRANCHES_BASIC = ["MET_pt", "nSV", "SV_dxy"]
BRANCHES_EXTENDED = BRANCHES_BASIC + [
    "SV_mass", "SV_ntracks", "SV_dlenSig", "SV_dlen",
    "nJet", "Jet_btagDeepFlavB",
]

def load_mc_from_xrootd(sample_name, sample_info, max_files=None):
    """Load MC sample from XRootD, return arrays."""
    import uproot
    import awkward as ak

    files = sample_info["files"]
    if max_files:
        files = files[:max_files]

    all_met = []
    all_rcav = []
    all_sv_mass_max = []
    all_has_bjet = []
    all_sv_dlsig_max = []
    n_total = 0
    n_selected = 0

    for fi, fpath in enumerate(files):
        print(f"  [{sample_name}] File {fi+1}/{len(files)}: {fpath.split('/')[-1][:30]}...")
        try:
            f = uproot.open(fpath, timeout=120)
            tree = f["Events"]
            n_in_file = tree.num_entries
            n_total += n_in_file

            # Read in chunks
            chunk_size = 500000
            for start in range(0, n_in_file, chunk_size):
                stop = min(start + chunk_size, n_in_file)
                arrays = tree.arrays(BRANCHES_EXTENDED, entry_start=start, entry_stop=stop)

                met = np.array(arrays["MET_pt"])
                nsv = np.array(arrays["nSV"])

                # MET > 200 cut
                met_mask = met > 200
                if np.sum(met_mask) == 0:
                    continue

                met_sel = met[met_mask]

                # SV_dxy — max per event
                sv_dxy = arrays["SV_dxy"][met_mask]
                rcav = np.zeros(len(met_sel))
                for j in range(len(met_sel)):
                    dxy_arr = np.array(sv_dxy[j])
                    if len(dxy_arr) > 0:
                        rcav[j] = np.max(dxy_arr)

                # Require at least 1 SV
                nsv_sel = nsv[met_mask]
                sv_mask = nsv_sel > 0
                if np.sum(sv_mask) == 0:
                    continue

                met_final = met_sel[sv_mask]
                rcav_final = rcav[sv_mask]

                # SV mass max
                sv_mass = arrays["SV_mass"][met_mask][sv_mask]
                sv_mass_max = np.zeros(len(met_final))
                for j in range(len(met_final)):
                    m = np.array(sv_mass[j])
                    if len(m) > 0:
                        sv_mass_max[j] = np.max(m)

                # B-jet veto
                btag = arrays["Jet_btagDeepFlavB"][met_mask][sv_mask]
                has_bjet = np.zeros(len(met_final), dtype=bool)
                for j in range(len(met_final)):
                    b = np.array(btag[j])
                    if len(b) > 0:
                        has_bjet[j] = np.max(b) > 0.2770

                # SV dlenSig max
                sv_dlsig = arrays["SV_dlenSig"][met_mask][sv_mask]
                sv_dlsig_max = np.zeros(len(met_final))
                for j in range(len(met_final)):
                    d = np.array(sv_dlsig[j])
                    if len(d) > 0:
                        sv_dlsig_max[j] = np.max(d)

                all_met.append(met_final)
                all_rcav.append(rcav_final)
                all_sv_mass_max.append(sv_mass_max)
                all_has_bjet.append(has_bjet)
                all_sv_dlsig_max.append(sv_dlsig_max)
                n_selected += len(met_final)

        except Exception as e:
            print(f"    ERROR: {e}")
            continue

    if n_selected == 0:
        return None

    result = {
        "met": np.concatenate(all_met),
        "rcav": np.concatenate(all_rcav),
        "sv_mass_max": np.concatenate(all_sv_mass_max),
        "has_bjet": np.concatenate(all_has_bjet),
        "sv_dlsig_max": np.concatenate(all_sv_dlsig_max),
        "n_total": n_total,
        "n_selected": n_selected,
        "xsec": sample_info["xsec"],
    }
    print(f"  [{sample_name}] {n_total:,} total -> {n_selected:,} selected ({100*n_selected/max(1,n_total):.2f}%)")
    return result


def load_data_cached():
    """Load data from cached npz files."""
    # Try enhanced cache first (has SV mass, btag, etc.)
    enhanced_path = os.path.join(os.path.dirname(__file__), "ftd_full_enhanced.npz")
    basic_path = os.path.join(os.path.dirname(__file__), "ftd_full_extracted.npz")

    if os.path.exists(enhanced_path):
        print(f"Loading enhanced data cache: {enhanced_path}")
        d = np.load(enhanced_path, allow_pickle=True)
        result = {"met": d["met"], "rcav": d["rcav"]}
        # Try to load extended fields
        for key in ["sv_mass_max", "has_bjet", "sv_dlsig_max"]:
            if key in d:
                result[key] = d[key]
        print(f"  {len(result['met']):,} events loaded")
        return result

    if os.path.exists(basic_path):
        print(f"Loading basic data cache: {basic_path}")
        d = np.load(basic_path)
        result = {"met": d["met"], "rcav": d["rcav"]}
        print(f"  {len(result['met']):,} events loaded")
        return result

    raise FileNotFoundError("No cached data found. Run ftd_cern_bveto_analysis.py in Docker first.")


def load_data_from_xrootd():
    """Load CMS data from XRootD (in Docker mode)."""
    import uproot
    import awkward as ak

    base = "root://eospublic.cern.ch//eos/opendata/cms/Run2016G/MET/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1/130000/"
    # File list from prior analysis
    file_ids = [
        "5A0A2D97-B8C6-9B44-A87A-C1F1E53B4B29",
        "89275E87-72CA-EE4D-BA09-44BA28D4BF6B",
        "B3A28C9D-7459-C54F-8238-C37E98E13C0E",
        "E16AEB7E-6CAB-0F42-9234-1AFD55AE2D88",
        "CE4E1F5C-7A6D-F94B-B9FB-4C4F1A2B900A",
        "FE025478-3014-3A41-A2CC-F5B8B175E72D",
        "57BCAC6D-00B7-FA47-A511-B847E47E5BA9",
        "A19E15DB-7D06-5244-B1DD-06A443778A3E",
        "20E1CFCE-D48C-CA40-AD4F-94BE7EA34135",
        "70E3FA43-89D7-854F-A228-E799AA2B3F10",
        "EBC1D27B-4A6C-5C4E-8BE5-68C8BF1BDE17",
        "D77E2099-CECE-5B42-8E62-69AB7FE69B3D",
        "24A26925-C1F2-D044-8A55-FC74DE424431",
        "47FE7C52-F14B-FD41-8C4A-92D93F3C00F2",
        "F6B15CAB-6EAD-5E4A-B98A-3BF1ADCCE797",
        "DF9DB2A4-A9C3-AE43-AEF1-DF46B30EA3AA",
        "A5FFD22D-0C96-4B44-848D-DC7D0EB6B352",
    ]
    files = [f"{base}{fid}.root" for fid in file_ids]
    return files


def run_analysis(data, mc_results):
    """Core analysis: Data vs MC comparison + FTD residual test."""
    from scipy import stats

    print("\n" + "="*70)
    print("DATA vs MONTE CARLO COMPARISON")
    print("="*70)

    met_data = data["met"]
    rcav_data = data["rcav"]

    # ---- 1. MET spectrum comparison ----
    met_bins = np.logspace(np.log10(200), np.log10(3000), 31)
    data_hist, _ = np.histogram(met_data, bins=met_bins)

    # Stack MC (shape-normalized to data total)
    mc_hists = {}
    mc_weights = {}
    for name, mc in mc_results.items():
        if mc is None:
            continue
        h, _ = np.histogram(mc["met"], bins=met_bins)
        # Weight by cross-section / n_total (relative weight)
        w = mc["xsec"] / mc["n_total"]
        mc_hists[name] = h * w
        mc_weights[name] = w
        print(f"\n  {name}: {mc['n_selected']:,} events (xsec={mc['xsec']:.1f} pb, eff={100*mc['n_selected']/mc['n_total']:.3f}%)")

    # Sum MC and normalize to data
    mc_sum = sum(mc_hists.values())
    if np.sum(mc_sum) > 0:
        mc_scale = np.sum(data_hist) / np.sum(mc_sum)
        mc_sum_norm = mc_sum * mc_scale
        for name in mc_hists:
            mc_hists[name] = mc_hists[name] * mc_scale
    else:
        mc_sum_norm = np.zeros_like(data_hist)
        mc_scale = 1.0

    # Data/MC ratio
    ratio = np.where(mc_sum_norm > 0, data_hist / mc_sum_norm, 1.0)
    ratio_err = np.where(mc_sum_norm > 0, np.sqrt(data_hist) / mc_sum_norm, 0.0)

    print(f"\n  Data: {len(met_data):,} events")
    print(f"  MC scale factor: {mc_scale:.4f}")

    # ---- 2. R_cav distribution: data vs MC ----
    rcav_bins = np.linspace(0, 30, 61)
    data_rcav_hist, _ = np.histogram(rcav_data, bins=rcav_bins)

    mc_rcav_hists = {}
    for name, mc in mc_results.items():
        if mc is None:
            continue
        h, _ = np.histogram(mc["rcav"], bins=rcav_bins)
        mc_rcav_hists[name] = h * mc_weights[name] * mc_scale

    mc_rcav_sum = sum(mc_rcav_hists.values())
    rcav_ratio = np.where(mc_rcav_sum > 0, data_rcav_hist / mc_rcav_sum, 1.0)

    # ---- 3. 2D (MET, R_cav) residual ----
    met_bins_2d = np.array([200, 300, 400, 600, 1000, 3000])
    rcav_bins_2d = np.array([0, 1, 2, 4, 8, 15, 30])

    data_2d, _, _ = np.histogram2d(met_data, rcav_data,
                                    bins=[met_bins_2d, rcav_bins_2d])

    mc_2d = np.zeros_like(data_2d)
    for name, mc in mc_results.items():
        if mc is None:
            continue
        h, _, _ = np.histogram2d(mc["met"], mc["rcav"],
                                  bins=[met_bins_2d, rcav_bins_2d])
        mc_2d += h * mc_weights[name] * mc_scale

    residual_2d = np.where(mc_2d > 0, (data_2d - mc_2d) / np.sqrt(mc_2d), 0)
    excess_2d = np.where(mc_2d > 0, (data_2d - mc_2d) / mc_2d, 0)

    print(f"\n  2D residual (sigma) max: {np.max(residual_2d):.1f}")
    print(f"  2D residual (sigma) min: {np.min(residual_2d):.1f}")

    # ---- 4. FTD correlation test on residual ----
    # For each MET bin, compute excess in R_cav tail
    # FTD predicts: excess grows with sqrt(MET)
    met_centers_2d = (met_bins_2d[:-1] + met_bins_2d[1:]) / 2
    sqrt_met_2d = np.sqrt(met_centers_2d)

    # Tail excess: fraction of events at large R_cav (>4cm)
    tail_idx = np.where(rcav_bins_2d[:-1] >= 4)[0]  # bins with R>4cm
    data_tail = np.sum(data_2d[:, tail_idx], axis=1)
    mc_tail = np.sum(mc_2d[:, tail_idx], axis=1)
    data_total_per_met = np.sum(data_2d, axis=1)
    mc_total_per_met = np.sum(mc_2d, axis=1)

    # Tail fraction difference
    data_tail_frac = np.where(data_total_per_met > 0,
                               data_tail / data_total_per_met, 0)
    mc_tail_frac = np.where(mc_total_per_met > 0,
                             mc_tail / mc_total_per_met, 0)
    tail_excess = data_tail_frac - mc_tail_frac

    valid = (data_total_per_met > 100) & (mc_total_per_met > 10)
    if np.sum(valid) >= 3:
        rho_tail, p_tail = stats.spearmanr(sqrt_met_2d[valid], tail_excess[valid])
        print(f"\n  FTD tail-excess vs sqrt(MET): rho={rho_tail:.4f}, p={p_tail:.4e}")
    else:
        rho_tail, p_tail = 0, 1
        print(f"\n  FTD tail-excess vs sqrt(MET): insufficient bins")

    # ---- 5. B-veto category comparison ----
    bveto_results = {}
    if "has_bjet" in data:
        print(f"\n--- B-veto category analysis ---")
        has_bjet_data = data["has_bjet"]
        no_bjet_data = ~has_bjet_data

        for cat_name, cat_mask_data in [("no_bjet", no_bjet_data), ("has_bjet", has_bjet_data)]:
            # Data
            md = met_data[cat_mask_data]
            rd = rcav_data[cat_mask_data]

            # Combined MC for this category
            mc_met_cat = []
            mc_rcav_cat = []
            mc_w_cat = []
            for name, mc in mc_results.items():
                if mc is None or "has_bjet" not in mc:
                    continue
                if cat_name == "no_bjet":
                    cm = ~mc["has_bjet"]
                else:
                    cm = mc["has_bjet"]
                mc_met_cat.append(mc["met"][cm])
                mc_rcav_cat.append(mc["rcav"][cm])
                mc_w_cat.extend([mc_weights[name] * mc_scale] * np.sum(cm))

            if len(mc_met_cat) > 0 and len(md) > 100:
                mc_met_c = np.concatenate(mc_met_cat)
                mc_rcav_c = np.concatenate(mc_rcav_cat)

                # KS test on R_cav distributions
                ks_stat, ks_p = stats.ks_2samp(rd, mc_rcav_c)

                # Correlation in data vs MC
                if len(md) > 10:
                    rho_d, _ = stats.spearmanr(np.sqrt(md), rd)
                else:
                    rho_d = 0
                if len(mc_met_c) > 10:
                    rho_mc, _ = stats.spearmanr(np.sqrt(mc_met_c), mc_rcav_c)
                else:
                    rho_mc = 0

                bveto_results[cat_name] = {
                    "n_data": len(md), "n_mc": len(mc_met_c),
                    "ks_stat": ks_stat, "ks_p": ks_p,
                    "rho_data": rho_d, "rho_mc": rho_mc,
                    "rho_diff": rho_d - rho_mc,
                }
                print(f"  {cat_name}: data={len(md):,}, MC={len(mc_met_c):,}")
                print(f"    KS stat={ks_stat:.4f}, p={ks_p:.4e}")
                print(f"    rho(data)={rho_d:.4f}, rho(MC)={rho_mc:.4f}, diff={rho_d-rho_mc:.4f}")

    # ---- 6. High-dlenSig category (strongest FTD signal) ----
    dlsig_results = {}
    if "sv_dlsig_max" in data:
        print(f"\n--- High decay-length-significance analysis ---")
        for dlsig_cut in [10, 30, 50]:
            high_dlsig_data = data["sv_dlsig_max"] > dlsig_cut
            md = met_data[high_dlsig_data]
            rd = rcav_data[high_dlsig_data]

            mc_met_dl = []
            mc_rcav_dl = []
            for name, mc in mc_results.items():
                if mc is None or "sv_dlsig_max" not in mc:
                    continue
                cm = mc["sv_dlsig_max"] > dlsig_cut
                if np.sum(cm) > 0:
                    mc_met_dl.append(mc["met"][cm])
                    mc_rcav_dl.append(mc["rcav"][cm])

            rho_d = rho_mc = 0
            ks_stat = ks_p = 0
            if len(md) > 10:
                rho_d, _ = stats.spearmanr(np.sqrt(md), rd)
            if len(mc_met_dl) > 0:
                mc_met_c = np.concatenate(mc_met_dl)
                mc_rcav_c = np.concatenate(mc_rcav_dl)
                if len(mc_met_c) > 10:
                    rho_mc, _ = stats.spearmanr(np.sqrt(mc_met_c), mc_rcav_c)
                    ks_stat, ks_p = stats.ks_2samp(rd, mc_rcav_c)

            dlsig_results[dlsig_cut] = {
                "n_data": len(md),
                "n_mc": len(mc_met_dl[0]) if len(mc_met_dl) > 0 else 0,
                "rho_data": rho_d, "rho_mc": rho_mc,
                "rho_diff": rho_d - rho_mc,
                "ks_stat": ks_stat, "ks_p": ks_p,
            }
            print(f"  dlenSig>{dlsig_cut}: data={len(md):,}, rho(data)={rho_d:.4f}, rho(MC)={rho_mc:.4f}, diff={rho_d-rho_mc:.4f}")

    # ---- 7. Conditional tail probability data vs MC ----
    print(f"\n--- Conditional tail probability P(R>5cm | MET) ---")
    met_edges_cond = np.array([200, 250, 300, 400, 500, 700, 1000, 1500, 3000])
    met_centers_cond = (met_edges_cond[:-1] + met_edges_cond[1:]) / 2
    R_CUT = 5.0

    ptail_data = []
    ptail_mc = []
    for i in range(len(met_edges_cond)-1):
        # Data
        mask_d = (met_data >= met_edges_cond[i]) & (met_data < met_edges_cond[i+1])
        n_d = np.sum(mask_d)
        n_tail_d = np.sum(rcav_data[mask_d] > R_CUT) if n_d > 0 else 0
        ptail_data.append(n_tail_d / max(n_d, 1))

        # MC
        mc_in_bin = 0
        mc_tail = 0
        for name, mc in mc_results.items():
            if mc is None:
                continue
            mask_mc = (mc["met"] >= met_edges_cond[i]) & (mc["met"] < met_edges_cond[i+1])
            n_mc = np.sum(mask_mc)
            mc_in_bin += n_mc * mc_weights[name] * mc_scale
            mc_tail += np.sum(mc["rcav"][mask_mc] > R_CUT) * mc_weights[name] * mc_scale
        ptail_mc.append(mc_tail / max(mc_in_bin, 1e-10))

    ptail_data = np.array(ptail_data)
    ptail_mc = np.array(ptail_mc)

    valid_cond = ptail_data > 0
    if np.sum(valid_cond) >= 3:
        rho_cond_data, _ = stats.spearmanr(met_centers_cond[valid_cond], ptail_data[valid_cond])
        rho_cond_mc, _ = stats.spearmanr(met_centers_cond[valid_cond], ptail_mc[valid_cond])
        print(f"  P(R>{R_CUT}cm|MET) rho with MET:")
        print(f"    Data:  {rho_cond_data:.4f}")
        print(f"    MC:    {rho_cond_mc:.4f}")
        print(f"    Diff:  {rho_cond_data - rho_cond_mc:.4f}")

        # FTD predicts tail probability grows with sqrt(MET)
        # Check power-law fit: P_tail ~ MET^beta
        valid_fit = (ptail_data > 0) & (met_centers_cond > 0)
        if np.sum(valid_fit) >= 3:
            log_met = np.log(met_centers_cond[valid_fit])
            log_ptail_data = np.log(ptail_data[valid_fit])
            slope_data, intercept_data = np.polyfit(log_met, log_ptail_data, 1)

            if np.any(ptail_mc[valid_fit] > 0):
                valid_mc_fit = valid_fit & (ptail_mc > 0)
                if np.sum(valid_mc_fit) >= 3:
                    log_ptail_mc = np.log(ptail_mc[valid_mc_fit])
                    slope_mc, _ = np.polyfit(np.log(met_centers_cond[valid_mc_fit]),
                                             log_ptail_mc, 1)
                else:
                    slope_mc = 0
            else:
                slope_mc = 0

            print(f"\n  Power-law exponent P_tail ~ MET^beta:")
            print(f"    Data beta:  {slope_data:.4f}")
            print(f"    MC beta:    {slope_mc:.4f}")
            print(f"    FTD prediction: beta = 0.5 (R_cav ~ sqrt(MET))")
    else:
        rho_cond_data = rho_cond_mc = slope_data = slope_mc = 0

    # ---- STORE RESULTS ----
    results = {
        "met_bins": met_bins,
        "data_hist": data_hist,
        "mc_hists": mc_hists,
        "mc_sum_norm": mc_sum_norm,
        "ratio": ratio,
        "ratio_err": ratio_err,
        "rcav_bins": rcav_bins,
        "data_rcav_hist": data_rcav_hist,
        "mc_rcav_hists": mc_rcav_hists,
        "mc_rcav_sum": mc_rcav_sum,
        "rcav_ratio": rcav_ratio,
        "met_bins_2d": met_bins_2d,
        "rcav_bins_2d": rcav_bins_2d,
        "residual_2d": residual_2d,
        "excess_2d": excess_2d,
        "data_2d": data_2d,
        "mc_2d": mc_2d,
        "sqrt_met_2d": sqrt_met_2d,
        "tail_excess": tail_excess,
        "rho_tail": rho_tail,
        "p_tail": p_tail,
        "bveto_results": bveto_results,
        "dlsig_results": dlsig_results,
        "met_centers_cond": met_centers_cond,
        "ptail_data": ptail_data,
        "ptail_mc": ptail_mc,
        "mc_scale": mc_scale,
    }
    return results


def make_plots(results, mc_results):
    """Generate publication-quality comparison plots."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib.gridspec import GridSpec

    fig = plt.figure(figsize=(20, 24))
    gs = GridSpec(4, 3, figure=fig, hspace=0.35, wspace=0.3)
    fig.suptitle("CMS MET Data vs SM Monte Carlo + FTD Cavitation Test",
                 fontsize=16, fontweight='bold', y=0.98)

    met_bins = results["met_bins"]
    met_centers = (met_bins[:-1] + met_bins[1:]) / 2
    rcav_bins = results["rcav_bins"]
    rcav_centers = (rcav_bins[:-1] + rcav_bins[1:]) / 2

    # ---- Panel 1: MET spectrum (data vs MC stack) ----
    ax1 = fig.add_subplot(gs[0, 0])
    # MC stack
    bottom = np.zeros_like(results["data_hist"], dtype=float)
    for name, mc in mc_results.items():
        if mc is None or name not in results["mc_hists"]:
            continue
        h = results["mc_hists"][name]
        ax1.bar(met_centers, h, width=np.diff(met_bins), bottom=bottom,
                alpha=0.7, label=MC_SAMPLES[name]["label"],
                color=MC_SAMPLES[name]["color"], edgecolor='none')
        bottom += h
    # Data points
    ax1.errorbar(met_centers, results["data_hist"],
                 yerr=np.sqrt(results["data_hist"]),
                 fmt='ko', markersize=3, label='CMS Data', zorder=10)
    ax1.set_xscale('log')
    ax1.set_yscale('log')
    ax1.set_xlabel('MET [GeV]')
    ax1.set_ylabel('Events / bin')
    ax1.set_title('MET Spectrum: Data vs MC')
    ax1.legend(fontsize=7, loc='upper right')
    ax1.set_xlim(200, 3000)

    # ---- Panel 2: Data/MC ratio ----
    ax2 = fig.add_subplot(gs[0, 1])
    valid = results["mc_sum_norm"] > 0
    ax2.errorbar(met_centers[valid], results["ratio"][valid],
                 yerr=results["ratio_err"][valid],
                 fmt='ko', markersize=4)
    ax2.axhline(1.0, color='r', linestyle='--', linewidth=1)
    ax2.fill_between([200, 3000], 0.8, 1.2, alpha=0.1, color='green')
    ax2.set_xscale('log')
    ax2.set_xlabel('MET [GeV]')
    ax2.set_ylabel('Data / MC')
    ax2.set_title('Data/MC Ratio (MET)')
    ax2.set_ylim(0, 3)
    ax2.set_xlim(200, 3000)

    # ---- Panel 3: R_cav distribution data vs MC ----
    ax3 = fig.add_subplot(gs[0, 2])
    bottom_r = np.zeros_like(results["data_rcav_hist"], dtype=float)
    for name, mc in mc_results.items():
        if mc is None or name not in results["mc_rcav_hists"]:
            continue
        h = results["mc_rcav_hists"][name]
        ax3.bar(rcav_centers, h, width=np.diff(rcav_bins), bottom=bottom_r,
                alpha=0.7, label=MC_SAMPLES[name]["label"],
                color=MC_SAMPLES[name]["color"], edgecolor='none')
        bottom_r += h
    ax3.errorbar(rcav_centers, results["data_rcav_hist"],
                 yerr=np.sqrt(np.maximum(results["data_rcav_hist"], 1)),
                 fmt='ko', markersize=3, label='CMS Data', zorder=10)
    ax3.set_xlabel('R_cav = max(SV_dxy) [cm]')
    ax3.set_ylabel('Events / bin')
    ax3.set_title('SV Displacement: Data vs MC')
    ax3.set_yscale('log')
    ax3.legend(fontsize=7)
    ax3.set_xlim(0, 15)

    # ---- Panel 4: Data/MC ratio in R_cav ----
    ax4 = fig.add_subplot(gs[1, 0])
    valid_r = results["mc_rcav_sum"] > 10
    rcav_ratio_err = np.where(valid_r,
                               np.sqrt(results["data_rcav_hist"]) / results["mc_rcav_sum"],
                               0)
    ax4.errorbar(rcav_centers[valid_r], results["rcav_ratio"][valid_r],
                 yerr=rcav_ratio_err[valid_r],
                 fmt='ko', markersize=4)
    ax4.axhline(1.0, color='r', linestyle='--', linewidth=1)
    ax4.fill_between([0, 30], 0.5, 1.5, alpha=0.1, color='green')
    ax4.set_xlabel('R_cav [cm]')
    ax4.set_ylabel('Data / MC')
    ax4.set_title('Data/MC Ratio (R_cav)')
    ax4.set_xlim(0, 15)
    ax4.set_ylim(0, 5)

    # ---- Panel 5: 2D residual heatmap ----
    ax5 = fig.add_subplot(gs[1, 1])
    res = results["residual_2d"]
    im = ax5.imshow(res.T, origin='lower', aspect='auto',
                     cmap='RdBu_r', vmin=-5, vmax=5,
                     extent=[0, len(results["met_bins_2d"])-1,
                             0, len(results["rcav_bins_2d"])-1])
    # Label axes with bin edges
    met_labels = [f"{int(x)}" for x in results["met_bins_2d"]]
    rcav_labels = [f"{int(x)}" for x in results["rcav_bins_2d"]]
    ax5.set_xticks(range(len(met_labels)))
    ax5.set_xticklabels(met_labels, fontsize=7, rotation=45)
    ax5.set_yticks(range(len(rcav_labels)))
    ax5.set_yticklabels(rcav_labels, fontsize=7)
    ax5.set_xlabel('MET [GeV]')
    ax5.set_ylabel('R_cav [cm]')
    ax5.set_title('(Data-MC)/sqrt(MC) [sigma]')
    plt.colorbar(im, ax=ax5, label='Significance')

    # Annotate cells
    for i in range(res.shape[0]):
        for j in range(res.shape[1]):
            val = res[i, j]
            if abs(val) > 0.5:
                color = 'white' if abs(val) > 3 else 'black'
                ax5.text(i, j, f'{val:.1f}', ha='center', va='center',
                        fontsize=7, color=color)

    # ---- Panel 6: FTD tail excess vs sqrt(MET) ----
    ax6 = fig.add_subplot(gs[1, 2])
    valid_te = results["tail_excess"] != 0
    if np.sum(valid_te) > 0:
        ax6.plot(results["sqrt_met_2d"][valid_te], results["tail_excess"][valid_te],
                 'ro-', markersize=8, linewidth=2, label='Data - MC tail excess')
        ax6.axhline(0, color='gray', linestyle='--')
        ax6.set_xlabel('sqrt(MET) [GeV^0.5]')
        ax6.set_ylabel('Tail fraction excess (data - MC)')
        rho_t = results["rho_tail"]
        ax6.set_title(f'FTD Test: Tail excess vs sqrt(MET)\nrho={rho_t:.4f}')
        ax6.legend(fontsize=8)
    else:
        ax6.text(0.5, 0.5, 'Insufficient data', ha='center', va='center',
                transform=ax6.transAxes)

    # ---- Panel 7: Conditional tail probability ----
    ax7 = fig.add_subplot(gs[2, 0])
    mc_cond = results["met_centers_cond"]
    ptail_d = results["ptail_data"]
    ptail_m = results["ptail_mc"]
    valid_p = ptail_d > 0
    if np.sum(valid_p) > 0:
        ax7.plot(mc_cond[valid_p], ptail_d[valid_p], 'ko-', markersize=6,
                 linewidth=2, label='Data', zorder=10)
        valid_pm = ptail_m > 0
        if np.sum(valid_pm) > 0:
            ax7.plot(mc_cond[valid_pm], ptail_m[valid_pm], 'rs--', markersize=6,
                     linewidth=1.5, label='MC')
        ax7.set_xscale('log')
        ax7.set_yscale('log')
        ax7.set_xlabel('MET [GeV]')
        ax7.set_ylabel('P(R_cav > 5cm | MET)')
        ax7.set_title('Conditional Tail Probability')
        ax7.legend()

    # ---- Panel 8: Excess fraction (data-MC)/MC vs R_cav ----
    ax8 = fig.add_subplot(gs[2, 1])
    excess_frac = np.where(results["mc_rcav_sum"] > 10,
                            (results["data_rcav_hist"] - results["mc_rcav_sum"]) / results["mc_rcav_sum"],
                            0)
    valid_ef = results["mc_rcav_sum"] > 10
    ax8.bar(rcav_centers[valid_ef], excess_frac[valid_ef],
            width=np.diff(rcav_bins)[valid_ef], alpha=0.7,
            color='steelblue', edgecolor='navy')
    ax8.axhline(0, color='red', linestyle='--')
    ax8.set_xlabel('R_cav [cm]')
    ax8.set_ylabel('(Data - MC) / MC')
    ax8.set_title('Fractional Excess vs R_cav')
    ax8.set_xlim(0, 15)

    # ---- Panel 9: dlenSig category comparison ----
    ax9 = fig.add_subplot(gs[2, 2])
    if results.get("dlsig_results"):
        cuts = sorted(results["dlsig_results"].keys())
        rho_data_vals = [results["dlsig_results"][c]["rho_data"] for c in cuts]
        rho_mc_vals = [results["dlsig_results"][c]["rho_mc"] for c in cuts]
        x = np.arange(len(cuts))
        width = 0.35
        ax9.bar(x - width/2, rho_data_vals, width, label='Data', color='navy', alpha=0.7)
        ax9.bar(x + width/2, rho_mc_vals, width, label='MC', color='red', alpha=0.7)
        ax9.set_xticks(x)
        ax9.set_xticklabels([f'>{c}' for c in cuts])
        ax9.set_xlabel('dlenSig cut')
        ax9.set_ylabel('Spearman rho(sqrt(MET), R_cav)')
        ax9.set_title('Correlation by dlenSig Category')
        ax9.legend()
        ax9.axhline(0, color='gray', linestyle='--')
    else:
        ax9.text(0.5, 0.5, 'No dlenSig data', ha='center', va='center',
                transform=ax9.transAxes)

    # ---- Panel 10: B-veto category comparison ----
    ax10 = fig.add_subplot(gs[3, 0])
    if results.get("bveto_results"):
        cats = list(results["bveto_results"].keys())
        rho_d = [results["bveto_results"][c]["rho_data"] for c in cats]
        rho_m = [results["bveto_results"][c]["rho_mc"] for c in cats]
        x = np.arange(len(cats))
        width = 0.35
        ax10.bar(x - width/2, rho_d, width, label='Data', color='navy', alpha=0.7)
        ax10.bar(x + width/2, rho_m, width, label='MC', color='red', alpha=0.7)
        ax10.set_xticks(x)
        ax10.set_xticklabels(cats)
        ax10.set_xlabel('Category')
        ax10.set_ylabel('Spearman rho')
        ax10.set_title('Correlation by B-veto Category')
        ax10.legend()
        ax10.axhline(0, color='gray', linestyle='--')
    else:
        ax10.text(0.5, 0.5, 'No b-veto data', ha='center', va='center',
                transform=ax10.transAxes)

    # ---- Panel 11: Scorecard ----
    ax11 = fig.add_subplot(gs[3, 1])
    ax11.axis('off')
    scorecard_text = "DATA vs MC SCORECARD\n" + "="*40 + "\n\n"

    rho_t = results.get("rho_tail", 0)
    scorecard_text += f"1. FTD tail excess vs sqrt(MET):\n"
    scorecard_text += f"   rho = {rho_t:+.4f}"
    scorecard_text += f" {'[FTD+]' if rho_t > 0.1 else '[Weak]'}\n\n"

    if results.get("dlsig_results"):
        for cut in sorted(results["dlsig_results"].keys()):
            dr = results["dlsig_results"][cut]
            diff = dr["rho_diff"]
            scorecard_text += f"2. dlenSig>{cut}: rho_diff = {diff:+.4f}"
            scorecard_text += f" {'[FTD+]' if diff > 0.02 else '[SM]'}\n"

    scorecard_text += f"\n3. Conditional tail P(R>5|MET):\n"
    scorecard_text += f"   Data growth vs MC growth"

    if results.get("bveto_results"):
        for cat in results["bveto_results"]:
            br = results["bveto_results"][cat]
            scorecard_text += f"\n4. {cat}: KS={br['ks_stat']:.4f} (p={br['ks_p']:.2e})"

    ax11.text(0.05, 0.95, scorecard_text, transform=ax11.transAxes,
              fontsize=8, verticalalignment='top', fontfamily='monospace',
              bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))

    # ---- Panel 12: Summary ----
    ax12 = fig.add_subplot(gs[3, 2])
    ax12.axis('off')
    summary = "INTERPRETATION\n" + "="*40 + "\n\n"
    summary += "FTD Prediction:\n"
    summary += "  R_cav ~ sqrt(E_MET)\n"
    summary += "  Excess at large R grows with MET\n\n"
    summary += "Key diagnostic:\n"
    summary += "  rho(data) - rho(MC) > 0 in signal\n"
    summary += "  regions indicates excess\n"
    summary += "  correlation NOT explained by SM.\n\n"
    summary += "MC samples used:\n"
    for name, mc in mc_results.items():
        if mc is not None:
            summary += f"  {name}: {mc['n_selected']:,} evts\n"

    ax12.text(0.05, 0.95, summary, transform=ax12.transAxes,
              fontsize=8, verticalalignment='top', fontfamily='monospace',
              bbox=dict(boxstyle='round', facecolor='lightcyan', alpha=0.8))

    outpath = os.path.join(os.path.dirname(__file__), "ftd_cavitation_MC_COMPARISON.png")
    fig.savefig(outpath, dpi=150, bbox_inches='tight')
    print(f"\n  Plot saved: {outpath}")
    plt.close(fig)


# ================ MAIN ================
if __name__ == "__main__":
    t0 = time.time()

    mc_cache_path = os.path.join(os.path.dirname(__file__), "ftd_mc_cache.npz")

    if MODE == "docker":
        # ---- Load MC from XRootD ----
        mc_results = {}
        for name, info in MC_SAMPLES.items():
            print(f"\n--- Loading MC: {name} ---")
            mc_results[name] = load_mc_from_xrootd(name, info)

        # Save MC cache
        mc_save = {}
        for name, mc in mc_results.items():
            if mc is not None:
                for key, val in mc.items():
                    mc_save[f"{name}__{key}"] = val
        np.savez_compressed(mc_cache_path, **mc_save)
        print(f"\nMC cache saved: {mc_cache_path}")

        # ---- Load data from XRootD ----
        print(f"\n--- Loading CMS Data ---")
        # Use cached enhanced data if available
        data = load_data_cached()

    else:
        # ---- Local mode: use cached MC + data ----
        if os.path.exists(mc_cache_path):
            print(f"Loading MC cache: {mc_cache_path}")
            mc_raw = np.load(mc_cache_path, allow_pickle=True)
            mc_results = {}
            for name in MC_SAMPLES:
                key_met = f"{name}__met"
                if key_met in mc_raw:
                    mc_results[name] = {
                        "met": mc_raw[f"{name}__met"],
                        "rcav": mc_raw[f"{name}__rcav"],
                        "n_total": int(mc_raw[f"{name}__n_total"]),
                        "n_selected": int(mc_raw[f"{name}__n_selected"]),
                        "xsec": float(mc_raw[f"{name}__xsec"]),
                    }
                    # Load optional fields
                    for opt in ["sv_mass_max", "has_bjet", "sv_dlsig_max"]:
                        k = f"{name}__{opt}"
                        if k in mc_raw:
                            mc_results[name][opt] = mc_raw[k]
                    print(f"  {name}: {mc_results[name]['n_selected']:,} events")
                else:
                    mc_results[name] = None
                    print(f"  {name}: not in cache")
        else:
            print("ERROR: No MC cache found. Run in Docker mode first:")
            print("  FTD_MODE=docker python3 ftd_cern_mc_comparison.py")
            sys.exit(1)

        data = load_data_cached()

    # ---- Run analysis ----
    from scipy import stats  # ensure available
    results = run_analysis(data, mc_results)

    # ---- Make plots ----
    make_plots(results, mc_results)

    # ---- Write results file ----
    results_path = os.path.join(os.path.dirname(__file__), "ftd_mc_comparison_results.txt")
    with open(results_path, 'w') as f:
        f.write("FTD CERN MC Comparison Results\n")
        f.write(f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Mode: {MODE}\n\n")

        f.write("MC Samples:\n")
        for name, mc in mc_results.items():
            if mc is not None:
                f.write(f"  {name}: {mc['n_selected']:,} selected / {mc['n_total']:,} total "
                       f"(xsec={mc['xsec']:.1f} pb)\n")
        f.write(f"\nData: {len(data['met']):,} events\n")
        f.write(f"MC scale factor: {results['mc_scale']:.4f}\n\n")

        f.write("KEY RESULTS:\n")
        f.write(f"  FTD tail excess vs sqrt(MET): rho={results['rho_tail']:.4f}, p={results['p_tail']:.4e}\n")

        if results.get("dlsig_results"):
            f.write("\n  Decay-length-significance categories:\n")
            for cut in sorted(results["dlsig_results"].keys()):
                dr = results["dlsig_results"][cut]
                f.write(f"    dlenSig>{cut}: rho_data={dr['rho_data']:.4f}, "
                       f"rho_MC={dr['rho_mc']:.4f}, diff={dr['rho_diff']:.4f}\n")

        if results.get("bveto_results"):
            f.write("\n  B-veto categories:\n")
            for cat in results["bveto_results"]:
                br = results["bveto_results"][cat]
                f.write(f"    {cat}: rho_data={br['rho_data']:.4f}, "
                       f"rho_MC={br['rho_mc']:.4f}, diff={br['rho_diff']:.4f}, "
                       f"KS={br['ks_stat']:.4f} (p={br['ks_p']:.2e})\n")

    print(f"\n  Results saved: {results_path}")
    print(f"\nTotal time: {time.time()-t0:.1f}s")
    print("Done.")
