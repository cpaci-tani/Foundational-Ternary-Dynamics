# FTD-0427 — Projection-Free Matched Gauss Transport v1

**Status:** [PRE-REGISTRATION — LOCKED BEFORE CAMPAIGN]
**Date:** 2026-07-23
**Scope:** selected experimental sidecar driven by production movement; the
production tick and voxel flux field are unchanged

## 1. Question

Can a local oriented-face flux, updated only by the signed current carried by
actual production movement events plus a divergence-free transverse term,
preserve a Gauss relation without repeated projection?

The tested equations are

\[
q_h=D J,\qquad \Delta s + D K=0,\qquad
J^{n+1}=J^n-K+C B,
\]

where `D` is the periodic backward-difference face divergence, `K` is the
integrated signed face current extracted from one production tick, and `C` is
the matched backward-difference curl. The implementation must establish the
algebraic identity `D C = 0` independently of the campaign.

## 2. Status lock

This is a **[SELECTED MECHANISM]**, not native charge emergence. It interprets
the ternary sign as a low-energy source candidate only in the movement-only
sector and adds a face/cochain placement for the existing flux type in an
experimental sidecar. It does not alter `Voxel::flux`, feed forces back into
`RenderBridge`, derive `U(1)`, recover Maxwell dynamics, or supersede
FTD-0421's nullity-zero result for the full production event set.

A pass establishes sufficiency of one local projection-free transport rule.
It does not establish that the frozen production engine already uses that
rule.

## 3. Frozen implementation

- Lattices: periodic `L=32` and `L=64`.
- Backends: Windows MSVC CPU and an independent WSL2 GCC CPU build. The
  history observer is CPU-only; no CUDA result is claimed.
- Production toggles: `disable_all()`, then `movement=true` and
  `dual_substrate=false`. In particular, `gauss_projection=false` for the
  complete run.
- Sources: one mobile sign `q` and one locked sign `-q`, for `q=+1,-1`.
- Directions: the six axial directions `+x,-x,+y,-y,+z,-z`.
- Motion: `0.99*C_SPEED`, 12 moving ticks followed by 8 stationary ticks.
- Initialization: one deterministic shortest oriented-face path from the
  mobile source to the stationary sink. This is an exact Gauss seed, not a
  Coulomb profile and not evidence for a force law.
- Transport extraction: before/after production state snapshots through
  `extract_moore_history_from_snapshots`; reaction terms are forbidden.
- Transverse challenge: after every tick, add a fixed nonzero analytic edge
  field through the matched curl with amplitude `1e-3`.
- Surface estimator: exact face-flux cube sums at radii `2,3,4` centered on
  the current mobile site.
- RNG seed: `0x0427` for every arm. The enabled dynamics consume no random
  values.

## 4. Frozen estimators

For every tick record:

- `transport_residual = max |Delta s + D K|`;
- `gauss_residual = max |D J - s|` after both `-K` and `+C B` updates;
- `curl_divergence = max |D C B|`;
- surface charge and surface/divergence telescope residual at radii 2, 3, 4;
- total signed state, current L1 norm, movement count, and whether any reaction
  was classified.

The radius plateau is

\[
P={\max_r Q_r-\min_r Q_r\over
\max(1,|\operatorname{mean}_r Q_r|)}.
\]

## 5. Acceptance gates

Every arm, size, backend, moving tick, and stationary tick must satisfy:

1. no production Gauss projection and no reaction term;
2. at least five actual movement events during the moving interval;
3. `transport_residual <= 1e-12`;
4. `curl_divergence <= 1e-12`;
5. `gauss_residual <= 1e-12`;
6. surface/divergence telescope residual `<= 1e-12`;
7. surface sign equals `q`, `|mean_r Q_r-q| <= 1e-12`, and `P <= 1e-12`;
8. global signed state remains exactly zero;
9. the transverse challenge is nontrivial: its curl L1 norm is positive;
10. the stationary interval preserves every preceding gate with zero current.

Windows and WSL2 maxima must agree to absolute `1e-12`; event counts must
agree exactly.

## 6. Locked outcomes

| outcome | interpretation |
|---|---|
| A: all gates pass | **[THEOREM — selected discrete complex] + [MEASURED — production movement compatibility]**: the selected face-current rule transports Gauss charge locally without repeated projection in the restricted movement sector |
| B: algebra passes, production coupling fails | **[SELECTED MATHEMATICAL MECHANISM ONLY]**: the cochain rule is consistent but the production movement adapter does not preserve its source dictionary |
| C: algebra fails | **[CLOSED NEGATIVE]**: the proposed matched operator complex is internally inconsistent |
| D: backend or observer mismatch | **[INVALID CAMPAIGN]**: repair only instrumentation and rerun under a new lock |

## 7. Explicit non-claims and successor

No result licenses the words emergent electromagnetism, photon, Coulomb law,
gauge symmetry, conserved full-production charge, or autonomous native
dressing. Genesis, evaporation, pair production, annihilation, weak
transmutation, production wave propagation, feedback forces, and radiative
stability are outside this v1 scope.

If Outcome A occurs, the only licensed successor is a separately
preregistered integration test that replaces the production longitudinal
projector with the matched local update and then tests an extended Coulomb
profile plus transverse propagation. It must not be enabled by default during
this campaign.
