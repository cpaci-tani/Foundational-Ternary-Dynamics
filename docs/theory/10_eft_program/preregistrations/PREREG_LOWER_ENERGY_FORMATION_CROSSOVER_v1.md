# FTD-0724 — Lower-energy formation crossover v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0724`  
**Date:** 2026-07-29  
**Parents:** `FTD-0722`, `FTD-0723`  
**Scope:** observer-only validation of the post-FTD-0723 lower-energy
extrapolation under the unchanged derived compact-pair/matched-field action;
no production state, default, toggle, scenario, interaction, field
normalization, tolerance, or ontology promotion.

## 1. Question locked before validation

Does the approximately momentum-linear field export measured by FTD-0723
continue far enough below `p=0.0200` to cross the exactly quadratic
low-momentum pair-energy ledger and leave a one-pass encounter in the
negative-energy sector?

The primary target is the pair-energy sign after a complete encounter.
Qualified capture additionally requires the unchanged detached-field
morphology gate. A negative-energy result without that morphology is not
labelled radiation or complete formation.

## 2. Parent lock and descriptive model

The unchanged action and immediately preceding campaign are identified by:

- connected-action header SHA-256
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0723 runner SHA-256
  `05AA224853D3CF4219002975102901C04E0C3E036EFCDA5BC80061E6DDA307E7`;
- FTD-0723 JSON SHA-256
  `E785C1061CD715B64414DD4685F80DDF2BC4C9A047B1EA2FB124834F45D38895`;
- FTD-0723 CSV SHA-256
  `6B8B3CE2EB93E6DC3AD7977B0CC388DB2218C026235EC3E2E681842E5C3F60F5`.

Across the five FTD-0723 momentum means, the post-run zero-intercept model is

\[
\Delta E_f(p)=a p,
\qquad a_{\rm mean}=0.016403955899417445.
\]

Applying the same construction to the minimum and maximum export envelopes
gives

\[
a_{\min}=0.015179885973080108,
\qquad
a_{\max}=0.017974058483949684.
\]

Equating these to the exact outside-support pair energy gives directional-fit
crossings `0.0077582625283910--0.0091869698888720` and mean crossing
`0.0083841135239819`. These are model predictions, not confidence intervals
and not derived asymptotics.

## 3. Frozen momenta and predictions

| momentum | exact initial pair energy | linear export envelope | locked prediction |
|---:|---:|---:|---|
| `0.0060` | `0.000070442814789407` | `0.000091079--0.000107844` | negative sector in 52/52 arms |
| `0.0075` | `0.000110060499050713` | `0.000113849--0.000134805` | negative sector in 52/52; held-out interior |
| `0.0085` | `0.000141360103700250` | `0.000129029--0.000152779` | transition band; no sign prediction |
| `0.0095` | `0.000176568723315951` | `0.000144209--0.000170754` | positive escape in 52/52; held-out exterior |
| `0.0120` | `0.000281683934678478` | `0.000182159--0.000215689` | positive escape in 52/52 arms |

No adaptive bisection, replacement momentum, or refit is allowed after output.

## 4. Locked campaign

Retain the FTD-0722/0723 action and geometry exactly, with one preregistered
horizon change required by the slower approach:

- `L=33`, `dt=1/4`, **48** forward and 48 state-only reverse steps;
- separation `1.30` for every unbound arm;
- all 13 unoriented Moore rays, both polarity orders, and the lattice-center
  and translated `(4,-3,2)` copies;
- minimum-energy periodic longitudinal initial face field, `B=0`, CG tolerance
  `1e-13`, at most 4096 iterations;
- canonical FTD-0468/0479 interaction normalization;
- exact quadratic-coat currents and matched face/edge field update;
- selected FTD-0721 compact well with depth `0.01` and squared cutoff `1.5`;
- common-action gate `1e-10`, reverse recovery `1e-8`, scalar-history
  translation/polarity gate `1e-9`, symmetric recoil gate `1e-9`.

The five unbound momenta give 260 histories. Repeat 52 already-bound controls
at separation `1.00`, momentum `0.015`, for the same 48-step horizon. Total:
312 complete forward/reverse histories. At field speed `1/sqrt(3)`, the
48-step causal radius is below half the periodic box, so the observation
horizon does not wrap a newly generated disturbance around the volume.

## 5. Unchanged classifiers

Record `negative_sector` when the arm is inside the compact graph and below
`-1e-6` for the final eight steps. An unbound arm is `captured` only if it also
starts outside and positive, enters and never exits, balances field gain
against pair loss within `1e-8`, has dynamic-field norm `>1e-8`, magnetic
energy `>1e-10`, and dynamic-field median doubled radius at least four.

Already-bound controls must remain inside and negative for the final eight
steps. For each momentum report negative-sector and capture fractions, graph
transitions, energy export, final pair energy, dynamic-field norm, magnetic
energy, and median radius. Both negative-sector and capture fractions must be
nonincreasing with incident momentum.

## 6. Locked verdict map

- All atomic gates and bound controls pass; `p=0.0060` and held-out `0.0075`
  reach the negative sector in 52/52 arms; `p=0.0095` and `0.0120` remain
  positive in 52/52 arms: `PREDICTED_LOWER_ENERGY_CROSSOVER_CONFIRMED`. The
  `0.0085` result and qualified-capture count are reported but do not choose
  this verdict.
- Atomic gates and controls pass; at least one arm reaches the negative sector
  and negative fractions are monotone, but the locked low/high predictions are
  not all satisfied: `LOWER_ENERGY_CROSSOVER_SHIFTED_OR_DIRECTIONAL`.
- No unbound arm reaches the negative sector:
  `NO_LOWER_ENERGY_CROSSOVER_OBSERVED_LOCKED_V1`.
- Negative-sector or capture fractions are nonmonotone:
  `NONMONOTONE_LOWER_ENERGY_RESPONSE`.
- Any already-bound control fails:
  `DERIVED_PAIR_BOUND_STATE_UNSTABLE_AT_48_TICKS`.
- Any root, current, Gauss, common-action, energy, inverse, symmetry, or recoil
  gate fails: `LOWER_ENERGY_TRANSACTION_UNRESOLVED`.

A confirmed negative-energy crossover without the unchanged outgoing-field
gate is recorded as energetic trapping, not qualified detached-field capture.
Even a qualified capture remains a selected classical formation witness, not
a physical particle, quantum bound state, derived electromagnetic law, or
production adoption.
