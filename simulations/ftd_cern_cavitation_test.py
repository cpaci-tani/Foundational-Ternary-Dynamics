#!/usr/bin/env python3
"""
FTD Topological Cavitation Test — CERN Open Data Analysis
==========================================================

Tests the FTD prediction that displaced secondary vertices (SVs)
in high-MET events follow a GEOMETRIC threshold:

    R_cav ~ sqrt(E_MET)     [FTD: hard diagonal boundary]

vs the Standard Model expectation:

    R ~ exponential smear  [SM: random LLP decay]

Data source: CMS Run 2016G MET primary dataset (NanoAOD v9)
    Record: https://opendata.cern.ch/record/30526
    ~27M events, 13 TeV pp collisions

Variables extracted:
    MET_pt   — Missing Transverse Energy (GeV)
    SV_dxy   — 2D transverse flight distance of secondary vertices (cm)

Epistemic status: EXPLORATORY. This is a first-look analysis, not a
publication-ready result. No background subtraction, no systematic
uncertainties, no material veto removal (which is the point — we
intentionally keep raw unfiltered SV data).

Theory reference: CLAUDE.md §6.5 (Weak-Like Behavior),
                  FTD cavitation hypothesis (this script)
"""

import os
import sys
import time
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
PLOT_FILE = os.path.join(OUTPUT_DIR, "ftd_cavitation_golden_plot.png")

# CMS MET NanoAOD — smallest file from record 30526 (Run2016G)
# ~267 MB, ~1.6M events
XROOTD_URL = (
    "root://eospublic.cern.ch//eos/opendata/cms/Run2016G/MET/NANOAOD/"
    "UL2016_MiniAODv2_NanoAODv9-v1/1110000/"
    "498DAB22-324C-A744-9F94-B9403276F9FC.root"
)

# HTTP fallback (same file, EOS public HTTP gateway)
HTTP_URL = (
    "https://eospublic.cern.ch/eos/opendata/cms/Run2016G/MET/NANOAOD/"
    "UL2016_MiniAODv2_NanoAODv9-v1/1110000/"
    "498DAB22-324C-A744-9F94-B9403276F9FC.root"
)

# Analysis cuts
MET_CUT = 100.0       # GeV — FTD requires high energy to puncture vacuum
SV_DXY_MIN = 0.01     # cm — minimum displacement (reject PV-compatible)
SV_DXY_MAX = 100.0    # cm — sanity cut (detector radius)

# Branches to read (minimal footprint)
BRANCHES = ["MET_pt", "nSV", "SV_dxy"]

# ---------------------------------------------------------------------------
# Step 1: Access the data
# ---------------------------------------------------------------------------
def open_root_file():
    """Try multiple access methods for the CERN NanoAOD file."""
    import uproot
    import ssl
    import certifi

    local_path = os.path.join(OUTPUT_DIR, "cms_met_nanoaod_sample.root")

    # Check for cached local file first
    if os.path.exists(local_path):
        print("[1/4] Found cached local file")
        print(f"  {local_path}")
        return uproot.open(local_path)

    # Method 1: Direct HTTP with SSL verification disabled (CERN uses own CA)
    print("[1/4] Attempting HTTP access to CERN EOS...")
    try:
        import aiohttp
        f = uproot.open(
            HTTP_URL,
            timeout=120,
            handler=uproot.MultithreadedHTTPSource,
            num_workers=4,
        )
        print("  [OK] HTTP access successful")
        return f
    except Exception as e:
        print(f"  [FAIL] HTTP via uproot: {e}")

    # Method 2: Download with requests (SSL verify=False for CERN self-signed)
    print("[1/4] Downloading via requests (SSL relaxed for CERN CA)...")
    try:
        import requests
        import urllib3
        urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
        r = requests.get(HTTP_URL, stream=True, timeout=60, verify=False)
        r.raise_for_status()
        total = int(r.headers.get('content-length', 0))
        downloaded = 0
        with open(local_path, 'wb') as out:
            for chunk in r.iter_content(chunk_size=8192*16):
                out.write(chunk)
                downloaded += len(chunk)
                if total > 0:
                    pct = 100 * downloaded / total
                    if downloaded % (10*1024*1024) < 8192*16:
                        print(f"  Downloading: {pct:.1f}% ({downloaded/1e6:.0f}/{total/1e6:.0f} MB)")
        print(f"  [OK] Downloaded to {local_path}")
        return uproot.open(local_path)
    except Exception as e:
        print(f"  [FAIL] requests download: {e}")
        # Clean up partial download
        if os.path.exists(local_path):
            os.remove(local_path)

    # Method 3: Try CERN Open Data portal redirect URL
    portal_url = "https://opendata.cern.ch/record/30526/files/CMS_Run2016G_MET_NANOAOD_UL2016_MiniAODv2_NanoAODv9-v1_1110000_file_index.txt"
    print("[1/4] Trying CERN Open Data portal file index...")
    try:
        import requests
        r = requests.get(portal_url, timeout=30, verify=True)
        r.raise_for_status()
        # Parse first file URL from index
        first_url = r.text.strip().split('\n')[0].strip()
        print(f"  Got file URL: {first_url[:80]}...")
        r2 = requests.get(first_url, stream=True, timeout=60, verify=False)
        r2.raise_for_status()
        total = int(r2.headers.get('content-length', 0))
        downloaded = 0
        with open(local_path, 'wb') as out:
            for chunk in r2.iter_content(chunk_size=8192*16):
                out.write(chunk)
                downloaded += len(chunk)
                if total > 0 and downloaded % (10*1024*1024) < 8192*16:
                    print(f"  Downloading: {100*downloaded/total:.1f}%")
        print(f"  [OK] Downloaded to {local_path}")
        return uproot.open(local_path)
    except Exception as e:
        print(f"  [FAIL] Portal index: {e}")
        if os.path.exists(local_path):
            os.remove(local_path)

    # Method 4: CMS outreach sample (smaller, guaranteed accessible)
    outreach_url = "https://cms-opendata-analyses.web.cern.ch/cms-opendata-analyses/NanoAODRun1Examples/SingleMu/CMS_Run2012B_SingleMu_NANOAOD_v1_sample.root"
    print("[1/4] Trying CMS outreach sample (fallback)...")
    outreach_path = os.path.join(OUTPUT_DIR, "cms_outreach_sample.root")
    try:
        import requests
        r = requests.get(outreach_url, stream=True, timeout=60, verify=True)
        r.raise_for_status()
        total = int(r.headers.get('content-length', 0))
        downloaded = 0
        with open(outreach_path, 'wb') as out:
            for chunk in r.iter_content(chunk_size=8192*16):
                out.write(chunk)
                downloaded += len(chunk)
        print(f"  [OK] Downloaded outreach sample ({downloaded/1e6:.1f} MB)")
        return uproot.open(outreach_path)
    except Exception as e:
        print(f"  [FAIL] Outreach sample: {e}")

    print("\n[FATAL] Cannot access CERN data.")
    print("  Try manually downloading a NanoAOD file from:")
    print("  https://opendata.cern.ch/record/30526")
    print(f"  Save as: {local_path}")
    sys.exit(1)


# ---------------------------------------------------------------------------
# Step 2: Extract MET and SV data
# ---------------------------------------------------------------------------
def extract_data(root_file):
    """
    Read MET_pt and SV_dxy from NanoAOD TTree.

    NanoAOD structure:
        Events TTree
        ├── MET_pt      : float per event
        ├── nSV          : int per event (number of secondary vertices)
        └── SV_dxy       : float[] per event (jagged array)

    For each event with MET > cut and at least one SV:
        - Take the MAXIMUM SV_dxy (furthest displacement)
        - This is R_cav in the FTD interpretation
    """
    import awkward as ak

    print("[2/4] Reading branches from Events tree...")
    tree = root_file["Events"]
    n_total = tree.num_entries
    print(f"  Total events in file: {n_total:,}")

    # Read in chunks to manage memory (NanoAOD can be large)
    met_list = []
    rcav_list = []
    n_with_sv = 0
    n_pass_met = 0

    chunk_size = 500_000
    for start in range(0, n_total, chunk_size):
        stop = min(start + chunk_size, n_total)
        print(f"  Processing events {start:,}–{stop:,}...")

        data = tree.arrays(BRANCHES, entry_start=start, entry_stop=stop)

        met = data["MET_pt"]
        sv_dxy = data["SV_dxy"]
        n_sv = data["nSV"]

        # Filter: MET > cut
        met_mask = met > MET_CUT
        n_pass_met += int(ak.sum(met_mask))

        # Filter: at least one SV
        sv_mask = n_sv > 0
        combined_mask = met_mask & sv_mask

        if ak.sum(combined_mask) == 0:
            continue

        met_sel = met[combined_mask]
        sv_sel = sv_dxy[combined_mask]

        # For events with multiple SVs, take the MAX displacement
        # This is R_cav: the furthest drift of the cavitation wave
        max_sv = ak.max(sv_sel, axis=1)

        # Apply SV quality cuts (no material veto — intentionally raw!)
        sv_quality = (max_sv > SV_DXY_MIN) & (max_sv < SV_DXY_MAX)

        met_final = ak.to_numpy(met_sel[sv_quality])
        rcav_final = ak.to_numpy(max_sv[sv_quality])

        met_list.append(met_final)
        rcav_list.append(rcav_final)
        n_with_sv += len(met_final)

    met_all = np.concatenate(met_list) if met_list else np.array([])
    rcav_all = np.concatenate(rcav_list) if rcav_list else np.array([])

    print(f"\n  Summary:")
    print(f"    Total events:           {n_total:,}")
    print(f"    Events with MET > {MET_CUT} GeV: {n_pass_met:,}")
    print(f"    Events with MET + SV:   {n_with_sv:,}")
    print(f"    After quality cuts:     {len(met_all):,}")

    return met_all, rcav_all


# ---------------------------------------------------------------------------
# Step 3: Generate the Golden Plot
# ---------------------------------------------------------------------------
def make_golden_plot(met, rcav):
    """
    The "Golden Plot": R_cav vs sqrt(E_MET)

    FTD prediction: Hard diagonal boundary (R_cav ~ sqrtMET)
    SM prediction:  Exponential smear (random decay positions)
    """
    print("[3/4] Generating the Golden Plot...")

    sqrt_met = np.sqrt(met)

    fig, axes = plt.subplots(1, 3, figsize=(21, 7))

    # --- Panel 1: Scatter plot with linear fit ---
    ax1 = axes[0]
    ax1.scatter(sqrt_met, rcav, s=1, alpha=0.15, c='steelblue', rasterized=True)

    # Linear regression: R_cav = a * sqrt(MET) + b
    # Use only data where both are finite
    mask = np.isfinite(sqrt_met) & np.isfinite(rcav) & (rcav > 0)
    if np.sum(mask) > 10:
        coeffs = np.polyfit(sqrt_met[mask], rcav[mask], 1)
        x_fit = np.linspace(np.min(sqrt_met[mask]), np.max(sqrt_met[mask]), 100)
        y_fit = np.polyval(coeffs, x_fit)
        ax1.plot(x_fit, y_fit, 'r-', linewidth=2, label=f'Linear fit: R = {coeffs[0]:.4f}sqrtMET + {coeffs[1]:.3f}')

        # Compute R² for the linear fit
        y_pred = np.polyval(coeffs, sqrt_met[mask])
        ss_res = np.sum((rcav[mask] - y_pred)**2)
        ss_tot = np.sum((rcav[mask] - np.mean(rcav[mask]))**2)
        r_squared = 1 - ss_res / ss_tot
        ax1.text(0.05, 0.95, f'R² = {r_squared:.4f}\nN = {np.sum(mask):,}',
                 transform=ax1.transAxes, fontsize=10, va='top',
                 bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    ax1.set_xlabel('sqrt(MET) [sqrtGeV]', fontsize=12)
    ax1.set_ylabel('R_cav (max SV_dxy) [cm]', fontsize=12)
    ax1.set_title('FTD Cavitation Test: R_cav vs sqrtMET', fontsize=13, fontweight='bold')
    ax1.legend(loc='lower right', fontsize=9)
    ax1.set_xlim(left=np.sqrt(MET_CUT))

    # --- Panel 2: 2D histogram (density) ---
    ax2 = axes[1]
    h = ax2.hist2d(sqrt_met[mask], rcav[mask],
                   bins=[80, 80],
                   norm=mcolors.LogNorm(),
                   cmap='inferno',
                   rasterized=True)
    plt.colorbar(h[3], ax=ax2, label='Events (log scale)')

    # Overlay the upper envelope (95th percentile in bins)
    n_bins = 30
    bin_edges = np.linspace(np.min(sqrt_met[mask]), np.max(sqrt_met[mask]), n_bins + 1)
    bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])
    percentiles_95 = []
    percentiles_50 = []
    for i in range(n_bins):
        in_bin = (sqrt_met[mask] >= bin_edges[i]) & (sqrt_met[mask] < bin_edges[i+1])
        if np.sum(in_bin) > 10:
            percentiles_95.append(np.percentile(rcav[mask][in_bin], 95))
            percentiles_50.append(np.percentile(rcav[mask][in_bin], 50))
        else:
            percentiles_95.append(np.nan)
            percentiles_50.append(np.nan)

    ax2.plot(bin_centers, percentiles_95, 'c-', linewidth=2, label='95th percentile')
    ax2.plot(bin_centers, percentiles_50, 'w--', linewidth=1.5, label='Median')
    ax2.set_xlabel('sqrt(MET) [sqrtGeV]', fontsize=12)
    ax2.set_ylabel('R_cav (max SV_dxy) [cm]', fontsize=12)
    ax2.set_title('Event Density (log scale)', fontsize=13, fontweight='bold')
    ax2.legend(loc='upper left', fontsize=9)

    # --- Panel 3: R_cav distribution ---
    ax3 = axes[2]
    ax3.hist(rcav[mask], bins=100, log=True, color='steelblue', alpha=0.7,
             edgecolor='navy', linewidth=0.3)
    ax3.set_xlabel('R_cav (max SV_dxy) [cm]', fontsize=12)
    ax3.set_ylabel('Events (log scale)', fontsize=12)
    ax3.set_title('SV Displacement Distribution', fontsize=13, fontweight='bold')
    ax3.axvline(x=2.5, color='red', linestyle='--', alpha=0.5, label='Beam pipe (~2.5 cm)')
    ax3.axvline(x=4.4, color='orange', linestyle='--', alpha=0.5, label='Pixel layer 1 (~4.4 cm)')
    ax3.axvline(x=7.3, color='green', linestyle='--', alpha=0.5, label='Pixel layer 2 (~7.3 cm)')
    ax3.legend(fontsize=8)

    fig.suptitle(
        'CMS Run2016G MET Dataset — Raw Secondary Vertex Displacements (No Material Veto)\n'
        'FTD predicts: hard boundary R_cav ~ sqrtMET  |  SM predicts: exponential smear',
        fontsize=11, style='italic', y=1.02
    )

    plt.tight_layout()
    plt.savefig(PLOT_FILE, dpi=200, bbox_inches='tight')
    print(f"  [OK] Plot saved to: {PLOT_FILE}")

    return sqrt_met[mask], rcav[mask], coeffs if np.sum(mask) > 10 else None


# ---------------------------------------------------------------------------
# Step 4: Statistical analysis
# ---------------------------------------------------------------------------
def analyze_results(sqrt_met, rcav, coeffs):
    """
    Test FTD vs SM predictions quantitatively.

    FTD: R_cav = a * sqrtMET + b  (linear in sqrtMET -> hard boundary)
    SM:  R_cav independent of MET (exponential decay, random position)
    """
    print("[4/4] Statistical Analysis...")
    print()

    # Test 1: Correlation between sqrtMET and R_cav
    from numpy import corrcoef
    rho = corrcoef(sqrt_met, rcav)[0, 1]
    print(f"  Pearson correlation rho(sqrtMET, R_cav) = {rho:.6f}")
    if abs(rho) > 0.1:
        print(f"  -> Significant correlation detected (|rho| > 0.1)")
        print(f"  -> This is UNEXPECTED under SM (SV position should be")
        print(f"    independent of MET for random LLP decays)")
    else:
        print(f"  -> Weak correlation (|rho| < 0.1)")
        print(f"  -> Consistent with SM random decay positions")

    # Test 2: Compare linear vs constant model
    if coeffs is not None:
        slope, intercept = coeffs
        print(f"\n  Linear fit: R_cav = {slope:.6f} × sqrtMET + {intercept:.4f}")
        print(f"  Slope significance: {slope:.6f}")

    # Test 3: Check for sharp upper boundary
    # In FTD, R_cav should have a HARD cutoff at the geometric threshold
    # In SM, the distribution should have an exponential tail
    print(f"\n  R_cav statistics:")
    print(f"    Mean:   {np.mean(rcav):.4f} cm")
    print(f"    Median: {np.median(rcav):.4f} cm")
    print(f"    Std:    {np.std(rcav):.4f} cm")
    print(f"    95th %%: {np.percentile(rcav, 95):.4f} cm")
    print(f"    99th %%: {np.percentile(rcav, 99):.4f} cm")
    print(f"    Max:    {np.max(rcav):.4f} cm")

    # Test 4: Kurtosis of R_cav distribution
    # Heavy tails (SM: exponential) -> positive kurtosis
    # Sharp cutoff (FTD: geometric) -> negative kurtosis
    from scipy.stats import kurtosis as calc_kurtosis, skew
    try:
        k = calc_kurtosis(rcav)
        s = skew(rcav)
        print(f"\n  Kurtosis: {k:.4f}  (positive = heavy tails, negative = sharp cutoff)")
        print(f"  Skewness: {s:.4f}  (positive = right tail)")
    except ImportError:
        # scipy not available, compute manually
        m = np.mean(rcav)
        s_std = np.std(rcav)
        k = np.mean(((rcav - m) / s_std) ** 4) - 3
        s = np.mean(((rcav - m) / s_std) ** 3)
        print(f"\n  Excess kurtosis: {k:.4f}")
        print(f"  Skewness: {s:.4f}")

    # Test 5: Binned envelope analysis — does the 95th percentile scale with sqrtMET?
    n_bins = 20
    bin_edges = np.linspace(np.min(sqrt_met), np.max(sqrt_met), n_bins + 1)
    bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])
    p95_vals = []
    for i in range(n_bins):
        in_bin = (sqrt_met >= bin_edges[i]) & (sqrt_met < bin_edges[i + 1])
        if np.sum(in_bin) > 20:
            p95_vals.append(np.percentile(rcav[in_bin], 95))
        else:
            p95_vals.append(np.nan)

    p95_arr = np.array(p95_vals)
    valid = np.isfinite(p95_arr)
    if np.sum(valid) > 5:
        envelope_coeffs = np.polyfit(bin_centers[valid], p95_arr[valid], 1)
        print(f"\n  95th-percentile envelope fit:")
        print(f"    R_95 = {envelope_coeffs[0]:.6f} × sqrtMET + {envelope_coeffs[1]:.4f}")
        if envelope_coeffs[0] > 0:
            print(f"  -> POSITIVE slope: upper boundary INCREASES with sqrtMET")
            print(f"  -> This is what FTD predicts (cavitation radius grows with energy)")
        else:
            print(f"  -> Flat or negative slope: no energy-dependent boundary")
            print(f"  -> Consistent with SM random decay")

    print("\n" + "=" * 60)
    print("  INTERPRETATION GUIDE")
    print("=" * 60)
    print("""
  IF you see a hard diagonal boundary on the scatter plot
  (clear edge where events pile up along R ~ sqrtMET):
    -> Consistent with FTD cavitation hypothesis
    -> The vacuum has a geometric re-manifestation threshold

  IF you see a diffuse cloud with no energy-dependent boundary
  (random scatter, exponential tail in R):
    -> Consistent with SM random LLP decay
    -> No evidence for geometric vacuum structure

  CAVEATS:
    - This is RAW data (no material veto removed — intentional)
    - Peaks at ~2.5, ~4.4, ~7.3 cm are detector structures
      (beam pipe, pixel layers), NOT physics
    - A proper analysis would need: background subtraction,
      MC comparison, systematic uncertainties, blinding
    - This is EXPLORATORY, not publication-ready
    """)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    print("=" * 60)
    print("  FTD Topological Cavitation Test")
    print("  CMS Run2016G MET NanoAOD (13 TeV)")
    print("=" * 60)
    print()

    t0 = time.time()

    # Step 1: Access data
    root_file = open_root_file()

    # Step 2: Extract MET and SV data
    met, rcav = extract_data(root_file)

    if len(met) == 0:
        print("\n[ERROR] No events passed selection. Cannot generate plot.")
        sys.exit(1)

    # Step 3: Generate the Golden Plot
    sqrt_met, rcav_sel, coeffs = make_golden_plot(met, rcav)

    # Step 4: Statistical analysis
    analyze_results(sqrt_met, rcav_sel, coeffs)

    elapsed = time.time() - t0
    print(f"\n  Total runtime: {elapsed:.1f} s")
    print(f"  Plot: {PLOT_FILE}")
