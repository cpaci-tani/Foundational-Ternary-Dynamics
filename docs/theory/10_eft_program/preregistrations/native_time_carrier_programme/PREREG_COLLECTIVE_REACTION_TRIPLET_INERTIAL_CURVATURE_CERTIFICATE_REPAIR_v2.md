# FTD-0892 — collective reaction-triplet/inertial-curvature certificate repair v2

**Identifier:** `FTD-0892`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0891`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0891 execution reported `62/68`. All ten source hashes,
the protocol hash, every Helmert/symplectic/collective-variable gate, every
impulse gate, strict convexity, the common-velocity minimizer, exact composite
dispersion, conditional inertia, binding-offset mismatch, and the substantive
static non-identifiability gates passed. Five certificate predicates failed:

1. C48 compared two algebraically equal rational expressions structurally
   after simplifying only one side;
2. C62 used `does not derive` while the frozen source says `do not derive`;
3. C63, C64, and C66 searched normalized Markdown while retaining backtick
   punctuation around frozen symbols.

C68 then failed only because it depends on C1--C67. These are certificate
representation defects. No source, equation, theorem statement, outcome, or
scope ceiling may change.

## 2. Frozen parent hashes

| artifact | SHA256 |
|---|---|
| `PREREG_COLLECTIVE_REACTION_TRIPLET_AND_INERTIAL_CURVATURE_BOUNDARY_v1.md` | `D273F1A61E1A55B26781116E3B9D3984DAFF843DB04F18E160C706EBEAC6C595` |
| `scripts/proofs/proof_collective_reaction_triplet_inertial_curvature.py` | `ED729418595D0B6B0F69F9381CB5DF007DF764E79CDF9145DF15DA9C4B6104FE` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitutions

The wrapper must find exactly one occurrence of each old anchor and apply only
these substitutions.

### R1 — C48 rational-expression normalization

The comparison is replaced by simplifying the difference between the two
frozen expressions to zero. The Hamiltonians and their curvatures are
unchanged.

### R2 — C62 source-grammar normalization

The marker `does not derive that action, its bond graph` is replaced by the
source's exact phrase `do not derive that action, its bond graph`.

### R3 — C63 Markdown-punctuation normalization

Before the check, define `protocol_plain = protocol_text.replace("`", "")`
and search the same frozen sentence without Markdown backticks.

### R4 — C64 Markdown-punctuation normalization

Search `protocol_plain` for `z^3, not r^3` and
`not make p_matter+p_field`; no physical statement changes.

### R5 — C66 Markdown-punctuation normalization

Search `protocol_plain` for the same Born/Bell, G-star, Lorentz, biology, and
completeness firewall.

The wrapper must verify each old anchor occurs exactly once, each replacement
is absent initially and present exactly once afterward, and both parent hashes
match before executing the repaired source in memory.

## 4. Inherited gates and outcome

All 68 FTD-0891 checks, their order, ten source hashes, collective symplectic
reduction, selected constituent dispersion, conditional composite inertia,
static-data non-identifiability, lattice Noether boundary, terminal markers,
and outcome map are inherited unchanged. The only expected effect is that C48,
C62--C64, and C66 recognize the already frozen evidence; C68 then passes if
C1--C67 pass.

## 5. Scope firewall

```text
REPAIR_SCOPE=C48_C62_C63_C64_C66_REPRESENTATION_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
COLLECTIVE_REACTION_TRIPLET=EXACT_SYMPLECTIC_SECTOR
COMPOSITE_INERTIA=CONDITIONAL_ON_SELECTED_DISPERSION
STATIC_HESSIAN_TO_MASS_SCALE=CLOSED_NEGATIVE
TOTAL_FIELD_MATTER_NOETHER_MOMENTUM=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
CONSTITUENT_FORMATION_STABLE_POLE_PRODUCTION=OPEN
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
