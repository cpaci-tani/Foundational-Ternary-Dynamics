# FTD-0662 — Internal-mode action-transfer ledger v3

**Status:** `[SELECTED DYNAMICS — CONSTRUCTIVE DYNAMIC-FIELD TRANSFER]`  
**Verdict:** `INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE`  
**Production impact:** none

## Result

V3 makes the tight-frame sum genuinely basis invariant by normalizing each
quadratic ledger history by its own initial doublet energy. It changes no
dynamics, physical threshold, shell, amplitude, duration, or transfer gate.

All 34 fresh histories pass.

| diagnostic | value | gate |
|---|---:|---:|
| minimum doublet-energy ratio | `0.4242551` | `<=0.60` |
| minimum maximum residual-norm/excitation ratio | `0.8369634` | `>=0.05` |
| dynamic residual field energy / initial excitation at tick 128 | `0.57573` | measured |
| minimum maximum far-shell fraction | `0.8526253` | `>=0.10` |
| shell onset order | `5 <= 10--11 <= 35` | near <= middle <= far |
| amplitude residual | `0.0014799` | `<=0.05` |
| sign residual | `0.0014028` | `<=0.05` |
| normalized tight-frame covariance | `0.0007051` | `<=0.05` |
| zero-observer residual | `2.776e-17` | `<=1e-14` |
| complete-energy drift | `2.665e-15` | `<=1e-12` |
| field-decomposition residual | `7.750e-16` | `<=1e-12` |
| inverse recovery | `2.676e-11` | `<=1e-10` |

In a representative arm, excitation starts entirely as constituent kinetic
energy `6.8941e-11`. By tick 128 the kinetic remainder is `5.09e-13`, binding
holds `2.8654e-11`, and the actual field holds `3.9778e-11`, of which
`3.9691e-11` is dynamic residual rather than instantaneous dressing. The
complete sum is unchanged.

## Ontological consequence

The selected composite behaves as a localized bound structure coupled to a
propagating substrate field. Exciting an internal constituent coordinate does
not merely animate a passive aura: energy leaves the core coordinate, appears
in binding plus a dynamic face/edge residual, and spreads from near to middle
to far shells. The full state remains deterministic, exactly energy
conserving at engine resolution, and state-only reversible.

This supplies a concrete reversible mechanism for effective matter decay or
radiative damping: a localized subsystem can lose observable amplitude by
dispersing information and energy into field degrees of freedom without a
fundamentally many-to-one update.

The result does not yet establish asymptotic radiation, a photon, irreversible
decay, a lifetime, or an infinite-volume resonance. Periodic return and volume
scaling remain untested. Stable matter still requires a protected complete
matter--field mode or nonlinear/topological invariant.
