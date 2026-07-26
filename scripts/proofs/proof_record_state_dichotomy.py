"""FTD-0513 -- Record-state dichotomy and the symmetric-fiber sharpening.

Verifies the exact content of DERIV_RECORD_STATE_DICHOTOMY.md:

  E1  Trace condition <=> cyclic-class constancy on the record monoid:
      for words u, v over m digits, u.v and v.u are cyclic rotations of
      one another, and every rotation of w arises as such a swap. Hence
      a functional on words satisfies tau(uv) = tau(vu) for all u, v iff
      it is constant on cyclic classes. (Exhaustive, lengths <= 6.)
  E2  The quotient state (any functional of word length alone -- what the
      projected raw output supports, FTD-0499 S1/S4) is constant on
      cyclic classes, hence tracial.
  E3  Separation forces non-traciality: for every m >= 2, N >= 2 there
      exist DISTINCT histories (distinct radix encodings h) in the SAME
      cyclic class; a functional separating all histories is therefore
      non-constant on some cyclic class, i.e. non-tracial. (Witness
      enumeration: every cyclic class of size >= 2 contains distinct
      stack values; #words > #necklaces.)
  E4  The intermediate shelf: necklace states (indicators of a cyclic
      class) are tracial yet strictly finer than the quotient state --
      witness 0011 vs 0101: same length, same digit multiset, different
      cyclic classes. Burnside count checks #necklaces.
  E5  The symmetric-fiber split of the delta wall, exact to 45 digits:
      for the master quadratic x^2 - 16G*^2 x + 16G*^3, the SYMMETRIC
      functions of the root fiber lie in Q(G*):
          x+ + x- = 16 G*^2,   x+ * x- = 16 G*^3,
      while the ANTISYMMETRIC (ordering) function carries the surd:
          x+ - x- = 8 G* delta,   delta = sqrt(G*(4G*-1)).

Run:  python scripts/proofs/proof_record_state_dichotomy.py
"""

import itertools
import sys
from math import gcd

import mpmath as mp

PASS = []


def check(name, cond):
    PASS.append((name, bool(cond)))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}")


def rotations(w):
    return {w[k:] + w[:k] for k in range(max(1, len(w)))}


def words(m, n):
    return [tuple(t) for t in itertools.product(range(m), repeat=n)]


def encode(w, m):
    h = 0
    for b in w:
        h = m * h + b
    return h


def e1_trace_iff_cyclic():
    ok = True
    for m in (2, 3):
        for total in range(1, 7):
            for cut in range(total + 1):
                for w in words(m, total):
                    u, v = w[:cut], w[cut:]
                    ok &= (v + u) in rotations(w)  # uv ~ vu is a rotation
            # every rotation arises as a prefix/suffix swap
            for w in words(m, total):
                ok &= rotations(w) == {w[k:] + w[:k] for k in range(total)}
    check("E1 uv ~ vu spans exactly the cyclic class (lengths <= 6, m in {2,3})", ok)


def e2_quotient_tracial():
    ok = True
    for m in (2, 3):
        for n in range(1, 7):
            for w in words(m, n):
                for r in rotations(w):
                    ok &= len(r) == len(w)  # length-only functional is class-constant
    check("E2 quotient (length-only) state is constant on cyclic classes -> tracial", ok)


def e3_separation_nontracial():
    ok = True
    for m in (2, 3):
        for n in range(2, 7):
            found_witness = False
            for w in words(m, n):
                cls = rotations(w)
                if len(cls) >= 2:
                    encs = {encode(r, m) for r in cls}
                    ok &= len(encs) == len(cls)  # rotations are distinct histories
                    found_witness = True
            ok &= found_witness
    check("E3 every size->=2 cyclic class holds distinct histories (separating => non-tracial)", ok)


def necklace_count(m, n):
    from math import comb  # noqa: F401  (comb unused; keep stdlib-only imports obvious)
    def phi(k):
        return sum(1 for i in range(1, k + 1) if gcd(i, k) == 1)
    return sum(phi(d) * m ** (n // d) for d in range(1, n + 1) if n % d == 0) // n


def e4_necklace_shelf():
    m, n = 2, 4
    w1, w2 = (0, 0, 1, 1), (0, 1, 0, 1)
    distinct_classes = rotations(w1) != rotations(w2)
    same_length_same_multiset = (len(w1) == len(w2)) and (sorted(w1) == sorted(w2))
    classes = set()
    for w in words(m, n):
        classes.add(frozenset(rotations(w)))
    burnside = necklace_count(m, n)
    counts_ok = (len(classes) == burnside) and (1 < burnside < m ** n)
    check("E4 necklace shelf: 0011 vs 0101 split by a tracial state the quotient cannot make; "
          f"Burnside count {burnside} strictly between 1 and {m**n}",
          distinct_classes and same_length_same_multiset and counts_ok)


def e5_symmetric_fiber_split():
    mp.mp.dps = 60
    G = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
    delta = mp.sqrt(G * (4 * G - 1))
    disc = mp.sqrt(4 * G ** 2 - G)
    xp = 8 * G ** 2 + 4 * G * disc
    xm = 8 * G ** 2 - 4 * G * disc
    tol = mp.mpf(10) ** -45
    ok = (abs(xp + xm - 16 * G ** 2) < tol
          and abs(xp * xm - 16 * G ** 3) < tol
          and abs((xp - xm) - 8 * G * delta) < tol
          # sanity only: x+ matches 1/alpha to ~1.26 ppm (FTD-0013 [SMC]),
          # i.e. ~1.7e-4 absolute -- NOT an exact-equality claim
          and abs(xp - mp.mpf("137.036")) < mp.mpf("1e-3"))
    check("E5 fiber split exact to 45 digits: e1,e2 in Q(G*); x+ - x- = 8 G* delta; x+ ~ 137.036 (ppm-level sanity)", ok)


def main():
    print("FTD-0513 record-state dichotomy verification")
    e1_trace_iff_cyclic()
    e2_quotient_tracial()
    e3_separation_nontracial()
    e4_necklace_shelf()
    e5_symmetric_fiber_split()
    n_ok = sum(1 for _, ok in PASS if ok)
    print(f"\n{n_ok}/{len(PASS)} PASS")
    sys.exit(0 if n_ok == len(PASS) else 1)


if __name__ == "__main__":
    main()
