# FTD-0747 CUDA result audit v2

**Status:** `[MIXED — FACE/EDGE PREFIX DRIFT; BODY CONSTRUCTIVE]`  
**Overall conjunction:** `[CLOSED NEGATIVE FOR THE FROZEN CUDA PORT]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`1FB4A49897D8FEC333C686A54D44A90EA6E51D799EDBD9168F8D313287F4FD5F`

## Execution record

The frozen WSL2 CUDA executable was invoked exactly once for each held-out
`face`, `edge`, and `body` arm. All arms reached tick 312 and serialized 313
CSV rows plus one JSON summary. Runtime was 532.4 s, 468.7 s, and 445.2 s,
respectively. No CPU fallback or post-run retuning occurred.

The independent serialized-record certificate passes `52/52` integrity
checks while explicitly printing the face/edge H1 gates and the three-arm
constructive conjunction as physics failures. Its success exit certifies the
frozen mixed/negative result; it does not relabel the failed physics gates.

## Registered verdict

| arm | exact discrete prefix | maximum scalar prefix difference | H2--H5 | independent verdict |
|---|---:|---:|---:|---|
| face | fail | `6.6578e-11` (pass) | all pass | `CAUSAL_HORIZON_PREFIX_DRIFT` |
| edge | fail | `7.8620e-11` (pass) | all pass | `CAUSAL_HORIZON_PREFIX_DRIFT` |
| body | pass | `8.9874e-11` (pass) | all pass | `CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE` |

The face mismatch is confined to `source_entries`: 158 of the 185 registered
prefix rows differ, over ticks 25--70, 72--87, and 89--184. The edge mismatch
is also confined to `source_entries`: 73 rows differ over ticks 100--107,
109--121, 128--137, 140--154, 156--160, 162--176, and 178--184. Every other
registered discrete prefix field is exact. The body support cardinality is
exact for the full prefix.

This is a real protocol failure. The CUDA sparse-current path does not preserve
the CPU instrument's exact current-support cardinality on two ray classes,
even though all registered scalar observables remain within `1e-10`. The
locked protocol requires both conditions, so later constructive behavior
cannot override the failure.

## Constructive behavior below the failed gate

All three arms independently pass execution, persistent-core, stable-near-field,
radius-48-arrival, and post-arrival gates. Radius-48 arrival occurs at tick 297
in every arm. Persistent negative-core onset is tick 80 (face), 96 (edge), and
115 (body). The late radius-eight energy ranges are:

- face: `0.00203312..0.00233521`;
- edge: `0.00188881..0.00215203`;
- body: `0.00179397..0.00214435`.

Final radius-48 outside energies are `1.63705e-7`, `1.64394e-7`, and
`1.64604e-7`. Maximum common-action residuals are below `5.37e-14`; maximum
energy residuals are below `6.58e-15`; maximum recoil defects are below
`5.21e-15`; and maximum regional residuals are below `5.56e-17`.

These facts support CUDA execution fidelity and show environmental persistence
for the body arm. They do not establish the registered three-ray conjunction.

## Serialized-verdict loader defect

The frozen WSL2 runner retained the carriage return on the last header field of
the Windows CRLF FTD-0745 baseline. Baseline loading therefore failed before
comparison and each raw JSON records `prefix_scalar_difference: null` with a
runner-generated prefix-drift token. Those raw tokens are invalid as gate
adjudications. The independent Python certificate normalizes CRLF, reconstructs
all H0--H5 gates directly from frozen CSV rows, and supplies the verdicts above.

After all artifacts were frozen, the loader was repaired by stripping a final
record `\r`. `causal_horizon_csv_loader` now verifies CRLF parsing and all 185
baseline rows for all three directions. The FTD-0747 executable and result
artifacts were not rebuilt or rerun.

## Claim boundary

FTD-0747 establishes that the CUDA field/observer implementation is fast and
numerically close to the CPU reference, but not yet a representation-identical
replacement for the registered sparse-current support diagnostic. The body-ray
environmental-persistence result is constructive under the frozen test. Face
and edge remain negative at H1. No production default, ontology, or physical
claim is promoted.

## Frozen result hashes

- face CSV: `F4C2F19794D884E3371E651C7DD7FC616D6CDF513B23804CD3224D2DFA3F3BC1`
- face JSON: `646145CFDCF1FAD622C7149AEE21EE060ECD2752D788E5D69835DC6AC50DA1F4`
- edge CSV: `F73592E08B1FB3FC7F000813D02F9AF8A89350CEF00ADFCDF5B2A3A1F9ED6876`
- edge JSON: `18FBF0E00EDDF13F34EE0EB37046C0F0FE71299AB0E7F8F4BAE9BBCF1E7D7092`
- body CSV: `87B76B3BE4E44DAD538CFB6988B003C5266B930EE335679046F713D9E31B48AE`
- body JSON: `A97262B5CDB70FB77B192CA74E4A41A73287F121CE0BB37A42488DC178B7F821`
