# PREREGISTRATION — Localized-basin relaxation v3

**Identifier:** `FTD-0681`  
**Status:** `[PREREGISTERED REPLICATION — NOT HELD OUT]`  
**Date locked:** 2026-07-28  
**Production changes:** forbidden

## Purpose

Replicate the full FTD-0679 trajectory with the corrected FTD-0680 field-index
decoder and repair the target-energy output contract.  FTD-0679 exposed its
core/shell outcome before this protocol, so v3 is a conformance replication,
not a new discovery test.

## Frozen corrections

1. The field observer is frozen to FTD-0680:
   - source SHA256
     `E7BA078079A24A41DF30B39ABEFBBD60897831D60DB4E087DA7CC547274727C7`;
   - asymmetric-origin test SHA256
     `1559C331BA44BA9C70AEF7741CEDC741F6089E7533A77D29D689ED00433FBBEA`.
2. The two target columns are defined exactly as

   ```text
   target_energy(t) = E_target(t),
   target_ratio(t)  = E_target(t)/E_target(0).
   ```

   The embedded reservoir observer returns the ratio, so the runner must write
   `target_energy=initial_target*returned_target` and
   `target_ratio=returned_target`.

No physical threshold or fit changes.

## Frozen dynamics and classifiers

All FTD-0679 dynamics, observer scales, exact gates, fit window, physical
classifiers, and polarity tolerances remain byte-for-byte equivalent in
meaning:

- `L=97`, ticks `0..80`, self-contact tick 81;
- signs `-1,+1` plus evolving control;
- launch `p_max=2.5e-7` in internal doublet `{6,7}`;
- `R_in=8`, `R_out=24`, instantaneous control center;
- fit `log R_core` over ticks `8..64`;
- decline `>=0.20`, positive rate, `DeltaBIC>=10`, `R^2>=0.995`;
- tick-80 remote gate `H_far>H_near` and `H_far>0`;
- polarity rate difference `<=1e-4`, history RMS `<=1e-5`, far-fraction
  difference `<=1e-4`;
- shell partition `<=1e-12`, energy/common residuals `<=1e-10`, inverse
  recovery `<=1e-8`.

The ordered verdicts are the v2 names with `V3` substituted.

## Added conformance gates

1. At tick zero, `target_ratio=1` and
   `target_energy=initial_target`, each within relative `1e-12`.
2. Every row satisfies
   `target_energy=initial_target*target_ratio` within relative `1e-12`.
3. Because the FTD-0679 origin `(48,48,48)` makes its shell radii invariant
   under the corrected x/z decode, each v3 core-ratio and shell series must
   reproduce the corresponding FTD-0679 raw value within relative `1e-12`.
   Failure yields execution-invalid rather than a changed physical verdict.

## Frozen provenance

- FTD-0679 raw JSON SHA256:
  `0C97C77BF036AB742195684B2B059D3C1BB89BB0B24AE9D9BEDFCCC17BA36AD9`;
- FTD-0679 raw CSV SHA256:
  `5B27C13E2C3CBE77BEEC0350FAE5E6B4BA1264CB4BF29F2A196B9471FEA477BA`;
- FTD-0679 status:
  `EXECUTION INVALID — OUTPUT CONTRACT`;
- FTD-0678 status:
  `EXECUTION INVALID — NO DYNAMICS SAMPLED`.

## Run of record

- CTest: `localized_basin_relaxation_v3`;
- outputs under `engine/results/ftd_0681/`;
- runner and Release executable hashes locked before invocation;
- independent standard-library Python verifier recomputes target identities,
  replication residuals, fits, all gates, and the final verdict.

Even a conforming result licenses only a finite-window statement about the
selected connected model.  It does not establish an asymptotic attractor,
bound field dressing, radiation, or a fundamental particle.
