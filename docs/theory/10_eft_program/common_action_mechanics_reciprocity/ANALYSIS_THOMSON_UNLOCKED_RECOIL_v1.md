# ANALYSIS: Thomson Unlocked Recoil v1

**FTD ID:** FTD-0288
**Status:** [MEASUREMENT -- NATIVE EMERGENT FLUX-GRADIENT RECOIL]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_UNLOCKED_RECOIL_v1.md`
**Lock commit:** `7260b274590ba2c7e4cae81336023aefd58c1acc`
**Lock tag:** `preregister-thomson-unlocked-recoil-v1`
**Artifact:** `engine/tests/campaign_thomson_unlocked_recoil.cpp`
**Artifact SHA256:** `f43194598188bab303eecbdebcf99655118f90d2024279ed3a8a56607d864acc`
**Dashboard scenario:** `s0-field-thomson-unlocked-recoil`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_unlocked_recoil$" --output-on-failure
```

CTest result:

```text
Test #227: thomson_unlocked_recoil ..........   Passed    1.64 sec
100% tests passed, 0 tests failed out of 1
```

The full console payload was recovered from
`engine/build/Testing/Temporary/LastTest.log`.

---

## 2. Protocol

Shared setup:

- Lattice: `L=33`
- Ticks: `200`
- Incoming beam: y-polarized plane wave along +x
- Mode: `n=4`
- Amplitude: `0.05`
- Charge: one negative electron-like voxel at the center
- Active field terms: `wave_propagation=true`, `coupling=true`
- Disabled common terms: `damping`, `genesis`, `gauss_projection`, `gravity`,
  `poisson_coulomb`, `lorentz_force`, `dual_substrate`, `pair_production`

Arms:

- `locked_linear_control`
- `native_legacy_unlocked`
- `native_emergent_unlocked`
- `diagnostic_qE_unlocked`

The diagnostic qE arm is explicitly [IMPOSED DIAGNOSTIC]. It applies
`F = alpha * s * E`, `E = -wave_vel`, before each tick. It exists only to show
whether an electric-force hook would move the charge in this same setup.

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_speed,0.57735026918962573,alpha,0.0072973525643314245,machine_gate,9.9999999999999998e-13,recoil_gate,1e-08
scope,native_recoil_measurement_not_alpha_derivation
ingredients,wave_propagation,true,coupling,true,damping,false,genesis,false,gauss_projection,false,gravity,false,poisson_coulomb,false,lorentz_force,false,diagnostic_qE,imposed_not_native
delta,native_legacy_extra_plus_minus_electron,disp_x,2.5871603996905208e-18,disp_y,1.3144673663748675e-17,disp_z,-1.0133440489063084e-17,vel_x,1.0127111044660188e-19,vel_y,-1.186770825546359e-20,vel_z,-1.7722444328155218e-19,disp_mag,1.6797692127359577e-17,vel_mag,2.0446315953437857e-19,max_abs,1.6797692127359577e-17,finite,true
delta,native_emergent_extra_plus_minus_electron,disp_x,-0.18043717939662743,disp_y,0.10786568006920259,disp_z,4.5669447980497945e-05,vel_x,-0.0018604192733977756,vel_y,0.00034913711126628038,vel_z,9.3372450815704385e-07,disp_mag,0.21022031950099582,vel_mag,0.0018928965812042473,max_abs,0.21022031950099582,finite,true
delta,diagnostic_qE_extra_plus_minus_electron,disp_x,3.8372365224856828e-19,disp_y,-0.0025053078788877881,disp_z,2.3507615965361283e-20,vel_x,4.8522448779499502e-20,vel_y,0.00033387238237809596,vel_z,6.9367720078440123e-20,disp_mag,0.0025053078788877881,vel_mag,0.00033387238237809596,max_abs,0.0025053078788877881,finite,true
gates,finite,true,deterministic,true,legacy_recoil,false,emergent_recoil,true,diagnostic_qE_transverse_recoil,true
verdict,NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED
interpretation,native_force_paths_measured_diagnostic_qE_is_imposed_not_a_derivation
```

Repeat gates:

| Repeat arm | Max abs delta |
|---|---:|
| locked linear | `0` |
| native legacy | `0` |
| native emergent | `0` |
| diagnostic qE | `0` |

---

## 4. Verdict

Frozen outcome:

```text
NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED
```

Interpretation:

- The legacy native force path does not recoil above gate:
  `disp_mag=1.68e-17`, `vel_mag=2.04e-19`.
- The emergent native force path does recoil above gate:
  `disp_mag=0.21022031950099582`, `vel_mag=0.0018928965812042473`.
- The imposed diagnostic qE arm also responds transversely:
  `disp_y=-0.0025053078788877881`, `vel_y=0.00033387238237809596`.
- All repeat runs are bit-exact under the frozen metrics.

This is a real native engine response, but it is the EFT `grad(|J|)` channel,
not a Thomson cross-section and not a QED amplitude. The qE response remains a
declared imposed diagnostic, not native physics.

---

## 5. Dashboard Companion

Added visual scenario:

```text
s0-field-thomson-unlocked-recoil
```

The dashboard enables the same plane-wave seed, unlocks the center negative
charge, and turns on the mock/native emergent flux-gradient channel. Browser
verification on `http://localhost:8080/?engine=mock`:

```text
selected=s0-field-thomson-unlocked-recoil
tick=60
displacement=(-6.24e-2, 9.75e-2, 0.00e+0)
velocity=(8.26e-5, 1.49e-3, -6.37e-18)
|F_emergent|=2.66e-4
console_errors=0
```

The dashboard is a visual companion. The C++ campaign above is the
machine-precision measurement.

---

## 6. Significance

Classical Thomson scattering starts from a wave force on a charge: an incoming
electromagnetic wave accelerates the charge, and the accelerated charge
radiates. FTD-0287 showed the locked field observatory alone had no mechanical
loop. FTD-0288 now shows:

1. the production legacy force path still does not supply the recoil;
2. the production emergent flux-gradient path does supply deterministic recoil;
3. a direct qE electric hook would also produce a transverse response, but only
   as an imposed diagnostic.

That gives the next honest target: separate intensity/flux-gradient recoil from
true electric Thomson recoil, and only then consider any radiation or alpha
observable. No alpha claim is promoted.

## 7. Successor mechanism audit — FTD-0435

FTD-0435 executed that separation without changing the production force. The
dominant transverse response is polarity-even, not the polarity-odd symmetry
required by `qE`; smaller longitudinal and orthogonal components are odd. An
equal-energy circular wave with spatially constant external `|J|` retains
`0.996758` of the linear-wave RMS response. The supported description is
therefore **self-field-mediated flux interference**, not external intensity
descent and not ordinary electric coupling. See
`AUDIT_FLUX_SELF_INTERFERENCE_RESPONSE.md`. The FTD-0288 historical record and
its original numerical values remain unchanged.
