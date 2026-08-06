# AUDIT — Sequential no-reset local transactions

**Identifier:** `FTD-0459`  
**Date executed:** 2026-07-24  
**Status:** `[CLOSED NEGATIVE — FROZEN PROTOCOL]`  
**Preregistration:** `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_SEQUENTIAL_NO_RESET_TRANSACTIONS_v1.md`  
**Run of record:** `engine/results/ftd_0459/windows_msvc_cpu.csv`

## Result

The frozen packet-plus-manifestation history did not support even its first
scheduled transaction. Across ticks 6--47, the production remainder scheduled
42 forward attempts. None was executable. At every attempt, zero of all 26
Moore-neighbour candidates simultaneously passed production particle
kinematics and the exact local zero-energy recoil condition.

The locked classification is therefore:

`FIRST_TRANSACTION_UNAVAILABLE`

This is stronger than a one-hop stall for the registered initial state. The
source-free packet capacity established by FTD-0457 does not survive unchanged
when the production state coupling is applied before the event and the event is
scheduled by production movement cadence.

## Mechanism of failure

Two distinct gates failed at different phases:

1. The initial speed `0.15` gives production kinetic energy
   `0.00605718042874157`. At tick 6 the forward work was
   `-0.014433673011440832`, and at tick 7 it was
   `-0.007638417628295072`. Either requested update would place the particle
   below its rest-energy floor, so the particle update was invalid and no
   recoil problem was constructed. Zeroes printed in the minimum-energy column
   at such rows are uncomputed default values, not zero-cost solutions.
2. At phases where the particle update was valid, the constrained local field
   minimum remained positive. The clearest case is tick 8:
   `work=-0.001908912196713391` and
   `E_min=+0.00043527452795674455`. A local exact-energy shell therefore did
   not exist. Later valid phases were still more expensive, reaching
   `E_min=0.576556489842183` at tick 41.

The failure is consequently not explained by a missing arbitrary impulse
choice. The FTD-0458 minimum-norm selector is reached only after a zero-energy
shell exists; here no candidate shell exists.

## Closure and reversibility

The negative result is valid rather than a numerical breakdown:

- exactly one manifested site survived all 48 ticks;
- full-history `J/W` reversal residual was
  `1.4838095610461252e-17`;
- state mismatches, momentum residual, and remainder residual were zero;
- the largest reported coupled-wave energy step was
  `0.0092437880077365833`, diagnostic only under the preregistered boundary;
- all registered finite and reversal gates passed.

The coupled observer evolution is reversible even though its scheduled
manifestation move is unavailable. Reversibility alone therefore does not make
a propagating matter event.

## Ontological consequence

For this frozen construction, the flux packet is not a self-guiding pilot wave
for the manifestation. It propagates and is distorted by the state source, but
the state does not become phase-locked to a sequence of admissible local
transactions. The packet, one-time dressing, state coupling, particle
dispersion, and movement clock are mutually unsynchronized.

The exact closed-negative scope is the preregistered `L=33`, amplitude `0.02`,
initial speed `0.15`, packet offset six, one-time `1e-4` dressing, R1 support,
48-tick no-reset protocol. The result does not exclude a separately derived
bound travelling solution. It does exclude describing the already registered
packet as one.

## Next discriminating audit

The next non-search calculation must decompose forward hop work into the linear
packet, one-time dressing, static-polarity source, and velocity-curl source at
the same scheduled ticks. That decomposition will identify whether the block
is caused by packet phase, accumulated self-field, or the selected matter
dispersion before any new packet shape or parameter is admitted.
