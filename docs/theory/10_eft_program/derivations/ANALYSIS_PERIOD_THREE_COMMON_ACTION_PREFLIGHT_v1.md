# FTD-0717 — Period-three common-action preflight v1

**Status:** `[SELECTED DYNAMICS — MINIMUM-NORM FIELD REQUIRES COUPLED SELECTION]`  
**Verdict:** `PERIOD_THREE_MINIMUM_NORM_FIELD_REQUIRES_COUPLED_SELECTION`  
**Production status:** unchanged

## Result

The independently selected FTD-0715 momenta and FTD-0716 minimum-norm field
pass the exact trajectory, current, Gauss, and translated-return gates but do
not pass the locked per-tick energy/recoil conjunction.

```text
maximum absolute Gauss residual       1.8873791418627661e-15
complete translated field residual    8.8817841970012523e-16
maximum kinetic-plus-field residual   0.097323287313177076
maximum local momentum defect         0.087457200591678622
maximum spline momentum defect        0.078999918644817022
```

The three-tick sums telescope: total matter kinetic-energy change is zero and
total field-energy change is zero. The failure is local in time. On the first
tick the matter kinetic energy changes by `-0.0989765`, while the fixed field
changes by only `+0.00165326`; the last tick reverses the matter change while
the field supplies only `-0.00280509`.

## Gauss successor result

FTD-0716 did not independently gate absolute Gauss matching. FTD-0717 now
does. The minimum-norm translated field matches the quadratic constituent
density at all four phases below `1.89e-15`. Thus FTD-0716 is not merely a
transverse relative-orbit solution; it is Gauss-realizable for the registered
source.

## Scope of the negative verdict

The energy residual in this preflight is kinetic plus matched-field energy.
The registered shape deforms, so the connected binding energy is an omitted
reservoir. The number `0.0973` is therefore not a complete binding-inclusive
energy no-go. It shows that the independently selected minimum-norm field does
not by itself exchange the kinetic energy required by the prescribed momenta.

The net matter impulse cannot be repaired by an ordinary pairwise internal
binding force whose impulses sum to zero. The minimum-norm field supplies far
too little opposite tick recoil under both pre-registered momentum observers.
However, the FTD-0716 translated operator has `2182` homogeneous zero-singular
directions, all set to zero by the minimum-norm field construction. The
common action may select a nonzero homogeneous component.

Therefore the correct conclusion is:

> Kinematic compatibility plus Gauss-realizable field existence does not
> select the dynamics. The matter and field must be solved together.

It is not correct to rescale the FTD-0716 field, choose one favorable momentum
definition post hoc, or fit a few null modes after seeing these residuals.

## Next candidate

The existing-variable branch remains open. A fresh coupled optimization must:

1. include the complete binding-energy change;
2. solve matter momenta and the full field solution family simultaneously;
3. minimize field norm subject to exact Gauss, translated return, per-tick
   energy, recoil, work, causality, and cubic covariance;
4. select its homogeneous basis algorithmically before execution;
5. replay forward and state-only reverse without post-hoc correction.

Only failure of that coupled family, followed by causal formation and
constituent-permutation alternatives, would price a new internal primitive.

## Provenance

- protocol: `BCAE18C3...BD294B`
- summary: `292F869D...F7B51`
- ticks: `20BB5020...CB1E0`
- runner: `559C84A6...BD923`
- proof: `641747B4...6D698`

