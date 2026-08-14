# PREREG — Contextual Born Pushforward Recovery v1

**Status:** `[PRE-REGISTRATION — DESIGN LOCKED; NOT EXECUTED]`  
**Date locked:** 2026-08-09 · **Ledger:** FTD-0825  
**Parent:** `SPEC_CONTEXTUAL_ACTUALIZATION_TIME_v1.md`  
**Scope:** physical recovery campaign; no reference-selector result counts as
substrate evidence.

## 1. Question and epistemic ceiling

Can an admissible substrate preparation class generate, without reading a
target probability vector, both an invariant equilibrium measure
`mu_eq(lambda)` and a deterministic context-complete pushforward satisfying

`(Sigma_C)_* mu_eq(o) = omega(E_o^C)`

on held-out contexts while preserving measurement independence and operational
no-signalling?

The candidate density is the existing free-flux positive-frequency occupation
`n = |phi_+|^2`. The first campaign is free-sector only. Any interacting or
genesis claim requires a separately locked extension. Success can establish a
measured bridge for the tested preparation class; it cannot by itself derive
the adopted potentiality algebra or the universal Born rule.

## 2. Frozen separation of roles

- The preparation pipeline may read substrate histories, local instrument
  descriptions, and preregistered clock-compliance data.
- The G* clock may decide **when** a batch is eligible. It may not read outcome
  labels, effect weights, remote settings, or selector state.
- The candidate selector may decide **which** joint record occurs. It may not
  read `omega(E)`, target frequencies, fitted cumulative weights, or any table
  derived from them.
- The imposed quantile selector in the reference model is a positive control
  for logical compatibility only and is ineligible for physical promotion.

Any direct or indirect target-probability read is **target leakage** and forces
Outcome D regardless of numerical agreement.

## 3. Data split and contexts

Before execution, the runner amendment must freeze a finite context family,
the preparation ensemble, seeds, sample count, and a deterministic split into:

1. construction contexts, used only to define substrate observables;
2. validation contexts, used to reject non-normalized or signalling models;
3. held-out contexts, unavailable to construction and used for the verdict.

The Bell reference family must include parallel, orthogonal, and the four CHSH
angles. The singlet and its observables remain `[SELECTED/IMPORTED]`; they are
not outputs of the campaign.

## 4. Required controls

1. **Reference positive:** imposed quantile selector reproduces supplied
   weights; reported as `REFERENCE`, never physical evidence.
2. **Classical control:** a locally factorized model gives `|S| <= 2`.
3. **Target-leak sentinel:** permuting hidden target tables must not change any
   substrate-produced weight or outcome stream.
4. **Amplitude, energy, and Rice controls:** the registered competitors to
   `|phi_+|^2` are fit and scored on the same held-out records.
5. **Context-independence audit:** the empirical distribution of `lambda`
   must be invariant across randomly assigned admissible settings.
6. **No-signalling audit:** every local marginal is compared across remote
   settings with family-wise uncertainty reported.
7. **Clock blindness:** phase, detuning, work, dissipation, and eligibility
   distributions are tested for dependence on settings and outcomes.
8. **Harmonic-clock control:** replace the quartic clock by a rate-matched
   harmonic clock without changing the selector pipeline.

## 5. Primary statistics

- Held-out total-variation distance between substrate frequencies and the
  state-effect targets, with multinomial confidence intervals fixed by the
  frozen sample count.
- Proper scoring-rule difference against amplitude, energy, and Rice controls.
- Maximum no-signalling marginal drift with simultaneous confidence bounds.
- Setting-prediction advantage from `lambda`; zero advantage is the
  measurement-independence target.
- CHSH value with the classical control and imported Tsirelson ceiling shown
  together.
- Clock-vs-harmonic difference in calibrated held-out performance, maintenance
  work, and perturbation recovery.

No post-hoc parameter search, near-miss scan, or formula-substitution score is
permitted. Numerical gates must be fixed in the runner amendment from declared
sampling error and instrument tolerance before the first physical run.

## 6. Pre-blessed outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — HELD-OUT RECOVERY** | all validity gates pass; `|phi_+|^2` wins the frozen held-out comparison; measurement independence and no-signalling pass | bridge promoted only for the tested preparation class; interacting/genesis extension may be preregistered |
| **B — CONTROL WEIGHTING** | a preregistered amplitude, energy, or Rice control wins | Born candidate fails in this regime; report winning control without reinterpretation |
| **C — CLOCK-NEUTRAL** | Born bridge passes but quartic and harmonic clocks are operationally equivalent after rate matching | G* is period normalization/gate cadence only; no distinct actualization role |
| **D — INVALID / CLOSED NEGATIVE** | target leakage, signalling, setting dependence, inconsistent preparation restrictions, or unhidden preferred order | archive branch as closed negative under the constitution stop rule |
| **E — INDETERMINATE** | valid run lacks power or rivals overlap under the frozen decision rule | no promotion; redesign must be separately locked |

## 7. Execution lock

No physical run is authorized by this document alone. Before execution, an
append-only amendment must record runner paths and SHA-256 hashes, engine build
identity, seeds, context split, sample count, and all numerical gates. Changing
any of those after first execution creates a new preregistration version.

This v1 record deliberately contains no result section.
