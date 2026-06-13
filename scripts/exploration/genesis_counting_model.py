#!/usr/bin/env python3
"""
FTD-0277 (Arc 3) — the analytic genesis-counting model. DEVELOPMENT VERSION.

Status: [DEVELOPMENT — no verdicts]. The comparison against the frozen falsifiers
(SCOPE_GENESIS_COUNTING_MODEL.md SS5) happens only after the pre-registration lock.

Target: derive N(A; drain, gamma) by an analytic recursion in FIRING RANK (no time
stepping), from:
  THEOREM ingredients : exact lattice Poisson Green's function G_L (OT-1.4)
  AXIOM ingredients   : genesis threshold |J| > K_GENESIS; flux cost K per firing
  MEASURED ingredients: one-shot burst (FTD-0267); slosh (FTD-0273);
                        first-order background (FTD-0272)
  IMPOSED register    : drain=0.5, gamma=0.02, G_C=sqrt(alpha), charge_coupling=1

Mechanism class (selected by the FTD-0276 Leg-A drain law, NOT tuned to it):
  SLOSH-PASS DISSIPATION COUNTING. The undamped field in the cluster region
  re-excites near-threshold void sites every slosh cycle; the kinetic drain is the
  dominant dissipation, removing a fraction `drain` of the local kinetic energy at
  each firing. The cascade therefore terminates after a number of firings set by
  the local energy budget divided by the per-firing kinetic-drain cost:

      N ~ E_loc / (drain * e_fire),   e_fire ~ kinetic energy at a firing site

  -> N proportional to 1/drain (Leg A measured k_eff ~ drain^-0.93), and
  -> N proportional to A^2 through E_loc ~ capture * (1/2)(A K)^2.

  The discriminating ALTERNATIVE (single-pass budget counting, each firing costing
  a fixed ~K^2/2 once) predicts k_eff independent of drain — falsified by Leg A.
  This selection-by-measurement is the model's first structural result.

Structural choices (explicit, kept few — each must survive F-3/F-4 falsifiers):
  S1. Local capture: the fraction of E0 retained in the cluster region is the
      Green's-function near-field weight within the threshold radius (computable,
      not fitted) times the wave-escape factor exp(-r/lambda_gamma) with
      lambda_gamma = c/gamma the friction length (gamma from the register).
  S2. Per-firing kinetic cost: e_fire = (1/2) K^2 * kappa with kappa = KE/PE = 1
      (equipartition of the travelling pulse; quadrature symmetry FTD-0257).
  S3. Sub-knee geometry: a site can fire only if the STATIC profile
      (center-retained flux spreading ~ 1/r + Gauss boost from the fired set)
      exceeds K — this gates WHICH shells participate (F-3 geometry falsifier);
      the pass counting sets HOW MANY total firings occur within the gated region.

v0 DEVELOPMENT FINDINGS (2026-06-12, first run — recorded for session 2):
  + The drain law emerges at exponent -1.000 (FTD-0276 measured -0.93): the
    slosh-pass mechanism's signature prediction holds structurally.
  + N ~ A^2 appears globally (model/engine ratio ~constant over two decades).
  - DEFECT 1 (gating): the threshold recursion lacks flux CONSUMPTION — the
    incoherent Gauss-boost sum from fired sites snowballs and gates ~389 sites
    at every A (engine: ~5 firings at A=10, FTD-0267). Fix: subtract the K flux
    cost from the local field per firing + carry the beta-v2 center back-reaction
    sign structure (FTD-0263) so the gate self-limits.
  - DEFECT 2 (capture): near_field≈0.80 is ~25x too generous; the capture
    functional must model the DISPERSAL RACE (the pulse escapes at speed c
    before the Gauss boost binds it — the FTD-0273 halo dominance). Expected
    effective capture ~3% at drain 0.5 (back-of-envelope from k_eff=0.059).
  Net magnitude is ~25-30x high; shape and drain scaling are structurally right.

Usage:
  python scripts/exploration/genesis_counting_model.py --sweep
  python scripts/exploration/genesis_counting_model.py --drain-scan --A 12
"""

import argparse
import math
import sys

import numpy as np

# ---- framework constants (mirror engine ontic chain) ------------------------
K_B = 0.511
K_MANIFEST = K_B
N_C = 3
K_GENESIS = N_C * K_MANIFEST          # 1.533
C2 = 1.0 / 3.0
C = math.sqrt(C2)                     # 0.57735
ALPHA = 1.0 / 137.036
G_C = math.sqrt(ALPHA)
EPS_QUANTUM = 0.5 * K_GENESIS ** 2    # 1.1750 (FTD-0273 flip quantum)

# ---- imposed register (SCOPE SS2) -------------------------------------------
DRAIN_DEFAULT = 0.5
GAMMA_DEFAULT = 0.02
CHARGE_COUPLING = 1.0


# ---- THEOREM ingredient: exact lattice Green's function ---------------------
def green_table(L=33):
    """Exact G_L(r) for the 18-pt Laplacian on the L^3 torus (OT-1.4 symbol),
    returned as a 3D array centred at the origin, plus |grad G| magnitudes."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    cx = np.cos(k)
    CX = cx[:, None, None]; CY = cx[None, :, None]; CZ = cx[None, None, :]
    M = (2.0 / 3.0) * (CX + CY + CZ) + (2.0 / 3.0) * (CX * CY + CY * CZ + CZ * CX) - 4.0
    inv = np.zeros_like(M)
    nz = np.abs(M) > 1e-12
    inv[nz] = 1.0 / M[nz]
    # delta source at origin: G = IFFT(inv) (sign: lap G = delta)
    G = np.fft.ifftn(inv).real
    # central-difference gradient magnitude
    gx = (np.roll(G, -1, 0) - np.roll(G, 1, 0)) * 0.5
    gy = (np.roll(G, -1, 1) - np.roll(G, 1, 1)) * 0.5
    gz = (np.roll(G, -1, 2) - np.roll(G, 1, 2)) * 0.5
    gmag = np.sqrt(gx * gx + gy * gy + gz * gz)
    return G, gmag


SHELLS = {  # r^2 -> (name, multiplicity) for the 27-block + 2nd shell
    1: ("SC", 6), 2: ("FCC", 12), 3: ("BCC", 8), 4: ("SC2", 6),
    5: ("r5", 24), 6: ("r6", 24), 8: ("r8", 12), 9: ("r9", 30),
}


def shell_sites(L=33, rmax2=20):
    """List of (r2, dx,dy,dz) lattice offsets up to rmax2, excluding origin."""
    out = []
    R = int(math.isqrt(rmax2)) + 1
    for dx in range(-R, R + 1):
        for dy in range(-R, R + 1):
            for dz in range(-R, R + 1):
                r2 = dx * dx + dy * dy + dz * dz
                if 0 < r2 <= rmax2:
                    out.append((r2, dx, dy, dz))
    return out


class CountingModel:
    """Analytic firing-rank recursion. No time stepping."""

    def __init__(self, drain=DRAIN_DEFAULT, gamma=GAMMA_DEFAULT, L=33,
                 kappa=1.0, verbose=False):
        self.drain = drain
        self.gamma = gamma
        self.kappa = kappa            # S2: KE/PE at firing (equipartition -> 1)
        self.L = L
        self.G, self.gmag = green_table(L)
        self.sites = shell_sites(L)
        self.verbose = verbose

    # -- S3: static threshold profile -----------------------------------------
    def field_at(self, r2, dx, dy, dz, A, fired):
        """Combined static field magnitude at offset (dx,dy,dz):
        wave-spread residual (~1/r, friction-damped) + Gauss boost from the
        fired set (exact |grad G_L| per source, incoherent polarity sum)."""
        r = math.sqrt(r2)
        # center-retained flux (A-1)K spreading spherically: amplitude ~ 1/r,
        # friction length lambda = C/gamma (time-of-flight damping)
        lam = C / self.gamma if self.gamma > 0 else float("inf")
        wave = (A - 1.0) * K_GENESIS * (1.0 / max(r, 1.0)) * math.exp(-r / lam)
        # Gauss boost: per fired source at (fx,fy,fz), |grad G| at separation;
        # polarities alternate (div-sign) -> incoherent sum (sqrt-N scaling)
        boost2 = 0.0
        for (fx, fy, fz) in fired:
            sx, sy, sz = dx - fx, dy - fy, dz - fz
            boost2 += self.gmag[sx % self.L, sy % self.L, sz % self.L] ** 2
        boost = CHARGE_COUPLING * math.sqrt(boost2)
        return wave + boost

    def gated_region(self, A, max_iter=40):
        """Self-consistent threshold set: which offsets see field > K_GENESIS.
        Rank recursion: start with the center fired; add the strongest
        above-threshold site; recompute; stop when none qualify."""
        fired = [(0, 0, 0)]
        candidates = list(self.sites)
        for _ in range(max_iter * 32):
            best = None
            for (r2, dx, dy, dz) in candidates:
                if (dx, dy, dz) in fired:
                    continue
                f = self.field_at(r2, dx, dy, dz, A, fired)
                if f > K_GENESIS and (best is None or f > best[0]):
                    best = (f, r2, dx, dy, dz)
            if best is None:
                break
            fired.append((best[2], best[3], best[4]))
        return fired

    # -- slosh-pass dissipation counting --------------------------------------
    def count(self, A):
        """N(A): gated region size sets the geometry; the slosh-pass budget sets
        the total firing count within it."""
        gated = self.gated_region(A)
        n_gated = len(gated)

        # S1: local capture = Green's near-field weight inside the gated radius
        if n_gated > 1:
            rmax = max(math.sqrt(dx * dx + dy * dy + dz * dz)
                       for (dx, dy, dz) in gated[1:])
        else:
            rmax = 1.0
        # fraction of the source's Green-function energy within rmax (computable)
        g2 = self.gmag ** 2
        # radial mask around origin
        idx = np.indices(g2.shape)
        half = self.L // 2
        d = ((idx + half) % self.L) - half
        r2grid = (d[0] ** 2 + d[1] ** 2 + d[2] ** 2).astype(float)
        inside = g2[r2grid <= rmax * rmax + 1e-9].sum()
        total = g2.sum()
        near_field = inside / total if total > 0 else 0.0
        lam = C / self.gamma if self.gamma > 0 else float("inf")
        capture = near_field * math.exp(-rmax / lam)

        # energy budget and per-firing kinetic-drain cost
        E0 = 0.5 * (A * K_GENESIS) ** 2
        e_fire = self.drain * (0.5 * K_GENESIS ** 2) * self.kappa
        n_budget = capture * E0 / e_fire if e_fire > 0 else float("inf")

        # the count is budget-limited but cannot exceed repeated re-excitation of
        # the gated region; one-shot burst (FTD-0267) caps at the budget count
        N = min(n_budget, n_gated * max(1.0, n_budget / max(n_gated, 1)))
        if self.verbose:
            print(f"  A={A:5.1f} gated={n_gated:4d} rmax={rmax:.2f} "
                  f"near_field={near_field:.3f} capture={capture:.3f} "
                  f"E0={E0:8.1f} e_fire={e_fire:.3f} n_budget={n_budget:8.1f}")
        return N, n_gated


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sweep", action="store_true")
    ap.add_argument("--drain-scan", action="store_true")
    ap.add_argument("--A", type=float, default=12.0)
    ap.add_argument("--drain", type=float, default=DRAIN_DEFAULT)
    ap.add_argument("--gamma", type=float, default=GAMMA_DEFAULT)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    if args.sweep:
        m = CountingModel(drain=args.drain, gamma=args.gamma, verbose=args.verbose)
        grid = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
        ref = {10: 4.0, 12: 8.4, 14: 16.4, 16: 21.6, 20: 27.4, 25: 32.6,
               30: 45.0, 40: 91.8, 50: 130.2, 70: 260.2, 90: 383.3}
        print(f"# counting model | drain={args.drain} gamma={args.gamma}")
        print(f"{'A':>5s} {'N_model':>9s} {'gated':>6s} {'FTD0261':>8s} {'ratio':>7s}")
        for A in grid:
            N, ng = m.count(A)
            r = N / ref[A] if ref.get(A) else float("nan")
            print(f"{A:5.0f} {N:9.2f} {ng:6d} {ref.get(A, float('nan')):8.1f} {r:7.2f}")

    if args.drain_scan:
        print(f"# drain scan at A={args.A} gamma={args.gamma}")
        print(f"{'drain':>6s} {'N_model':>9s} {'k_eff':>8s}")
        ks = []
        for d in [0.125, 0.25, 0.375, 0.5, 0.625, 0.75]:
            m = CountingModel(drain=d, gamma=args.gamma)
            N, _ = m.count(args.A)
            k = N / args.A ** 2
            ks.append((d, k))
            print(f"{d:6.3f} {N:9.2f} {k:8.4f}")
        # implied exponent
        xs = [math.log(d) for d, k in ks if k > 0]
        ys = [math.log(k) for d, k in ks if k > 0]
        n = len(xs)
        if n >= 2:
            sx, sy = sum(xs), sum(ys)
            sxx = sum(x * x for x in xs); sxy = sum(x * y for x, y in zip(xs, ys))
            p = (n * sxy - sx * sy) / (n * sxx - sx * sx)
            print(f"# implied k_eff ~ drain^{p:+.3f}  (FTD-0276 measured -0.93)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
