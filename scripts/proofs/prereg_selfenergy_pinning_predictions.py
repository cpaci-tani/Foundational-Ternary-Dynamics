"""
PREREG_SELFENERGY_PINNING_v1 -- frozen prediction generator.

Computes, exactly (k-space, no simulation), the self-energy of a unit ternary
charge's Gauss-forced flux field on odd-L periodic lattices, for the two
candidate operator families the engine's projector could realize:

  P1 (central-difference family, symbol Sum_i sin^2 k_i -- the +-2h composite
      div_c . grad_c; the fixed point the projector iteration converges to if
      the correction gradient is central-difference):
        J_i(k) = -i sin(k_i) shat / Sum_j sin^2 k_j
  P2 (matched 18-point family, symbol 4*sigma18(k) -- what a matched-stencil
      solve per FTD-0350 would realize):
        |J(k)|^2 = 1 / (4*sigma18(k))   (decomposition-independent)

Frozen observables per L in {17, 33, 65}:
  E_half   = (1/2) Sum_x |J|^2          (tracker convention -- CANONICAL for
                                         the conjecture, pinned in the prereg)
  E_local7 = Sum over charge site + 6 face nbrs of |J|^2   (evaporation-rule
                                         convention, no 1/2; P1 only --
                                         P2's real-space field is
                                         decomposition-dependent)
  E_term6  = (c^2/2) * pairs-once 18-pt gradient energy    (action convention;
                                         P1 only)
  p_evap   = 0.1 * exp(-E_local7 / K_B^2), K_B = 0.511     (derived, P1 only)

L -> infinity references: P1 -> W_SC = 0.5054620197173260 (SC Watson integral;
Glasser-Zucker closed form), P2 -> (1/2)*G18(0) computed by even-L
extrapolation.

Run:  python scripts/proofs/prereg_selfenergy_pinning_predictions.py
Odd-L values are exact finite-lattice numbers (machine precision), not fits.
"""
import numpy as np

K_B = 0.511
C2 = 1.0 / 3.0
W_SC = 0.5054620197173260


def sigma18(KX, KY, KZ):
    cx, cy, cz = np.cos(KX), np.cos(KY), np.cos(KZ)
    return 1.0 - (cx + cy + cz) / 6.0 - (cx * cy + cx * cz + cy * cz) / 6.0


def predictions_for_L(L):
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    KX, KY, KZ = np.meshgrid(k, k, k, indexing="ij")
    N = L ** 3

    s_arr = np.zeros((L, L, L))
    s_arr[0, 0, 0] = 1.0
    s_arr -= s_arr.mean()
    sh = np.fft.fftn(s_arr)

    # ---- P1: central-difference family ----
    sin2 = np.sin(KX) ** 2 + np.sin(KY) ** 2 + np.sin(KZ) ** 2
    nz = sin2 > 1e-14  # odd L: only k=0 excluded
    phi = np.zeros_like(sh)
    phi[nz] = sh[nz] / sin2[nz]
    J = np.stack(
        [np.real(np.fft.ifftn(-1j * np.sin(K) * phi)) for K in (KX, KY, KZ)],
        axis=-1,
    )
    mag2 = np.sum(J ** 2, axis=-1)
    E_half_P1 = 0.5 * float(np.sum(mag2))

    nbrs = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
    E_local7 = float(mag2[0, 0, 0]) + sum(
        float(mag2[dx % L, dy % L, dz % L]) for dx, dy, dz in nbrs
    )
    p_evap = 0.1 * np.exp(-E_local7 / K_B ** 2)

    # pairs-once Term-6 gradient energy (face 1/3 x6, edge 1/6 x12)
    grad_site = np.zeros((L, L, L))
    for ax in range(3):
        for sgn in (1, -1):
            d = np.roll(J, sgn, axis=ax) - J
            grad_site += (1.0 / 3.0) * np.sum(d ** 2, axis=-1)
    for (a, b) in [(0, 1), (0, 2), (1, 2)]:
        for sa in (1, -1):
            for sb in (1, -1):
                d = np.roll(np.roll(J, sa, axis=a), sb, axis=b) - J
                grad_site += (1.0 / 6.0) * np.sum(d ** 2, axis=-1)
    E_term6 = (C2 / 2.0) * 0.5 * float(np.sum(grad_site))

    # ---- P2: matched 18-point family (E_half only) ----
    s18 = 4.0 * sigma18(KX, KY, KZ)
    nz18 = s18 > 1e-14
    E_half_P2 = 0.5 * float(np.sum(1.0 / s18[nz18])) / N

    return E_half_P1, E_local7, p_evap, E_term6, E_half_P2


print("=" * 100)
print("PREREG_SELFENERGY_PINNING_v1 -- frozen predictions (exact odd-L values)")
print("=" * 100)
print(
    f"{'L':>4} {'P1 E_half':>14} {'P1 E_local7':>13} {'P1 p_evap/tick':>15} "
    f"{'P1 E_term6':>12} {'P2 E_half':>14}"
)
for L in (17, 33, 65):
    e1, e7, pe, e6, e2 = predictions_for_L(L)
    print(f"{L:>4} {e1:>14.9f} {e7:>13.9f} {pe:>15.6e} {e6:>12.9f} {e2:>14.9f}")

# L -> infinity reference for P2 via even-L extrapolation (1/L law)
vals = []
for L in (64, 128, 192):
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    KX, KY, KZ = np.meshgrid(k, k, k, indexing="ij")
    s18 = 4.0 * sigma18(KX, KY, KZ)
    nz18 = s18 > 1e-14
    vals.append((L, 0.5 * float(np.sum(1.0 / s18[nz18])) / L ** 3))
(La, Ea), (Lb, Eb) = vals[-2], vals[-1]
# model E(L) = E_inf - a/L:  a from the two points, then E_inf = E(Lb) + a/Lb
a_coef = (Eb - Ea) / (1.0 / La - 1.0 / Lb)
P2_inf = Eb + a_coef / Lb
print("-" * 100)
print(f"P1 L->inf reference: W_SC = {W_SC:.13f}")
print(f"P2 L->inf estimate:  (1/2)G18(0) ~= {P2_inf:.9f}   "
      f"(from L={La},{Lb} 1/L extrapolation; values {Ea:.9f}, {Eb:.9f})")
print(f"Discrimination gap P1 vs P2 at L=65: "
      f"{abs(predictions_for_L(65)[0] - predictions_for_L(65)[4]):.6f} "
      "(absolute, tracker convention)")
