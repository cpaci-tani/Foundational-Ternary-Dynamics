#!/usr/bin/env python3
r"""
viz_ultralocality_map.py — the UL (ultralocality) map: a Moore-shell
hierarchy of a 3D weight field, laid flat.

INSTRUMENT ONLY — [MEASUREMENT INFRASTRUCTURE]. This script defines a
visualization/diagnostic; it makes no physics claim and mints no LEDGER row.
Before any UL-map output is used as *evidence* for or against the Phase-J
ultralocality conjecture (Theorem 7: [THEOREM at L=2] + [CONJECTURE general
L]; see SPEC_ALGEBRAIC_SPINE.md and the FTD-0350 zero-mode analysis), the
measurement must be pre-registered per the project discipline.

Definition
----------
Around a center voxel c of a 3D field W (nonnegative weights — e.g. |K|,
|corr|, |response|):

  UL_x   := Moore neighborhood of radius x = { v : d_cheb(v, c) <= x }
            (Moore neighborhoods ARE Chebyshev balls; side 2x+1, so
             UL_0 = 1^3 single voxel, UL_1 = 3^3 = 27, UL_2 = 5^3 = 125, ...
             — level index grows +1, cube side grows +2, always odd so a
             true center voxel exists, matching the engine convention)
  S_x    := UL_x \ UL_{x-1}, the shell of NEW sites at level x
            (|S_0| = 1; |S_x| = (2x+1)^3 - (2x-1)^3 = 26, 98, 218, ...)
  w_x    := sum of W over S_x            (per-shell weight)
  F_x    := (sum_{y<=x} w_y) / (sum W)   (cumulative fraction inside UL_x)
  eps_x  := 1 - F_x                      (tail weight OUTSIDE UL_x)

Ultralocality diagnostic: "W is ultralocal at level x0 with tolerance eps"
  <=>  eps_{x0} < eps.
For the Phase-J claim the interesting statement is eps_1 ~ 0 (all weight in
UL_1) at every lattice size L.

Flat layout ("contact sheet"): level row x shows the (2x+1) z-slices of the
UL_x sub-cube left-to-right; shell sites S_x in full color, interior sites
(already shown in earlier rows) dimmed. Log color scale (decays span many
decades).

Usage
-----
  python viz_ultralocality_map.py --demo            # 3 synthetic kernels
  python viz_ultralocality_map.py field.npy         # your own (2R+1)^3 array
  python viz_ultralocality_map.py field.npy --center i j k --levels 4

Input .npy: a 3D array; --center defaults to the array's central index
(array sides must be odd in that case). Values are used as |W|.
"""

import argparse
import sys

import numpy as np


# ── Core structure ──────────────────────────────────────────────────────────

def chebyshev_grid(shape, center):
    """d_cheb(v, center) for every site of a 3D array."""
    zi, yi, xi = np.indices(shape)
    return np.maximum.reduce([
        np.abs(zi - center[0]),
        np.abs(yi - center[1]),
        np.abs(xi - center[2]),
    ])


def ul_decompose(W, center=None, levels=None):
    """Decompose |W| into Moore shells around `center`.

    Returns dict with:
      levels        : int, max UL level analysed
      shell_weight  : w_x per level (array, length levels+1)
      shell_sites   : |S_x| per level
      cum_fraction  : F_x per level
      tail          : eps_x per level
    """
    W = np.abs(np.asarray(W, dtype=float))
    if W.ndim != 3:
        raise ValueError("W must be a 3D array")
    if center is None:
        if any(s % 2 == 0 for s in W.shape):
            raise ValueError("even-sided array has no central voxel — pass --center")
        center = tuple(s // 2 for s in W.shape)
    d = chebyshev_grid(W.shape, center)
    max_level = int(d.max()) if levels is None else int(levels)

    total = W.sum()
    if total <= 0:
        raise ValueError("W has no weight")

    shell_weight, shell_sites = [], []
    for x in range(max_level + 1):
        mask = d == x
        shell_weight.append(W[mask].sum())
        shell_sites.append(int(mask.sum()))
    shell_weight = np.array(shell_weight)
    cum = np.cumsum(shell_weight) / total
    return {
        "levels": max_level,
        "center": center,
        "shell_weight": shell_weight,
        "shell_sites": np.array(shell_sites),
        "cum_fraction": cum,
        "tail": 1.0 - cum,
    }


# ── Flat rendering ──────────────────────────────────────────────────────────

def render_ul_map(ax, W, center=None, levels=None, cmap="magma",
                  floor_decades=8):
    """Draw the flat UL map onto a matplotlib Axes.

    Level row x: the (2x+1) z-slices of the UL_x sub-cube, left-to-right,
    shell sites at full alpha, interior dimmed. Log10 color, clipped
    `floor_decades` below the max.
    """
    import matplotlib.colors as mcolors
    import matplotlib.pyplot as plt  # noqa: F401  (colormap registry)

    W = np.abs(np.asarray(W, dtype=float))
    if center is None:
        center = tuple(s // 2 for s in W.shape)
    d_full = chebyshev_grid(W.shape, center)
    R = int(d_full.max()) if levels is None else int(levels)

    vmax = W.max()
    vmin = vmax * 10.0 ** (-floor_decades)
    norm = mcolors.LogNorm(vmin=vmin, vmax=vmax)
    cmap_obj = plt.get_cmap(cmap)

    pad = 1                      # gap between tiles / rows (in cells)
    row_tops = []
    y_cursor = 0
    # Widest row is level R: (2R+1) tiles of side (2R+1) plus gaps.
    canvas_w = (2 * R + 1) * (2 * R + 1) + pad * (2 * R)
    rows_h = sum((2 * x + 1) + pad for x in range(R + 1)) - pad
    canvas = np.zeros((rows_h, canvas_w, 4))          # RGBA, transparent

    cz, cy, cx = center
    for x in range(R + 1):
        side = 2 * x + 1
        row_tops.append(y_cursor)
        # sub-cube of UL_x, clipped to array bounds
        z0, z1 = cz - x, cz + x + 1
        y0, y1 = cy - x, cy + x + 1
        x0, x1 = cx - x, cx + x + 1
        if min(z0, y0, x0) < 0 or z1 > W.shape[0] or y1 > W.shape[1] or x1 > W.shape[2]:
            break  # level exceeds the supplied array
        sub = W[z0:z1, y0:y1, x0:x1]
        dsub = chebyshev_grid(sub.shape, (x, x, x))
        row_w = side * side + pad * (side - 1)
        x_cursor = (canvas_w - row_w) // 2            # center the row
        for zi in range(side):
            tile = sub[zi]
            rgba = cmap_obj(norm(np.clip(tile, vmin, vmax)))
            interior = dsub[zi] < x                    # already shown above
            rgba[interior, 3] = 0.25                   # dim, don't hide
            canvas[y_cursor:y_cursor + side,
                   x_cursor:x_cursor + side] = rgba
            x_cursor += side + pad
        y_cursor += side + pad

    ax.imshow(canvas, origin="upper", interpolation="nearest")
    for x, top in enumerate(row_tops[:R + 1]):
        ax.text(-2, top + x + 0.5, f"UL$_{{{x}}}$", ha="right", va="center",
                fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    for s in ax.spines.values():
        s.set_visible(False)
    return norm, cmap_obj


# ── Synthetic demo kernels (clearly labeled SYNTHETIC) ─────────────────────

def demo_kernels(R=4):
    side = 2 * R + 1
    c = (R, R, R)
    d_cheb = chebyshev_grid((side,) * 3, c)
    zi, yi, xi = np.indices((side,) * 3)
    r_eu = np.sqrt((zi - R) ** 2 + (yi - R) ** 2 + (xi - R) ** 2)
    kernels = {
        "ultralocal (delta + 1e-2 NN)": np.where(d_cheb == 0, 1.0,
                                        np.where(d_cheb == 1, 1e-2, 1e-9)),
        "exponential (lambda=1)": np.exp(-d_cheb / 1.0),
        "power law (1+r)^-3": (1.0 + r_eu) ** -3,
    }
    return kernels


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[1])
    ap.add_argument("field", nargs="?", help=".npy 3D array of weights")
    ap.add_argument("--demo", action="store_true",
                    help="render three SYNTHETIC kernels side by side")
    ap.add_argument("--center", nargs=3, type=int, metavar=("Z", "Y", "X"))
    ap.add_argument("--levels", type=int, default=None)
    ap.add_argument("--out", default="ul_map.png")
    args = ap.parse_args()

    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    if args.demo or not args.field:
        kernels = demo_kernels(R=4)
        fig = plt.figure(figsize=(13, 7.5))
        gs = fig.add_gridspec(2, len(kernels), height_ratios=[2.2, 1.0],
                              hspace=0.25)
        for i, (name, W) in enumerate(kernels.items()):
            ax = fig.add_subplot(gs[0, i])
            render_ul_map(ax, W, levels=args.levels)
            ax.set_title(f"SYNTHETIC: {name}", fontsize=10)
            dec = ul_decompose(W, levels=args.levels)
            axd = fig.add_subplot(gs[1, i])
            lv = np.arange(dec["levels"] + 1)
            axd.semilogy(lv, np.maximum(dec["tail"], 1e-16), "o-",
                         label=r"tail  $\varepsilon_x$")
            axd.semilogy(lv, dec["shell_weight"] / dec["shell_weight"].sum(),
                         "s--", label=r"shell frac  $w_x/\Sigma w$")
            axd.set_xlabel("UL level $x$")
            axd.set_xticks(lv)
            axd.grid(alpha=0.3)
            axd.legend(fontsize=8)
            print(f"{name}:")
            for x in lv:
                print(f"  UL_{x}: |S_x|={dec['shell_sites'][x]:4d}  "
                      f"F_x={dec['cum_fraction'][x]:.6f}  "
                      f"eps_x={dec['tail'][x]:.3e}")
        fig.suptitle("UL map — Moore-shell hierarchy laid flat "
                     "(rows: UL$_x$ cubes as z-slice contact sheets; "
                     "shell sites bright, interior dimmed; log color)",
                     fontsize=11)
    else:
        W = np.load(args.field)
        center = tuple(args.center) if args.center else None
        dec = ul_decompose(W, center=center, levels=args.levels)
        fig = plt.figure(figsize=(9, 8))
        gs = fig.add_gridspec(2, 1, height_ratios=[2.4, 1.0], hspace=0.25)
        ax = fig.add_subplot(gs[0])
        render_ul_map(ax, W, center=center, levels=args.levels)
        ax.set_title(args.field, fontsize=10)
        axd = fig.add_subplot(gs[1])
        lv = np.arange(dec["levels"] + 1)
        axd.semilogy(lv, np.maximum(dec["tail"], 1e-16), "o-",
                     label=r"tail $\varepsilon_x$")
        axd.set_xlabel("UL level $x$")
        axd.grid(alpha=0.3)
        axd.legend()
        for x in lv:
            print(f"UL_{x}: |S_x|={dec['shell_sites'][x]:5d}  "
                  f"F_x={dec['cum_fraction'][x]:.6f}  "
                  f"eps_x={dec['tail'][x]:.3e}")

    fig.savefig(args.out, dpi=150, bbox_inches="tight")
    print(f"\nwrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
