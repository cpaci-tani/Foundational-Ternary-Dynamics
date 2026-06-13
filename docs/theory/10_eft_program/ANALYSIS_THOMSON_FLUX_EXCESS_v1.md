# ANALYSIS: Thomson Flux-Excess Discriminator v1

**FTD ID:** FTD-0289
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- NATIVE EMERGENT EXCESS FLUX DEFLECTION]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_FLUX_EXCESS_v1.md`
**Lock commit:** `acb4005a`
**Lock tag:** `preregister-thomson-flux-excess-v1`
**Artifact:** `engine/tests/campaign_thomson_flux_excess.cpp`
**Artifact SHA256:** `1f562ac9e9e0f3fdeb72bce00fda2c00f70117271439ef27d418d05c29ec7589`
**Dashboard scenario:** `s0-field-thomson-unlocked-recoil`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_flux_excess$" --output-on-failure
```

CTest result:

```text
Test #228: thomson_flux_excess ..........   Passed    1.92 sec
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
- Local residual radius: `8`
- Machine repeat gate: `1e-12`
- Excess gate: `1e-8`

For each mode, the campaign runs:

- `beam_only`
- `charge_only`
- `charge_plus_beam`
- `charge_plus_beam_repeat`

The frozen residual observable is:

```text
residual = charge_plus_beam - beam_only - charge_only
```

The subtraction is performed componentwise over six components:
`Jx,Jy,Jz,wave_vel_x,wave_vel_y,wave_vel_z`.

Modes:

| Mode | Force path | Charge locked? |
|---|---|---|
| `locked_linear` | no force/movement | yes |
| `native_legacy` | legacy native force/movement | no |
| `native_emergent` | native emergent flux-gradient force/movement | no |

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,local_radius,8,c_speed,0.57735026918962573,alpha,0.0072973525643314245,machine_gate,9.9999999999999998e-13,excess_gate,1e-08
scope,baseline_subtracted_flux_excess_not_alpha_derivation
observable,residual=charge_plus_beam-minus-beam_only-minus-charge_only
residual,locked_plus_minus_beam_minus_charge,l2,3.0181603626380473e-14,rel_l2,3.8944842137800063e-15,max_abs,4.3465665094943873e-16,mean_abs,3.0473335317608949e-17,energy,4.5546459872997334e-28,local_energy,2.3979225436087662e-29,transverse_centroid,0.14296393616691488,finite,true
residual,legacy_plus_minus_beam_minus_charge,l2,3.0103408467918511e-14,rel_l2,3.8843942989433421e-15,max_abs,4.3378928921145032e-16,mean_abs,3.078326811687389e-17,energy,4.5310760069317566e-28,local_energy,2.3904667633649649e-29,transverse_centroid,0.14488588624303,finite,true
residual,emergent_plus_minus_beam_minus_charge,l2,0.00042546065759857619,rel_l2,5.4899329705502643e-05,max_abs,6.3648580289611865e-05,mean_abs,5.6150551413465128e-07,energy,9.0508385582104294e-08,local_energy,3.2674008350733898e-08,local_cx,1.1943325778150228e-11,local_cy,-3.4010459033592432e-13,local_cz,-5.5552607522498244e-13,transverse_centroid,6.5136806232915638e-13,finite,true
gates,finite,true,deterministic,true,locked_linear,true,legacy_excess,false,emergent_excess,true,emergent_transverse,false
verdict,NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED
interpretation,field_residual_observable_only_no_alpha_or_cross_section_claim
```

Repeat residuals:

| Repeat arm | `max_abs` |
|---|---:|
| locked linear | `0` |
| native legacy | `0` |
| native emergent | `0` |

---

## 4. Verdict

Frozen outcome:

```text
NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED
```

Interpretation:

- The locked-linear control remains machine-linear after baseline subtraction:
  `max_abs=4.35e-16`, `rel_l2=3.89e-15`.
- The native legacy force/movement path also remains at machine residual:
  `max_abs=4.34e-16`, `rel_l2=3.88e-15`.
- The native emergent flux-gradient path creates an above-gate residual:
  `l2=4.2546065759857619e-4`, `rel_l2=5.4899329705502643e-5`,
  `max_abs=6.3648580289611865e-5`.
- The frozen transverse-centroid subtype does not fire:
  `emergent_transverse=false`.

This is a baseline-subtracted field/wave residual measurement. It is not a
Thomson cross-section, not a QED scattering amplitude, and not an alpha
derivation.

---

## 5. Dashboard Companion

The dashboard companion now exposes the same residual concept in the P1
Thomson and fine-structure panels:

```text
excess residual = charge_plus_beam - beam_only - charge_only
```

Browser verification on `http://localhost:8080/?engine=mock&v=ftd0289` showed
the panel opens for `s0-field-thomson-unlocked-recoil`, reaches tick `200`, and
displays the FTD-0289 C++ canonical residual values. The JS mock's own live
residual reads zero in this run, while the visual flux centroid and carrier
motion are nonzero.

That distinction is useful: the dashboard is an ingredient observatory, not the
machine-precision adjudicator. The C++ campaign above remains the run of record.

---

## 6. Significance

FTD-0287 established that a locked charge plus beam is just the sum of its
parts at machine precision. FTD-0288 established that an unlocked carrier can
recoil in the native emergent flux-gradient channel. FTD-0289 now separates
free wave propagation and charge-only field structure from the combined run.

The result is narrow but meaningful: the emergent channel contains an actual
above-gate flux/wave residual after the obvious baselines are subtracted, while
the legacy path does not.

The next honest target is not "alpha from Thomson scattering." It is a more
primitive question: derive or measure the native radiation/angular-power
observable that turns this excess residual into a continuum-limit scattering
quantity, if such an observable exists.
