# Audit — finite-support environmental closure pre-execution v1

**Identifier:** FTD-0745  
**Status:** `[PRE-EXECUTION CONFORMANCE PASS — NO PHYSICS RESULT]`  
**Date:** 2026-07-29  
**Production status:** unchanged

**Post-execution note:** the authorized clean run subsequently completed. Its
verdict is `ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL`; see
[`AUDIT_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_v1.md`](AUDIT_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_v1.md).
This document remains the historical pre-execution record.

## 1. Verdict

The corrected FTD-0745 held-out M2 runner compiles and conforms to the locked
protocol. One initial process was stopped before serialization after a
verdict-routing defect was found by source review. It wrote no CSV or JSON and
therefore produced no inspectable history or environmental-closure verdict.

## 2. Frozen inputs

| artifact | SHA-256 |
|---|---|
| locked protocol | `D5FB9923FCBF69E2DFD75300FEE4C381AE28EAA10843BF0D52B2D60FCE456888` |
| FTD-0739 discovery-prefix CSV | `E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622` |
| held-out C++ runner | `7F2205D688A53EF802126FB529C560D1B743BCC896D8DBE769DC54BDDD28776E` |
| Release executable | `B140CE3047A7EA263FF4F2829CD80FCD7C2122EA19EA12822900A2CFB31A6688` |
| static conformance proof | `9A4ABE574EF1F7D283C8744E5DF9C6C73A0CD8A9BD0DA6F50A33A8CCFA906112` |

The runner embeds both the protocol and discovery-prefix hashes. Any source or
executable change before execution requires a new pre-execution audit.

## 3. Checks

- static protocol/source certificate: `63/63 PASS`;
- pinned MSVC 14.44 Release compile: pass;
- already qualified FTD-0686 batched observer unit CTest: `1/1 PASS`;
- completed candidate dynamics executions: `0`;
- aborted pre-serialization processes: `1`;
- result CSV/JSON: absent by design.

The static certificate checks the volume/horizon, causal contact formula,
five-history matrix, exact dynamics options, six shells, discovery/validation
split, prefix comparison, late core/near-field gates, ordered arrival, no-return
test, control/polarity checks, output schema, and every registered verdict
token.

## 4. Pre-result implementation amendment

The locked protocol assigns a source detected outside the registered support
radius to `ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL`. The initial runner instead
included that observer failure in its exact/infrastructure conjunction, which
would have emitted `ENVIRONMENTAL_CLOSURE_EXECUTION_INVALID`. Source review
found the mismatch while the first process was still running. The process was
terminated, including its child process, before output serialization; the
`engine/results/ftd_0745` result files remained absent.

The amendment changes only that failure routing and makes non-finite
not-yet-arrived shell summaries valid JSON `null` values. It does not change the
protocol, dynamics, histories, thresholds, observables, or verdict order. Two
new static checks pin the corrected routing. The corrected runner and
executable hashes above supersede the aborted implementation.

## 5. Authorized execution

Exactly one fresh execution of
`test_finite_support_environmental_closure.exe` is authorized. The result must
be independently reconstructed from its CSV/JSON before any ledger status is
changed from pre-execution.

## 6. Claim boundary

This audit certifies instrumentation conformance only. It supplies no evidence
for environmental closure, metastability, a particle, a bound state, charge,
mass, spin, statistics, Lorentz recovery, or native reduction.
