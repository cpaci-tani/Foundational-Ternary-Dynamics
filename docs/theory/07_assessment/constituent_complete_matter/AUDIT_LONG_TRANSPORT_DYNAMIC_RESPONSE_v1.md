# FTD-0768 long-transport dynamic-response clearing — result audit v1

**Status:** `[EXECUTION INVALID — CONTINUOUS REVERSE RECOVERY]`  
**Verdict:** `LONG_TRANSPORT_EXECUTION_INVALID`  
**Run date:** 2026-07-31  
**Audit date:** 2026-08-01  
**Production impact:** none

## 1. Frozen record

The fourth qualified WSL2 CUDA execution produced exactly one immutable
artifact:

- artifact:
  `engine/results/ftd_0768/ftd_0768_long_transport_dynamic_response_v1.json`;
- artifact SHA256:
  `FE7E1915030E16DF896CE89036EC088C80A7C538DB90301DE3BDAA1AA3170C3F`;
- protocol SHA256:
  `5E4D0E9A81BD8C7E901A765792284E1BEF64129791CC874357D22F9630A2F48F`;
- volume `L=321`, formation horizon 160 ticks, preparation age 128 ticks,
  response horizon 768 ticks, and checkpoints every 64 ticks;
- moving branch boost `q=+0.030` along the face direction and matched `q=0`
  rest control.

The result file was serialized at `2026-07-31 21:12:28-05:00`. The three
earlier pre-result aborts remain recorded separately in
[`AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_PREEXEC_v1.md`](AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_PREEXEC_v1.md);
none produced an artifact or physics verdict.

## 2. Independent certification

The frozen independent certificate
`scripts/proofs/proof_long_transport_dynamic_response.py` reconstructs every
registered scalar, interval ledger, checkpoint gate, and outcome from the
artifact. It reports:

```text
artifact_sha256  FE7E1915030E16DF896CE89036EC088C80A7C538DB90301DE3BDAA1AA3170C3F
checks           2221
passed checks    2220
sole failure     reverse continuous recovery
outcome          LONG_TRANSPORT_EXECUTION_INVALID
```

The certificate self-test independently passes `30/30` and reconstructs its
synthetic `CLEARED_LOCAL_RESPONSE_DECAYS` fixture. The failure is therefore a
registered result-gate failure, not a classifier self-test failure.

## 3. Exact invalidating defect

The moving branch returns its discrete state exactly after 768 reverse steps,
but the maximum continuous matter/field recovery error is

```text
3.8786822642578e-9.
```

The preregistered gate is `1e-10`. The result exceeds it by
`38.786822642578` times. The artifact therefore has

```text
reverse_discrete_exact = true
reverse_valid          = false
outcome                = LONG_TRANSPORT_EXECUTION_INVALID.
```

No later response classifier is admissible when execution validity fails.
The tolerance is not changed and the completed artifact is not reclassified.

## 4. Quarantined descriptive record

The following values describe the invalid run but carry no FTD-0768 physics
status:

- parent, aging, rest initialization, moving initialization, and the entire
  forward history are valid;
- all 13 checkpoints from `tau=0` through `tau=768` are internally valid;
- maximum rest-core displacement is `5.6843418860808e-14`;
- the moving center reaches displacement `9.743699...` at `tau=704` and
  `10.63174924610081` at `tau=768`, so the checkpoint observer's spatial
  clearing flag becomes true at those two checkpoints;
- the moving arm records 27 site hops, minimum graph margin
  `0.1768897704908412`, minimum energy margin `0.0009399372401562057`, minimum
  root singular value `0.9865411797246026`, and maximum condition number
  `1.0797485437881993`;
- maximum moving-arm common-action and energy residuals are
  `4.5492743686731396e-14` and `4.348084392535867e-15`;
- fixed-slab and moving-control-volume transport identities, complementary
  boundary quadrature, and mask-sweep quadrature remain at numerical roundoff;
- no complete field download occurs during the registered observer path.

These values cannot be used to choose among `CLEARED_LOCAL_RESPONSE_DECAYS`,
`CLEARED_LOCAL_RESPONSE_PERSISTS`, and `CLEARED_LOCAL_RESPONSE_MIXED`, because
the mandatory state-only forward/reverse conjunction failed first. In
particular, the two descriptively cleared checkpoints do not establish a wake,
radiation, environmental memory, long mobile identity, or a particle.

## 5. Consequence and next admissible work

FTD-0768 is consumed as execution-invalid. It neither closes nor establishes
the underlying long-mobile-matter or cleared-response physics question. The
shorter FTD-0760/0761 finite-time relational-core witnesses remain at their
recorded scope; FTD-0768 does not extend their certified horizon.

A fresh successor may diagnose whether continuous reverse error grows with
horizon, checkpoint cadence, arithmetic backend, or state component, and may
then freeze a corrected algorithm if the defect is implementation-caused. It
may not relax the `1e-10` threshold, edit this artifact, or promote the
descriptive clearing rows. No successor is registered here.

Production tick rules, primitives, defaults, toggles, scenarios, constants,
`RenderBridge`, particle claims, wake labels, Lorentz claims, and unitarity
claims are unchanged.
