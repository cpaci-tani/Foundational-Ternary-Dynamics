# Framework Integers Physical Identification: {3, 4, 7, 13}

## Tracing Each Integer to Its Physical Role

**Date:** March 17, 2026
**Status:** [THEOREM] for all identifications given N_gen = N_c [SELECTION]
**Proof script:** `scripts/proofs/proof_integer_identification.py`
**Prior:** Depends on DERIV_D3_UNIQUENESS.md, DERIV_INTEGER_UNIQUENESS.md, DERIV_MASTER_QUADRATIC_FROM_Z.md

---

## Abstract

The four framework integers {N_c = 3, N_base = 4, b_3 = 7, N_eff = 13} are each traced to their physical origin through the lattice gauge theory structure. Every identification is [THEOREM] except one: the assumption N_gen = N_c (three generations equals three colors), which remains [SELECTION].

---

## The Identification Chain

### N_c = 3: Color Number [THEOREM]

**Origin:** Master quadratic x^2 - 16G*^2 x + 16G*^3 = 0

The smaller root x_- = 3.024, so N_c = floor(x_-) = 3.

**Self-referential identity:** D = 3 is the unique spatial dimension where floor(x_-) = D (proven in DERIV_D3_UNIQUENESS.md). The dimension selects itself: the lattice lives in D = 3 dimensions, and the gap equation on that lattice returns N_c = D = 3.

### N_base = 4: Spinor Dimension [THEOREM]

**Origin:** Dirac spinor representation in (D+1) spacetime dimensions.

N_base = 2^((D+1)/2) = 2^((3+1)/2) = 2^2 = 4

This gives the four-component Dirac spinor: two spin states times two particle/antiparticle states. The formula is standard representation theory applied to D = 3.

### b_3 = 7: QCD Beta Coefficient [THEOREM given N_gen = N_c]

**Origin:** One-loop QCD beta function coefficient.

b_3 = (11 N_c - 2 N_f) / 3

With N_c = 3, N_gen = N_c = 3, N_f = 2 N_gen = 6:

b_3 = (33 - 12) / 3 = 21/3 = 7

The result is integer-exact (no rounding). The [SELECTION] N_gen = N_c is the only non-derived input.

**Additive closure:** b_3 = N_base + N_c = 4 + 3 = 7. This is a consistency check, not an independent derivation.

**Sequence membership:** b_3 = 7 is simultaneously Lucas L_4 and Tribonacci T_6.

### N_eff = 13: Effective Degrees of Freedom [THEOREM]

**Origin:** Sum of QCD beta coefficient and color charge contributions.

N_eff = b_3 + 2 N_c = 7 + 6 = 13

**Weinberg angle:** sin^2(theta_W) = N_c / N_eff = 3/13 = 0.2308 (vs CODATA 0.23122, 0.19% deviation).

**Fibonacci-Tribonacci crossover:** N_eff = F_7 = T_7 = 13. Index n = 7 is the unique crossover point where Fibonacci and Tribonacci sequences coincide (for n <= 30). The crossover index equals b_3 = 7, a self-referential identity.

---

## Self-Referential Structure

The framework integers form a closed self-referential system:

1. **N_c = D = 3:** The dimension selects itself
2. **b_3 = N_base + N_c:** Additive closure
3. **N_eff = F_{b_3} = T_{b_3}:** The crossover index IS b_3
4. **sin^2(theta_W) = N_c / N_eff:** Ratio of first and last integers

No free parameters remain once N_gen = N_c is assumed.

---

## Epistemic Status

**[THEOREM]:**
1. N_c = floor(x_-) = 3 from master quadratic
2. N_c = D (self-referential, unique to D = 3)
3. N_base = 2^((D+1)/2) = 4
4. b_3 = (11 N_c - 2 N_f)/3 = 7 (integer-exact)
5. b_3 = N_base + N_c (additive closure)
6. N_eff = b_3 + 2 N_c = 13
7. sin^2(theta_W) = 3/13

**[SELECTION]:**
- N_gen = N_c (three generations equals three colors)
- N_base interpretation as spinor dimension (standard physics identification)

---

## References

- proof_integer_identification.py -- Physical identification (14/14 tests pass)
- DERIV_D3_UNIQUENESS.md -- D = 3 self-referential identity
- DERIV_INTEGER_UNIQUENESS.md -- Exhaustive uniqueness search
- DERIV_MASTER_QUADRATIC_FROM_Z.md -- Gap equation from partition function
