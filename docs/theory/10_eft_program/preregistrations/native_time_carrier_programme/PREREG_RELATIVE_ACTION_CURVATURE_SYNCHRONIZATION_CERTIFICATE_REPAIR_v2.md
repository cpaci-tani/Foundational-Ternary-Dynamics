# FTD-0957 pre-registration — Relative-action-curvature synchronization certificate repair v2

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0956 immutable execution `108/111`, Outcome D.

## 1. Failure classification

The FTD-0956 parent certificate halted on exactly three checks:

1. the `sigma=-1` theta equation remained as a rational expression exactly
   equal to the registered expected expression but was compared after
   `trigsimp` rather than full algebraic `simplify`;
2. the same verifier defect occurred for `sigma=+1`; and
3. SymPy returned undirected complex infinity (`zoo`) for the real one-sided
   limit of `EllipticK(m)` as `m -> 1-`, although the registered real integral
   diverges positively.

All Hamilton equations, reduced equations, charge and energy ledgers,
positivity/stability gates, Floquet gates, crossing-section gates, and scope
gates passed. The immutable parent remains preserved.

## 2. Frozen parent hashes

| artifact | SHA-256 |
|---|---|
| `PREREG_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_AND_CROSSING_SECTION_ENERGY_v1.md` | `EB22D8BC597A22E676D9B38BD38C9E1DB8B9C9D703D68A856A9B3525CE2D4D28` |
| `proof_relative_action_curvature_synchronization_crossing_section_energy.py` | `04BAE420DFC7C49CA5A5DCAA4D6E2F547DF4F1EF91C7A8ADE2EC4D79F8613FE3` |

## 3. Authorized in-memory repairs

Exactly two source substitutions are authorized in a wrapper; neither parent
file may change.

### R1 — algebraic equality normalization

Replace the unique comparison

```python
sp.trigsimp(flow[index]-expected[index]) == 0
```

with

```python
sp.simplify(flow[index]-expected[index]) == 0
```

The displayed note may remain `trigsimp`; only the Boolean equality
normalization changes.

### R2 — real divergent lower-bound verifier

Replace the direct CAS limit

```python
separatrix = sp.limit(sp.elliptic_k(m), m, 1, dir="-")
```

with the exact real lower-bound limit

```python
eps = sp.symbols("epsilon", positive=True, real=True)
separatrix = sp.limit(sp.asinh(1/sp.sqrt(eps)), eps, 0, dir="+")
```

For `m=1-eps` and `0<=y<=1`,

\[
 1-m\cos^2y=\epsilon+(1-\epsilon)\sin^2y
 \le \epsilon+y^2.
\]

Therefore the terminal part of the elliptic integral is bounded below by

\[
 \int_0^1{dy\over\sqrt{\epsilon+y^2}}
 =\operatorname{asinh}(1/\sqrt\epsilon)\longrightarrow+\infty.
\]

This verifies the same registered positive divergence without accepting
`zoo` as a physical value.

## 4. Forbidden changes

No equation, variable, source hash, selected scale, tolerance, window,
crossing section, energy/charge ledger, outcome table, classifier, epistemic
tag, or scope statement may change. No numerical search, fitting, empirical
substitution, engine mutation, Born/Bell read, or `G*` identification is
authorized.

## 5. Acceptance

The wrapper must:

1. verify the parent protocol, parent certificate, and this repair protocol;
2. prove both old anchors occur exactly once and both replacements are absent;
3. apply exactly the two in-memory substitutions;
4. execute the inherited certificate to `111/111` and exit zero;
5. prove the parent files remain byte-identical; and
6. run a separate repair-integrity checklist.

If any condition fails, Outcome D remains and no theorem is licensed. If all
conditions pass, the frozen FTD-0956 Outcome B classifier is licensed through
FTD-0957.

