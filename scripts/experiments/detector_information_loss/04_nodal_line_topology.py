"""
04_nodal_line_topology.py — The skeleton of interference is invisible

WHY THIS MATTERS:
The zero-contours of Re(psi) and Im(psi) form intricate networks that
encode the full source geometry.  Where Re=0 and Im=0 cross, psi=0
(a vortex core).  |psi|^2 collapses both networks into "dark fringes"
— a single scalar threshold — destroying the Im=0 network entirely
and losing all topological invariants (crossing count, linking numbers,
genus of the nodal surface).

Quantifies: contour counts, arc lengths, what |psi|^2 retains vs loses.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, amplitude_field,
    setup_style, make_figure, save_json, DEFAULTS,
)


def extract_contour_stats(ax, field, level=0.0, color='red'):
    """Extract contour at given level, return (contour_collection, n_segments, total_arc_length).

    Uses allsegs for accurate per-segment counting (get_paths() merges into
    one compound path in matplotlib 3.8+).
    """
    cs = ax.contour(field, levels=[level], colors=[color], linewidths=0.5)
    n_segments = 0
    total_length = 0.0

    if hasattr(cs, 'allsegs'):
        for seg_list in cs.allsegs:
            for seg in seg_list:
                n_segments += 1
                if len(seg) > 1:
                    diffs = np.diff(seg, axis=0)
                    total_length += np.sum(np.sqrt(diffs[:, 0] ** 2 + diffs[:, 1] ** 2))
    else:
        # Fallback for older matplotlib
        for collection in cs.collections:
            for path in collection.get_paths():
                n_segments += 1
                verts = path.vertices
                if len(verts) > 1:
                    diffs = np.diff(verts, axis=0)
                    total_length += np.sum(np.sqrt(diffs[:, 0] ** 2 + diffs[:, 1] ** 2))
    return cs, n_segments, total_length


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("04  NODAL LINE TOPOLOGY — The skeleton of interference")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Zero-contours of Re(psi) and Im(psi) form intricate networks")
    print("  encoding source geometry.  |psi|^2 collapses both into 'dark")
    print("  fringes' — the Im=0 network is lost entirely, along with all")
    print("  topological invariants (crossing count, linking numbers).")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    amp = amplitude_field(psi_re, psi_im)
    born = born_rule(psi_re, psi_im)
    born_norm = born / max(born.max(), 1e-12)

    # 2. Extract contours using matplotlib
    print("  Extracting nodal contours ...")
    plt = setup_style()

    # Invisible figure for contour extraction only
    fig_tmp, ax_tmp = plt.subplots()
    cs_re, n_re, len_re = extract_contour_stats(ax_tmp, psi_re, level=0.0, color='red')
    plt.close(fig_tmp)

    fig_tmp, ax_tmp = plt.subplots()
    cs_im, n_im, len_im = extract_contour_stats(ax_tmp, psi_im, level=0.0, color='blue')
    plt.close(fig_tmp)

    # Dark fringes in |psi|^2: contour at low threshold
    dark_threshold = 0.02 * born.max() if born.max() > 0 else 0.01
    fig_tmp, ax_tmp = plt.subplots()
    cs_dark, n_dark, len_dark = extract_contour_stats(
        ax_tmp, born, level=dark_threshold, color='yellow')
    plt.close(fig_tmp)

    print(f"  Re(psi) = 0 contours:  {n_re} segments,  arc length = {len_re:.1f} px")
    print(f"  Im(psi) = 0 contours:  {n_im} segments,  arc length = {len_im:.1f} px")
    print(f"  |psi|^2 dark fringes:  {n_dark} segments,  arc length = {len_dark:.1f} px")
    print(f"  Total nodal (Re+Im):   {n_re + n_im} segments,  {len_re + len_im:.1f} px")
    print(f"  Lost by |psi|^2:       Im=0 network ({n_im} segments, {len_im:.1f} px)")
    print()

    # ------------------------------------------------------------------
    # Figure

    # Panel (a): Re=0 (red) + Im=0 (blue) overlaid on faint amplitude
    amp_bg = np.where(amp > 0, amp / max(amp.max(), 1e-12) * 0.25, 0)

    def overlay_both_nodal(ax):
        ax.contour(psi_re, levels=[0], colors=['#ff4444'], linewidths=0.6, alpha=0.9)
        ax.contour(psi_im, levels=[0], colors=['#4488ff'], linewidths=0.6, alpha=0.9)
        ax.text(0.02, 0.96, 'Red = Re(psi)=0', transform=ax.transAxes,
                fontsize=7, color='#ff4444', va='top')
        ax.text(0.02, 0.90, 'Blue = Im(psi)=0', transform=ax.transAxes,
                fontsize=7, color='#4488ff', va='top')

    # Panel (b): |psi|^2 with dark fringe contours only
    def overlay_dark_fringes(ax):
        ax.contour(born, levels=[dark_threshold], colors=['#ffcc66'],
                   linewidths=0.6, alpha=0.9)
        ax.text(0.02, 0.96, f'Dark fringes (thr={dark_threshold:.4f})',
                transform=ax.transAxes, fontsize=7, color='#ffcc66', va='top')

    # Panel (c): overlay showing what is lost — Re=0 retained (faint), Im=0 highlighted
    def overlay_lost(ax):
        # Show Re=0 faintly (retained as dark fringes)
        ax.contour(psi_re, levels=[0], colors=['#ff4444'], linewidths=0.3, alpha=0.4)
        # Im=0 bold — this is what is LOST
        ax.contour(psi_im, levels=[0], colors=['#44ff88'], linewidths=1.0, alpha=0.95)
        ax.text(0.02, 0.96, 'GREEN = Im(psi)=0 (LOST)', transform=ax.transAxes,
                fontsize=7, color='#44ff88', va='top', fontweight='bold')
        ax.text(0.02, 0.90, 'faint red = Re(psi)=0 (partially retained)',
                transform=ax.transAxes, fontsize=7, color='#ff4444', va='top')

    # Panel (d): statistics comparison
    def stats_overlay(ax):
        ax.clear()
        ax.set_facecolor('#0a0a0f')

        categories = ['Re=0\nsegments', 'Im=0\nsegments', 'Dark fringe\nsegments',
                       'Re=0\narc (px)', 'Im=0\narc (px)', 'Dark fringe\narc (px)']
        vals = [n_re, n_im, n_dark, len_re, len_im, len_dark]

        # Two groups of bars: counts (left) and arc lengths (right)
        ax2 = ax.twinx()

        x = np.array([0, 1, 2])
        width = 0.35

        # Segment counts
        bars1 = ax.bar(x - width / 2, [n_re, n_im, n_dark], width,
                        color=['#ff4444', '#4488ff', '#ffcc66'],
                        edgecolor='none', label='Segments')
        # Arc lengths (normalized to fit)
        max_len = max(len_re, len_im, len_dark, 1)
        bars2 = ax2.bar(x + width / 2, [len_re, len_im, len_dark], width,
                         color=['#ff8888', '#88aaff', '#ffdd88'],
                         edgecolor='none', alpha=0.7, label='Arc length')

        ax.set_xticks(x)
        ax.set_xticklabels(['Re=0', 'Im=0', '|psi|^2\nfringes'], fontsize=7)
        ax.set_ylabel('Segment count', fontsize=7, color='#ff4444')
        ax2.set_ylabel('Arc length (px)', fontsize=7, color='#88aaff')
        ax.set_title('Nodal line statistics', color='#c8d8e8', fontsize=9)
        ax.tick_params(colors='#667788')
        ax2.tick_params(colors='#667788')

        # Annotate
        for bar, v in zip(bars1, [n_re, n_im, n_dark]):
            ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                    str(v), ha='center', va='bottom', color='#c8d8e8', fontsize=7)

    metrics_text = (
        f"Re=0: {n_re} segs, {len_re:.0f} px  |  "
        f"Im=0: {n_im} segs, {len_im:.0f} px  |  "
        f"|psi|^2 fringes: {n_dark} segs, {len_dark:.0f} px  |  "
        f"LOST: Im=0 network ({n_im} segs, {len_im:.0f} px)"
    )

    from experiments.detector_information_loss.field_engine import (
        OUTPUT_DIR, BG_COLOR, TEXT_COLOR, SUBTLE_COLOR, ACCENT_COLOR, GRID_COLOR,
    )

    fig, axes = plt.subplots(2, 2, figsize=(12, 12))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle("Nodal Line Topology: The Skeleton of Interference is Invisible",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.97)

    # (a) Re=0 + Im=0 on faint amplitude
    ax = axes[0, 0]
    ax.imshow(amp_bg, origin='lower', cmap='gray', aspect='equal')
    overlay_both_nodal(ax)
    ax.set_title('(a) Re=0 (red) + Im=0 (blue) nodal lines',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([]); ax.set_yticks([])

    # (b) |psi|^2 with dark fringes
    ax = axes[0, 1]
    ax.imshow(born_norm, origin='lower', cmap='gray', aspect='equal')
    overlay_dark_fringes(ax)
    ax.set_title('(b) |psi|^2 with dark-fringe contours',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([]); ax.set_yticks([])

    # (c) What |psi|^2 loses
    ax = axes[1, 0]
    ax.imshow(amp_bg, origin='lower', cmap='gray', aspect='equal')
    overlay_lost(ax)
    ax.set_title('(c) What |psi|^2 loses (green = Im=0)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([]); ax.set_yticks([])

    # (d) Statistics bar chart
    ax = axes[1, 1]
    stats_overlay(ax)

    fig.text(0.5, 0.02, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])
    outpath = OUTPUT_DIR / '04_nodal_line_topology.png'
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '04_nodal_line_topology.py',
        'description': 'Nodal line (Re=0, Im=0) topology vs dark fringe contours in |psi|^2',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'Re_zero_segments': int(n_re),
            'Re_zero_arc_length_px': float(len_re),
            'Im_zero_segments': int(n_im),
            'Im_zero_arc_length_px': float(len_im),
            'total_nodal_segments': int(n_re + n_im),
            'total_nodal_arc_length_px': float(len_re + len_im),
            'dark_fringe_segments': int(n_dark),
            'dark_fringe_arc_length_px': float(len_dark),
            'dark_fringe_threshold': float(dark_threshold),
            'Im_zero_lost_segments': int(n_im),
            'Im_zero_lost_arc_length_px': float(len_im),
            'fraction_topology_lost': float(n_im / max(n_re + n_im, 1)),
        },
    }
    save_json('04_nodal_line_topology', summary)

    print(f"  RESULT: The full field has {n_re + n_im} nodal segments")
    print(f"  ({len_re + len_im:.0f} px total arc length).")
    print(f"  |psi|^2 retains only {n_dark} dark-fringe segments.")
    print(f"  The Im=0 network ({n_im} segments, {len_im:.0f} px) is completely lost.")
    print()


if __name__ == '__main__':
    main()
