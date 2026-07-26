"""FTD-0565 -- Coat Peierls-clock toy (EXPLORATORY; Python quick-check only).

Probes the un-priced tine of the FTD-0552 fork: instead of subtracting the
quadratic coat's self-force, ask what it buys. In a 1D/2D neutral-jellium
lattice toy with the tensor B-spline coat and the periodic lattice Green
function, measure:

  H1  Neutrality and partition of unity are exact for every coat order.
  H2  The self-energy U(x) is periodic with stationary points at integer
      and half-integer subcell positions and nonzero force at generic x
      (the FTD-0552 mechanism reproduced in the toy).
  H3  The internal clock exists: small oscillations about the well minimum
      ring at omega_0 = sqrt(U''(x_min)/m); direct simulation of
      x'' = -U'(x)/m matches the curvature prediction.
  H4  Peierls-Nabarro width scaling: the corrugation Delta_U falls steeply
      with coat smoothness (B1 hat -> B2 quadratic -> B3 cubic).
  H5  Locality: the corrugation and omega_0 are insensitive to ring size
      (N = 32 vs 64), i.e. the well is a local object.
  H6  Separability test of the 2026-07-26 savant prediction: is
      U(x,y) - U(x,0) - U(0,y) + U(0,0) small against the corrugation?
      (An honest kill-test: the Green function is not a tensor product,
      so the prediction may die here. Whatever obtains is reported.)

Scope guards: bare inertia m = 1 and coupling g = 1 are TOY choices; no
engine claim, no MeV claim, no identification of omega_0 with any physical
frequency. Canonical confirmation requires the C++ coat implementation.

Run:  python scripts/exploration/explore_coat_peierls_clock.py
"""

import math
import sys

import numpy as np

PASS = []


def check(name, cond):
    PASS.append((name, bool(cond)))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}")


def bspline(order, t):
    a = abs(t)
    if order == 1:  # hat, support (-1, 1)
        return max(0.0, 1.0 - a)
    if order == 2:  # quadratic, support (-1.5, 1.5)
        if a <= 0.5:
            return 0.75 - a * a
        if a <= 1.5:
            return 0.5 * (1.5 - a) ** 2
        return 0.0
    if order == 3:  # cubic, support (-2, 2)
        if a <= 1.0:
            return (4.0 - 6.0 * a * a + 3.0 * a ** 3) / 6.0
        if a <= 2.0:
            return (2.0 - a) ** 3 / 6.0
        return 0.0
    raise ValueError(order)


def green_1d(N):
    """Periodic 1D lattice Green function of -Laplacian, zero mode excluded."""
    m = np.arange(1, N)
    lam = 2.0 - 2.0 * np.cos(2.0 * np.pi * m / N)
    j = np.arange(N)
    G = np.zeros((N, N))
    for d in range(N):
        G[j, (j + d) % N] = np.sum(np.cos(2.0 * np.pi * m * d / N) / lam) / N
    return G


def coat_density(N, x, order):
    rho = np.array([bspline(order, ((j - x + N / 2) % N) - N / 2) for j in range(N)])
    return rho - np.sum(rho) / N  # neutralizing uniform background


def self_energy(N, x, order, G):
    rho = coat_density(N, x, order)
    return 0.5 * rho @ G @ rho


def h1_neutrality():
    ok = True
    rng = np.random.default_rng(565)
    for order in (1, 2, 3):
        for x in rng.uniform(0, 1, 5):
            raw = sum(bspline(order, j - x) for j in range(-4, 6))
            ok &= abs(raw - 1.0) < 1e-12                     # partition of unity
            ok &= abs(np.sum(coat_density(32, x, order))) < 1e-12  # neutral
    check("H1 partition of unity + exact neutrality (B1/B2/B3, random x)", ok)


def scan_U(N, order, G, xs):
    return np.array([self_energy(N, x, order, G) for x in xs])


def h2_landscape(G32):
    N, order = 32, 2
    xs = np.linspace(0.0, 1.0, 201)
    U = scan_U(N, order, G32, xs)
    ok = abs(U[0] - U[-1]) < 1e-12                      # period 1
    h = 1e-4                                            # central-difference forces
    for x0 in (0.0, 0.5):                               # stationary by symmetry
        ok &= abs(self_energy(N, x0 + h, order, G32)
                  - self_energy(N, x0 - h, order, G32)) / (2 * h) < 1e-9
    generic = (self_energy(N, 0.25 + h, order, G32)
               - self_energy(N, 0.25 - h, order, G32)) / (2 * h)
    ok &= abs(generic) > 1e-4                           # self-force at generic x
    corr = U.max() - U.min()
    print(f"    B2 corrugation Delta_U = {corr:.6e}; U(1/2)-U(0) = {U[100]-U[0]:+.6e}")
    check("H2 U(x) periodic; stationary at 0 and 1/2; nonzero generic self-force", ok)
    return xs, U


def h3_internal_clock(G32, xs, U):
    N, order = 32, 2
    x_min = xs[np.argmin(U)]
    h = 1e-3
    Upp = (self_energy(N, x_min + h, order, G32) - 2 * self_energy(N, x_min, order, G32)
           + self_energy(N, x_min - h, order, G32)) / h**2
    omega0 = math.sqrt(max(Upp, 0.0))
    # leapfrog integration of x'' = -U'(x), small amplitude
    amp = 5e-3
    x, v = x_min + amp, 0.0
    dt = 0.02 / max(omega0, 1e-9)
    T = int(6 * math.pi / (omega0 * dt))
    crossings = []
    for step in range(T):
        Fm = -(self_energy(N, x + h, order, G32) - self_energy(N, x - h, order, G32)) / (2 * h)
        v += Fm * dt
        x_new = x + v * dt
        if (x - x_min - 0) * (x_new - x_min) < 0:
            crossings.append(step * dt)
        x = x_new
    ok = omega0 > 0 and len(crossings) >= 4
    if ok:
        period = 2.0 * np.mean(np.diff(crossings))
        omega_sim = 2.0 * math.pi / period
        rel = abs(omega_sim - omega0) / omega0
        print(f"    omega_0(curvature) = {omega0:.6f}; omega(sim) = {omega_sim:.6f}; rel = {rel:.2e}")
        ok &= rel < 5e-2
    check("H3 internal clock: simulated oscillation matches sqrt(U'') to <5%", ok)
    return omega0


def h4_width_scaling(G32):
    N = 32
    xs = np.linspace(0.0, 1.0, 101)
    corr = {}
    for order in (1, 2, 3):
        U = scan_U(N, order, G32, xs)
        corr[order] = U.max() - U.min()
    r12, r23 = corr[1] / corr[2], corr[2] / corr[3]
    print(f"    corrugation: B1 = {corr[1]:.3e}, B2 = {corr[2]:.3e}, B3 = {corr[3]:.3e} "
          f"(ratios {r12:.1f}x, {r23:.1f}x)")
    ok = corr[1] > 2 * corr[2] > 0 and corr[2] > 2 * corr[3] > 0
    check("H4 Peierls width scaling: corrugation falls monotonically and steeply "
          "B1 -> B2 -> B3 (>2x each; ratios reported)", ok)


def h5_locality(G32, omega0_32):
    G64 = green_1d(64)
    xs = np.linspace(0.0, 1.0, 101)
    c32 = float(np.ptp(scan_U(32, 2, G32, xs)))
    c64 = float(np.ptp(scan_U(64, 2, G64, xs)))
    h = 1e-3
    x0 = 0.0 if scan_U(64, 2, G64, np.array([0.0]))[0] < scan_U(64, 2, G64, np.array([0.5]))[0] else 0.5
    Upp64 = (self_energy(64, x0 + h, 2, G64) - 2 * self_energy(64, x0, 2, G64)
             + self_energy(64, x0 - h, 2, G64)) / h**2
    om64 = math.sqrt(max(Upp64, 0.0))
    rel_c = abs(c64 - c32) / c32
    rel_o = abs(om64 - omega0_32) / omega0_32
    print(f"    corrugation N=32 vs 64: rel diff = {rel_c:.2e}; omega_0 rel diff = {rel_o:.2e}")
    check("H5 locality: corrugation and omega_0 shift <2% from N=32 to N=64",
          rel_c < 2e-2 and rel_o < 2e-2)


def h6_separability():
    N = 20
    m1 = np.arange(N)
    kx, ky = np.meshgrid(2 * np.pi * m1 / N, 2 * np.pi * m1 / N, indexing="ij")
    lam = (2 - 2 * np.cos(kx)) + (2 - 2 * np.cos(ky))
    lam[0, 0] = np.inf  # exclude zero mode

    def U2(x, y):
        w = np.array([[bspline(2, ((i - x + N / 2) % N) - N / 2)
                       * bspline(2, ((j - y + N / 2) % N) - N / 2)
                       for j in range(N)] for i in range(N)])
        w -= w.sum() / N**2
        F = np.fft.fft2(w)
        return 0.5 * float(np.sum(np.abs(F) ** 2 / lam)) / N**2

    xs = np.linspace(0.0, 0.5, 6)
    corr1d = max(U2(x, 0.0) for x in xs) - min(U2(x, 0.0) for x in xs)
    worst = 0.0
    for x in xs:
        for y in xs:
            R = U2(x, y) - U2(x, 0.0) - U2(0.0, y) + U2(0.0, 0.0)
            worst = max(worst, abs(R))
    ratio = worst / corr1d
    print(f"    2D nonseparability residual / 1D corrugation = {ratio:.3f}")
    # honest kill-test: PASS means the measurement ran; the verdict is the ratio
    check(f"H6 separability measured (residual/corrugation = {ratio:.3f}; "
          f"{'separable to <10%' if ratio < 0.1 else 'NOT separable — savant prediction refuted'})",
          True)
    return ratio


def main():
    print("FTD-0565 coat Peierls-clock toy (EXPLORATORY)")
    h1_neutrality()
    G32 = green_1d(32)
    xs, U = h2_landscape(G32)
    om = h3_internal_clock(G32, xs, U)
    h4_width_scaling(G32)
    h5_locality(G32, om)
    h6_separability()
    n_ok = sum(1 for _, ok in PASS if ok)
    print(f"\n{n_ok}/{len(PASS)} PASS")
    sys.exit(0 if n_ok == len(PASS) else 1)


if __name__ == "__main__":
    main()
