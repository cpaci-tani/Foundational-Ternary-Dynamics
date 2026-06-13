# ANALYSIS: Fixed-Charge Coupled Tick Source/Work Continuity v1

**FTD ID:** FTD-0296
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- FIXED-CHARGE SOURCE WORK CONTINUITY CONFIRMED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_COUPLED_SOURCE_WORK_v1.md`
**Lock commit:** `5d88062e`
**Lock tag:** `preregister-thomson-coupled-source-work-v1`
**Artifact:** `engine/tests/campaign_thomson_coupled_source_work.cpp`
**Artifact SHA256:** `95747a57895973577e0054d075752b79e74173507097652e31498b125d7ec88e`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_coupled_source_work$" --output-on-failure
```

CTest result:

```text
Test #235: thomson_coupled_source_work ......   Passed    1.82 sec
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
- Particle: one locked negative charge at lattice center
- Coupling: on
- Movement and forces: off
- Ball radii: `{5, 7, 9, 11, 13}`
- Accumulation: long-double Kahan
- Balance absolute gate: `1e-10`
- Balance scale-relative gate: `1e-12`

The fixed coupled identity was:

```text
W* = W + c^2 L J
W' = W* + S
J' = J + W'
S = G_C (grad_state + curl_state_velocity)
Work_i = W*_i dot S_i + 0.5 |S_i|^2 + 0.5 J_i dot (K S)_i
Delta H_V + Phi_out_source_free - Work_V = 0
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,g_c,0.085424543102854369,alpha,0.0072973525643314245,balance_abs_gate,1.00000000000000003643e-10,balance_scale_rel_gate,9.99999999999999979887e-13
ball_radii,5,7,9,11,13
scope,fixed_charge_coupled_tick_source_work_not_alpha_or_cross_section
source,S=G_C*(grad_state+curl_state_velocity),movement,false,charge_locked,true
identity,Delta_H_V_plus_Phi_source_free_out_minus_Work_source_equals_0
work,Work_i=Wstar_i_dot_S_i_plus_0p5_S_i2_plus_0p5_J_i_dot_KS_i
accumulation,long_double_kahan
balance_summary,max_abs_dE,0.00912169070541428428367,max_abs_flux,0.000341694828402740419064,max_abs_work,0.00912169070541427734478,max_abs_balance,4.42910139987484630097e-16,rms_balance,9.83291097236411076231e-17,max_scale_rel_balance,2.20455860816464694277e-16,finite,true
gates,coupled_source_work,true
verdict,FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED
interpretation,fixed_source_work_accounting_only_next_step_unlocked_recoil_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED
```

Interpretation:

- The source/work term is not tiny: `max_abs_work=0.009121690705414277`.
- Boundary flux is smaller but finite: `max_abs_flux=0.0003416948284027404`.
- The full finite-volume source/work balance closes at roundoff:
  `max_abs_balance=4.429101399874846e-16`.
- The scale-relative balance also closes:
  `max_scale_rel_balance=2.204558608164647e-16`.

---

## 5. Significance

This extends the source-free local tick theorem to the fixed state-flux source
used by the engine. The project now has a native energy accounting chain:

```text
source-free energy invariant
source-free finite-volume current
fixed-source work term
```

That is the right foundation for returning to the unlocked recoil experiment.
The next open target is moving-source/recoil accounting, where particle motion
and flux redistribution add terms beyond the fixed-source phase-read/phase-write
identity tested here.

No radiation, Thomson cross-section, QED amplitude, or fine-structure-constant
claim is promoted by this run.
