# PREREG: Thomson Unlocked Recoil v1

**FTD ID:** FTD-0288
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED]
**Parent:** FTD-0287 Thomson recoil observatory
**Engine artifact:** `engine/tests/campaign_thomson_unlocked_recoil.cpp`
**Artifact SHA256:** `f43194598188bab303eecbdebcf99655118f90d2024279ed3a8a56607d864acc`
**Lock tag:** `preregister-thomson-unlocked-recoil-v1`

---

## 1. Question

FTD-0287 established that the visual dashboard's locked-charge plane-wave
scenario is a field observatory: the combined field equals `beam_only +
electron_only` to machine precision. It did not show mechanical recoil.

FTD-0288 asks the next narrower question:

```text
When the negative charge is unlocked, do the current production force paths
produce deterministic beam-induced recoil, and does an explicitly imposed
diagnostic qE electric hook respond in the expected transverse channel?
```

This is an engine-instrument campaign. It cannot derive alpha, a Thomson
cross-section, or a QED scattering amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_unlocked_recoil.cpp
```

Frozen source hash:

```text
f43194598188bab303eecbdebcf99655118f90d2024279ed3a8a56607d864acc
```

Any source edit after this lock requires a v2 or a new FTD ID.

---

## 3. Fixed Protocol

Shared setup:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 33` |
| Ticks | `200` |
| Plane-wave mode | `mode_n = 4` |
| Plane-wave amplitude | `0.05` |
| Particle | one negative charge at lattice center |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Machine repeat gate | `1e-12` |
| Recoil gate | `1e-8` |

Shared toggles:

| Toggle | Value |
|---|---|
| `wave_propagation` | `true` |
| `coupling` | `true` |
| `damping` | `false` |
| `genesis` | `false` |
| `gauss_projection` | `false` |
| `gravity` | `false` |
| `poisson_coulomb` | `false` |
| `lorentz_force` | `false` |
| `dual_substrate` | `false` |
| `pair_production` | `false` |
| `symmetric_movement_order` | `false` |
| `strict_validation` | `true` |

Arms:

| Arm | `forces` | `movement` | `emergent_forces` | Charge locked? | Meaning |
|---|---:|---:|---:|---:|---|
| `locked_linear_control` | false | false | false | true | FTD-0287-style locked control |
| `native_legacy_unlocked` | true | true | false | false | production legacy `grad(div J)` force path |
| `native_emergent_unlocked` | true | true | true | false | production EFT flux-gradient force path |
| `diagnostic_qE_unlocked` | false | true | false | false | [IMPOSED DIAGNOSTIC] applies `F = alpha * s * E`, `E = -wave_vel`, before each tick |

Each relevant mode runs:

- `electron_only`
- `electron_plus_beam`
- `electron_plus_beam_repeat`

The locked control also records `beam_only`.

Measured values:

- final unwrapped displacement plus sub-voxel remainder;
- final velocity and max speed;
- max native acceleration or diagnostic qE force;
- field, wave, particle kinetic, and total energy;
- Gauss audit and total Poynting vector;
- repeat deltas.

---

## 4. Outcomes

### Outcome L: `NATIVE_LEGACY_RECOIL_DETECTED`

Criteria:

- all finite gates pass;
- all repeat max-absolute deltas are `<= 1e-12`;
- `native_legacy_unlocked` has `electron_plus_beam - electron_only`
  displacement magnitude or velocity magnitude `> 1e-8`.

Interpretation:

The production legacy force path contains a deterministic beam-sensitive
mechanical response. This still is not a Thomson cross-section or alpha
derivation; it is a native engine response requiring follow-up mechanism audit.

### Outcome E: `NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED`

Criteria:

- Outcome L does not trigger;
- all finite and repeat gates pass;
- `native_emergent_unlocked` has `electron_plus_beam - electron_only`
  displacement magnitude or velocity magnitude `> 1e-8`.

Interpretation:

The EFT flux-gradient production path responds mechanically to the beam. This
is expected to be an intensity/flux-gradient style response unless separately
proven otherwise; do not call it Thomson scattering.

### Outcome Q: `NATIVE_NO_RECOIL_ELECTRIC_HOOK_REQUIRED_DIAGNOSTIC_QE_RESPONDS`

Criteria:

- Outcomes L and E do not trigger;
- all finite and repeat gates pass;
- the diagnostic qE arm has `electron_plus_beam - electron_only` displacement
  magnitude or velocity magnitude `> 1e-8`;
- the diagnostic displacement is transverse: `abs(disp_y) >
  10 * max(abs(disp_x), abs(disp_z), 1e-8)`.

Interpretation:

The production native force paths do not move the resting charge under this
beam, but an explicitly imposed electric hook does. This identifies a missing
engine ingredient for a Thomson-style recoil experiment. It is not a derivation.

### Outcome N: `NO_RECOIL_NATIVE_OR_DIAGNOSTIC`

Criteria:

- all finite and repeat gates pass;
- no native or diagnostic recoil gate triggers.

Interpretation:

The fixed setup is too weak or otherwise unsuitable for recoil observation.
Do not tune within v1; design a v2 with a declared reason.

### Outcome X: `NONFINITE_PROTOCOL`

Criteria:

- any finite gate fails, or the electron disappears in an electron arm.

Interpretation:

The protocol is invalid. No physics conclusion.

### Outcome R: `NONDETERMINISTIC_PROTOCOL`

Criteria:

- any repeat max-absolute delta exceeds `1e-12`.

Interpretation:

The protocol is invalid as a machine-precision measurement. No physics
conclusion until determinism is restored or the gate is revised in a v2.

---

## 5. Banned Moves

- No amplitude scan.
- No changing `L`, `ticks`, `mode_n`, `amp`, or gates after seeing output.
- No adding Poisson, Lorentz, damping, radiation, or Gauss-projection variants
  to v1 after seeing output.
- No claiming that diagnostic qE is native physics.
- No alpha, cross-section, or QED amplitude claim from this campaign.
- No substitution identity: inserting `alpha` into diagnostic qE is a declared
  imposed diagnostic, not a derivation.

---

## 6. Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_unlocked_recoil --parallel 24
```

Run of record, after this file is committed and tagged:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_unlocked_recoil$" --output-on-failure
```

---

## 7. Expected Significance

The classical Thomson picture needs an electromagnetic wave to exert a force on
a charge, causing charge motion and radiation. FTD-0287 showed the dashboard
instrument alone did not provide that mechanical loop. FTD-0288 tells us
whether the existing native engine force paths already contain an unlocked
beam-sensitive recoil channel, or whether the electric-force hook is still an
explicit missing ingredient for any honest Thomson-style alpha observable.
