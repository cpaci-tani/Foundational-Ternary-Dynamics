# Audit — Removal-Time Pulse Bound

**FTD ID:** FTD-0589  
**Status:** `[THEOREM + NUMERICAL FACT + MEASURED]` +
`[CLOSED NEGATIVE — ARBITRARY ONE-TIME REMOVALS FOR N <= 6]` +
`[BOUNDARY SUPERSEDED BY FTD-0590 — N=7 CLOSED]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_6_CLOSED_NEXT_COUNT_7_UNRESOLVED`

## Finding

The FTD-0588 five-source residual-tail opening was not a physical candidate.
It was slack introduced by applying the triangle inequality before cancelling
the two constant pieces of a finite rectangular pulse.

The exact response is

\[
 r_n-r_{n-T}=2\sec(\theta/2)\sin(T\theta/2)
 \sin((n-(T-1)/2)\theta),
\]

which changes the one-source residual envelope from the valid but loose
`2(1+sec(theta/2))` to `2sec(theta/2)`.

Combining this with FTD-0588's common-history coefficient gives

\[
 |J|\le C_L\sqrt{N-r}+rP_L.
\]

The maximum is strictly below `K_GENESIS` for every `N<=6` and every removal
partition on all four registered volumes. A first-event induction therefore
forbids descendant genesis for arbitrary positions, polarities, and one-time
removal ticks at those counts.

## Scope controls

- The theorem is finite-volume and registered only at `L={9,17,33,65}`.
- The uncontained ontology is not replaced by a periodic container; these are
  finite computational quotients.
- `N=7` is not proved capable of genesis. It is the first count this bound
  fails to exclude.
- The 96 negative arms do not prove the theorem; they verify engine
  conformance and exercise complete removal.
- No force, movement, Gauss projection, hidden void kinematics, amplitude
  injection, or post-hoc schedule selection enters.

## Result integrity

- preregistration lock:
  `F438DBB1950E009641B1332D57B23B2EDFC23CD522A4E23C17E5FCC967AF5A33`;
- C++/Python spectral agreement: `<=5e-15`;
- exact pulse checks: 8,736;
- Gram checks: 48;
- cubic rotations: 24;
- live arms/ticks: 96/12,288;
- genesis/bound contradictions: 0/0;
- independent proof: 120/120 PASS;
- production/default/toggle/scenario changes: none.

## Program consequence

FTD-0588 remains active provenance for the spatial `sqrt(N)` theorem, but its
open five-source tail is superseded. The frozen causal source sector now has a
uniform closed-negative result through six sources. It still does not produce
reciprocal mobile matter. FTD-0590 subsequently evaluates the cubic-orbit
coherence norm and closes `N=7`; FTD-0591 closes the separately preregistered
`N=8` count, and FTD-0592 closes `N=9`. The next unevaluated count is `N=10`.
