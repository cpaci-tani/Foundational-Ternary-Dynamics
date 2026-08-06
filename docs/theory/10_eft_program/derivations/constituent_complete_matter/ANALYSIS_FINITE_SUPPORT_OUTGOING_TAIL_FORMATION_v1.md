# Analysis — finite-support outgoing-tail formation v1

**Identifier:** FTD-0739  
**Status:** `[SELECTED DYNAMICS — CONSTRUCTIVE FINITE-SUPPORT CAUSAL FORMATION]`  
**Verdict:** `FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE`  
**Date:** 2026-07-29  
**Production status:** unchanged

## 1. Question answered

The earlier capture histories began with a quotient-wide Poisson dressing.
FTD-0739 replaced that field by an exactly Gauss-compatible face field with
support confined to the centered radius-four cube. It asked whether the same
selected reciprocal matter--current--field action would:

1. form a durable negative relational core on every registered cubic ray;
2. account for the capture time through pair-to-field energy transfer;
3. transport a source-free field tail through radius 12 before periodic
   self-contact; and
4. remain polarity-symmetric and state-only invertible.

All registered gates passed.

## 2. Run of record

| item | value |
|---|---:|
| volume | `L=145` |
| compact support radius | `R0=4` |
| forward horizon | `136` ticks |
| earliest periodic self-contact | tick `137` |
| histories | `5` |
| serialized states/roots | `1365` |
| unbound core passes | `4/4` |
| first-passage passes | `4/4` |
| outgoing-tail passes | `4/4` |
| bound controls | `1/1` |
| polarity scalar difference | `0` |

The Release run completed in approximately 64.6 minutes. It used the frozen
source and executable recorded by the pre-execution audit.

## 3. Causal chronology

| arm | graph transitions | final entry | first radius-12 tail | negative-tail onset | entry-to-onset delay |
|---|---|---:|---:|---:|---:|
| face `<001>` | `7;26;65` | `65` | `62` | `80` | `15` |
| edge `<01-1>` | `7;26;81` | `81` | `62` | `96` | `15` |
| body `<111>`, `+/-` | `7;26;100` | `100` | `62` | `115` | `15` |
| body `<111>`, `-/+` | `7;26;100` | `100` | `62` | `115` | `15` |

The tail reaches radius 12 before the final graph entry on every ray. All
deposited-current support remains at radius at most three. Therefore the
exterior signal at radius 12 is separated from the active source region and
arrives before the pair begins its final capture episode.

The same 15-tick final-entry-to-negative-tail delay previously measured with a
quotient-wide dress survives the compact preparation. This makes the delay
robust to that preparation change at the tested parameters. It does not prove
that 15 is universal or derive it from the action.

## 4. Energy transfer and outgoing field

| arm | initial pair energy | final pair energy | field-energy change | final outside energy at radius 12 |
|---|---:|---:|---:|---:|
| face | `+2.81683934678e-4` | `-1.28766374414e-3` | `+1.56934767881e-3` | `5.01490114683e-5` |
| edge | `+2.81683934678e-4` | `-9.21351505130e-4` | `+1.20303543981e-3` | `4.82055971527e-5` |
| body, either polarity | `+2.81683934678e-4` | `-4.97436709611e-4` | `+7.79120644292e-4` | `4.68642275771e-5` |

For each unbound arm,

\[
E_{\rm pair}(t)=E_{\rm pair}(t_e)
 -\big[E_{\rm field}(t)-E_{\rm field}(t_e)\big]
\]

predicts the first negative tick exactly. The maximum pointwise residual is
`6.993e-15`. The largest complete pair-plus-field endpoint defect is
`6.654e-15`.

Maximum radius-12 outside energy is `5.014901146831457e-5`, and maximum
cumulative outward transport is `5.014901146833996e-5`. The cumulative
outward sequence has no resolved negative increment larger than
`6.99e-18`, so no inward step is numerically resolved at this precision. That
monotonicity is a measured auxiliary observation, not an additional locked
gate.

The initially bound face control remains graph-inside and negative on all
forward and reverse stored states. Its small energy exchange has the opposite
sign—field to pair—but never releases the control.

## 5. Exactness and reversibility

| diagnostic | measured maximum | gate |
|---|---:|---:|
| common-action residual | `5.148e-14` | `1e-10` |
| total-energy residual | `3.119e-15` | `1e-8` |
| recoil defect | `2.686e-14` | `1e-9` |
| causal-speed excess | `0` | `1e-12` |
| regional-ledger residual | `3.000e-14` | `1e-10` |
| state-only inverse recovery | `2.636e-11` | `1e-8` |
| first-passage residual | `6.993e-15` | `1e-8` |

The two body-conjugate histories agree exactly across every persisted scalar
and discrete history field apart from the conjugated polarity label.

## 6. Independent certificate and hashes

[`proof_finite_support_outgoing_tail_formation.py`](../../../../scripts/proofs/proof_finite_support_outgoing_tail_formation.py)
independently reconstructs every transition, continuous negative onset,
entry-to-onset energy prediction, tail gate, bound control, polarity metric,
global maximum, count, and verdict from the CSV/JSON records.

Result: **`147/147 PASS`**.  
Proof SHA-256:
`1E52538EB505980182F19A0B2FDBB27A92F8AE12396F00099704D4907E3DE368`.

| result artifact | SHA-256 |
|---|---|
| CSV | `E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622` |
| JSON | `237F6EA3343BF6DA7C2E0979C5B77C4DD848EAF22FB79DF187A7C34055A19D5C` |

## 7. Ontological consequence

FTD-0739 closes M1 constructively for the selected `(s,C,F)` action:

> A finite-support, neutral, exactly Gauss-compatible preparation can evolve
> into a durable negative relational core while an outgoing source-separated
> field tail crosses an exterior shell before any possible periodic return.

The witness no longer depends on a quotient-wide tick-zero dress. It supports
the interpretation of matter-like formation as a local relational capture
transaction accompanied by energy export into the surrounding field.

This remains a selected classical ontology. The compact preparation, explicit
constituent phase space, compact pair interaction, and matched face/edge action
are not derived from the five postulates.

## 8. Boundary and next gate

The result does not establish:

- survival after tick 136 or after environmental return;
- a volume-independent bound-field plateau;
- an invariant or asymptotic basin;
- two-object composability;
- native formation from production genesis/reaction rules;
- charge, a particle pole, mass, spin, statistics, or Lorentz recovery.

M2 is now licensed. It must test environmental closure over increasing causal
buffers and horizons, distinguish bound dressing from the detached tail, and
measure whether any inward sustaining flux is required. Replaying the same
single volume for longer after periodic contact is not admissible M2 evidence.
