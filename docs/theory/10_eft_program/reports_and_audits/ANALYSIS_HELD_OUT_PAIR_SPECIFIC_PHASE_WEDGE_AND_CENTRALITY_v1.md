# FTD-0913 — Held-out pair-specific phase-wedge/centrality result v1

**Identifier:** `FTD-0913`  
**Date:** 2026-08-11  
**Status:** `[LOCKED/RUN — OUTCOME D (P-C-)]`
`+ [CLOSED NEGATIVE — TWO-ENDPOINT WEDGE AS PAIR-SPECIFIC PROTECTED MEMORY]`
`+ [CLOSED NEGATIVE — FTD-0907 EXACT CENTRAL LAW AS CURRENT PRODUCTION LAW]`
`+ [OPEN — DIFFERENT NATURAL BINDING/TOPOLOGICAL CARRIER]`  
**Protocol:** FTD-0911, SHA-256
`D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE`  
**Instrument lock:** FTD-0912, SHA-256
`F134953BAD8D8353BF8DB8DB2E89DF685BD4404BE8F30A61F6FEA499F2F895BA`

## 1. Verdict

The held-out campaign is protocol-valid and returns its frozen **Outcome D**:

```text
PAIR_SPECIFICITY=FAIL
EXACT_CENTRALITY=FAIL
PAIR_CENTRALITY_VERDICT=OUTCOME_D_NOT_PAIR_SPECIFIC_NOT_EXACT_CENTRAL
```

The current production tick forms durable neutral pair carriers, as FTD-0908
measured, but it does not bind the bilateral phase-wedge sign specifically to
the actual endpoint pairing. It also does not instantiate the exact central
phase-space dynamics under which the FTD-0907 wedge would be conserved.

This answers the immediate mechanism question negatively and precisely:

> The missing dynamics are not merely an unmeasured maintenance cost. The
> present production law lacks both a pair-specific chirality binding and the
> central restoring symmetry needed to make `ell` a conserved recursive
> memory coordinate.

Per the frozen protocol, no perturbation/work campaign is licensed for this
two-endpoint wedge candidate.

## 2. Execution integrity and corpus

- FTD-0911 source/protocol preflight: `28/28`;
- FTD-0912 runner/adjudicator preflight: `23/23`;
- focused CTest `held_out_pair_specific_phase_wedge_centrality`: `1/1`,
  `116.25 s`;
- frozen independent raw-corpus adjudicator: `20/20`; and
- production state/RNG nonmutation and all reconstruction/algebraic controls:
  pass.

| Corpus artifact | Bytes | SHA-256 |
|---|---:|---|
| `ftd_0911_pair_observations_v1.csv` | 6,378,113 | `2ACA1F93D6EEB421591700EF0AF9D60FB9E247171F6AD0FC28DC703FDE547686` |
| `ftd_0911_tick_census_v1.csv` | 1,317,514 | `F4AA7FD179A53B8671BB960EE6317D6214AEC47C9D11CCFEAAA4CEEAF63F35B2` |
| `ftd_0911_derangements_v1.csv` | 5,152 | `658D034C719656E4D21A8F1D3610F34F746F694AFFCD75B95853DCE34D68C279` |
| `ftd_0911_chronology_controls_v1.csv` | 6,872 | `023FDC6969DDDB90417CD423DA4F09C7F90A9A14389EE55FB830C07DCF3A2614` |
| `ftd_0911_central_transitions_v1.csv` | 2,962,069 | `91109D526040DAB8F4105327C0CA7A399B2A479EBF8D5233D6A2C4D5EDEE1CAC` |
| `ftd_0911_summary_v1.json` | 49,129 | `6E708F04CEEF79F271AD696D721E25817809EB190A78B615555CCDE0099C6A97` |

All files live under `engine/results/ftd_0911/`. The frozen runner is
`09295483...CE5D`; the independent adjudicator is `7FB9F357...5CEA`.

## 3. Pair-specificity result

All six live cells meet the qualification gate, so `P-` is substantive rather
than an insufficient-data outcome. There are 41 qualified live arms. Only six
actual pairings strictly beat every cyclic derangement; no cell reaches the
required six of eight.

| L | Family | Qualified seeds | Pair-pass seeds | Retained actual IDs | Actual-minus-best-null margins |
|---:|---|---:|---:|---:|---|
| 19 | `axial_live` | 7 | 3 | 17 | -15, 1, 2, 19, -1, -7, -12 |
| 19 | `diagonal_live` | 6 | 0 | 17 | -8, -7, -6, 0, 0, 0 |
| 19 | `axial_no_bath` | 7 | 0 | 36 | -18, -17, -3, -10, -18, -38, -12 |
| 23 | `axial_live` | 7 | 2 | 18 | 1, -5, -1, -19, -13, 1, -9 |
| 23 | `diagonal_live` | 6 | 0 | 29 | -16, 0, 0, 0, 0, 0 |
| 23 | `axial_no_bath` | 8 | 1 | 42 | 3, -42, -22, -66, -28, 0, -6, -22 |

Across qualified arms the exact integer margin has minimum/median/maximum
`-66 / -6 / 19`: six positive, nine ties, and 26 negative. Ties fail by
pre-registration. The empty controls have no pair observations and do not
enter the decision.

Because every comparison uses identical tick support and identical numbers of
lag-one transitions, this is not a threshold or normalization artifact. The
actual pairing simply does not possess unique sign persistence relative to
simultaneous endpoint derangement.

## 4. Parameter-free centrality result

For every consecutive pair history the campaign verifies the exact algebraic
midpoint identity

\[
\Delta\ell=\bar q\wedge\Delta p+\Delta q\wedge\bar p.
\]

All identity checks pass; the maximum identity residual is
`2.220446049250313e-15`. Thus the centrality failure is not a bookkeeping or
finite-difference identity error.

The two required central-map terms are then tested separately:

\[
T_p=\bar q\wedge\Delta p=0,
\qquad
T_q=\Delta q\wedge\bar p=0,
\qquad
\Delta\ell=0.
\]

Every one of the `15,191` live consecutive transitions fails exact
centrality. In the load-bearing no-bath sector, all 16 seeds qualify and all
`8,138/8,138` transitions fail; zero of 16 no-bath arms passes. Maximum
observed magnitudes are

```text
max |Delta ell| = 3.7834365792996576
max |T_p|       = 5.609592425877794
max |T_q|       = 3.5487240437416308
```

The no-bath result rules out blaming the failure on the imposed Langevin
thermostat. At one production tick and for the FTD-0907 endpoint projection,
the current wave/Gauss/genesis/coupling dynamics are not a radial central map.

This closes only the exact form class tested. It does not rule out a
coarse-grained stochastic effective law, a different observable, or a
topologically protected carrier.

## 5. Consequence for the “two sides” mechanism

The FTD-0907 two-endpoint construction contains the correct kinematic
ingredients:

- a durable actual `+/-` carrier;
- a native polar axis; and
- a time-odd clockwise/counterclockwise readout.

But two sides are not yet a recursive system. Recursion requires a closed
operation that returns the state to its own branch. In phase-space language,
that closure is the symmetry that makes the wedge invariant. Production has
no such symmetry here: endpoint reassignment leaves persistence essentially
unchanged, and every centrality transition fails.

So the pair should no longer be treated as the likely natural memory unit.
The next candidate must add a **barrier or invariant**, not another fitted
coefficient. The smallest structurally distinct route is a closed oriented
loop/plaquette or primal-dual circulation sector whose branch cannot flip
without a defect or zero crossing. That is where a geometric quarter-turn
`i`, left/right conjugacy, and self-dual energy can become native rather than
being imposed on two freely evolving endpoints.

## 6. Epistemic disposition

- **Retained:** FTD-0907's representation theorem and conditional central-
  Hamiltonian mathematics.
- **Measured and retained:** FTD-0908/0910 production carrier formation and
  finite sign-stable intervals.
- **Closed negative in scope:** actual endpoint pairing as the source of
  protected persistence under the current production stack.
- **Closed negative in scope:** the exact FTD-0907 central law as the current
  one-tick production law for this observable.
- **Forbidden next move:** perturbing this pair wedge and calling recovery a
  memory test.
- **Open next move:** derive a native closed-loop/topological orientation
  variable and its exact local conservation/defect law before any campaign.

No G*, clock-period, context, outcome, selector, or Born quantity was read.
No selected type or production law was added.

```text
FROZEN_OUTCOME=D
PROTOCOL_VALID=TRUE
INDEPENDENT_ADJUDICATION=20/20
PAIR_DISCRIMINATOR_QUALIFIED=TRUE
PAIR_SPECIFICITY=FAIL
EXACT_CENTRALITY_QUALIFIED=TRUE
EXACT_CENTRALITY=FAIL
MIDPOINT_IDENTITY_PASS=TRUE
NO_BATH_CENTRAL_FAILURES=8138/8138
PERTURBATION_CAMPAIGN_LICENSED=FALSE
PAIR_WEDGE_PROTECTED_MEMORY_CLOSED_NEGATIVE_IN_SCOPE=TRUE
EXACT_CENTRAL_PRODUCTION_LAW_CLOSED_NEGATIVE_IN_SCOPE=TRUE
LOOP_TOPOLOGICAL_CARRIER=OPEN
GSTAR_GEARBOX_IDENTIFIED=FALSE
BORN_OR_CONTEXT_READ=FALSE
PRODUCTION_TICK_MODIFIED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
```
