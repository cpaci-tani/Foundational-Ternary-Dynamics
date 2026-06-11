#!/usr/bin/env python3
"""
genesis_na_law_forward.py -- FTD-0110 nonlinear bridge: full-pipeline forward model.

Predicts the cluster-size law N(A) (genesis-firing count vs injection amplitude A)
from substrate parameters by reproducing the ENGINE's full tick pipeline -- including
the two flux-injection channels the prior model (genesis_throttle_model.py) omitted:

  (1) the state-flux COUPLING source  delta_j += G_C * grad(s)   [phase_read]
  (2) the GAUSS PROJECTION             flux[void] -= grad(phi),
      with  Lap18 phi = div(J) - charge_coupling*(s - mean_s)     [gauss_project]

Engine tick order (render_bridge.cpp:558-573), faithfully mirrored here:
  phase_read   : delta_j = c^2 * Lap18(flux) [+ G_C*grad(s) if coupling]
  phase_write  : wave_vel += delta_j ; flux += wave_vel ; damping/langevin ;
                 genesis (prob p = 1 - exp(-(|J|-K_GEN)/K_MAN)), kinetic+flux drain
  gauss_project: flux[void] -= grad(phi)   (phi mean-removed)

The 18-point O_h-isotropic Laplacian's Fourier symbol is
  M(k) = (2/3)(cx+cy+cz) + (2/3)(cx*cy + cy*cz + cz*cx) - 4,   ci = cos(k_i),
so the Gauss solve is FFT-exact: phi_hat = source_hat / M(k), zero mode removed.
This IS the engine's SOR operator at convergence; --gauss-mode sor cross-checks the
finite-iteration truncation the engine actually runs.

INPUT TAXONOMY (decides derivation vs boundary):
  framework-derived : K_GENESIS = N_c*K_MANIFEST, K_MANIFEST, N_c, c^2 = 1/3,
                      the 18-pt Laplacian, charge_coupling = 1
  engine-tuning     : K_GENESIS_KINETIC_DRAIN = 0.5, DAMPING, Langevin gamma/T,
                      and the coupling term's G_C = sqrt(alpha)  (alpha-dependency)
A clean [DERIVED] law must depend only on the first list (alpha flagged explicitly
via the --coupling on/off arm).

[EPISTEMIC TAG: forward MODEL / quick-check tier. Match != derivation. The engine
 (campaign_genesis_geometry.cpp) is the canonical measurement; this script predicts
 the firing SET to be compared against it. Frozen under PREREG_FTD0110_NA_LAW_v1.]

Usage:
  python genesis_na_law_forward.py --A 14 --L 32 --seeds 8 --gauss on --coupling on
  python genesis_na_law_forward.py --sweep --L 32 --seeds 8 --out results/model_na.csv

LEDGER: FTD-0110 (nonlinear bridge attack)
Date:   2026-06-11
"""

import argparse
import math
import os
import sys

import numpy as np

# ----------------------------------------------------------------------------
# Constants from the ontic chain (engine/include/ftd/ontic/)
# ----------------------------------------------------------------------------
K_B = 0.511                       # electron mass (sim units) = K_MANIFEST
K_MANIFEST = K_B                  # genesis/evaporation Boltzmann scale  [framework]
N_C = 3                           # color charge number                  [framework]
K_GENESIS = N_C * K_MANIFEST      # = 1.533 genesis threshold            [framework]
C2 = 1.0 / 3.0                    # CFL wave speed squared (c = 1/sqrt 3) [framework]
ALPHA = 1.0 / 137.036            # fine-structure (master-quadratic root, [SMC])
G_C = math.sqrt(ALPHA)            # state-flux coupling = sqrt(alpha)    [alpha-dep]
DAMPING = 0.001                   # base damping                         [engine-tune]
K_GENESIS_FLUX_EPSILON = 1e-9
DEFAULT_DRAIN = 0.5               # K_GENESIS_KINETIC_DRAIN              [engine-tune]

# SC face (6) and FCC edge (12) neighbour offsets -- the 18-point stencil.
SC_OFFSETS = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
FCC_OFFSETS = [
    (1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
    (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
    (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1),
]


def shell_class(dx, dy, dz):
    """Classify a voxel offset from the injection centre into an O_h shell."""
    r2 = dx * dx + dy * dy + dz * dz
    if r2 == 0:
        return "center"
    if r2 == 1:
        return "SC"        # face,   r = 1
    if r2 == 2:
        return "FCC"       # edge,   r = sqrt 2
    if r2 == 3:
        return "BCC"       # corner, r = sqrt 3
    if r2 == 4:
        return "SC2"       # 2nd face shell, r = 2
    return "outer"


class GenesisField:
    """Full-pipeline genesis burst on an L^3 periodic lattice.

    Mirrors the engine tick. Genesis is probabilistic with a deterministic
    per-(seed, voxel, tick) RNG so the ensemble matches the engine's seed sweep.
    """

    def __init__(self, L, amplitude, seed,
                 use_gauss=True, gauss_mode="fft", sor_iters=150,
                 use_coupling=True, drain=DEFAULT_DRAIN,
                 charge_coupling=1.0):
        self.L = L
        self.N = L * L * L
        self.A = amplitude
        self.seed = np.uint64(seed)
        self.use_gauss = use_gauss
        self.gauss_mode = gauss_mode
        self.sor_iters = sor_iters
        self.use_coupling = use_coupling
        self.drain = drain
        self.charge_coupling = charge_coupling

        self.flux = np.zeros((self.N, 3))
        self.wave_vel = np.zeros((self.N, 3))
        self.state = np.zeros(self.N, dtype=np.int8)

        self._build_neighbours()
        if use_gauss and gauss_mode == "fft":
            self._build_green_symbol()

        # delta injection at centre (x-axial), |J| = A * K_GENESIS
        cx = L // 2
        self.center = self._idx(cx, cx, cx)
        self.cxyz = (cx, cx, cx)
        self.flux[self.center, 0] = amplitude * K_GENESIS

        self.genesis_per_tick = []
        self.total_genesis = 0
        self.firing_set = []   # (tick, idx, dx, dy, dz, shell, |J|)
        self.tick_count = 0

    # -- index helpers --------------------------------------------------------
    def _idx(self, x, y, z):
        L = self.L
        return ((x % L) * L + (y % L)) * L + (z % L)

    def _build_neighbours(self):
        L = self.L
        self.sc_nbr = np.zeros((self.N, 6), dtype=np.int32)
        self.fcc_nbr = np.zeros((self.N, 12), dtype=np.int32)
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    i = self._idx(x, y, z)
                    for k, (dx, dy, dz) in enumerate(SC_OFFSETS):
                        self.sc_nbr[i, k] = self._idx(x + dx, y + dy, z + dz)
                    for k, (dx, dy, dz) in enumerate(FCC_OFFSETS):
                        self.fcc_nbr[i, k] = self._idx(x + dx, y + dy, z + dz)

    def _build_green_symbol(self):
        """Fourier symbol M(k) of the 18-pt Laplacian; reciprocal with zero mode = 0."""
        L = self.L
        k = 2.0 * np.pi * np.fft.fftfreq(L) * L / L  # = 2*pi*n/L
        cx = np.cos(k)
        CX = cx[:, None, None]
        CY = cx[None, :, None]
        CZ = cx[None, None, :]
        M = (2.0 / 3.0) * (CX + CY + CZ) + (2.0 / 3.0) * (CX * CY + CY * CZ + CZ * CX) - 4.0
        inv = np.zeros_like(M)
        nz = np.abs(M) > 1e-12
        inv[nz] = 1.0 / M[nz]
        inv[0, 0, 0] = 0.0  # zero mode removed (mean-free phi)
        self._green_inv = inv

    # -- field ops ------------------------------------------------------------
    def _lap18(self, f):
        sc = np.sum(f[self.sc_nbr], axis=1)
        fcc = np.sum(f[self.fcc_nbr], axis=1)
        return sc / 3.0 + fcc / 6.0 - 4.0 * f

    def _grad_state(self):
        """G_C * grad(s) central-difference, per the engine coupling source."""
        s = self.state.astype(np.float64)
        g = np.zeros((self.N, 3))
        # +x face is sc_nbr[:,0], -x is [:,1]; +y [:,2]/-y[:,3]; +z[:,4]/-z[:,5]
        g[:, 0] = (s[self.sc_nbr[:, 0]] - s[self.sc_nbr[:, 1]]) * 0.5
        g[:, 1] = (s[self.sc_nbr[:, 2]] - s[self.sc_nbr[:, 3]]) * 0.5
        g[:, 2] = (s[self.sc_nbr[:, 4]] - s[self.sc_nbr[:, 5]]) * 0.5
        return G_C * g

    def _divergence(self):
        jx, jy, jz = self.flux[:, 0], self.flux[:, 1], self.flux[:, 2]
        div = (jx[self.sc_nbr[:, 0]] - jx[self.sc_nbr[:, 1]]) * 0.5
        div += (jy[self.sc_nbr[:, 2]] - jy[self.sc_nbr[:, 3]]) * 0.5
        div += (jz[self.sc_nbr[:, 4]] - jz[self.sc_nbr[:, 5]]) * 0.5
        return div

    def _grad_phi(self, phi):
        g = np.zeros((self.N, 3))
        g[:, 0] = (phi[self.sc_nbr[:, 0]] - phi[self.sc_nbr[:, 1]]) * 0.5
        g[:, 1] = (phi[self.sc_nbr[:, 2]] - phi[self.sc_nbr[:, 3]]) * 0.5
        g[:, 2] = (phi[self.sc_nbr[:, 4]] - phi[self.sc_nbr[:, 5]]) * 0.5
        return g

    def _solve_phi(self, source):
        if self.gauss_mode == "fft":
            src = source.reshape(self.L, self.L, self.L)
            phi = np.fft.ifftn(np.fft.fftn(src) * self._green_inv).real
            phi = phi.reshape(self.N)
        else:  # finite-iteration SOR, engine-faithful truncation
            phi = np.zeros(self.N)
            omega = 1.5
            for _ in range(self.sor_iters):
                sc = np.sum(phi[self.sc_nbr], axis=1)
                fcc = np.sum(phi[self.fcc_nbr], axis=1)
                gs = (sc / 3.0 + fcc / 6.0 - source) * 0.25
                phi = phi + omega * (gs - phi)
        return phi - phi.mean()

    def _rng_uniform(self, idx):
        """Deterministic per-(seed, voxel, tick) uniform in [0,1) -- SplitMix64.

        Pure Python-int arithmetic with explicit mod-2^64 masking (numpy uint64
        scalar ops silently promote to float on overflow and lose bits).
        """
        M = 0xFFFFFFFFFFFFFFFF
        z = (int(self.seed)
             ^ ((idx * 0x9E3779B97F4A7C15) & M)
             ^ ((int(self.tick_count) * 0xD1B54A32D192ED03) & M)) & M
        z = (z + 0x9E3779B97F4A7C15) & M
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & M
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & M
        z = z ^ (z >> 31)
        return (z >> 11) / float(1 << 53)

    # -- one engine tick ------------------------------------------------------
    def tick(self):
        self.tick_count += 1

        # phase_read: wave + coupling source
        delta_j = np.zeros((self.N, 3))
        for c in range(3):
            delta_j[:, c] = C2 * self._lap18(self.flux[:, c])
        if self.use_coupling:
            delta_j += self._grad_state()   # G_C * grad(s)

        # phase_write: leapfrog + damping + genesis
        self.wave_vel += delta_j
        self.flux += self.wave_vel
        self.flux *= (1.0 - DAMPING)
        self.wave_vel *= (1.0 - DAMPING)

        jmag2 = np.sum(self.flux ** 2, axis=1)
        cand = np.where((self.state == 0) & (jmag2 > K_GENESIS ** 2))[0]
        genesis_this_tick = 0
        cx, cy, cz = self.cxyz
        for i in cand:
            jmag = math.sqrt(jmag2[i])
            p = 1.0 - math.exp(-(jmag - K_GENESIS) / K_MANIFEST)
            if self._rng_uniform(int(i)) < p:
                genesis_this_tick += 1
                self.state[i] = 1 if self.flux[i, 0] >= 0 else -1
                self.wave_vel[i] *= (1.0 - self.drain)
                if jmag > K_GENESIS_FLUX_EPSILON:
                    self.flux[i] *= max(0.0, 1.0 - K_GENESIS / jmag)
                # record firing geometry
                x, y, z = (int(i) // (self.L * self.L),
                           (int(i) // self.L) % self.L, int(i) % self.L)
                dx = ((x - cx + self.L // 2) % self.L) - self.L // 2
                dy = ((y - cy + self.L // 2) % self.L) - self.L // 2
                dz = ((z - cz + self.L // 2) % self.L) - self.L // 2
                self.firing_set.append((self.tick_count, int(i), dx, dy, dz,
                                        shell_class(dx, dy, dz), jmag))

        # gauss_project: flux[void] -= grad(phi)
        if self.use_gauss:
            mean_s = self.state.mean()
            source = self._divergence() - self.charge_coupling * (
                self.state.astype(np.float64) - mean_s)
            phi = self._solve_phi(source)
            gphi = self._grad_phi(phi)
            void = self.state == 0
            self.flux[void] -= gphi[void]

        self.genesis_per_tick.append(genesis_this_tick)
        self.total_genesis += genesis_this_tick
        return genesis_this_tick

    def run(self, max_ticks=60, quiet_ticks=3):
        quiet = 0
        for _ in range(max_ticks):
            g = self.tick()
            if g == 0 and self.total_genesis > 0:
                quiet += 1
                if quiet >= quiet_ticks:
                    break
            else:
                quiet = 0
        return self.total_genesis


def run_amplitude(A, L, seeds, ticks=60, firing_rows=None, **kw):
    """Run `seeds` seeds at amplitude A. If firing_rows is a list, append
    per-firing geometry rows (seed, tick, idx, dx, dy, dz, shell, |J|)."""
    counts = []
    for s in range(seeds):
        f = GenesisField(L, A, seed=0xE0102000 + s, **kw)
        n = f.run(max_ticks=ticks)
        counts.append(n)
        if firing_rows is not None:
            for (tick, idx, dx, dy, dz, shell, jmag) in f.firing_set:
                firing_rows.append((0xE0102000 + s, tick, idx, dx, dy, dz, shell, jmag))
    return float(np.mean(counts)), float(np.std(counts)), counts


def main():
    ap = argparse.ArgumentParser(description="FTD-0110 N(A) forward model")
    ap.add_argument("--A", type=float, default=None, help="single amplitude")
    ap.add_argument("--L", type=int, default=32)
    ap.add_argument("--seeds", type=int, default=8)
    ap.add_argument("--ticks", type=int, default=60)
    ap.add_argument("--gauss", choices=["on", "off"], default="on")
    ap.add_argument("--gauss-mode", choices=["fft", "sor"], default="fft")
    ap.add_argument("--sor-iters", type=int, default=150)
    ap.add_argument("--coupling", choices=["on", "off"], default="on")
    ap.add_argument("--drain", type=float, default=DEFAULT_DRAIN)
    ap.add_argument("--sweep", action="store_true",
                    help="run the FTD-0261 11-point amplitude grid")
    ap.add_argument("--firing", action="store_true",
                    help="dump per-firing geometry for the single --A")
    ap.add_argument("--firing-dir", default=None,
                    help="dir to write firing_A<A>.csv (sweep mode, for Jaccard/shell)")
    ap.add_argument("--out", default=None, help="CSV output path for --sweep")
    args = ap.parse_args()

    kw = dict(use_gauss=(args.gauss == "on"), gauss_mode=args.gauss_mode,
              sor_iters=args.sor_iters, use_coupling=(args.coupling == "on"),
              drain=args.drain)

    print(f"# FTD-0110 N(A) forward model | L={args.L} seeds={args.seeds} "
          f"gauss={args.gauss}/{args.gauss_mode} coupling={args.coupling} drain={args.drain}")
    print(f"# K_GENESIS={K_GENESIS:.4f} K_MANIFEST={K_MANIFEST} G_C=sqrt(alpha)={G_C:.6f} c^2={C2:.6f}")

    if args.sweep:
        import csv as _csv
        grid = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
        rows = []
        if args.firing_dir:
            os.makedirs(args.firing_dir, exist_ok=True)
        print(f"{'A':>5s} {'N_mean':>9s} {'N_std':>8s} {'k=N/A^2':>9s}")
        for A in grid:
            fr = [] if args.firing_dir else None
            mean, std, counts = run_amplitude(A, args.L, args.seeds,
                                              ticks=args.ticks, firing_rows=fr, **kw)
            print(f"{A:5d} {mean:9.2f} {std:8.2f} {mean / A**2:9.4f}")
            rows.append((A, mean, std, mean / A**2))
            if fr is not None:
                fp = os.path.join(args.firing_dir, f"firing_A{A}.csv")
                with open(fp, "w", newline="") as fh:
                    w = _csv.writer(fh)
                    w.writerow(["seed", "tick", "idx", "dx", "dy", "dz", "shell", "jmag"])
                    w.writerows(fr)
        if args.out:
            with open(args.out, "w", newline="") as fh:
                w = _csv.writer(fh)
                w.writerow(["A", "N_mean", "N_std", "k"])
                w.writerows(rows)
            print(f"# wrote {args.out}")
    elif args.A is not None:
        f = GenesisField(args.L, args.A, seed=0xE0102000, **kw)
        n = f.run(max_ticks=args.ticks)
        print(f"A={args.A}: N_gen={n}  per_tick={f.genesis_per_tick}")
        if args.firing:
            print("# tick idx dx dy dz shell |J|")
            for fs in f.firing_set:
                print(f"  {fs[0]:3d} {fs[1]:7d} {fs[2]:3d} {fs[3]:3d} {fs[4]:3d} {fs[5]:>6s} {fs[6]:.4f}")
    else:
        ap.error("pass --A <amp> or --sweep")


if __name__ == "__main__":
    main()
