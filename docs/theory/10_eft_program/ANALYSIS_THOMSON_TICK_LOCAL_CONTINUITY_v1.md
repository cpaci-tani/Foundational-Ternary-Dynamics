# ANALYSIS: Source-Free Discrete Tick Local Continuity v1

**FTD ID:** FTD-0294
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- NUMERIC RELATIVE-GATE INVALIDATED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v1.md`
**Lock commit:** `7ebc236e`
**Lock tag:** `preregister-thomson-tick-local-continuity-v1`
**Artifact:** `engine/tests/campaign_thomson_tick_local_continuity.cpp`
**Artifact SHA256:** `6b137c83016b9aefb10d47d22df0094487ab761c06e167870a209004ada99aa3`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity$" --output-on-failure
```

CTest result:

```text
Test #233: thomson_tick_local_continuity ....***Failed    0.75 sec
0% tests passed, 1 tests failed out of 1
```

The executable failure is the frozen classified outcome. After this analysis,
the CTest target is marked `WILL_FAIL` so future full-suite runs stay green
only if the invalidation reproduces.

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
- Balance relative gate: `1e-12`

The frozen local density and current were:

```text
h_i = 0.5 |W_i|^2 + 0.5 J_i dot (KJ)_i - 0.5 W_i dot (KJ)_i
Phi_i->j = 0.5 c^2 w_ij [J_i(old) dot W_j(next) - W_i(next) dot J_j(old)]
Delta H_V + Phi_out = 0
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,alpha,0.0072973525643314245,balance_abs_gate,1.00000000000000003643e-10,balance_rel_gate,9.99999999999999979887e-13
ball_radii,5,7,9,11,13
scope,source_free_single_substrate_tick_local_continuity_not_alpha_or_cross_section
density,h_i=0p5_W_i2_plus_0p5_J_i_dot_KJ_i_minus_0p5_W_i_dot_KJ_i,K=-c2L
current,Phi_i_to_j=0p5*c2*w_ij*(J_i_old_dot_W_j_next-W_i_next_dot_J_j_old),outward_positive
identity,Delta_H_V_plus_Phi_out_equals_0
accumulation,long_double_kahan
balance_summary,max_abs_dE,4.44089209850062616169e-16,max_abs_flux,2.7087059567747053373e-16,max_outward_flux,2.7087059567747053373e-16,max_inward_flux,2.21290933831518812333e-16,max_abs_balance,4.66471208448310248329e-16,rms_balance,9.30114134402137910897e-17,max_rel_balance,1,finite,true
gates,local_continuity,false
verdict,SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED
interpretation,source_free_local_tick_current_only_next_step_is_state_coupling_source_work_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED
```

Interpretation:

- The absolute residual closes far below the absolute gate:
  `max_abs_balance=4.664712084483102e-16`.
- The relative residual fails because the v1 denominator was
  `abs(Delta H_V) + abs(Phi_out)`, which becomes degenerate on quiet
  intervals and surfaces.
- The maximum relative value is therefore `1`, even though the absolute error
  is roundoff-sized.

This is a numeric relative-gate invalidation, not a disproof of the local
current. The next version should keep the same density/current and use a
scale-relative denominator tied to the finite-volume energy magnitude.

---

## 5. Significance

FTD-0294 v1 is not promoted as a confirmed local theorem because it missed its
own frozen relative gate. It nevertheless strongly indicates the edge current
is algebraically right: all finite-volume balances close at `~1e-16` absolute
scale.

No radiation, Thomson cross-section, QED amplitude, state-coupling source, or
alpha claim is promoted by this v1 result.
