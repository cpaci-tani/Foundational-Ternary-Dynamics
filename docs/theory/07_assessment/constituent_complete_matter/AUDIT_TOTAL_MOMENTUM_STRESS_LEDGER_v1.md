# FTD-0769 total momentum stress ledger — result audit v1

**Status:** `[EXECUTION INVALID — INHERITED CONTINUOUS REVERSE RECOVERY]`
**Verdict:** `MOMENTUM_LEDGER_BASELINE_INVALID` (all three axes)
**Run date:** 2026-08-02
**Audit date:** 2026-08-02
**Production impact:** none

## 1. Frozen record

The qualified WSL2 CUDA execution produced exactly one immutable artifact:

- artifact:
  `engine/results/ftd_0769/ftd_0769_total_momentum_stress_ledger_v1.json`;
- artifact SHA256:
  `544E6A9A9273438A212DC9B61D2BF5C47C11DEF3611853754DCC19950735D24F`;
- companion CSV SHA256:
  `7B6A2D851DEC6233021D48A171A1E6E92F09D7B2E5B0CFE51C3FE4D6EEB1FE5D`;
- protocol SHA256:
  `215B03A85A76B706E91099CA24E276FAC3B57DE3852353981456F79F411D8A13`;
- volume `L=321`, formation horizon 160 ticks, preparation age 128 ticks,
  discovery horizon 768 ticks, checkpoints every 64 ticks;
- moving branch boost `q=+0.030` along the body ray `direction=[0,0,1]`, the
  same FTD-0760/0761/0768 parent construction the pre-registration §5 froze.

The exactness pre-check (`L=11`) and §8 firewall (`L=17`) both passed
immediately before the run, on the WSL2/RTX 5090 build, matching the native
Windows build's results exactly (`site_channel_under_component_mask =
33.248864317517494` both places; firewall `identity=3.64e-22`,
`parity=3.94e-22`, `probe_parity=9.53e-22`, `probe_bonds=true`).

## 2. Independent certification

The new independent certificate
`scripts/proofs/proof_total_momentum_stress_ledger_result.py` reconstructs
every registered scalar and the outcome from the artifact's raw fields (not
trusting the engine's self-reported `outcome` string), and separately
cross-checks the artifact's shared forward/reverse fields against the local
FTD-0768 parent artifact. It reports:

```text
certificate_checks     29
certificate_failures   []
outcome                MOMENTUM_LEDGER_BASELINE_INVALID
reverse_recovery       3.8786822642578045e-09
reverse_gate           1e-10
cross_check (8 fields) 0 mismatches vs FTD-0768
passed                 true
```

`--self-test` independently passes `3/3` synthetic-fixture checks
(baseline-invalid reproduction, a clean-reverse fixture correctly reaching the
physics-gate branch, and a firewall failure correctly classified as
infrastructure-unresolved), so the classifier's own logic is exercised before
being trusted against the real artifact.

## 3. Exact invalidating defect — inherited, not new

The moving branch returns its discrete state exactly after 768 reverse steps,
but the maximum continuous matter/field recovery error is

```text
3.8786822642578045e-9.
```

The preregistered gate (§6.3 G2, frozen unchanged from FTD-0768, **not
loosened** per Banned move B4) is `1e-10`. The result exceeds it by
`~38.79`x — the exact multiple FTD-0768 reported.

This is not a new defect in the momentum-ledger instrumentation. Every shared
forward/reverse field is **bit-identical** to FTD-0768's own artifact:

| field | FTD-0768 | FTD-0769 |
|---|---|---|
| `moving_initial_hash` | `b69929895d39426c` | `b69929895d39426c` |
| `moving_forward_final_hash` | `997362e823bf7b3d` | `997362e823bf7b3d` |
| `moving_reversed_hash` | `b83cc04eb496a415` | `b83cc04eb496a415` |
| `reverse_recovery` | `3.8786822642578045e-09` | `3.8786822642578045e-09` |
| `reverse_discrete_exact` | `true` | `true` |
| `reverse_valid` | `false` | `false` |
| `reverse_steps` | `768` | `768` |
| `reverse_maximum_common` | `4.5498327327919696e-14` | `4.5498327327919696e-14` |

The new stress-ledger observer is purely additive instrumentation (§0 of the
pre-registration): it reads the frozen forward/reverse trajectory, it does not
alter it. The pre-registration's own §6.3 anticipated exactly this outcome
before execution, quoting FTD-0768's `reverse_recovery` value verbatim and
pre-committing to `MOMENTUM_LEDGER_BASELINE_INVALID` "if the same near-miss
recurs." It recurred, bit-for-bit.

```text
reverse_discrete_exact = true
reverse_valid          = false
outcome                = MOMENTUM_LEDGER_BASELINE_INVALID (all three axes)
```

Per §7 item 2 of the verdict map, a G2 inherited-execution-validity-gate
failure is checked immediately after infrastructure/pre-check gates and before
any of the §7 items 3–11 physics buckets. No later bucket, discriminator, or
localization comparison is evaluated once G2 fails; the per-axis `verdicts`
array confirms this directly (`qualifying_checkpoints: 0`,
`L1_bucket`/`L2_bucket` empty, on all three axes).

## 4. Quarantined descriptive record

The following values describe the invalid run but carry no FTD-0769 physics
status:

- parent, aging, rest initialization, moving initialization, and the entire
  forward history are valid; all 13 checkpoints from `tau=0` through `tau=768`
  are internally valid;
- forward displacement reaches `10.63174924610081` sites at `tau=768`,
  identical to FTD-0768's descriptive trajectory;
- `boundary_margin = 4.0795289339388034 > 0` (no periodic-boundary contact);
  `far_field_active = false`; `prior_mismatch = false`;
- rest-arm gates are clean (`rest_arm_clean: true` on every axis) — the
  invalidating defect is isolated to the moving branch's continuous reverse
  recovery, not to the rest control;
- `exchange_sign_inverted: false` on every axis, but this flag is only
  meaningful "if items 1–5 all pass" (§7) — it is a vacuous default here, not
  a measurement, and must not be read as a physics result.

These values cannot be used to adjudicate `MOMENTUM_LEDGER_CORE_RETAINED`,
`MOMENTUM_LEDGER_THROUGH_FLOWING`, `MOMENTUM_LEDGER_NEAR_ZONE_ACCUMULATING`,
`MOMENTUM_LEDGER_OVER_DEPLETING`, or `MOMENTUM_LEDGER_MIXED`, because the
mandatory G2 state-only forward/reverse conjunction failed first (§9: "Only
items 7–11 of §7 are physics verdicts, and only after G0–G7 all pass").

## 5. Consequence and next admissible work

FTD-0769 is consumed as execution-invalid. It establishes neither closure nor
non-closure of total matter+field momentum for the FTD-0760/0761 moving-core
family, and neither confirms nor refutes the regional-localization question
Arc 2 set out to answer. The regional momentum-transport identity (M1/M2) and
the η−τ≡1 corollary derived in the pre-registration's own §2 remain valid
mathematics — they are proven independently of any engine run and are
untouched by this execution failure — but they license no claim about *this*
moving-core family's momentum content, because the family's own dynamical
record never reaches a state execution validity accepts.

Per §8 of the pre-registration: "Interrupted or failed modes are not tuned or
rerun under this pre-registration; a fresh `v2` would be required." No
successor is registered here. A fresh `v2` could shorten the discovery horizon
below whatever threshold keeps `reverse_recovery` under `1e-10` (FTD-0768's
own note that the defect is horizon-dependent, per its §5 "Consequence and
next admissible work"), change the arithmetic backend, or diagnose the defect
directly; it may not relax the `1e-10` threshold, edit this artifact, or
promote any of the quarantined descriptive rows above.

Production tick rules, primitives, defaults, toggles, scenarios, constants,
`RenderBridge`, particle claims, momentum claims, and Lorentz claims are
unchanged.
