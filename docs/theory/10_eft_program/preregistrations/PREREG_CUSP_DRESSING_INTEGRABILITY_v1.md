# PRE-REGISTRATION — Cusp-dressing integrability v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0494`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0493`

## Question

Can the exact centered-trace omission

```text
W_cusp=(g q/2) sum_a J_a |d_a|
```

be absorbed into a local, single-valued dressing energy constructed only from
the frozen particle representation `(site,remainder)`, the adjacent face
electric field, polarity, and the existing coupling?  No history, source
provenance, branch label, global Poisson solve, or new primitive variable is
admitted.

## Candidate fixed by the one-cell work

Exact closure for every straight segment from a knot fixes the cellwise
candidate, with zero chosen at the knot, to

```text
U_n(r)=(g q/2) sum_a J_a(n) |r_a|,
J_a(n)=E_a(n)-E_a(n-e_a).
```

The candidate is gauge invariant, polarity-mirror even when `E` and `q` are
both mirrored, local, and covariant under signed cubic permutations.

Production identifies the threshold representations

```text
(n,e_a) ~ (n+e_a,0).
```

If site offsets `C(n)` make the energy representation independent, they must
obey

```text
C(n+e_a)-C(n)=(g q/2) J_a(n).
```

Such offsets are path independent only if every plaquette holonomy vanishes:

```text
H_ab(n)=(g q/2)[J_a(n)+J_b(n+e_a)
                 -J_b(n)-J_a(n+e_b)] = 0.
```

## Locked controls and counterexample

1. Require `U_n(d)=W_cusp` below `1e-12` for both polarities, all sign
   octants, translations, and signed coordinate permutations.
2. Require exact forward/reverse antisymmetry when the energy change is taken
   between the same two physical states.
3. Measure the raw threshold representation mismatch
   `(gq/2)J_a(n)` and verify the required site-offset equation.
4. On an even periodic lattice use

   ```text
   h(x,y)=j(-1)^(x+y),
   E_x=h/2, E_y=-h/2, E_z=0.
   ```

   Then `J_x=h`, `J_y=-h`, `J_z=0`, so `div E=0` at every site while
   the `xy` holonomy is nonzero.  Require divergence below `1e-12`, the two
   path sums to disagree by more than `1e-6`, and the analytic holonomy
   identity below `1e-12`.
5. Differentiate `U_n` with respect to its two adjacent faces.  Require the
   exact nonzero Euler derivative norm

   ```text
   ||dU/dE||_2=|g q| sqrt(sum_a |r_a|^2/2)
   ```

   for a nonzero remainder.  This records that adding `U_n` to a common
   action changes the field equation beyond the FTD-0478 transported current.
6. Differentiate `U_n` with respect to position away from the cusp.  Require
   that the centered interaction gradient plus `grad U_n` exactly reconstruct
   the ordinary one-sided branch derivative.  A variational cusp repair must
   therefore inherit the FTD-0491 branch problem.
7. A zero-jump control must have zero local energy, threshold mismatch,
   holonomy, and field derivative only when the remainder is also zero.

## Frozen verdicts

- `GLOBAL_LOCAL_DRESSING_STATE_FUNCTION` only if the one-cell primitive,
  threshold gluing, every registered plaquette, and the existing field
  variation all close without a new term.
- `CELLWISE_PRIMITIVE_GLOBAL_MEMORY_OBSTRUCTION` if the local primitive is
  exact but a Gauss-free allowed field has nonzero plaquette holonomy.
- `IMPLEMENTATION_INVALID` if the analytic primitive, holonomy, symmetry, or
  derivative controls fail.

## Consequence

A nonzero source-free holonomy proves that no single-valued local dressing
energy of the frozen variables can absorb FTD-0493 globally.  A cumulative
work ledger can close energy only by becoming a new history variable.  Adding
the cellwise potential to the action also changes the field equation, so it
cannot be silently appended to the existing matched-current transaction.

No production toggle, scenario, force amplification, or infrared campaign is
authorized.

Run-of-record SHA256 values:

- test: `8D46BF13B48B7DE86595DA3A530FD653B9415322B30B929A82C1AFF8F0462BBE`;
- header: `A70EC9EB4308266E32F1E3C4BDB3BE79523811CC2E6A8FFCB4CCF15261C1ABF9`;
- implementation:
  `DA1402C5952B8CFCAB02C54A4AD9142A98373352C53CA94F0E92253BC99B0562`.
