"""Is nine squares minimal for the M18 flux symbol?

FTD-0816 exhibits  -L18 = (4/3) sum_i sin^2(q_i/2)
                         + (2/3) sum_{i<j} [sin^2((q_i-q_j)/2) + sin^2((q_i+q_j)/2)]
and prices it at spinor dimension 16, because n squares demand n mutually
anticommuting structures and dimension 2^k carries 2k+1.  Nine first fits at
16.  Seven would fit at 8.  So whether nine is MINIMAL is worth an hour: it
is the difference between a fourfold and a twofold Dirac price.

WHAT IS ASKED.  Write x = q/2 and choose the corresponding half-angle
nearest-neighbour function class V:
trigonometric polynomials of degree <= 1 per variable in x, i.e. the span of
the 27 monomials prod_i g_i with g_i in {1, cos x_i, sin x_i}.  Minimise n
subject to  p = sum_{a=1}^n f_a^2  with every f_a in V.

SCOPE, stated up front.  This is minimality over V, the hopping range the
physics wants -- NOT over all trigonometric polynomials.  For ordinary
polynomials the degree bound deg f <= deg(p)/2 is a theorem, because the
top-degree forms satisfy sum (top)^2 = 0 and cannot cancel.  On the torus
that argument fails: the extreme-frequency coefficient of sum f_a^2 is
sum_a c_{a,K}^2, a sum of COMPLEX squares, which can vanish.  So a longer-
range hop is not excluded here, only a longer-range hop is also not what a
local matter sector may use.

DECOMPOSITION (phase 0).
  1. [exact]   a closed form for p that is independent of the FTD-0816
               route, as a cross-check on both.
  2. [exact]   the ambient space is 19-dimensional, not 27, forced by the
               zero set.
  3. [exact]   a zero-set lower bound on n.
  4. [search]  Burer-Monteiro least squares for n = 9 down to 3.
  5. [numeric] anything the search finds is rechecked on a DFT grid and
               fresh momenta.  It remains a numerical candidate: no
               symbolic, rational-reconstruction or interval existence
               certificate is supplied; a failed descent is evidence only.

Reproduction:  python scripts/experiments/temporal_interior/derive_sos_rank_minimal.py
"""

import itertools

import numpy as np
import sympy as sp
from scipy.optimize import least_squares

SEED = 20260808
GS = ("1", "c", "s")            # the three factors available per axis
MONOMIALS = [m for m in itertools.product(GS, repeat=3)]      # 27


# ---------------------------------------------------------------------------
# 1. The symbol, two ways
# ---------------------------------------------------------------------------

def symbolic_checks():
    """Confirm the closed form and the nine-square identity, exactly."""
    q = sp.symbols("q1 q2 q3", real=True)
    x = sp.symbols("x1 x2 x3", real=True)

    mL = (4 - sp.Rational(2, 3) * sum(sp.cos(qi) for qi in q)
          - sp.Rational(2, 3) * sum(sp.cos(q[i]) * sp.cos(q[j])
                                    for i, j in ((0, 1), (1, 2), (2, 0))))

    # FTD-0816's decomposition, in x = q/2.
    nine = (sp.Rational(4, 3) * sum(sp.sin(xi) ** 2 for xi in x)
            + sp.Rational(2, 3) * sum(sp.sin(x[i] - x[j]) ** 2
                                      + sp.sin(x[i] + x[j]) ** 2
                                      for i, j in ((0, 1), (1, 2), (2, 0))))
    sub = {q[i]: 2 * x[i] for i in range(3)}
    r1 = sp.simplify(sp.expand_trig(mL.subs(sub) - nine))
    assert r1 == 0, "FTD-0816's nine-square identity failed to reproduce"

    # An independent closed form.  With U_i = sin^2(x_i),
    #     p = (4/3) [ 3 sum U_i - 2 sum_{i<j} U_i U_j ].
    U = [sp.sin(xi) ** 2 for xi in x]
    closed = sp.Rational(4, 3) * (3 * sum(U)
                                  - 2 * sum(U[i] * U[j]
                                            for i, j in ((0, 1), (1, 2), (2, 0))))
    r2 = sp.simplify(sp.expand_trig(mL.subs(sub) - closed))
    assert r2 == 0, "the U-form closed expression failed"

    # The regrouping that makes the count transparent:
    #     p = (4/3) sum_i s_i^2 (1 + c_j^2 + c_k^2)     -> 3 + 6 = 9 squares,
    # a DIFFERENT nine from FTD-0816's (each edge pair is rotated by 45 deg),
    # so nine is not an artefact of one grouping.
    prod = sp.Rational(4, 3) * sum(
        sp.sin(x[i]) ** 2 * (1 + sp.cos(x[j]) ** 2 + sp.cos(x[k]) ** 2)
        for i, j, k in ((0, 1, 2), (1, 2, 0), (2, 0, 1)))
    r3 = sp.simplify(sp.expand_trig(mL.subs(sub) - prod))
    assert r3 == 0, "the product regrouping failed"
    return mL, q, x


# ---------------------------------------------------------------------------
# 2. The ambient space collapses to 19
# ---------------------------------------------------------------------------

def zero_set_reduction():
    """p vanishes exactly on {0, pi}^3, which kills the 8 s-free monomials.

    p = (4/3) sum_i s_i^2 (1 + c_j^2 + c_k^2) and the bracket is >= 1, so
    p = 0 iff every sin x_i = 0.  Squares are nonnegative, so every f_a must
    vanish at all 8 of those points.  A monomial with no s-factor takes the
    values of a character of {+-1}^3 there, and the 8 characters form an
    invertible 8x8 Hadamard matrix -- so vanishing at all 8 forces every
    s-free coefficient to zero.
    """
    zeros = list(itertools.product((0.0, np.pi), repeat=3))
    sfree = [m for m in MONOMIALS if "s" not in m]
    H = np.array([[monomial_value(m, z) for m in sfree] for z in zeros])
    assert H.shape == (8, 8)
    assert abs(np.linalg.det(H)) > 1e-9, "the s-free evaluation matrix is singular"
    keep = [i for i, m in enumerate(MONOMIALS) if "s" in m]
    return keep, np.linalg.matrix_rank(H)


def monomial_value(m, x):
    v = 1.0
    for g, xi in zip(m, x):
        v *= 1.0 if g == "1" else (np.cos(xi) if g == "c" else np.sin(xi))
    return v


# ---------------------------------------------------------------------------
# 3. Zero-set lower bound
# ---------------------------------------------------------------------------

def gradient_bound():
    """n >= rank of the Hessian of p at a zero.

    At a zero every f_a vanishes, so Hess p = 2 sum_a grad f_a grad f_a^T,
    whose rank is at most n.
    """
    x = sp.symbols("x1 x2 x3", real=True)
    p = sp.Rational(4, 3) * sum(
        sp.sin(x[i]) ** 2 * (1 + sp.cos(x[j]) ** 2 + sp.cos(x[k]) ** 2)
        for i, j, k in ((0, 1, 2), (1, 2, 0), (2, 0, 1)))
    ranks = set()
    for z in itertools.product((0, sp.pi), repeat=3):
        H = sp.Matrix(3, 3, lambda i, j: sp.diff(p, x[i], x[j]).subs(
            dict(zip(x, z))))
        ranks.add((H.rank(), sp.simplify(H[0, 0])))
    return ranks


# ---------------------------------------------------------------------------
# 4. The search
# ---------------------------------------------------------------------------

def build_grid():
    """5 points per axis resolve every frequency in [-2, 2] exactly.

    p has degree <= 2 per variable in x, and so does every f_a^2 for f_a in
    V, so agreement on this 125-point grid is agreement as functions -- the
    5-point DFT is invertible on frequencies -2..2.  No sampling error.
    """
    node = 2.0 * np.pi * np.arange(5) / 5.0
    pts = np.array(list(itertools.product(node, node, node)))
    Z = np.array([[monomial_value(m, x) for m in MONOMIALS] for x in pts])
    q = 2.0 * pts
    pv = (4.0 - (2.0 / 3.0) * np.cos(q).sum(axis=1)
          - (2.0 / 3.0) * sum(np.cos(q[:, i]) * np.cos(q[:, j])
                              for i, j in ((0, 1), (1, 2), (2, 0))))
    return pts, Z, pv


def try_rank(Zk, pv, n, rng, tries):
    """Burer-Monteiro: solve p = sum_a (Zk F[:,a])^2 in least squares.

    'trf', not 'lm': for n >= 7 the parameter count (19n) exceeds the 125
    residuals and lm refuses underdetermined systems.  The Jacobian is
    supplied analytically -- d r_p / d F_{nu b} = 2 (Zk F)_{pb} Zk_{p nu} --
    because a finite-difference Jacobian on 171 parameters both costs more
    and stalls a decade earlier than the true one.
    """
    d = Zk.shape[1]

    def resid(v):
        return ((Zk @ v.reshape(d, n)) ** 2).sum(axis=1) - pv

    def jac(v):
        G = Zk @ v.reshape(d, n)                      # (P, n)
        return (2.0 * G[:, None, :] * Zk[:, :, None]).reshape(len(pv), d * n)

    # CORRECTION 2026-08-09: the initial scale is now DISPERSED.  The
    # first version drew every restart from scale 0.7, which samples one
    # basin many times rather than many basins once, and reported the
    # minimum as five.  Dispersing the scale finds a numerical FOUR
    # candidate within tens of starts.  Fresh-point residuals are not an
    # exact or interval existence certificate.  Restart counts only bound what a search of
    # this kind can bound if the starts are actually independent.
    best, bestF = np.inf, None
    for _ in range(tries):
        sc = 0.2 + 4.0 * rng.random()
        F0 = rng.normal(scale=sc, size=d * n)
        res = least_squares(resid, F0, jac=jac, method="trf",
                            xtol=1e-15, ftol=1e-15, gtol=1e-15, max_nfev=20000)
        rms = float(np.sqrt(np.mean(res.fun ** 2)))
        if rms < best:
            best, bestF = rms, res.x.reshape(d, n)
        if best < 1e-13:
            break
    return best, bestF


def certify(F, keep, tol=1e-9):
    """Re-verify a found decomposition on fresh random momenta."""
    rng = np.random.default_rng(SEED + 7)
    pts = rng.uniform(0.0, 2.0 * np.pi, size=(4000, 3))
    Z = np.array([[monomial_value(MONOMIALS[i], x) for i in keep] for x in pts])
    q = 2.0 * pts
    pv = (4.0 - (2.0 / 3.0) * np.cos(q).sum(axis=1)
          - (2.0 / 3.0) * sum(np.cos(q[:, i]) * np.cos(q[:, j])
                              for i, j in ((0, 1), (1, 2), (2, 0))))
    return float(np.max(np.abs((Z @ F) ** 2 @ np.ones(F.shape[1]) - pv)))


def symmetry_orbits():
    """Sign characters of the 19 monomials, and the O_h orbit structure.

    The symmetry group is Z_2^3 semidirect S_3: x_i -> -x_i flips s_i, and
    S_3 permutes axes.  A monomial's sign character is which coordinates
    carry an s.  Orbits under S_3:

      (-,+,+)  one s   : s_i, s_i c_j, s_i c_k, s_i c_j c_k   3 x 4 = 12
      (-,-,+)  two s   : s_i s_j, s_i s_j c_k                 3 x 2 =  6
      (-,-,-)  three s : s_1 s_2 s_3                          1 x 1 =  1

    For an f_a lying in ONE character sector the sign group acts by +-1, so
    f_a^2 is sign-invariant and its orbit is just the S_3 orbit: size 3, or
    1 for the (-,-,-) sector, whose space is one-dimensional so the singlet
    cannot repeat.  Pure-sector covariant lengths are therefore
    3t + 3u + d with d in {0, 1}: 1, 3, 4, 6, 7, 9, 10, 12, ...

    ⚠ **That does not rule out a covariant 5.**  An f_a MIXING sectors --
    say s_1 + s_2 s_3 -- is not a sign eigenvector, and its orbit can have
    any size dividing |Z_2^3 : S_3| = 48.  Five is not itself a divisor of
    48, so no SINGLE orbit has size 5; but 5 = 2 + 3 = 1 + 4 = 1 + 2 + 2 are
    all arithmetically available.  The covariant search below is over
    pure-sector orbits only, so what it establishes is that a covariant
    seven EXISTS, not that seven is the covariant minimum.
    """
    sizes = {"one_s": 0, "two_s": 0, "three_s": 0}
    for m in MONOMIALS:
        k = m.count("s")
        if k == 1:
            sizes["one_s"] += 1
        elif k == 2:
            sizes["two_s"] += 1
        elif k == 3:
            sizes["three_s"] += 1
    assert sizes == {"one_s": 12, "two_s": 6, "three_s": 1}, sizes
    reachable = set()
    for t in range(5):            # t triplets from the one-s sector
        for u in range(5):        # u triplets from the two-s sector
            for d in (0, 1):      # the lone singlet
                n = 3 * t + 3 * u + d
                if 0 < n <= 12:
                    reachable.add(n)
    return sizes, sorted(reachable)


def exact_seven():
    """The covariant seven, solved exactly rather than fitted.

    The pure-sector ansatz has six free parameters -- a s_i + b s_i(c_j+c_k)
    + d s_i c_j c_k for the one-s triplet, e s_j s_k + f s_j s_k c_i for the
    two-s triplet, m s_1 s_2 s_3 for the singlet -- and sympy returns eight
    real solutions, all sign variants of

        -L18 = 4 sum_i sin^2 x_i cos^2 x_j cos^2 x_k
             + (16/3) sum_i sin^2 x_j sin^2 x_k cos^2 x_i
             + 4 sin^2 x_1 sin^2 x_2 sin^2 x_3 ,      x = q/2.

    Hand check with A = sin^2 x_1 etc.:  the first sum is
    4[S1 - 2 S2 + 3 ABC], the second (16/3)[S2 - 3 ABC], the third 4 ABC;
    the ABC terms cancel exactly (12 - 16 + 4 = 0) and what remains is
    4 S1 - (8/3) S2 = (4/3)[3 S1 - 2 S2] = -L18.  The singlet is not
    decoration: it is what cancels the triple product.
    """
    x = sp.symbols("x1 x2 x3", real=True)
    s = [sp.sin(v) for v in x]
    c = [sp.cos(v) for v in x]
    cyc = ((0, 1, 2), (1, 2, 0), (2, 0, 1))
    tot = (sum((2 * s[i] * c[j] * c[k]) ** 2 for i, j, k in cyc)
           + sum((4 * sp.sqrt(3) / 3 * s[j] * s[k] * c[i]) ** 2
                 for i, j, k in cyc)
           + (2 * s[0] * s[1] * s[2]) ** 2)
    q = [2 * v for v in x]
    p = (4 - sp.Rational(2, 3) * sum(sp.cos(qi) for qi in q)
         - sp.Rational(2, 3) * sum(sp.cos(q[i]) * sp.cos(q[j])
                                   for i, j in ((0, 1), (1, 2), (2, 0))))
    r = sp.simplify(sp.expand_trig(sp.expand(tot - p)))
    assert r == 0, f"the exact seven-square identity failed: {r}"
    return r


def gram_is_covariant(F, keep, tol=1e-8):
    """Is a found decomposition invariant under the cubic group?

    The Gram G = F F^T is the invariant of a decomposition.  Under an axis
    permutation or a sign flip the monomial basis permutes (with signs), so
    a covariant decomposition has P G P^T = G for every group element.
    """
    idx = {MONOMIALS[i]: r for r, i in enumerate(keep)}
    worst = 0.0
    G = F @ F.T
    for perm in itertools.permutations(range(3)):
        for flip in itertools.product((1, -1), repeat=3):
            P = np.zeros_like(G)
            for m, r in idx.items():
                mm = tuple(m[perm[t]] for t in range(3))
                sgn = 1.0
                for t in range(3):
                    if m[perm[t]] == "s" and flip[t] < 0:
                        sgn = -sgn
                P[idx[mm], r] = sgn
            worst = max(worst, float(np.abs(P @ G @ P.T - G).max()))
    return worst


def try_symmetric(pv, pts, n_triplets_1, n_triplets_2, use_singlet, rng, tries):
    """Search only cubic-covariant decompositions.

    Each triplet contributes three squares f_1, f_2, f_3 related by the axis
    permutation, so f_i is fixed by its stabiliser: symmetric in (j, k).
    """
    P = len(pv)
    T1 = []   # one-s triplet: s_i, s_i(c_j + c_k), s_i c_j c_k
    T2 = []   # two-s triplet: s_j s_k, s_j s_k c_i
    for i, j, k in ((0, 1, 2), (1, 2, 0), (2, 0, 1)):
        s, c = np.sin(pts), np.cos(pts)
        T1.append(np.stack([s[:, i],
                            s[:, i] * (c[:, j] + c[:, k]),
                            s[:, i] * c[:, j] * c[:, k]], axis=1))
        T2.append(np.stack([s[:, j] * s[:, k],
                            s[:, j] * s[:, k] * c[:, i]], axis=1))
    sing = (np.sin(pts[:, 0]) * np.sin(pts[:, 1]) * np.sin(pts[:, 2]))[:, None]

    npar = 3 * n_triplets_1 + 2 * n_triplets_2 + (1 if use_singlet else 0)
    if npar == 0:
        return np.inf

    def resid(v):
        o, tot = 0, np.zeros(P)
        for _ in range(n_triplets_1):
            a = v[o:o + 3]; o += 3
            for B in T1:
                tot += (B @ a) ** 2
        for _ in range(n_triplets_2):
            a = v[o:o + 2]; o += 2
            for B in T2:
                tot += (B @ a) ** 2
        if use_singlet:
            tot += (sing[:, 0] * v[o]) ** 2
        return tot - pv

    best = np.inf
    for _ in range(tries):
        r = least_squares(resid, rng.normal(scale=0.8, size=npar),
                          method="trf", xtol=1e-15, ftol=1e-15, gtol=1e-15,
                          max_nfev=20000)
        best = min(best, float(np.sqrt(np.mean(r.fun ** 2))))
        if best < 1e-13:
            break
    return best


def spinor_dim(n):
    """Smallest 2^k carrying n mutually anticommuting Hermitian structures."""
    k = 0
    while 2 * k + 1 < n:
        k += 1
    return 2 ** k


def main():
    print(__doc__.split("Reproduction:")[0].strip().split("\n")[0])
    print()

    symbolic_checks()
    print("  [verify] FTD-0816 nine-square identity           EXACT (sympy)")
    print("  [verify] independent U-form closed expression    EXACT (sympy)")
    print("  [verify] product regrouping, a DIFFERENT nine    EXACT (sympy)")

    keep, hrank = zero_set_reduction()
    print(f"  [verify] s-free evaluation matrix rank = {hrank} (exact 8)")
    print(f"  [verify] ambient collapses 27 -> {len(keep)} monomials")
    assert len(keep) == 19, "expected 19 monomials carrying an s factor"

    ranks = gradient_bound()
    assert len(ranks) == 1, "the 8 zeros should be equivalent"
    r, h00 = ranks.pop()
    print(f"  [verify] Hessian at every zero = {h00}*I, rank {r} "
          f"=> n >= {r}")

    pts, Z, pv = build_grid()
    Zk = Z[:, keep]
    print(f"  [verify] grid {Z.shape[0]} pts exactly resolves deg-2 symbols")

    sizes, reachable = symmetry_orbits()
    print(f"  [verify] sign-character orbits {sizes}")
    print(f"  [verify] pure-sector covariant lengths <= 12: {reachable}")
    assert 5 not in reachable and 4 in reachable
    print("  [verify] 5 does not divide 48, so no SINGLE orbit has size 5;")
    print("           5 = 2+3 = 1+4 stays open for mixed-sector orbits")

    seven = exact_seven()
    print(f"  [verify] exact covariant 7-square identity: residual {seven}"
          f"  (sympy, symbolic)")

    rng = np.random.default_rng(SEED)
    print()
    print("  UNRESTRICTED SEARCH (any f_a in V)")
    print("   n   params   starts   residual RMS      verdict")
    print("  --------------------------------------------------------------")
    found = {}
    # n = 4's basin is RARE: dispersed runs have hit it after 13, 52 and
    # 146 starts, and one 200-start run missed it entirely.  The budget
    # below makes a miss improbable rather than merely unlucky.
    for n in range(9, 2, -1):
        tries = 8 if n >= 7 else (3000 if n == 4 else
                                  (200 if n >= 5 else 1200))
        rms, F = try_rank(Zk, pv, n, rng, tries)
        if rms < 1e-12:
            found[n] = F
        print(f"  {n:2d}   {len(keep)*n:5d}   {tries:5d}    {rms:.3e}     "
              f"{'decomposition found' if rms < 1e-12 else 'none found'}")

    nmin = min(found) if found else None
    print()
    print("  CUBIC-COVARIANT SEARCH (whole orbits only)")
    print("   n   composition                    residual RMS      verdict")
    print("  --------------------------------------------------------------")
    sym_ok = []
    for n, (t1, t2, d) in ((3, (1, 0, 0)), (3, (0, 1, 0)), (4, (1, 0, 1)),
                           (4, (0, 1, 1)), (6, (2, 0, 0)), (6, (1, 1, 0)),
                           (7, (2, 0, 1)), (7, (1, 1, 1)), (9, (3, 0, 0)),
                           (9, (2, 1, 0))):
        rms = try_symmetric(pv, pts, t1, t2, bool(d), rng, 40)
        tag = f"{t1} one-s + {t2} two-s + {d} singlet"
        ok = rms < 1e-12
        if ok:
            sym_ok.append(n)
        print(f"  {n:2d}   {tag:28s}   {rms:.3e}     "
              f"{'found' if ok else 'none'}")
    nsym = min(sym_ok) if sym_ok else None

    print()
    if nmin is not None:
        err = certify(found[nmin], keep)
        print(f"  [verify] n = {nmin} re-checked on 4000 fresh momenta: "
              f"max |residual| = {err:.3e}")
        assert err < 1e-9, "the numerical candidate failed its fresh-point recheck"
        G = found[nmin] @ found[nmin].T
        ev = np.linalg.eigvalsh(G)
        print(f"  [verify] Gram is PSD, rank {int((ev > 1e-10).sum())}, "
              f"min eig {ev.min():+.2e}")
        dev = gram_is_covariant(found[nmin], keep)
        print(f"  [verify] cubic covariance of the n = {nmin} Gram: "
              f"max |P G P^T - G| = {dev:.3e} "
              f"({'covariant' if dev < 1e-8 else 'NOT covariant'})")
    print()
    print(f"  RESULT: best numerical unrestricted candidate n = {nmin}"
          f"  -> candidate spinor dimension "
          f"{spinor_dim(nmin)}")
    print(f"          covariant EXISTENCE at n = {nsym} (exact) -> spinor "
          f"dimension {spinor_dim(nsym)}")
    print(f"          FTD-0816's nine            -> spinor dimension "
          f"{spinor_dim(9)}")
    print()
    print("  The covariant number is an upper bound: the search covered")
    print("  pure-sector orbits only, so a covariant 5 or 6 built from")
    print("  mixed-sector orbits is not excluded.")


if __name__ == "__main__":
    main()
