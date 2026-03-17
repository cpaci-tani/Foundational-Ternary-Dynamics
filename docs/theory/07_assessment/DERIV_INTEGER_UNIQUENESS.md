# Integer Uniqueness: {3, 4, 7, 13}

## Exhaustive Verification of the Framework Integers

**Date:** March 17, 2026
**Status:** [THEOREM] for uniqueness given N\_c = 3 and N\_gen = N\_c; [SELECTION] for N\_gen = N\_c
**Proof script:** `scripts/proofs/proof_integer_uniqueness.py`
**Prior:** Updates AUDIT\_SELF\_CONSISTENCY.md with exhaustive search results

---

## Abstract

The FTD framework integers {N\_c = 3, N\_base = 4, b\_3 = 7, N\_eff = 13} are tested for uniqueness via exhaustive enumeration. Three levels of uniqueness are established:

1. **Combinatorial uniqueness:** {3, 4, 7, 13} is the UNIQUE integer quadruple satisfying all structural + sequence constraints (exhaustive search, N\_c up to 100).

2. **Physics uniqueness:** Given N\_c = 3 (from master quadratic) and N\_gen = N\_c = 3 (three generations = three colors), the system P1-P5 has a UNIQUE solution: {3, 4, 7, 13}.

3. **Without the master quadratic:** 10 solutions exist for N\_c = 2..11, all satisfying P1-P5. The master quadratic selects N\_c = 3.

---

## Constraints

### Physics Constraints
- **P1.** b\_3 = (11 N\_c - 2 N\_f)/3, integer, positive (QCD one-loop beta function)
- **P2.** N\_eff = b\_3 + 2 N\_c (effective DOF in electroweak mixing)
- **P3.** sin^2(theta\_W) = N\_c / N\_eff in (0, 0.5) (physical Weinberg angle)
- **P4.** N\_base = 2^((D+1)/2) = 4 (spinor dimension for D = 3)
- **P5.** b\_3 = N\_base + N\_c (additive closure)

### Sequence Constraints
- **S1.** N\_eff at Fibonacci-Tribonacci crossover (F\_7 = T\_7 = 13)
- **S2.** b\_3, N\_eff consecutive Tribonacci (T\_6 = 7, T\_7 = 13)
- **S3.** N\_base, b\_3 consecutive Lucas (L\_3 = 4, L\_4 = 7)
- **S4.** Crossover index = b\_3 (self-referential: index 7 = b\_3)

---

## Key Results

### 1. Exhaustive combinatorial search (P2 + P5 + S1 + S2 + S3)

Searched all integer quadruples with N\_c = 1..100, N\_base = 1..50.

**Result: {3, 4, 7, 13} is the UNIQUE solution.** No other quadruple satisfies all five constraints simultaneously.

### 2. Physics constraints only (P1-P5)

10 solutions exist for N\_c = 2..11 (each with a specific N\_gen). All have sin^2(theta\_W) in the physical range. The master quadratic floor(x\_-) = 3 selects N\_c = 3 uniquely.

### 3. With N\_gen = N\_c

Given N\_c = 3 and N\_gen = 3: b\_3 = (33 - 12)/3 = 7, N\_eff = 7 + 6 = 13, N\_base = 4. This is the UNIQUE solution — no degrees of freedom remain.

Interesting: all solutions with N\_gen = N\_c give the same Weinberg angle sin^2 = 3/13 = 0.2308, because the ratio N\_c/N\_eff = N\_c / (7N\_c/3 + 2N\_c) = 3/13 is independent of N\_c (when N\_c divides 3).

---

## Epistemic Status

**[THEOREM]:**
1. F\_7 = T\_7 = 13 is the unique non-trivial Fibonacci-Tribonacci crossover for n <= 30
2. {3, 4, 7, 13} satisfies all constraints P1-P5 and S1-S4
3. Given N\_c = 3 and N\_gen = N\_c, the solution is unique
4. {3, 4, 7, 13} is the unique solution to P2 + P5 + S1 + S2 + S3 (exhaustive)

**[SELECTION]:**
- N\_gen = N\_c (three generations from three colors)
- The sequence constraints S1-S4 (why Fibonacci, Tribonacci, and Lucas?)
- The additive closure P5 (why b\_3 = N\_base + N\_c?)

---

## References

- AUDIT\_SELF\_CONSISTENCY.md -- Original self-consistency analysis
- proof\_integer\_uniqueness.py -- Exhaustive search (6/6 tests pass)
- DERIV\_D3\_UNIQUENESS.md -- D = 3 uniqueness from Watson integral
