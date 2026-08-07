# PRE-REGISTRATION -- Collective-coordinate genesis counting model v1 (FTD-0277)

**Status:** `[PRE-REGISTRATION]` -- local hash-lock; run of record follows the
hash-lock in this session.
**Date:** 2026-06-14
**LEDGER id (reserved):** FTD-0277
**Git tag:** pending owner commit. This live attempt is locked by the SHA256
artifact hashes in SS3, not by a repository tag.
**Executes:** the FTD-0277 Arc-3 counting-model route scoped in
[`SCOPE_GENESIS_COUNTING_MODEL.md`](../../../03_derivations/foundational_mechanics/SCOPE_GENESIS_COUNTING_MODEL.md):
can a firing-rank collective-coordinate model derive the current-stack `N(A)` law
**given** the imposed engine register?

---

## SS1 - Purpose and narrow target

The prior current-stack work leaves a precise nonlinear-bridge boundary:

- FTD-0261 measured the current-stack `N(A)` law: broken power, knee near
  `A = 16`, sub-knee exponent about `3.69`, super-knee exponent about `1.86`.
- FTD-0269 showed a substrate-parameter forward model reproduces the **shape**
  only after using load-bearing engine constants: kinetic drain `0.5`,
  Langevin friction `gamma = 0.02`, `G_C = sqrt(alpha)`, and unit charge coupling.
- FTD-0276 closed the proposed `drain^2 = 1/4` origin negative and measured
  the empirical drain direction `k_eff ~ drain^-0.93`; drain and `gamma` remain
  imposed or calibrated unless separately derived.

This v1 attempt asks a narrower question: does the existing analytic
firing-rank recursion in `genesis_counting_model.py` reduce the engine's
current-stack law to a collective-coordinate counting model **given** the imposed
register?

Allowed success ceiling: `[CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]`.
Unconditional FTD-0110 closure is out of scope.

## SS2 - Frozen inputs and definitions

- **Model artifact:** `scripts/exploration/genesis_counting_model.py`.
- **Analyzer artifact:** `scripts/exploration/analyze_genesis_counting.py`.
- **Imposed register:** `drain = 0.5`, `gamma = 0.02`,
  `G_C = sqrt(alpha)`, `charge_coupling = 1`.
- **Target grid:** `A = {10,12,14,16,20,25,30,40,50,70,90}`.
- **FTD-0261 target means:** `{10:4.0, 12:8.4, 14:16.4, 16:21.6,
  20:27.4, 25:32.6, 30:45.0, 40:91.8, 50:130.2, 70:260.2, 90:383.3}`.
- **A=14 engine shell profile:** from
  `engine/results/genesis_geometry_2026-06-11/geom_A14.csv`, row-count profile
  `{center:0.059701, SC:0.358209, FCC:0.134328, BCC:0.373134, SC2:0.074627,
  outer:0}`.
- **Broken-power diagnostic:** least-squares continuous hinge in log-log space.
  The fitted knee is searched only over `A in [10,30]`.
- **Curve error:** RMS of `log10(N_model / N_FTD0261)` over the frozen grid.
- **Drain diagnostic:** log-log exponent of `k_eff = N(12)/12^2` over
  `drain = {0.125,0.25,0.375,0.5,0.625,0.75}` at `gamma = 0.02`.
- **Gamma diagnostic:** compare `gamma = 0` and `gamma = 0.10` on the same
  frozen grid; the declared direction is knee non-decrease and high-A ratio
  decrease with larger `gamma`.

## SS3 - Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/genesis_counting_model.py` | `4fdaa1f9e9e32735fbab9d0ed9752b09bc6610a19e637c778595b397fc1d617b` |
| `scripts/exploration/analyze_genesis_counting.py` | `7a4506022cf6927062b3d587a3c4082a5cda076ad3ea8c36bdf80ade96fd9a1b` |

Syntax-only check before lock:

```
python -m py_compile scripts/exploration/analyze_genesis_counting.py
```

## SS4 - Prior information (disclosed)

This is **not blind**. The development header of
`genesis_counting_model.py` already records the v0 expected failure modes:
the drain exponent emerges structurally near `-1`, but the gating lacks flux
consumption and the capture functional is about `25x` too generous. The
prior-favoured outcome is therefore **v1 CLOSED-NEGATIVE**, with the value of
the run lying in recording exactly which legs fail under fixed gates.

No threshold below is chosen from a post-run result.

## SS5 - Frozen verdict gates

The analyzer encodes these gates:

- **F1 broken-power shape:** pass iff knee `in [14,18]`, `p_lo in [3.3,4.1]`,
  and `p_hi in [1.6,2.1]`.
- **F2 magnitude:** pass iff curve `log10` RMS `<= 0.15`.
- **F3 A=10 firing count:** pass iff `N_model(10) in [3,7]`.
- **F4 firing geometry:** pass iff A=14 shell-profile L1 distance to the
  FTD-0269 engine profile is `<= 0.30`.
- **F5 drain law:** pass iff drain exponent is in `[-1.20,-0.70]`.
- **F6 gamma direction:** pass iff `knee(gamma=0.10) >= knee(gamma=0)` and
  the mean high-A ratio for `A >= 30` decreases at `gamma=0.10`.

Overall verdict:

- `CONDITIONAL_DERIVED_GIVEN_IMPOSED_INPUT` iff **all** F1-F6 pass.
- `COUNTING_MODEL_V1_CLOSED_NEGATIVE` iff any primary gate F1-F4 fails.
- `PARTIAL_BOUNDARY` iff F1-F4 pass but F5 or F6 fails.

## SS6 - Run of record

Run only after SS3 is written:

```
python scripts/exploration/analyze_genesis_counting.py ^
  --out scripts/exploration/results/genesis_counting_v1/analysis.txt ^
  --json-out scripts/exploration/results/genesis_counting_v1/analysis.json
```

The result directory is local run provenance. The canonical verdict is recorded
in the post-run analysis document.

## SS7 - Pre-declared exclusions

1. No changing the A-grid, target means, shell profile, or pass bands after the
   run.
2. No retuning the capture functional, `kappa`, shell cutoff, drain, or `gamma`
   inside v1 after observing the analyzer output.
3. A drain-law pass alone is not a derivation of `N(A)`; it is only one required
   leg.
4. A v1 closed-negative verdict does not demote the FTD-0110 linear
   `k = 1/4` theorem and does not by itself close every future collective
   coordinate route. It closes this locked v1 route.
5. Zero promotions: FTD-0013 stays `[SMC]`, MC-T4.3 stays
   `[FOUNDATIONAL OBSTRUCTION]`, FTD-0110 unconditional nonlinear bridge stays
   `[OPEN]` unless the full gate passes and a separate status update is made.

## SS8 - Hash-lock declaration

This document and the SS3 artifact hashes define the local lock for the 2026-06-14
run. Because the workspace is already dirty and no clean commit/tag is made in
this session, this is explicitly **not** a repository-tag lock. A future archival
commit may tag the exact files, but it must not alter SS2, SS5, or the analyzer
logic and still claim continuity with this v1 run.
