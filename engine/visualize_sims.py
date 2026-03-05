"""
FTD Simulation Visualizer
=========================
Reads CSV data exported by ftd_sim scenarios H/I/J and generates
publication-quality figures.

Usage:
    python visualize_sims.py [output_dir]

Default output_dir: engine/build/Release/output
Generates PNG files in the same directory.
"""

import sys
import os
import glob
import re
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm, Normalize
from matplotlib import cm
from pathlib import Path

# Use a clean style
plt.rcParams.update({
    'figure.facecolor': '#0a0a1a',
    'axes.facecolor': '#0a0a1a',
    'text.color': 'white',
    'axes.labelcolor': 'white',
    'xtick.color': 'white',
    'ytick.color': 'white',
    'axes.edgecolor': '#333355',
    'font.family': 'monospace',
    'font.size': 10,
    'figure.dpi': 150,
})


def load_density_slice(filepath):
    """Load a 2D density slice CSV into a numpy array."""
    data = np.genfromtxt(filepath, delimiter=',', skip_header=1)
    # Columns: x, y, density, Jx, Jy, Jz, state
    xs = data[:, 0].astype(int)
    ys = data[:, 1].astype(int)
    density = data[:, 2]
    states = data[:, 6].astype(int)

    N = max(xs.max(), ys.max()) + 1
    grid = np.zeros((N, N))
    state_grid = np.zeros((N, N))
    for i in range(len(xs)):
        grid[ys[i], xs[i]] = density[i]
        state_grid[ys[i], xs[i]] = states[i]

    return grid, state_grid, N


def load_timeseries(filepath):
    """Load diagnostics timeseries CSV."""
    data = np.genfromtxt(filepath, delimiter=',', names=True)
    return data


# =========================================================================
# Figure 1: Interference Pattern Evolution (Scenario I)
# =========================================================================

def plot_interference(outdir, figdir):
    """Multi-panel interference pattern evolution."""
    files = sorted(glob.glob(os.path.join(outdir, 'slice_z_t*.csv')))
    if not files:
        print("  No interference data found, skipping.")
        return

    # Select key frames
    n_panels = min(len(files), 6)
    indices = np.linspace(0, len(files) - 1, n_panels, dtype=int)
    selected = [files[i] for i in indices]

    fig, axes = plt.subplots(2, 3, figsize=(16, 10))
    fig.suptitle('FTD INTERFERENCE PATTERN EVOLUTION', fontsize=16,
                 fontweight='bold', color='#00ccff', y=0.98)

    for ax_idx, (ax, fpath) in enumerate(zip(axes.flat, selected)):
        grid, _, N = load_density_slice(fpath)

        # Extract tick from filename
        tick = re.search(r't(\d+)', os.path.basename(fpath))
        tick_num = int(tick.group(1)) if tick else ax_idx

        # Use log scale for better visibility
        vmin = np.percentile(grid[grid > 0], 5) if np.any(grid > 0) else 1e-6
        vmax = grid.max() if grid.max() > 0 else 1.0

        im = ax.imshow(grid, cmap='inferno', origin='lower',
                       norm=LogNorm(vmin=max(vmin, 1e-6), vmax=max(vmax, 1e-5)),
                       interpolation='bilinear')
        ax.set_title(f't = {tick_num}', color='#ffaa00', fontsize=12)
        ax.set_xlabel('x')
        ax.set_ylabel('y')

        # Mark source positions
        mid = N // 2
        half = N // 5
        for sx, sy in [(mid-half, mid-half), (mid+half, mid-half),
                       (mid-half, mid+half), (mid+half, mid+half)]:
            ax.plot(sx, sy, 'w+', markersize=8, markeredgewidth=1.5)

    # Fill remaining axes if fewer than 6 files
    for ax in axes.flat[len(selected):]:
        ax.axis('off')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    savepath = os.path.join(figdir, 'fig_interference_evolution.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 2: Single High-Res Interference Heatmap
# =========================================================================

def plot_interference_heatmap(outdir, figdir):
    """Single large-format interference heatmap at final time."""
    fpath = os.path.join(outdir, 'slice_xy_final.csv')
    if not os.path.exists(fpath):
        # Try the last time-step file
        files = sorted(glob.glob(os.path.join(outdir, 'slice_z_t*.csv')))
        if not files:
            print("  No interference final slice found, skipping.")
            return
        fpath = files[-1]

    grid, _, N = load_density_slice(fpath)

    fig, ax = plt.subplots(figsize=(10, 10))
    fig.suptitle('FTD FLUX INTERFERENCE PATTERN', fontsize=18,
                 fontweight='bold', color='#00ccff', y=0.96)

    vmin = np.percentile(grid[grid > 0], 2) if np.any(grid > 0) else 1e-6
    vmax = grid.max() if grid.max() > 0 else 1.0

    im = ax.imshow(grid, cmap='inferno', origin='lower',
                   norm=LogNorm(vmin=max(vmin, 1e-6), vmax=max(vmax, 1e-5)),
                   interpolation='bilinear')

    cbar = plt.colorbar(im, ax=ax, shrink=0.8, pad=0.02)
    cbar.set_label('Flux density |J|', color='white')
    cbar.ax.yaxis.set_tick_params(color='white')
    plt.setp(plt.getp(cbar.ax.axes, 'yticklabels'), color='white')

    ax.set_xlabel('x (lattice units)', fontsize=12)
    ax.set_ylabel('y (lattice units)', fontsize=12)
    ax.set_title(f'z-slice at midplane, {N}³ lattice', color='#aaaacc', fontsize=11)

    # Mark source positions
    mid = N // 2
    half = N // 5
    for sx, sy in [(mid-half, mid-half), (mid+half, mid-half),
                   (mid-half, mid+half), (mid+half, mid+half)]:
        ax.plot(sx, sy, 'w+', markersize=12, markeredgewidth=2)
        ax.annotate('src', (sx+1, sy+1), color='white', fontsize=7, alpha=0.6)

    savepath = os.path.join(figdir, 'fig_interference_heatmap.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 3: Helium Atom Flux Field (Scenario H)
# =========================================================================

def plot_helium(outdir, figdir):
    """Helium atom density slices showing electron shell structure."""
    files = sorted(glob.glob(os.path.join(outdir, 'he_slice_t*.csv')))
    if not files:
        print("  No helium data found, skipping.")
        return

    # Select 4 key frames: early, mid, late, final
    n_panels = min(len(files), 4)
    indices = np.linspace(0, len(files) - 1, n_panels, dtype=int)
    selected = [files[i] for i in indices]

    fig, axes = plt.subplots(1, 4, figsize=(18, 5))
    fig.suptitle('FTD HELIUM ATOM: FLUX DENSITY EVOLUTION', fontsize=16,
                 fontweight='bold', color='#00ff88', y=1.02)

    for ax, fpath in zip(axes, selected):
        grid, state_grid, N = load_density_slice(fpath)

        tick = re.search(r't(\d+)', os.path.basename(fpath))
        tick_num = int(tick.group(1)) if tick else 0

        # Custom colormap: dark blue -> cyan -> white for density
        im = ax.imshow(grid, cmap='hot', origin='lower',
                       norm=LogNorm(vmin=max(grid[grid > 0].min(), 1e-4) if np.any(grid > 0) else 1e-4,
                                    vmax=grid.max() if grid.max() > 0 else 1.0),
                       interpolation='bilinear')

        # Overlay particles: protons (+) in blue, electrons (-) in red
        pos_y, pos_x = np.where(state_grid > 0)
        neg_y, neg_x = np.where(state_grid < 0)
        if len(pos_x) > 0:
            ax.scatter(pos_x, pos_y, c='cyan', s=60, marker='^', edgecolors='white',
                       linewidths=0.5, label='proton', zorder=5)
        if len(neg_x) > 0:
            ax.scatter(neg_x, neg_y, c='#ff3366', s=60, marker='o', edgecolors='white',
                       linewidths=0.5, label='electron', zorder=5)

        ax.set_title(f't = {tick_num}', color='#ffaa00', fontsize=12)
        ax.set_xlabel('x')
        if ax == axes[0]:
            ax.set_ylabel('y')
            ax.legend(loc='lower left', fontsize=7, facecolor='#1a1a2e',
                      edgecolor='#333355')

        # Zoom to central region
        mid = N // 2
        margin = max(8, N // 4)
        ax.set_xlim(mid - margin, mid + margin)
        ax.set_ylim(mid - margin, mid + margin)

    plt.tight_layout()
    savepath = os.path.join(figdir, 'fig_helium_evolution.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 4: Helium Atom Timeseries
# =========================================================================

def plot_helium_timeseries(outdir, figdir):
    """Helium atom diagnostics over time."""
    fpath = os.path.join(outdir, 'timeseries.csv')
    if not os.path.exists(fpath):
        print("  No helium timeseries found, skipping.")
        return

    data = load_timeseries(fpath)

    fig, axes = plt.subplots(2, 2, figsize=(14, 8))
    fig.suptitle('FTD HELIUM ATOM: DIAGNOSTIC TIMESERIES', fontsize=16,
                 fontweight='bold', color='#00ff88', y=0.98)

    ticks = data['tick']

    # Panel 1: Total flux
    ax = axes[0, 0]
    ax.plot(ticks, data['total_flux'], color='#00ccff', linewidth=1)
    ax.set_ylabel('Total Flux')
    ax.set_xlabel('Tick')
    ax.set_title('Total Flux |J|', color='#aaaacc')

    # Panel 2: Manifested particles
    ax = axes[0, 1]
    ax.plot(ticks, data['manifested'], color='#ff6600', linewidth=1.5, label='total')
    ax.plot(ticks, data['positive'], color='cyan', linewidth=1, label='+1')
    ax.plot(ticks, data['negative'], color='#ff3366', linewidth=1, label='-1')
    ax.set_ylabel('Count')
    ax.set_xlabel('Tick')
    ax.set_title('Particle Count', color='#aaaacc')
    ax.legend(fontsize=8, facecolor='#1a1a2e', edgecolor='#333355')

    # Panel 3: Total energy
    ax = axes[1, 0]
    ax.plot(ticks, data['total_energy'], color='#ffcc00', linewidth=1)
    ax.set_ylabel('Energy')
    ax.set_xlabel('Tick')
    ax.set_title('Total Energy', color='#aaaacc')

    # Panel 4: sLoop count + entropy
    ax = axes[1, 1]
    ax2 = ax.twinx()
    ln1 = ax.plot(ticks, data['sloop_count'], color='#cc00ff', linewidth=1, label='sLoops')
    ln2 = ax2.plot(ticks, data['total_entropy'], color='#00ff88', linewidth=1, alpha=0.7, label='Entropy')
    ax.set_ylabel('sLoop Count', color='#cc00ff')
    ax2.set_ylabel('Entropy', color='#00ff88')
    ax.set_xlabel('Tick')
    ax.set_title('Self-Reference & Entropy', color='#aaaacc')
    lns = ln1 + ln2
    labs = [l.get_label() for l in lns]
    ax.legend(lns, labs, fontsize=8, facecolor='#1a1a2e', edgecolor='#333355')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    savepath = os.path.join(figdir, 'fig_helium_timeseries.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 5: Pair Production (Scenario J)
# =========================================================================

def plot_pair_production(outdir, figdir):
    """Pair production: early-time density snapshots showing beam collision."""
    files = sorted(glob.glob(os.path.join(outdir, 'pair_slice_t*.csv')),
                   key=lambda f: int(re.search(r't(\d+)', os.path.basename(f)).group(1)))
    if not files:
        print("  No pair production data found, skipping.")
        return

    # Select early-time frames (beam collision is most interesting at t<20)
    early = [f for f in files if int(re.search(r't(\d+)', os.path.basename(f)).group(1)) <= 10]
    late = [f for f in files if int(re.search(r't(\d+)', os.path.basename(f)).group(1)) > 10]

    selected = early[:3] + (late[:3] if late else [])
    n_panels = min(len(selected), 6)

    fig, axes = plt.subplots(2, 3, figsize=(16, 10))
    fig.suptitle('FTD PAIR PRODUCTION: FLUX BEAM COLLISION', fontsize=16,
                 fontweight='bold', color='#ff6600', y=0.98)

    for ax_idx, (ax, fpath) in enumerate(zip(axes.flat, selected)):
        grid, state_grid, N = load_density_slice(fpath)

        tick = re.search(r't(\d+)', os.path.basename(fpath))
        tick_num = int(tick.group(1)) if tick else ax_idx

        im = ax.imshow(grid, cmap='magma', origin='lower',
                       norm=LogNorm(vmin=max(grid[grid > 0].min(), 1e-5) if np.any(grid > 0) else 1e-5,
                                    vmax=grid.max() if grid.max() > 0 else 1.0),
                       interpolation='bilinear')

        # Overlay particles
        pos_y, pos_x = np.where(state_grid > 0)
        neg_y, neg_x = np.where(state_grid < 0)
        if len(pos_x) > 0:
            ax.scatter(pos_x, pos_y, c='cyan', s=80, marker='^', edgecolors='white',
                       linewidths=1, zorder=5)
        if len(neg_x) > 0:
            ax.scatter(neg_x, neg_y, c='#ff3366', s=80, marker='v', edgecolors='white',
                       linewidths=1, zorder=5)

        ax.set_title(f't = {tick_num}', color='#ffaa00', fontsize=12)
        ax.set_xlabel('x')
        if ax_idx % 3 == 0:
            ax.set_ylabel('y')

    for ax in axes.flat[len(selected):]:
        ax.axis('off')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    savepath = os.path.join(figdir, 'fig_pair_production.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 6: Pair Production Timeseries
# =========================================================================

def plot_pair_timeseries(outdir, figdir):
    """Pair production diagnostics over time."""
    # The pair production timeseries is in the same output dir
    fpath = os.path.join(outdir, 'timeseries.csv')
    if not os.path.exists(fpath):
        print("  No pair production timeseries found, skipping.")
        return

    data = load_timeseries(fpath)
    if len(data) < 10:
        print("  Timeseries too short, skipping.")
        return

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle('FTD PAIR PRODUCTION: DIAGNOSTICS', fontsize=16,
                 fontweight='bold', color='#ff6600', y=1.02)

    ticks = data['tick']

    ax = axes[0]
    ax.plot(ticks, data['total_flux'], color='#00ccff', linewidth=1.5)
    ax.set_ylabel('Total Flux')
    ax.set_xlabel('Tick')
    ax.set_title('Flux Field Energy', color='#aaaacc')

    ax = axes[1]
    ax.plot(ticks, data['manifested'], color='white', linewidth=2, label='total')
    ax.plot(ticks, data['positive'], color='cyan', linewidth=1.5, label='matter (+1)')
    ax.plot(ticks, data['negative'], color='#ff3366', linewidth=1.5, label='antimatter (-1)')
    ax.set_ylabel('Particle Count')
    ax.set_xlabel('Tick')
    ax.set_title('Genesis Events', color='#aaaacc')
    ax.legend(fontsize=8, facecolor='#1a1a2e', edgecolor='#333355')

    ax = axes[2]
    ax.plot(ticks, data['total_energy'], color='#ffcc00', linewidth=1.5)
    ax.set_ylabel('Energy')
    ax.set_xlabel('Tick')
    ax.set_title('Total System Energy', color='#aaaacc')

    plt.tight_layout()
    savepath = os.path.join(figdir, 'fig_pair_timeseries.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Figure 7: Composite Dashboard
# =========================================================================

def plot_dashboard(interf_dir, figdir, he_dir=None, pair_dir=None):
    """Single composite figure showing all three scenarios."""
    if he_dir is None: he_dir = interf_dir
    if pair_dir is None: pair_dir = interf_dir

    fig = plt.figure(figsize=(20, 12))
    fig.suptitle('FOUNDATIONAL TERNARY DYNAMICS — SIMULATION DASHBOARD',
                 fontsize=20, fontweight='bold', color='#00ccff', y=0.98)

    # --- Panel A: Interference ---
    ax1 = fig.add_subplot(2, 3, 1)
    fpath = os.path.join(interf_dir, 'slice_xy_final.csv')
    if not os.path.exists(fpath):
        files = sorted(glob.glob(os.path.join(interf_dir, 'slice_z_t*.csv')))
        fpath = files[-1] if files else None

    if fpath and os.path.exists(fpath):
        grid, _, N = load_density_slice(fpath)
        vmin = np.percentile(grid[grid > 0], 5) if np.any(grid > 0) else 1e-6
        ax1.imshow(grid, cmap='inferno', origin='lower',
                   norm=LogNorm(vmin=max(vmin, 1e-6), vmax=max(grid.max(), 1e-5)),
                   interpolation='bilinear')
        mid = N // 2
        half = N // 5
        for sx, sy in [(mid-half, mid-half), (mid+half, mid-half),
                       (mid-half, mid+half), (mid+half, mid+half)]:
            ax1.plot(sx, sy, 'w+', markersize=8, markeredgewidth=1.5)
    ax1.set_title('A. Interference Pattern', color='#ffaa00', fontsize=13, fontweight='bold')

    # --- Panel B: Helium Atom ---
    ax2 = fig.add_subplot(2, 3, 2)
    he_files = sorted(glob.glob(os.path.join(he_dir, 'he_slice_t*.csv')))
    if he_files:
        grid, state_grid, N = load_density_slice(he_files[-1])
        ax2.imshow(grid, cmap='hot', origin='lower',
                   norm=LogNorm(vmin=max(grid[grid > 0].min(), 1e-4) if np.any(grid > 0) else 1e-4,
                                vmax=max(grid.max(), 1e-3)),
                   interpolation='bilinear')
        pos_y, pos_x = np.where(state_grid > 0)
        neg_y, neg_x = np.where(state_grid < 0)
        if len(pos_x): ax2.scatter(pos_x, pos_y, c='cyan', s=40, marker='^', edgecolors='white', linewidths=0.5, zorder=5)
        if len(neg_x): ax2.scatter(neg_x, neg_y, c='#ff3366', s=40, marker='o', edgecolors='white', linewidths=0.5, zorder=5)
        mid = N // 2
        margin = max(8, N // 4)
        ax2.set_xlim(mid - margin, mid + margin)
        ax2.set_ylim(mid - margin, mid + margin)
    ax2.set_title('B. Helium Atom', color='#00ff88', fontsize=13, fontweight='bold')

    # --- Panel C: Pair Production ---
    ax3 = fig.add_subplot(2, 3, 3)
    pair_files = sorted(glob.glob(os.path.join(pair_dir, 'pair_slice_t*.csv')),
                        key=lambda f: int(re.search(r't(\d+)', os.path.basename(f)).group(1)))
    # Pick the frame with most interesting physics (~t=5-10)
    target_file = None
    for f in pair_files:
        t = int(re.search(r't(\d+)', os.path.basename(f)).group(1))
        if 5 <= t <= 10:
            target_file = f
    if not target_file and pair_files:
        target_file = pair_files[min(5, len(pair_files)-1)]

    if target_file:
        grid, state_grid, N = load_density_slice(target_file)
        ax3.imshow(grid, cmap='magma', origin='lower',
                   norm=LogNorm(vmin=max(grid[grid > 0].min(), 1e-5) if np.any(grid > 0) else 1e-5,
                                vmax=max(grid.max(), 1e-4)),
                   interpolation='bilinear')
        pos_y, pos_x = np.where(state_grid > 0)
        neg_y, neg_x = np.where(state_grid < 0)
        if len(pos_x): ax3.scatter(pos_x, pos_y, c='cyan', s=60, marker='^', edgecolors='white', linewidths=1, zorder=5)
        if len(neg_x): ax3.scatter(neg_x, neg_y, c='#ff3366', s=60, marker='v', edgecolors='white', linewidths=1, zorder=5)
    ax3.set_title('C. Pair Production', color='#ff6600', fontsize=13, fontweight='bold')

    # --- Bottom row: timeseries (use helium or pair production)
    ts_path = os.path.join(he_dir, 'timeseries.csv')
    if not os.path.exists(ts_path):
        ts_path = os.path.join(pair_dir, 'timeseries.csv')
    if os.path.exists(ts_path):
        data = load_timeseries(ts_path)
        ticks = data['tick']

        ax4 = fig.add_subplot(2, 3, 4)
        ax4.plot(ticks, data['total_flux'], color='#00ccff', linewidth=1)
        ax4.set_ylabel('Total Flux')
        ax4.set_xlabel('Tick')
        ax4.set_title('D. Flux Field Energy', color='#aaaacc', fontsize=12)

        ax5 = fig.add_subplot(2, 3, 5)
        ax5.plot(ticks, data['manifested'], color='white', linewidth=1.5, label='total')
        ax5.plot(ticks, data['positive'], color='cyan', linewidth=1, label='+1')
        ax5.plot(ticks, data['negative'], color='#ff3366', linewidth=1, label='-1')
        ax5.set_ylabel('Count')
        ax5.set_xlabel('Tick')
        ax5.set_title('E. Particle Genesis', color='#aaaacc', fontsize=12)
        ax5.legend(fontsize=7, facecolor='#1a1a2e', edgecolor='#333355')

        ax6 = fig.add_subplot(2, 3, 6)
        ax6.plot(ticks, data['total_energy'], color='#ffcc00', linewidth=1)
        ax6.set_ylabel('Energy')
        ax6.set_xlabel('Tick')
        ax6.set_title('F. System Energy', color='#aaaacc', fontsize=12)

    # Footer
    fig.text(0.5, 0.01,
             'Foundational Ternary Dynamics | G* = 2.9587 | α⁻¹ = 137.036 | Discrete Lattice Simulation',
             ha='center', color='#666688', fontsize=9, fontstyle='italic')

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    savepath = os.path.join(figdir, 'fig_dashboard.png')
    plt.savefig(savepath, dpi=200, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {savepath}")


# =========================================================================
# Main
# =========================================================================

def main():
    base = os.path.join(os.path.dirname(__file__), 'build', 'Release')

    interf_dir = os.path.join(base, 'output_interf')
    he_dir = os.path.join(base, 'output_he')
    pair_dir = os.path.join(base, 'output_pair')
    # Fallback to combined output/ if separate dirs don't exist
    if not os.path.isdir(interf_dir):
        interf_dir = os.path.join(base, 'output')
    if not os.path.isdir(he_dir):
        he_dir = os.path.join(base, 'output')
    if not os.path.isdir(pair_dir):
        pair_dir = os.path.join(base, 'output')

    figdir = os.path.join(base, 'figures')
    os.makedirs(figdir, exist_ok=True)

    print(f"FTD Simulation Visualizer")
    print(f"  Interference: {interf_dir}")
    print(f"  Helium:       {he_dir}")
    print(f"  Pair prod:    {pair_dir}")
    print(f"  Output figs:  {figdir}")
    print()

    print("Generating figures...")
    plot_interference(interf_dir, figdir)
    plot_interference_heatmap(interf_dir, figdir)
    plot_helium(he_dir, figdir)
    plot_helium_timeseries(he_dir, figdir)
    plot_pair_production(pair_dir, figdir)
    plot_pair_timeseries(pair_dir, figdir)
    plot_dashboard(interf_dir, figdir, he_dir=he_dir, pair_dir=pair_dir)

    print("\nDone! All figures saved.")


if __name__ == '__main__':
    main()
