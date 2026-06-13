# ANALYSIS: Thomson Native Finite-Volume Continuity Meter v1

**FTD ID:** FTD-0291
**Date:** 2026-06-13
**Status:** [MEASUREMENT -- NATIVE GRAPH CONTINUITY CANDIDATE INVALIDATED]
**Pre-registration:** `docs/theory/10_eft_program/preregistrations/PREREG_THOMSON_NATIVE_CONTINUITY_v1.md`
**Lock commit:** `47ccbee4`
**Lock tag:** `preregister-thomson-native-continuity-v1`
**Artifact:** `engine/tests/campaign_thomson_native_continuity.cpp`
**Artifact SHA256:** `357a2a2b4bd7fb8d8604a4c30490f68ab9a404e8574ed6e55b034056a5b3f3e8`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_native_continuity$" --output-on-failure
```

CTest result:

```text
Test #230: thomson_native_continuity ........   Passed    8.96 sec
100% tests passed, 0 tests failed out of 1
```

The full console payload was recovered from
`engine/build/Testing/Temporary/LastTest.log`.

The test passes because candidate invalidation is a pre-registered classified
outcome. Only non-finite, nondeterministic, or locked-linear-superposition
failure outcomes are process failures.

---

## 2. Protocol

Shared setup:

- Lattice: `L=33`
- Ticks: `200`
- Incoming beam: y-polarized plane wave along +x
- Mode: `n=4`
- Amplitude: `0.05`
- Ball radii: `{5, 7, 9, 11, 13}`
- Machine repeat gate: `1e-12`
- Balance absolute gate: `1e-8`
- Balance relative gate: `1e-6`
- Graph outward-flux gate: `1e-8`

The frozen residual field is:

```text
J_res = J_charge_plus_beam - J_beam_only - J_charge_only
W_res = W_charge_plus_beam - W_beam_only - W_charge_only
```

The finite-volume balance meter uses endpoint-attributed graph energy over the
engine's 18-neighbor wave-Laplacian stencil and the fixed candidate current:

```text
F_i->j = -c^2 w_ij 0.5 (W_i + W_j) dot (J_j - J_i)
balance = Delta E_V + sum_boundary F_i->j
```

---

## 3. Frozen Output

Key lines:

```text
protocol,L,33,ticks,200,mode_n,4,amp,0.050000000000000003,c_wave,0.57735026918962573,alpha,0.0072973525643314245,machine_gate,9.9999999999999998e-13,balance_abs_gate,1e-08,balance_rel_gate,9.9999999999999995e-07,graph_flux_gate,1e-08
ball_radii,5,7,9,11,13
scope,native_graph_finite_volume_continuity_not_alpha_or_cross_section
energy,graph_hamiltonian,E_V=sum_inside_0p5_W2_plus_endpoint_attributed_0p5_c2_gradJ2
current,F_i_to_j=-c2*w_ij*0p5*(W_i+W_j)_dot_(J_j-J_i),stencil,18_neighbor_laplacian
balance_summary,locked_linear_beam_only_graph_balance,max_abs_dE,0.089289384523970838,max_abs_flux,0.0052903798351870857,max_outward_flux,0.0052903798351870857,max_inward_flux,0.0049606926200744614,max_abs_balance,0.085722521337618693,rms_balance,0.033201877722726945,max_rel_balance,1,finite,true
balance_summary,locked_linear_residual_graph_balance,max_abs_dE,1.5087482480596341e-30,max_abs_flux,1.3967080560803303e-31,max_outward_flux,1.2503560659457779e-31,max_inward_flux,1.3967080560803303e-31,max_abs_balance,1.5323646792748475e-30,rms_balance,3.2361139893528859e-31,max_rel_balance,1,finite,true
balance_summary,locked_linear_repeat_graph_balance,max_abs_dE,0,max_abs_flux,0,max_outward_flux,0,max_inward_flux,0,max_abs_balance,0,rms_balance,0,max_rel_balance,0,finite,true
balance_summary,native_legacy_beam_only_graph_balance,max_abs_dE,0.089289384523970838,max_abs_flux,0.0052903798351870857,max_outward_flux,0.0052903798351870857,max_inward_flux,0.0049606926200744614,max_abs_balance,0.085722521337618693,rms_balance,0.033201877722726945,max_rel_balance,1,finite,true
balance_summary,native_legacy_residual_graph_balance,max_abs_dE,1.5136973489972856e-30,max_abs_flux,1.4058040245149e-31,max_outward_flux,1.2509001558701531e-31,max_inward_flux,1.4058040245149e-31,max_abs_balance,1.5379400564390664e-30,rms_balance,3.249665201542447e-31,max_rel_balance,1,finite,true
balance_summary,native_legacy_repeat_graph_balance,max_abs_dE,0,max_abs_flux,0,max_outward_flux,0,max_inward_flux,0,max_abs_balance,0,rms_balance,0,max_rel_balance,0,finite,true
balance_summary,native_emergent_beam_only_graph_balance,max_abs_dE,0.089289384523970838,max_abs_flux,0.0052903798351870857,max_outward_flux,0.0052903798351870857,max_inward_flux,0.0049606926200744614,max_abs_balance,0.085722521337618693,rms_balance,0.033201877722726945,max_rel_balance,1,finite,true
balance_summary,native_emergent_residual_graph_balance,max_abs_dE,1.4399223895571102e-08,max_abs_flux,7.2143434756759083e-10,max_outward_flux,7.2143434756759083e-10,max_inward_flux,1.0019630120933133e-10,max_abs_balance,1.4399223895571102e-08,rms_balance,4.5848856251719499e-09,max_rel_balance,1,finite,true
balance_summary,native_emergent_repeat_graph_balance,max_abs_dE,0,max_abs_flux,0,max_outward_flux,0,max_inward_flux,0,max_abs_balance,0,rms_balance,0,max_rel_balance,0,finite,true
gates,finite,true,repeats_deterministic,true,locked_linear,true,beam_continuity,false,legacy_flux,false,legacy_source,false,emergent_flux,false,emergent_source,true
verdict,NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED
interpretation,native_finite_volume_accounting_only_no_alpha_cross_section_or_qed_claim
```

---

## 4. Verdict

Frozen outcome:

```text
NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED
```

Interpretation:

- Determinism passed: all repeat graph-balance metrics are exactly zero.
- Locked-linear residual control passed: `max_abs_balance=1.53e-30`.
- Legacy residual control passed: `max_abs_balance=1.54e-30`.
- The free-wave beam-only control failed the continuity gate:
  `max_abs_balance=0.085722521337618693`, `max_rel_balance=1`.
- Therefore the frozen graph-current candidate cannot be used to certify
  radiation or source work in the recoil setup.
- The emergent residual trace has `max_abs_balance=1.44e-8`, above the fixed
  balance gate, while outward graph flux remains below gate at `7.21e-10`.
  Because the free-wave control failed first, this is not promoted as a native
  source claim.

---

## 5. Significance

FTD-0288 showed native emergent flux-gradient recoil. FTD-0289 showed an
above-gate residual field/wave response. FTD-0290 showed that the response did
not become above-gate outward Poynting shell power. FTD-0291 now says the first
simple native graph-current candidate also cannot carry the claim.

That is useful negative knowledge. The next required object is not a longer
Thomson run or an alpha panel. It is an exact discrete-time local energy
continuity theorem for the actual `phase_read`/`phase_write` update, including
the update's time staggering and the distinction between ledger energy and
Hamiltonian graph energy.

No radiation, Thomson cross-section, QED amplitude, or fine-structure-constant
claim is promoted by this run.
