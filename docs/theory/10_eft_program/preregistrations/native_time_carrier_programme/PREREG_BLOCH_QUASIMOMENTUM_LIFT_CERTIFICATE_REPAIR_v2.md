# FTD-0895 — Bloch-quasimomentum lift certificate repair v2

**Identifier:** `FTD-0895`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0894`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0894 execution reported `75/81`. All ten source hashes,
the protocol hash, torus-addition witnesses, compact-subgroup obstruction,
finite-range periodicity, exact Fourier coefficients, wrap/lift/carry
identities, momentum-scale ambiguity, FTD-0893 compatibility, and every scope
firewall passed. Six certificate predicates failed:

1. C16 compared equivalent exponential products structurally after
   `expand_power_exp` rather than simplifying their quotient;
2. C34 received SymPy's correct domain-qualified `Piecewise` form for the
   logarithm series rather than a bare logarithm;
3. C35, C37, and C39 left exact roots of unity in incompatible radical/power
   representations until `expand_complex` was applied; and
4. C73 searched for ASCII `observer-only` while the frozen header contains a
   Unicode nonbreaking hyphen in that phrase.

These are certificate representation defects. No source, equation, theorem
statement, outcome, or scope ceiling may change.

## 2. Frozen parent hashes

| artifact | SHA256 |
|---|---|
| `PREREG_BLOCH_QUASIMOMENTUM_LIFT_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md` | `2EC2030AC29C287093019CA8DCD5542577312B9730EFF5B33C4324956CBDC791` |
| `scripts/proofs/proof_bloch_quasimomentum_lift_local_momentum_map_trilemma.py` | `161E64EDB1782C953243B986DEF00C7BD41EC353E912C6AFF9FD0A1682422A0A` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitutions

The wrapper must find exactly one occurrence of each old anchor and apply only
the following four substitutions.

### R1 — C16 exponential normalization

Replace structural equality of the expanded products by simplification of
their quotient to one. The characters and labels are unchanged.

### R2 — C34 Piecewise branch normalization

Apply `piecewise_fold` and compare the first, domain-qualified branch to
`log(1+z)`. The infinite series, coefficient, and convergence domain are
unchanged.

### R3 — C35/C37/C39 exact-root normalization

Apply `expand_complex` before simplifying the already frozen factor identity.
The three registered exact angles and target identity are unchanged.

### R4 — C73 source-punctuation normalization

Search the exact frozen Unicode phrase `observer‑only research
instrumentation`; retain the independent interaction-scale conjunct.

The wrapper must verify each old anchor occurs exactly once, each replacement
is absent initially and present exactly once afterward, and all three artifact
hashes match before executing the repaired source in memory.

## 4. Inherited gates and outcome

All 81 FTD-0894 checks, their order, ten source hashes, character algebra,
compact-subgroup proof, finite-range obstruction, sawtooth series, exact
wrap/lift/carry witnesses, FTD-0893 mass compatibility, corpus boundaries,
terminal markers, and outcome map are inherited unchanged. The only expected
effect is that C16, C34, C35, C37, C39, and C73 recognize the already frozen
evidence.

## 5. Scope firewall

```text
REPAIR_SCOPE=C16_C34_C35_C37_C39_C73_REPRESENTATION_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
QUASIMOMENTUM_ADDITION=EXACT_MODULO_RECIPROCAL_LATTICE
GLOBAL_CONTINUOUS_HOMOMORPHIC_T3_TO_R3_SECTION=IMPOSSIBLE
FINITE_RANGE_GLOBAL_UNWRAPPED_GENERATOR=IMPOSSIBLE
WINDING_HISTORY_TYPE=OPEN_CANDIDATE_NOT_SELECTED
LOCAL_STRESS_ROUTE=NOT_RULED_OUT
PHYSICAL_MOMENTUM_SCALE=OPEN
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
