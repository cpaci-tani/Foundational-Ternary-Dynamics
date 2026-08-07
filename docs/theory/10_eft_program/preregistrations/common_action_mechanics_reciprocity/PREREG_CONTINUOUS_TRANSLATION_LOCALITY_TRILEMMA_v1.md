# PRE-REGISTRATION — Continuous-translation locality trilemma

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0554`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0540`, `FTD-0552`, `FTD-0553`  
**Scope:** observer-only theorem plus finite periodic escape witness. No
production state, coupling shape, force, phase, toggle, default, scenario,
self-field subtraction, or ontology change.

## 1. Locked theorem

Let `U(a)` be a continuous one-parameter group of homogeneous convolution
operators on a one-dimensional integer lattice. Require:

1. `U(0)=I` and `U(a+b)=U(a)U(b)`;
2. every `U(a)` is unitary, so every translation-invariant positive quadratic
   field norm is preserved;
3. `U(1)=T`, the one-site lattice shift;
4. every convolution kernel has finite support.

For fixed `a`, finite support gives a Laurent-polynomial Fourier symbol
`p_a(z)`. Unitarity gives `|p_a(e^{ik})|=1` for every `k`. The locked algebraic
lemma is that a finite Laurent polynomial of constant unit modulus is a phase
times a monomial:

```text
p_a(z)=exp(i theta(a)) z^m(a),   m(a) in Z.       (1)
```

Continuity makes integer `m(a)` locally constant. Since `m(0)=0`, it is zero
on the connected parameter line, contradicting `U(1)=T`. Thus a nontrivial
exact continuous translation group on a discrete lattice must give up strict
finite-range locality, unitarity/energy preservation, homogeneity, or the
one-site shift endpoint.

The tensor-product lift makes the same statement in three dimensions.

## 2. Locked nonlocal escape

For odd periodic `L=2M+1`, define the canonical band-limited fractional shift
of a cardinal site by

```text
S_f(n)=1/L sum_{m=-M}^{M}
       exp[2 pi i m(n-f)/L].                       (2)
```

Equation (2) must satisfy:

- real weights, partition `sum_n S_f(n)=1`, and `S_r(n)=delta_(n,r)` for
  integer `r`;
- exact Fourier composition `S_(a+b)=U(a)S_b` and constant `l2` norm;
- for every noninteger registered `f`, support on all `L` sites and at least
  one negative weight;
- for a neutral integer-offset composite, every translation-invariant
  quadratic spectral energy is independent of `f` because the entire source
  transform changes only by `exp(-ikf)`.

For a finite move `f_0->f_1`, solve exact one-dimensional continuity in Fourier
space. The resulting current is required to have support growing with `L`,
demonstrating that the pinning-free escape is not a local current update.

## 3. Locked campaign

Use `L=17,33`, fractions

```text
f in {0,1/8,1/4,3/8,1/2},                         (3)
```

composition pairs `(1/8,1/4)` and `(1/4,1/4)`, neutral separations `d=1,3`,
and continuity moves `0->1/8`, `1/8->1/4`, `1/4->1/2`.

Required gates:

- reality, partition, cardinality, norm, group composition, neutral spectral
  energy, and continuity residuals below `1e-12`;
- every noninteger kernel has support exactly `L` and minimum weight below
  zero;
- every registered nonzero move has density-change and current support greater
  than `L/2`;
- the local quadratic `d=1` control retains a positive FTD-0553 coefficient
  and barrier above `1e-12`.

No geometry, volume, support threshold, or tolerance changes are permitted
after execution.

## 4. Locked verdicts

- theorem identities close, the band-limited witness removes the Peierls
  dependence, and its support is nonlocal:
  `EXACT_TRANSLATION_REQUIRES_NONLOCAL_COUPLING`;
- a finite-range exact unitary continuous-shift witness satisfying `U(1)=T`
  appears: `LOCAL_CONTINUOUS_TRANSLATION_COUNTEREXAMPLE`;
- an algebraic or executable witness gate fails:
  `CONTINUOUS_TRANSLATION_TRILEMMA_OBSERVER_INVALID`.

The first verdict does not license (2) as FTD dynamics. Its global signed coat
and global continuity current violate the frozen locality/positive-coat
contract. It instead establishes that the compact-coat Peierls force cannot be
removed merely by choosing a higher-order local interpolation kernel.

## 5. Run disposition

Run 2026-07-26 on the pinned MSVC CPU observer. The theorem and both witnesses
close: the band-limited translation group preserves neutral field energy to
`6.94e-17`, while every registered noninteger density change and exact current
has full `L` support and the minimum kernel weight is `-0.216195...`. The
locked verdict is `EXACT_TRANSLATION_REQUIRES_NONLOCAL_COUPLING`. See
[`AUDIT_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md).
