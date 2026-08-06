# PREREG — Orientation degree versus Gauss charge (FTD-0564)

**Status:** `[PRE-REGISTERED — EXACT OBSERVER / SCOPED NO-GO]`  
**Date:** 2026-07-26  
**Parent results:** FTD-0392, FTD-0398, FTD-0502, FTD-0563  
**Production effect:** none. This record authorizes observer-only algebra and synthetic fields. It does not modify the production tick, defaults, scenarios, ontology, or force law.

## 1. Frozen question

Can a nonzero Berg–Lüscher degree of the normalized native flux direction

\[
\widehat J=J/|J|
\]

on the octahedral Moore shell, by itself, force or quantize the enclosed electric Gauss flux?

The two observables are frozen as follows.

1. `orientation_degree` is the signed Berg–Lüscher solid-angle sum on the eight outward-oriented octahedral faces divided by `4 pi`.
2. `gauss_flux` is the exact flux of the piecewise-affine vertex field through the geometric octahedron. On each face it is `area * normal dot mean(vertex fields)`.

No fitted normalization, target value, stochastic seed, continuum extrapolation, or physical constant is used.

## 2. Locked fields

At the six unit-axis vertices `n in {+/-e_x,+/-e_y,+/-e_z}`, execute every arm below for

```text
A in {1, 1/2, 1/4, 1/8, 1/16}
polarity in {+1,-1}
cyclic cubic axis rotations in {0,1,2}.
```

The locked field families are:

```text
H(A,p,n) = p A n                         (hedgehog family)
T(A,p,n) = p A (n + 2 e_z)               (translated-image family)
```

with the fixed offset direction rotated together with the field under the cubic arm. `H` has orientation degree `p`. `T` is nowhere zero on the shell and its normalized image lies in an open hemisphere, so its degree is zero. Both have the same geometric Gauss flux `4 p A`; the constant offset integrates to zero on a closed polyhedron.

## 3. Locked identities and gates

The observer must establish, to `1e-12`:

1. all 30 hedgehog arms have `degree=p` and `flux=4 p A`;
2. all 30 translated-image arms have `degree=0` and `flux=4 p A`;
3. positive rescaling changes flux linearly while leaving degree fixed;
4. polarity reversal changes both signs exactly;
5. cyclic cubic rotations preserve both observables;
6. the paired arms `H` and `T` have equal nonzero flux but unequal degree;
7. no scalar function of degree alone can reproduce flux on the locked set, because one degree value occurs with five different flux magnitudes and equal flux occurs at degrees zero and nonzero;
8. the periodic cubic divergence has image equal to the complete zero-sum site-source subspace, citing and independently cross-checking FTD-0502's exact `rank(D)=V-1` construction for `L=3,5`;
9. production `J`, `W`, `J_L`, `J_R`, `W_L`, and `W_R` are regular real vector-space variables, while the optional SU(2)/SU(3) links are default-off, imposed, and write-only with respect to substrate evolution. These source-provenance facts are code-inspection assertions tied to file hashes in the run record.

The independent Python proof must reproduce the solid-angle and geometric-flux identities without importing the C++ implementation.

## 4. Frozen verdicts

- **INDEPENDENT:** all gates pass. Orientation degree neither determines nor is determined by electric Gauss flux. Topology alone cannot quantize electric-charge magnitude in the frozen variables.
- **COUNTEREXAMPLE FAILED:** any synthetic algebraic identity fails. No no-go conclusion is licensed.
- **PROVENANCE INVALID:** any frozen code-provenance assertion is false. The current-variable scope is invalid and must be rederived before a verdict.

## 5. Licensed conclusion if INDEPENDENT

The result closes only the claim that the normalized flux-direction degree *alone* supplies electric charge. It does not close topological defects, and it does not reinterpret FTD-0392/0398 as electric-charge campaigns.

On a periodic three-torus, any regular isolated vector-field zeros must have total index zero by the imported Poincaré–Hopf theorem (`chi(T^3)=0`); local defect/anti-defect pairs remain possible. A viable constructive route therefore requires both:

1. a protected defect sector or compact/bundle variable supplying an integer sign/class; and
2. a native common action or constitutive law supplying and stabilizing the Gauss-flux magnitude, energy, recoil, and transport.

The result does not authorize adding such a variable or action. That branch remains `[OPEN]`.

## 6. Required artifacts

- `engine/include/ftd/eft/orientation_gauss_independence.h`
- `engine/src/eft/orientation_gauss_independence.cpp`
- `engine/tests/test_orientation_gauss_independence.cpp`
- `scripts/proofs/proof_orientation_gauss_independence.py`
- `engine/results/ftd_0564/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/derivations/THEOREM_ORIENTATION_GAUSS_INDEPENDENCE.md`
- `docs/theory/07_assessment/constituent_complete_matter/AUDIT_ORIENTATION_GAUSS_INDEPENDENCE.md`

