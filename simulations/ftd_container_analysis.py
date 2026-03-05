#!/usr/bin/env python3
"""
FTD Topological Cavitation Test -- CONTAINER VERSION
=====================================================

Runs inside the CERN cms-cloud/python-vnc Docker container.
Uses XRootD protocol for fast data access (no full file download needed).
Processes ALL 17 files from CMS Run2016G MET NanoAOD (record 30526).

Usage (from host):
  docker run --rm -v /path/to/simulations:/work \
    gitlab-registry.cern.ch/cms-cloud/python-vnc:latest \
    python3 /work/ftd_container_analysis.py
"""

import os
import sys
import time
import numpy as np

# Matplotlib setup (no display in container)
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
OUTPUT_DIR = os.environ.get("FTD_OUTPUT", "/home/cmsusr")  # Container home dir
PLOT_FILE = os.path.join(OUTPUT_DIR, "ftd_cavitation_FULL_27M.png")
DATA_CACHE = os.path.join(OUTPUT_DIR, "ftd_full_extracted.npz")

MET_CUT = 100.0
SV_DXY_MIN = 0.01
SV_DXY_MAX = 100.0
BRANCHES = ["MET_pt", "nSV", "SV_dxy"]

# XRootD protocol -- should be faster than HTTPS from inside container
XROOTD_BASE = "root://eospublic.cern.ch//eos/opendata/cms/Run2016G/MET/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1"
# HTTP fallback
HTTP_BASE = "https://eospublic.cern.ch/eos/opendata/cms/Run2016G/MET/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1"

# All 17 files sorted by size (smallest first)
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


def detect_protocol():
    """Try XRootD first, fall back to HTTP."""
    import uproot

    test_file = FILES[0]
    xrd_url = f"{XROOTD_BASE}/{test_file}"
    http_url = f"{HTTP_BASE}/{test_file}"

    print("Testing data access protocols...")

    # Try XRootD
    try:
        t0 = time.time()
        f = uproot.open({xrd_url: "Events"})
        n = f.num_entries
        t1 = time.time()
        f.close() if hasattr(f, 'close') else None
        print(f"  XRootD: OK ({n:,} entries, {t1-t0:.1f}s to open)")
        return "xrootd", XROOTD_BASE
    except Exception as e:
        print(f"  XRootD: FAILED ({e})")

    # Try HTTPS
    try:
        t0 = time.time()
        f = uproot.open({http_url: "Events"})
        n = f.num_entries
        t1 = time.time()
        f.close() if hasattr(f, 'close') else None
        print(f"  HTTPS: OK ({n:,} entries, {t1-t0:.1f}s to open)")
        return "https", HTTP_BASE
    except Exception as e:
        print(f"  HTTPS: FAILED ({e})")

    print("  ERROR: No working protocol found!")
    return None, None


def extract_remote(base_url, fname):
    """Stream-read MET and SV from a remote NanoAOD file (only reads needed branches)."""
    import uproot
    import awkward as ak

    url = f"{base_url}/{fname}"
    short = fname.split("/")[-1][:20]

    t0 = time.time()
    try:
        f = uproot.open(url)
        tree = f["Events"]
        n_total = tree.num_entries
    except Exception as e:
        print(f"  ERROR opening {short}: {e}")
        return 0, 0, np.array([]), np.array([])

    met_list = []
    rcav_list = []
    n_pass = 0

    chunk_size = 500_000
    for start in range(0, n_total, chunk_size):
        stop = min(start + chunk_size, n_total)
        try:
            data = tree.arrays(BRANCHES, entry_start=start, entry_stop=stop)
        except Exception as e:
            print(f"    chunk {start}-{stop} ERROR: {e}")
            continue

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

    elapsed = time.time() - t0
    met_arr = np.concatenate(met_list) if met_list else np.array([])
    rcav_arr = np.concatenate(rcav_list) if rcav_list else np.array([])

    print(f"  {short}: {n_total:,} events, {n_pass:,} selected ({elapsed:.0f}s, "
          f"{n_total/elapsed:.0f} evt/s)")

    return n_total, n_pass, met_arr, rcav_arr


def process_all_files():
    """Process all files via streaming read."""
    # Check cache
    if os.path.exists(DATA_CACHE):
        print(f"[CACHE] Loading pre-extracted data from {DATA_CACHE}")
        d = np.load(DATA_CACHE)
        return d["met"], d["rcav"]

    protocol, base_url = detect_protocol()
    if base_url is None:
        print("FATAL: Cannot access data. Exiting.")
        sys.exit(1)

    print(f"\nUsing {protocol} protocol")
    print(f"Processing {len(FILES)} files")
    print("=" * 70)

    all_met = []
    all_rcav = []
    total_events = 0
    total_selected = 0

    for i, fname in enumerate(FILES):
        print(f"\n[{i+1}/{len(FILES)}]", end="")
        n_total, n_pass, met, rcav = extract_remote(base_url, fname)

        if len(met) > 0:
            all_met.append(met)
            all_rcav.append(rcav)
            total_events += n_total
            total_selected += n_pass

        cum = sum(len(m) for m in all_met)
        print(f"  >> Cumulative: {total_events:,} events, {cum:,} selected")

        # Save intermediate results every 3 files
        if (i + 1) % 3 == 0 and all_met:
            met_tmp = np.concatenate(all_met)
            rcav_tmp = np.concatenate(all_rcav)
            partial = DATA_CACHE.replace(".npz", f"_partial_{i+1}.npz")
            np.savez_compressed(partial, met=met_tmp, rcav=rcav_tmp)
            print(f"  >> Saved checkpoint ({i+1} files, {len(met_tmp):,} events)")

    print("\n" + "=" * 70)
    print(f"COMPLETE: {total_events:,} total events, {total_selected:,} selected")

    met_all = np.concatenate(all_met) if all_met else np.array([])
    rcav_all = np.concatenate(all_rcav) if all_rcav else np.array([])

    np.savez_compressed(DATA_CACHE, met=met_all, rcav=rcav_all)
    print(f"Cached to {DATA_CACHE}")

    # Clean up partial files
    import glob
    for f in glob.glob(DATA_CACHE.replace(".npz", "_partial_*.npz")):
        os.remove(f)

    return met_all, rcav_all


def make_full_plot(met, rcav):
    """Generate the 6-panel golden plot."""
    n_events = len(met)
    label_str = f"~{n_events/1e6:.1f}M" if n_events > 1e6 else f"{n_events:,}"
    print(f"\nGenerating Golden Plot ({label_str} selected events)...")

    sqrt_met = np.sqrt(met)
    mask = np.isfinite(sqrt_met) & np.isfinite(rcav) & (rcav > 0)
    sm = sqrt_met[mask]
    rc = rcav[mask]

    fig, axes = plt.subplots(2, 3, figsize=(21, 14))

    # --- Panel 1: Scatter + linear fit ---
    ax = axes[0, 0]
    if len(rc) > 200_000:
        rng = np.random.default_rng(42)
        idx = rng.choice(len(rc), 200_000, replace=False)
        ax.scatter(sm[idx], rc[idx], s=0.3, alpha=0.03, c='steelblue', rasterized=True)
    else:
        ax.scatter(sm, rc, s=0.5, alpha=0.05, c='steelblue', rasterized=True)

    coeffs = np.polyfit(sm, rc, 1)
    x_fit = np.linspace(sm.min(), sm.max(), 100)
    ax.plot(x_fit, np.polyval(coeffs, x_fit), 'r-', lw=2,
            label=f'Linear: R = {coeffs[0]:.4f}*sqrt(MET) + {coeffs[1]:.3f}')

    ss_res = np.sum((rc - np.polyval(coeffs, sm))**2)
    ss_tot = np.sum((rc - rc.mean())**2)
    r2 = 1 - ss_res / ss_tot
    rho = np.corrcoef(sm, rc)[0, 1]

    ax.text(0.05, 0.95, f'R2 = {r2:.6f}\nrho = {rho:.6f}\nN = {len(rc):,}',
            transform=ax.transAxes, fontsize=10, va='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    ax.set_xlabel('sqrt(MET) [sqrt(GeV)]')
    ax.set_ylabel('R_cav (max SV_dxy) [cm]')
    ax.set_title('R_cav vs sqrt(MET) -- Scatter', fontweight='bold')
    ax.legend(loc='lower right', fontsize=9)
    ax.set_xlim(left=np.sqrt(MET_CUT))

    # --- Panel 2: 2D density + envelope ---
    ax = axes[0, 1]
    h = ax.hist2d(sm, rc, bins=[100, 100], norm=mcolors.LogNorm(), cmap='inferno',
                  rasterized=True)
    plt.colorbar(h[3], ax=ax, label='Events (log)')

    n_bins = 40
    edges = np.linspace(sm.min(), sm.max(), n_bins + 1)
    centers = 0.5 * (edges[:-1] + edges[1:])
    p50, p95, p99 = [], [], []
    for j in range(n_bins):
        in_bin = (sm >= edges[j]) & (sm < edges[j+1])
        if np.sum(in_bin) > 50:
            vals = rc[in_bin]
            p50.append(np.percentile(vals, 50))
            p95.append(np.percentile(vals, 95))
            p99.append(np.percentile(vals, 99))
        else:
            p50.append(np.nan); p95.append(np.nan); p99.append(np.nan)

    ax.plot(centers, p99, 'm-', lw=1.5, label='99th pct')
    ax.plot(centers, p95, 'c-', lw=2, label='95th pct')
    ax.plot(centers, p50, 'w--', lw=1.5, label='Median')
    ax.set_xlabel('sqrt(MET) [sqrt(GeV)]')
    ax.set_ylabel('R_cav [cm]')
    ax.set_title('Event Density (log) + Envelope', fontweight='bold')
    ax.legend(loc='upper left', fontsize=8)

    # --- Panel 3: SV distribution ---
    ax = axes[0, 2]
    ax.hist(rc, bins=200, log=True, color='steelblue', alpha=0.7,
            edgecolor='navy', linewidth=0.2)
    ax.set_xlabel('R_cav (max SV_dxy) [cm]')
    ax.set_ylabel('Events (log)')
    ax.set_title('SV Displacement Distribution', fontweight='bold')
    for x, c, lbl in [(2.5, 'red', 'Beam pipe ~2.5cm'),
                       (4.4, 'orange', 'Pixel L1 ~4.4cm'),
                       (7.3, 'green', 'Pixel L2 ~7.3cm'),
                       (10.2, 'cyan', 'Pixel L3 ~10.2cm')]:
        ax.axvline(x=x, color=c, ls='--', alpha=0.5, label=lbl)
    ax.legend(fontsize=7)

    # --- Panel 4: MET distribution ---
    ax = axes[1, 0]
    ax.hist(met[met > MET_CUT], bins=200, log=True, color='coral', alpha=0.7,
            edgecolor='darkred', linewidth=0.2)
    ax.set_xlabel('MET [GeV]')
    ax.set_ylabel('Events (log)')
    ax.set_title('MET Distribution (events with SV)', fontweight='bold')
    ax.axvline(x=200, color='blue', ls=':', alpha=0.5, label='200 GeV')
    ax.axvline(x=500, color='purple', ls=':', alpha=0.5, label='500 GeV')
    ax.legend(fontsize=9)

    # --- Panel 5: R_cav vs MET (linear) ---
    ax = axes[1, 1]
    met_m = met[mask]
    if len(rc) > 200_000:
        ax.scatter(met_m[idx], rc[idx], s=0.3, alpha=0.03, c='darkorange', rasterized=True)
    else:
        ax.scatter(met_m, rc, s=0.5, alpha=0.05, c='darkorange', rasterized=True)
    coeffs_lin = np.polyfit(met_m, rc, 1)
    x_lin = np.linspace(MET_CUT, met_m.max(), 100)
    ax.plot(x_lin, np.polyval(coeffs_lin, x_lin), 'r-', lw=2,
            label=f'Linear: slope={coeffs_lin[0]:.6f}')
    r2_lin = 1 - np.sum((rc - np.polyval(coeffs_lin, met_m))**2) / ss_tot
    ax.text(0.05, 0.95, f'R2 = {r2_lin:.6f}',
            transform=ax.transAxes, fontsize=10, va='top',
            bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))
    ax.set_xlabel('MET [GeV]')
    ax.set_ylabel('R_cav [cm]')
    ax.set_title('R_cav vs MET (linear comparison)', fontweight='bold')
    ax.legend(loc='lower right', fontsize=9)

    # --- Panel 6: Envelope slopes ---
    ax = axes[1, 2]
    p95a = np.array(p95); p99a = np.array(p99); p50a = np.array(p50)
    valid = np.isfinite(p95a)
    ax.plot(centers[valid], p99a[valid], 'm-o', ms=3, label='99th pct')
    ax.plot(centers[valid], p95a[valid], 'c-o', ms=3, label='95th pct')
    ax.plot(centers[valid], p50a[valid], 'k-o', ms=3, label='Median')
    if np.sum(valid) > 5:
        e95 = np.polyfit(centers[valid], p95a[valid], 1)
        e99 = np.polyfit(centers[valid], p99a[valid], 1)
        ax.plot(centers[valid], np.polyval(e95, centers[valid]), 'c--', lw=1,
                label=f'95th slope={e95[0]:.4f}')
        ax.plot(centers[valid], np.polyval(e99, centers[valid]), 'm--', lw=1,
                label=f'99th slope={e99[0]:.4f}')
    ax.set_xlabel('sqrt(MET) [sqrt(GeV)]')
    ax.set_ylabel('R_cav percentile [cm]')
    ax.set_title('Envelope Analysis (FTD: positive slope?)', fontweight='bold')
    ax.legend(fontsize=7)

    fig.suptitle(
        f'CMS Run2016G MET -- FULL DATASET ({label_str} selected) -- Raw SV Displacements\n'
        'FTD: hard boundary R_cav ~ sqrt(MET)  |  SM: exponential smear  |  No material veto',
        fontsize=13, fontweight='bold', y=1.01)

    plt.tight_layout()
    plt.savefig(PLOT_FILE, dpi=200, bbox_inches='tight')
    print(f"[OK] Plot saved: {PLOT_FILE}")

    return sm, rc, coeffs


def full_statistics(sm, rc, coeffs):
    """Statistical analysis."""
    print("\n" + "=" * 60)
    print("  FULL DATASET STATISTICAL ANALYSIS")
    print("=" * 60)

    rho = np.corrcoef(sm, rc)[0, 1]
    rho_lin = np.corrcoef(sm**2, rc)[0, 1]

    print(f"\n  N events:  {len(rc):,}")
    print(f"  Pearson rho(sqrt(MET), R_cav) = {rho:.6f}  [FTD]")
    print(f"  Pearson rho(MET, R_cav)       = {rho_lin:.6f}  [linear]")
    print(f"  Linear fit: R = {coeffs[0]:.6f} * sqrt(MET) + {coeffs[1]:.4f}")

    print(f"\n  R_cav statistics:")
    for pct in [50, 95, 99, 99.9]:
        print(f"    {pct:5.1f}th: {np.percentile(rc, pct):.4f} cm")
    print(f"    Mean:  {rc.mean():.4f} cm")
    print(f"    Std:   {rc.std():.4f} cm")
    print(f"    Max:   {rc.max():.4f} cm")

    m, s = rc.mean(), rc.std()
    kurt = np.mean(((rc - m)/s)**4) - 3
    skew = np.mean(((rc - m)/s)**3)
    print(f"\n  Kurtosis: {kurt:.4f}")
    print(f"  Skewness: {skew:.4f}")

    # Envelope
    n_bins = 40
    edges = np.linspace(sm.min(), sm.max(), n_bins + 1)
    centers = 0.5 * (edges[:-1] + edges[1:])
    p95_vals = []
    for j in range(n_bins):
        in_bin = (sm >= edges[j]) & (sm < edges[j+1])
        if np.sum(in_bin) > 50:
            p95_vals.append(np.percentile(rc[in_bin], 95))
        else:
            p95_vals.append(np.nan)
    p95a = np.array(p95_vals)
    valid = np.isfinite(p95a)
    if np.sum(valid) > 5:
        env = np.polyfit(centers[valid], p95a[valid], 1)
        print(f"\n  95th-pct envelope slope = {env[0]:.6f} cm/sqrt(GeV)")
        print(f"    -> {'POSITIVE (boundary grows)' if env[0] > 0 else 'FLAT/NEGATIVE (no boundary)'}")

    if abs(rho) > abs(rho_lin):
        print(f"\n  >> sqrt(MET) correlates better -- consistent with FTD")
    else:
        print(f"\n  >> MET correlates better -- disfavors FTD sqrt scaling")


if __name__ == "__main__":
    print("=" * 60)
    print("  FTD Cavitation Test -- CONTAINER VERSION")
    print("  CMS Run2016G MET NanoAOD (~27M events)")
    print("=" * 60)

    # Ensure output dir exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    t0 = time.time()
    met, rcav = process_all_files()

    if len(met) == 0:
        print("[ERROR] No events passed selection.")
        sys.exit(1)

    print(f"\nTotal selected: {len(met):,}")
    sm, rc, coeffs = make_full_plot(met, rcav)
    full_statistics(sm, rc, coeffs)

    elapsed = time.time() - t0
    print(f"\nRuntime: {elapsed:.0f}s ({elapsed/60:.1f} min)")
    print(f"Plot: {PLOT_FILE}")
