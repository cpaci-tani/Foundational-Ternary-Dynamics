# ANALYSIS: Source-Free Discrete Tick Local Continuity v2

**FTD ID:** FTD-0295
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- SOURCE-FREE LOCAL TICK CONTINUITY CONFIRMED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v2.md`
**Lock commit:** `1d4a29a5`
**Lock tag:** `preregister-thomson-tick-local-continuity-v2`
**Artifact:** `engine/tests/campaign_thomson_tick_local_continuity_v2.cpp`
**Artifact SHA256:** `9b48ca418e784ba98e35708563214b22c78cf2580f880fda9fa923cef4c7a804`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity_v2$" --output-on-failure
```

CTest result:

```text
Test #234: thomson_tick_local_continuity_v2 ...   Passed    0.76 sec
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
- Particle: none
- Coupling, damping, forces, movement, and all phenomenological extensions: off
- Ball radii: `{5, 7, 9, 11, 13}`
- Accumulation: long-double Kahan
- Balance absolute gate: `1e-10`
- Balance scale-relative gate: `1e-12`

The local density and current were unchanged from v1:

```text
h_i = 0.5 |W_i|^2 + 0.5 J_i dot (KJ)_i - 0.5 W_i dot (KJ)_i
Phi_i->j = 0.5 c^2 w_ij [J_i(old) dot W_j(next) - W_i(next) dot J_j(old)]
Delta H_V + Phi_out = 0
```

The gated relative denominator was changed from exchange scale to
finite-volume energy scale:

```text
scale = max(abs(H_V_old), abs(H_V_next),
            abs(Delta H_V) + abs(Phi_out), 1e-300)
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,alpha,0.0072973525643314245,balance_abs_gate,1.00000000000000003643e-10,balance_scale_rel_gate,9.99999999999999979887e-13
ball_radii,5,7,9,11,13
scope,source_free_single_substrate_tick_local_continuity_not_alpha_or_cross_section
density,h_i=0p5_W_i2_plus_0p5_J_i_dot_KJ_i_minus_0p5_W_i_dot_KJ_i,K=-c2L
current,Phi_i_to_j=0p5*c2*w_ij*(J_i_old_dot_W_j_next-W_i_next_dot_J_j_old),outward_positive
identity,Delta_H_V_plus_Phi_out_equals_0
relative_metric,scale=max(abs(H_V_old),abs(H_V_next),abs_delta_plus_abs_flux,1e-300),exchange_relative_reported_not_gated
accumulation,long_double_kahan
balance_summary,max_abs_dE,4.44089209850062616169e-16,max_abs_flux,2.7087059567747053373e-16,max_outward_flux,2.7087059567747053373e-16,max_inward_flux,2.21290933831518812333e-16,max_abs_balance,4.66471208448310248329e-16,rms_balance,9.30114134402137910897e-17,max_scale_rel_balance,2.98137309593416839599e-16,max_exchange_rel_balance,1,finite,true
gates,local_continuity,true,exchange_relative_degenerate,true
verdict,SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED
interpretation,source_free_local_tick_current_only_next_step_is_state_coupling_source_work_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED
```

Interpretation:

- The absolute finite-volume balance closes at roundoff:
  `max_abs_balance=4.664712084483102e-16`.
- The scale-relative balance also closes:
  `max_scale_rel_balance=2.981373095934168e-16`.
- The v1 exchange-relative metric still reports `1`, confirming that v1's
  failure was a relative-denominator pathology on quiet exchanges.

---

## 5. Significance

This confirms the source-free local current for the discrete tick energy.
FTD now has:

```text
Delta H_V + Phi_out(boundary V) = 0
```

for the actual source-free `phase_read`/`phase_write` update, not a continuum
surrogate. That is the missing local footing behind the flux-recoil program.

The next move is to add state-coupling source/work terms and then return to
the charge-plus-beam recoil setup. No radiation, Thomson cross-section, QED
amplitude, or fine-structure-constant claim is promoted by this run.
