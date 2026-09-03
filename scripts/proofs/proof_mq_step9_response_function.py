"""close_mq_step9.py — attack on step 9 of the M2-M4 chain.

Step 9 is the ONLY place G* enters the linear coefficient A = 16 G*^2, and it
enters solely through W_BCC = G*^2/(2 pi). If W_BCC is the wrong response
function there, A loses all Gamma-function content and the master quadratic has
no derivation from this chain.

Step 9 as stated:
    Gamma_pm(Phi) = 4 log[ 1 + 2 W_BCC y_pm (1 - cos Phi) ]
    Gamma_pm(Phi) = Gamma_pm(0) + (1/2) beta_pm Phi^2 + O(Phi^4)
    beta_pm = 8 W_BCC y_pm  ==  1/e^2,   x = 4 pi beta

T1  IDENTIFY the formula. Claim: it is exactly the rank-1 (single-site) defect
    determinant, via the matrix determinant lemma
        det(M0 + v e0 e0^T) = det(M0) * (1 + v * G0(0,0)),   G0(0,0) = W.
    Verified numerically by dense linear algebra on a finite torus.

T2  COMPUTE the object step 9 needs (a twist stiffness) and show it is a
    rational bond-geometry sum, never a Brillouin-zone integral.

T3  The DILEMMA: under either reading the chain fails to deliver 16 G*^2.

T4  Forward falsifier for the engine, under each reading.
"""
from __future__ import annotations
import itertools, math
from fractions import Fraction
import numpy as np
from mpmath import mp, mpf, gamma, pi, sqrt, quad, besseli, exp, inf

mp.dps = 30
G4 = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
W_BCC = G4 ** 2 / (2 * pi)


def rule(t):
    print("\n" + "=" * 78); print(t); print("=" * 78)


# neighbour sets, in the SAME convention the derivation's lambda uses
# (lambda(k) = (1/z) sum_delta cos(k.delta))
NEIGH = {
    "SC":  [d for d in itertools.product([-1, 0, 1], repeat=3) if sum(map(abs, d)) == 1],
    "BCC": [d for d in itertools.product([-1, 1], repeat=3)],                  # cos kx cos ky cos kz
    "FCC": [d for d in itertools.product([-1, 0, 1], repeat=3) if sum(map(abs, d)) == 2],
}
LAM = {
    "SC":  lambda k: (np.cos(k[0]) + np.cos(k[1]) + np.cos(k[2])) / 3,
    "BCC": lambda k: np.cos(k[0]) * np.cos(k[1]) * np.cos(k[2]),
    "FCC": lambda k: (np.cos(k[0]) * np.cos(k[1]) + np.cos(k[1]) * np.cos(k[2])
                      + np.cos(k[2]) * np.cos(k[0])) / 3,
}

rule("T0  CONVENTION CHECK: lambda(k) == (1/z) sum_delta cos(k.delta) ?")
rng = np.random.default_rng(7)
for name, ds in NEIGH.items():
    k = rng.uniform(-np.pi, np.pi, (500, 3))
    lhs = LAM[name](k.T)
    rhs = np.mean([np.cos(k @ np.array(d, float)) for d in ds], axis=0)
    print(f"  {name:4s} z={len(ds):2d}  max|lhs-rhs| = {np.max(np.abs(lhs-rhs)):.2e}  "
          f"-> {'OK' if np.max(np.abs(lhs-rhs)) < 1e-12 else 'MISMATCH'}")


# ─────────────────────────────────────────────────────────────────────
rule("T1  WHAT IS 4 log[1 + 2 W y (1-cos Phi)] ?  (matrix determinant lemma)")
# Build M0 = (1 - lambda(k) + m^2) on an N^3 torus, in real space, then add a
# single-site potential v at the origin and compare det ratio with 1 + v*G0(0,0).
N, m2, v = 8, 0.35, 0.7
ks = 2 * np.pi * np.arange(N) / N
KX, KY, KZ = np.meshgrid(ks, ks, ks, indexing="ij")
for name in ("SC", "BCC", "FCC"):
    diag_k = 1.0 - LAM[name]((KX, KY, KZ)) + m2          # inverse propagator in k-space
    G0_00 = float(np.mean(1.0 / diag_k))                  # = (1/N^3) sum_k 1/(1-lam+m^2)
    # real-space operator: M0 = F^dagger diag F  (circulant); rank-1 update at site 0
    idx = np.arange(N ** 3)
    r = np.stack(np.unravel_index(idx, (N, N, N)), axis=1)
    phase = np.exp(1j * (r @ np.stack([KX.ravel(), KY.ravel(), KZ.ravel()])))
    M0 = (phase * diag_k.ravel()) @ phase.conj().T / N ** 3
    M0 = np.real(M0)
    e0 = np.zeros(N ** 3); e0[0] = 1.0
    M1 = M0 + v * np.outer(e0, e0)
    s0, ld0 = np.linalg.slogdet(M0)
    s1, ld1 = np.linalg.slogdet(M1)
    lhs = ld1 - ld0                                       # log det(M0+V) - log det(M0)
    rhs = math.log(1 + v * G0_00)                         # matrix determinant lemma
    print(f"  {name:4s} log det ratio = {lhs:.12f}   log(1 + v*G0(0,0)) = {rhs:.12f}"
          f"   diff = {abs(lhs-rhs):.2e}")
print("\n  => step 9's functional form IS the single-site (rank-1) defect free energy,")
print("     with W playing the role of the LOCAL Green function G0(0,0).")
print("     It is a local susceptibility. It is not a twist response.")


# ─────────────────────────────────────────────────────────────────────
rule("T2  THE OBJECT STEP 9 NEEDS: the twist stiffness (k -> 0 curvature)")
print("A helicity modulus / inverse coupling 1/e^2 is the k->0 curvature of the")
print("INVERSE propagator: 1 - lambda(k) ~ (1/2) sum_munu M_munu k_mu k_nu.")
print("W is the Brillouin-zone INTEGRAL of the propagator. Opposite operations.\n")
print(f"  {'lattice':8s} {'z':>3s} {'Taylor coeff s of 1-lambda ~ s*k^2':>36s} "
      f"{'exact':>10s} {'W (BZ integral)':>18s}")
W_num = {"SC": float(quad(lambda t: exp(-t) * besseli(0, t / 3) ** 3, [0, inf])),
         "BCC": float(W_BCC),
         "FCC": float(9 * gamma(mpf(1) / 3) ** 6 / (2 ** (mpf(14) / 3) * pi ** 4))}
S_EXACT = {}
for name, ds in NEIGH.items():
    z = len(ds)
    # 1 - lambda = (1/z) sum_d (1-cos(k.d)) ~ (1/(2z)) sum_d (k.d)^2
    # isotropic part: coefficient of kx^2 is (1/(2z)) * sum_d d_x^2
    num = sum(d[0] ** 2 for d in ds)
    s_exact = Fraction(num, 2 * z)
    S_EXACT[name] = s_exact
    # numerical confirmation by finite difference along a random direction
    u = rng.normal(size=3); u /= np.linalg.norm(u)
    h = 1e-4
    s_num = (1 - LAM[name]((h * u[0], h * u[1], h * u[2]))) / h ** 2
    iso = all(sum(d[i] ** 2 for d in ds) == num for i in range(3))
    print(f"  {name:8s} {z:3d} {float(s_exact):36.12f} {str(s_exact):>10s} "
          f"{W_num[name]:18.12f}   (fd check {s_num:.9f}, isotropic={iso})")

print("\n  Every stiffness coefficient is a RATIONAL number: it is (1/2z) * sum_delta")
print("  delta_x^2, a finite sum over integer bond vectors. No lattice geometry can")
print("  put a Gamma function in a Taylor coefficient of a finite-range lambda(k).")
print(f"\n  BCC: s = {S_EXACT['BCC']} exactly, versus W_BCC = G*^2/(2 pi) = "
      f"{float(W_BCC):.12f}")
print(f"  ratio W_BCC / s = {float(W_BCC)/float(S_EXACT['BCC']):.12f} "
      f"(= 2*W_BCC, transcendental) -- these are not the same quantity.")


# ─────────────────────────────────────────────────────────────────────
rule("T3  THE DILEMMA")
KAPS = {"4pi": 4 * math.pi, "2pi": 2 * math.pi, "pi": math.pi, "1": 1.0}
ALPHA_INV = 137.035999177
print("Reading (i): beta is a LOCAL SUSCEPTIBILITY (what the formula computes).")
print("   Then W is correct, A = 16 G*^2 follows -- but beta is not 1/e^2, so the")
print("   step x = 4 pi beta -> alpha^-1 is unjustified. No alpha.\n")
print("Reading (ii): beta is a TWIST STIFFNESS (what 1/e^2 requires).")
print("   Then the coefficient is the rational s, not W. A = kappa*z*s:\n")
print(f"   {'lattice':8s} {'z':>3s} {'s':>6s} " + "".join(f"{'A(k='+n+')':>14s}" for n in KAPS))
hit = []
for name, ds in NEIGH.items():
    z, s = len(ds), float(S_EXACT[name])
    row = f"   {name:8s} {z:3d} {str(S_EXACT[name]):>6s} "
    for kn, kv in KAPS.items():
        A = kv * z * s
        row += f"{A:14.6f}"
        if ALPHA_INV < A < 2 * ALPHA_INV:
            hit.append((name, kn, A))
    print(row)
print(f"\n   A values in the admissible window (alpha^-1, 2*alpha^-1): {hit}")
print("   Every entry is a rational multiple of pi (or rational). G* is ABSENT.")
print("   16 G*^2 = %.6f is not reachable: it is transcendental of a kind the" % float(16 * G4 ** 2))
print("   bond-geometry sum cannot produce.")

print("\n  Cross-check of the exact algebraic claim:")
print(f"    A_step9  = 4pi * 8 * W_BCC = 4pi*8*G*^2/(2pi) = 16 G*^2 = "
      f"{float(4*pi*8*W_BCC):.12f}")
print(f"    A_stiff  = 4pi * 8 * (1/2)                    = 16 pi   = "
      f"{float(4*math.pi*8*0.5):.12f}")
print("    The pi cancels in the first ONLY because W carries a 1/(2 pi).")
print("    That 1/(2 pi) is the Watson identity, i.e. a BZ integral -- the very")
print("    thing a stiffness is not.")


# ─────────────────────────────────────────────────────────────────────
rule("T4  FORWARD FALSIFIER (engine-measurable, no knowledge of alpha required)")
Q = 1 / (16 * G4)
yp = (1 + sqrt(1 - 4 * Q)) / 2
print("Measure the compact-phase twist stiffness of a BCC-stencil lattice in the")
print("engine (uniform boundary twist, curvature of F at k->0), in units of the")
print("bond coupling. The two readings predict:\n")
print(f"   step-9 reading (beta = z*W*y+) : beta = {float(8*W_BCC*yp):.9f}")
print(f"   stiffness reading (beta = z*s*y+, s = 1/2) : beta = "
      f"{float(8*0.5*float(yp)):.9f}")
print(f"   bare harmonic stiffness (y+ -> 1)          : beta = {float(8*0.5):.9f}")
print("\nThese differ by a factor of 2*W_BCC = %.6f. The measurement is decisive"
      % float(2 * W_BCC))
print("and cannot be tuned, because no free parameter separates them.")
