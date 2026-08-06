# FTD-0776 — Native q_active Temporal Pilot v1

**Status:** `[ENGINE FACT — MEASURED, OBSERVABLE- AND CONFIGURATION-SCOPED NEGATIVE]` +
`[RETIRED — q_active CLOCK CANDIDATE IN THE LOCKED L=32/SEED-1/CPU-SOR PROFILE]` +
`[OPEN — AUTONOMOUS COUPLED MATTER--FIELD RECURRENCE]`  
**Verdict:**
`Q_ACTIVE_RECURRENCE_UNQUALIFIED_IN_LOCKED_L32_SEED1_CPU_SOR_PROFILE`  
**Production impact:** none; an observation-only target was built in an
isolated detached worktree and production physics source remained unchanged

## 1. Result in one sentence

All four raw-valid 200,000-tick arms produced zero complete cycles in the
preselected aggregate `q_active`, so recurrence failed before any quartic
occupancy, speed, moment, `G*`, or waveform diagnostic became admissible.

## 2. What was measured

The sole primary observable was

\[
q_{\rm active}(t)=\sum_{s_i\ne0}J_{i,x}(t).
\]

The experiment used the production state--flux operator at the
selected/parametric `G_C=sqrt(alpha)`, with wave propagation, Gauss projection,
and genesis/evaporation enabled. Langevin, imposed de Broglie/Coulomb clocking,
latency, dual substrate, forces, movement, and weak/gauge extensions were
disabled. This is a selected four-rule, single-substrate isolation profile,
not the engine's complete default toggle profile.

The frozen matrix was periodic `L=32`, seed `1`, CPU six-iteration SOR,
native `dt=1`, amplitudes `A={10,12,14,16}`, 2,000 transient ticks, and
200,000 recorded ticks per arm.

The effective profile also inherits the FULL 18-point stencil, periodic
mean-charge-subtracted six-pass approximate Gauss map, charge coupling `1`,
default selected genesis threshold/ramp, `kinetic_drain=0.5`, and effective
evaporation under the enabled genesis path. Injection is a one-time center
replacement, and CSV tick `0` is measured after engine tick 2,001. These are
scope constraints, not postulate-forced content.

## 3. Complete primary gate table

| amplitude | raw validity | raw positive crossings | complete cycles / required | recurrence | period/amplitude CV | occupancy/speed exponents | moments | correlated `G` functionals | waveform ratio |
|---:|---|---:|---:|---|---|---|---|---|---|
| 10 | PASS | 1 | 0/8 | **FAIL** | N/A | N/A | N/A | N/A | N/A |
| 12 | PASS | 1 | 0/8 | **FAIL** | N/A | N/A | N/A | N/A | N/A |
| 14 | PASS | 0 | 0/8 | **FAIL** | N/A | N/A | N/A | N/A | N/A |
| 16 | PASS | 0 | 0/8 | **FAIL** | N/A | N/A | N/A | N/A | N/A |

The analyzer's initial-boundary insertion rule fired in no arm. One crossing
does not define a complete cycle; two crossings are required to delimit one.

Descriptive controls did not alter the outcome. `q_all` also yielded zero
complete cycles. `q_center` yielded `0,0,1,7` accepted cycles after its
quality screen, still below eight in every arm and ineligible to replace the
primary observable.

## 4. Artifact and raw-data certificate

The complete ignored evidence bundle is
[`engine/results/gstar_qactive_pilot_20260802/`](../../../../engine/results/gstar_qactive_pilot_20260802/).
Its key files are the
[`result report`](../../../../engine/results/gstar_qactive_pilot_20260802/RESULT_REPORT.md),
[`gate table`](../../../../engine/results/gstar_qactive_pilot_20260802/gate_table.csv),
[`campaign certificate`](../../../../engine/results/gstar_qactive_pilot_20260802/campaign_certificate.json),
and [`artifact manifest`](../../../../engine/results/gstar_qactive_pilot_20260802/artifact_manifest.csv).
The post-run
[`execution-profile audit`](../../../../engine/results/gstar_qactive_pilot_20260802/EXECUTION_PROFILE_AUDIT.md)
records the inherited configuration details that further narrow the result.

| amplitude | raw SHA256 |
|---:|---|
| 10 | `694DE76F1F0186EBF7773D667C3738FB94857C344DB1D77475FBD4462C394609` |
| 12 | `FC0032E0FE1289FDBEFF0EDA9030A82CBEFF46262D9364CCA06522398CB977B4` |
| 14 | `EA8F4596858BE3AF60D6314A9EFC174AAC4670D0A4E3467D27D3317752E0BE53` |
| 16 | `0BECBDCC77001E6174B7294EF0880ED3D12ECB95815CDE8B5B0B0C0356ED0594` |

Every raw file has the exact registered header, 200,000 rows, contiguous ticks
`0..199999`, finite fields, and nonempty manifested support. The independent
verifier binds the transferred analyzer hash and arguments, all four primary
audit hashes, and reconstructs the decisive crossing count directly from the
raw CSVs. The manifest contains 68 artifact hashes; its CSV SHA256 is
`21DC65487839E98C5978A24EB43A8673559668064536F52C24DFD6FADA8343F6`.
The execution-profile audit SHA256 is
`8DB630D640F856B4B93DB48545D42EBE27B8B5F8821C6B00E6974D021163DA06`.

The immutable pre-run lock SHA256 is
`3FECCBCC92452DC7C066C6B7A594F65D9358A9E23464D667C7BCC77AD072662E`.
Its separate provenance erratum SHA256 is
`D7F1C0EB1F3B9FE15BAA3DBACCDC23BF618C9B7E093F5FB6A22F8B6655113BD7`.
The erratum discloses pre-lock timing probes and one quarantined 679-row
partial run; neither entered scientific analysis. There is no contemporaneous
analyzer-command transcript, so the certificate honestly identifies its
argument binding as post-run metadata reconstruction. Direct raw-data
reconstruction makes that limitation non-decisive.

## 5. Exact mathematics retained, not tested

The inherited continuous target is

\[
\rho_4(x)=\frac{2}{\sqrt\pi G^*\sqrt{1-x^4}},
\qquad G^*=\frac{\Gamma(1/4)}{\Gamma(3/4)},
\]

with registered moments

\[
\langle x^4\rangle=\frac13,\qquad
\langle x^6\rangle=\frac{12}{5G^{*2}},\qquad
\langle x^8\rangle=\frac5{21},
\]

and the selected quadratic-edge waveform ratio `48*pi/G*^4`.

These identities remain `[EXACT — CONTINUOUS QUARTIC IDENTITIES, UNCHANGED]`
under `[CONDITIONAL — BRANCH-REVERSIBLE FIXED UNIT-MASS NATURAL COORDINATE;
SELECTED QUADRATIC EDGE FOR B4]`. They were not reached or tested here.
Estimating them after recurrence failed would manufacture phase and
turning-amplitude normalizations.

## 6. What this establishes

`[ENGINE FACT]`: in the exact locked `L=32`, seed-1, single-substrate CPU/SOR
profile, the preselected aggregate `q_active` is recurrence-unqualified at all
four tested amplitudes. The candidate role of this observable is `[RETIRED]`
for this exact profile.

This is distinct from FTD-0772. FTD-0772 retrospectively tested the signed
fixed-ray FTD-0659 bare-doublet observer `Q_u` across 18 cells and found its
window return/stationarity unqualified. FTD-0776 is a prospective long-run
production-engine aggregate-observer pilot. Their common lesson is narrower
than a no-clock theorem: neither registered observable currently supplies the
autonomous coupled matter--field recurrence required by the quartic clock
program.

## 7. What this does not establish

- It is not a universal no-clock theorem.
- It does not reject quartic dynamics or alter the exact `G*` mathematics.
- It does not test other seeds, volumes, backends, SOR resolutions, couplings,
  observables, or complete-state recurrence.
- It does not derive or measure `G*`, phase, proper time, a minimum
  dimensionless `dt`, body recurrence, gauge structure, color, `SU(2)`, or
  `SU(3)`.
- It does not establish another aggregate as a clock.

The immutable lock's broader label
`NO_STABLE_NATIVE_CLOCK_IN_LOCKED_L32_SEED1_PROFILE` and the analyzer's
`NO_STABLE_NATIVE_CLOCK` are `[RETIRED]` as scientific conclusions and remain
only as provenance.

## 8. Stop rule and next admissible work

The first campaign did not pass, so no scale-rate, larger-volume, body,
monodromy, exchange, or minimum-`dt` campaign was run. The live question stays
`[OPEN — AUTONOMOUS COUPLED MATTER--FIELD RECURRENCE]`.

A future protocol must be written before new results are viewed and must
center complete-state recurrence/localization and coordinate closure, with
explicit seed, volume, backend, kinetic, and coupling ablations. Repeating
this aggregate-observable sweep or substituting a control after failure is not
licensed.

The adversarial reviews recorded here are AI-simulated audits, not external
human physics validation.
