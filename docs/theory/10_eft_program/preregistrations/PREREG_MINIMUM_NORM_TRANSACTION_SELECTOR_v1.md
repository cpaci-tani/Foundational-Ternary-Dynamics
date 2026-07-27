# PRE-REGISTRATION — Minimum-norm local transaction selector v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0458`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0457`  
**Engine artifact:** `engine/tests/campaign_minimum_norm_transaction_selector.cpp`

**Locked SHA-256:**

- campaign: `984FE695762F7EB3F434B37D58F4614804F5904E56A0D99177116628E6B7A50E`
- independent certificate:
  `B9971BD60068B2806B7C97A914274E2B9C1A909F316579391CD45833EBBAFF60`

## 1. Question

Does the covariant-null construction used in FTD-0454 through FTD-0457 equal
the unique minimum-Euclidean-norm member of the supported zero-energy,
fixed-momentum transaction shell, and does that selector commute with the full
48-element cubic group on a native localized-packet event?

## 2. Frozen theorem statement

For supported impulse `S`, coefficient `c`, and full-rank momentum map `A`, let

```text
E(S) = 1/2 ||S||^2 + c.S,
A S = p.
```

The constrained minimizer `S_min` is unique. If `E_min<0`, every zero-energy
solution is

```text
S = S_min + z,
z in ker(A),
||z|| = r = sqrt(-2 E_min).
```

Let `n=P_ker(A)c`. Since `P_ker(A)S_min=-n`, Cauchy-Schwarz gives

```text
||S||^2 >= ||S_min||^2 + r^2 - 2 r ||n||,
```

with a unique equality point when `n != 0`:

```text
S_* = S_min + r n/||n||.
```

The registered claim is that `S_*` is exactly the solver's existing
`zero_energy_impulse`. This is a theorem about the selected quadratic event
functional, not a derivation of that functional from the five postulates.

## 3. Frozen native seed

- `L=33`, `+x` face hop, `q=+1`, speed `0.15`, work `1e-4`;
- 36-site `R=1` source/target support;
- FTD-0457 discrete-curl packet with `sigma_x=sigma_t=3`, `k0=pi/4`,
  direction `-x`, sampled after eight source-free ticks;
- packet amplitude fixed at `0.02`, above the registered FTD-0457 threshold;
- same minimal bound dressing and exact paired `Delta J=Delta W=S` functional.

No amplitude or direction search is performed.

## 4. Frozen algebraic certificate

Independently reconstruct `A`, `c`, `P_ker(A)c`, and the affine-shell geometry
from the pre-event fields and support. Require:

- rank-three Gram determinant finite and nonzero;
- ambient dimension `108`, nullity `105`, zero-shell dimension `104`;
- `E_min<-1e-8`, `r>1e-8`, and `||n||>1e-8`;
- `A n`, `P_ker(A)S_min+n`, shell-radius, selected-direction, selected-norm
  lower-bound, zero-energy, and momentum residuals each `<=1e-10`;
- five independent same-shell alternatives at registered angles
  `{pi/8,pi/4,pi/2,3pi/4,pi}` have energy/momentum residual `<=1e-10` and
  norm-squared excess above the selected point `>1e-10`.

The alternatives use one deterministic tangent direction obtained by
projecting the first eligible supported Cartesian basis vector into `ker(A)`
and orthogonalizing it against `n`.

## 5. Frozen cubic-covariance gate

Generate all six axis permutations and eight sign triples. For each of the 48
signed permutations:

- transform lattice coordinates about the central site;
- transform `J`, `W`, support, hop displacement, requested recoil, and the
  selected impulse as polar vectors;
- re-solve and re-certify the transformed native event;
- require work, minimum energy, selected norm, transformed minimum impulse,
  transformed selected impulse, complete event energy/momentum, and independent
  add/remove reversal residuals `<=1e-10`.

## 6. Locked classification

- `MINIMUM_NORM_LOCAL_SELECTOR_UNIQUE_CUBIC_COVARIANT`: theorem certificate and
  all 48 native covariance arms pass;
- `MINIMUM_NORM_SELECTOR_UNIQUE_NOT_COVARIANT`: theorem certificate passes but
  at least one native covariance arm fails;
- `MINIMUM_NORM_SELECTOR_DEGENERATE`: `n=0`, the Gram system loses rank, or an
  equal-norm distinct shell point is found;
- `MINIMUM_NORM_SELECTOR_NOT_EXISTENT`: the fixed seed has `E_min>=0`;
- `PROTOCOL_INVALID`: any registered reconstruction, closure, alternative,
  transformation, or reversal gate is numerically invalid.

## 7. Interpretation boundary

A positive verdict removes the continuous impulse ambiguity *conditional on*
adopting minimum impulse norm as a local variational selector. It does not show
that the frozen production tick uses the selector, establish consecutive hops,
or solve edge/corner route memory. No production dynamics are changed.

## 8. Recorded outcome

The independent certificate proves the existing covariant-null solution equals
the unique minimum-norm shell point for the registered full-rank event. Five
same-shell alternatives have strictly greater norm. All 48 signed-permutation
arms pass native covariance, complete closure, and reversal gates.

**Verdict:** `MINIMUM_NORM_LOCAL_SELECTOR_UNIQUE_CUBIC_COVARIANT`.
