# ANALYSIS: Source-Free Discrete Tick Energy Invariant v1

**FTD ID:** FTD-0292
**Status:** [MEASUREMENT -- NUMERIC GATE INVALIDATED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/charge_gauss_native_em/PREREG_THOMSON_TICK_INVARIANT_v1.md`
**Lock commit:** `87f0cda2`
**Lock tag:** `preregister-thomson-tick-invariant-v1`
**Artifact:** `engine/tests/campaign_thomson_tick_invariant.cpp`
**Artifact SHA256:** `5e6e2b77796d8a91f02bc7b2a85c9c862dd1f4e91b832be19ae5d5b41c455e16`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant$" --output-on-failure
```

CTest result:

```text
Test #231: thomson_tick_invariant ...........***Failed    0.97 sec
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
- Modified-energy absolute gate: `1e-10`
- Modified-energy relative gate: `1e-12`
- Naive-energy drift visibility gate: `1e-6`

The frozen invariant candidate was:

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
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,alpha,0.0072973525643314245,modified_abs_gate,1e-10,modified_rel_gate,9.9999999999999998e-13,naive_drift_gate,9.9999999999999995e-07
scope,source_free_single_substrate_tick_invariant_not_alpha_or_cross_section
update,W_next=W+c2*LJ,J_next=J+W_next
observable,E_tick=0p5_W2_plus_0p5_JKJ_minus_0p5_WKJ,equivalent_cross=0p5_W_dot_c2LJ
energy,tick,0,kinetic,4.1367374469244576,gradient,4.1367374469201561,cross,1.0000680839006293e-15,naive,8.2734748938446145,modified,8.2734748938446163,finite,true
energy,tick,200,kinetic,4.3316576627705121,gradient,4.730740451278999,cross,-0.78892322020446859,naive,9.0623981140495111,modified,8.2734748938450426,finite,true
drift_summary,initial_naive,8.2734748938446145,final_naive,9.0623981140495111,max_abs_naive_drift,0.79866632316192465,max_rel_naive_drift,0.096533359127749893,max_naive_tick,69,initial_modified,8.2734748938446163,final_modified,8.2734748938450426,max_abs_modified_drift,2.1104895608914376e-11,max_rel_modified_drift,2.5509106971021582e-12,max_modified_tick,4,finite,true
gates,modified_invariant,false,naive_drift_seen,true
verdict,DISCRETE_TICK_INVARIANT_INVALIDATED
interpretation,source_free_tick_invariant_only_next_step_is_local_current_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
DISCRETE_TICK_INVARIANT_INVALIDATED
```

Interpretation:

- The naive continuum-style energy visibly drifted:
  `max_abs_naive_drift=0.79866632316192465`.
- The modified tick energy stayed much tighter:
  `max_abs_modified_drift=2.1104895608914376e-11`.
- The absolute modified gate passed (`2.11e-11 < 1e-10`).
- The relative modified gate failed (`2.55e-12 > 1e-12`).

This is best read as a numeric measurement-gate invalidation, not as a physics
disproof of the modified invariant. The v1 artifact used ordinary double
accumulation over the full lattice and set a relative gate too close to the
observed summation floor.

---

## 5. Significance

FTD-0291 showed that the first local graph-current candidate was not adequate.
FTD-0292 v1 gives the next clue: the correct global object is almost certainly
the discrete tick modified energy, not the naive continuum energy. But v1 did
not meet its own frozen relative gate, so the claim is not promoted.

The honest next step is a v2 measurement with precision-controlled
accumulation, while keeping the same algebraic observable and avoiding any
gate relaxation based on this result.

No radiation, Thomson cross-section, QED amplitude, or alpha claim is promoted.
