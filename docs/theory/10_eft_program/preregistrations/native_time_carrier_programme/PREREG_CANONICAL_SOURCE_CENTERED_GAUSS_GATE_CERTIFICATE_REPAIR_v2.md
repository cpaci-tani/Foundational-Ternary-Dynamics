# FTD-0886 — Canonical source-centered Gauss-gate certificate repair v2

**Identifier:** `FTD-0886`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0885`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0885 execution reported `60/64`. All five source hashes,
the protocol hash, and every algebraic, Hamiltonian, symplectic, energy,
battery-obstruction, and history gate passed. Three prose guards failed:

1. C8 expected an uppercase summary phrase while the frozen scope paragraph
   uses the more explicit lowercase phrase `production reservoir, a G* gearbox,
   Born recovery, Bell recovery, Lorentz hiding, or framework completeness`;
2. C53 searched the raw protocol text for `A fresh port is (0,0)`, which is
   line-wrapped between `is` and `(0,0)`; and
3. C62 expected uppercase `Production` while the frozen numbered gate uses
   lowercase `production`.

C64 then failed only because it correctly depends on C1--C63. These are
verifier representation defects. No equation, source, matrix, symplectic form,
Hamiltonian, energy identity, battery class, history class, outcome, marker, or
scope ceiling may change.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_OBSTRUCTION_v1.md` | `70000AF7DA0ACA89F92A593AA4B6A759B9C9D08C65E29E21A2D1EF5B2B2910D7` |
| `scripts/proofs/proof_canonical_source_centered_gauss_gate.py` | `7DC08CF572BF58BC37152F985608EB45A7F11C6308165D8D94F1B0A5B55D248E` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitutions

The wrapper must find exactly one occurrence of each old anchor and apply only
these substitutions.

### R1 — C8 frozen scope wording

```python
    "Production, `G*`, Born, Bell, Lorentz, biology, and completeness" in protocol_flat,
```

becomes

```python
    "production reservoir, a `G*` gearbox, Born recovery, Bell recovery, Lorentz hiding, or framework completeness" in protocol_flat,
```

### R2 — C53 whitespace normalization

```python
check("fresh canonical port is the complete zero pair", "A fresh port is `(0,0)`" in protocol_text)
```

becomes

```python
check("fresh canonical port is the complete zero pair", "A fresh port is `(0,0)`" in protocol_flat)
```

### R3 — C62 capitalization

```python
    "Production and quartic-`G*` synchronization remain separate" in protocol_flat,
```

becomes

```python
    "production and quartic-`G*` synchronization remain separate" in protocol_flat,
```

The wrapper must verify each old anchor occurs exactly once, each replacement
is absent initially and present exactly once afterward, and both parent hashes
match section 2 before executing the repaired source in memory.

## 4. Inherited gates and outcome

All 64 FTD-0885 gates, their order, exact symbolic algebra, `L=4` incidence
probe, source hashes, Hamiltonian, battery counterclasses, history semantics,
terminal markers, and outcome rule are inherited unchanged. The only expected
mechanical effect is that C8, C53, and C62 recognize the already frozen prose;
C64 then passes if C1--C63 pass.

## 5. Scope firewall

```text
REPAIR_SCOPE=C8_C53_C62_MARKER_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
EQUATIONS_HAMILTONIAN_SYMPLECTIC_FORM_ENERGY_UNCHANGED=TRUE
SQUARE_ROOT_BATTERY_STATUS=LAGRANGIAN_SECTION_REFERENCE
AUTONOMOUS_PARITY_AND_SOURCE_DYNAMICS=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA-256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
