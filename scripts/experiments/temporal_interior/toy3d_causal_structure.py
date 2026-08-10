"""toy3d_causal_structure.py — strict reach and effective-cone bookkeeping
around one event in a three-dimensional discrete spacetime.

THE QUESTION.  A local dispersive update has both a strict dependency
support and a long-wave propagation scale.  Splitting their difference
produces four useful bookkeeping regions.  This is not a unique four-region
causal ontology: dispersive continuum systems can have the same distinction.

  REACH      the update rule touches a finite neighbour set each tick, so
             after t ticks the field is EXACTLY zero outside the t-fold
             dilation of the causal polytope.  This bound is unconditional
             -- it is locality, not dynamics.
  EFFECTIVE  the long-wave speed scale is C = 1/sqrt(3), so a smooth packet
             is concentrated near distance C*t.  This is not a support bound.

Between them sits a shell that is formally reachable but outside the
effective cone.  This script measures the precursor present there in the
declared point-source run.

WHAT IS COMPUTED (nothing is drawn that is not first computed):
  1. GEOMETRY   the three candidate causal polytopes, their inradii and
                circumradii, and the light-cone sphere at C*t.
  2. CENSUS     for each t, how many lattice sites are inside the cone,
                in the precursor shell, and unreachable.
  3. VELOCITY   an analytic group-velocity formula sampled on a 181^3 grid
                of the Brillouin zone.  This is evidence about, not a proof
                of, the global supremum.  In any case the effective cone is
                NOT the strict front: a supremum over group velocities
                bounds where the disturbance is concentrated, not where
                it is exactly zero.  The strict front is the reach
                boundary; the shell between them holds a precursor.
  4. LEAKAGE    a real 3-D leapfrog run from a point source,
                measuring the amplitude that actually reaches the
                precursor shell.

THE HEADLINE, established below: at t = 1 the effective cone contains no
lattice site but the origin, while the update rule has already touched its
18 neighbours.  Those non-origin dependencies are outside the effective
cone but inside the exact support.  This is distinguishable immediately;
the resolution is the strict/effective distinction shown in panel (e).
"""
from __future__ import annotations

from pathlib import Path
import itertools

import numpy as np
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection
from scipy.spatial import ConvexHull


C_CONE = 1.0 / np.sqrt(3.0)

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, C2, C3, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks




# =====================================================================
# 1. GEOMETRY — the three candidate causal polytopes
# =====================================================================
SHELL = {n: [p for p in itertools.product([-1, 0, 1], repeat=3)
             if sum(abs(x) for x in p) == n] for n in (1, 2, 3)}
FACE, EDGE, CORNER = SHELL[1], SHELL[2], SHELL[3]

POLY = {
    "octahedron (6)":     np.array(FACE, float),
    "cuboctahedron (18)": np.array(EDGE, float),
    "cube (26)":          np.array(CORNER, float),
}


def polytope_facets(pts):
    """Exact facets of the convex hull, merged into coplanar faces."""
    hull = ConvexHull(pts)
    planes = {}
    for eq, simplex in zip(hull.equations, hull.simplices):
        key = tuple(np.round(eq, 9))
        planes.setdefault(key, set()).update(simplex.tolist())
    faces = []
    for (a, b, c, d), idx in planes.items():
        n = np.array([a, b, c])
        P = pts[sorted(idx)]
        ctr = P.mean(axis=0)
        u = P[0] - ctr
        u /= np.linalg.norm(u)
        v = np.cross(n, u)
        ang = np.arctan2((P - ctr) @ v, (P - ctr) @ u)
        faces.append(P[np.argsort(ang)])
    return faces, hull


def inradius(pts):
    """min over facets of the distance from the origin (support function)."""
    _, hull = polytope_facets(pts)
    return min(-eq[3] for eq in hull.equations)


def reach_ticks(n):
    """M18 lattice distance: the fewest ticks in which the update rule can
    touch site n.  The 18-reach polytope is {|x|_inf <= 1} & {|x|_1 <= 2},
    so its gauge is max(|n|_inf, ceil(|n|_1 / 2))."""
    n = np.abs(np.asarray(n))
    return max(int(n.max()), int(np.ceil(n.sum() / 2)))


def census(tmax=10, R=26):
    """Site counts by causal region, per tick."""
    g = np.arange(-R, R + 1)
    X, Y, Z = np.meshgrid(g, g, g, indexing="ij")
    P = np.stack([X, Y, Z], -1).reshape(-1, 3)
    rad = np.linalg.norm(P, axis=1)
    linf = np.abs(P).max(axis=1)
    l1 = np.abs(P).sum(axis=1)
    dreach = np.maximum(linf, np.ceil(l1 / 2)).astype(int)
    rows = []
    for t in range(1, tmax + 1):
        reach = dreach <= t
        incone = rad <= C_CONE * t + 1e-12
        rows.append(dict(t=t, cone=int((incone & reach).sum()),
                         prec=int((reach & ~incone).sum()),
                         reach=int(reach.sum())))
        assert not (incone & ~reach).any(), "light cone escaped the reach"
    return rows


# =====================================================================
# 2. VELOCITY — analytic formula evaluated on a finite Brillouin-zone grid
# =====================================================================
def symbol_L(k):
    c = np.cos(k)
    return (2 / 3) * c.sum(-1) + (2 / 3) * (
        c[..., 0] * c[..., 1] + c[..., 1] * c[..., 2] + c[..., 2] * c[..., 0]
    ) - 4


def group_velocity(k):
    """|grad_k Omega| for the leapfrog Omega = 2 asin((C/2) sqrt(-L)),
    differentiated analytically.  dL/dk_i = -(2/3) sin k_i (1 + c_j + c_l)."""
    c, s = np.cos(k), np.sin(k)
    mL = -symbol_L(k)
    S = (C_CONE / 2) * np.sqrt(np.maximum(mL, 0))
    j = [(1, 2), (2, 0), (0, 1)]
    grad = np.stack([
        (C_CONE / 3) * s[..., i] * (1 + c[..., a] + c[..., b])
        / (np.sqrt(np.maximum(1 - S ** 2, 1e-300))
           * np.sqrt(np.maximum(mL, 1e-300)))
        for i, (a, b) in enumerate(j)], -1)
    return np.linalg.norm(grad, axis=-1)


def max_group_velocity(n=181):
    """Scan the irreducible zone; k=0 is excluded (removable, limit = C)."""
    ax = np.linspace(0, np.pi, n)
    K1, K2, K3 = np.meshgrid(ax, ax, ax, indexing="ij")
    k = np.stack([K1, K2, K3], -1)
    v = group_velocity(k)
    v[0, 0, 0] = 0.0                       # removable singularity
    i = np.unravel_index(np.argmax(v), v.shape)
    return v.max(), k[i], v


# =====================================================================
# 3. LEAKAGE — a real leapfrog run, point source
# =====================================================================
def greens_profile(ticks, R=26, nbin=1.0):
    """Radial profile of |phi| for a POINT source after `ticks` ticks.

    A point source is the only unambiguous probe here: its initial support
    is one site, so the strict support at time t is exactly the reach
    polytope and no padding convention is needed.  (A Gaussian source has
    infinite tails, and measuring "beyond the cone" then silently clips
    the front's own shoulder -- a mistake worth naming, since it reads as
    a few percent of spurious leakage.)"""
    g = np.arange(-R, R + 1)
    N = len(g)
    X, Y, Z = np.meshgrid(g, g, g, indexing="ij")
    rad = np.sqrt(X * X + Y * Y + Z * Z)
    phi = np.zeros((N, N, N))
    phi[R, R, R] = 1.0
    prev = phi.copy()                       # at rest: phi(-1) = phi(0)

    def lap(f):
        out = -4.0 * f
        for ax in (0, 1, 2):
            out += (np.roll(f, 1, ax) + np.roll(f, -1, ax)) / 3.0
        for a, b in ((0, 1), (1, 2), (2, 0)):
            for sa in (1, -1):
                for sb in (1, -1):
                    out += np.roll(np.roll(f, sa, a), sb, b) / 6.0
        return out

    for _ in range(ticks):
        phi, prev = 2 * phi - prev + C_CONE ** 2 * lap(phi), phi

    a = np.abs(phi)
    # locality is exact: nothing may exist beyond the reach circumradius
    beyond = a[rad > np.sqrt(2) * ticks + 1e-9]
    assert beyond.size == 0 or beyond.max() == 0.0, "locality violated"

    edges = np.arange(0, np.sqrt(2) * ticks + 2 * nbin, nbin)
    rc, am = [], []
    for lo in edges[:-1]:
        m = (rad >= lo) & (rad < lo + nbin)
        if m.sum():
            rc.append(lo + nbin / 2)
            am.append(a[m].max())
    return np.array(rc), np.array(am)


# =====================================================================
# 4. PANELS
# =====================================================================
def draw_polytope(ax, pts, color, alpha, lw, ls="-"):
    faces, _ = polytope_facets(pts)
    ax.add_collection3d(Poly3DCollection(
        faces, facecolor=color, alpha=alpha, edgecolor="none", zorder=1))
    segs = []
    for f in faces:
        segs += [[f[i], f[(i + 1) % len(f)]] for i in range(len(f))]
    ax.add_collection3d(Line3DCollection(
        segs, colors=color, linewidths=lw, linestyles=ls, zorder=3))


def sphere(ax, r, color, alpha):
    u = np.linspace(0, 2 * np.pi, 60)
    v = np.linspace(0, np.pi, 32)
    ax.plot_surface(r * np.outer(np.cos(u), np.sin(v)),
                    r * np.outer(np.sin(u), np.sin(v)),
                    r * np.outer(np.ones_like(u), np.cos(v)),
                    color=color, alpha=alpha, linewidth=0, shade=True,
                    zorder=2, antialiased=True)


def panel_cell(ax):
    """(a) the t=1 causal cell in 3-D.

    Three objects only, and one exact tangency: the octahedron's inradius
    IS the cone speed, so its eight faces touch the effective-cone sphere."""
    draw_polytope(ax, POLY["cuboctahedron (18)"], C1, 0.0, 0.85)
    draw_polytope(ax, POLY["octahedron (6)"], CG, 0.0, 0.7, ls=":")
    sphere(ax, C_CONE, C2, 0.80)
    pts = np.array(FACE + EDGE, float)
    ax.scatter(*pts.T, s=11, color=C1, depthshade=False)
    ax.scatter([0], [0], [0], s=17, color=CK, depthshade=False)
    style_3d(ax, 1.30)
    for i, (txt, col) in enumerate([
            ("effective cone, radius $C$", C2),
            ("octahedron: faces tangent", CG),
            ("reach at $t=1$: 18 sites", C1)]):
        ax.text2D(0.02, 0.145 - 0.058 * i, txt, transform=ax.transAxes,
                  fontsize=FS_ANN, color=col, ha="left", va="center")
    ax.set_title("(a)  the causal cell at $t=1$: 18 sites\n"
                 "touched, every one outside the cone")


def panel_minkowski(ax):
    """(b) past / present / future / elsewhere, in (x, y, t)."""
    T = 2.0
    for sgn in (+1, -1):
        # reach: square-based pyramid (the z=0 section of the cuboctahedron)
        q = [(-1, -1), (1, -1), (1, 1), (-1, 1)]
        top = [(T * a, T * b, sgn * T) for a, b in q]
        ax.add_collection3d(Poly3DCollection(
            [top], facecolor=C1, alpha=0.05, edgecolor="none"))
        segs = [[(0, 0, 0), p] for p in top]
        segs += [[top[i], top[(i + 1) % 4]] for i in range(4)]
        ax.add_collection3d(Line3DCollection(segs, colors=C1, linewidths=0.9))
        # long-wave effective cone: circular
        th = np.linspace(0, 2 * np.pi, 80)
        tt = np.linspace(0, T, 26)
        TT, TH = np.meshgrid(tt, th, indexing="ij")
        RR = C_CONE * TT
        ax.plot_surface(RR * np.cos(TH), RR * np.sin(TH), sgn * TT,
                        color=C2, alpha=0.34, linewidth=0, shade=True)
    ax.scatter([0], [0], [0], s=20, color=CK, depthshade=False)
    # no z-label: it lands on top of the left "elsewhere"; future/past
    # already orient the time axis, and the title names it
    style_3d(ax, 2.30)
    for x, y, txt, col in [(0.50, 0.955, "future", CK),
                           (0.50, 0.055, "past", CK),
                           (0.05, 0.505, "elsewhere", CG),
                           (0.95, 0.505, "elsewhere", CG)]:
        ax.text2D(x, y, txt, transform=ax.transAxes, fontsize=FS_ANN,
                  color=col, ha="center", va="center")
    ax.text2D(0.02, 0.145, "effective cone $C|t|$", transform=ax.transAxes,
              fontsize=FS_ANN, color=C2, ha="left", va="center")
    ax.text2D(0.02, 0.087, "reach $|t|$", transform=ax.transAxes,
              fontsize=FS_ANN, color=C1, ha="left", va="center")
    ax.set_title("(b)  four bookkeeping regions in $(x,y,t)$:\n"
                 "cone inside reach, reach inside elsewhere")


def style_3d(ax, lim, zlab=None):
    ax.set_xlim(-lim, lim); ax.set_ylim(-lim, lim); ax.set_zlim(-lim, lim)
    ax.set_box_aspect((1, 1, 1))
    ax.view_init(elev=20, azim=34)
    ax.set_xticks([]); ax.set_yticks([]); ax.set_zticks([])
    for a in (ax.xaxis, ax.yaxis, ax.zaxis):
        a.pane.set_alpha(0.0)
        a.line.set_color((1, 1, 1, 0))
    ax.grid(False)
    if zlab:
        ax.set_zlabel(zlab, labelpad=-8)


def panel_radial(ax, rows):
    """(c) the two boundaries as functions of t, with the sites marked."""
    t = np.array([r["t"] for r in rows], float)
    ax.plot(t, C_CONE * t, color=C2, lw=1.8, label="effective cone  $Ct$")
    ax.plot(t, 1.0 * t, color=C1, lw=1.5, ls="--",
            label="reach, inradius  $t$")
    ax.plot(t, np.sqrt(2) * t, color=C1, lw=1.0, ls=":",
            label="reach, circumradius  $\\sqrt{2}\\,t$")
    ax.fill_between(t, C_CONE * t, np.sqrt(2) * t, color=C1, alpha=0.07)
    # staggered in x: at 7 pt these three lines are closer in y than the
    # font is tall, so horizontal separation is what keeps them apart
    for d, lab, xs in ((1.0, "face", 9.85), (np.sqrt(2), "edge", 8.05),
                       (np.sqrt(3), "corner", 6.30)):
        ax.axhline(d, color=CG, lw=0.6, ls="-", zorder=0)
        ax.text(xs, d + 0.09, lab, fontsize=FS_ANN, color=CG, va="bottom",
                ha="right")
    ax.annotate("cone reaches the corner\nat $t=3$ exactly", xy=(3, np.sqrt(3)),
                xytext=(4.15, 0.62), fontsize=FS_ANN, color=CK,
                ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_xlim(0, 10); ax.set_ylim(0, 6.2)
    ax.set_xlabel("ticks  $t$")
    ax.set_ylabel("distance from the event")
    # opaque frame: the circumradius line runs straight through this text
    ax.legend(loc="upper left", handlelength=1.8, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.set_title("(c)  the precursor shell is the gap\n"
                 "between strict reach and the effective scale")


def panel_census(ax, rows):
    """(d) how many sites sit in each region."""
    t = np.array([r["t"] for r in rows])
    cone = np.array([r["cone"] for r in rows], float)
    reach = np.array([r["reach"] for r in rows], float)
    ax.semilogy(t, reach, "o-", color=C1, ms=4, label="reachable")
    ax.semilogy(t, np.maximum(cone, 0.5), "s-", color=C2, ms=4,
                label="inside the effective cone")
    ax.set_xlim(0.4, 10.6); ax.set_ylim(0.5, 6e4)
    ax.set_xticks([2, 4, 6, 8, 10])
    ax.set_xlabel("ticks  $t$")
    ax.set_ylabel("lattice sites")
    ax.legend(frameon=False, loc="lower right", handlelength=1.8,
              borderpad=0.2, labelspacing=0.28)
    ax.annotate("$t=1$: 18 touched,\nonly the origin in the cone",
                xy=(1, 18), xytext=(1.5, 1500),
                fontsize=FS_ANN, color=CK, ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_title("(d)  the cone acquires its first\n"
                 "other site only at $t=2$")


def panel_front(ax, vmax, vgrid):
    """(e) sampled group-velocity distribution versus the strict front."""
    v = vgrid.ravel()
    v = v[np.isfinite(v) & (v > 0)]
    ax.hist(v / C_CONE, bins=90, color=C1, alpha=0.55, edgecolor="none")
    ax.axvline(1.0, color=C2, lw=1.6)
    ax.set_yscale("log")
    ax.set_xlim(0, 1.06)
    ax.set_xlabel("group speed  $|\\nabla_k\\Omega| \\,/\\, C$")
    ax.set_ylabel("Brillouin-zone samples")
    ax.text(0.98, 3e4, f"grid max $= {vmax/C_CONE:.6f}\\,C$", fontsize=FS_ANN,
            color=C2, ha="right", va="center")
    ax.text(0.03, 3e4, "no sampled mode\nexceeds $C$", fontsize=FS_ANN,
            color=CK, ha="left", va="center")
    ax.set_title("(e)  the grid scan finds none above $C$:\n"
                 "$C$ is the effective cone, not the front")


def panel_greens(ax, profiles, decades):
    """(f) the Green's function across all four regions, at one t."""
    t, (rc, am) = profiles
    ax.axvspan(0, C_CONE * t, color=C2, alpha=0.10, lw=0)
    ax.axvspan(C_CONE * t, np.sqrt(2) * t, color=C1, alpha=0.07, lw=0)
    nz = am > 0                             # zero bins are omitted, not floored
    ax.semilogy(rc[nz], am[nz], "o-", color=CK, ms=3.2, lw=1.2)
    ax.axvline(C_CONE * t, color=C2, lw=1.4)
    ax.axvline(np.sqrt(2) * t, color=C1, lw=1.4, ls="--")
    ax.set_xlim(0, np.sqrt(2) * t + 3.2)
    ax.set_ylim(1e-15, 6.0)
    decade_ticks(ax.yaxis, [1e-12, 1e-8, 1e-4, 1],
                 ["$10^{-12}$", "$10^{-8}$", "$10^{-4}$", "$1$"])
    ax.set_xlabel(f"distance from the event  (at $t={t}$)")
    ax.set_ylabel("$\\max\\,|\\phi|$  in the shell")
    ax.text(C_CONE * t * 0.5, 2.0, "main packet", fontsize=FS_ANN, color=C2,
            ha="center", va="center")
    ax.text((C_CONE * t + np.sqrt(2) * t) * 0.5, 2.0, "precursor",
            fontsize=FS_ANN, color=C1, ha="center", va="center")
    ax.text(np.sqrt(2) * t + 1.6, 2.0, "exactly\nzero", fontsize=FS_ANN,
            color=CK, ha="center", va="center")
    ax.set_title(f"(f)  one Green-function profile: packet, then\n"
                 f"{decades:.0f} decades of precursor, then nothing")


def main():
    print("3-D causal structure of a discrete spacetime")
    print(f"  C = 1/sqrt(3) = {C_CONE:.9f}")

    print("\n  [1] causal polytopes")
    for nm, pts in POLY.items():
        r = inradius(pts)
        cr = np.linalg.norm(pts, axis=1).max()
        print(f"      {nm:22s} inradius {r:.6f}   circumradius {cr:.6f}")
    print(f"      effective cone at t=1              radius {C_CONE:.6f}")

    rows = census()
    print("\n  [2] census by tick   (cone / precursor / reachable)")
    for r in rows[:6]:
        print(f"      t={r['t']:2d}   {r['cone']:6d} {r['prec']:7d}"
              f" {r['reach']:8d}")
    print(f"      ...")
    r = rows[-1]
    print(f"      t={r['t']:2d}   {r['cone']:6d} {r['prec']:7d} {r['reach']:8d}")

    vmax, kmax, vgrid = max_group_velocity()
    print(f"\n  [3] max group velocity = {vmax:.9f} = {vmax/C_CONE:.9f} C")
    print(f"      attained at k/pi = {kmax/np.pi}")

    TG = 10
    rc, am = greens_profile(TG)
    print(f"\n  [4] point-source Green's function at t={TG}"
          f"   (cone {C_CONE*TG:.3f}, support {np.sqrt(2)*TG:.3f})")
    for r, a in zip(rc, am):
        zone = "packet   " if r < C_CONE * TG else "precursor"
        if r < 2 or r % 2 < 1:
            print(f"      r={r:5.1f}  {zone}  max|phi| = {a:.3e}")
    inside = am[rc < C_CONE * TG].min()
    edge = am[am > 0][-1]                   # last bin that is not exactly 0
    decades = float(np.log10(inside / edge))
    print(f"      fall across the precursor shell: {inside:.2e} -> {edge:.2e}"
          f"   ({decades:.1f} decades)")
    print(f"      beyond the reach polytope: identically zero (asserted)")

    fig = plt.figure(figsize=(fs.TEXTWIDTH_IN, 8.1000), constrained_layout=True)
    gs = fig.add_gridspec(3, 2, height_ratios=[1.34, 0.86, 0.86])
    axa = fig.add_subplot(gs[0, 0], projection="3d")
    axb = fig.add_subplot(gs[0, 1], projection="3d")
    panel_cell(axa)
    panel_minkowski(axb)
    panel_radial(fig.add_subplot(gs[1, 0]), rows)
    panel_census(fig.add_subplot(gs[1, 1]), rows)
    panel_front(fig.add_subplot(gs[2, 0]), vmax, vgrid)
    panel_greens(fig.add_subplot(gs[2, 1]), (TG, (rc, am)), decades)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12, hspace=0.06,
                                wspace=0.06)
    fs.save(fig, "causal3d")
    print(f"\n  wrote {fs.FIGDIR/'causal3d.pdf'}")


if __name__ == "__main__":
    main()
