# ANALYSIS: Thomson Recoil Observatory v1

**FTD ID:** FTD-0287
**Status:** [OBSERVATION -- LINEAR SUPERPOSITION, NO MECHANICAL RECOIL]
**Artifact:** `engine/tests/campaign_thomson_recoil_observatory.cpp`
**Artifact SHA256:** `E6D6BE58F2C1EC808F8F1C0FFC7B40DDEA48D193DE3F596C23651DEDD942EA45`
**Dashboard scenario:** `s0-field-thomson-scattering`
**Base commit before work:** `99e5a92ae9ba3b2b1fa70532f850fba65613e283`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_recoil_observatory$" --output-on-failure
```

CTest result:

```text
Test #226: thomson_recoil_observatory ....... Passed    0.43 sec
100% tests passed, 0 tests failed out of 1
```

The full console payload was recovered from
`engine/build/Testing/Temporary/LastTest.log`.

---

## 2. Protocol

The campaign is a machine-precision companion to the visual dashboard scenario.
It uses the same ingredients:

- Lattice: `L=33`
- Ticks: `200`
- Incoming beam: y-polarized plane wave moving in +x
- Mode: `n=4`
- Amplitude: `0.05`
- Charge: one locked negative electron-like voxel at the center
- Active terms: `wave_propagation=true`, `coupling=true`
- Disabled terms: `damping`, `gauss_projection`, `forces`, `movement`,
  `poisson_coulomb`

This is deliberately a field observatory. The electron-like site is locked, so
the run cannot establish mechanical recoil.

The measured arms were:

- `beam_only`
- `electron_only`
- `electron_plus_beam`
- `electron_plus_beam_repeat`

The core discriminator is the final-field residual:

```text
electron_plus_beam - beam_only - electron_only
```

computed over every double-valued flux and wave-velocity component in the full
cell.

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_speed,0.57735026918962573,double_epsilon,2.2204460492503131e-16,machine_gate,9.9999999999999998e-13
scope,instrument_not_alpha_derivation
ingredients,wave_propagation,true,coupling,true,damping,false,gauss_projection,false,forces,false,movement,false,poisson_coulomb,false,locked_charge,true
residual,plus_minus_beam_minus_electron,l2,3.0181603626380473e-14,rel_l2,3.8944842137800063e-15,max_abs,4.3465665094943873e-16,mean_abs,3.0473335317608949e-17,max_index,122221,finite,true
residual,electron_plus_beam_repeat,l2,0,rel_l2,0,max_abs,0,mean_abs,0,max_index,0,finite,true
verdict,LINEAR_SUPERPOSITION_NO_RECOIL_OBSERVED
interpretation,locked_charge_field_observatory_not_mechanical_recoil
```

Selected observables:

| Arm | Center `wv_y` | Center energy r=3 | Lateral energy y+10 | Poynting x | Poynting y |
|---|---:|---:|---:|---:|---:|
| beam only | -0.005965939504180607 | 0.10243808136682556 | 0.10243808136682560 | 13.303585701696846 | 0 |
| electron only | 1.0059623952895335e-17 | 0.005046894323536485 | 2.914121169738401e-05 | 4.367277370096984e-20 | -4.691108139626702e-20 |
| electron + beam | -0.005965939504180636 | 0.10748497569036179 | 0.10311536164794022 | 13.303585701697585 | 0.003966720280411029 |

---

## 4. Verdict

Frozen outcome:

```text
LINEAR_SUPERPOSITION_NO_RECOIL_OBSERVED
```

The repeat run is bit-exact. The combined field equals the sum of the two
single-ingredient fields to a maximum absolute residual of
`4.3465665094943873e-16`, essentially double precision.

This means the dashboard can show a visually meaningful field pattern around a
locked charge, but the current protocol does not produce nonlinear scattering
or mechanical electron recoil. Energy density and Poynting observables can still
change because they are quadratic functions of the superposed fields; those
cross-terms are real diagnostics, but they are not evidence of a new force
law.

---

## 5. Significance

This result is useful precisely because it is negative.

Alpha is the dimensionless strength of the electron-photon interaction. A true
FTD-native path toward alpha would need a measured coupling response, not a
visual overlap of two independently evolving fields. FTD-0287 shows that the
current dashboard recipe is an honest instrument for observing the field
ingredients, but it is not yet the interaction paradigm.

The next valid recoil/scattering protocol must add one missing ingredient and
freeze it before interpretation:

- an unlocked charge with a measured trajectory, or
- an explicitly audited electric force response to the incoming wave, or
- an acceleration/radiation channel whose outgoing angular power is compared to
  a pre-registered Thomson-style observable.

Until then, no alpha claim is promoted.
