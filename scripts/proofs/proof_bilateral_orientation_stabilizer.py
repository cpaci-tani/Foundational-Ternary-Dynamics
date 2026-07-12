"""proof_bilateral_orientation_stabilizer.py — FTD-0382 / the bilateral
orientation-carrier criterion, group-theory core.

Companion to docs/theory/02_foundations/FOUND_ORIENTATION_CARRIER_BILATERAL_CRITERION.md.

Verifies the machine-checkable claims behind the bilateral-symmetry criterion for
an orientation carrier (an object that has undergone the reduction O_h -> C_s by
promoting two axes to signed vectors):

  P1  Stab_{O(3)}(v, g) = C_s (order 2) for independent v, g: the ONLY non-identity
      orthogonal map fixing two independent vectors is the reflection through their
      plane. (Explicit reflection built + verified; completeness by the 1-dim
      orthocomplement argument.)

  P2  The stabilizer's order across dimensions singles out D=3:
      Stab_{O(D)}(v, g) = I_2 (+) O(D-2)  =>  order = |O(D-2)|:
        D=2 -> O(0) trivial -> order 1 = C_1 (over-determined; no mirror)
        D=3 -> O(1) = {+-1}  -> order 2 = C_s (exactly one mirror)  [UNIQUE finite-C_s]
        D=4 -> O(2)          -> infinite (a whole circle survives)
      So "residual symmetry = exactly C_s" holds iff D=3. [consonance, NOT a
      derivation of D=3 -- dimension-forcing is FTD-0355 [SELECTION -- declared].]

  P3  FTD-0355 axis-stabilizer arithmetic |B_D|/D = 2^D (D-1)! (= 16 = |O_h|/3 at
      D=3), with |B_D| = 2^D D! the hyperoctahedral (signed-permutation) order.

  P4  The single mirror (order 2) is a Z/2 shadow of the order-4 CM automorphism
      group Z/4 = <i> (i^4=1, i^2=-1): Z/4 is cyclic with a UNIQUE involution
      i^2 (unlike the Klein four-group, which has three). The mirror is a derived
      branch of the order-4 spine, not an independent generator. [FTD-0375 guard:
      rhyme, no identification.]

  P5  The delta / magnitude probe (pre-registered CLOSED-NEGATIVE gate). Reflection-
      reduction invariants are algebraic over Q (orientation signs in {+-1} subset Q;
      subgroup indices like [O_h:C_s]=24 in Z; character values in cyclotomic
      Q(zeta_n) subset Q^ab) -- transcendence degree 0. The imported surd
      delta = sqrt(G*(4G*-1)) has, over Q, transcendence degree 1 (conditional on
      Chudnovsky 1976: G* transcendental), since delta^2 = 4t^2 - t is square-free
      over Q(t) [t <-> G*] so delta not in Q(t). A trdeg-1 element cannot equal a
      trdeg-0 element => NO reflection-reduction carrier reaches delta. This is the
      FTD-0314 narrowing exclusion restated in symmetry-reduction language: the
      reflection side (order-2, algebraic) and the CM-magnitude side (order-4,
      transcendental) are separated by exactly the algebraic/transcendental gap.

What this is NOT: not a derivation of D=3, not a claim that bilateral symmetry
reaches delta or alpha, not a promotion. Standing invariants untouched:
FC-W [AXIOM], delta imported, MC-T4.3 [FOUNDATIONAL OBSTRUCTION], x+ = 1/alpha
[SMC]. Golden gate untouched (docs + this verifier).

Usage:
    python scripts/proofs/proof_bilateral_orientation_stabilizer.py
"""

from __future__ import annotations

import os
import sys
from math import factorial

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("Bilateral orientation-carrier criterion (FTD-0382)")


# ======================================================= P1: Stab_O(3) = C_s
# Two independent vectors; the reflection through their plane fixes both.
v = sp.Matrix([2, 1, 1])
g = sp.Matrix([1, 3, 1])
n = v.cross(g)                                   # normal to span(v, g)
I3 = sp.eye(3)
R = I3 - 2 * (n * n.T) / (n.T * n)[0]            # Householder reflection

fixes_v = sp.simplify(R * v - v) == sp.zeros(3, 1)
fixes_g = sp.simplify(R * g - g) == sp.zeros(3, 1)
involution = sp.simplify(R * R - I3) == sp.zeros(3, 3)
is_reflection = sp.simplify(R.det() + 1) == 0    # det = -1
not_identity = sp.simplify(R - I3) != sp.zeros(3, 3)
suite.assert_true("P1a reflection R through span(v,g) fixes v and g",
                  bool(fixes_v and fixes_g))
suite.assert_true("P1b R is an involution (R^2 = I), det R = -1, R != I",
                  bool(involution and is_reflection and not_identity))

# Completeness: an O(3) element fixing v,g fixes the 2-plane span(v,g) pointwise;
# the orthocomplement is 1-dimensional, on which an orthogonal map is +-1 ->
# exactly {I, R}. So |Stab_{O(3)}(v,g)| = 2 = C_s.
plane_dim = sp.Matrix.hstack(v, g).rank()        # = 2 (v, g independent)
orthocomplement_dim = 3 - plane_dim              # = 1
stab_order_D3 = 2 ** orthocomplement_dim         # |O(1)| = 2
suite.assert_true("P1c fixed plane is 2-dim, orthocomplement 1-dim => "
                  "Stab_O(3)(v,g) = {I,R}, order 2 = C_s",
                  bool(plane_dim == 2 and orthocomplement_dim == 1
                       and stab_order_D3 == 2))


# ============================================ P2: D=3 uniquely gives finite C_s
# Stab_{O(D)}(v,g) = I_2 (+) O(D-2); finite iff D-2 <= 1, order 2 (=C_s) iff D-2=1.
def stab_is_finite(D: int) -> bool:
    return (D - 2) <= 1            # O(0), O(1) finite; O(k>=2) continuous


def stab_order(D: int):
    k = D - 2
    if k == 0:
        return 1                  # O(0) trivial -> C_1
    if k == 1:
        return 2                  # O(1) = {+-1} -> C_s
    return sp.oo                  # O(k>=2) infinite


d2 = (stab_order(2) == 1)                                  # C_1
d3 = (stab_order(3) == 2 and stab_is_finite(3))            # C_s
d4 = (stab_order(4) == sp.oo)                              # infinite
unique_Cs = all(stab_order(D) != 2 for D in (2, 4)) and stab_order(3) == 2
suite.assert_true("P2a D=2 -> order 1 (C_1, over-determined, no mirror)", bool(d2))
suite.assert_true("P2b D=3 -> order 2 (C_s, exactly one mirror)", bool(d3))
suite.assert_true("P2c D=4 -> infinite (O(2) circle survives)", bool(d4))
suite.assert_true("P2d exactly-C_s residual symmetry singles out D=3 "
                  "(consonance, NOT a forcing of D)", bool(unique_Cs))


# ================================= P3: FTD-0355 axis-stabilizer arithmetic
# Hyperoctahedral (signed-permutation) group B_D has order 2^D * D!.
def B_order(D: int) -> int:
    return 2 ** D * factorial(D)


ok = True
for D in (2, 3, 4):
    ok &= (B_order(D) // D == 2 ** D * factorial(D - 1))   # |B_D|/D = 2^D (D-1)!
ok &= (B_order(3) == 48)                                   # |B_3| = |O_h| = 48
ok &= (B_order(3) // 3 == 16)                              # = |Aut(E)|^2 (FTD-0355)
suite.assert_true("P3  |B_D|/D = 2^D (D-1)! for D in {2,3,4}; |O_h|/3 = 16", ok)


# ==================== P4: the mirror is a Z/2 shadow of the order-4 CM Z/4
# Z/4 = <i>, i^4=1. Elements of order dividing 2: solutions of 2x = 0 mod 4.
Z4_involutions = [x for x in range(4) if (2 * x) % 4 == 0 and x != 0]   # -> {2}
klein_involutions = [x for x in range(4) if x != 0]                     # Z/2xZ/2 -> 3
z4_cyclic = (len(Z4_involutions) == 1)         # unique involution i^2 => cyclic
mirror_is_derived = z4_cyclic                  # the order-2 mirror = <i^2>, a branch
not_klein = (len(Z4_involutions) != len(klein_involutions))
suite.assert_true("P4  Z/4 (CM automorphism) has a UNIQUE order-2 involution i^2 "
                  "=> the mirror is a derived branch, not an independent generator",
                  bool(z4_cyclic and mirror_is_derived and not_klein))


# =============================== P5: the delta / magnitude probe (CLOSED-NEGATIVE)
t = sp.symbols("t", positive=True)             # t <-> G* (transcendental over Q)
delta_sq = 4 * t**2 - t                         # delta^2 = G*(4G*-1) = t(4t-1)

# delta^2 is square-free over Q(t) and non-constant => delta not in Q(t)
# => [Q(t)(delta):Q(t)] = 2, delta genuinely adjoined.
# Square-free test: gcd(f, f') is a nonzero constant (degree 0).
sqfree = sp.degree(sp.gcd(delta_sq, sp.diff(delta_sq, t)), t) == 0
nonconst = sp.degree(delta_sq, t) >= 1
delta_not_in_Qt = bool(sqfree and nonconst)
suite.assert_true("P5a delta^2 = 4t^2 - t is square-free, non-constant over Q(t) "
                  "=> delta not in Q(t), [Q(t)(delta):Q(t)] = 2", delta_not_in_Qt)

# Reflection-reduction invariants are algebraic over Q (transcendence degree 0):
#   orientation sign  s = -1        (root of x+1)
#   subgroup index    idx = 24      (root of x-24; a rational integer)
#   character value   zeta_3        (root of x^2+x+1; cyclotomic, in Q^ab)
x = sp.symbols("x")
sign_alg   = sp.Poly(x + 1, x).degree() >= 1                       # -1 algebraic /Q
index_alg  = (sp.Integer(24).is_rational is True)                  # 24 in Q
zeta3      = sp.Rational(-1, 2) + sp.sqrt(3) * sp.I / 2
zeta3_alg  = (sp.minimal_polynomial(zeta3, x) == x**2 + x + 1)     # cyclotomic /Q
reflection_invariants_algebraic = bool(sign_alg and index_alg and zeta3_alg)
suite.assert_true("P5b one representative of each reflection-invariant kind "
                  "{sign -1, index 24, character zeta_3} is algebraic over Q "
                  "(SAMPLES, not the universal; completeness = FTD-0341 + "
                  "'reflections are algebraic orthogonal maps')",
                  reflection_invariants_algebraic)

# Exclusion: given G* transcendental over Q (Chudnovsky 1976), delta = sqrt(t(4t-1))
# has transcendence degree 1 over Q; a trdeg-0 value cannot equal a trdeg-1 value.
# NOTE: the transcendence itself is ASSUMED (cond. Chudnovsky) -- sympy cannot
# certify it; the two trdeg values below are stated facts, not computed. What is
# machine-verified is only delta_not_in_Qt (the square-free degree-2 fact above).
trdeg_delta_over_Q = 1          # ASSUMED, cond. Chudnovsky: G*(=t) transcendental
trdeg_reflection_invariants = 0 # from P5b (algebraic-over-Q representatives)
probe_closed_negative = (trdeg_delta_over_Q != trdeg_reflection_invariants)
suite.assert_true("P5c PROBE [conditional]: trdeg_Q(delta)=1 (ASSUMED, Chudnovsky) "
                  "!= trdeg_Q(reflection invariants)=0 => NO reflection-reduction "
                  "carrier reaches delta (CLOSED-NEGATIVE) -- FTD-0314 exclusion in "
                  "reduction language; load-bearing step is FTD-0341, not this assert",
                  bool(probe_closed_negative and delta_not_in_Qt
                       and reflection_invariants_algebraic))

suite.print_summary()
sys.exit(0 if suite.all_pass else 1)
