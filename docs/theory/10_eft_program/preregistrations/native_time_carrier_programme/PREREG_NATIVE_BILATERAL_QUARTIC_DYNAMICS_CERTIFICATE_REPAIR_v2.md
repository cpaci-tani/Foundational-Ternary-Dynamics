# FTD-0838 — Native bilateral/quartic dynamics certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 22/22]`  
**Date:** 2026-08-10  
**Scope:** tooling-only repair of the invalid FTD-0837 exact certificate  
**Production impact:** none

## 1. Parent and defect

FTD-0837 was locked under pre-run protocol SHA-256
`7D25CABBC31C941976228E840D30E3550E16E77F6CC2298B4D38538616EB1AF1`
and script SHA-256
`4EE5CA8EE94B9B99D14A267D55A431EDAE76A2CA1143C42837123CA5DBDBD768`.
It returned `20/22`. Only C14 and C18 failed. In both cases SymPy had
constructed algebraically identical expressions in different structural
forms:

```text
g**2 - 2*g + 1        versus (1-g)**2
-4*eta*(eta-1)        versus 4*eta*(1-eta)
```

The exact differences simplify to zero. Under the parent lock no theorem was
booked.

## 2. Frozen repair

The v2 wrapper must:

1. fail unless the full FTD-0837 script matches its frozen SHA-256;
2. replace exactly one occurrence of each structural comparison by comparison
   of the exact simplified difference with zero;
3. fail unless each repair target occurs exactly once;
4. change only the terminal certificate identifier from `FTD-0837` to
   `FTD-0838`; and
5. execute the complete parent certificate, including all nine input-source
   hashes and all 22 gates.

No equation, source input, check, tolerance, status firewall, or outcome rule
may change.

## 3. Locked implementation

```text
scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction_v2.py
```

Script SHA-256:
`F5136926BB0045EC01F5478BAFE5BDCB933CB7299F1DDBFA55B35C80D3FBCF7A`

After this protocol hash is recorded in the preregistration manifest, run
exactly:

```text
python scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction_v2.py
```

## 4. Outcomes

- **Outcome A — repaired exact certificate:** all 22 inherited checks pass.
  Book FTD-0837 Outcome B under FTD-0838: the source-scoped three-part
  obstruction and the minimum conditional radial/bath extension. Keep the
  coarse-graining pair closure and the physical `G*` gearbox selected/open.
- **Outcome B — repair invalid:** any check fails. Book no theorem. Any further
  repair requires a new lock.

## 5. Recorded outcome

The hash-matching v2 wrapper ran once. The parent hash, both repair-target
counts, all nine source hashes, and all 22 inherited checks passed. Terminal
output:

```text
FTD-0838 native bilateral/quartic dynamics repaired certificate: 22/22 PASS
FROZEN_PRODUCTION_CORE_ORIENTED_EXCHANGE_ABSENT
FROZEN_PRODUCTION_CORE_SMOOTH_QUARTIC_RESTORER_ABSENT
FROZEN_PRODUCTION_CORE_NONZERO_STABLE_SHELL_ABSENT
MINIMAL_BILATERAL_RADIAL_BATH_EXTENSION_CONDITIONAL_THEOREM
COARSE_GRAINING_PAIR_CLOSURE_STATUS=SELECTED_AND_OPEN
GSTAR_SUBSTRATE_GEARBOX_STATUS=NOT_DERIVED
```

Outcome A holds. The scope remains exactly the frozen production core and the
registered radial-gain class.
