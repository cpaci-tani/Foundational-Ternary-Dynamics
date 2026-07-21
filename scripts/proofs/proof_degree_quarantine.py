"""
Degree-quarantine lemma — verification (LEMMA_DEGREE_QUARANTINE.md).

T1 (twisted-linear closure): half-period BZ translations k -> k + pi*v act on
    the axis cosines by per-axis sign flips, hence preserve each cosine
    monomial's multidegree. The span of ALL such twists of the engine symbol
    family {1, e1, e2} is exactly the 7-dimensional space
    span{1, c1, c2, c3, c1c2, c1c3, c2c3}; the BCC monomial e3 = c1*c2*c3 is
    NOT in it.  (Verified constructively: build the twist-orbit matrix over
    the 8-monomial basis and check rank / e3-coefficient.)

T2 (spectral-functional blindness): every function F(L18) of the dynamical
    operator has symbol F(sigma18(k)), sigma18 = 1 - e1/6 - e2/6. EXACT
    witness that e3 is not a function of sigma18: the two cosine triples
        P  = (1/2, -1/2, 1/4)   ->  e1 + e2 = 0  ->  sigma18 = 1, e3 = -1/16
        Q  = (0, 0, 0)          ->  e1 + e2 = 0  ->  sigma18 = 1, e3 = 0
    (cos is surjective onto [-1,1], so both are realized by real k-points).
    Hence no F(L18)-type object measures or generates the e3 coordinate.

T3 is an engine-inventory fact (see the lemma doc), not verified here.

Run: python scripts/proofs/proof_degree_quarantine.py   (exit 0 = all pass)
"""
import itertools
import sys

import numpy as np

# Monomial basis of multilinear cosine polynomials: subsets of {1,2,3}
BASIS = [(), (1,), (2,), (3,), (1, 2), (1, 3), (2, 3), (1, 2, 3)]
IDX = {S: i for i, S in enumerate(BASIS)}


def e1_vec():
    v = np.zeros(8)
    for S in [(1,), (2,), (3,)]:
        v[IDX[S]] = 1.0
    return v


def e2_vec():
    v = np.zeros(8)
    for S in [(1, 2), (1, 3), (2, 3)]:
        v[IDX[S]] = 1.0
    return v


def one_vec():
    v = np.zeros(8)
    v[IDX[()]] = 1.0
    return v


def twist(vec, signs):
    """Apply c_i -> signs[i]*c_i to a monomial-basis vector."""
    out = np.zeros(8)
    for S in BASIS:
        s = 1.0
        for ax in S:
            s *= signs[ax - 1]
        out[IDX[S]] += s * vec[IDX[S]]
    return out


def main():
    ok = True

    # ---- T1: build the full twist orbit of {1, e1, e2} ----
    orbit = []
    for signs in itertools.product([1, -1], repeat=3):
        for base in (one_vec(), e1_vec(), e2_vec()):
            orbit.append(twist(base, signs))
    M = np.array(orbit)
    rank = np.linalg.matrix_rank(M, tol=1e-12)
    # e3-coefficient column must be identically zero across the orbit
    e3_col = M[:, IDX[(1, 2, 3)]]
    t1a = rank == 7
    t1b = np.all(e3_col == 0.0)
    print(f"T1: twist-orbit rank = {rank} (expect 7, the deg<=2 space)  "
          f"{'PASS' if t1a else 'FAIL'}")
    print(f"T1: e3-component of every orbit element = 0                "
          f"{'PASS' if t1b else 'FAIL'}")
    # and e3 itself is independent: adding it raises the rank to 8
    e3v = np.zeros(8)
    e3v[IDX[(1, 2, 3)]] = 1.0
    rank8 = np.linalg.matrix_rank(np.vstack([M, e3v]), tol=1e-12)
    t1c = rank8 == 8
    print(f"T1: rank with e3 adjoined = {rank8} (expect 8)             "
          f"{'PASS' if t1c else 'FAIL'}")
    ok &= t1a and t1b and t1c

    # ---- T2: exact witness ----
    def sigma18(c):
        e1 = sum(c)
        e2 = c[0] * c[1] + c[0] * c[2] + c[1] * c[2]
        return 1.0 - e1 / 6.0 - e2 / 6.0

    def e3(c):
        return c[0] * c[1] * c[2]

    P = (0.5, -0.5, 0.25)
    Q = (0.0, 0.0, 0.0)
    sP, sQ = sigma18(P), sigma18(Q)
    t2a = abs(sP - sQ) < 1e-15
    t2b = abs(e3(P) - e3(Q)) > 1e-3
    print(f"T2: sigma18(P) = {sP:.15f}, sigma18(Q) = {sQ:.15f}         "
          f"{'PASS' if t2a else 'FAIL'}")
    print(f"T2: e3(P) = {e3(P):+.6f} vs e3(Q) = {e3(Q):+.6f} (differ)   "
          f"{'PASS' if t2b else 'FAIL'}")
    # realizability: k-points with these cosines exist (|c| <= 1 componentwise)
    t2c = all(abs(x) <= 1.0 for x in P + Q)
    print(f"T2: witness cosines realizable (|c|<=1)                    "
          f"{'PASS' if t2c else 'FAIL'}")
    ok &= t2a and t2b and t2c

    print("\nALL PASS" if ok else "\nFAILURES PRESENT")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
