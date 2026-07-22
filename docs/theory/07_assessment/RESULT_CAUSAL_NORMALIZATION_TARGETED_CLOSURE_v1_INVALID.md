# FTD-0403 v1 — Targeted causal-normalization closure result

**Frozen outcome:** **INVALID — boundary-movement instrument incompatible with the selected causal domain**

**Lock:** `preregister-causal-normalization-targeted-closure-v1` at commit `39f668e46fb94a4aa4724c90ca2f2cdb3142fa90`; preregistration SHA256 `09a73ab988a8d54c3bca283c1500406cca4e9e98aa8ac93a05efe375a29bb61a`.

## Verdict

T1 passed: the exact verifier closed A1–A7 and S1–S9. The fresh T2 run passed 13 of 14 targets. `boundary_movement` failed two assertions because its historical fixture directly assigned `velocity.x=-1` and expected a one-tick face crossing. Under FTD-0402, movement entry correctly projects that externally mutated velocity inside `C_SPEED=1/sqrt(3)`; the resulting one-tick remainder has magnitude below one, so no face crossing occurs and the reflective velocity does not flip.

The failure is an instrument-domain error, not evidence that the causal projection failed. Nevertheless, v1 froze every relevant native-test failure as `INVALID`; it cannot be narrated into closure. T3–T6 were not run after the verdict became mechanically fixed.

## Licensed consequence

FTD-0402 remains `PARTIAL` and `§12-cnorm` remains open under this v1 result. A v2 attempt requires an explicit test-fixture repair that exercises boundary crossing with an in-budget velocity plus accumulated movement remainder, followed by a fresh lock. No production engine behavior, framework type, or physics claim changes.
