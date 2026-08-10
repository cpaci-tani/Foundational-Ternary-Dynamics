"""derive_composite_cone.py — free multiparticle threshold kinematics.

THE QUESTION, POSED CORRECTLY.  "Does a composite inherit C_eff?" is
ill-posed until one supplies an interacting bound-state dispersion.  This
script computes the narrower free-particle question:

    a composite at total momentum K distributes K among its constituents;
    each constituent contributes its own c_a^2 in proportion to the share
    of the momentum it carries.

For free constituents at their fixed-total-momentum threshold, that
distribution is fixed
exactly by minimising the total energy at fixed total momentum,

    E(K) = min over {k_a : sum k_a = K} of  sum_a omega_a(k_a),

which gives the equal-velocity stationary condition.  This is not a bound
state: there is no interaction, normalizable internal wavefunction, binding
energy, or stress contribution.  Those missing terms can change the
center-of-mass dispersion and must be derived before applying the result to
nucleons, atoms, laboratory bodies, or astronomical bodies.

WHY IT MATTERS.  The two candidate answers differ enormously:

  (A) composite behaves like a fundamental particle of mass M_tot:
          delta = M_tot^2 / 6              -- grows as (number)^2
  (B) composite inherits a weighted average of its constituents:
          delta = sum_a w_a M_a^2 / 6      -- saturates at the CONSTITUENT
                                              scale, independent of N

The calculation distinguishes (A) and (B) only for the stated free
threshold.  It does not decide the corresponding bound-state or macroscopic
question.
"""
from __future__ import annotations

import numpy as np

C = 1.0 / np.sqrt(3.0)
C2 = C * C


def omega_axis(k, M):
    """Exact M18 lattice-KG dispersion along an axis:
    4 sin^2(w/2) = C^2 (-L) + M^2,  with -L(k,0,0) = 4 sin^2(k/2)."""
    W = 4.0 * C2 * np.sin(k / 2.0) ** 2 + M * M
    return 2.0 * np.arcsin(np.sqrt(W) / 2.0)


def species_c2(M, h=1e-4):
    """c_a^2 = m_a * omega''(0), with m_a = omega_a(0)."""
    m = omega_axis(0.0, M)
    w2 = (omega_axis(h, M) - 2 * omega_axis(0.0, M) + omega_axis(-h, M)) / h ** 2
    return m * w2, m


def composite_c2(masses, Kmax=2e-3, nK=9):
    """c_comp^2 from E(K) = min_{sum k_a = K} sum_a omega_a(k_a).

    The minimisation is done exactly for two constituents by scalar search
    on the split, and analytically for N identical ones (k_a = K/N by
    symmetry)."""
    masses = list(masses)
    Ks = np.linspace(0.0, Kmax, nK)
    E = np.empty(nK)
    if len(set(masses)) == 1:
        n = len(masses)
        for i, K in enumerate(Ks):
            E[i] = n * omega_axis(K / n, masses[0])
    else:
        assert len(masses) == 2, "explicit split search is coded for 2"
        M1, M2 = masses
        for i, K in enumerate(Ks):
            # equal-velocity split; bracket generously and refine
            qs = np.linspace(-abs(K) - 1e-6, 2 * abs(K) + 1e-6, 20001) \
                if K > 0 else np.array([0.0])
            E[i] = np.min(omega_axis(qs, M1) + omega_axis(K - qs, M2))
    E0 = E[0]
    # E(K) = E0 + c^2 K^2 / (2 E0):  fit the K^2 coefficient
    A = np.stack([Ks ** 2, Ks ** 4], -1)
    a2, _ = np.linalg.lstsq(A[1:], (E - E0)[1:], rcond=None)[0]
    return 2.0 * E0 * a2, E0


print("=" * 76)
print("(1) SINGLE SPECIES: c_a^2 / C^2 - 1  against the prediction M^2/6")
print("=" * 76)
print(f"{'M':>8} {'c_a^2/C^2 - 1':>18} {'M^2/6':>14} {'ratio':>10}")
for M in (0.05, 0.1, 0.2, 0.3, 0.5):
    c2a, m = species_c2(M)
    d = c2a / C2 - 1.0
    print(f"{M:8.3f} {d:18.10f} {M*M/6:14.10f} {d/(M*M/6):10.5f}")

print()
print("=" * 76)
print("(2) TWO UNEQUAL CONSTITUENTS: is delta_comp the weighted average?")
print("=" * 76)
print("    weights are the inertial-mass fractions  mu_a = m_a / c_a^2")
print(f"\n{'M1':>7} {'M2':>7} {'delta_comp':>14} {'weighted avg':>14} "
      f"{'as M_tot':>14} {'ratio A/B':>10}")
for M1, M2 in ((0.3, 0.3), (0.3, 0.5), (0.2, 0.6), (0.1, 0.5), (0.4, 0.45)):
    c2c, E0 = composite_c2((M1, M2))
    dc = c2c / C2 - 1.0
    (c1, m1), (c2, m2) = species_c2(M1), species_c2(M2)
    mu1, mu2 = m1 / c1, m2 / c2
    pred = (mu1 * (c1 / C2 - 1) + mu2 * (c2 / C2 - 1)) / (mu1 + mu2)
    naive = (M1 + M2) ** 2 / 6.0
    print(f"{M1:7.2f} {M2:7.2f} {dc:14.9f} {pred:14.9f} {naive:14.9f} "
          f"{naive/dc:10.3f}")

print()
print("=" * 76)
print("(3) N IDENTICAL CONSTITUENTS: does the effect grow with N?")
print("=" * 76)
m0 = 0.3
print(f"    constituent mass M = {m0};  a fundamental particle of the same")
print(f"    TOTAL mass would have delta = (N M)^2/6\n")
print(f"{'N':>4} {'M_tot':>8} {'delta_comp':>15} {'constituent M^2/6':>19}"
      f" {'(N M)^2/6':>14} {'suppression':>13}")
for n in (1, 2, 3, 5, 10, 30):
    c2c, E0 = composite_c2([m0] * n)
    dc = c2c / C2 - 1.0
    naive = (n * m0) ** 2 / 6.0
    print(f"{n:4d} {n*m0:8.2f} {dc:15.10f} {m0*m0/6:19.10f}"
          f" {naive:14.6f} {naive/dc:13.1f}")
print("""
    The composite's excess is the CONSTITUENT's, independent of N.  A
    fundamental particle of the same total mass would have N^2 times more.
    Answer: (B).""")

print()
print("=" * 76)
print("(4) SCOPE")
print("=" * 76)
print("""
    These are FREE MULTIPARTICLE THRESHOLDS, not bound-state dispersions.
    No inference to nucleons, atoms, kilogram-scale bodies, or the Earth is
    licensed until an interacting E_B(K) is derived with binding-field,
    stress, and renormalization contributions included.""")
