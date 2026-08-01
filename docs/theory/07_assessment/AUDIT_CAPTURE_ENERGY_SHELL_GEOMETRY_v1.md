# Audit — FTD-0733 capture-energy shell geometry v1

**Status:** `[AUDIT PASS — SELECTED-POTENTIAL DOMAIN THEOREM CERTIFIED]`  
**Date:** 2026-07-29

## Findings

1. The exact derivative factors as
   `-48 D (d-3/2)(d-1)`, fixing one monotone root on each side of `d=1` for
   every `0<K<D`.
2. The strict negative-energy domain is the connected interval between those
   roots; it is not the entire compact graph support `d<3/2`.
3. The interval shrinks monotonically with instantaneous kinetic energy and
   disappears at `K=D`.
4. All 84 persisted FTD-0732 records reproduce their initialized/rejected
   classification from this theorem. Exactly the six inward-position probes
   lie outside their own fixed-kinetic intervals.
5. `radial_impulse_plus` is independently recovered as the maximum kinetic
   variant in all 12 volume-direction-polarity groups.
6. Every parent lies inside the interval associated with that maximum kinetic
   level, but the body-diagonal inward scale margin is only
   `7.50e-4` (`L=33`) and `7.68e-4` (`L=65`).
7. A symmetric raw-coordinate perturbation box is therefore badly conditioned
   against the actual energy domain. This is a domain-parameterization defect,
   not evidence for hidden state.
8. The theorem belongs to the selected compact potential. It is not a derived
   particle size, hard core, stability basin, or physical force law.

## Correct statement

The FTD-0732 captured parents possess a nonempty local negative-energy domain,
but position and momentum margins are coupled. A finite mixed-corner survival
test must remain inside the kinetic-dependent interval before its evolution
can diagnose stability.

## Verification

- protocol `E4C639DC…E26DB`;
- FTD-0732 source CSV `15926F9E…E2AD`;
- certificate `0574272D…812C`, `654/654 PASS`;
- no production state, dynamics, defaults, toggles, or scenarios changed.
