# PRE-REGISTRATION — Local polarity regularity trilemma

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0540`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0538`, `FTD-0539`  
**Scope:** exact observer-only analysis of whether the FTD-0478 compact
subcell-polarity representation itself forces the FTD-0539 reflection-plane
cusp. No production state, default, toggle, scenario, force, collision law,
phase order, field ontology, normalization, action coefficient, energy
definition, smoothing rule, or tolerance changes.

## 1. Locked one-dimensional theorem

For a particle coordinate `x in [0,1]`, write `w_n(x)` for its coupling weight
at integer site `n`. Prove the following statements without numerical fitting:

1. If only sites `0` and `1` may carry weight and

   ```text
   w_0+w_1=1,       0*w_0+1*w_1=x,
   ```

   then `w_0=1-x`, `w_1=x` uniquely. Integer-translation covariance therefore
   gives the cardinal hat kernel `Lambda(u)=max(1-|u|,0)`.
2. The hat kernel has left/right derivatives `+1` and `-1` at its cardinal
   center, hence a derivative jump of magnitude `2`.
3. More generally, no locally finite family of nonnegative `C1` weights can
   simultaneously be cardinal at every integer and reproduce the first
   moment. At `x=0`, every off-center nonnegative cardinal weight has a local
   minimum and therefore zero derivative, whereas differentiating
   `sum_n n w_n(x)=x` requires `sum_n n w'_n(0)=1`.

The third statement is the locked no-go. It may not be weakened to the
particular hat kernel or inferred only from finite-difference data.

## 2. Locked three-dimensional lift

Prove that the multiaffine cardinal basis on a unit cube is unique and equals

```text
phi_v(r)=product_i [v_i r_i+(1-v_i)(1-r_i)],  v_i in {0,1}.
```

This is exactly the FTD-0478 trilinear shape. Restriction to any lattice axis
recovers the one-dimensional theorem, so the `C1` obstruction survives the
three-dimensional tensor lift. The proof does not claim that every possible
eight-site, non-multiaffine distribution is unique.

## 3. Locked escape witnesses

Evaluate two analytic witnesses to price the assumptions that must change:

- the centered quadratic B-spline, which is compact, nonnegative, `C1`,
  partition preserving, and first-moment reproducing, but has weights
  `(1/8,3/4,1/8)` at an integer and is therefore not cardinal;
- the Catmull-Rom cardinal cubic kernel (`a=-1/2`), which is compact, `C1`,
  cardinal, partition preserving, and first-moment reproducing, but has a
  negative lobe with exact minimum `-2/27` at `|u|=4/3`.

The implementation is an executable witness of the polynomial identities;
the theorem status rests on the analytic proof, not grid sampling.

## 4. Locked gates and verdicts

Require:

- exact two-equation uniqueness of the nearest-cell weights;
- exact hat one-sided slopes and jump;
- agreement of the current FTD-0478 shape with the unique tensor-product
  trilinear basis below `1e-12` on both polarities and translated/cubic copies;
- partition and first-moment residuals below `1e-12` for both escape witnesses;
- exact quadratic integer weights and Catmull-Rom negative-lobe value;
- invalid/nonfinite inputs fail closed.

Verdicts:

- all analytic identities and witnesses close:
  `LOCAL_POLARITY_REGULARITY_TRILEMMA_PROVED`;
- an analytic premise is false or a symbolic identity fails:
  `LOCAL_POLARITY_REGULARITY_TRILEMMA_REFUTED`;
- only floating-point witness gates fail:
  `LOCAL_POLARITY_REGULARITY_WITNESS_UNRESOLVED`.

The positive verdict does not validate a replacement action. It proves that a
restart must explicitly choose at least one price: a nonsmooth selector,
non-cardinal smooth coat, signed interpolation lobes, or a new primitive
carrier/shape variable. Exact energy and unique inversion remain separate
gates.
