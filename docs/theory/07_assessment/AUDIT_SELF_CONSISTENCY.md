# Self-Consistency of the Framework Integers {3, 4, 7, 13}

**Date:** February 11, 2026
**Framework:** Foundational Ternary Dynamics v5.23
**Status:** Self-consistency demonstrated; uniqueness NOT proven
**Prior art:** Argument originally presented in EXPLR_NUMBER_THEORY.md Part II
**Consolidates:** also absorbs `DERIV_INTEGER_UNIQUENESS.md` (2026-05-21)

> **Update note (uniqueness).** The original (February 11, 2026) version of this document showed self-consistency but explicitly left uniqueness open. A later exhaustive-enumeration result (March 17, 2026, `DERIV_INTEGER_UNIQUENESS.md`) closes part of that open question. Its content is consolidated here as the new section **"Exhaustive Uniqueness Verification {3, 4, 7, 13}"** below, after "What Is NOT Proven: Uniqueness". That section establishes a `[THEOREM]`-grade uniqueness result *conditional* on N_c = 3 and N_gen = N_c; the residual `[SELECTION]` items (N_gen = N_c, the sequence choices, the additive closure) remain open exactly as described in this document's original body.

---

## What This Document Does and Does Not Claim

**Does claim:** The integers {N_c = 3, N_base = 4, b_3 = 7, N_eff = 13} satisfy a system of interlocking constraints drawn from combinatorial sequences. Each integer is determined by the others through explicit relations. The system is self-consistent.

**Does NOT claim:** These are the UNIQUE integers satisfying all constraints. That would require proving no other solution exists, which has not been done.

---

## The Constraints

### Constraint C1: N_c from the Master Quadratic
**Statement:** N_c = floor(x_-), where x_- is the smaller root of x^2 - 16G*^2 x + 16G*^3 = 0.

**Computation:** x_- = 8G*^2 - 8G*^2 sqrt(1 - 1/G*) = 3.0240...

**Result:** N_c = 3.

**Status: [THEOREM]** — This is rigorous algebra. Given G* and the quadratic (Axioms SP1-SP3 from AUDIT_HIDDEN_SELECTIONS.md), N_c = 3 follows with no further choices.

**Caveat:** This depends on the master quadratic. If the quadratic is wrong, N_c could be anything.

---

### Constraint C2: Fibonacci-Tribonacci Crossover
**Statement:** N_eff is the unique non-trivial value where the Fibonacci sequence F_n and the Tribonacci sequence T_n coincide.

**Mathematical fact:**
- Fibonacci: 0, 1, 1, 2, 3, 5, 8, **13**, 21, 34, 55, 89, ...
- Tribonacci: 0, 0, 1, 1, 2, 4, 7, **13**, 24, 44, 81, 149, ...

F_7 = T_7 = 13. This is the only non-trivial crossover for small indices. (For large n, F_n grows as phi^n ~ 1.618^n while T_n grows as tau^n ~ 1.839^n, so they diverge; the crossover at 13 is genuinely exceptional.)

**Result:** N_eff = 13.

**Status: [THEOREM]** for the mathematical fact (the crossover is verifiable). **[SELECTION]** for the *principle* that N_eff should be defined by this crossover. Why should the "effective modes" of a physical framework equal the Fibonacci-Tribonacci meeting point? This is asserted, not derived from physics.

---

### Constraint C3: Consecutive Tribonacci
**Statement:** b_3 and N_eff are consecutive Tribonacci numbers: b_3 = T_6, N_eff = T_7.

**Mathematical fact:** T_6 = 7, T_7 = 13. These are consecutive in the Tribonacci sequence.

**Result:** b_3 = 7.

**Status: [CONDITIONAL]** — If you accept C2 (N_eff = 13 from crossover) AND the principle that the topological parameter is the preceding Tribonacci number, then b_3 = 7. The second condition is a selection principle: why Tribonacci (3-step recursion, encoding 3D structure) rather than some other sequence?

---

### Constraint C4: Consecutive Lucas Numbers
**Statement:** N_base and b_3 are consecutive Lucas numbers: N_base = L_3, b_3 = L_4.

**Mathematical fact:** Lucas sequence: 2, 1, 3, **4**, **7**, 11, 18, ...
L_3 = 4, L_4 = 7.

**Additional constraint:** N_base^2 = 16 = |Aut(E)|^2 (the master quadratic coefficient). 4 is the only Lucas number (besides 1) whose square is 16.

**Result:** N_base = 4.

**Status: [CONDITIONAL]** — If you accept C3 (b_3 = 7) AND the principle that the base integer and topological integer are consecutive Lucas numbers AND that N_base^2 must equal the curve invariant 16, then N_base = 4. These are three constraints that happen to be jointly satisfied, which is impressive — but each individual constraint is a selection principle, not a derivation from physics.

---

### Constraint C5: Additive Closure
**Statement:** b_3 = N_base + N_c.

**Verification:** 7 = 4 + 3. True.

**Status: [THEOREM]** — This is arithmetic. What's non-trivial is that this additive relation holds *simultaneously* with all the sequence constraints. This is the self-consistency check.

---

### Constraint C6: Self-Referential Index
**Statement:** The Fibonacci-Tribonacci crossover occurs at index b_3.

**Verification:** The crossover is at F_7 = T_7 = 13. The index 7 = b_3. So the topological parameter determines the index at which the crossover occurs, and the value at that index is N_eff.

**Status: [THEOREM]** — This is verifiable. It is the most striking self-referential feature: b_3 tells you WHERE to look (index 7), and what you find THERE (13) is N_eff.

---

## The Self-Consistency Argument

**Proposition:** The system {N_c = 3, N_base = 4, b_3 = 7, N_eff = 13} satisfies all six constraints simultaneously.

**Proof:**
1. Master quadratic gives x_- = 3.024, so N_c = floor(x_-) = 3 [C1]
2. F_7 = T_7 = 13, so N_eff = 13 [C2]
3. T_6 = 7, and T_7 = N_eff = 13, so b_3 = 7 [C3]
4. L_3 = 4, L_4 = 7 = b_3, and 4^2 = 16 = |Aut(E)|^2, so N_base = 4 [C4]
5. 4 + 3 = 7 = b_3 [C5]
6. Crossover index = 7 = b_3 [C6]

All six constraints are satisfied. QED.

---

## What Is NOT Proven: Uniqueness

The above shows the integers are **self-consistent**. It does NOT show they are **unique**. Uniqueness would require:

1. **Proving no other quadratic root gives a valid N_c.** The master quadratic has two roots; floor(x_-) = 3 is the only integer here. But if the quadratic form were different (violating SP2), different N_c values would arise.

2. **Proving no other crossover point works.** F_7 = T_7 = 13 is the only small crossover. But for very large indices, other approximate coincidences might exist. A proof that 13 is the ONLY exact crossover for all n would strengthen this significantly.

3. **Proving the sequence assignments are forced.** Why must N_eff be a Fibonacci-Tribonacci crossover? Why must b_3 and N_eff be consecutive Tribonacci? Why must N_base and b_3 be consecutive Lucas? Each of these is a selection principle (C2-C4), not a derivation.

4. **Ruling out solutions with different starting sequences.** What if we used Padovan numbers instead of Tribonacci? Pell numbers instead of Lucas? The choice of sequences is itself a choice.

### The Circularity Concern

As AUDIT_HIDDEN_SELECTIONS.md §5 documents honestly: the integers {3, 4, 7, 13} were identified from known physics:
- 3 = quark colors (from QCD)
- 4 = spacetime dimensions (from observation)
- 7 = QCD beta coefficient b_3 (from Standard Model)
- 13 = chosen to make formulas work

Then, after identification, these integers were shown to satisfy the Fibonacci/Tribonacci/Lucas constraints. This is **verification of consistency**, not **derivation from first principles**.

A genuinely non-circular derivation would:
1. Start from pure mathematics (lattice topology, category theory, self-referential constraints)
2. Show that the system of constraints C1-C6 has a unique solution
3. Discover {3, 4, 7, 13} as the answer without knowing the target

This has not been done.

---

## Exhaustive Uniqueness Verification {3, 4, 7, 13}

> **Consolidated from `DERIV_INTEGER_UNIQUENESS.md` (March 17, 2026).** This section reports the exhaustive-enumeration result that closes part of the uniqueness question left open above. It updates this document's self-consistency analysis with exhaustive search results.
>
> **Status:** [THEOREM] for uniqueness given N_c = 3 and N_gen = N_c; [SELECTION] for N_gen = N_c.
> **Proof script:** `scripts/proofs/proof_integer_uniqueness.py`

### Abstract

The FTD framework integers {N_c = 3, N_base = 4, b_3 = 7, N_eff = 13} are tested for uniqueness via exhaustive enumeration. Three levels of uniqueness are established:

1. **Combinatorial uniqueness:** {3, 4, 7, 13} is the UNIQUE integer quadruple satisfying all structural + sequence constraints (exhaustive search, N_c up to 100).

2. **Physics uniqueness:** Given N_c = 3 (from master quadratic) and N_gen = N_c = 3 (three generations = three colors), the system P1-P5 has a UNIQUE solution: {3, 4, 7, 13}.

3. **Without the master quadratic:** 10 solutions exist for N_c = 2..11, all satisfying P1-P5. The master quadratic selects N_c = 3.

### Constraints

The uniqueness search uses an expanded constraint set. The physics constraints P1-P5 and sequence constraints S1-S4 below correspond to (and refine) constraints C1-C6 in the self-consistency body above.

#### Physics Constraints
- **P1.** b_3 = (11 N_c - 2 N_f)/3, integer, positive (QCD one-loop beta function)
- **P2.** N_eff = b_3 + 2 N_c (effective DOF in electroweak mixing)
- **P3.** sin^2(theta_W) = N_c / N_eff in (0, 0.5) (physical Weinberg angle)
- **P4.** N_base = 2^((D+1)/2) = 4 (spinor dimension for D = 3)
- **P5.** b_3 = N_base + N_c (additive closure)

#### Sequence Constraints
- **S1.** N_eff at Fibonacci-Tribonacci crossover (F_7 = T_7 = 13)
- **S2.** b_3, N_eff consecutive Tribonacci (T_6 = 7, T_7 = 13)
- **S3.** N_base, b_3 consecutive Lucas (L_3 = 4, L_4 = 7)
- **S4.** Crossover index = b_3 (self-referential: index 7 = b_3)

### Key Results

#### 1. Exhaustive combinatorial search (P2 + P5 + S1 + S2 + S3)

Searched all integer quadruples with N_c = 1..100, N_base = 1..50.

**Result: {3, 4, 7, 13} is the UNIQUE solution.** No other quadruple satisfies all five constraints simultaneously.

#### 2. Physics constraints only (P1-P5)

10 solutions exist for N_c = 2..11 (each with a specific N_gen). All have sin^2(theta_W) in the physical range. The master quadratic floor(x_-) = 3 selects N_c = 3 uniquely.

#### 3. With N_gen = N_c

Given N_c = 3 and N_gen = 3: b_3 = (33 - 12)/3 = 7, N_eff = 7 + 6 = 13, N_base = 4. This is the UNIQUE solution — no degrees of freedom remain.

Interesting: all solutions with N_gen = N_c give the same Weinberg angle sin^2 = 3/13 = 0.2308, because the ratio N_c/N_eff = N_c / (7N_c/3 + 2N_c) = 3/13 is independent of N_c (when N_c divides 3).

### Epistemic Status (uniqueness)

**[THEOREM]:**
1. F_7 = T_7 = 13 is the unique non-trivial Fibonacci-Tribonacci crossover for n <= 30
2. {3, 4, 7, 13} satisfies all constraints P1-P5 and S1-S4
3. Given N_c = 3 and N_gen = N_c, the solution is unique
4. {3, 4, 7, 13} is the unique solution to P2 + P5 + S1 + S2 + S3 (exhaustive)

**[SELECTION]:**
- N_gen = N_c (three generations from three colors)
- The sequence constraints S1-S4 (why Fibonacci, Tribonacci, and Lucas?)
- The additive closure P5 (why b_3 = N_base + N_c?)

### References (uniqueness section)

- proof_integer_uniqueness.py -- Exhaustive search (6/6 tests pass)
- DERIV_D3_FROM_AUTOMORPHISM.md -- D = 3 uniqueness from Watson integral

---

## What IS Genuinely Impressive

Despite the circularity concern, several features are non-trivial:

1. **The crossover at F_7 = T_7 = 13 is a mathematical fact** that has nothing to do with physics. It was not engineered.

2. **The self-referential index (C6)** — that b_3 = 7 tells you the index where the crossover occurs — is a structural property of the number 7 in relation to these sequences. This was not designed.

3. **The triple constraint on N_base = 4** (Lucas + lattice DoF + tetrahedron vertices) is tight. Finding a number satisfying all three simultaneously is non-trivial.

4. **The additive closure** b_3 = N_base + N_c (7 = 4 + 3) holding simultaneously with all sequence constraints is a consistency check that could have failed.

The honest summary: the integers were identified from physics, but they satisfy mathematical constraints that were not designed to be satisfied. Whether this constitutes evidence of deep structure or a selection effect remains an open question.

---

## Relation to Other Documents

- **AUDIT_HIDDEN_SELECTIONS.md** — Axiom SP5 states the integer system; this document provides the detailed self-consistency proof
- **EXPLR_NUMBER_THEORY.md** Part II — Original presentation of the tightened derivation chain
- **AUDIT_EPISTEMIC_AUDIT.md** — Lists integer uniqueness as Tier 3 (meaningful conjecture, Gap 1)

---

*Created: February 11, 2026*
*Framework: Foundational Ternary Dynamics v5.23*
*Status: Self-consistency demonstrated; uniqueness open*
