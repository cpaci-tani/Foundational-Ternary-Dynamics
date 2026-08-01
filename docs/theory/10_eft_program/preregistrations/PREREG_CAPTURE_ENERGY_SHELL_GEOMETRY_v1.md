# FTD-0733 — Capture-energy shell geometry v1

**Status:** `[LOCKED BEFORE CERTIFICATION]`  
**Date:** 2026-07-29  
**Production status:** observer-only; no engine evolution or rule change

## Question

What is the exact connected negative-pair-energy radial domain of the selected
compact interaction at the kinetic energies realized by the FTD-0732 captured
states and perturbations?

This protocol resolves the domain defect exposed by FTD-0732. It does not
shrink the failed `5%` perturbation until it passes and does not test dynamical
survival.

## Frozen inputs

1. FTD-0732 CSV:
   `engine/results/ftd_0732/ftd_0732_captured_state_perturbation_survival_v1.csv`,
   SHA-256 `15926F9E64B8DE3A633CCE4794B07DAF40E6293D29D97DE63C89493980C2E2AD`.
2. Well depth `D=1/100` and cutoff coordinate `d_c=3/2`.
3. The selected interaction potential

   ```text
   V(d) = -16 D (d-3/2)^2 (d-3/4),  0 <= d < 3/2,
          0,                           d >= 3/2,
   ```

   where `d=r^2`.
4. Pair internal energy `E_K(d)=K+V(d)`, where `K` is the nonnegative
   constituent kinetic energy above rest reconstructed from each persisted
   FTD-0732 initial record as `K=E_pair-V(r^2)`.

No parameter is fitted.

## Exact theorem to certify

For every fixed `0<K<D`:

1. `E_K(d)` is strictly decreasing on `(3/4,1)` and strictly increasing on
   `(1,3/2)`;
2. there are unique roots
   `d_-(K) in (3/4,1)` and `d_+(K) in (1,3/2)`;
3. the connected captured radial set is exactly

   ```text
   C_K = {r >= 0 : E_K(r^2)<0}
       = (sqrt(d_-(K)), sqrt(d_+(K)));
   ```

4. the interval contracts monotonically as `K` rises and collapses to the
   minimum `d=1` as `K -> D`.

The proof must use the exact derivative

```text
dE_K/dd = -48 D (d-3/2)(d-1)
```

and certified monotone root isolation, not an unconstrained root search.

## Data certificate

For each `(L,direction,polarity)` group:

1. reconstruct `K` for every persisted variant;
2. verify that coordinate-only and field-only variants preserve the expected
   kinetic energy and that polarity mirrors agree;
3. identify `K_max` from the registered momentum variants without fitting;
4. certify the common radial interval valid for every registered kinetic level
   as `C_common=C_Kmax`;
5. report inner/outer roots in both `d` and `r`, the allowed multiplicative
   range relative to the unperturbed captured separation, and the parent's
   normalized squared-radius coordinate

   ```text
   u = (r_parent^2-d_-)/(d_+-d_-);
   ```

6. verify directly whether the old `0.95 r_parent` and `1.05 r_parent` probes
   lie in `C_common`.

Root brackets must have width below `1e-30`. Every reported inside/outside
classification must be separated from zero energy by more than `1e-12`,
except interval endpoints, which are certified by the monotone sign bracket.

## Verdict map

- `SELECTED_CAPTURE_ENERGY_SHELL_DERIVED`: the exact theorem and every source
  certificate pass; all parents lie strictly inside their common intervals.
- `CAPTURE_SHELL_EMPTY_AT_REGISTERED_KINETIC`: some registered `K_max>=D`, or
  no nonempty common interval contains its parent.
- `CAPTURE_SHELL_CERTIFICATION_INVALID`: the source hash, exact algebra,
  polarity reconstruction, or root-isolation gates fail.

The first verdict derives only the admissible domain of the **selected**
potential. It does not establish dynamical attraction, an open stability basin,
a physical particle radius, or derivation of the potential from the five
postulates.

## Next-use lock

A later mixed-corner survival protocol must parameterize radial perturbations
inside `C_common`, preferably by preregistered fractions of its squared-radius
width. It must recompute the relevant shell when momentum changes. It may not
reuse a Cartesian percentage that crosses the energy boundary, and it may not
infer dynamical stability from this static domain theorem.
