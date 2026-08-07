# ANALYSIS: Thomson Moving-Recoil Source/Work Accounting v1

**FTD ID:** FTD-0297
**Status:** [MEASUREMENT -- SUBVOXEL RECOIL ACCOUNTED BY ADDITIVE SOURCE WORK]
**Pre-registration:** `preregister-thomson-moving-recoil-accounting-v1`
**Lock commit:** `0ba544f5`
**Engine artifact:** `engine/tests/campaign_thomson_moving_recoil_accounting.cpp`
**Artifact SHA256:** `aae604ea897943102273f89b819735283804474d26a2a531f769835dc46f5c89`
**Run command:** `ctest --test-dir engine/build -C Release -R "^thomson_moving_recoil_accounting$" --output-on-failure`

---

## 1. Verdict

```text
SUBVOXEL_RECOIL_ACCOUNTED_BY_ADDITIVE_SOURCE_WORK
```

The native emergent unlocked arm shows deterministic extra recoil, but the
particle never makes an integer lattice hop during the 200-tick protocol:

```text
native_emergent_extra_disp_mag = 0.21022031950099582
native_emergent_extra_vel_mag  = 0.0018928965812042473
native_emergent_transport_events = 0
```

With no integer transport event, the field after the full tick is still
accounted by the additive `phase_read`/`phase_write` source/work law:

```text
emergent_plus_max_abs_balance       = 4.64038529823795897755e-16
emergent_plus_max_scale_rel_balance = 2.25693826280852869163e-16
```

No alpha, Thomson cross-section, radiation, or QED amplitude claim is promoted.

---

## 2. Frozen Gates

The run-of-record used the FTD-0297 pre-registered artifact and commands.
CTest passed:

```text
1/1 Test #236: thomson_moving_recoil_accounting ... Passed
```

Gate summary:

```text
finite              = true
deterministic       = true
locked_accounting   = true
legacy_accounting   = true
emergent_accounting = true
legacy_recoil       = false
emergent_recoil     = true
any_transport       = false
```

All repeat deltas were exactly zero under the frozen `1e-12` determinism gate.

---

## 3. Balance Results

The locked fixed-source control reproduces FTD-0296:

```text
locked_plus_max_abs_work              = 0.00912169070541427734478
locked_plus_max_abs_balance           = 4.42910139987484630097e-16
locked_plus_max_scale_rel_balance     = 2.20455860816464694277e-16
```

The native legacy unlocked arm remains motionless to numerical noise and also
closes:

```text
legacy_extra_disp_mag                 = 1.6797692127359577e-17
legacy_plus_max_abs_balance           = 4.45607092891542322377e-16
legacy_plus_max_scale_rel_balance     = 2.12705459459635864206e-16
legacy_plus_transport_events          = 0
```

The native emergent unlocked arm recoils but stays subvoxel:

```text
emergent_plus_disp_x                  = -0.18043717939662737
emergent_plus_disp_y                  =  0.1078656800692025
emergent_plus_disp_z                  =  4.5669447980500365e-05
emergent_plus_speed                   =  0.0018928965812042464
emergent_plus_max_accel               =  0.00069962658185837427
emergent_plus_transport_events        = 0
emergent_plus_max_abs_work            = 0.00912169072396147584403
emergent_plus_max_abs_balance         = 4.64038529823795897755e-16
emergent_plus_max_scale_rel_balance   = 2.25693826280852869163e-16
```

---

## 4. Interpretation

FTD-0297 narrows the moving-source problem.

The unlocked emergent charge responds to the flux-gradient force, but for the
fixed 200-tick Thomson setup that response is subvoxel: the engine updates
velocity and remainder, while the manifested state remains at the original
lattice site. Since no integer hop occurs, `phase_movement` does not carry
self-field flux to another voxel. The full-tick field balance therefore closes
with the same additive source/work law already derived for `phase_read` and
`phase_write`.

This is stronger than FTD-0296 as a measurement, but narrower than a theorem:
it confirms the fixed unlocked-recoil protocol in the no-transport branch. It
does not yet account for the field-energy effect of an actual integer hop.

---

## 5. Open Boundary

Still open:

```text
Integer-transport recoil work:
when phase_movement moves state and carries self-field flux to a target voxel,
derive the additional finite-volume work/transport term.
```

The next honest campaign should force or wait for an integer transport event
under a pre-registered protocol, then measure whether the residual equals a
local flux-carry expression from `phase_movement`.

---

## 6. Non-Claims

This result does not:

- derive `alpha`;
- measure a Thomson cross-section;
- establish radiation;
- compute a QED amplitude;
- prove the full moving-source theorem;
- scan for a favorable amplitude, mode, lattice size, or gate.

It says only that the fixed FTD-0297 unlocked emergent recoil remains subvoxel
and is accounted by the additive source/work identity to machine precision.
