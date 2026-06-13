# ANALYSIS: Source-Free Discrete Tick Energy Invariant v2

**FTD ID:** FTD-0293
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- DISCRETE TICK MODIFIED ENERGY CONFIRMED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_TICK_INVARIANT_v2.md`
**Lock commit:** `83863d5e`
**Lock tag:** `preregister-thomson-tick-invariant-v2`
**Artifact:** `engine/tests/campaign_thomson_tick_invariant_v2.cpp`
**Artifact SHA256:** `c362d35e1a2c61216982bb7ae2c8cf4ee916e59f1e3bcc77a62cee993caa8b5f`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant_v2$" --output-on-failure
```

CTest result:

```text
Test #232: thomson_tick_invariant_v2 ........   Passed    1.00 sec
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
- Accumulation: long-double Kahan
- Modified-energy absolute gate: `1e-10`
- Modified-energy relative gate: `1e-12`
- Naive-energy drift visibility gate: `1e-6`

The frozen invariant was unchanged from v1:

```text
E_tick = 0.5 W^2 + 0.5 J K J - 0.5 W K J
       = 0.5 W^2 + E_grad + 0.5 W dot c^2 L J
```

The comparison energy was:

```text
E_naive = 0.5 W^2 + E_grad
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,alpha,0.0072973525643314245,modified_abs_gate,1.00000000000000003643e-10,modified_rel_gate,9.99999999999999979887e-13,naive_drift_gate,9.99999999999999954748e-07
scope,source_free_single_substrate_tick_invariant_precision_v2_not_alpha_or_cross_section
update,W_next=W+c2*LJ,J_next=J+W_next
observable,E_tick=0p5_W2_plus_0p5_JKJ_minus_0p5_WKJ,equivalent_cross=0p5_W_dot_c2LJ
accumulation,long_double_kahan
energy,tick,0,kinetic,4.13673744692420530811,gradient,4.13673744692420530811,cross,7.17314933579987767942e-16,naive,8.27347489384841061622,modified,8.27347489384841061622,finite,true
energy,tick,200,kinetic,4.33165766277029984366,gradient,4.73074045128262721249,cross,-0.788923220204510777798,naive,9.06239811405292705615,modified,8.27347489384841594529,finite,true
drift_summary,initial_naive,8.27347489384841061622,final_naive,9.06239811405292705615,max_abs_naive_drift,0.798666323156913549042,max_rel_naive_drift,0.096533359127099926944,max_naive_tick,69,initial_modified,8.27347489384841061622,final_modified,8.27347489384841594529,max_abs_modified_drift,7.10542735760100185871e-15,max_rel_modified_drift,8.58820199343822426235e-16,max_modified_tick,192,finite,true
gates,modified_invariant,true,naive_drift_seen,true
verdict,DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED
interpretation,source_free_tick_invariant_confirmed_before_local_current_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED
```

Interpretation:

- The naive continuum-style energy drifts strongly:
  `max_abs_naive_drift=0.7986663231569135`.
- The modified tick energy closes under both gates:
  `max_abs_modified_drift=7.105427357601002e-15`,
  `max_rel_modified_drift=8.588201993438224e-16`.
- This confirms that FTD-0291 failed because it used the wrong energy object
  for the discrete tick.

---

## 5. Significance

This is the first clean native footing for the recoil/flux program after the
Poynting-shell negative result. The source-free engine tick does not conserve
the naive continuum energy. It conserves a discrete modified energy with a
cross term:

```text
0.5 W dot c^2 L J
```

That term is not cosmetic. It changes what a finite-volume current must move
across a boundary. The next target is therefore an exact local continuity law
for `E_tick`, not a stronger Poynting shell and not an alpha/cross-section
claim.

No radiation, Thomson cross-section, QED amplitude, or fine-structure-constant
claim is promoted by this run.
