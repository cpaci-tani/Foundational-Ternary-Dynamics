# PREREG: Alpha No-Alpha Engine Probe v1

**FTD ID:** FTD-0285  
**Date:** 2026-06-13  
**Status:** [PRE-REGISTRATION -- LOCK PENDING]  
**Parent:** FTD-0284 alpha dynamical readout discriminator  
**Engine artifact:** `engine/tests/campaign_alpha_no_alpha_probe.cpp`  
**Artifact SHA256:** `883a917358077f626e90f5affacabd8e565f48fd1cc00aa775ac6e4c7ffbdade`  
**Intended tag:** `preregister-alpha-no-alpha-engine-probe-v1`

---

## 1. Question

Does the live FTD engine produce the master-quadratic alpha normalization from
no-alpha-input dynamics, or does it produce only the unit geometric Coulomb
response already identified by Phase G?

This is not a search for a near miss. It is a fixed discriminator following
FTD-0284.

---

## 2. Frozen Artifact

The only run-of-record artifact is:

```text
engine/tests/campaign_alpha_no_alpha_probe.cpp
```

Frozen source hash:

```text
883a917358077f626e90f5affacabd8e565f48fd1cc00aa775ac6e4c7ffbdade
```

Any source edit after this pre-registration requires a new FTD ID or a v2
pre-registration. Build-system edits that do not alter the artifact source may
be documented, but the run-of-record verdict must cite the artifact hash above.

---

## 3. Fixed Protocol

The executable freezes:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 32` |
| Separations | `r = {5, 7, 9}` |
| Ticks per configuration | `300` |
| SOR iterations | `100` |
| Backend | CPU (`RenderBridge::force_cpu()`) |
| Native source coupling | `coulomb_charge_coupling = 1.0` |
| Tolerance | `10%` relative |

No finite-size window may be changed after the lock. No extra lattice sizes may
be added to rescue the verdict.

---

## 4. Leak Guard

The native arm must use:

```text
toggles.coupling = false
toggles.gauss_projection = true
toggles.coulomb_charge_coupling = 1.0
toggles.forces = false
toggles.poisson_coulomb = false
toggles.lorentz_force = false
toggles.damping = false
toggles.langevin = false
toggles.de_broglie_clock = false
toggles.db_clock_coulomb = false
```

The purpose is to remove all direct alpha paths known from the current engine:

- `toggles.coupling = true` injects `G_C = sqrt(alpha)` through `phase_read`.
- `damping` and several derived physics toggles consume alpha-valued engine
  constants.
- `poisson_coulomb`, `forces`, and `lorentz_force` are not part of the native
  Gauss-projection Coulomb readout.

The native arm may use the lattice Green-function prediction
`alpha_r(r,L) = 2 r G_L(r)` because that is the Phase-G zero-parameter
geometric theorem, not an alpha input.

---

## 5. Arms

### Arm A: native unit response

```text
coulomb_charge_coupling = 1.0
classification = NO_ALPHA_INPUT
```

Measured quantity:

```text
g_dyn^2(r) = alpha_r(r,L) / (2 r G_L(r))
```

Expected native-null result:

```text
mean(g_dyn^2) = 1 within 10%
```

### Arm B: Postulate-W positive control

```text
g_W^2 = 2*pi*ALPHA_TREE = 2*pi/x_plus
coulomb_charge_coupling = g_W
classification = EXPLICIT_MASTER_QUADRATIC_MATCHING_INPUT
```

This arm is a control only. If it passes, it shows that the engine can reproduce
the alpha normalization once the matching coupling is inserted. It is not a
derivation of alpha.

Expected control result:

```text
mean(g_dyn^2) = g_W^2 within 10%
```

---

## 6. Outcomes

### Outcome N: `NATIVE_NULL_WITH_POSTULATE_W_CONTROL`

Criteria:

- Arm A matches `mean(g_dyn^2) = 1` within tolerance.
- Arm B matches `mean(g_dyn^2) = g_W^2` within tolerance.

Interpretation:

FTD's engine-native no-alpha response is the geometric unit response. Recovering
`1/x_plus` requires a separate matching choice `g_W^2 = 2*pi/x_plus`. This
supports the FTD-0284 `NATIVE-NULL` branch and leaves FTD-0013 unchanged as a
strongly motivated conjectural identification, not a dynamical derivation.

### Outcome D: `DYNAMICAL_FOUND_CANDIDATE_REQUIRES_LEAK_AUDIT`

Criteria:

- Arm A does not match unit response.
- Arm A instead matches `g_W^2 = 2*pi/x_plus` within tolerance.
- Arm B passes.

Interpretation:

This would be a candidate dynamical alpha result. It is not automatically a
promotion. It must first survive an alpha-leak audit and a larger v2 blinded
replication.

### Outcome I: `INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT`

Criteria:

- Arm B fails, or the executable cannot distinguish the control from native
  response.

Interpretation:

The measurement artifact or current engine wiring is not suitable for the alpha
question. Do not promote or demote the alpha claim from this run.

### Outcome U: `UNCLASSIFIED_NATIVE_RESPONSE`

Criteria:

- Arm B passes, but Arm A matches neither unit response nor `g_W^2`.

Interpretation:

The simple FTD-0284 outcome tree is incomplete. Record the result, run no
post-hoc search, and author a new v2 pre-registration before further probing.

---

## 7. Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_alpha_no_alpha_probe --parallel 24
```

Run of record, after this file is committed and tagged:

```sh
ctest --test-dir engine/build -C Release -R "^alpha_no_alpha_probe$" --output-on-failure
```

The run is not valid unless `git rev-list -n1 preregister-alpha-no-alpha-engine-probe-v1`
resolves before execution.

---

## 8. Banned Moves

- No numerical search over couplings.
- No changing `L`, `r`, ticks, SOR, tolerance, or toggles after seeing output.
- No substituting `ALPHA`, `G_C`, or CODATA into the native arm.
- No calling Arm B a derivation.
- No continuum extrapolation claim from this single finite-cell probe.
- No promotion of FTD-0013 without a follow-up leak audit and v2 replication.

---

## 9. Prior

Prior-favored outcome: Outcome N.

Reason: Phase G already says the engine-native Coulomb response is geometric,
and FTD-0284's static contract shows the unit geometric normalization differs
from `1/x_plus` by the factor `21.809984...`.

This prior does not relax the other outcomes. If Outcome D fires, it is recorded
as written.
