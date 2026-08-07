# FTD-0746 causal-horizon environmental persistence — pre-execution audit v1

**Identifier:** FTD-0746  
**Status:** `[PRE-EXECUTION CONFORMANCE PASS — NO PHYSICS RESULT]`  
**Date:** 2026-07-29  
**Protocol:**
[`PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md`](../../10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md)

## Verdict

The held-out `L=321`, `T=312` implementation conforms to the locked protocol
and is ready for exactly one fresh execution of each command-selected
`face`, `edge`, and `body` arm. No physics result existed when this audit was
written. The no-argument smoke path produced no campaign artifact.

The campaign is a forward environmental-persistence test. It intentionally
does not rerun the already qualified bound, polarity, or inverse controls and
licenses no long-horizon reversibility statement.

## Frozen artifacts

| Artifact | SHA-256 |
|---|---|
| protocol | `B98DB9B18050D1799814ABD0B6C70936BF631AEF258CF969FC8D15E7B8DCA9A0` |
| FTD-0745 discovery CSV | `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C` |
| C++ runner source | `2D98C02E2A7968189B2E5A1B87B4AC96ED5B0A4D6964AA3987250714F89276DD` |
| WSL2 Release executable | `AE7542832247A7FB6BD09FB752AFDAFEED55636FCFFBEF67BFDC17034E6193B7` |
| static conformance proof | `07C74D7A6C66E9B260EA846BE9E7616D2DBD865912DB3ED210806F918581314D` |

The protocol hash and frozen FTD-0745 hash are embedded in the runner. The
runner writes one CSV and one standard JSON summary per arm. Optional
non-finite diagnostics serialize as JSON `null`; a non-finite required gate
still makes execution invalid.

## Conformance checks

- `python scripts/proofs/proof_causal_horizon_environmental_persistence_protocol_conformance.py`
  passes `64/64` checks.
- The target builds in pinned Windows Release and in the existing WSL2 Release
  CUDA-enabled build tree.
- The Windows and WSL2 no-argument smoke invocations pass and emit no campaign
  CSV or JSON.
- The runner fixes `L=321`, horizon `312`, contact tick `313`, support radius
  `4`, all six observer shells, the FTD-0745 prefix through tick `184`, the
  core and near-field windows, the radius-48 arrival deadline, and the
  post-arrival no-return window.
- The runner contains no reverse solver and declares `inverse_tested: false`.

## Resource and execution lock

One live arm is expected to occupy roughly half of the WSL2 virtual machine's
30 GiB memory allocation. The three arms therefore run serially. Resource
scheduling may not change inputs, gates, horizon, arm count, or verdict order,
and a result observed in an earlier arm may not cancel a later arm.

The clean result set must contain exactly 313 forward rows per arm, 939 rows
in total. After all arms finish, a separate record proof will parse the raw
standard JSON and independently reconstruct every gate before the result is
entered as physics evidence.

## Scope guard

Passing would establish only a finite-horizon, causal-buffer environmental
persistence witness for the selected reciprocal `(s,C,F)` dynamics. It would
not establish an asymptotic soliton, invariant basin, autonomous moving
particle, physical radiation field, native reduction, or uncontained-space
limit. Failing a locked gate closes this candidate at the first failed gate;
it does not authorize a threshold repair or post-hoc rerun.
