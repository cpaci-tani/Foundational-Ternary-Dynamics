# FTD-0768 long-transport dynamic-response clearing — implementation audit v1

**Status:** `[HISTORICAL PRE-EXECUTION AUDIT — THREE PRE-RESULT ABORTS; FOURTH RUN LATER EXECUTION-INVALID]`  
**Superseded execution status:** [`AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md`](AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md)  
**Date:** 2026-07-31  
**Scope:** observer and campaign infrastructure only; no production, ontology, or physics-result change

## 1. Locked protocol

The controlling protocol is
[`PREREG_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md`](../../10_eft_program/preregistrations/constituent_complete_matter/PREREG_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md),
whose SHA256 is
`5E4D0E9A81BD8C7E901A765792284E1BEF64129791CC874357D22F9630A2F48F`.
It remains unchanged. The registered run is one aged `L=321`, 768-tick
rest/moving face pair with fixed checkpoints, a `d>=9` clearing gate,
paired actual/residual response, regional energy transport, and complete
moving-arm state-only reversal.

## 2. Qualified implementation

The following observer-side source files now exist in the worktree:

- `engine/include/ftd/eft/paired_field_response.h`;
- `engine/include/ftd/eft/cuda_paired_field_response.h`;
- `engine/src/eft/paired_field_response.cpp`;
- `engine/cuda/cuda_paired_field_response.cu`.

The current implementation defines the four locked regions, reconstructs the
integer-time magnetic field from the matched half-step field, builds the
selected compact bound representative independently for rest and moving
states, and reduces the following quantities separately for actual and
actual-minus-bound fields:

```text
Delta U = U(F_q)-U(F_0),
U_delta = (1/2)||F_q-F_0||_H^2,
U_cross = <F_0,F_q-F_0>_H,
r_energy = Delta U-U_cross-U_delta.
```

The CUDA path is designed to keep complete fields resident and return only
regional scalar reductions. A separate resident regional-energy observer
records pre-current boundary transport and current-source exchange using the
same modified field-energy density as the matched pipeline.

The implementation is wired into the isolated `ftd_cuda_m3_observer` research
library and `ftd_eft`. It does not enter the production tick or `ftd_core`.

The focused WSL2 RTX 5090 CTest `cuda_paired_field_response` passes with zero
failures in 0.49 seconds. Its current `L={17,33}` matrix establishes:

- exact zero response for identical pairs;
- direct reconstruction of independently injected one-face electric and
  one-edge magnetic differences;
- the signed-energy/interference/difference-norm identity for actual and
  selected-residual channels in all four regions;
- CPU/CUDA scalar parity;
- integer translation, charge conjugation, proper cubic rotation, and
  reflected signed-pair covariance;
- resident regional modified-energy CPU/CUDA parity with nonzero boundary and
  source channels;
- complementary inside/outside boundary quadrature with equal-and-opposite
  source-free transport and an explicit orientation residual;
- exact discrete moving-control-volume transport: old/new mask energy on the
  same before-state, complementary mask sweep, endpoint energy change, and the
  discrete Reynolds identity;
- explicit affine, quadratic, and mixed polynomial response identities;
- zero complete-field downloads.

This closes the complete registered observer qualification matrix.

The independent FTD-0768 classifier/interval/momentum/moving-volume self-test
returns `CLEARED_LOCAL_RESPONSE_DECAYS` with 30/30 checks on its constructed
fixture. The inherited
artifact certificates remain green: FTD-0763 `175/175`, FTD-0764 `1524/1524`,
FTD-0766 `404/404`, and FTD-0767 `113/113`.

## 3. Superseded second-attempt identity

| Artifact | SHA256 |
|---|---|
| `engine/include/ftd/eft/paired_field_response.h` | `BEFAEB9DF5281B0699C54492D14CC31A912EE55D58445369A7EC4C843B2095A9` |
| `engine/include/ftd/eft/cuda_paired_field_response.h` | `ACEF7F67885F19726EB476F0EFB7C7097ED7C9AD360ED904AD1BF71F1F2DCAF5` |
| `engine/src/eft/paired_field_response.cpp` | `E0469DB5514DCDBE69654FE5A409C60AFE94633A2D157B9C699BAA474E33CD87` |
| `engine/cuda/cuda_paired_field_response.cu` | `6B84558E06D82B79C7D5906A664A95377BE1E874498B4FBB79B5D027507F631D` |
| `engine/tests/test_cuda_paired_field_response.cpp` | `ECE549E31964A54B0019C201383809E5ACCAF00159E4B00C211E19FC87D77FF5` |
| `engine/tests/campaign_long_transport_dynamic_response_cuda.cpp` | `44B280DBE328DCFAD61104C28EE010C91DDEE28DC34687CB7F634BDF9F951FA6` |
| `scripts/proofs/proof_long_transport_dynamic_response.py` | `A4D6238C537407155C8888A425E5245E3FD0C5D20170192F25CB691023F8774B` |
| WSL2 executable `engine/build_wsl/campaign_long_transport_dynamic_response_cuda` | `1206D653584390DA0BD2286320209E0B4D86BCE5703CEF034D4FDC8D1598239D` |

These hashes identify the second pre-result attempt exactly. They are retained
as provenance but are no longer the authorized execution identity because the
runner and certificate require the repairs in section 4. The runner refuses
execution if `engine/results/ftd_0768` already exists. It writes no artifact
until formation, aging, both 768-tick forward arms, and the full moving-arm
reverse attempt have returned. The certificate is outcome-neutral: it verifies
an invalid result without converting it into a valid physics result.

### Superseded third-attempt identity

| Artifact | SHA256 |
|---|---|
| `engine/include/ftd/eft/paired_field_response.h` | `BEFAEB9DF5281B0699C54492D14CC31A912EE55D58445369A7EC4C843B2095A9` |
| `engine/include/ftd/eft/cuda_paired_field_response.h` | `ACEF7F67885F19726EB476F0EFB7C7097ED7C9AD360ED904AD1BF71F1F2DCAF5` |
| `engine/src/eft/paired_field_response.cpp` | `E0469DB5514DCDBE69654FE5A409C60AFE94633A2D157B9C699BAA474E33CD87` |
| `engine/cuda/cuda_paired_field_response.cu` | `6B84558E06D82B79C7D5906A664A95377BE1E874498B4FBB79B5D027507F631D` |
| `engine/tests/test_cuda_paired_field_response.cpp` | `63E0B24684F962DAE9A83A16EE97F3610B2E41CC4414B3137DF2DD5E2C5249AE` |
| `engine/tests/campaign_long_transport_dynamic_response_cuda.cpp` | `E9498A3CB640FAF67F5123E5117E1B0A720BDF7B5608B588FB6EE7F58F2EC9E4` |
| `scripts/proofs/proof_long_transport_dynamic_response.py` | `3368A06D63FF0BCC57CBB3C0DC213FF0F7962F707E73F45E25E0DD8CC49F6AB0` |
| WSL2 executable `engine/build_wsl/campaign_long_transport_dynamic_response_cuda` | `E4CB1DA840E95675F42BE6B183080D47311B23BB5C348ED5CFDF0C8831569137` |

The protocol hash remains unchanged. The third-attempt runner recorded its
schema, field representation, observer mode, tolerances, every-tick maximum
rest displacement, measured checkpoint momentum candidates and their
initial-total defects, and complete discrete reversal metadata. The
independent certificate reconstructs every checkpoint interval and both
boundary orientations.

### Qualified fourth-attempt identity

| Artifact | SHA256 |
|---|---|
| `engine/include/ftd/eft/paired_field_response.h` | `46EEEAEB748291A51B06F27B4E6BE409ED01DD4CAE90AB6789A7374D89A4B3DC` |
| `engine/include/ftd/eft/cuda_paired_field_response.h` | `ACEF7F67885F19726EB476F0EFB7C7097ED7C9AD360ED904AD1BF71F1F2DCAF5` |
| `engine/src/eft/paired_field_response.cpp` | `EA0519E08597398A15FB0883E398E4658C707E0F366B35087E544F84454C3953` |
| `engine/cuda/cuda_paired_field_response.cu` | `6B84558E06D82B79C7D5906A664A95377BE1E874498B4FBB79B5D027507F631D` |
| `engine/tests/test_cuda_paired_field_response.cpp` | `DA00C4D8ACFAE0862A9332A825E34B8A22F0CA985500708D7390C6FF6946BF3F` |
| `engine/tests/campaign_long_transport_dynamic_response_cuda.cpp` | `B79970999A90F3E2C516F823C97DCFF2CEE1BEF9254D942A6306FE33C7907E95` |
| `scripts/proofs/proof_long_transport_dynamic_response.py` | `A5F65A2E450535099E989952D7F25C1922AD2D1A0AC72B7FB798830098F4D3A7` |
| WSL2 executable `engine/build_wsl/campaign_long_transport_dynamic_response_cuda` | `4629E6843E320B20DF55B8F0169803870A26E3E57D22B36293956FB8EE79BF64` |

The fourth-attempt record adds `mask_sweep`, its independently reduced
complement, initial and endpoint region energies, transported energy change,
per-step and cumulative transport identities, and endpoint-chain residuals.
The moving-control-volume ledger is therefore no longer inferred by summing
different fixed masks.

## 4. Execution authorization and absent-result provenance

Immediately before the first launch, `engine/results/ftd_0768` was absent and
no FTD-0768 process was active. At `2026-07-31T14:01:22-05:00`, after
rechecking the then-frozen hashes, the WSL2 executable was launched as

```text
./campaign_long_transport_dynamic_response_cuda --run
```

During the live implementation audit, the regional ledger was found to name
the source-free regional energy change as boundary transport without the
independent complementary boundary quadrature required by preregistration
section 5. The process was terminated before formation completed, before any
registered checkpoint printed, and before any result directory or artifact
existed. This was an infrastructure abort, not a scientific result.

The observer now partitions every face/edge contribution between the region
and its complement and requires

```text
Phi_into(Omega) + Phi_into(complement Omega) = 0
```

within the frozen tolerance, independently of the local current/source split.
The revised CPU/CUDA test passes in 0.42 seconds with a nonzero oriented
transport fixture. The corrected hashes above were frozen at
`2026-07-31T14:07:28-05:00`. After another absent-result/process/hash check,
the corrected executable launched at `2026-07-31T14:09:47-05:00`.

That second attempt was also terminated before formation completed, before a
registered checkpoint, and before any result directory or artifact existed.
The continuing audit found that the checkpoint serializer wrote zero-valued
local and spline field-momentum candidates at `tau=0` rather than measuring
them directly from the initial fields. It also found that the independent
certificate checked cumulative regional channels but did not reconstruct and
certify equation (7) on every checkpoint interval. Because both records are
explicit preregistration requirements, allowing the attempt to finish would
have produced an uncertifiable artifact. This is a second infrastructure
abort, not a physics result and not consumption of the registered campaign.

The completion audit required, before relaunch:

- direct field-momentum measurement at every checkpoint, including `tau=0`,
  with defects referenced to each arm's measured initial total;
- interval-by-interval regional balance reconstruction in the independent
  certificate, including complementary-boundary orientation;
- every-tick rest-displacement enforcement and complete discrete metadata in
  the forward/reverse equality predicate;
- the explicitly preregistered affine, quadratic, and mixed response fixtures;
- refreshed source, certificate, and executable hashes after all repairs.

All five items are now implemented. The expanded focused CTest passes in
0.47 seconds; the classifier/interval/momentum self-test passes 10/10; the
inherited `175/175`, `1524/1524`, `404/404`, and `113/113` certificates remain
green; `git diff --check` is clean for the touched implementation and
documentation.

Immediately before the third launch, no FTD-0768 process or result directory
existed and the qualified hashes above matched. At
`2026-07-31T14:25:52.8381892-05:00`, the WSL2 CUDA executable launched with
Windows host PID `36524` and WSL process PID `423`.

The third attempt was terminated at
`2026-07-31T14:44:51.2769202-05:00`, again before any checkpoint, result
directory, or artifact. The fixed laboratory slab ledger was valid, but the
moving radius-eight region was recentered on every later constituent center.
Each one-step observation used one fixed later-centered mask on both the
before and after fields, so its local balance was exact. Summing those steps,
however, changed masks without recording the energy swept into or out of the
control volume. Therefore the accumulated moving-region ledger did not equal
the endpoint energy change required by the protocol's moving-boundary clause.

For energy density `h_t(x)` and region indicator `chi_t(x)`, the missing exact
term is exposed by

```text
sum_x chi_(t+1) h_(t+1) - sum_x chi_t h_t
 = sum_x chi_(t+1) (h_(t+1)-h_t)
 + sum_x (chi_(t+1)-chi_t) h_t.
```

The second term is the discrete Reynolds/mask-sweep contribution. The repaired
runner measures it from the old and new masks on the same resident before-
state, measures the complementary outside sweep independently, and checks the
endpoint-to-endpoint identity at every step and checkpoint interval. The
focused WSL2 CTest passes 1/1 in 0.49 seconds, the expanded self-test passes
30/30, all inherited certificates remain green, `git diff --check` is clean,
and the result directory remains absent. The third attempt has no physics
status and did not consume the registered outcome.

## 5. Epistemic boundary

Nothing at this checkpoint establishes long mobile identity, laboratory-slab
clearing, persistent response, wake formation, momentum closure, radiation,
or a particle ontology. The complete preregistered pre-execution qualification
passes under the qualified fourth-attempt hashes; that is an infrastructure
result only. After an absent-result/process/hash check, the fourth WSL2 CUDA
execution launched at `2026-07-31T14:53:12.8215412-05:00` with Windows host
PID `72608` and WSL PID `464`. It is active and has no artifact or physics
status until its registered output exists and the independent certificate
passes.
Production tick rules, primitives, defaults, toggles, scenarios, constants,
and `RenderBridge` remain unchanged.

## 6. Subsequent result

The fourth execution later completed and produced the immutable registered
artifact. Its independent certificate passes `2220/2221` checks but rejects
continuous reverse recovery (`3.8786822642578e-9` against `1e-10`), yielding
`LONG_TRANSPORT_EXECUTION_INVALID`. This pre-execution document remains the
provenance record for the three aborts and qualification only; the current
status is controlled by the result audit linked above.
