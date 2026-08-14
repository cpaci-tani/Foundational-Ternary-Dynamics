# PRE-REGISTRATION — Ternary eligibility clutch Hamiltonian repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0867`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED OUTCOME B 40/40]`  
**Parent:** `FTD-0866`, execution-invalid `39/40`

## 1. Frozen parent

The invalid parent remains unchanged:

| Artifact | Frozen pre-run hash |
|---|---|
| FTD-0866 protocol | `8AF35869092B3D133185F8246201CDCC4FC251FA819E5CD8C0E844075402F761` |
| `scripts/proofs/proof_ternary_eligibility_clutch_handshake.py` | `6EA25BC3071C2F23DC3B0FBFF640AC5F835D58467F31B1827B345A03D10B0677` |

The first run passed all seven source hashes and 32 of the 33 subsequent exact
gates. The sole failure was C14.

## 2. Exact diagnosis

The parent intended to insert `epsilon=s^2` into the FTD-0865 Hamiltonian

\[
 H=\omega I+\nu(I_c+I_r)+s^2\chi g(\theta)I_r. \tag{1}
\]

Instead, its SymPy expression encoded

```text
omega*I + nu*action + s**2*chi*gate*relative_action
```

where `action` and `relative_action` are independent. Consequently the exact
derivative lacks `nu`:

\[
 \frac{\partial H_{0866}}{\partial I_r}=s^2\chi g(\theta),
\]

not the registered FTD-0865 derivative

\[
 \frac{\partial H}{\partial I_r}=\nu+s^2\chi g(\theta). \tag{2}
\]

This is an equation-transcription repair, not a structural-equality repair.

## 3. Sole permitted correction

The repair wrapper must:

1. verify the frozen parent script hash;
2. find exactly one occurrence of the encoded Hamiltonian line;
3. replace only `nu * action` by `nu * (action + relative_action)` on that
   line, thereby reading `action` as `I_c` and `relative_action` as `I_r`;
4. execute all inherited 40 checks in memory without modifying the parent;
5. return success only if the inherited result is `40/40`.

No source, clutch map, gate, coupling ratio, event preparation, decoder,
energy ledger, reset order, outcome definition, or scope ceiling may change.

## 4. Locked implementation and outcomes

The unrun wrapper is

```text
scripts/proofs/proof_ternary_eligibility_clutch_handshake_v2.py
```

- **Repaired Outcome B:** the one permitted Hamiltonian-coordinate correction
  is made and the inherited exact certificate passes `40/40`.
- **Outcome C:** parent hash mismatch, replacement-count mismatch, or any
  inherited failure. Book no theorem.

Expected result: repaired Outcome B. This expectation is frozen before the
first execution.

No numerical search, tolerance, fit, Born target, Bell setting, production
mutation, or whole-framework completeness claim is permitted.

## 5. Recorded outcome

The wrapper verified the frozen parent hash, made exactly the one permitted
Hamiltonian-coordinate correction in memory, and the inherited certificate
passed `40/40`. FTD-0866 remains execution-invalid at `39/40`; all positive
claims are booked only from this repaired certificate.
