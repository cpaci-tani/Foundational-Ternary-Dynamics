"""Render quark/color experiment data as static PNG images.

Reads quark_viz_data.json and produces PNG files I can directly read.
"""
from __future__ import annotations
import json
import os
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa
import numpy as np

JSON = Path(r"C:/Users/cpaci/Desktop/ftd/quark_viz_data.json")
OUTDIR = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/quark_pngs")
OUTDIR.mkdir(exist_ok=True, parents=True)

COLOR_MAP = {
    0: "#888888",  # colorless
    1: "#e53935",  # R
    2: "#43a047",  # G
    3: "#1e88e5",  # B
}
COLOR_NAME = {0: "colorless", 1: "R", 2: "G", 3: "B"}

def render_snapshot(snap, L, title, outpath, focus_box=None):
    """Render one 3D scatter snapshot. focus_box=(low,high) for axis range."""
    fig = plt.figure(figsize=(7, 6))
    ax = fig.add_subplot(111, projection="3d")
    coords = snap.get("coords", [])
    if not coords:
        ax.set_title(f"{title}\n(empty)")
        plt.savefig(outpath, dpi=110, bbox_inches="tight")
        plt.close(fig)
        return
    # Group by (state, color)
    groups = {}
    for c in coords:
        key = (c["s"], c["c"])
        groups.setdefault(key, ([], [], [])).__getitem__  # ensure dict entry
    # Actually, the above doesn't work — use proper dict
    groups = {}
    for c in coords:
        key = (c["s"], c["c"])
        if key not in groups:
            groups[key] = ([], [], [])
        groups[key][0].append(c["x"])
        groups[key][1].append(c["y"])
        groups[key][2].append(c["z"])

    # Plot each group
    for (s, col), (xs, ys, zs) in groups.items():
        marker = "o" if s > 0 else "D"
        edgecolor = "black"
        size = 90 if len(coords) < 50 else (30 if len(coords) < 500 else 5)
        label = f"{COLOR_NAME[col]} {'q' if s > 0 else 'q̄'} (n={len(xs)})"
        ax.scatter(xs, ys, zs,
                   c=COLOR_MAP[col], marker=marker, s=size,
                   edgecolors=edgecolor, linewidths=0.5,
                   alpha=0.85 if len(coords) < 200 else 0.4,
                   label=label)
    ax.set_xlabel("x"); ax.set_ylabel("y"); ax.set_zlabel("z")
    if focus_box:
        lo, hi = focus_box
        ax.set_xlim(lo, hi); ax.set_ylim(lo, hi); ax.set_zlim(lo, hi)
    else:
        ax.set_xlim(0, L); ax.set_ylim(0, L); ax.set_zlim(0, L)
    ax.set_title(title, fontsize=11)
    if len(groups) <= 8 and len(coords) < 200:
        ax.legend(loc="upper left", fontsize=8, framealpha=0.85)
    plt.tight_layout()
    plt.savefig(outpath, dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"  -> {outpath.name}: n={len(coords)}")

def render_all(data):
    exp = data["experiments"]

    # Single-color quarks at t=300 (FULL LATTICE — voxels are spread far from center)
    for key in ["red_quark", "green_quark", "blue_quark"]:
        e = exp[key]
        L = e["L"]
        snap = next(s for s in e["snapshots"] if s["tick"] == 300)
        render_snapshot(snap, L, f"{key} at t=300 (full L={L} lattice)",
                        OUTDIR / f"{key}_t300_full.png", focus_box=(0, L))

    # R quark time evolution (full lattice)
    for tick in [0, 30, 100, 300]:
        e = exp["red_quark"]
        L = e["L"]
        snap = next((s for s in e["snapshots"] if s["tick"] == tick), None)
        if snap:
            render_snapshot(snap, L, f"R quark t={tick} (full lattice)",
                            OUTDIR / f"red_quark_t{tick:03d}_full.png", focus_box=(0, L))

    # Diagonal and symmetric (full lattice)
    for key in ["diagonal_xy", "rgb_symmetric"]:
        e = exp[key]
        L = e["L"]
        snap = next(s for s in e["snapshots"] if s["tick"] == 300)
        render_snapshot(snap, L, f"{key} at t=300 (full lattice)",
                        OUTDIR / f"{key}_t300_full.png", focus_box=(0, L))

    # Meson and two-R (full lattice — voxels can be scattered)
    for key in ["meson_RRbar", "two_R_quarks"]:
        e = exp[key]
        L = e["L"]
        snap = next(s for s in e["snapshots"] if s["tick"] == 300)
        render_snapshot(snap, L, f"{key} at t=300 (full lattice)",
                        OUTDIR / f"{key}_t300_full.png", focus_box=(0, L))

    # Baryon experiments — full lattice view because of flooding
    for key in ["baryon_RGB", "baryon_RGB_with_strong", "baryon_RGB_no_color_forces"]:
        e = exp[key]
        L = e["L"]
        snap = next(s for s in e["snapshots"] if s["tick"] == 300)
        # For flooded ones, use full lattice; for collapsed, zoom
        if snap["n_manifested"] > 100:
            render_snapshot(snap, L, f"{key} at t=300 (FLOODED, full lattice)",
                            OUTDIR / f"{key}_t300_full.png", focus_box=(0, L))
        else:
            c = L // 2
            render_snapshot(snap, L, f"{key} at t=300 (zoomed)",
                            OUTDIR / f"{key}_t300_zoom.png", focus_box=(c-8, c+8))

def render_color_size_chart(data):
    """Bar chart: cluster size at t=300 vs color."""
    exp = data["experiments"]
    keys = ["red_quark", "green_quark", "blue_quark"]
    sizes = []
    for k in keys:
        snap = next(s for s in exp[k]["snapshots"] if s["tick"] == 300)
        sizes.append(snap["n_manifested"])
    fig, ax = plt.subplots(figsize=(7, 4.5))
    bars = ax.bar(["R (pure +x)", "G (pure +y)", "B (pure +z)"], sizes,
                  color=["#e53935", "#43a047", "#1e88e5"], edgecolor="black")
    for bar, s in zip(bars, sizes):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.05,
                f"n={s}", ha="center", fontsize=12)
    ax.set_ylabel("Cluster size at t=300")
    ax.set_title("Color asymmetry: R/G/B cluster sizes from identical-magnitude flux\n"
                 "(Moore Layer Theorem says these should be equal)")
    ax.set_ylim(0, 5.5)
    plt.tight_layout()
    plt.savefig(OUTDIR / "color_asymmetry.png", dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"  -> color_asymmetry.png: R={sizes[0]} G={sizes[1]} B={sizes[2]}")

def main():
    with JSON.open() as f:
        data = json.load(f)
    print(f"Loaded {len(data['experiments'])} experiments")
    print(f"Output dir: {OUTDIR}")
    render_all(data)
    render_color_size_chart(data)
    print("\nAll PNGs written.")
    print(f"Files: {[p.name for p in sorted(OUTDIR.glob('*.png'))]}")

if __name__ == "__main__":
    main()
