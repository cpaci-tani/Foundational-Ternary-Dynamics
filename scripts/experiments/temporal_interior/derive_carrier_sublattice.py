"""Which sublattice does the matter carrier actually need?

FTD-0816 4c settled the COUNT (seven covariant, five unrestricted).  This
settles the ONTOLOGY, which is the load-bearing half: a square's argument
fixes the displacements its hopping operator uses, and displacements are
sites.  So the question "what does a common cone cost the postulates" is
the question "which half-offsets appear".

With x = q/2, a monomial prod_i g_i(x_i) has displacement component m
equal to {0} when g_m = 1 and {+-1/2} when g_m is cos or sin, because
cos(q/2) and sin(q/2) are both combinations of exp(+-i q/2).  So the
support shell is read off the pattern of constant factors:

    s_i          -> (+-1/2, 0, 0)        axis half-offset  (face centre)
    s_i c_j      -> (+-1/2, +-1/2, 0)    edge midpoint
    s_i c_j c_k  -> (+-1/2, +-1/2, +-1/2) BODY CENTRE
    s_i s_j c_k  -> (+-1/2, +-1/2, +-1/2) BODY CENTRE
    s_1 s_2 s_3  -> (+-1/2, +-1/2, +-1/2) BODY CENTRE

The subspace with NO constant factor -- every g_i in {cos, sin}, at least
one sin -- is the BCC sector.  It has 2^3 - 1 = 7 elements.  Seven.  The
covariant seven-square decomposition is one square per BCC basis monomial,
which is why it is covariant and why it is exactly seven.

WHAT IS AT STAKE.  SC sites plus body centres IS the BCC lattice.  If the
carrier needs only body centres, the ontological demand is not a finer
cubic lattice (which would halve a_phys and move every dimensional
prediction with it) but the OTHER sublattice of a BCC lattice -- and FTD
already reaches BCC independently, via the Watson identity W_3 = G*^2/2pi
and the SU(3) triple cosine product.  If instead the shortest
decompositions must reach to face and edge offsets, the demand is the
finer lattice and the price is much higher.

Reproduction:
    python scripts/experiments/temporal_interior/derive_carrier_sublattice.py
"""

import itertools

import numpy as np
import sympy as sp
from scipy.optimize import least_squares

SEED = 20260808
GS = ("1", "c", "s")
MONOMIALS = [m for m in itertools.product(GS, repeat=3)]


def shell(m):
    """Which displacement shell a monomial's hopping operator occupies."""
    nz = sum(1 for g in m if g != "1")
    return {0: "(0,0,0)", 1: "(1/2,0,0) face", 2: "(1/2,1/2,0) edge",
            3: "(1/2,1/2,1/2) BODY"}[nz]


def monomial_value(m, x):
    v = 1.0
    for g, xi in zip(m, x):
        v *= 1.0 if g == "1" else (np.cos(xi) if g == "c" else np.sin(xi))
    return v


def displacement_support(coeffs, basis, tol=1e-9):
    """Displacement set of a function, read off its DFT.

    A function of x = q/2 built from {1, cos x_i, sin x_i} has frequencies
    k in {-1,0,1}^3, and a frequency k IS a displacement d = k/2 in lattice
    units.  Sampling on a 5^3 grid resolves k in [-2,2] without aliasing,
    so the DFT support is the displacement support exactly.

    Read off rather than reasoned out: the point is to check the
    combinatorial claim in the docstring, not to restate it.
    """
    N = 5
    node = 2.0 * np.pi * np.arange(N) / N
    g = np.zeros((N, N, N))
    for a, b, cc in itertools.product(range(N), repeat=3):
        x = (node[a], node[b], node[cc])
        g[a, b, cc] = sum(w * monomial_value(m, x)
                          for w, m in zip(coeffs, basis) if w != 0.0)
    F = np.fft.fftn(g) / N ** 3
    ks = np.fft.fftfreq(N, d=1.0 / N).astype(int)
    disp = set()
    for a, b, cc in itertools.product(range(N), repeat=3):
        if abs(F[a, b, cc]) > tol:
            disp.add((ks[a] / 2.0, ks[b] / 2.0, ks[cc] / 2.0))
    return disp


def shell_of(disp):
    """Classify a displacement set by how many components are nonzero."""
    tags = set()
    for d in disp:
        nz = sum(1 for c in d if abs(c) > 1e-12)
        tags.add({0: "(0,0,0)", 1: "face", 2: "edge", 3: "BODY"}[nz])
    return tags


def spinor_dim(n):
    """Smallest 2^k carrying n mutually anticommuting Hermitian structures."""
    k = 0
    while 2 * k + 1 < n:
        k += 1
    return 2 ** k


def build_grid():
    node = 2.0 * np.pi * np.arange(5) / 5.0
    pts = np.array(list(itertools.product(node, node, node)))
    Z = np.array([[monomial_value(m, x) for m in MONOMIALS] for x in pts])
    q = 2.0 * pts
    pv = (4.0 - (2.0 / 3.0) * np.cos(q).sum(axis=1)
          - (2.0 / 3.0) * sum(np.cos(q[:, i]) * np.cos(q[:, j])
                              for i, j in ((0, 1), (1, 2), (2, 0))))
    return pts, Z, pv


def try_rank(Zk, pv, n, rng, tries):
    d = Zk.shape[1]

    def resid(v):
        return ((Zk @ v.reshape(d, n)) ** 2).sum(axis=1) - pv

    def jac(v):
        G = Zk @ v.reshape(d, n)
        return (2.0 * G[:, None, :] * Zk[:, :, None]).reshape(len(pv), d * n)

    best, bestF = np.inf, None
    for _ in range(tries):
        # dispersed initial scale -- see the CORRECTION note in main()
        sc = 0.2 + 4.0 * rng.random()
        r = least_squares(resid, rng.normal(scale=sc, size=d * n), jac=jac,
                          method="trf", xtol=1e-15, ftol=1e-15, gtol=1e-15,
                          max_nfev=20000)
        rms = float(np.sqrt(np.mean(r.fun ** 2)))
        if rms < best:
            best, bestF = rms, r.x.reshape(d, n)
        if best < 1e-13:
            break
    return best, bestF


def main():
    print("Which sublattice does the matter carrier need?")
    print()

    bcc = [i for i, m in enumerate(MONOMIALS)
           if "1" not in m and "s" in m]
    face = [i for i, m in enumerate(MONOMIALS)
            if sum(1 for g in m if g != "1") == 1 and "s" in m]
    edge = [i for i, m in enumerate(MONOMIALS)
            if sum(1 for g in m if g != "1") == 2 and "s" in m]
    print(f"  [verify] BCC sector (no constant factor)  = {len(bcc)} monomials")
    print(f"  [verify] face shell (+-1/2,0,0)           = {len(face)}")
    print(f"  [verify] edge shell (+-1/2,+-1/2,0)       = {len(edge)}")
    # 7 = {c,s}^3 minus (c,c,c);  3 = one s and two constants;
    # 9 = 3 axis pairs x {(s,c), (c,s), (s,s)}.
    assert (len(bcc), len(face), len(edge)) == (7, 3, 9)
    assert len(bcc) + len(face) + len(edge) == 19

    # Displacement check, read off the DFT, on one representative of each
    # shell -- including the two that are NOT body-centred, as controls.
    for m, want in ((("s", "c", "c"), "BODY"), (("s", "s", "c"), "BODY"),
                    (("s", "s", "s"), "BODY"), (("s", "1", "1"), "face"),
                    (("s", "c", "1"), "edge")):
        D = displacement_support([1.0], [m])
        tags = shell_of(D)
        print(f"  [verify] {''.join(m):5s} -> {len(D):2d} displacements, "
              f"shell {sorted(tags)}")
        assert tags == {want}, f"{m}: expected {want}, got {tags}"

    # The covariant seven IS the diagonal decomposition of the BCC sector.
    x = sp.symbols("x1 x2 x3", real=True)
    s, c = [sp.sin(v) for v in x], [sp.cos(v) for v in x]
    cyc = ((0, 1, 2), (1, 2, 0), (2, 0, 1))
    seven = (sum((2 * s[i] * c[j] * c[k]) ** 2 for i, j, k in cyc)
             + sum((4 * sp.sqrt(3) / 3 * s[j] * s[k] * c[i]) ** 2
                   for i, j, k in cyc)
             + (2 * s[0] * s[1] * s[2]) ** 2)
    q = [2 * v for v in x]
    p = (4 - sp.Rational(2, 3) * sum(sp.cos(qi) for qi in q)
         - sp.Rational(2, 3) * sum(sp.cos(q[i]) * sp.cos(q[j])
                                   for i, j in ((0, 1), (1, 2), (2, 0))))
    assert sp.simplify(sp.expand_trig(sp.expand(seven - p))) == 0
    print("  [verify] the covariant seven = one square per BCC basis "
          "monomial, weights (4,4,4,16/3,16/3,16/3,4)   EXACT")

    # The BCC sector is the structure factor's complement.
    #
    # The 8 half-argument triple products prod_i g_i(q_i/2), g in {cos, sin},
    # satisfy sum of squares = prod_i (c_i^2 + s_i^2) = 1 identically.  One
    # of the 8 is the pure cosine S = prod cos(q_i/2) -- which is exactly the
    # BCC nearest-neighbour structure factor, the 8 neighbours sitting at
    # (+-1/2, +-1/2, +-1/2).  The other 7 ARE the carrier sector.  So the
    # equal-weight sum of the carrier squares is 1 - S^2 = (1-S)(1+S), and
    # -L_BCC = 8(1 - S).
    S = c[0] * c[1] * c[2]
    equal = sum(sp.prod([(s if t[i] else c)[i] for i in range(3)]) ** 2
                for t in itertools.product((0, 1), repeat=3) if any(t))
    assert sp.simplify(sp.expand_trig(sp.expand(equal - (1 - S ** 2)))) == 0
    print("  [verify] the 8 half-argument triple products square-sum to 1;")
    print("           the excluded one IS the BCC structure factor")
    print("           S = prod cos(q_i/2), so the 7 carrier squares sum to")
    print("           1 - S^2 = (1-S)(1+S), and -L_BCC = 8(1-S).   EXACT")

    pts, Z, pv = build_grid()
    rng = np.random.default_rng(SEED)

    print()
    print("  MINIMUM RANK, RESTRICTED TO THE BCC SECTOR (body centres only)")
    print("   n   residual RMS      verdict")
    print("  ---------------------------------------------")
    bcc_min = None
    for n in range(7, 2, -1):
        rms, _ = try_rank(Z[:, bcc], pv, n, rng, 300)
        ok = rms < 1e-12
        if ok:
            bcc_min = n
        print(f"  {n:2d}   {rms:.3e}     "
              f"{'found' if ok else 'none found'}")

    print()
    print("  DOES THE UNRESTRICTED FIVE STAY INSIDE THE BCC SECTOR?")
    keep = [i for i, m in enumerate(MONOMIALS) if "s" in m]
    rms5, F5 = try_rank(Z[:, keep], pv, 5, rng, 120)
    assert rms5 < 1e-12, "the unrestricted five was not reproduced"
    w = np.linalg.norm(F5, axis=1) ** 2          # weight per monomial
    inbcc = sum(w[r] for r, i in enumerate(keep) if i in bcc)
    outside = sum(w[r] for r, i in enumerate(keep) if i not in bcc)
    print(f"  [verify] five reproduced, residual {rms5:.3e}")
    print(f"  [verify] weight on BCC monomials     = {inbcc:.4f}")
    print(f"  [verify] weight on face/edge shells  = {outside:.4f}"
          f"   ({100*outside/(inbcc+outside):.1f}% of the total)")

    print()
    # CORRECTION 2026-08-09.  The unrestricted minimum is FOUR, found once
    # the search's initial scale is dispersed (the earlier fixed-scale
    # search sampled one basin 400 times and reported five).  The BCC
    # restriction now cuts finely: confined to body centres the search
    # reaches five and stalls there over 2000 dispersed starts, while the
    # four uses the face and edge shells.  So "body centres suffice" holds
    # at length five; the shortest carrier buys one square by leaving
    # them.  Either way no finer cubic lattice is wanted -- every shell in
    # play is a half-offset of the existing one.  The stall at four is
    # search evidence of exactly the kind that mis-reported the minimum
    # before, and is asserted at that confidence and no higher; the only
    # proof here is the Hessian floor n >= 3.
    assert bcc_min == 5, "expected the BCC-restricted search to stall at five"
    print(f"  RESULT: BCC-only search minimum = {bcc_min}; unrestricted = 4")
    print( "          (certified elsewhere; see derive_sos_rank_minimal.py).")
    print( "          Body centres suffice at length five; the four buys its")
    print( "          shortness by leaving them for the face/edge shells.")
    print( "          No finer cubic lattice either way: every shell in play")
    print( "          is a half-offset of the existing lattice, so a_phys")
    print( "          does not move.")
    print()
    print( "          Sign-character orbits INSIDE the BCC sector are 3 (scc),")
    print( "          3 (ssc) and 1 (sss), so pure-sector covariant lengths")
    print( "          there are 3, 4, 6, 7 -- the exact covariant seven; the")
    print( "          unrestricted four is non-covariant.")
    print()
    print("  THE MASS LADDER.  H = sum_mu Gamma^mu phi_mu + Gamma^(n+1) M has")
    print("  H^2 = sum phi^2 + M^2 only if the mass structure anticommutes")
    print("  with every kinetic one, so a mass costs one MORE structure.")
    print("  Saturation ALTERNATES: 3, 5, 7 squares saturate their dimension;")
    print("  4, 6, 8 leave one structure spare.")
    print()
    print("   carrier            structures   spinor dim   spare")
    print("  ------------------------------------------------------")
    for lab, n in (("four, massless", 4), ("four + mass", 5),
                   ("seven (covariant)", 7), ("seven + mass", 8)):
        d = spinor_dim(n)
        cap = 2 * int(np.log2(d)) + 1
        print(f"  {lab:18s} {n:6d}       {d:6d}      {cap - n:4d}")
    assert spinor_dim(4) == 4 and spinor_dim(5) == 4
    assert spinor_dim(7) == 8 and spinor_dim(8) == 16
    print()
    print("  So the minimal carrier ADMITS A MASS at ordinary Dirac dimension")
    print("  -- four kinetic structures plus the mass exactly fill the five")
    print("  that dimension 4 carries -- while cubic covariance with a mass")
    print("  forces dimension 16.  Covariance, not mass, is what multiplies")
    print("  the spinor.  (An earlier version, built on the mis-reported")
    print("  five, concluded the carrier was necessarily massless; that")
    print("  prediction is withdrawn.)")


if __name__ == "__main__":
    main()
