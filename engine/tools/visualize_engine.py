"""
FTD Engine Visualizer — generates figures from CSV scenario outputs.

Usage:
    python visualize_engine.py          # all panels
    python visualize_engine.py panel    # single panel: interference | pair | force
"""
import sys, os, glob
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import matplotlib.gridspec as gridspec

OUTDIR = os.path.join(os.path.dirname(__file__), "output")
FIGDIR = os.path.join(os.path.dirname(__file__), "output", "figures")
os.makedirs(FIGDIR, exist_ok=True)

# ── Helpers ──────────────────────────────────────────────────────────
def load_slice(path):
    """Load a density-slice CSV. Returns x, y, density as 2D arrays."""
    data = np.genfromtxt(path, delimiter=',', names=True)
    cols = data.dtype.names
    x_col, y_col = cols[0], cols[1]
    xs = np.unique(data[x_col]).astype(int)
    ys = np.unique(data[y_col]).astype(int)
    N = len(xs)
    density = data['density'].reshape(N, N)
    return xs, ys, density

def load_timeseries(path):
    """Load a diagnostics timeseries CSV."""
    return np.genfromtxt(path, delimiter=',', names=True)

# ── Panel 1: Interference Fringes ────────────────────────────────────
def plot_interference():
    idir = os.path.join(OUTDIR, "interference")
    slices = sorted(glob.glob(os.path.join(idir, "slice_z_t*.csv")))
    if not slices:
        print("No interference data found. Run: ftd_sim I 64 200 output/interference")
        return

    # Pick key snapshots
    picks = [slices[0], slices[len(slices)//4], slices[len(slices)//2], slices[-1]]
    ticks = []
    for p in picks:
        base = os.path.basename(p)
        t = int(base.replace("slice_z_t", "").replace(".csv", ""))
        ticks.append(t)

    fig, axes = plt.subplots(1, 4, figsize=(18, 4.5))
    fig.suptitle("FTD Interference Pattern — 4 Coherent Sources (64³ lattice)",
                 fontsize=14, fontweight='bold', color='white')
    fig.patch.set_facecolor('#0a0a12')

    for ax, path, tick in zip(axes, picks, ticks):
        xs, ys, density = load_slice(path)
        vmax = max(density.max(), 1e-10)
        im = ax.imshow(density.T, origin='lower', cmap='inferno',
                       extent=[xs[0], xs[-1], ys[0], ys[-1]],
                       vmin=0, vmax=vmax * 0.8, interpolation='bilinear')
        ax.set_title(f"t = {tick}", color='white', fontsize=11)
        ax.set_xlabel("x", color='#888')
        ax.set_ylabel("y", color='#888')
        ax.tick_params(colors='#666')
        ax.set_facecolor('#0a0a12')

    plt.tight_layout(rect=[0, 0, 1, 0.92])
    outpath = os.path.join(FIGDIR, "interference_fringes.png")
    plt.savefig(outpath, dpi=150, facecolor=fig.get_facecolor())
    plt.close()
    print(f"  Saved: {outpath}")

# ── Panel 2: Pair Production ────────────────────────────────────────
def plot_pair_production():
    pdir = os.path.join(OUTDIR, "pair")
    ts_path = os.path.join(pdir, "timeseries.csv")
    if not os.path.exists(ts_path):
        print("No pair production data. Run: ftd_sim J 32 100 output/pair")
        return

    ts = load_timeseries(ts_path)

    # Find density slices
    slices = sorted(glob.glob(os.path.join(pdir, "pair_slice_t*.csv")))
    final_slice = os.path.join(pdir, "pair_slice_final.csv")

    fig = plt.figure(figsize=(16, 8))
    fig.patch.set_facecolor('#0a0a12')
    fig.suptitle("FTD Pair Production — Counter-Propagating Flux Beams (32³)",
                 fontsize=14, fontweight='bold', color='white')

    gs = gridspec.GridSpec(2, 4, figure=fig, hspace=0.35, wspace=0.3)

    # Top row: 4 density snapshots
    snap_picks = []
    if len(slices) >= 4:
        indices = [0, len(slices)//3, 2*len(slices)//3, -1]
        snap_picks = [slices[i] for i in indices]
    elif slices:
        snap_picks = slices[:4]

    for i, path in enumerate(snap_picks):
        ax = fig.add_subplot(gs[0, i])
        xs, ys, density = load_slice(path)
        base = os.path.basename(path)
        t = base.replace("pair_slice_t", "").replace(".csv", "").replace("final", "end")
        ax.imshow(density.T, origin='lower', cmap='magma',
                  extent=[xs[0], xs[-1], ys[0], ys[-1]],
                  interpolation='bilinear')
        ax.set_title(f"t = {t}", color='white', fontsize=10)
        ax.tick_params(colors='#666', labelsize=7)
        ax.set_facecolor('#0a0a12')

    # Bottom-left: timeseries — manifested count
    ax1 = fig.add_subplot(gs[1, :2])
    ax1.set_facecolor('#0a0a12')
    ax1.plot(ts['tick'], ts['manifested'], color='#ff6644', linewidth=1.5, label='Total')
    if 'positive' in ts.dtype.names:
        ax1.plot(ts['tick'], ts['positive'], color='#44aaff', linewidth=1, label='+', alpha=0.8)
        ax1.plot(ts['tick'], ts['negative'], color='#ff44aa', linewidth=1, label='-', alpha=0.8)
    ax1.set_xlabel("Tick", color='#888')
    ax1.set_ylabel("Particles", color='#888')
    ax1.set_title("Particle Count", color='white', fontsize=11)
    ax1.legend(fontsize=8, facecolor='#1a1a2e', edgecolor='#333', labelcolor='white')
    ax1.tick_params(colors='#666')

    # Bottom-right: total flux
    ax2 = fig.add_subplot(gs[1, 2:])
    ax2.set_facecolor('#0a0a12')
    ax2.plot(ts['tick'], ts['total_flux'], color='#00ffcc', linewidth=1.5)
    ax2.set_xlabel("Tick", color='#888')
    ax2.set_ylabel("Total Flux", color='#888')
    ax2.set_title("Total Flux (Energy)", color='white', fontsize=11)
    ax2.tick_params(colors='#666')

    outpath = os.path.join(FIGDIR, "pair_production.png")
    plt.savefig(outpath, dpi=150, facecolor=fig.get_facecolor())
    plt.close()
    print(f"  Saved: {outpath}")

# ── Panel 3: Force Law Profile ──────────────────────────────────────
def plot_force_law():
    fdir = os.path.join(OUTDIR, "force")
    prof_path = os.path.join(fdir, "force_profile.csv")
    if not os.path.exists(prof_path):
        print("No force profile data. Run: ftd_sim K 48 1000 output/force")
        return

    # CSV has string 'axis' column — read manually
    import csv
    rows = []
    with open(prof_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)

    # Average across all axes at each radius
    from collections import defaultdict
    by_r = defaultdict(lambda: {'density': [], 'grad': [], 'div': []})
    for row in rows:
        r_val = float(row['r'])
        by_r[r_val]['density'].append(float(row['density']))
        by_r[r_val]['grad'].append(float(row['grad_divJ_mag']))
        by_r[r_val]['div'].append(float(row['div_J']))

    r = np.array(sorted(by_r.keys()))
    density = np.array([np.mean(by_r[rv]['density']) for rv in r])
    gdj = np.array([np.mean(by_r[rv]['grad']) for rv in r])
    div_data = np.array([np.mean(by_r[rv]['div']) for rv in r])

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.patch.set_facecolor('#0a0a12')
    fig.suptitle("FTD Force Law Profile — Single +1 Source (48³, 1000 ticks)",
                 fontsize=14, fontweight='bold', color='white')

    mask = (r >= 2) & (r <= 20)
    r_ref = r[mask]

    # (a) Density vs r
    ax = axes[0]
    ax.set_facecolor('#0a0a12')
    ax.loglog(r[mask], density[mask], 'o-', color='#ff9944', markersize=4, linewidth=1.2)
    # Reference 1/r line
    d0 = density[mask][0]
    ax.loglog(r_ref, d0 * (r_ref[0] / r_ref), '--', color='#666', label='1/r')
    ax.loglog(r_ref, d0 * (r_ref[0] / r_ref)**2, ':', color='#444', label='1/r²')
    ax.set_xlabel("r (lattice units)", color='#888')
    ax.set_ylabel("|J| (density)", color='#888')
    ax.set_title("Flux Density vs Radius", color='white', fontsize=11)
    ax.legend(fontsize=8, facecolor='#1a1a2e', edgecolor='#333', labelcolor='white')
    ax.tick_params(colors='#666')

    # (b) |grad(div J)| vs r  (force proxy)
    ax = axes[1]
    ax.set_facecolor('#0a0a12')
    ax.loglog(r[mask], gdj[mask], 's-', color='#44ccff', markersize=4, linewidth=1.2, label='|∇(∇·J)|')
    # Reference 1/r² line
    if gdj[mask][0] > 0:
        g0 = gdj[mask][0]
        ax.loglog(r_ref, g0 * (r_ref[0] / r_ref)**2, '--', color='#666', label='1/r²')
        ax.loglog(r_ref, g0 * (r_ref[0] / r_ref)**3, ':', color='#444', label='1/r³')
    ax.set_xlabel("r (lattice units)", color='#888')
    ax.set_ylabel("|∇(∇·J)|", color='#888')
    ax.set_title("Force Law (Coulomb Check)", color='white', fontsize=11)
    ax.legend(fontsize=8, facecolor='#1a1a2e', edgecolor='#333', labelcolor='white')
    ax.tick_params(colors='#666')

    # (c) Divergence vs r
    ax = axes[2]
    ax.set_facecolor('#0a0a12')
    ax.semilogx(r[mask], div_data[mask], 'D-', color='#cc44ff', markersize=4, linewidth=1.2)
    ax.axhline(0, color='#444', linewidth=0.5)
    ax.set_xlabel("r (lattice units)", color='#888')
    ax.set_ylabel("div(J)", color='#888')
    ax.set_title("Divergence (Gauss Law Check)", color='white', fontsize=11)
    ax.tick_params(colors='#666')

    # Add power-law fit annotation
    log_r = np.log(r[mask])
    log_d = np.log(density[mask])
    valid = np.isfinite(log_d)
    if valid.sum() > 2:
        slope = np.polyfit(log_r[valid], log_d[valid], 1)[0]
        axes[0].annotate(f"slope = {slope:.2f}", xy=(0.05, 0.05),
                         xycoords='axes fraction', color='#ff9944', fontsize=9)

    log_g = np.log(gdj[mask])
    valid_g = np.isfinite(log_g) & (gdj[mask] > 0)
    if valid_g.sum() > 2:
        slope_g = np.polyfit(log_r[valid_g], log_g[valid_g], 1)[0]
        axes[1].annotate(f"slope = {slope_g:.2f}", xy=(0.05, 0.05),
                         xycoords='axes fraction', color='#44ccff', fontsize=9)

    plt.tight_layout(rect=[0, 0, 1, 0.92])
    outpath = os.path.join(FIGDIR, "force_law_profile.png")
    plt.savefig(outpath, dpi=150, facecolor=fig.get_facecolor())
    plt.close()
    print(f"  Saved: {outpath}")

# ── Main ─────────────────────────────────────────────────────────────
if __name__ == "__main__":
    panels = sys.argv[1:] if len(sys.argv) > 1 else ["interference", "pair", "force"]

    print("FTD Engine Visualizer")
    print("=" * 50)

    for panel in panels:
        if panel == "interference":
            plot_interference()
        elif panel == "pair":
            plot_pair_production()
        elif panel == "force":
            plot_force_law()
        else:
            print(f"Unknown panel: {panel}")

    print(f"\nFigures saved to: {FIGDIR}")
