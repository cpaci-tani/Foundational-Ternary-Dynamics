# PRE-REGISTRATION — Production local flux-carry work v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0461`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; BUILD CORRECTION RECORDED]`  
**Parents:** `FTD-0297`, `FTD-0449`, `FTD-0459`, `FTD-0460`  
**Engine artifact:** `engine/tests/campaign_production_local_flux_carry_work.cpp`

**Locked campaign SHA-256:**
`475E614D7F18D7928F16AA7B25AE98A697DF4C62B9BD5453B1D480E165E7EFFE`

**Pre-execution implementation correction:** the first build failed before
execution because the campaign named the existing
`native_energy_contract.h` observer as `native_wave_energy.h`. A forwarding
header with the latter name was added; the locked campaign body, equations,
estimators, and gates did not change. No result existed before the correction.
Forwarding-header SHA-256:
`E763073C3D45ABAE9D047D3DE827817382FDF607850A8A90FC38017488C2570A`.

## 1. Question

Does the production movement rule's local self-field carry close the integer-hop
energy contract, does the prior fixed-field endpoint work still close it, or is
an additional carry correction required?

## 2. Frozen history and event

Reproduce the FTD-0459 `L=33`, 48-tick packet + dressing + moving-polarity
history with the FTD-0460 snapshot observer. Do not execute a persistent move.
At each of the same 42 scheduled ticks, construct two reversible
counterfactual copies:

1. move `s`, velocity, and remainder from source to `+x` target while holding
   `J/W` fixed;
2. perform the same move plus the production local carry: transfer the source
   site's `J` vector to the target with magnitude capped at `K_B`; do not carry
   `W`, matching `phase_movement`.

Use the FTD-0452 observer Hamiltonian

`H = H_wave_modified - G_C s div(J)`

and the exact fixed-field endpoint work

`W_endpoint = G_C[(div J)_target-(div J)_source]`.

Record:

- fixed-field closure `Delta H_fixed + W_endpoint`;
- production zero-particle-work residual `Delta H_carry`;
- selected endpoint-work residual `Delta H_carry + W_endpoint`;
- exact carry correction `C_carry = Delta H_carry-Delta H_fixed`;
- required closing particle work `W_required=-Delta H_carry`;
- carried magnitude and exact add/remove reversal.

All 42 endpoint-work values must reproduce FTD-0459 exactly to `1e-12`.

## 3. Frozen gates and verdicts

- 42 attempts; all finite;
- fixed-field closure, algebraic correction identity, endpoint-work replay, and
  event reversal residuals `<=1e-12`;
- `PRODUCTION_LOCAL_CARRY_ENERGY_NEUTRAL` if
  `max |Delta H_carry|<=1e-12`;
- otherwise `ENDPOINT_WORK_CLOSES_PRODUCTION_LOCAL_CARRY` if
  `max |Delta H_carry+W_endpoint|<=1e-12`;
- otherwise `PRODUCTION_LOCAL_CARRY_REQUIRES_EXPLICIT_WORK_CORRECTION`;
- any gate failure returns `PROTOCOL_INVALID`.

## 4. Interpretation boundary

The energy functional is the selected exact wave-plus-interaction observer used
by FTD-0452, not a demonstrated production total Hamiltonian. This audit can
identify a missing local carry term in that contract. It cannot by itself prove
which term production must apply or that the carried source-site vector is the
entire bound self-field.

## 5. Result

All validity gates passed. The local carry correction has RMS
`1.81999100843571e-5` and maximum magnitude `6.04366710726422e-5`; neither
zero production particle work nor uncorrected endpoint work closes every arm.
Verdict: `PRODUCTION_LOCAL_CARRY_REQUIRES_EXPLICIT_WORK_CORRECTION`.
