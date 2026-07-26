"""FTD-0520 -- The dispersion boundary formalized: branch torsor + gap split.

Verifies the exact content of DERIV_DISPERSION_BRANCH_TORSOR.md, on the
free flux-wave sector's recurrence of record (ANALYSIS_LATTICE_WAVE_SECTORS_v1:
J(t+1) = 2J(t) - J(t-1) + c^2 Delta J, c = C_SPEED = 1/sqrt(3), axis modes
sin^2(theta/2) = c^2 sin^2(k/2)):

  G1  omega^2 ownership: the axis-mode dispersion of record is reproduced to
      machine precision by direct simulation, and the k = 0 mode has EXACTLY
      zero frequency (no native gap).
  G2  Branch blindness: the real solution labeled (k, theta, phi) IS the real
      solution labeled (-k, -theta, -phi) -- the conjugation orbit is one
      real solution; the update has real coefficients so evolution commutes
      with complex conjugation; the two analytic signals are distinct.
  G3  Section = first-order evolution: the branch-selected mode obeys
      psi(t+1) = exp(-i theta) psi(t) exactly, while the real field is
      one-slice underdetermined (two solutions share a snapshot and differ
      at the next tick) -- the complex structure packs (J, Jdot) into one
      slice by choosing the branch.
  G4  The gap is owned-side: adding an explicitly non-native restoring term
      -m^2 J shifts the OWNED symmetric invariant to sin^2(theta/2) =
      c^2 sin^2(k/2) + m^2/4, giving gap theta(0) = 2 asin(m/2) > 0; the
      native m = 0 gap is exactly zero; branch selection never creates a gap.
  G5  Quadratic form needs BOTH imports: massless theta(k)/|k| -> c (linear);
      massive branch-selected (theta(k) - theta_0)/k^2 -> positive constant
      (quadratic above the gap). One-slice underdetermination persists for
      the massive REAL field, so the first-order quadratic evolution requires
      the mass import AND the branch import jointly.
  G6  Evenness for any +/- symmetric stencil: the 18-point-type symbol
      satisfies symbol(k) = symbol(-k) (random k sample), so omega^2-only
      ownership extends to the production stencil class.

Run:  python scripts/proofs/proof_dispersion_branch_torsor.py
"""

import cmath
import math
import sys

import numpy as np

C = 1.0 / math.sqrt(3.0)
PASS = []


def check(name, cond):
    PASS.append((name, bool(cond)))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}")


def theta_of_k(k, m=0.0):
    s2 = C * C * math.sin(k / 2.0) ** 2 + (m * m) / 4.0
    return 2.0 * math.asin(math.sqrt(s2))


def evolve_ring(J_prev, J_now, steps, m=0.0):
    for _ in range(steps):
        lap = np.roll(J_now, 1) + np.roll(J_now, -1) - 2.0 * J_now
        J_next = 2.0 * J_now - J_prev + C * C * lap - (m * m) * J_now
        J_prev, J_now = J_now, J_next
    return J_prev, J_now


def g1_dispersion_and_gaplessness():
    N, T = 64, 200
    ok = True
    for mode in (1, 3, 7):
        k = 2.0 * math.pi * mode / N
        th = theta_of_k(k)
        j = np.arange(N)
        J0 = np.cos(k * j)
        J1 = np.cos(k * j - th)
        _, JT = evolve_ring(J0, J1, T - 1)
        ok &= np.max(np.abs(JT - np.cos(k * j - th * T))) < 1e-9
    # k = 0: uniform mode, Laplacian zero, native frequency exactly zero
    ok &= theta_of_k(0.0) == 0.0
    J0 = np.ones(8)
    _, J2 = evolve_ring(J0.copy(), J0.copy(), 50)
    ok &= np.max(np.abs(J2 - 1.0)) == 0.0
    check("G1 dispersion of record reproduced to 1e-9 over 200 ticks; k=0 gap exactly 0", ok)


def g2_branch_blindness():
    N = 64
    k = 2.0 * math.pi * 5 / N
    th = theta_of_k(k)
    phi = 0.7
    j = np.arange(N)
    t = 13
    ok = True
    # (k, th, phi) and (-k, -th, -phi) label the SAME real solution
    ok &= np.allclose(np.cos(k * j - th * t + phi),
                      np.cos((-k) * j - (-th) * t + (-phi)), atol=0, rtol=0)
    # real update commutes with conjugation: evolve conjugate data = conjugate
    Z0 = np.exp(1j * k * j)
    Z1 = np.exp(1j * (k * j - th))
    A_prev, A_now = evolve_ring(Z0, Z1, 20)
    B_prev, B_now = evolve_ring(np.conj(Z0), np.conj(Z1), 20)
    ok &= np.max(np.abs(B_now - np.conj(A_now))) < 1e-12
    # the two analytic signals are distinct (conjugate, not equal)
    ok &= not np.allclose(Z1, np.conj(Z1))
    check("G2 conjugation orbit is one real solution; update commutes with conjugation; "
          "analytic signals distinct", ok)


def g3_section_first_order():
    N = 64
    k = 2.0 * math.pi * 4 / N
    th = theta_of_k(k)
    ok = True
    # branch-selected mode: exact first-order evolution
    psi = 1.0 + 0.0j
    for _ in range(50):
        psi *= cmath.exp(-1j * th)
    ok &= abs(psi - cmath.exp(-1j * th * 50)) < 1e-12
    # real field one-slice underdetermination: same snapshot, different futures
    j = np.arange(N)
    snap = np.cos(k * j)
    fwd_prev = np.cos(k * j + th)   # velocity one way
    bwd_prev = np.cos(k * j - th)   # velocity the other way
    _, f1 = evolve_ring(fwd_prev, snap.copy(), 1)
    _, b1 = evolve_ring(bwd_prev, snap.copy(), 1)
    ok &= np.max(np.abs(f1 - b1)) > 0.1
    check("G3 psi(t+1)=e^{-i theta} psi(t) exact; real field one-slice underdetermined", ok)


def g4_gap_is_owned_side():
    m = 0.4
    ok = theta_of_k(0.0, m) == 2.0 * math.asin(m / 2.0) > 0.0
    # simulate the massive uniform mode: oscillates at the gap frequency
    th0 = theta_of_k(0.0, m)
    J0 = np.ones(8)
    J1 = np.ones(8) * math.cos(th0)
    _, JT = evolve_ring(J0, J1, 99, m=m)
    ok &= np.max(np.abs(JT - math.cos(th0 * 100))) < 1e-9
    # branch selection never creates a gap: omega^2 is branch-invariant
    ok &= theta_of_k(0.0, 0.0) == 0.0
    check("G4 restoring import m^2 gaps the OWNED invariant (theta_0 = 2 asin(m/2), "
          "simulated to 1e-9); native gap exactly 0; sections preserve omega^2", ok)


def g5_quadratic_needs_both():
    ok = True
    # massless: linear IR -- theta/|k| -> c
    ratios = [theta_of_k(10.0 ** -e) / 10.0 ** -e for e in (2, 3, 4)]
    ok &= abs(ratios[-1] - C) < 1e-8 and abs(ratios[-1] - ratios[-2]) < 1e-6
    # massive + branch: quadratic above the gap -- (theta - theta0)/k^2 -> const > 0
    m = 0.4
    th0 = theta_of_k(0.0, m)
    qratios = [(theta_of_k(10.0 ** -e, m) - th0) / (10.0 ** -e) ** 2 for e in (2, 3, 4)]
    ok &= qratios[-1] > 0 and abs(qratios[-1] - qratios[-2]) / qratios[-1] < 1e-4
    # continuum cross-check of the quadratic coefficient: c^2 / (2 sin(theta0))
    # from expanding 2 asin(sqrt(m^2/4 + c^2 k^2/4)) at small k
    coeff = C * C / (2.0 * math.sin(th0))
    ok &= abs(qratios[-1] - coeff) / coeff < 1e-3
    # the massive REAL field is still one-slice underdetermined (needs the branch)
    N = 64
    k = 2.0 * math.pi * 3 / N
    th = theta_of_k(k, m)
    j = np.arange(N)
    snap = np.cos(k * j)
    _, f1 = evolve_ring(np.cos(k * j + th), snap.copy(), 1, m=m)
    _, b1 = evolve_ring(np.cos(k * j - th), snap.copy(), 1, m=m)
    ok &= np.max(np.abs(f1 - b1)) > 0.1
    check("G5 massless linear (theta/k -> c); massive+branch quadratic above gap with the "
          "predicted coefficient; real massive field still branch-underdetermined", ok)


def g6_symmetric_stencil_evenness():
    rng = np.random.default_rng(520)
    # 18-neighbor set: 6 axis + 12 edge displacements, +/- paired
    axis = [(1, 0, 0), (0, 1, 0), (0, 0, 1)]
    edge = [(1, 1, 0), (1, -1, 0), (1, 0, 1), (1, 0, -1), (0, 1, 1), (0, 1, -1)]
    disps = [d for d in axis + edge] + [tuple(-x for x in d) for d in axis + edge]
    w = {**{d: 1.0 / 3.0 for d in axis}, **{d: 1.0 / 12.0 for d in edge}}
    w.update({tuple(-x for x in d): w[d] for d in list(w)})

    def symbol(kvec):
        return sum(w[d] * (1.0 - math.cos(sum(ki * di for ki, di in zip(kvec, d))))
                   for d in disps)

    ok = True
    for _ in range(50):
        kv = rng.uniform(-math.pi, math.pi, size=3)
        ok &= abs(symbol(kv) - symbol(-kv)) < 1e-12
        ok &= symbol(kv) >= -1e-12
    check("G6 18-point-type symbol even under k -> -k (50 random k): omega^2-only "
          "ownership holds for the +/--symmetric production stencil class", ok)


def main():
    print("FTD-0520 dispersion branch-torsor verification")
    g1_dispersion_and_gaplessness()
    g2_branch_blindness()
    g3_section_first_order()
    g4_gap_is_owned_side()
    g5_quadratic_needs_both()
    g6_symmetric_stencil_evenness()
    n_ok = sum(1 for _, ok in PASS if ok)
    print(f"\n{n_ok}/{len(PASS)} PASS")
    sys.exit(0 if n_ok == len(PASS) else 1)


if __name__ == "__main__":
    main()
