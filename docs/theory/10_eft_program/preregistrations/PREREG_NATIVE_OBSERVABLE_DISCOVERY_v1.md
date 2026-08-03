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
- **Recurrence precondition (added after the second re-audit).** Closure is
  testable only on records that revisit coordinate values: a monotone
  (single-pass) record satisfies `qddot = F(q)` vacuously via `t = t(q)`, and
  its stationarity metric is uncomputable. A channel whose detrended record
  does not revisit values is `UNINFORMATIVE` by construction, not `FAILED`.
- **Fold-license rule (added 2026-08-03).** A quotient `q -> q mod L` is an
  admissible coordinate move **iff the dynamics is equivariant under
  `q -> q + L`**: the quotient map is then a local isometry, constant mass
  survives, and single-valued `F` descends. The lattice torus licenses this
  natively for *position-type* observables, whose monotone records are the
  unwrapped lifts of rotors. For flux-type zero modes (`V = 0`, periodic under
  every `L`) the wrap is dynamically licensed but the circumference is a **free
  selection** that sets the rotor frequency `Omega = p/L` — the same
  answer-in-the-premise failure mode as the retired Quarter Principle's scale
  `s`. Amplitude-space dyadic root-lifting is a nonlinear relabeling outside
  the affine class and is **inadmissible** (its recorded fingerprint: FTD-0777's
  lifted harmonic carrier retains arcsine fourth moment `3/8`, not quartic
  `1/3`). Winding-register folding (mod `2^K` on the cycle count) is affine on
  the angle and admissible, but presupposes a cycling carrier and adds no
  state (FTD-0777, binding) — it is a readout layer downstream of GATE A, not
  a route to it.

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
cadence so the two corpora are comparable. **Additionally, and non-negotiably,
the dump must resolve at least 20 ticks per clock cycle of the candidate mode**
(added 2026-08-03 per FTD-0780: the FTD-0659 corpus samples at 2.887 ticks per
cycle, which makes finite-difference accelerations carry order-unity error and
renders Gate A uncomputable on it). If a candidate's frequency is not known in
advance, a short pilot must establish it before the full dump is scheduled.

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

**Existing-corpus note (2026-08-03).** The FTD-0776 corpus already contains one
member of this admissible class: `q_center = J_x(L/2, L/2, L/2)`, the x-flux at
the single central voxel (provenance: `RESULT_REPORT.md`, where it was a
descriptive control). It is flux-type (no torus wrap applies) but bounded and
recurrent, so the recurrence precondition holds without a fold. An
**exploratory, post-hoc** v2 screen (not preregistered; carries no verdict
weight) gives, across arms `A={10,12,14,16}`: `M1 = 0.418/0.216/0.146/0.207`
against the `0.90` closed threshold, with surrogate-null excess
`0.346/0.191/0.136/0.188`. Reading: **not closed as one degree of freedom, and
not noise** — a deterministic phase-locked superposition, consistent with the
`640-660` spectral lines needed for 80% of its variance. A single fixed voxel
sees the whole normal-mode bath; this motivates the fixed low-`k` **mode**
channels of §2 item 3, which project one mode instead of summing all of them.

## 4. Metrics — identical to FTD-0778, reused without modification

**The v1 metrics are RETIRED** (FTD-0778 §3.1: degenerate on drift, self-term bias
floor at `R^2 = 2/3` on white noise, and a noise guard that could not fire). Use
`scripts/experiments/closure_metrics_v2.py`: detrend before binning, true second
difference, and `M1_excess` over a phase-randomised surrogate null. `P0` is retired
entirely as circular. The band-fraction null is `0.5`, not `1.0`. Acceleration computed two
independent ways and required to agree; Savitzky–Golay variant as noise
control. Bin counts, drop rules, and the `0.95` threshold are inherited
verbatim from `PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md`.

## 5. Look-elsewhere protection [MANDATORY]

Screening `N_cand = 13` channels per arm, **each in both its coordinate and its
conjugate**, across `4` arms is `13 x 2 x 4 = 104` tests — not 52, as an earlier
revision stated. A single channel crossing threshold is therefore **not** by itself a
pass.

**Correction 2026-08-03.** Concordance across the four arms is a *robustness* check on
correlated replicates (same seed, profile and lattice; only amplitude differs), **not a
multiplicity correction**. The held-out `seed = 2` re-screen is the only genuine
control. The threshold must additionally be calibrated against a surrogate null before
use — see `scripts/experiments/closure_metrics_v2.py`, which reports excess over a
phase-randomised null rather than a raw `R^2`.

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

## Addendum 2026-08-03 — carrier corner: quantitative targets for the pre-test

Three additions from the cornering analysis (sidebranch §32), none altering the locked
configuration or metrics:

1. **Primary falsifier, per mode: the sign of `dOmega/dA`.** The carrier class requires
   hardening anharmonicity so the fundamental can climb above the single acoustic band.
   A measured softening sign across the registered channels closes the carrier question
   negatively for this sector. This is the cheapest decision the dump can deliver and
   should be evaluated first.
2. **Quantitative target for GATE C: the flow curve.** For a mixed
   quadratic-plus-quartic potential, `G_meas = (1/A) sqrt(6 pi I/(mu Omega))` flows
   monotonically from `sqrt(3 pi) = 3.06998` (harmonic endpoint) to `G* = 2.95868`
   (quartic endpoint) as a function of `a = A sqrt(lambda/mu)/omega_0`, reaching 50%
   of span near `a ~ 1` and 98% near `a ~ 8`. The test is curve collapse across the
   amplitude family with the single fitted scale `A_*`, not the endpoint number. A
   harmonic mode must sit at the top of the curve (the FTD-0659 doublet does).
3. **Regime tension, flagged for a v2.** This preregistration deliberately matches the
   FTD-0776 configuration, but that campaign's centre injections thermalised (lattice
   saturation within 1.4% of the run) with no localisation at any tested amplitude —
   consistent with softening nonlinearity or sub-threshold amplitude. If the present
   screen returns `NO_CLOSURE` with a hardening sign, the registered follow-up is an
   amplitude scan upward toward `A_c = G* omega_B/(2 sqrt pi)`, as a separate locked
   preregistration, not an edit to this one.
