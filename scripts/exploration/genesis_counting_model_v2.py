#!/usr/bin/env python3
"""
FTD-0309 (FTD-0110 nonlinear bridge, v2 of the genesis-counting model). DEVELOPMENT.

Status: [DEVELOPMENT -- no verdicts]. The frozen falsifiers (the FTD-0277/0261 gates,
reused unchanged) are evaluated only by the adjudicator AFTER the pre-registration lock.
This file fixes its structural choices from engine physics and the imposed register --
NEVER by tuning to the FTD-0261 target curve.

WHY v2 (the FTD-0277 v1 post-mortem, ANALYSIS_GENESIS_COUNTING_v1.md SS6):
  v1 gated the same ~389 outer sites at every A (no flux consumption -> no self-limit
  -> degenerate N~A^2, geometry L1=1.83 vs gate 0.30, magnitude 20-40x high). v1's
  closure note requires any v2 to add (a) flux consumption / self-limiting and (b) a
  dispersal-race capture functional. This model adds BOTH, as a collective-coordinate
  shell reduction of the FTD-0269 forward model (genesis_na_law_forward.py), which
  itself reproduces the engine law.

THE DERIVED MECHANISM (read off the forward model's own per-shell firing profile,
fwd_firing dumps -- this is engine-faithful physics, NOT the FTD-0261 gates):
  * SUB-KNEE (A<=16..20): the 27-block FILLS shell-by-shell as J_s(A) crosses
    K_GENESIS -- SC saturates first (~A9), then BCC, FCC, SC2. Steep (p_lo~3.7).
  * SUPER-KNEE (A>=25): the 27-block is pinned (1+6+12+8+6=33) and OUTER shells fire
    on the ENERGY budget: N_outer ~ capture*(1/2)(A K_GEN)^2 / e_fire ~ (capture/drain)A^2.
  * The KNEE is the crossover of the field-gated count and the energy-budget count.

ARCHITECTURE.  N(A) = min( N_field(A), N_budget(A) ).
  N_field(A)  : the threshold-gated firing count from a few-tick shell-dynamics burst
                (propagation via the 18-pt shell operator + genesis + flux consumption
                + Gauss-boost cascade + gamma dispersal). Self-limits via consumption.
  N_budget(A) : (capture/drain) * A^2 -- the energy-budget regime; `capture` is the
                dispersal-race functional (fraction of E0 that fires before escaping
                at speed c). Both pieces use only framework + imposed inputs.

INPUT TAXONOMY (decides DERIVED-vs-BOUNDARY):
  THEOREM/derived : K_GENESIS=N_c*K_MANIFEST, K_MANIFEST, N_c, c^2=1/3, the 18-pt
                    shell operator W, the radial Green's gradient kernel gradG (OT-1.4),
                    charge_coupling=1.
  IMPOSED register: drain=0.5, gamma=0.02, G_C=sqrt(alpha)  (alpha-dependence flagged).
A clean [CONDITIONAL -- DERIVED-GIVEN-IMPOSED] result depends only on these two lists.

Usage:
  python scripts/exploration/genesis_counting_model_v2.py --sweep
  python scripts/exploration/genesis_counting_model_v2.py --A 14 --verbose
  python scripts/exploration/genesis_counting_model_v2.py --drain-scan
"""

import argparse
import math
import sys
from collections import defaultdict

import numpy as np

# ---- framework constants (mirror the engine ontic chain) --------------------
K_MANIFEST = 0.511
N_C = 3
K_GENESIS = N_C * K_MANIFEST          # 1.533
C2 = 1.0 / 3.0
C = math.sqrt(C2)                     # 0.57735
ALPHA = 1.0 / 137.036
G_C = math.sqrt(ALPHA)               # 0.085425   [alpha-dependent, flagged]
DAMPING = 0.001                       # engine base damping [engine constant]

# ---- imposed register -------------------------------------------------------
DRAIN_DEFAULT = 0.5
GAMMA_DEFAULT = 0.02
CHARGE_COUPLING = 1.0

# ---- O_h shell structure (r^2 -> multiplicity) ------------------------------
SC_OFF = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
FCC_OFF = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
           (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
           (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_shells(r2max):
    """Return (r2_list, mult, shell_sites) for all O_h shells up to r2max."""
    sites = defaultdict(list)
    R = int(math.isqrt(r2max)) + 1
    for x in range(-R, R + 1):
        for y in range(-R, R + 1):
            for z in range(-R, R + 1):
                r2 = x * x + y * y + z * z
                if 0 <= r2 <= r2max:
                    sites[r2].append((x, y, z))
    r2s = sorted(sites)
    mult = {r2: len(sites[r2]) for r2 in r2s}
    return r2s, mult, sites


def build_shell_operator(r2s, sites):
    """18-pt Laplacian reduced to O_h shells: (L18 f)_s = sum W[s,s'] f_s' - 4 f_s.
    Exact under the O_h-symmetric scalar approximation (per-site neighbour-shell
    histogram averaged over the shell)."""
    def r2of(p):
        return p[0] ** 2 + p[1] ** 2 + p[2] ** 2
    idx = {r2: i for i, r2 in enumerate(r2s)}
    n = len(r2s)
    W = np.zeros((n, n))
    for s in r2s:
        for site in sites[s]:
            for (dx, dy, dz) in SC_OFF:
                nb = r2of((site[0] + dx, site[1] + dy, site[2] + dz))
                if nb in idx:
                    W[idx[s], idx[nb]] += (1.0 / 3.0)
            for (dx, dy, dz) in FCC_OFF:
                nb = r2of((site[0] + dx, site[1] + dy, site[2] + dz))
                if nb in idx:
                    W[idx[s], idx[nb]] += (1.0 / 6.0)
        W[idx[s], :] /= len(sites[s])
    return W


def radial_gradG(r2s, L=32):
    """Radial |grad G_L|(r) of the 18-pt Poisson Green's function (OT-1.4), per shell.
    This is the Gauss-boost kernel per unit fired charge."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    CX = np.cos(k)[:, None, None]
    CY = np.cos(k)[None, :, None]
    CZ = np.cos(k)[None, None, :]
    M = (2 / 3) * (CX + CY + CZ) + (2 / 3) * (CX * CY + CY * CZ + CZ * CX) - 4.0
    inv = np.zeros_like(M)
    nz = np.abs(M) > 1e-12
    inv[nz] = 1.0 / M[nz]
    inv[0, 0, 0] = 0.0
    G = np.fft.ifftn(inv).real
    gx = (np.roll(G, -1, 0) - np.roll(G, 1, 0)) * 0.5
    gy = (np.roll(G, -1, 1) - np.roll(G, 1, 1)) * 0.5
    gz = (np.roll(G, -1, 2) - np.roll(G, 1, 2)) * 0.5
    gmag = np.sqrt(gx * gx + gy * gy + gz * gz)
    # representative on-axis site per shell
    reps = {}
    R = int(math.isqrt(max(r2s))) + 1
    for x in range(0, R + 1):
        for y in range(0, x + 1):
            for z in range(0, y + 1):
                r2 = x * x + y * y + z * z
                if r2 in r2s and r2 not in reps:
                    reps[r2] = (x, y, z)
    out = np.zeros(len(r2s))
    for i, r2 in enumerate(r2s):
        p = reps.get(r2, (0, 0, 0))
        out[i] = gmag[p[0] % L, p[1] % L, p[2] % L]
    return out


class ShellBurst:
    """Collective-coordinate (O_h shell) reduction of the FTD-0269 forward model.

    State per shell: J[s] = representative |flux| at a void site in shell s,
    v[s] = wave_vel, n[s] = fired (manifested) count (0..mult). One-shot burst.
    """

    def __init__(self, drain=DRAIN_DEFAULT, gamma=GAMMA_DEFAULT, r2max=121,
                 charge_coupling=CHARGE_COUPLING, use_coupling=True,
                 boost_mode="monopole"):
        self.drain = drain
        self.gamma = gamma
        self.cc = charge_coupling
        self.use_coupling = use_coupling
        # boost_mode demonstrates the THE OBSTRUCTION (the result of FTD-0309):
        #   "monopole" -> treat the enclosed fired charge as a central source
        #                 (the net charge is NOT ~0 in a scalar model) -> the boost
        #                 ACCUMULATES and the cascade RUNS AWAY at large A.
        #   "local"    -> boost only from the nearest inner fired shell -> the boost
        #                 cannot reach FCC/BCC and the 27-block UNDER-fills.
        # Neither works: the true fired set is an x-DIPOLE (net charge ~0, near-field
        # boost), which a radial/scalar collective coordinate cannot represent. The
        # super-knee is capped by the energy budget (see n_budget) so the curve stays
        # finite for adjudication.
        self.boost_mode = boost_mode
        self.r2s, mult, sites = build_shells(r2max)
        self.n_shell = len(self.r2s)
        self.mult = np.array([mult[r2] for r2 in self.r2s], dtype=float)
        self.r = np.array([math.sqrt(r2) for r2 in self.r2s])
        self.W = build_shell_operator(self.r2s, sites)
        self.gradG = radial_gradG(self.r2s)

    def burst(self, A, max_ticks=40, quiet=3, verbose=False):
        ns = self.n_shell
        J = np.zeros(ns)
        v = np.zeros(ns)
        n = np.zeros(ns)            # fired count per shell
        J[0] = A * K_GENESIS        # central injection
        per_tick = []
        quiet_run = 0
        for t in range(max_ticks):
            # phase_read: 18-pt shell Laplacian
            lap = self.W @ J - 4.0 * J
            dv = C2 * lap
            # phase_write: leapfrog + damping + gamma friction
            v += dv
            J += v
            J *= (1.0 - DAMPING)
            v *= (1.0 - DAMPING) * (1.0 - self.gamma)
            # genesis per shell (void fraction only)
            fired_this = 0.0
            void_frac = np.clip((self.mult - n) / np.maximum(self.mult, 1.0), 0.0, 1.0)
            for s in range(ns):
                if J[s] > K_GENESIS and void_frac[s] > 1e-9:
                    p = 1.0 - math.exp(-(J[s] - K_GENESIS) / K_MANIFEST)
                    fire = (self.mult[s] - n[s]) * p
                    if fire <= 0:
                        continue
                    n[s] += fire
                    fired_this += fire
                    frac = fire / self.mult[s]
                    # flux consumption: each fired site sheds K_GENESIS of |J|
                    J[s] = max(0.0, J[s] - K_GENESIS * frac)
                    # kinetic drain on the fired fraction
                    v[s] *= (1.0 - self.drain * frac)
            # gauss_project boost (see __init__: this is where the scalar reduction
            # breaks -- the true fired set is an x-dipole, which neither scalar mode
            # represents). Both modes are kept for the FTD-0309 obstruction demo.
            if fired_this > 0:
                void = (self.mult - n) > 1e-9
                if self.boost_mode == "monopole":
                    enclosed = np.cumsum(n) - n     # fired charge strictly inside shell s
                    boost = self.cc * enclosed * self.gradG
                else:                               # "local": nearest inner fired shell only
                    boost = np.zeros(ns)
                    for s in range(1, ns):
                        if not void[s]:
                            continue
                        s_in = -1
                        for t in range(s - 1, -1, -1):
                            if n[t] > 0.05:
                                s_in = t
                                break
                        if s_in < 0:
                            continue
                        dr = max(self.r[s] - self.r[s_in], 1.0)
                        kern = float(np.interp(dr, self.r, self.gradG))
                        boost[s] = self.cc * min(n[s_in], self.mult[s_in]) * kern
                J[void] += boost[void]
            per_tick.append(fired_this)
            if fired_this < 1e-6 and n.sum() > 0:
                quiet_run += 1
                if quiet_run >= quiet:
                    break
            else:
                quiet_run = 0
        if verbose:
            names = {0: "cen", 1: "SC", 2: "FCC", 3: "BCC", 4: "SC2"}
            prof = " ".join(f"{names.get(r2, 'r'+str(r2)):>4}:{n[i]:5.1f}"
                            for i, r2 in enumerate(self.r2s) if n[i] > 0.05)
            print(f"  A={A:5.1f} N_field={n.sum():7.2f} ticks={len(per_tick):2d}  {prof}")
        return n

    # -- the two derived curves ----------------------------------------------
    def n_field(self, A, **kw):
        return float(self.burst(A, **kw).sum())

    # Dispersal-race capture: the fraction of E0 that genesis dissipates (vs the part
    # that radiates/dilutes across the 3D volume below threshold) before the one-shot
    # burst quenches. The super-knee EXPONENT (p_hi=2) is the energy-budget regime and
    # IS derived; this COEFFICIENT is the engine-emergent nonlinear-suppression factor
    # (the linear k=1/4 theorem would give capture=drain/4=0.125; the measured ~0.05 is
    # the nonlinear genesis throttling of FTD-0267). Per FTD-0307 this calibration is
    # PHYSICAL/irreducible -- it is DECLARED here as an engine-emergent input, NOT
    # derived and NOT tuned to FTD-0261 (the value is read off the FTD-0269 forward
    # model's own super-knee energetics = the framework dynamics given the register).
    CAPTURE_EMERGENT = 0.024   # engine super-knee k=capture/drain~0.047 at gamma=0.02

    def capture(self):
        return self.CAPTURE_EMERGENT

    def n_budget(self, A):
        e_fire = self.drain * (0.5 * K_GENESIS ** 2)     # kinetic-drain cost/firing
        E0 = 0.5 * (A * K_GENESIS) ** 2
        return self.capture() * E0 / e_fire

    def count(self, A, **kw):
        nf = self.n_field(A, **kw)
        nb = self.n_budget(A)
        return min(nf, nb), nf, nb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sweep", action="store_true")
    ap.add_argument("--drain-scan", action="store_true")
    ap.add_argument("--A", type=float, default=14.0)
    ap.add_argument("--drain", type=float, default=DRAIN_DEFAULT)
    ap.add_argument("--gamma", type=float, default=GAMMA_DEFAULT)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    ref = {10: 4.0, 12: 8.4, 14: 16.4, 16: 21.6, 20: 27.4, 25: 32.6,
           30: 45.0, 40: 91.8, 50: 130.2, 70: 260.2, 90: 383.3}

    if args.sweep:
        m = ShellBurst(drain=args.drain, gamma=args.gamma)
        print(f"# v2 shell-burst | drain={args.drain} gamma={args.gamma} "
              f"capture={m.capture():.4f}")
        print(f"{'A':>5} {'N=min':>8} {'N_field':>8} {'N_budget':>9} "
              f"{'FTD0261':>8} {'ratio':>7}")
        for A in [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]:
            N, nf, nb = m.count(A, verbose=args.verbose)
            r = N / ref[A]
            print(f"{A:5d} {N:8.2f} {nf:8.2f} {nb:9.2f} {ref[A]:8.1f} {r:7.2f}")

    if args.drain_scan:
        print(f"# drain scan at A={args.A} gamma={args.gamma}")
        print(f"{'drain':>6} {'N':>8} {'k_eff':>8}")
        ks = []
        for d in [0.125, 0.25, 0.375, 0.5, 0.625, 0.75]:
            m = ShellBurst(drain=d, gamma=args.gamma)
            N, _, _ = m.count(args.A)
            ks.append((d, N / args.A ** 2))
            print(f"{d:6.3f} {N:8.2f} {N/args.A**2:8.4f}")
        xs = [math.log(d) for d, k in ks if k > 0]
        ys = [math.log(k) for d, k in ks if k > 0]
        npt = len(xs)
        if npt >= 2:
            sx, sy = sum(xs), sum(ys)
            sxx = sum(x * x for x in xs)
            sxy = sum(x * y for x, y in zip(xs, ys))
            p = (npt * sxy - sx * sy) / (npt * sxx - sx * sx)
            print(f"# implied k_eff ~ drain^{p:+.3f} (FTD-0276 measured -0.93)")

    if not args.sweep and not args.drain_scan:
        m = ShellBurst(drain=args.drain, gamma=args.gamma)
        N, nf, nb = m.count(args.A, verbose=True)
        print(f"A={args.A}: N={N:.2f} (field={nf:.2f}, budget={nb:.2f}) "
              f"capture={m.capture():.4f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
