# ANALYSIS: Thomson Radiation Shell Meter v1

**FTD ID:** FTD-0290
**Status:** [MEASUREMENT -- NO BASELINE-SUBTRACTED OUTWARD POWER]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/charge_gauss_native_em/PREREG_THOMSON_RADIATION_SHELLS_v1.md`
**Lock commit:** `8ccfee7b`
**Lock tag:** `preregister-thomson-radiation-shells-v1`
**Artifact:** `engine/tests/campaign_thomson_radiation_shells.cpp`
**Artifact SHA256:** `a47de9c1bb52f92a6dc35471f4eba516fb76acaf9b66abd3de44dd6431d67edf`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_radiation_shells$" --output-on-failure
```

CTest result:

```text
Test #229: thomson_radiation_shells .........   Passed    1.51 sec
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
- Shell radii: `{5, 7, 9, 11, 13, 15}`
- Shell half-width: `0.5`
- Machine repeat gate: `1e-12`
- Outward power gate: `1e-8`
- Angular structure gate: `1e-3`

The frozen residual field is:

```text
J_res = J_charge_plus_beam - J_beam_only - J_charge_only
W_res = W_charge_plus_beam - W_beam_only - W_charge_only
```

The shell meter computes:

```text
E_res = -W_res
B_res = curl(J_res)
S_res = E_res × B_res
P_out(R) = Σ_shell max(0, S_res · r_hat)
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,shell_half_width,0.5,c_speed,0.57735026918962573,alpha,0.0072973525643314245,machine_gate,9.9999999999999998e-13,power_gate,1e-08,angular_gate,0.001
shell_radii,5,7,9,11,13,15
scope,residual_field_poynting_shell_meter_not_alpha_or_cross_section
observable,S_res=E_res_cross_B_res_from_residual_field_charge_plus_beam-minus-beam_only-minus-charge_only
shell_summary,locked_residual_shells,max_abs_power,4.841790445760983e-30,max_outward_power,2.4224129770411117e-30,max_net_abs_power,2.590028498646311e-31,strongest_radius,0,strongest_dipole,0,strongest_quadrupole,0,finite,true
shell_summary,legacy_residual_shells,max_abs_power,4.847003926973922e-30,max_outward_power,2.4219423915622456e-30,max_net_abs_power,2.5994450797253127e-31,strongest_radius,0,strongest_dipole,0,strongest_quadrupole,0,finite,true
shell_summary,emergent_residual_shells,max_abs_power,2.5242201929664388e-09,max_outward_power,2.0869970474307537e-09,max_net_abs_power,1.8976712229187541e-09,strongest_radius,0,strongest_dipole,0,strongest_quadrupole,0,finite,true
gates,finite,true,deterministic,true,locked_linear,true,legacy_radiation,false,emergent_radiation,false,emergent_structured,false
verdict,NO_BASELINE_SUBTRACTED_OUTWARD_POWER
interpretation,residual_field_shell_power_only_no_alpha_cross_section_or_qed_claim
```

Repeat shell residuals were exactly zero for locked, legacy, and emergent
arms under the frozen metrics.

---

## 4. Verdict

Frozen outcome:

```text
NO_BASELINE_SUBTRACTED_OUTWARD_POWER
```

Interpretation:

- The locked-linear shell meter is machine-zero:
  `max_abs_power=4.84e-30`, `max_outward_power=2.42e-30`.
- The native legacy shell meter is also machine-zero:
  `max_abs_power=4.85e-30`, `max_outward_power=2.42e-30`.
- The native emergent shell meter has a finite sub-gate trace:
  `max_abs_power=2.5242201929664388e-9`,
  `max_outward_power=2.0869970474307537e-9`.
- The frozen outward power gate was `1e-8`, so the emergent trace is not a
  detection.
- No angular-structure branch can fire because no shell reached the fixed
  power gate.

This result does not erase FTD-0289. It says that FTD-0289's above-gate
field/wave residual does not become above-gate outward residual Poynting power
on the fixed FTD-0290 shells.

---

## 5. Significance

FTD-0288 measured native emergent carrier recoil. FTD-0289 measured an
above-gate residual field/wave response. FTD-0290 asked whether that response
already qualifies as outward radiation.

The answer under the frozen protocol is no. The emergent channel creates a
local/excess residual, but this shell meter does not yet see a radiative
outflow above gate.

That is a useful boundary: the project should not claim Thomson scattering,
cross-section recovery, or any alpha consequence from the recoil/excess result
alone. The next honest work is either to derive the correct native radiation
observable from the engine equations or to pre-register a strictly justified
continuum/longer-time shell campaign. No tuning follows from this v1 result.
