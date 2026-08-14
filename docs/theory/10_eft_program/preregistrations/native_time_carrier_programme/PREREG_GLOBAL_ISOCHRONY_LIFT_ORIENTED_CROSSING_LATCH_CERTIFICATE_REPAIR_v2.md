# FTD-0959 pre-registration — Global isochrony-lift crossing-latch certificate repair v2

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0958 immutable execution `91/93`, Outcome D.

## 1. Failure classification

The FTD-0958 parent certificate halted on exactly two structural-equality
checks:

1. SymPy returned the correct lifted action equation as
   `-K*(2*delta+4*pi*w)/2`, but the Boolean compared it structurally with
   `-K*(delta+2*pi*w)`; and
2. the gate-antiphase switch energy simplified to
   `2*A*chi*(-e_0+e_1)`, while the Boolean expected the algebraically identical
   factor order `2*(e_1-e_0)*chi*A`.

All source, barrier-divergence, global-isochrony no-go, lifted-flow,
symplecticity, branch-transition, orientation, ternary-latch, finite-memory,
controller-commensurability, and scope gates passed. The immutable parent
remains preserved.

## 2. Frozen parent hashes

| artifact | SHA-256 |
|---|---|
| `PREREG_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md` | `927F60B630584EDBFFD40922C25D1E57F97C09B2F175C696C1D2FE29C27782FE` |
| `proof_global_isochrony_lift_oriented_crossing_latch_boundary.py` | `2F8F237E01E2B60AFD7614348537345F470CEEC672020E3E93F3A3B9232898E6` |

## 3. Authorized in-memory repairs

Exactly two Boolean-expression substitutions are authorized.

### R1 — lifted action equality

Replace

```python
-sp.diff(Hlift, delta) == -K*tilde
```

with

```python
sp.simplify(-sp.diff(Hlift, delta)+K*tilde) == 0
```

### R2 — antiphase switch-work equality

Replace

```python
sp.simplify(switch.subs(phi, sp.pi)) == 2*(e1-e0)*chi*Acar
```

with

```python
sp.simplify(switch.subs(phi, sp.pi)-2*(e1-e0)*chi*Acar) == 0
```

Displayed expressions, equations, variables, and expected values remain
unchanged.

## 4. Forbidden changes

No source hash, equation, selected type, winding rule, potential class,
barrier assumption, memory count, controller condition, outcome table,
classifier, epistemic tag, tolerance, or scope statement may change. No
numerical search, fitting, empirical substitution, engine mutation, Born/Bell
read, or `G*` identification is authorized.

## 5. Acceptance

The wrapper must:

1. verify the parent protocol, parent certificate, and this repair protocol;
2. prove both old anchors occur exactly once and both replacements are absent;
3. apply exactly the two in-memory substitutions;
4. execute the inherited certificate to `93/93` and exit zero;
5. prove the parent files remain byte-identical; and
6. run a separate repair-integrity checklist.

If any condition fails, Outcome D remains. If all pass, the frozen FTD-0958
Outcome B classifier is licensed through FTD-0959.
