# PRE-REGISTRATION — Fixed-J recoil capacity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0453`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0438`, `FTD-0447`, `FTD-0450`, `FTD-0452`  
**Engine artifact:** `engine/tests/campaign_fixed_j_recoil_capacity.cpp`  
**Campaign SHA256:** `2c06ffbdf76bb90bc694d21e4a3a6156238100c64e639902ed3bfbdb37b81470`  
**Helper SHA256:** `ed8b3b25d316da627077bd3e64ccdceeb01ae1dd5b3937ad7ed082e271061c92`

## 1. Question

For the minimal cubic-covariant fixed-`J` background that produces a registered
hop work, can any update of the existing site-centered `wave_vel` field—allowed
over the entire periodic lattice—carry the selected equal-and-opposite central
field momentum while changing the exact modified tick energy by zero?

## 2. Frozen variational problem

The central field momentum is

```text
P_i = -sum_x wave_vel_x dot D_i J_x.
```

At fixed `J`, write a proposed update as `u_x=Delta wave_vel_x`. Its momentum
constraint is linear:

```text
A u = R,  A_(i,x) = -D_i J_x,
```

where `R` is FTD-0451's selected recoil. The exact tick-energy change is

```text
Delta E_tick = 0.5 ||u||^2 + sum_x b_x dot u_x,
b_x = wave_vel_x + 0.5 C_WAVE^2 L J_x.
```

The campaign solves the global constrained minimum analytically. With
`M=A A^T` and `r'=R+A b`,

```text
min_(Au=R) Delta E_tick
  = 0.5 r'^T M^-1 r' - 0.5 ||b||^2.
```

If this minimum is strictly positive, no zero-energy recoil update exists,
even with nonlocal support. This is stronger than testing one guessed kick.

## 3. Frozen fixtures

- periodic `L=11`, initial `wave_vel=0`, source at the lattice centre;
- all 26 Moore displacements `d`;
- selected initial particle velocity `0.15 d/|d|`;
- registered positive work `W=1e-4`;
- for `k` nonzero components of `d`, set
  `g=W/(G_C k)` and place `J=2g a_i` at `target+a_i`, one signed unit vector
  `a_i=sign(d_i)e_i` per active axis; all other `J=0`;
- this gives `divJ(source)=0`, `divJ(target)=k g`, hence exact hop work `W`.

The optimizer may update `wave_vel` at every lattice site. `J`, state, the
production tick, work, and selected particle map remain fixed.

## 4. Frozen gates

- 26 valid fixtures and nonsingular `A A^T` systems;
- measured endpoint work residual `<=1e-12`;
- optimized field-momentum residual `<=1e-12`;
- direct modified tick-energy change agrees with the analytic minimum to
  `1e-12`;
- the minimum tick-energy change is greater than `1e-8` in every arm;
- applying the negative update restores `wave_vel`, field momentum, and tick
  energy to `1e-12`;
- within each face/edge/corner orbit, minimum-energy spread `<=1e-12`;
- all values finite.

## 5. Locked outcomes

- `FIXED_J_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD`: all gates pass.
- `FIXED_J_ZERO_ENERGY_RECOIL_EXISTS`: any valid arm has analytic minimum
  `<=1e-8` and a zero-energy solution is therefore not excluded.
- `CENTRAL_RECOIL_CONSTRAINT_UNREALIZABLE`: the desired recoil is outside the
  central-gradient map's range.
- `PROTOCOL_INVALID`: any algebraic, covariance, finiteness, or reversal gate
  fails.

## 6. Interpretation boundary

The first outcome is a no-go only for the registered minimal work background,
fixed `J`, the FTD-0438 central momentum generator, the FTD-0450 selected
particle branch, and updates of existing site-centered `wave_vel`. It does not
exclude simultaneous `Delta J`, a pre-existing wave background, a different
momentum generator, or a selected 13-channel extension.

No production dynamics are changed and no physical constant is fitted.
