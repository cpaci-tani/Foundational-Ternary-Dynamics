"""proof_bcc_twist_stiffness_measurement.py — FTD-1025 forward falsifier.

Measures the compact-phase twist stiffness (helicity modulus) of the BCC
neighbour set that the M2-M4 chain's own lambda(k) = cos kx cos ky cos kz
specifies, and compares it with the two readings of step 9.

    step-9 reading   beta / y = z * W_BCC = 8 * 1.393203929686 = 11.145631
    stiffness reading beta / y = z * s     = 8 * (1/2)          =  4.000000

The measurement is alpha-blind: nothing here knows the fine-structure constant.

M1  RIGOROUS BOUND.  For H = -J sum_b cos(dtheta_b), the standard helicity
    modulus estimator is
        Upsilon = (1/V)[ J <sum_b (xhat.delta_b)^2 cos dtheta_b>
                         - beta J^2 <( sum_b (xhat.delta_b) sin dtheta_b )^2> ].
    First term <= (1/V) J sum_b (xhat.delta_b)^2; second term >= 0. So
    Upsilon <= J * (1/V) sum_b (xhat.delta_b)^2, a purely geometric ceiling.

M2  EXACT GAUSSIAN TWIST.  Impose theta_r = phi_r + k.r on the harmonic model
    and minimise; the free-energy curvature is the same bond sum, exactly.

M3  MONTE CARLO.  Compact XY on the BCC neighbour set, heat-bath (von Mises)
    two-colour updates, helicity modulus by the fluctuation estimator.
"""
from __future__ import annotations
import itertools, math
import numpy as np
from mpmath import mp, mpf, gamma, pi, sqrt

mp.dps = 25
G4 = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
W_BCC = float(G4 ** 2 / (2 * pi))
Q = 1 / (16 * G4)
Y_PLUS = float((1 + sqrt(1 - 4 * Q)) / 2)

# BCC nearest neighbours: the 8 body-diagonal shifts. This is exactly the set
# for which (1/8) sum_delta cos(k.delta) = cos kx cos ky cos kz.
NEIGH = [d for d in itertools.product([-1, 1], repeat=3)]
Z = len(NEIGH)
# each bond once: the 4 shifts with delta_x = +1
BONDS = [d for d in NEIGH if d[0] == 1]
assert len(BONDS) == Z // 2


def rule(t):
    print("\n" + "=" * 78); print(t); print("=" * 78)


rule("M0  PREDICTIONS UNDER TEST  (alpha-blind)")
PRED_STEP9 = Z * W_BCC
PRED_STIFF = Z * 0.5
print(f"  W_BCC = G*^2/(2 pi)            = {W_BCC:.12f}")
print(f"  y+    = (1+sqrt(1-4/(16G*)))/2 = {Y_PLUS:.12f}")
print(f"  step-9 reading   Upsilon/J = z*W_BCC = {PRED_STEP9:.9f}"
      f"   (beta = {PRED_STEP9*Y_PLUS:.9f} after the y+ factor)")
print(f"  stiffness reading Upsilon/J = z*s    = {PRED_STIFF:.9f}"
      f"   (beta = {PRED_STIFF*Y_PLUS:.9f} after the y+ factor)")
print(f"  ratio between readings = 2*W_BCC = {2*W_BCC:.9f}")


rule("M1  RIGOROUS GEOMETRIC CEILING")
# (1/V) sum_b (xhat.delta_b)^2, bonds counted once
ceiling = sum(d[0] ** 2 for d in BONDS)
print(f"  bonds per site (each counted once) : {len(BONDS)}")
print(f"  (1/V) sum_b (xhat.delta_b)^2       : {ceiling}")
print(f"  => Upsilon/J <= {ceiling} for ANY temperature and ANY state,")
print(f"     because <cos> <= 1 and the fluctuation term is subtracted.")
print(f"\n  step-9 prediction {PRED_STEP9:.6f} exceeds this ceiling by a factor "
      f"{PRED_STEP9/ceiling:.6f} = 2*W_BCC.")
print(f"  It is therefore UNREACHABLE, not merely unlikely.")


rule("M2  EXACT GAUSSIAN TWIST  (deterministic, no sampling)")
# Harmonic model S = (J/2) sum_b (dtheta_b)^2 with theta_r = phi_r + k.r.
# Linear term vanishes for periodic phi; F(k)-F(0) = (J/2) k^2 sum_b (xhat.delta)^2.
# Verified here against a direct k-space evaluation of z*(1-lambda(k))/k^2 -> z*s.
for kmag in (1e-2, 1e-3, 1e-4):
    lam = math.cos(kmag) * 1.0 * 1.0            # k along xhat: cos(kx)*cos(0)*cos(0)
    val = Z * (1 - lam) / kmag ** 2
    print(f"  k = {kmag:7.0e}   z*(1-lambda(k))/k^2 = {val:.10f}")
print(f"  limit k->0 : z*s = {Z}*1/2 = {PRED_STIFF:.6f}   (matches the bond sum "
      f"{ceiling})")


rule("M3  MONTE CARLO — compact XY on the BCC neighbour set")
print("  H = -J sum_b cos(theta_i - theta_j),  J = 1,  heat-bath (von Mises),")
print("  two-colour update (the 8 body-diagonal bonds flip parity of x+y+z).\n")


def measure(N: int, T: float, sweeps: int, burn: int, seed: int):
    rng = np.random.default_rng(seed)
    th = rng.uniform(0, 2 * np.pi, (N, N, N))
    x, y, z = np.indices((N, N, N))
    colour = (x + y + z) % 2
    masks = [colour == 0, colour == 1]
    beta = 1.0 / T
    acc_t1, acc_t2, acc_t2sq, n = 0.0, 0.0, 0.0, 0

    for sweep in range(sweeps):
        for m in masks:
            h = np.zeros((N, N, N), dtype=complex)
            for d in NEIGH:
                h += np.exp(1j * np.roll(th, shift=(-d[0], -d[1], -d[2]), axis=(0, 1, 2)))
            kap = beta * np.abs(h[m])
            mu = np.angle(h[m])
            th[m] = np.random.default_rng(rng.integers(1 << 62)).vonmises(mu, kap)
        if sweep < burn:
            continue
        t1 = 0.0
        s_sum = 0.0
        for d in BONDS:                      # each bond once; xhat.delta = +1
            dth = th - np.roll(th, shift=(-d[0], -d[1], -d[2]), axis=(0, 1, 2))
            t1 += np.sum(np.cos(dth))
            s_sum += np.sum(np.sin(dth))
        acc_t1 += t1
        acc_t2 += s_sum
        acc_t2sq += s_sum ** 2
        n += 1

    V = N ** 3
    term1 = acc_t1 / n / V                                    # J * <sum cos>/V
    var_s = acc_t2sq / n - (acc_t2 / n) ** 2                  # <S^2> - <S>^2
    term2 = beta * var_s / V
    return term1 - term2, term1, term2


print(f"  {'N':>3s} {'T':>5s} {'sweeps':>7s} {'Upsilon/J':>11s} {'term1':>9s} "
      f"{'term2':>9s} {'<= 4 ?':>7s} {'vs step9':>9s}")
rows = []
for N, T, sw in ((12, 0.20, 6000), (12, 0.50, 6000), (12, 1.00, 6000),
                 (12, 1.50, 6000), (12, 2.00, 6000), (16, 0.50, 4000)):
    ups, t1, t2 = measure(N, T, sw, sw // 4, seed=1000 + N + int(100 * T))
    ok = "yes" if ups <= ceiling + 1e-9 else "NO"
    rows.append((N, T, ups))
    print(f"  {N:3d} {T:5.2f} {sw:7d} {ups:11.6f} {t1:9.5f} {t2:9.5f} {ok:>7s} "
          f"{ups/PRED_STEP9:9.4f}")

rule("VERDICT")
best = max(r[2] for r in rows)
print(f"  highest measured Upsilon/J over all runs : {best:.6f}")
print(f"  geometric ceiling                        : {ceiling}")
print(f"  stiffness-reading prediction             : {PRED_STIFF:.6f}")
print(f"  step-9 prediction                        : {PRED_STEP9:.6f}")
print()
if best <= ceiling + 1e-9 and best < PRED_STEP9:
    print("  The measured twist stiffness never approaches z*W_BCC and cannot,")
    print(f"  being capped at {ceiling} by bond geometry. The step-9 identification")
    print("  beta = z*W*y is REFUTED as a twist response.")
    print("  Consistent with AUDIT_MQ_STEP9_RESPONSE_FUNCTION.md: W = G_0(0,0) is a")
    print("  local susceptibility, not a stiffness. FTD-1025 closure CONFIRMED by")
    print("  independent measurement.")
else:
    print("  UNEXPECTED: the measurement does not refute the step-9 reading.")
    print("  The FTD-1025 closure must be reopened.")
