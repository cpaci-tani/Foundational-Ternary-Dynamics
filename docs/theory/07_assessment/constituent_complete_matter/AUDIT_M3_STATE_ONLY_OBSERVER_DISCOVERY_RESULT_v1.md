# FTD-0754 — M3 state-only observer discovery result audit v1

**Status:** `[CONSTRUCTIVE NUMERICAL FACT — STATE-ONLY OBSERVER DISCOVERY; M3 OPEN]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_STATE_ONLY_OBSERVER_DISCOVERY_v1.md`  
**Certificate:** `scripts/proofs/proof_state_only_observer_discovery.py`

## Verdict

The registered centered-readout separator is constructive as a discovery
observer. A complete instantaneous connected-pair state deterministically
produces:

- the selected finite-support minimum-energy Gauss dressing `F_b`;
- an exact residual centered field;
- its outgoing radial Maxwell characteristic `F_o`;
- the complementary incoming-plus-radial background `F_bg`.

No history, future state, route, preparation label, tick, or exterior-return
information enters the observer. Incoming, standing, radial-static, central,
cubic-rotation, polarity, bound-only, and Gauss-negative algebraic controls
pass. The three registered FTD-0753 histories replay byte-for-byte while the
new observer runs.

This passes observer **discovery only**. It is not held-out validation and does
not establish M3.

## Registered replay

The exact FTD-0753 face, edge, and body histories at periodic `L=321` were
replayed through tick 312. Each arm reproduced all 313 prior scalar CSV rows
byte-for-byte. The observer evaluated the frozen ticks
`{0,80,96,115,160,240,297,312}` and shells `{8,12,16,24,32,48}`.

The conjunction is:

- legacy scalar rows: `939/939` exact;
- state-only observations: `24/24` valid;
- independent certificate: `116/116`;
- focused CTests: `2/2`, including a separate complete-observer covariance
  test on a nontrivial divergence-free residual field.

Across all snapshots:

- maximum centered reconstruction residual: `4.7705e-18`;
- maximum actual-minus-bound Gauss residual: `9.4209e-14`;
- maximum quadratic energy-partition residual: `6.9389e-18`;
- maximum characteristic flux-identity residual: `2.1684e-19`.

The initial state on every arm equals its selected bound representative, so
the registered residual, outgoing, incoming, radial, and background energies
are exactly zero.

## Detached-shell discriminator

The radius-48 shell at tick 312 is strongly outgoing in the instantaneous
characteristic decomposition:

| arm | full residual outgoing fraction | incoming fraction | radial fraction | shell-48 outgoing | shell-48 incoming | outgoing/incoming |
|---|---:|---:|---:|---:|---:|---:|
| face | `0.70979` | `0.09025` | `0.19995` | `8.9533e-6` | `1.1543e-9` | `7756.5` |
| edge | `0.80558` | `0.07735` | `0.11706` | `8.9752e-6` | `1.1668e-9` | `7692.0` |
| body | `0.76150` | `0.06663` | `0.17187` | `8.9841e-6` | `1.1687e-9` | `7687.3` |

This is a stronger statement than the earlier radius-only energy tail: the
shell direction is decided by the local Maxwell characteristic, not by
distance. The near-core outgoing/incoming/radial fractions differ across ray
classes, so the result does not assert microscopic isotropy or call the whole
dynamic residual radiation.

## Bound--residual energy is not factorized

The exact outgoing/background energy partition applies inside the residual
field. It does not make the selected finite-support bound dressing orthogonal
to that residual. The recorded identity is

\[
H(F)=H(F_{\rm b})+H(R)+I_{\rm br},
\qquad
I_{\rm br}=H(F)-H(F_{\rm b})-H(R).
\]

Excluding the three zero-residual initial snapshots, `I_br` is nonzero in all
21 dynamic discovery snapshots: negative in 20 and positive in one. The
largest `abs(I_br)/H(F_b)` is `17.657%` in the edge arm at tick 160; the body
and face maxima are `16.159%` and `15.398%`. Consequently the result does not
license an additive interpretation in which `core + F_b` owns one independent
energy and the residual field owns the rest. The cross term is presently a
mixture of primitive boundary exchange and centered-readout corrections, not a
single relational interaction energy.

The FTD-0754B analytic addendum subsequently derives and measures the exact
selected-observer identity

\[
I_{\rm br}=I_{\partial K}+I_A+I_B.
\]

Here `I_partialK` is the primitive support-boundary exchange, `I_A` is the
face-centering correction, and `I_B` is the integer-time magnetic-readout cross
term. Existing-corpus replay preserves 24/24 old total strings exactly; the
primitive boundary identity closes to `3.6234e-16` and the full three-term
reconstruction to `1.3010e-18`. The centered total changes sign at face tick
115 even though the primitive boundary term remains negative. Consequently the
centered total cannot be a matter-membership margin or energy-ownership
observable. See
[THEOREM_STATE_ONLY_BOUNDARY_ENERGY_LEDGER_v1.md](../../10_eft_program/derivations/constituent_complete_matter/THEOREM_STATE_ONLY_BOUNDARY_ENERGY_LEDGER_v1.md).

This qualification changes no observer gate: centered field reconstruction,
Gauss compatibility, and the characteristic partition of `R` remain exact at
their registered tolerances.

## Exact scope boundary

The decomposition is exact in the registered odd-volume centered readout.
It is not an exact primitive-face/edge cochain decomposition and is not proven
to be the unique physical or ontological split. `F_b` is unique only within
the selected finite-support minimum-energy Gauss problem. `F_o/F_bg` are the
registered instantaneous characteristic convention.

The discovery histories were already observed in FTD-0753. Deterministic
replay was necessary because their archived records contain scalar ledgers,
not complete state checkpoints. Exact replay prevents trajectory drift but
does not make those states held out.

Consequently:

- FTD-0754 licenses a separately frozen FTD-0755 validation protocol;
- all formulas, shells, thresholds, negative controls, perturbation measure,
  volume ladder, and regularity bounds must be frozen before new states run;
- no discovery margin may be counted as a validation success;
- M3, full M2, particle identity, detached photon/radiation ontology,
  autonomous motion, poles, unitarity, and Lorentz recovery remain open.

Production defaults, the established CUDA library, scenarios, and ontology
are unchanged.

## Reproducibility

- protocol SHA-256:
  `D0861537AE33953169AD220E2E3416DF4D6B0BABFBDFF82CC553B85139879EC0`;
- result CSV/JSON and manifest:
  `engine/results/ftd_0754/`;
- independent certificate: `116/116`;
- runner uses the FTD-0752 explicit-rounding research backend and preserves
  exact FTD-0753 scalar rows.
