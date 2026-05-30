# SPEC - Alpha Readout Contract

**Tag:** [REFERENCE] / [OPEN PROGRAM]
**Date:** 2026-05-18
**LEDGER:** FTD-0152 [SYNTHESIS] - formalizes MC-T4.3 closure criteria; introduces no new theorem and promotes no claim.
**Companion docs:** `SPEC_MATH_FIRST_ONTOLOGY.md`, `SPEC_PHYSICS_BRIDGE.md`, `SPEC_FQCR.md`, `SPEC_DOCTRINE_LEDGER.md`, `SPEC_OPEN_MATH_BY_SECTOR.md`, `FOUND_STRUCTURAL_DECOUPLING.md`

---

## 0. Purpose

This document states what it would mean to "earn the map" from the FTD/FQCR algebraic spine to the physical electromagnetic coupling.

It is the alpha-sector specialization of the math-first ontology in `SPEC_MATH_FIRST_ONTOLOGY.md`: finite invariant structure is primary, but physical constants require admissible operational readouts.

The current canonical status is:

- The master quadratic and FQCR transfer-matrix structure are mathematical spine content.
- The dominant branch `x_+` matches `1/alpha` to 1.26 ppm.
- The identification `x_+ <-> 1/alpha` is [STRONGLY MOTIVATED CONJECTURE], not a derivation.
- Four classical/action/gauge-field channels have failed to carry the master-quadratic value into engine observables.

Therefore the missing object is not another fit. The missing object is an operational readout rule.

---

## 1. The Map To Be Earned

The desired theorem has the following shape:

> **Alpha Readout Theorem (target, not established).** From FTD primitives plus an explicitly stated readout rule, construct a dimensionless operational electromagnetic coupling `alpha_read` such that `alpha_read = 1/x_+`, where `x_+` is the dominant root of the master quadratic / FQCR transfer operator. The construction must not use physical `alpha` or CODATA values as input, and it must explain why charge measurements access this readout rather than another distinguished algebraic number.

This is stronger than structural uniqueness. It must connect three layers:

1. **Algebraic layer:** `G*`, coefficient 16, master quadratic, FQCR branch.
2. **Readout layer:** a rule selecting a public/measurable observable from the finite substrate.
3. **Operational layer:** a measurement protocol corresponding to electromagnetic coupling.

Failure at any layer keeps the claim at [STRONGLY MOTIVATED CONJECTURE].

---

## 2. The Contract

Any proposed closure of MC-T4.3 must specify a tuple:

```
ARC = (P, A_obs, O_EM, R, C)
```

where:

- `P` is a preparation class: which FTD configurations or boundary conditions count as charge-like test systems.
- `A_obs` is the admissible observable algebra: which finite, gauge-invariant, translation/O_h-compatible functionals are allowed.
- `O_EM` is the electromagnetic measurement functional: what quantity a charge/scattering/field-strength measurement reads.
- `R` is the readout map: how `O_EM` returns a dimensionless inverse coupling.
- `C` is the calibration discipline: which dimensional or unit conventions are used, and why the result is dimensionless or calibration-invariant.

The proposal passes the **admissibility gate** only if all five elements are stated before checking any physical target value.

---

## 3. Hard Exclusion Rules

A closure attempt fails immediately if it does any of the following:

- Uses physical `alpha`, CODATA `1/alpha`, or a measured QED value as an input.
- Inserts `g_c`, `alpha`, or `x_+` into a standard formula and calls the result a derivation.
- Chooses a free parameter whose only role is to tune the output to `1/alpha`.
- Reuses an already closed-negative action-level path without changing the mechanism class.
- Produces a distinguished number but does not explain why electromagnetic measurements read that number.
- Depends on an arbitrary dimensional calibration where a dimensionless ratio should suffice.
- Treats standard QED/SM formulae as if they were FTD substrate results.

These rules preserve the difference between a physical readout and a substitution identity.

---

## 4. No-Go Boundary Already Established

The following channels do not earn the map:

| Channel | Status | Reason |
|---|---|---|
| Static Coulomb `V(r)` prefactor | Closed for alpha-readout | Periodic lattice Green's function fits with no fine-structure slot. |
| L=2 action / partition function | Closed at L=2 | Phase J ultralocality decouples the algebraic spine from the action. |
| Tick-cycle dynamical `V(r)` | Closed under tested protocol | Gauss projection erases the longitudinal `G_C` contribution every tick. |
| Fixed-field Wilson-Dirac `g-2` | Closed under tested protocol | Measures Wilson-r lattice artifact, not a QED Schwinger loop. |

These do not falsify the algebraic spine. They constrain the allowed readout mechanism to non-action or discrete-native channels.

---

## 5. Candidate Mechanism Classes

### A. Boundary-Condition Readout

**Target.** Define a finite or undefined-boundary self-consistency condition whose admissible spectrum includes the master-quadratic branch and selects `x_+` as the physical inverse coupling.

**Must prove.**

- The boundary rule is stated without `alpha`.
- The master quadratic emerges from the boundary problem rather than being pasted in.
- `x_+` is selected by stability, positivity, dominance, or measurement accessibility.
- `x_-` has a distinct interpretation, or is shown not to be the electromagnetic branch.

**Immediate falsifier.** The boundary condition has an adjustable coefficient equivalent to `alpha`, or both roots remain equally admissible as electromagnetic readouts.

### B. Observable-Selection Readout

**Target.** Define an FTD-native observable algebra whose public electromagnetic measurement functional has the FQCR/master-quadratic dominant eigenvalue as its inverse coupling.

**Must prove.**

- The observable is gauge-invariant and finite-block definable.
- The observable escapes the site-local mode-erasure no-go by using a non-site-local or bilinear/plaquette/flux-loop structure.
- The readout is operational: a charge interaction or scattering measurement would access it.
- The transfer/eigenvalue structure is derived from the observable algebra, not inserted after the fact.

**Immediate falsifier.** The observable is merely "the thing whose eigenvalue is `x_+`" without a measurement protocol.

### C. Quantization / Readout Rule

**Target.** Derive a discrete normalization rule for charge, flux, or action readout such that the measured electromagnetic coupling is the reciprocal of the dominant branch.

**Must prove.**

- The rule is dimensionless or calibration-invariant.
- The normalization is forced by finite trace, charge quantization, or public-readout consistency.
- It does not reduce to imported QED normalization.

**Immediate falsifier.** The rule is just `e^2/(4*pi) = 1/x_+` written in different notation.

### D. Discrete-Native Measurement Path

**Target.** Bypass continuum QFT reconstruction and define an engine-native measurement whose dimensionless output can be compared directly to an electromagnetic observable.

**Must prove.**

- The measured quantity is stable under lattice size, preparation variations, and calibration gauge.
- The experimental comparator is operationally clear.
- The result is not a fit to an SM formula supplied after the fact.

**Immediate falsifier.** The observable is not L-stable, not preparation-stable, or cannot be tied to a real measurement.

---

## 6. First Attack: Observable-Selection Readout

The most promising first target is Candidate B, not because it is easy, but because it best fits the failures already observed.

The action channel is blind. Site-local state readouts erase modes. Classical gauge-field prefactors are projected away. A plausible surviving mechanism must therefore live in a **non-site-local public observable**, such as:

- a closed flux-loop / Wilson-loop-style readout,
- a plaquette bivector readout,
- a bilinear link observable,
- a boundary-to-boundary transfer observable,
- or a reference frame projection from a finite observable algebra to a public measurement channel.

### ARC-B1 Target

Construct a finite-block observable family `O_N` and transfer/readout operator `T_O` satisfying:

1. `O_N` is built from FTD-native fields (`J`, `s`, finite differences, loops, plaquettes, or boundary traces).
2. `O_N` is invariant under translations, cubic symmetry, and gauge redundancies relevant to the chosen field representation.
3. The characteristic or fixed-point equation of `T_O` is the master quadratic or FQCR Model V branch at `t = 1`.
4. The dominant eigenvalue is selected by positivity/stability/accessibility, not by empirical matching.
5. The measurement interpretation says how a charge-like preparation reads `1/x_+`.

This is the first proof obligation. It can close positive, close negative, or split into a narrower mechanism.

### ARC-B1 Anti-Targets

Do not:

- start from the master quadratic and reverse-engineer `T_O`;
- call a visual/geometric analogy a measurement rule;
- identify `x_+` with `1/alpha` before deriving the readout;
- import the QED scattering formula as the definition of `O_EM`;
- use numerical searches for near-misses.

---

## 7. Status Levels

| Level | Meaning | Claim impact |
|---|---|---|
| ARC-0 | Tuple `(P, A_obs, O_EM, R, C)` stated and passes exclusion rules | Work package admissible |
| ARC-1 | Mathematical readout theorem proved inside FTD/FQCR structures | Upgrades mechanism, not yet physics |
| ARC-2 | Operational protocol tied to charge/scattering/field measurement | Candidate physical readout |
| ARC-3 | Measurement or derivation returns `1/x_+` without target input | `x_+ <-> 1/alpha` can be considered for tag upgrade |
| ARC-N | Mechanism fails a hard exclusion or falsifier | Preserve as closed-negative provenance |

No tag changes occur before ARC-3.

---

## 8. Near-Term Work Packages

| ID | Work package | Deliverable | Status |
|---|---|---|---|
| ARC-B1 | Observable-selection formalization | Candidate `A_obs`, `O_EM`, `T_O`, and proof attempt | [OPEN] |
| ARC-A1 | Boundary-condition sketch audit | Determine whether boundary self-consistency can produce the master quadratic without insertion | [OPEN] |
| ARC-C1 | Charge-normalization no-cheat audit | Catalogue which normalizations are imported QED vs FTD-native | [OPEN] |
| ARC-D1 | Discrete-native comparator inventory | List engine observables with real experimental comparators and dimensionless ratios | [OPEN] |

Recommended order: ARC-B1 -> ARC-C1 -> ARC-D1 -> ARC-A1. Boundary conditions are attractive but high-risk for hidden parameter insertion; observable-selection exposes the core issue fastest.

---

## 9. External Presentation Rule

Until ARC-3 exists, write:

> FTD/FQCR produces a rigid finite algebraic candidate for the inverse electromagnetic coupling. The identification with physical `1/alpha` is strongly motivated by structural uniqueness and empirical proximity, but the operational readout mechanism remains open.

Do not write:

> FTD derives alpha.

The project earns the map only when a reviewer can trace the chain from substrate preparation to observable algebra to public electromagnetic measurement without finding an inserted physical coupling.
