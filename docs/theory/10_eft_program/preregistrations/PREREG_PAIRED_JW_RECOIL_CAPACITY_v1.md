# PRE-REGISTRATION — Paired J/W recoil capacity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0454`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0293`, `FTD-0438`, `FTD-0452`, `FTD-0453`  
**Engine artifact:** `engine/tests/campaign_paired_jw_recoil_capacity.cpp`  
**Campaign SHA256:** `4801c383d6f8193c5ef355b16ffc309fb4c0291e294ffbf40066a1d249de85c7`  
**Helper SHA256:** `c1ac49fb0ee222e3192b89d6b64f4e382e7b18c3edfd64327275beb5ed30aec5`

## 1. Question

Does the fixed-J obstruction in FTD-0453 disappear when the recoil source is
inserted in the production symplectic ordering, so that one impulse `S` changes
both members of the conjugate pair relative to the source-free control,

```text
W_event = W_control + S,
J_event = J_control + S?
```

The required transaction must close central field momentum and the combined
particle + interaction + exact modified tick energy.

## 2. Frozen control and event maps

Start from `W_old=0` and the registered minimal cubic `J_old` family. The
source-free control tick is

```text
W_c = C_WAVE^2 L J_old,
J_c = J_old + W_c.
```

The event arm adds an arbitrary whole-lattice impulse `S` to both fields and
moves charge `q=+1` from source to target. Particle energy increases by the
registered `W=1e-4` under the FTD-0450 selected map.

Each unit background is normalized after the control step so that

```text
G_C q [div(J_c)_target-div(J_c)_source] = 1e-4.
```

This is a deterministic normalization of the registered fixture, not a scan
or physical-constant fit.

## 3. Exact constrained quadratic

Periodic summation by parts gives

```text
Delta P_i = -sum_x S_x dot D_i(J_c-W_c)_x
          = -sum_x S_x dot D_i J_old_x
          = (A S)_i.
```

The wave-energy change is

```text
Delta E_tick = 0.5||S||^2 + c_wave dot S,
c_wave = W_c + 0.5 K J_c - 0.5 K W_c,
K = -C_WAVE^2 L.
```

The baseline particle `+W` and fixed-control hop interaction `-W` cancel.
The field-dependent remainder of the interaction Hamiltonian is

```text
-G_C q div(S)_target.
```

Therefore the complete event energy is again

```text
F(S)=0.5||S||^2+c dot S,
```

with an exactly known linear coefficient. The campaign analytically minimizes
`F` subject to `A S=R` over all `3*11^3=3993` impulse components.

If the minimum is non-positive, the campaign constructs a zero-energy solution
by adding the cubic-covariant null vector obtained by projecting `c` into
`ker(A)`. If the minimum is positive, no zero-energy solution exists anywhere
in the constraint space.

## 4. Frozen fixtures and gates

- periodic `L=11`, all 26 Moore directions, source at lattice centre;
- initial particle velocity `0.15 d/|d|`, work `1e-4`;
- unit minimal background shape from FTD-0453, normalized after the exact
  source-free control step;
- control-work residual `<=1e-12`;
- analytic versus direct event momentum residual `<=1e-12`;
- analytic versus direct complete-energy residual `<=1e-12`;
- reverse subtraction restores control `J`, `W`, momentum, and energy to
  `1e-12`;
- orbit spread of the analytic minimum `<=1e-12`;
- all systems nonsingular and all values finite.

Classification gate:

- positive obstruction: every minimum `>1e-8`;
- constructive closure: every minimum `<=0`, projected-null norm `>1e-12`,
  and the constructed zero-energy solution closes total energy and recoil to
  `1e-12`;
- otherwise mixed.

## 5. Locked outcomes

- `PAIRED_JW_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD`;
- `PAIRED_JW_ZERO_ENERGY_RECOIL_CONSTRUCTED`;
- `MIXED_PAIRED_JW_RECOIL_CAPACITY`;
- `CENTRAL_PAIRED_RECOIL_CONSTRAINT_UNREALIZABLE`;
- `PROTOCOL_INVALID`.

## 6. Interpretation boundary

The negative outcome closes the most general additive paired impulse for the
registered minimal, initially quiet field and selected central generator. It
does not exclude a pre-existing travelling-wave background, a non-additive
canonical transformation, another exactly conserved momentum, or a
13-channel extension.

No production tick is changed and no numerical search is performed.
