#!/usr/bin/env python3
"""
VERIFIER for the E1 obstruction-map note (ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md).

Establishes, by computation this session, the TWO load-bearing numerical facts
behind the E1 (simple-cubic Watson constant) transcendence-degree question.
It proves NOTHING about algebraic independence -- that is the open problem.

  (1) [THEOREM, verified] The SC Watson constant is the 24th-division Gamma product:
        W_S := (1/pi^3) INT_[0,pi]^3 dk/(3 - cos k1 - cos k2 - cos k3)
             = 3 * INT_0^inf e^{-3t} I0(t)^3 dt
             = (sqrt(6)/(32 pi^3)) * Gamma(1/24)Gamma(5/24)Gamma(7/24)Gamma(11/24).
      => the SC constant's Gamma-content is Q(zeta_24)-class (24th-division points),
         whose individual transcendence is OPEN (Fermat-curve genus > 1 obstruction).

  (2) [NUMERICAL FACT, multiplicative-only, height <= 1e6] The SC Gamma-product
        P_SC = Gamma(1/24)Gamma(5/24)Gamma(7/24)Gamma(11/24)
      is NOT multiplicatively expressible via {Gamma(1/3), Gamma(1/4), pi, 2, 3}:
      PSLQ finds no integer relation among their logarithms up to coefficient
      height 1e6.  CAVEAT (mandatory): PSLQ-on-logs detects only MULTIPLICATIVE
      relations; ALGEBRAIC independence (what E1 asks) is strictly stronger and is
      NOT addressed by this test.  This is an absence-of-relation test against a
      basis fixed BEFORE the search, not a near-miss search.

State of the art (external, cited in the note): trdeg Q(pi, Gamma(1/4), W_SC, W_FCC)
has proven floor 2 (Chudnovsky 1976) and conjectural value 4 (Rohrlich-Lang / GPC);
the whole gap is open (Waldschmidt Conj 5.23; Lang's conjecture proven only n=3,4,6).
"""
from mpmath import mp, mpf, gamma, pi, sqrt, quad, exp, besseli, inf, fabs, log, pslq, nstr

mp.dps = 60
FAILS = 0


def check(name, cond):
    global FAILS
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}")
    if not cond:
        FAILS += 1


g = lambda a, b: gamma(mpf(a) / mpf(b))

print("=" * 72)
print("  VERIFIER: E1 simple-cubic Watson constant = Q(zeta_24) Gamma-class")
print("=" * 72)

# (1) SC closed form
print("\n-- (1) SC Watson constant closed form [THEOREM, verified] --")
torus_SC = quad(lambda t: exp(-3 * t) * besseli(0, t) ** 3, [0, inf])
P_SC = g(1, 24) * g(5, 24) * g(7, 24) * g(11, 24)
W_S = sqrt(6) / (32 * pi ** 3) * P_SC
print(f"    torus-mean INT e^-3t I0(t)^3 dt = {nstr(torus_SC, 30)}")
print(f"    (sqrt6/32pi^3) * P_SC           = {nstr(W_S, 30)}")
check("W_S == 3 * torus-mean (Watson [0,pi]^3 normalization)", fabs(W_S - 3 * torus_SC) < mpf('1e-25'))
print(f"    => SC Gamma-content = P_SC = G(1/24)G(5/24)G(7/24)G(11/24) = {nstr(P_SC, 20)}  (Q(zeta_24)-class)")

# (2) multiplicative non-reducibility of P_SC (pre-registered basis)
print("\n-- (2) P_SC not multiplicatively reducible to {G(1/3),G(1/4),pi,2,3} [NUMERICAL FACT, mult-only, h<=1e6] --")
basis = [log(P_SC), log(g(1, 3)), log(g(1, 4)), log(pi), log(mpf(2)), log(mpf(3))]
rel = pslq(basis, maxcoeff=10 ** 6, maxsteps=10 ** 5)
print(f"    PSLQ over logs of {{P_SC, G(1/3), G(1/4), pi, 2, 3}} -> {rel}")
check("no multiplicative relation up to height 1e6 (rel is None)", rel is None)
print("    CAVEAT: catches multiplicative relations ONLY; algebraic independence (E1) is strictly stronger and OPEN.")

print("\n" + "=" * 72)
if FAILS == 0:
    print("  BOTH FACTS VERIFIED. Transcendence/independence itself is OPEN (see the note).")
else:
    print(f"  {FAILS} CHECK(S) FAILED.")
print("=" * 72)
raise SystemExit(1 if FAILS else 0)
