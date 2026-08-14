# FTD-0888 — Autonomous phase-parity/source-reaction certificate repair v2

**Identifier:** `FTD-0888`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0887`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0887 execution reported `68/72`. All six source hashes,
the protocol hash, and every Hamiltonian, phase-window, positivity,
symplecticity, inverse, energy-split, minimality, and scope gate passed. Three
representation guards failed:

1. C8 searched raw protocol text for a sentence split between `remains` and
   `fixed` by line wrapping;
2. C27 searched raw protocol text for a sentence split between `of` and
   `different-color` by line wrapping; and
3. C65 compared the unsimplified symbolic determinant
   `sin(eta)^2+cos(eta)^2` directly with one even though C40 had already
   simplified and proved that determinant to be one.

C72 then failed only because it correctly depends on C1--C71. These are
verifier representation defects. No source, equation, phase window,
Hamiltonian, matrix, symplectic form, energy identity, split angle, outcome,
marker, or scope ceiling may change.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_SPLITTER_v1.md` | `484EC4ED25C322D93B44F88267259B81AE510AE659AE22C4366A5DE69635146A` |
| `scripts/proofs/proof_autonomous_phase_parity_source_reaction_splitter.py` | `814B0AA2E8A555C9F48D9BCAD27C970B07862D1868888A6E0B8C321FEBA97399` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitutions

The wrapper must find exactly one occurrence of each old anchor and apply only
these substitutions.

### R1 — C8 whitespace normalization

```python
    "The equilibrium charge `s_0` remains fixed" in protocol_text,
```

becomes

```python
    "The equilibrium charge `s_0` remains fixed" in protocol_flat,
```

### R2 — C27 whitespace normalization

```python
check("no cross-color commutation is assumed", "No commutation of different-color generators is assumed" in protocol_text)
```

becomes

```python
check("no cross-color commutation is assumed", "No commutation of different-color generators is assumed" in protocol_flat)
```

### R3 — C65 symbolic normalization

```python
check("complete reaction pair is retained", "complete reaction pair is retained" in protocol_flat and M.det() == 1)
```

becomes

```python
check("complete reaction pair is retained", "complete reaction pair is retained" in protocol_flat and sp.simplify(M.det()) == 1)
```

The wrapper must verify each old anchor occurs exactly once, each replacement
is absent initially and present exactly once afterward, and both parent hashes
match section 2 before executing the repaired source in memory.

## 4. Inherited gates and outcome

All 72 FTD-0887 gates, their order, six source hashes, exact symbolic algebra,
phase windows, common Hamiltonian, reaction splitter, self-dual selection
boundary, energy ledger, terminal markers, and outcome rule are inherited
unchanged. The only expected mechanical effect is that C8, C27, and C65
recognize the already frozen evidence; C72 then passes if C1--C71 pass.

## 5. Scope firewall

```text
REPAIR_SCOPE=C8_C27_C65_REPRESENTATION_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
EQUATIONS_HAMILTONIAN_SYMPLECTIC_FORM_ENERGY_UNCHANGED=TRUE
AUTONOMOUS_PHASE_PARITY_CONTROLLER=REFERENCE_ONLY
SELF_DUAL_HISTORY_REACTION_SPLIT=SELECTED_CHANNEL_SYMMETRY
SPATIAL_TERNARY_SOURCE_RECOIL=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA-256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
