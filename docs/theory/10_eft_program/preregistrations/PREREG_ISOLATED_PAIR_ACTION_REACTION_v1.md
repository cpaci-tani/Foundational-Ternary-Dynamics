# PRE-REGISTRATION — Isolated-pair action-reaction mirror v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0437`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** `FTD-0436` neutral-pair wave response  
**Engine artifact:** `engine/tests/campaign_isolated_pair_action_reaction.cpp`  
**Artifact SHA256:** `783f186187b33e07ad467b4f4a54ce8ef78010e1d752099a7245d2035a674c6a`

## 1. Question

The registered wave-free controls in FTD-0436 showed both signs of an isolated
neutral pair translating together by `0.628381254469` sites along their pair
axis while maintaining separation. FTD-0437 determines the symmetry source:

> Does the center-of-mass drift reverse with dipole orientation, remain fixed
> with lattice scan orientation, or depend on particle injection order?

Any resolved center-of-mass acceleration from a resting isolated pair closes
action-reaction balance for the selected force in this protocol. The mirror
classifies the defect; it does not repair it.

## 2. Frozen matrix

| Quantity | Value |
|---|---:|
| lattice | periodic `L=33` |
| ticks | `200` |
| pair separation | `8` |
| axes | `x,y,z` |
| polarity orientation `+` | `+1` at low coordinate, `-1` at high coordinate |
| polarity orientation `-` | `-1` at low coordinate, `+1` at high coordinate |
| injection orders | positive-first, negative-first |
| RNG seed | `4370` |
| repeat arm | axis `y`, orientation `+`, positive-first |
| repeat gate | `1e-12` |
| motion gate | `1e-8` |

The 12 primary arms are the Cartesian product of three axes, two polarity
orientations, and two injection orders. No incident wave is injected. Enabled
production terms are exactly `wave_propagation`, `coupling`, `forces`,
`movement`, and `emergent_forces`, with `strict_validation=true`. All other
Boolean extensions are disabled.

## 3. Estimators

For equal inertial masses,

$$
d_{CM}=\frac{d_++d_-}{2},\qquad
d_P=\frac{d_+-d_-}{2}.
$$

At each tick the production `f_coulomb` diagnostics give

$$
F_{net}=F_++F_-.
$$

The campaign reports center-of-mass displacement, half-relative displacement,
RMS and integrated net force, minimum separation, both particle IDs, and the
current accounted dynamic energy.

For each axis and injection order, let `C_+` and `C_-` be the signed center-of-
mass components along that axis for the two polarity orientations. Define

$$
r_{odd}=\frac{|C_++C_-|}{|C_+|+|C_-|},\qquad
r_{even}=\frac{|C_+-C_-|}{|C_+|+|C_-|}.
$$

Injection-order residual is the maximum absolute difference between otherwise
identical positive-first and negative-first trajectory/force summaries.

## 4. Locked outcomes

All outcomes require finite values, both IDs surviving all 200 ticks, exact
repeat residual at or below `1e-12`, and unchanged registered toggles/backend.

### Outcome Z — `ACTION_REACTION_BALANCED`

Every center-of-mass displacement magnitude and RMS net force is at or below
`1e-8`. The selected pair force is balanced in this protocol.

### Outcome D — `DIPOLE_ORIENTED_SELF_PROPULSION`

- motion exceeds `1e-8` on every axis;
- `r_odd<=0.10` on every axis and injection order;
- injection-order residual is at or below `1e-12`.

The isolated pair self-propels along its oriented polarity axis. Cubic symmetry
may hold, but internal action-reaction balance is closed negative.

### Outcome S — `SCAN_ORIENTATION_SELF_ACCELERATION`

- motion exceeds `1e-8` on every axis;
- `r_even<=0.10` on every axis and injection order;
- injection-order residual is at or below `1e-12`.

The drift is polarity-even and follows a lattice/update orientation rather than
the physical dipole orientation. Action-reaction and discrete symmetry are
closed negative in this protocol.

### Outcome I — `INJECTION_ORDER_DEPENDENT_RESPONSE`

The injection-order residual exceeds `1e-12`. Particle construction history
leaks into the two-body mechanics; the protocol closes permutation invariance
negative and no cleaner action-reaction classification is licensed.

### Outcome M — `MIXED_SELF_ACCELERATION`

Resolved motion exists but none of Z/D/S/I passes. Action-reaction remains
closed negative; the defect has mixed axis/orientation structure.

### Outcome A — `PAIR_ANNIHILATION_OR_LOSS`

Either tracked ID disappears. This is a valid movement outcome, but the
200-tick action-reaction estimator is inadmissible.

### Outcome X — `INVALID_PROTOCOL`

Any nonfinite value, incomplete force history, forbidden toggle/backend
violation, or repeat residual above `1e-12` invalidates the campaign.

## 5. Banned moves

- No force, wave, binding, separation, duration, gate, or orientation changes
  after first execution.
- No symmetrized movement order may be enabled.
- No action-reaction claim may be rescued by subtracting the observed isolated
  center-of-mass motion.
- No electric, atomic, photon, gauge, or conservation claim follows from cubic
  rotation of a nonzero self-propulsion defect.
