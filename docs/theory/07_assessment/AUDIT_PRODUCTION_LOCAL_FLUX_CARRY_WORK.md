# AUDIT — Production local flux-carry work

**Identifier:** `FTD-0461`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — FINITE EVENT IDENTITY]` +
`[MEASURED — PRODUCTION CARRY CORRECTION]` +
`[CLOSED NEGATIVE — ENERGY-NEUTRAL LOCAL CARRY]`  
**Run of record:** `engine/results/ftd_0461/windows_msvc_cpu.csv`

## Result

Production's source-site flux carry is not energy-neutral under the FTD-0452
wave-plus-interaction observer, and fixed-field endpoint work alone does not
close an event that includes the carry. The locked verdict is:

`PRODUCTION_LOCAL_CARRY_REQUIRES_EXPLICIT_WORK_CORRECTION`

Across all 42 FTD-0459 attempt ticks:

- fixed-field identity `Delta H_fixed + W_endpoint` closes below
  `1.734723475976807e-18`;
- local carry magnitude ranges from `9.010659412412682e-5` to
  `0.007004529023102515`, so the `K_B=0.511` cap never activates;
- carry correction has RMS `1.81999100843571e-5` and maximum magnitude
  `6.04366710726422e-5`;
- production's zero-particle-work residual reaches
  `0.014441489747420258`;
- endpoint work after including carry misses closure by up to
  `6.04366710726422e-5`;
- exact add/remove event reversal closes below `8.673617379884036e-19`;
- all endpoint works reproduce FTD-0459 exactly.

The correction changes none of the FTD-0459 particle-kinematic
classifications: endpoint and corrected required work each admit 12 of 42
attempts. The local carry therefore cannot rescue the registered first event.

## Exact identity

Let `T` be production's source-to-target local `J` transfer, with `W` held
fixed. Define

`C_carry = H(J+T,W;s_b)-H(J,W;s_b)`

relative to the already moved-state fixed-field event. Since fixed-field
translation gives `Delta H_fixed=-W_endpoint`, the required particle work is
exactly

`W_required = W_endpoint-C_carry`.

This is finite-dimensional algebra once the FTD-0452 quadratic observer and
production transfer `T` are adopted. The measured identity residual is below
`1.734723475976807e-18` in every arm. It is not a proof that this observer is
the production Hamiltonian.

## Consequence for the current engine

Production movement currently changes neither velocity nor particle energy at
an integer hop. It nevertheless relocates `J` from source to target. Under the
selected exact wave-plus-interaction functional, that is an uncompensated
event-energy change. FTD-0297's prior additive source/work closure explicitly
stopped before integer transport; FTD-0461 closes that open measurement and
finds a nonzero transport term.

The term is small compared with the FTD-0460 static self-source barrier. Thus
two defects remain distinct:

1. total-field endpoint work charges the polarity against its accumulated
   self-source field;
2. production's partial source-site carry introduces an additional local
   field-energy correction that no current particle update pays.

## Next gate

The next audit must compare partial local carry with rigid translation of an
explicit source-generated dressing. A complete translated composite should
preserve its isolated self-energy by lattice translation symmetry; only cross
energy with the external packet/dressing may change. If rigid translation
closes while local carry does not, the production rule is tearing the dressing.
If even rigid translation has a residual after the external cross term is
accounted, the proposed bound/radiative split is wrong.

## Build provenance

The first build failed before execution because the campaign used a descriptive
forwarding-header name not present in the tree. A one-line forwarding header to
`native_energy_contract.h` was added. The locked campaign source, estimators,
and gates were unchanged; no result preceded the correction.
