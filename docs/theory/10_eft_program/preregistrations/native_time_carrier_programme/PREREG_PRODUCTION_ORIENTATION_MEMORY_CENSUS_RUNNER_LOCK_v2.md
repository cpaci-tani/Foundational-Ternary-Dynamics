# FTD-0909 — Production orientation-memory census runner lock v2

**Identifier:** `FTD-0909`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — RUNNER/ADJUDICATOR LOCKED; PRE-RUN]`  
**Parent protocol:** FTD-0908, SHA-256
`53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993`

## 1. Purpose

FTD-0908 was locked before its runner existed. This companion lock freezes
the implemented read-only runner, its compiled local executable, and a
separate Python adjudicator before the first 32-arm campaign execution. It
changes no arm, threshold, production source, observable, control, outcome,
or promotion ceiling in FTD-0908.

FTD-0908 remains the governing physics protocol. FTD-0909 only closes the
instrument-identity interval between protocol lock and data generation.

## 2. Frozen instrument chain

| Artifact | SHA-256 |
|---|---|
| `engine/tests/campaign_production_orientation_memory_census.cpp` | `4FBA0AF9F02440CCA7B166BFFD1A5C2875B18D86B4E402E004F23C4412CB9F34` |
| `engine/CMakeLists.txt` | `51EFD78AEEEBBCCBD4CCC58FB96969C0826BCFF8BFAEB12A4BC79DDF5B05E841` |
| `engine/build/Release/campaign_production_orientation_memory_census.exe` | `83EE291952AFED3A70921A8DC1C6ABEF56275485B714961E7FB6BDDCBC644DD8` |
| `scripts/proofs/proof_production_neutral_dipole_phase_wedge_formation_census_result.py` | `26FD25DA518F1FA000C3DCBC459CEAEC54871950267DAAD61CEB89946F0F2A6A` |
| `scripts/proofs/proof_production_orientation_memory_census_runner_lock_preflight.py` | `D2B9A4A580F65310DB64C67E6BCD129EE67BDF1E796ADCCA192B81E1D60993CD` |

The executable lock identifies the exact local binary of record for the
first run; it is not a claim of cross-toolchain reproducibility. The C++
source and production-source locks are the portable scientific identity.

The parent protocol's seven production/reference source hashes remain
binding without change. The runner embeds the parent protocol hash and the
frozen matrix. The CMake registration names only this runner as CTest
`production_orientation_memory_census`.

## 3. Compile-only qualification

Before this lock, `engine/build_native.bat` completed successfully under the
project-pinned MSVC 14.44.35207 toolchain. The compile linked the frozen
executable but did not execute CTest or the campaign. The result directory
`engine/results/ftd_0908/` remained absent.

The runner-lock preflight passed `28/28` before any campaign data existed. It
checks the parent protocol, all seven production/reference locks, runner,
adjudicator, CMake registration, exact executable, matrix constants,
production-ID/Moore support, state/RNG nonmutation audits, reconstruction
fields, and promotion firewalls.

## 4. Independent adjudication contract

The Python adjudicator is frozen before data. It does not call the engine and
cannot modify the corpus. From the stored CSV and JSON files it must:

1. verify the parent protocol, runner, and seven production/reference hashes;
2. require exactly 32 arms and 96 ticks per arm;
3. reconstruct the polar axis from the stored minimum-image separation;
4. reconstruct `q_plus`, `q_minus`, `p_plus`, `p_minus`, `ell`, and `chi`
   from the stored endpoint fields;
5. independently check Moore support, nonnegative IDs, uniqueness per tick,
   symmetric Gram loss, signed-cubic covariance, inversion invariance, and
   canonical time reversal;
6. reconstruct every per-tick pair aggregate and the deterministic
   rotated-negative null;
7. reconstruct every production-ID sign-stable run and all arm metrics; and
8. apply the frozen FTD-0908 invalid/A/B/C decision tree independently of the
   runner's reported verdict.

The swept-area control is tied to the frozen FTD-0907 analyzer source and is
also reported per observation by the runner. It is not inferred from missing
corpus fields.

## 5. Execution rule

Immediately before data generation, both the FTD-0908 source/protocol
preflight and the FTD-0909 runner-lock preflight must pass. The executable is
then invoked exactly once through focused CTest. The independent adjudicator
runs only after the executable closes all three corpus files.

Any mismatch between runner and adjudicator makes the run invalid. No source,
parser, threshold, tolerance, arm, family, or outcome rule may be repaired
after seeing data without preserving the invalid corpus and locking a new
versioned repair.

## 6. Epistemic ceiling

This lock creates no physical evidence and promotes no claim. Even a valid
FTD-0908 Outcome A remains only
`[MEASURED — PRODUCTION PERSISTENT ORIENTATION-MEMORY CANDIDATES]`.
Formation of the imposed central law, perturbation recovery, maintenance and
erasure work, rectifier coupling, G* cadence, Bell/Born content, and
operational hiding remain untested.

```text
PARENT_PROTOCOL_CHANGED=FALSE
PRODUCTION_SOURCES_CHANGED=FALSE
RUNNER_COMPILED=TRUE
CAMPAIGN_EXECUTED=FALSE
RESULT_CORPUS_EXISTS=FALSE
RUNNER_AND_ADJUDICATOR_FROZEN=TRUE
RUNNER_LOCK_PREFLIGHT=28/28
PRODUCTION_TICK_MODIFIED=FALSE
GSTAR_READ=FALSE
CONTEXT_OUTCOME_BORN_READ=FALSE
CENTRAL_MEMORY_LAW_TESTED=FALSE
MAINTENANCE_ERASURE_WORK_CLOSED=FALSE
PRODUCTION_FORMATION_VERDICT=NOT_YET_AVAILABLE
STATUS=LOCKED_PRE_RUN
```
