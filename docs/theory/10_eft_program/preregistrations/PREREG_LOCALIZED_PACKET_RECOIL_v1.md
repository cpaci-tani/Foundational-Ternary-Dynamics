# PRE-REGISTRATION — Localized-packet recoil gate v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0457`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0456`  
**Engine artifact:** `engine/tests/campaign_localized_packet_recoil.cpp`

**Locked SHA-256:**

- campaign: `4D97BFB60875CE476B9DA1C3C59C851FFA7596362ABE9B9690D1EA0BD46AD026`
- packet helper: `8AA0E4DBE189D2EADD277F43A5E7652D8459663F463B0B7654CA623CF02F64BA`

## 1. Question

Can a finite-energy localized transverse flux packet enable the exact `R=1`
zero-energy recoil transaction with a threshold energy stable as the periodic
box grows, or did FTD-0456 require a box-filling background?

## 2. Frozen packet and event

- face hop `d=(+1,0,0)`, `q=+1`, speed `0.15`, work `1e-4`;
- `R=1` union of source/target Chebyshev balls, 36 sites;
- volumes `L in {33,49,65}`;
- packet directions `sigma in {-1,+1}`;
- source-free sample ticks `{0,8,16}`;
- packet center initially six sites upstream of the link in its direction;
- discrete-curl transverse packet copied from the canonical scenario logic:
  `sigma_x=sigma_t=3`, carrier `k0=pi/4`, carrier phase zero;
- packet axis displaced by one transverse width so the manifested link crosses
  a field lobe rather than the curl core;
- packet amplitude bracket `[0,1]`, 80 bisection iterations.

The packet field is the centered discrete curl of `psi e_x`, and its conjugate
field uses the production kick-drift one-way phase

```text
W = -sigma C_SPEED D_x J - (C_SPEED^2/2) Laplacian(J).
```

It evolves only under the exact source-free production wave map. At each sample
tick the same minimal bound dressing used by FTD-0456 is superposed before the
counterfactual hop observer is evaluated.

## 3. Frozen measurements and gates

For each of the six `(L,direction)` families:

- require at least one sampled tick to bracket the constrained minimum from
  `>1e-8` at amplitude zero to `<-1e-8` at amplitude one;
- threshold analytic minimum, direct complete energy, momentum, and outside-
  support residuals must pass `1e-10`;
- measured hop work must pass `1e-12`;
- initial discrete divergence must be `<=1e-12`;
- unit-packet exact tick-energy drift over 16 forward ticks must be `<=1e-10`;
- 16 inverse ticks must recover initial `J/W` to `<=1e-10`;
- after the selected local event and eight source-free ticks, at least `5%` of
  the event-control difference norm must lie outside the original `R=1` mask;
- reversing those eight ticks, removing the impulse, and reversing the control
  tick must recover the pre-event field to `<=1e-10`.

For each family select the sampled tick with the lowest threshold packet
energy. Report all sampled arms, the selected energy, direction, volume,
outgoing fraction, and reversal residual.

## 4. Locked volume-stability rule

Across the six selected family minima:

- coefficient of variation of unit packet energy `<=1%`;
- coefficient of variation of threshold packet energy `<=10%`;
- relative difference of the mean selected threshold energy between `L=49`
  and `L=65` `<=5%`.

These tolerances test box stability, not agreement with an external constant.

## 5. Locked classification

- `LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_STABLE`: all six families cross and
  every residual, propagation, reversal, and stability gate passes;
- `LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_UNSTABLE`: all six cross and algebraic
  gates pass, but at least one volume-stability gate fails;
- `NO_LOCALIZED_PACKET_R1_THRESHOLD`: no family crosses;
- `MIXED_LOCALIZED_PACKET_R1_THRESHOLD`: only some families cross;
- `PROTOCOL_INVALID`: a registered algebraic, divergence, energy, support,
  outgoing-residue, or reversal gate fails.

## 6. Interpretation boundary

A positive verdict establishes conditional local phase-space capacity for one
finite packet family. It does not make the optimizer a production rule, derive
manifestation, establish repeated particle motion, or prove a quantum photon.
No production dynamics are changed and no numerical near-match search is run.

## 7. Recorded outcome

All 18 sampled arms and all six volume/direction families crossed. Selected
threshold packet energies are volume-identical between `L=49` and `L=65` at
printed precision. Unit-energy CV is `2.62e-15`, selected threshold-energy CV is
`5.11%`, and the high-volume relative difference is zero. Every divergence,
energy, momentum, outgoing-residue, and reversal gate passed.

**Verdict:** `LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_STABLE`.
