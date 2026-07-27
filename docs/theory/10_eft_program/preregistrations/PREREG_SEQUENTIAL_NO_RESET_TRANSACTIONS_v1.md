# PRE-REGISTRATION — Sequential no-reset local transactions v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0459`  
**Status:** `[EXECUTED — CLOSED NEGATIVE]`  
**Parents:** `FTD-0457`, `FTD-0458`  
**Engine artifact:** `engine/tests/campaign_sequential_no_reset_transactions.cpp`

**Locked artifact hashes (SHA-256):**

- campaign: `0ADA40BA4F1723C5F96A4EDE475549C3EAFAB476ED302C099900C7313EC36708`;
- coupled-tick observer: `E743457083D8A0296458E00D2783002862DD3E9C1C932A593788BED0561DA697`.

## 1. Question

Can the finite-packet, minimum-norm local transaction repeat from the actual
field left by prior events, with manifested-state coupling and production
movement cadence, or is it only a one-shot counterfactual that requires the
packet or bound dressing to be rebuilt before every hop?

## 2. Frozen initial state

- periodic `L=33` lattice;
- one `s=+1` manifestation at the central site;
- initial velocity `(0.15,0,0)`, production flat momentum, zero remainder;
- FTD-0457 discrete-curl packet: direction `+x`, six sites upstream,
  `sigma_x=sigma_t=3`, carrier `pi/4`, amplitude `0.02`;
- one initial minimal `+x` dressing normalized by the FTD-0457 source-free
  one-tick work convention to `1e-4`;
- no subsequent packet, dressing, field, momentum, or remainder reset.

## 3. Frozen 48-tick observer dynamics

Each tick executes:

1. the exact kick-drift `J/W` wave update including the production coupling
   source `-G_C grad(s)+G_C curl(s v)`;
2. production remainder accumulation `r += v`;
3. if `r_x>=1`, a candidate `+x` hop is scheduled;
4. work is measured from the actual post-wave endpoint divergences;
5. particle momentum is updated with the corrected production dispersion;
6. the exact `R=1` paired-impulse capacity is solved from the actual pre/post
   wave states;
7. when `E_min<=0`, execute the unique FTD-0458 minimum-norm zero-energy
   impulse, move only the ternary manifestation, update velocity/momentum, and
   subtract one from `r_x`;
8. when unavailable, veto the hop and retain the accumulated remainder.

At each scheduled attempt, also evaluate all 26 Moore neighbours from the same
pre-event history. This is diagnostic: production remainder selects the actual
`+x` channel, while the count reveals whether the transaction functional alone
would determine direction.

## 4. Frozen gates

- run exactly 48 ticks;
- require exactly one manifested site throughout;
- every executed event must have kinematic work residual, complete event-energy
  residual, momentum residual, selector certificate, support, and add/remove
  reversal residual `<=1e-10`;
- every non-event and event tick must be algebraically invertible;
- reverse the complete 48-tick history, including every impulse, state move,
  momentum update, and remainder update;
- require final reversal residuals for `J`, `W`, state, velocity, momentum, and
  remainder `<=1e-10`;
- all finite checks and Gram systems must remain valid.

The coupled tick's field-plus-interaction energy drift between events is
reported, not used as an acceptance gate, because the correct forced discrete
modified Hamiltonian has not yet been derived.

## 5. Locked classification

- `SEQUENTIAL_NO_RESET_TRANSACTIONS_SELF_SUSTAINING`: at least four scheduled
  forward hops execute and every registered gate passes;
- `SEQUENTIAL_TRANSACTIONS_STALL_AFTER_N`: one to three hops execute, then all
  later scheduled attempts are unavailable;
- `FIRST_TRANSACTION_UNAVAILABLE`: zero hops execute;
- `SEQUENTIAL_TRANSACTIONS_INTERMITTENT`: one to three execute but a later
  attempt recovers after at least one veto;
- `PROTOCOL_INVALID`: any closure, support, finiteness, manifestation-count, or
  full-history reversal gate fails.

## 6. Interpretation boundary

A positive result remains observer-constructed selected mechanics. It does not
modify production, prove the minimum-norm principle is ontic, or establish
edge/corner transport. A stall is a closed-negative result for this frozen
packet, amplitude, cadence, coupling, and no-reset protocol—not for all possible
localized matter packets.

## 7. Execution result

The locked Windows/MSVC CPU run completed with all validity and reversal gates
passing. It scheduled 42 attempts, executed zero, found zero eligible Moore
neighbours at every attempt, and returned `FIRST_TRANSACTION_UNAVAILABLE`.
See `docs/theory/07_assessment/AUDIT_SEQUENTIAL_NO_RESET_TRANSACTIONS.md`.
