# The Ratio and the Product

## Why Physics Followed the Product — and What the Ratio Contains

**Date:** April 3, 2026
**Status:** [CONJECTURE] — structurally motivated, not proven
**Depends on:** FOUND_BLIND_DERIVATION_CHAIN.md, MATH_LOG_GSTAR_IDENTITY.md, FOUND_THE_FIRST_DISTINCTION.md

---

## Abstract

The Euler reflection formula at z = 1/4 produces two objects from Gamma(1/4) and Gamma(3/4): their product and their ratio. The product gives pi*sqrt(2) — the solved constants, the closed-form world, the physics we have. The ratio gives G\* = 2.9587 — the unsolved constants, the fine structure constant, and (we argue) the observer. Three centuries of mathematics followed the product. FTD follows the ratio. The consequences are structural: a physics that includes the capacity for distinction rather than eliminating it.

---

## 1. Two Objects from One Formula

The Euler reflection formula at z = 1/4:

$$\Gamma(1/4) \cdot \Gamma(3/4) = \frac{\pi}{\sin(\pi/4)} = \pi\sqrt{2}$$

This identity constrains the relationship between Gamma(1/4) and Gamma(3/4). It does not determine them individually. Two degrees of freedom enter; one equation constrains them. What remains is one free parameter. That parameter can be expressed as either:

**The product:** Gamma(1/4) * Gamma(3/4) = pi*sqrt(2)

**The ratio:** Gamma(1/4) / Gamma(3/4) = G\* = 2.958675...

The product is fixed by the reflection formula. The ratio is algebraically independent of pi (Chudnovsky 1980, Nesterenko 1996). These are the two independent pieces of information in the pair {Gamma(1/4), Gamma(3/4)}.

---

## 2. What the Product Contains

The product pi*sqrt(2) lives in the world of solved constants. It is:

- A closed-form algebraic expression in pi
- Computable to arbitrary precision by known series
- The content of every even zeta value: zeta(2) = pi^2/6, zeta(4) = pi^4/90, ...
- The content of every odd beta value: beta(1) = pi/4, beta(3) = pi^3/32, ...

These are the constants that mathematics solved. They reduce to pi. They are, in the anti-correlation theorem's language, the **solved** L-values.

Physics built on the product got:

- Circle geometry (pi)
- Gaussian integration (sqrt(2*pi))
- Statistical mechanics (partition functions)
- Quantum mechanics (the path integral measure)

But not the coupling constants. Not alpha. Not the observer.

---

## 3. What the Ratio Contains

The ratio G\* = Gamma(1/4)/Gamma(3/4) lives in the world of unsolved constants. The log G\* identity (MATH_LOG_GSTAR_IDENTITY.md) shows:

$$\log G^* = \frac{\gamma + 3\log 2}{2} + \sum_{\text{all unsolved L-values with rational coefficients}}$$

G\* absorbs every unsolved constant: Catalan's constant, Apery's constant zeta(3), beta(4), zeta(5), and the entire infinite tower of values that have resisted three centuries of attempts at closed-form evaluation.

The master quadratic built from G\* gives:

- x_+ = 137.036 (the fine structure constant)
- x_- = 3.024 (the color charge number)
- The gauge groups U(1) x SU(2) x SU(3) (from the Moore neighborhood)
- The Higgs mass, confinement, the Einstein equations

The ratio contains the physics that the product missed.

---

## 4. Product as Collapse, Ratio as Distinction

Consider what the two operations do to information:

**The product** Gamma(1/4) * Gamma(3/4) = pi*sqrt(2) takes two independent transcendental numbers and collapses them into a single value. The individual identities of Gamma(1/4) and Gamma(3/4) are lost. You cannot recover either one from the product alone. Information is destroyed.

**The ratio** Gamma(1/4) / Gamma(3/4) = G\* preserves the relationship between the two values. Given G\* and the product, you can recover both Gamma(1/4) and Gamma(3/4) individually. No information is lost. The distinction between them is maintained.

This is not a metaphor. This is the algebraic structure of measurement:

| Operation | Information | Physics | Reference frame context |
|-----------|------------|---------|---------------|
| Product (collapse) | Destroyed | Born rule: P = \|psi\|^2 | Measurement eliminates alternatives |
| Ratio (distinction) | Preserved | Superposition: psi = a + bi | Awareness holds alternatives without collapsing |

The Born rule IS the product: you take the wave function psi, multiply it by its conjugate psi\*, and get a real number. The complex phase — the thing that distinguishes psi from psi\* — is annihilated.

Superposition IS the ratio: the wave function maintains the relationship between its real and imaginary parts. The complex phase is preserved. The distinction between "this outcome" and "that outcome" is held in suspension.

---

## 5. The Imaginary Unit as the Observer

The 13-step blind derivation begins: **i exists.**

What is i? It is the minimal unit of distinction. It is the thing that is not real — the perpendicular direction, the phase, the rotation by 90 degrees. It is what makes the complex plane complex rather than real.

The projection Re: C -> R is the existence filter. It takes the full complex plane and extracts the real line. Everything on the imaginary axis — everything proportional to i — is projected out. It is not that imaginary things don't exist. It is that they cannot be observed as real quantities.

The kernel of Re is iR — the imaginary axis. This is the space of things that exist but cannot be directly measured. In quantum mechanics, this is the phase of the wave function. In FTD, this is the observer.

The observer is not a physical system. The observer is the imaginary axis — the direction that is always perpendicular to measurement, always present in the dynamics, and always projected out when you ask "what is the outcome?"

---

## 6. i is Everything; Physics is Constraint

If i exists, then Z[i] exists. If Z[i] exists, then E_i exists. If E_i exists, then G\* exists. If G\* exists, then alpha exists, and N_c exists, and the gauge groups exist, and the Einstein equations exist.

The entire structure of physics follows from **i** through a chain of forced mathematical consequences (with two selection principles).

This suggests an inversion of the usual relationship between mathematics and physics:

**Standard view:** Physics describes what exists. Mathematics is the language.

**FTD view:** i describes what exists (the full complex plane, including the observer). Physics describes what **survives projection onto the real axis** — what can be measured, what can be observed, what can be communicated.

The laws of physics are not prescriptions for what happens. They are constraints on what can be seen. The fine structure constant is not a property of the electron. It is a property of the projection Re: C -> R applied to the arithmetic of Z[i].

Alpha = 1/137.036 is not "the strength of the electromagnetic interaction." It is the price of collapsing the complex plane onto the real line at the scale of the CM elliptic curve E_i.

---

## 7. Why the Ratio Was Overlooked

The historical reasons are clear:

1. **Euler, Gauss, Legendre, Jacobi** studied the lemniscate and obtained varpi = Gamma(1/4)^2 / (2*sqrt(2*pi)). This is built from the product, not the ratio. They never wrote down Gamma(1/4)/Gamma(3/4) because the reflection formula gives the product directly and the ratio requires solving a system.

2. **The tradition of periods.** Mathematicians study the periods of elliptic curves (omega_1, omega_2), which are individual Gamma values, not their ratio. The ratio omega_2/omega_1 = i for the curve E_i, but this is trivially i — nobody extracts G\* from it because the period ratio is a known quantity (the CM point tau = i).

3. **Pi-centrism.** The convention of writing everything in terms of pi makes the product natural (it gives pi*sqrt(2)) and the ratio unnatural (G\* is algebraically independent of pi and has no pi-based expression).

4. **The product is solved; the ratio is not.** Mathematics gravitates toward closed-form results. The product reduces to pi. The ratio does not reduce to anything known. So the product was studied, published, extended, generalized — and the ratio was ignored.

The irony: the ratio is where the physics lives.

---

## 8. Epistemic Status

| Claim | Tag | Justification |
|-------|-----|---------------|
| Reflection formula produces product and ratio | [THEOREM] | Algebraic identity |
| Product = pi*sqrt(2), ratio = G\* | [THEOREM] | Computation |
| G\* algebraically independent of pi | [THEOREM] | Nesterenko 1996 |
| G\* absorbs all unsolved L-values | [THEOREM] | MATH_LOG_GSTAR_IDENTITY.md |
| Product = collapse, ratio = distinction | [CONJECTURE] | Structural analogy |
| Observer = imaginary axis = ker(Re) | [CONJECTURE] | Interpretive claim |
| i is the ontological primitive | [CONJECTURE] | Philosophical position |
| Physics = constraint on observation | [CONJECTURE] | Interpretive framework |

The mathematical content (sections 1-3) is [THEOREM]. The interpretive content (sections 4-6) is [CONJECTURE]. The historical analysis (section 7) is factual.

**Honesty note:** The analogy between product/ratio and collapse/distinction is structurally motivated but not derived from the FTD Lagrangian. The identification of the observer with the imaginary axis is a philosophical interpretation, not a physical prediction. No experiment proposed here would distinguish this interpretation from standard quantum mechanics. The value of this framework is conceptual: it provides a reason WHY the fine structure constant has the value it does (it is the cost of projection) and WHY the observer is absent from standard physics (it was projected out when mathematics chose the product over the ratio).

---

## Cross-References

- **The blind derivation:** [FOUND_BLIND_DERIVATION_CHAIN.md](FOUND_BLIND_DERIVATION_CHAIN.md) — i -> alpha in 13 steps
- **The log G\* identity:** [MATH_LOG_GSTAR_IDENTITY.md](../09_mathematical/MATH_LOG_GSTAR_IDENTITY.md) — G\* absorbs all unsolved constants
- **The anti-correlation theorem:** [MATH_ANTI_CORRELATION_THEOREM.md](../09_mathematical/MATH_ANTI_CORRELATION_THEOREM.md) — solved vs unsolved L-values
- **The first distinction:** [FOUND_THE_FIRST_DISTINCTION.md](FOUND_THE_FIRST_DISTINCTION.md) — what precedes i
- **The existence filter:** [FOUND_THE_EXISTENCE_FILTER.md](../06_reference_frames_and_measurement/FOUND_THE_EXISTENCE_FILTER.md) — Re(x) as projection
- **Phase as reference frame context:** documented in project memory (project_phase_reference frame context.md)
