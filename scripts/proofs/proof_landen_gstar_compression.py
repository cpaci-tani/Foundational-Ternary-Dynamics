"""
proof_landen_gstar_compression.py

Verification of the Landen-compression route to G* and pi.
Proposed 2026-05-21; verified here by high-precision computation.

WHAT THIS SCRIPT ESTABLISHES
  [THEOREM]  the three identities I1, I2, I3 below are exact (verified
             to >130 digits against independent Gamma / pi references).
  [FACT]     the Landen-compression recurrence reconstructs F0, L0
             (hence G*, pi) using only +, -, *, /, sqrt -- no Gamma,
             no K, no imported pi -- with quadratic (precision-doubling)
             convergence.

WHAT IT DOES NOT ESTABLISH
  This is NOT a new physics result and NOT a new theorem about G*.
  G* = Gamma(1/4)/Gamma(3/4) is already [THEOREM]; this is a new
  *presentation / computation* of G*, squarely in the Landen / AGM /
  Gauss-Legendre algorithmic family (the descending Landen recurrence
  IS the AGM, reparameterised). It strengthens the COMPUTATION of G*
  (algebraic-spine link 2), which was never the weak link; it does not
  touch the x_+ = 1/alpha identification (link 5).

IDENTITIES  (G* := Gamma(1/4)/Gamma(3/4);  F(z) := 2F1(1/2,1/2;1;z))
  F0 := F(1/2),   L0 := F'(1/2)/F(1/2)
  I1:  L0 = 4 / G*^2
  I2:  G* = 2 / sqrt(L0)
  I3:  pi = 2 / (L0 * F0^2)

SYMBOLIC PROOF OF I1  (recorded; the numeric check below confirms it)
  F(z) = (2/pi) K(sqrt z).  At z = 1/2 the modulus k = k' = 1/sqrt(2).
  F(1/2)  = (2/pi) K
  F'(1/2) = (2/pi) (2E - K)              [ d/dz of (2/pi)K, at z = 1/2 ]
  => L0 = (2E - K)/K = 2E/K - 1.
  Legendre relation at k = k' = 1/sqrt2:   2 E K - K^2 = pi/2
  => E = pi/(4K) + K/2  =>  2E/K = pi/(2 K^2) + 1
  => L0 = pi/(2 K^2).
  K(1/sqrt2) = Gamma(1/4)^2 / (4 sqrt pi)  =>  K^2 = Gamma(1/4)^4/(16 pi)
  => L0 = 8 pi^2 / Gamma(1/4)^4.
  Gamma(1/4) Gamma(3/4) = pi sqrt2  =>  Gamma(1/4)^4 = 2 pi^2 G*^2
  => L0 = 8 pi^2 / (2 pi^2 G*^2) = 4 / G*^2.            [ I1, QED ]
  I2 is algebra from I1.
  I3:  L0 F0^2 = (pi/2K^2) * (2/pi K)^2 = (pi/2K^2)(4K^2/pi^2) = 2/pi.

The Landen recurrence (descending Landen transformation of 2F1):
  z_0 = 1/2;  s_j = sqrt(1 - z_j);  y_j = (1-s_j)/(1+s_j);  z_{j+1}=y_j^2
  F_j   = (1 + y_j) F_{j+1}
  L_j   = A_j + B_j L_{j+1},   A_j = 1/(2 s_j (1+s_j)),
                               B_j = 2 y_j / (s_j (1+s_j)^2).
"""
import sys
import mpmath as mpm

mpm.mp.dps = 160
half = mpm.mpf(1) / 2

# -- independent reference values (these DO use Gamma and pi) --------------
F0_ref  = mpm.hyp2f1(half, half, 1, half)
Fp0_ref = mpm.mpf(1) / 4 * mpm.hyp2f1(mpm.mpf(3) / 2, mpm.mpf(3) / 2, 2, half)
L0_ref  = Fp0_ref / F0_ref
GSTAR   = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
PI      = +mpm.pi


def digits(a, b):
    """Decimal digits of agreement between a and b."""
    if a == b:
        return float(mpm.mp.dps)
    return float(-mpm.log10(abs(a - b) / abs(b)))


def landen_compress(m, T):
    """Reconstruct (F0, L0) by m Landen folds + a T-term terminal series.
    Uses only +, -, *, /, sqrt. No Gamma, no K, no imported pi."""
    z = half
    s_list, y_list = [], []
    for _ in range(m):
        s = mpm.sqrt(1 - z)
        y = (1 - s) / (1 + s)
        s_list.append(s)
        y_list.append(y)
        z = y * y
    zm = z
    # terminal series:  F(z) = sum_n c_n z^n,  c_n = (binom(2n,n)/4^n)^2
    # ratio c_{n+1}/c_n = ((2n+1)/(2n+2))^2
    F = mpm.mpf(0)
    Fp = mpm.mpf(0)
    c = mpm.mpf(1)
    zp = mpm.mpf(1)                       # zp = zm^n
    for n in range(0, T + 1):
        F += c * zp
        if n >= 1:
            Fp += n * c * (zp / zm)      # n c_n zm^{n-1}
        c *= (mpm.mpf(2 * n + 1) / mpm.mpf(2 * n + 2)) ** 2
        zp *= zm
    Lm = Fp / F
    Fj, Lj = F, Lm
    for j in range(m - 1, -1, -1):
        s, y = s_list[j], y_list[j]
        A = 1 / (2 * s * (1 + s))
        B = 2 * y / (s * (1 + s) ** 2)
        Fj = (1 + y) * Fj
        Lj = A + B * Lj
    return Fj, Lj


def main():
    print("=" * 70)
    print("  Landen-compression route to G* and pi  --  verification")
    print("=" * 70)
    print(f"  mp.dps = {mpm.mp.dps}")
    print(f"  G* = {mpm.nstr(GSTAR, 45)}")
    print(f"  F0 = {mpm.nstr(F0_ref, 45)}")
    print(f"  L0 = {mpm.nstr(L0_ref, 45)}")
    print("-" * 70)

    # -- the three identities ---------------------------------------------
    i1 = digits(L0_ref, 4 / GSTAR ** 2)
    i2 = digits(GSTAR, 2 / mpm.sqrt(L0_ref))
    i3 = digits(PI, 2 / (L0_ref * F0_ref ** 2))
    i4 = digits(F0_ref, GSTAR / mpm.sqrt(2 * PI))     # corollary F0 = G*/sqrt(2pi)
    print("  identity checks (decimal digits of agreement):")
    print(f"    I1   L0 = 4 / G*^2          : {i1:8.1f}")
    print(f"    I2   G* = 2 / sqrt(L0)      : {i2:8.1f}")
    print(f"    I3   pi = 2 / (L0 F0^2)     : {i3:8.1f}")
    print(f"    --   F0 = G* / sqrt(2 pi)   : {i4:8.1f}  (corollary)")
    thresh = mpm.mp.dps - 25
    ok_id = min(i1, i2, i3, i4) > thresh
    print(f"    => identities {'CONFIRMED' if ok_id else 'FAILED'} "
          f"(all agree to > {thresh} digits)")
    print("-" * 70)

    # -- Landen z-collapse -------------------------------------------------
    print("  Landen recurrence -- argument collapse z_{j+1} ~ z_j^2/16:")
    z = half
    for j in range(6):
        s = mpm.sqrt(1 - z)
        y = (1 - s) / (1 + s)
        z = y * y
        print(f"    z_{j + 1} = {mpm.nstr(z, 6)}")
    print("-" * 70)

    # -- reconstruction: fixed small T, growing m  (shows the doubling) ----
    print("  reconstruction accuracy, FIXED T=5, growing fold-count m")
    print("  (digits of agreement vs the independent references):")
    print(f"  {'m':>3} {'T':>4} {'F0':>9} {'L0':>9} {'G*':>9} {'pi':>9}")
    ok_recon = True
    for m in range(1, 7):
        Fr, Lr = landen_compress(m, 5)
        dF = digits(Fr, F0_ref)
        dL = digits(Lr, L0_ref)
        dG = digits(2 / mpm.sqrt(Lr), GSTAR)
        dP = digits(2 / (Lr * Fr ** 2), PI)
        print(f"  {m:>3} {5:>4} {dF:>9.1f} {dL:>9.1f} {dG:>9.1f} {dP:>9.1f}")
    print("-" * 70)

    # -- saturation: enough folds + terms recovers full precision ---------
    Fr, Lr = landen_compress(5, 25)
    dG = digits(2 / mpm.sqrt(Lr), GSTAR)
    dP = digits(2 / (Lr * Fr ** 2), PI)
    print(f"  saturation check  (m=5, T=25):  G* to {dG:.1f} digits, "
          f"pi to {dP:.1f} digits")
    ok_recon = min(dG, dP) > thresh
    print(f"  => recurrence {'CONFIRMED' if ok_recon else 'FAILED'}")
    print("=" * 70)

    if ok_id and ok_recon:
        print("  RESULT: all identities and the recurrence verified.")
        print("  Scope: a Landen/AGM-family computation of G* and pi;")
        print("  strengthens the COMPUTATION of G* (spine link 2),")
        print("  not a new theorem and not a physics result.")
        return 0
    print("  RESULT: a check FAILED -- see above.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
