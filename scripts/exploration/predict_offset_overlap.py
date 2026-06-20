#!/usr/bin/env python3
"""
predict_offset_overlap.py -- FTD-0281 rung-b diagnostic

Predicts, from the OPERATOR eigenvectors and the actual off-center Gaussian J(0),
the relative excitation power the engine SHOULD show for each level. The engine
autocorrelation C(t) = Σ_probe J(0)·J(t) decomposes in the operator eigenbasis as

    C(t) = Σ_n |c_n|²_probe cos(ω_n t),   c_n = <ψ_n | J(0)>  (projected on probe set)

so the PSD power at ω_n is ∝ (Σ_probe ψ_n J(0))² weighted by the probe overlap.
We compute, for the SAME off-center Gaussian the campaign injects (single x-comp),
the modal coefficient c_n = <ψ_n | g> and the probe-restricted weight, and report
the predicted power ratio P_n/P_1s. This separates two hypotheses:

  H-excite: the off-center packet has ~0 overlap with the 2p eigenvector
            => the engine cannot show a 2p line (excitation-limited, even off-center).
  H-probe : the packet DOES overlap the 2p, but the centered symmetric probe ball
            cancels the (odd-parity) 2p contribution in the autocorrelation
            => a probe-geometry artifact, fixable with an antisymmetric probe.

Uses the engine-dumped phi_C and the same L18 operator as the canonical analyzer.

Usage:
  python predict_offset_overlap.py --phi <phiC.csv> --omega0 1.5 --sigma 3.0 \
      --offset 5 --amp 0.05 --radius 9 --k 8
"""
import argparse
import math
import sys
import os

import numpy as np

sys.path.insert(0, os.path.dirname(__file__))
from analyze_atomic_spectroscopy import read_phi, build_L18
import scipy.sparse as sp
import scipy.sparse.linalg as spla

C2 = 1.0 / 3.0


def gaussian_xpacket(L, c, sigma, offset, amp):
    """The campaign's single-x-component Gaussian flux packet, centered at
    (c+offset, c, c). Returns the SCALAR x-component field as a flat vector
    (the operator acts per-component; the x-component carries the whole packet)."""
    g = np.zeros(L * L * L)
    inv2s2 = 1.0 / (2.0 * sigma * sigma)
    half = int(math.ceil(4.0 * sigma))
    px, py, pz = c + offset, c, c
    for dx in range(-half, half + 1):
        for dy in range(-half, half + 1):
            for dz in range(-half, half + 1):
                x, y, z = px + dx, py + dy, pz + dz
                if not (0 <= x < L and 0 <= y < L and 0 <= z < L):
                    continue
                r2 = dx * dx + dy * dy + dz * dz
                g[(x * L + y) * L + z] = amp * math.exp(-r2 * inv2s2)
    return g


def probe_mask(L, c, R):
    m = np.zeros(L * L * L, dtype=bool)
    half = int(math.ceil(R))
    R2 = R * R
    for dx in range(-half, half + 1):
        for dy in range(-half, half + 1):
            for dz in range(-half, half + 1):
                if dx * dx + dy * dy + dz * dz > R2:
                    continue
                x, y, z = c + dx, c + dy, c + dz
                if not (0 <= x < L and 0 <= y < L and 0 <= z < L):
                    continue
                m[(x * L + y) * L + z] = True
    return m


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--phi", required=True)
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--sigma", type=float, default=3.0)
    ap.add_argument("--offset", type=int, default=5)
    ap.add_argument("--amp", type=float, default=0.05)
    ap.add_argument("--radius", type=float, default=9.0)
    ap.add_argument("--k", type=int, default=8)
    args = ap.parse_args()

    L, phi = read_phi(args.phi)
    c = L // 2
    A_lap = build_L18(L, periodic=True)
    A = (-C2) * A_lap + sp.diags(2.0 * args.omega0 * (-phi))
    vals, vecs = spla.eigsh(A, k=args.k, which="SA")
    order = np.argsort(vals)
    vals = vals[order]; vecs = vecs[:, order]
    omega = np.sqrt(np.clip(args.omega0 ** 2 + vals, 0, None))

    g = gaussian_xpacket(L, c, args.sigma, args.offset, args.amp)
    mask = probe_mask(L, c, args.radius)

    print("=" * 78)
    print(f"OFFSET-OVERLAP PREDICTION   L={L}  offset={args.offset}  sigma={args.sigma}"
          f"  radius={args.radius}")
    print(f"phi={args.phi}")
    print("=" * 78)
    print(f"{'n':>3} {'omega':>10} {'binding':>10} {'<psi|g>':>13} "
          f"{'<psi|g>_probe':>14} {'P_n/P_1s(probe)':>16}")

    # full-lattice modal coefficient and probe-restricted coefficient
    c_full = vecs.T @ g
    gp = g.copy(); gp[~mask] = 0.0
    # the autocorr C(t)=Σ_probe J0·J(t); J0=g restricted to probe, J(t)=Σ c_n ψ_n cos.
    # power at ω_n ∝ ( Σ_probe g·ψ_n )·c_n_full  (J0 on probe dotted into mode) squared-ish;
    # report both the full overlap and the probe-weighted overlap a_n = <g_probe|ψ_n>·<ψ_n|g>.
    a_n = np.array([ (gp @ vecs[:, n]) * c_full[n] for n in range(len(vals)) ])
    # 1s reference = the deepest bound mode (n=0)
    P = a_n ** 2
    P1s = P[0] if P[0] != 0 else float('nan')
    for n in range(len(vals)):
        cp = gp @ vecs[:, n]
        print(f"{n:>3} {omega[n]:>10.6f} {args.omega0-omega[n]:>+10.6f} "
              f"{c_full[n]:>+13.4e} {cp:>+14.4e} {P[n]/P1s:>16.4e}")
    print("-" * 78)
    print("Interpretation:")
    print("  <psi|g> ~ 0 at the 2p levels  => H-excite (packet doesn't excite 2p).")
    print("  <psi|g> large but <psi|g>_probe ~ 0 => H-probe (symmetric probe cancels 2p).")
    print("=" * 78)


if __name__ == "__main__":
    main()
