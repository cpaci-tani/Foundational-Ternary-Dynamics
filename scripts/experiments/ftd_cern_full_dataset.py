#!/usr/bin/env python3
"""
FTD Topological Cavitation Test -- FULL DATASET
================================================

Processes ALL 17 files from CMS Run2016G MET NanoAOD (record 30526).
~27M events, 13 TeV pp collisions, ~24.4 GB total.

Strategy: 4 parallel downloads + pipeline extraction to maximize throughput
on CERN's slow (~200 KB/s per connection) servers.
"""

import os
import sys
import time
import subprocess
import threading
import queue
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from concurrent.futures import ThreadPoolExecutor, as_completed

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
PLOT_FILE = os.path.join(OUTPUT_DIR, "ftd_cavitation_FULL_27M.png")
DATA_CACHE = os.path.join(OUTPUT_DIR, "ftd_full_extracted.npz")
TEMP_DIR = os.path.join(OUTPUT_DIR, "_temp_downloads")

MET_CUT = 100.0
SV_DXY_MIN = 0.01
SV_DXY_MAX = 100.0
BRANCHES = ["MET_pt", "nSV", "SV_dxy"]

# How many parallel downloads
N_PARALLEL = 4

BASE_URL = "https://eospublic.cern.ch/eos/opendata/cms/Run2016G/MET/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1"

# (path, size_MB) -- sorted by size (smallest first for fast early results)
FILES = [
    ("270000/D7A2449E-B767-D846-811E-235735B47141.root", 111),
    ("1110000/498DAB22-324C-A744-9F94-B9403276F9FC.root", 255),
    ("270000/0485102E-7DF7-8641-BBBB-E76473C455E2.root", 543),
    ("270000/1ED9314C-5C6A-6F4F-8467-BFD8A8024781.root", 1014),
    ("270000/0EC589CC-D2FA-1446-9A1E-74ED4A779498.root", 1320),
    ("270000/13526412-8C47-4A49-A11C-CABB7F1C4AF7.root", 1267),
    ("270000/178B2AD7-9C20-9545-925B-5233A1C03B9D.root", 1409),
    ("270000/8E86E692-B324-2F49-9853-5335D371CDA3.root", 1628),
    ("270000/9FD09B43-C8E5-384B-9894-50160B606CEE.root", 1633),
    ("270000/6A4F07DD-F1D1-164F-B509-AFBA9877D6D5.root", 1646),
    ("270000/A52A2155-40A4-AB4B-8FCC-60E16E1A6A84.root", 1665),
    ("270000/51DD2D9D-496F-5E4D-BA61-5BBD952631CA.root", 1687),
    ("270000/0AADFEBE-F652-FF44-B0A4-5954D894D308.root", 1691),
    ("270000/D937BCB9-279D-7741-B42C-5CC44E2B0E13.root", 1852),
    ("270000/CE2F8BC4-D806-8041-A857-54CD636BCEC7.root", 2103),
    ("270000/88F48B67-EADD-C14F-BC13-19D5E52C28CC.root", 2124),
    ("270000/7036A078-EB71-3C49-BB25-BBE7F191A606.root", 2459),
]

# We already have this file cached from the single-file analysis
CACHED_SAMPLE = os.path.join(OUTPUT_DIR, "cms_met_nanoaod_sample.root")
CACHED_SAMPLE_FNAME = "1110000/498DAB22-324C-A744-9F94-B9403276F9FC.root"


def download_file(url, dest, expected_mb=0):
    """Download with curl -k (CERN self-signed cert).
    Returns (success, dest_path).
    """
    cmd = [
        "curl", "-L", "-k",
        "--connect-timeout", "60",
        "--speed-limit", "10000",     # fail if < 10 KB/s
        "--speed-time", "120",        # ... for 120 seconds
        "--retry", "3",
        "--retry-delay", "10",
        "-o", dest, url
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0 or not os.path.exists(dest):
        return False
    if expected_mb > 0:
        actual_mb = os.path.getsize(dest) / 1e6
        if actual_mb < expected_mb * 0.5:
            return False
    return True


def extract_from_file(filepath):
    """Extract MET and max-SV-dxy from a single NanoAOD file."""
    import uproot
    import awkward as ak

    f = uproot.open(filepath)
    tree = f["Events"]
    n_total = tree.num_entries

    met_list = []
    rcav_list = []
    n_pass = 0

    chunk_size = 500_000
    for start in range(0, n_total, chunk_size):
        stop = min(start + chunk_size, n_total)
        data = tree.arrays(BRANCHES, entry_start=start, entry_stop=stop)

        met = data["MET_pt"]
        sv_dxy = data["SV_dxy"]
        n_sv = data["nSV"]

        met_mask = met > MET_CUT
        sv_mask = n_sv > 0
        combined = met_mask & sv_mask

        if ak.sum(combined) == 0:
            continue

        met_sel = met[combined]
        sv_sel = sv_dxy[combined]
        max_sv = ak.max(sv_sel, axis=1)

        quality = (max_sv > SV_DXY_MIN) & (max_sv < SV_DXY_MAX)

        met_list.append(ak.to_numpy(met_sel[quality]))
        rcav_list.append(ak.to_numpy(max_sv[quality]))
        n_pass += int(ak.sum(quality))

    f.close()

    met_arr = np.concatenate(met_list) if met_list else np.array([])
    rcav_arr = np.concatenate(rcav_list) if rcav_list else np.array([])
    return n_total, n_pass, met_arr, rcav_arr


def download_and_extract(idx, fname, size_mb):
    """Download a file, extract data, delete file. Thread-safe."""
    short = fname.split("/")[-1][:20]
    temp_path = os.path.join(TEMP_DIR, f"file_{idx}.root")

    # Check if this is the cached sample
    if fname == CACHED_SAMPLE_FNAME and os.path.exists(CACHED_SAMPLE):
        print(f"  [{idx+1}/{len(FILES)}] {short} -- USING CACHED COPY")
        try:
            n_total, n_pass, met, rcav = extract_from_file(CACHED_SAMPLE)
            return (idx, True, n_total, n_pass, met, rcav, 0, short)
        except Exception as e:
            print(f"  [{idx+1}/{len(FILES)}] {short} -- CACHE ERROR: {e}")
            # Fall through to download

    # Download
    url = f"{BASE_URL}/{fname}"
    t0 = time.time()
    ok = download_file(url, temp_path, expected_mb=size_mb)
    dl_time = time.time() - t0

    if not ok:
        print(f"  [{idx+1}/{len(FILES)}] {short} -- DOWNLOAD FAILED ({dl_time:.0f}s)")
        if os.path.exists(temp_path):
            os.remove(temp_path)
        return (idx, False, 0, 0, np.array([]), np.array([]), dl_time, short)

    actual_mb = os.path.getsize(temp_path) / 1e6
    speed = actual_mb / dl_time if dl_time > 0 else 0
    print(f"  [{idx+1}/{len(FILES)}] {short} -- downloaded ({actual_mb:.0f} MB, {dl_time:.0f}s, {speed:.1f} MB/s)")

    # Extract
    t1 = time.time()
    try:
        n_total, n_pass, met, rcav = extract_from_file(temp_path)
        ext_time = time.time() - t1
        print(f"  [{idx+1}/{len(FILES)}] {short} -- extracted ({n_total:,} events, {n_pass:,} selected, {ext_time:.1f}s)")
    except Exception as e:
        print(f"  [{idx+1}/{len(FILES)}] {short} -- EXTRACT ERROR: {e}")
        n_total, n_pass, met, rcav = 0, 0, np.array([]), np.array([])

    # Clean up
    if os.path.exists(temp_path):
        os.remove(temp_path)

    return (idx, True, n_total, n_pass, met, rcav, dl_time, short)


def process_all_files():
    """Download and extract all files with parallel downloads."""
    # Check for cached extracted data
    if os.path.exists(DATA_CACHE):
        print(f"[CACHE] Loading pre-extracted data from {DATA_CACHE}")
        d = np.load(DATA_CACHE)
        return d["met"], d["rcav"]

    os.makedirs(TEMP_DIR, exist_ok=True)

    total_size = sum(s for _, s in FILES)
    print(f"Processing {len(FILES)} files ({total_size/1000:.1f} GB total)")
    print(f"Using {N_PARALLEL} parallel downloads")
    print("=" * 70)

    all_met = []
    all_rcav = []
    total_events = 0
    total_selected = 0
    completed = 0

    with ThreadPoolExecutor(max_workers=N_PARALLEL) as executor:
        futures = {}
        for i, (fname, size_mb) in enumerate(FILES):
            future = executor.submit(download_and_extract, i, fname, size_mb)
            futures[future] = (i, fname, size_mb)

        for future in as_completed(futures):
            idx, ok, n_total, n_pass, met, rcav, dl_time, short = future.result()
            completed += 1

            if ok and len(met) > 0:
                all_met.append(met)
                all_rcav.append(rcav)
                total_events += n_total
                total_selected += n_pass

            cum_selected = sum(len(m) for m in all_met)
            print(f"  >> Progress: {completed}/{len(FILES)} files done, "
                  f"{total_events:,} events, {cum_selected:,} selected")
            print()

            # Save intermediate results every 3 files
            if completed % 3 == 0 and all_met:
                met_tmp = np.concatenate(all_met)
                rcav_tmp = np.concatenate(all_rcav)
                np.savez_compressed(DATA_CACHE + f".partial_{completed}",
                                    met=met_tmp, rcav=rcav_tmp)
                print(f"  >> Saved intermediate results ({completed} files, {len(met_tmp):,} events)")

    print("\n" + "=" * 70)
    print(f"COMPLETE: {total_events:,} total events, {total_selected:,} selected")

    # Clean up temp dir
    if os.path.exists(TEMP_DIR):
        import shutil
        shutil.rmtree(TEMP_DIR, ignore_errors=True)

    met_all = np.concatenate(all_met) if all_met else np.array([])
    rcav_all = np.concatenate(all_rcav) if all_rcav else np.array([])

    # Cache final results
    np.savez_compressed(DATA_CACHE, met=met_all, rcav=rcav_all)
    print(f"Cached extracted data to {DATA_CACHE}")

    # Clean up partial saves
    import glob
    for f in glob.glob(DATA_CACHE + ".partial_*"):
        os.remove(f)

    return met_all, rcav_all


def make_full_plot(met, rcav):
    """Generate the full-dataset golden plot."""
    n_events = len(met)
    label_str = f"~{n_events/1e6:.1f}M" if n_events > 1e6 else f"{n_events:,}"

    print(f"\nGenerating the Golden Plot ({label_str} selected events)...")

    sqrt_met = np.sqrt(met)
    mask = np.isfinite(sqrt_met) & np.isfinite(rcav) & (rcav > 0)
    sqrt_met_m = sqrt_met[mask]
    rcav_m = rcav[mask]

    fig, axes = plt.subplots(2, 3, figsize=(21, 14))

    # ---- Row 1: Main analysis ----

    # Panel 1: Scatter
    ax1 = axes[0, 0]
    # For large datasets, use random subsample for scatter
    if len(rcav_m) > 200_000:
        rng = np.random.default_rng(42)
        idx_sub = rng.choice(len(rcav_m), 200_000, replace=False)
        ax1.scatter(sqrt_met_m[idx_sub], rcav_m[idx_sub], s=0.3, alpha=0.03,
                    c='steelblue', rasterized=True)
    else:
        ax1.scatter(sqrt_met_m, rcav_m, s=0.5, alpha=0.05, c='steelblue', rasterized=True)
    coeffs = np.polyfit(sqrt_met_m, rcav_m, 1)
    x_fit = np.linspace(np.min(sqrt_met_m), np.max(sqrt_met_m), 100)
    ax1.plot(x_fit, np.polyval(coeffs, x_fit), 'r-', lw=2,
             label=f'Linear: R = {coeffs[0]:.4f}*sqrt(MET) + {coeffs[1]:.3f}')
    y_pred = np.polyval(coeffs, sqrt_met_m)
    ss_res = np.sum((rcav_m - y_pred)**2)
    ss_tot = np.sum((rcav_m - np.mean(rcav_m))**2)
    r2 = 1 - ss_res / ss_tot
    rho = np.corrcoef(sqrt_met_m, rcav_m)[0, 1]
    ax1.text(0.05, 0.95, f'R2 = {r2:.6f}\nrho = {rho:.6f}\nN = {len(rcav_m):,}',
             transform=ax1.transAxes, fontsize=10, va='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    ax1.set_xlabel('sqrt(MET) [sqrt(GeV)]', fontsize=11)
    ax1.set_ylabel('R_cav (max SV_dxy) [cm]', fontsize=11)
    ax1.set_title('R_cav vs sqrt(MET) -- Scatter', fontsize=12, fontweight='bold')
    ax1.legend(loc='lower right', fontsize=9)
    ax1.set_xlim(left=np.sqrt(MET_CUT))

    # Panel 2: 2D density
    ax2 = axes[0, 1]
    h = ax2.hist2d(sqrt_met_m, rcav_m, bins=[100, 100],
                   norm=mcolors.LogNorm(), cmap='inferno', rasterized=True)
    plt.colorbar(h[3], ax=ax2, label='Events (log)')

    # Envelope analysis
    n_bins = 40
    bin_edges = np.linspace(np.min(sqrt_met_m), np.max(sqrt_met_m), n_bins + 1)
    bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])
    p95 = []
    p50 = []
    p99 = []
    for j in range(n_bins):
        in_bin = (sqrt_met_m >= bin_edges[j]) & (sqrt_met_m < bin_edges[j+1])
        if np.sum(in_bin) > 50:
            p95.append(np.percentile(rcav_m[in_bin], 95))
            p50.append(np.percentile(rcav_m[in_bin], 50))
            p99.append(np.percentile(rcav_m[in_bin], 99))
        else:
            p95.append(np.nan)
            p50.append(np.nan)
            p99.append(np.nan)

    ax2.plot(bin_centers, p99, 'm-', lw=1.5, label='99th pct')
    ax2.plot(bin_centers, p95, 'c-', lw=2, label='95th pct')
    ax2.plot(bin_centers, p50, 'w--', lw=1.5, label='Median')
    ax2.set_xlabel('sqrt(MET) [sqrt(GeV)]', fontsize=11)
    ax2.set_ylabel('R_cav [cm]', fontsize=11)
    ax2.set_title('Event Density (log) + Envelope', fontsize=12, fontweight='bold')
    ax2.legend(loc='upper left', fontsize=8)

    # Panel 3: R_cav distribution
    ax3 = axes[0, 2]
    ax3.hist(rcav_m, bins=200, log=True, color='steelblue', alpha=0.7,
             edgecolor='navy', linewidth=0.2)
    ax3.set_xlabel('R_cav (max SV_dxy) [cm]', fontsize=11)
    ax3.set_ylabel('Events (log)', fontsize=11)
    ax3.set_title('SV Displacement Distribution', fontsize=12, fontweight='bold')
    ax3.axvline(x=2.5, color='red', ls='--', alpha=0.5, label='Beam pipe ~2.5cm')
    ax3.axvline(x=4.4, color='orange', ls='--', alpha=0.5, label='Pixel L1 ~4.4cm')
    ax3.axvline(x=7.3, color='green', ls='--', alpha=0.5, label='Pixel L2 ~7.3cm')
    ax3.axvline(x=10.2, color='cyan', ls='--', alpha=0.5, label='Pixel L3 ~10.2cm')
    ax3.legend(fontsize=7)

    # ---- Row 2: Deeper diagnostics ----

    # Panel 4: MET distribution
    ax4 = axes[1, 0]
    ax4.hist(met[met > MET_CUT], bins=200, log=True, color='coral', alpha=0.7,
             edgecolor='darkred', linewidth=0.2)
    ax4.set_xlabel('MET [GeV]', fontsize=11)
    ax4.set_ylabel('Events (log)', fontsize=11)
    ax4.set_title('MET Distribution (all events with SV)', fontsize=12, fontweight='bold')
    ax4.axvline(x=200, color='blue', ls=':', alpha=0.5, label='200 GeV')
    ax4.axvline(x=500, color='purple', ls=':', alpha=0.5, label='500 GeV')
    ax4.legend(fontsize=9)

    # Panel 5: R_cav vs MET (linear scale, not sqrt)
    ax5 = axes[1, 1]
    if len(rcav_m) > 200_000:
        ax5.scatter(met[mask][idx_sub], rcav_m[idx_sub], s=0.3, alpha=0.03,
                    c='darkorange', rasterized=True)
    else:
        ax5.scatter(met[mask], rcav_m, s=0.5, alpha=0.05, c='darkorange', rasterized=True)
    coeffs_lin = np.polyfit(met[mask], rcav_m, 1)
    x_lin = np.linspace(MET_CUT, np.max(met[mask]), 100)
    ax5.plot(x_lin, np.polyval(coeffs_lin, x_lin), 'r-', lw=2,
             label=f'Linear: R = {coeffs_lin[0]:.6f}*MET + {coeffs_lin[1]:.3f}')
    y_pred_lin = np.polyval(coeffs_lin, met[mask])
    ss_res_lin = np.sum((rcav_m - y_pred_lin)**2)
    r2_lin = 1 - ss_res_lin / ss_tot
    ax5.text(0.05, 0.95, f'R2 = {r2_lin:.6f}',
             transform=ax5.transAxes, fontsize=10, va='top',
             bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))
    ax5.set_xlabel('MET [GeV]', fontsize=11)
    ax5.set_ylabel('R_cav [cm]', fontsize=11)
    ax5.set_title('R_cav vs MET (linear, for comparison)', fontsize=12, fontweight='bold')
    ax5.legend(loc='lower right', fontsize=9)

    # Panel 6: Envelope slopes
    ax6 = axes[1, 2]
    p95_arr = np.array(p95)
    p99_arr = np.array(p99)
    p50_arr = np.array(p50)
    valid = np.isfinite(p95_arr)

    ax6.plot(bin_centers[valid], p99_arr[valid], 'm-o', ms=3, label='99th percentile')
    ax6.plot(bin_centers[valid], p95_arr[valid], 'c-o', ms=3, label='95th percentile')
    ax6.plot(bin_centers[valid], p50_arr[valid], 'k-o', ms=3, label='Median')

    if np.sum(valid) > 5:
        env95 = np.polyfit(bin_centers[valid], p95_arr[valid], 1)
        env99 = np.polyfit(bin_centers[valid], p99_arr[valid], 1)
        env50 = np.polyfit(bin_centers[valid], p50_arr[valid], 1)
        ax6.plot(bin_centers[valid], np.polyval(env95, bin_centers[valid]), 'c--', lw=1,
                 label=f'95th fit: slope={env95[0]:.4f}')
        ax6.plot(bin_centers[valid], np.polyval(env99, bin_centers[valid]), 'm--', lw=1,
                 label=f'99th fit: slope={env99[0]:.4f}')

    ax6.set_xlabel('sqrt(MET) [sqrt(GeV)]', fontsize=11)
    ax6.set_ylabel('R_cav percentile [cm]', fontsize=11)
    ax6.set_title('Envelope Analysis (FTD: positive slope?)', fontsize=12, fontweight='bold')
    ax6.legend(fontsize=7)

    fig.suptitle(
        f'CMS Run2016G MET -- FULL DATASET ({label_str} selected events) -- Raw SV Displacements\n'
        'FTD: hard boundary R_cav ~ sqrt(MET)  |  SM: exponential smear  |  No material veto',
        fontsize=13, fontweight='bold', y=1.01
    )

    plt.tight_layout()
    plt.savefig(PLOT_FILE, dpi=200, bbox_inches='tight')
    print(f"[OK] Plot saved to: {PLOT_FILE}")

    return sqrt_met_m, rcav_m, coeffs


def full_statistics(sqrt_met, rcav, coeffs):
    """Full statistical analysis on the complete dataset."""
    print("\n" + "=" * 60)
    print("  FULL DATASET STATISTICAL ANALYSIS")
    print("=" * 60)

    rho = np.corrcoef(sqrt_met, rcav)[0, 1]
    print(f"\n  N events analyzed:  {len(rcav):,}")
    print(f"  Pearson rho(sqrt(MET), R_cav) = {rho:.6f}")

    if coeffs is not None:
        print(f"  Linear fit: R = {coeffs[0]:.6f} * sqrt(MET) + {coeffs[1]:.4f}")

    print(f"\n  R_cav statistics:")
    print(f"    Mean:   {np.mean(rcav):.4f} cm")
    print(f"    Median: {np.median(rcav):.4f} cm")
    print(f"    Std:    {np.std(rcav):.4f} cm")
    print(f"    95th:   {np.percentile(rcav, 95):.4f} cm")
    print(f"    99th:   {np.percentile(rcav, 99):.4f} cm")
    print(f"    99.9th: {np.percentile(rcav, 99.9):.4f} cm")
    print(f"    Max:    {np.max(rcav):.4f} cm")

    try:
        from scipy.stats import kurtosis, skew
        print(f"\n  Kurtosis: {kurtosis(rcav):.4f}")
        print(f"  Skewness: {skew(rcav):.4f}")
    except ImportError:
        m = np.mean(rcav)
        s = np.std(rcav)
        print(f"\n  Kurtosis: {np.mean(((rcav-m)/s)**4)-3:.4f}")
        print(f"  Skewness: {np.mean(((rcav-m)/s)**3):.4f}")

    # Envelope slopes
    n_bins = 40
    bin_edges = np.linspace(np.min(sqrt_met), np.max(sqrt_met), n_bins + 1)
    bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])
    p95_vals = []
    for j in range(n_bins):
        in_bin = (sqrt_met >= bin_edges[j]) & (sqrt_met < bin_edges[j+1])
        if np.sum(in_bin) > 50:
            p95_vals.append(np.percentile(rcav[in_bin], 95))
        else:
            p95_vals.append(np.nan)
    p95_arr = np.array(p95_vals)
    valid = np.isfinite(p95_arr)
    if np.sum(valid) > 5:
        env = np.polyfit(bin_centers[valid], p95_arr[valid], 1)
        print(f"\n  95th-percentile envelope:")
        print(f"    Slope = {env[0]:.6f} cm/sqrt(GeV)")
        if env[0] > 0:
            print(f"    -> POSITIVE: upper boundary grows with energy")
        else:
            print(f"    -> FLAT/NEGATIVE: no energy-dependent boundary")

    # Compare sqrt(MET) vs MET correlation
    rho_lin = np.corrcoef(np.square(sqrt_met), rcav)[0, 1]
    print(f"\n  Correlation comparison:")
    print(f"    rho(sqrt(MET), R_cav) = {rho:.6f}  [FTD prediction]")
    print(f"    rho(MET, R_cav)       = {rho_lin:.6f}  [linear]")
    if abs(rho) > abs(rho_lin):
        print(f"    -> sqrt(MET) correlates better (favors FTD)")
    else:
        print(f"    -> linear MET correlates better (disfavors FTD)")


if __name__ == "__main__":
    print("=" * 60)
    print("  FTD Cavitation Test -- FULL DATASET")
    print("  CMS Run2016G MET NanoAOD (~27M events, 13 TeV)")
    print(f"  Parallel downloads: {N_PARALLEL}")
    print("=" * 60)

    t0 = time.time()

    met, rcav = process_all_files()

    if len(met) == 0:
        print("[ERROR] No events passed selection.")
        sys.exit(1)

    print(f"\nTotal selected events: {len(met):,}")

    sqrt_met, rcav_sel, coeffs = make_full_plot(met, rcav)
    full_statistics(sqrt_met, rcav_sel, coeffs)

    elapsed = time.time() - t0
    print(f"\nTotal runtime: {elapsed:.0f} s ({elapsed/60:.1f} min)")
    print(f"Plot: {PLOT_FILE}")
