# PRE-REGISTRATION — Native Observable Discovery (Closure Screen) v1

**Date locked:** 2026-08-03  
**Identifier:** `FTD-0779`  
**Status:** `[PRE-REGISTRATION — LOCKED / AWAITING EXECUTION]`  
**Parents:** `FTD-0659`, `FTD-0772`, `FTD-0776`, `FTD-0778`  
**Campaign type:** `[PROSPECTIVE OBSERVER-DISCOVERY SCREEN]` requiring one new
observation-only telemetry dump. Not a recurrence campaign; not a physics
claim.  
**Scope:** telemetry addition and observer analysis only. No engine state,
update rule, Hamiltonian, clock variable, phase variable, coupling,
calibration, tolerance, toggle, scenario, production path, or golden state may
change. Golden hash must be re-verified unchanged after the telemetry patch.

## 0. Question

> Does **any** admissible native channel behave as a natural coordinate —
> acceleration a single-valued function of position, constant effective mass,
> no dependence on unobserved state — in a fixed, preregistered profile?

FTD-0778 established that closure is logically prior to occupancy: the quartic
occupancy law and every `G*`-bearing moment are defined only in a fixed natural
coordinate, and `G*` is provably invisible to spectral data. It also found that
the three recorded channels (`q_active`, `q_all`, `q_center`) fail closure, and
diagnosed why: all three are **global extensive aggregates**, and `q_active` in
particular sums over an index set that grows from `3` to `32768`.

This campaign screens a preregistered set of candidate channels for closure
*before* any of them is granted a recurrence campaign.

## 1. The design rule this campaign exists to enforce

> **The summation index set must be fixed in advance and must not depend on
> engine state.**

`q_active` summed over "currently manifested voxels" — a state-dependent set
that saturated the lattice within `1.4%` of the run. Any candidate whose
support is defined by a dynamical predicate is inadmissible under this
preregistration, regardless of how natural it looks.

Two further admissibility rules follow from FTD-0778's diagnosis:

- **No unbounded extensive sums.** A channel scaling with the number of active
  degrees of freedom tracks population, not phase.
- **Locality or mode-resolution required.** Candidates must be local (bounded
  fixed support), a projection onto a fixed mode, or a body-frame observable.

## 2. Required telemetry [OBSERVATION-ONLY]

A new default-off dumper, gated behind a toggle that is OFF in every production
and golden path. Per tick, for a fixed preregistered configuration:

1. **Fixed-site channels.** For a preregistered fixed list of `N_site = 8` site
   indices chosen by geometry alone (not by activity): the site state, flux
   components, and wave-velocity components. Sites are named in §3 before any
   run.
2. **Fixed-block channels.** For a preregistered fixed cubic block of side `4`
   centred on the seed: the block-summed flux components. Support is fixed and
   bounded; it does not follow manifestation.
3. **Fixed low-`k` modes.** The real and imaginary parts of the spatial Fourier
   coefficients of `J_x` at the preregistered wavevectors
   `k in {(1,0,0), (1,1,0), (1,1,1), (2,0,0)} * 2pi/L`. Fixed basis, chosen
   before the run.
4. **Covariates.** `active_count` and total energy, recorded for diagnosis but
   carrying no decision weight.

Sampling: every tick for at least `2 x 10^5` ticks, matching the FTD-0776
cadence so the two corpora are comparable.

## 3. Locked configuration and candidate register

- Lattice `L = 32`, seed `1`, CPU/SOR profile, Langevin and imposed de Broglie
  phase and latency disabled — identical to the FTD-0776 profile so that the
  only changed variable is the observable.
- Amplitudes `A in {10, 12, 14, 16}`, matching FTD-0776.
- **Candidate count is fixed at `N_cand = 8 + 1 + 4 = 13` channels per arm**
  (eight sites, one block, four modes), each screened in both its coordinate
  and its conjugate. The exact site index list and mode list are written into
  this file before execution and may not be extended afterward.

Site list (geometric, fixed): the seed voxel, its six axial nearest
neighbours, and the body-diagonal neighbour at `(+1,+1,+1)`.

## 4. Metrics — identical to FTD-0778, reused without modification

`P0` momentum consistency; `M1` positional closure `R^2`; `M2` phase-plane
closure `R^2`; `M3` stationarity of `F`; `N1` noise control (upper half-band
power and lag-1 autocorrelation of the acceleration). Acceleration computed two
independent ways and required to agree; Savitzky–Golay variant as noise
control. Bin counts, drop rules, and the `0.95` threshold are inherited
verbatim from `PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md`.

## 5. Look-elsewhere protection [MANDATORY]

Screening `N_cand = 13` channels per arm across `4` arms is `52` tests. A
single channel crossing `R^2 > 0.95` is therefore **not** by itself a pass.

- **Full-table reporting.** The complete `52`-row screen table is reported
  regardless of outcome. Selective reporting of a winner is prohibited.
- **Concordance requirement.** A channel qualifies only if it passes in
  **at least three of the four amplitude arms**.
- **Held-out validation.** Any channel passing §5's concordance requirement
  must then be re-screened on an independent seed (`seed = 2`) at the same
  configuration, declared here in advance. Failure on the held-out seed
  demotes the channel to `[CANDIDATE — NOT CONFIRMED]`.
- **No post-hoc channels.** A channel not in the §3 register may not be added
  after inspection, in this campaign or in a v1 successor that reuses this
  corpus.

## 6. Permitted verdicts

- `NATIVE_OBSERVABLE_DISCOVERY_CLOSURE_FOUND` — at least one registered channel
  passes concordance and held-out validation.
- `NATIVE_OBSERVABLE_DISCOVERY_NO_CLOSURE` — no registered channel passes.
- `NATIVE_OBSERVABLE_DISCOVERY_UNINFORMATIVE` — telemetry or noise controls
  fail; the screen is void.

## 7. What each outcome licenses

**On `CLOSURE_FOUND`:** a single downstream entitlement — the qualifying
channel becomes eligible for a Gate B (quarticity) campaign under a *separate*
preregistration. It licenses no claim about quarticity, `G*`, occupancy,
minimum `dt`, or native time. In particular, closure is necessary and not
sufficient for a clock: a channel may close on a non-quartic potential, or on
a quartic one whose amplitude family is too narrow to test.

**On `NO_CLOSURE`:** an `[ENGINE FACT]` scoped to the registered channel set
and profile. It does **not** establish that the substrate lacks a natural
coordinate. The indicated next step would be body-frame observables from the
Phase 3 body-tracking layer, which are outside this campaign because that layer
is not built.

**On either:** no production change, no toggle promotion, no scenario, and no
supersession of FTD-0772, FTD-0776, or FTD-0778.

## 8. Cost and ordering rationale

The screen itself is seconds of analysis per channel on a completed dump; the
dump is one campaign's worth of wall time. FTD-0778 showed that omitting this
step cost four `~26`-minute runs whose verdict, correctly read, was a statement
about the observable rather than about the substrate. Gate A is placed first
precisely because it is the cheap discriminator.
