# PRE-REGISTRATION — Common-action kick reciprocity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0468`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0467`  
**Engine artifact:** `engine/tests/campaign_common_action_kick_reciprocity.cpp`

**Locked campaign SHA-256:**
`5FAF98C9D0C1183C0DA6D9482DF39F6A5A0B77F886664B501467221A85AE502C`

## 1. Question

Does the existing native coupling kick already carry the exact field recoil of
the matter force obtained from the same interaction?

For the central field momentum

`P_i^field = -sum_x W(x) dot D_i J(x)`,

the stationary electric source kick is

`Delta W = -G_C grad(s)`.

At fixed `J`, the common-action matter impulse is

`I_i^matter = G_C sum_x s(x) D_i div(J)(x)`.

Because the periodic central derivatives are skew-adjoint and commute, the
registered hypothesis is the exact finite-lattice identity

`Delta P^field + I^matter = 0`.

## 2. Frozen fixtures

No production tick or force is changed.

1. **Static single-polarity controls:** `L=17`, signs `+/-`, axes `x/y/z`,
   one center polarity, and `J_i=a r_i^2` with fixed `a=1e-3`.
2. **Static pair controls:** `L=17`, each axis and both polarity
   orientations, separation six, and `J_i=a r_i^3`. These fixtures make the
   summed matter impulse nonzero rather than testing only a symmetric zero.
3. **Evolving native histories:** `L=33`, each axis and both opposite-pair
   orientations, separation eight, 64 source-free/coupling wave ticks, plus a
   fixed axis-aligned longitudinal travelling mode (`n=2`, amplitude `0.02`,
   phase `0.37`). Forces, movement, damping, Gauss projection, reactions, and
   genesis remain disabled. At every snapshot the shadow source kick is
   evaluated on the actual `J` history.

The static fixtures include a fixed deterministic nonzero `W`; the identity
must be independent of the pre-kick field momentum. No amplitude, separation,
tick, support, or tolerance scan is permitted.

## 3. Observer transaction

For each snapshot:

1. compute the complete common-action matter impulse from the current `s,J`;
2. copy the state and add `-G_C grad(s)` to `W` at every site;
3. measure the central field-momentum change directly;
4. subtract the same source kick and require exact field recovery;
5. count the support and require that every changed site is a face neighbor of
   at least one manifested polarity.

This is the exact electric source component already present in production,
not a fitted recoil or a minimum-norm optimizer.

## 4. Gates

- every static record has nonzero finite matter impulse above `1e-14`, and
  every evolving arm has impulse RMS above `1e-8`;
- `|Delta P^field + I^matter| <= 1e-12` on every record;
- direct field inverse residual `<=1e-12`;
- source support is face-local with zero change elsewhere;
- polarity reversal is odd and axis rotations reproduce the registered axial
  values to `1e-12` in the static controls;
- a direct algebraic evaluation of the summation-by-parts pairing agrees with
  the measured momentum change to `1e-12`.

## 5. Locked classifications

- `COMMON_ACTION_KICK_MOMENTUM_RECIPROCITY_EXACT`: every gate passes;
- `COMMON_ACTION_KICK_RECIPROCITY_FAILS`: protocol controls pass but at least
  one nontrivial record fails momentum closure;
- `PROTOCOL_INVALID`: any finiteness, nontriviality, inverse, locality,
  covariance, or formula-replay control fails.

## 6. Interpretation boundary

Success proves only instantaneous momentum reciprocity of the stationary
electric source kick and its common-action matter force. It does not prove
full-tick energy conservation, a finite-hop event, continuous source motion,
transverse photon guidance, or production stability. Failure would show that
even the written interaction cannot use the current `J/W` variables as a
recoil channel. The production tick remains frozen.

## 7. Execution record

All 12 static records and all 384 evolving-history snapshots close. Worst
momentum residual is `4.08e-15`, worst direct formula residual is `4.08e-15`,
and worst inverse residual is `6.94e-18`. Support is exactly six face sites for
one polarity and twelve for the separated pair, with zero update outside.
Locked verdict:

`COMMON_ACTION_KICK_MOMENTUM_RECIPROCITY_EXACT`.
