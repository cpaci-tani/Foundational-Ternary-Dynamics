# PREREGISTRATION — Causal excitation separation v2

**Identifier:** `FTD-0685`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CORRECTION AND EXECUTION]`  
**Date locked:** 2026-07-28  
**Production changes:** forbidden; selected observer-only campaign

## Reason for the new version

FTD-0684 stopped before its tick-zero observation and before every dynamical
solve.  Its only failed preflight required the recentered composite mean to be
bitwise equal to `(64,64,64)`.  The independently compiled initialization
probe measures

```text
center = (63.999999999999986,64,64)
norm(center-origin) = 1.4210854715202004e-14.
```

The component-aware zero profile, localized-basin zero state, normalization,
analytic modes, reservoir decomposition, and fresh excitation all pass.  The
reservoir target energy is positive (`1.749144045293269e-13`).  FTD-0684 then
skipped initialization only because `initial_fields_equal` was incorrectly
seeded from the exact-center boolean.

## Sole prospective correction

Replace the three bitwise center equalities with

```text
norm(measured_center - fixed_origin) <= 1e-12.
```

The fixed shell origin remains exactly `(64,64,64)`; no shell moves with the
measured center.  Keep observer preflight and initial-fields equality as
separate booleans.  The latter is still bitwise for all face/edge arrays.

## Frozen inheritance

Every FTD-0684 physical choice and outcome definition is inherited without
change:

- `L=129`, horizon 112, contact tick 113, fixed origin `(64,64,64)`;
- signs `-1,+1`, same dressed control, fresh `p_max=1.25e-7`;
- radii `{8,16,24,32,40,48}` and arrival threshold `0.001`;
- component-aware FTD-0683 profile and signed FTD-0671 regional ledger;
- late window `88..112`, all plateau thresholds, spatial/late classes;
- target schema, exact gates, polarity gates, energy/common tolerances, and
  state-only reversal.

No FTD-0684 physical result exists to reproduce or avoid.  V2 is held out with
respect to every dynamical and morphology value.

## Frozen provenance

- FTD-0684 protocol SHA256:
  `CA82DDAFC93AB7FB339EB3D2186B3C974E8602EE8BA4AD295FC9E3DE6A6A589E`;
- FTD-0684 executed runner SHA256:
  `08A789677A577859A37DF1881290108FC8233C09F7D50FCE6F55CE9EB4134D11`;
- FTD-0684 executed binary SHA256:
  `97DB3E8C83B34EF2CC799788999C4DF6D012FA22E33EA2BEB9637B241A21B613`;
- FTD-0684 initialization-only JSON SHA256:
  `25C05D32C04FD3EC994E7A959536FFF724B0BFA9245873D578F2CCEDC3A455A7`;
- FTD-0684 initialization-only CSV SHA256:
  `606D23C03FE98A94E9EDEB9FB53F6E678F3482B9A2396D300BC626056D09BA3F`;
- diagnostic source SHA256:
  `4702AFAFF1DA4630F9DA6047960C23CA300F397F269020DF89B61B9C10B0DD75`.

## Run of record

- planned CTest target: `causal_excitation_separation_v2`;
- outputs: `engine/results/ftd_0685/` JSON and long-form CSV;
- runner and Release executable hashes must be locked before invocation;
- independent certification remains mandatory before interpretation.
