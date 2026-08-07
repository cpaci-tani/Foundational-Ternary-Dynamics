# AUDIT — Fixed-J recoil capacity

**Date:** 2026-07-24  
**Identifier:** `FTD-0453`  
**Status:** `[THEOREM — GLOBAL FIXED-J W-KICK MINIMUM]` + `[CLOSED NEGATIVE — ZERO-ENERGY RECOIL FOR REGISTERED MINIMAL FIELD]`  
**Verdict:** `FIXED_J_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD`  
**Pre-registration:** [`PREREG_FIXED_J_RECOIL_CAPACITY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_FIXED_J_RECOIL_CAPACITY_v1.md)  
**Run of record:** `engine/results/ftd_0453/windows_msvc_cpu.csv`

## 1. Result

FTD-0452 showed that particle work is already balanced by interaction energy,
so a recoil update must carry momentum without adding net wave tick energy.
FTD-0453 asked whether the existing site-centred `wave_vel` field can do this
at fixed `J`.

The test did not guess a local kick. It allowed all `3*11^3=3993` wave-velocity
components to participate and solved the exact constrained quadratic minimum

```text
min Delta E_tick subject to Delta P_field = R.
```

All 26 desired recoils lie in the central-gradient map's range. Momentum closes
to `1.53e-19`, endpoint work to `1.36e-20`, and the analytic energy minimum
agrees with direct evaluation to `8.33e-17`. The least possible wave-energy
increase is nevertheless strictly positive:

| Hop orbit | Arms | Minimum `Delta E_tick` | Cost / registered work |
|---|---:|---:|---:|
| face | 6 | `0.0804518749409` | `804.52` |
| edge | 12 | `0.160905691225` | `1609.06` |
| corner | 8 | `0.241359076100` | `2413.59` |

Every orbit has exactly zero internal spread. Applying the negative optimizer
restores `wave_vel`, central field momentum, and tick energy exactly.

## 2. Why this is a no-go, not a failed ansatz

At fixed `J`, let `u=Delta wave_vel`, let `A` be the linear map from `u` to the
central field-momentum change, and define

```text
b = wave_vel + 0.5 C_WAVE^2 L J.
```

Then

```text
Delta E_tick = 0.5 ||u||^2 + b dot u,
A u = R.
```

Completing the square and solving with `M=A A^T` gives the global minimum

```text
Delta E_min = 0.5 (R+A b)^T M^-1 (R+A b) - 0.5 ||b||^2.
```

Because this minimum is positive, no vector in the entire affine constraint
space has zero energy change. Enlarging the support from target, endpoints, or
one shell to the whole lattice cannot repair the registered fixture. The
obstruction is energetic capacity, not placement.

## 3. Intuitive reading

1. **Can the field point its momentum the right way?** Yes; direction is not
   the problem.
2. **Can distributing recoil over more sites make it free?** No; the global
   optimizer already used every site.
3. **Why is the cost so much larger than the hop work?** The minimal field is
   weakly graded, so producing the required canonical momentum needs a large
   conjugate-velocity change.
4. **Why do edge and corner costs scale upward?** The same work is divided
   among more directional gradients while the required particle recoil retains
   the same magnitude.
5. **Does exact reversal save conservation?** It proves the constructed update
   is algebraically reversible, but the forward state still has positive extra
   wave energy.
6. **Does this kill a flux-mediated particle?** No. It kills only a fixed-`J`,
   initially quiet, site-vector recoil realization for the registered minimal
   field.

## 4. What remains open

The production wave step never changes `wave_vel` in isolation: it advances a
conjugate pair and then writes `J'=J+wave_vel'`. The next native gate must allow
simultaneous `Delta J` and `Delta wave_vel`, recompute the interaction
Hamiltonian after the field change, and test the complete particle-plus-field
transaction. That is a nonlinear constrained problem because both the
momentum generator and hop work depend on `J`.

A pre-existing travelling-wave background can also provide negative linear
energy capacity through the `wave_vel dot u` term. That is a separate
conditional mechanism, not a rescue of the vacuum/minimal-field result.

Only if the simultaneous native pair fails is the 13-channel conjugate field
the next justified ontology extension.

## 5. Scope boundary

This result assumes:

- the FTD-0438 central translation generator;
- the FTD-0450 selected longitudinal particle branch;
- fixed `J` during the half-tick recoil;
- initially zero `wave_vel`;
- the preregistered minimal cubic work backgrounds.

It does not exclude other `J` backgrounds, simultaneous field-coordinate
updates, a different exactly conserved momentum, or hidden Moore-channel
degrees of freedom. No production dynamics were changed.

## 6. Reproducibility

- campaign SHA256: `2c06ffbdf76bb90bc694d21e4a3a6156238100c64e639902ed3bfbdb37b81470`
- helper SHA256: `ed8b3b25d316da627077bd3e64ccdceeb01ae1dd5b3937ad7ed082e271061c92`
- record SHA256: `48338c4809b94849d0c86442a0695d020c42f06f4755cfcba8d8fd5429fac556`
- compiler: pinned MSVC `14.44.35207`, Release
- focused result: `FIXED_J_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD`

