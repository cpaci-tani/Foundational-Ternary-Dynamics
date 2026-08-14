# FTD-0836 — Bilateral self-dual quartic clock certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 17/17]`  
**Date:** 2026-08-10  
**Scope:** exact certificate repair only  
**Production impact:** none

## 1. Disclosed predecessor result

FTD-0835 ran once from its hash lock and returned `16/17`. C1--C16 passed.
C17 computed

```text
linear multiplier = 1 - 2*eta
stability margin  = -4*eta*(eta - 1)
ledger residual   = 0
```

but compared the stability margin structurally with `4*eta*(1-eta)`. SymPy
does not regard those two expression trees as identical even though their
difference simplifies exactly to zero. FTD-0835 remains certificate-invalid
and books no theorem.

## 2. Locked repair

Duplicate the complete FTD-0835 exact certificate into a new script. Change
only:

1. the C17 stability comparison from structural equality to
   `simplify(margin - expected_margin) == 0`; and
2. the terminal verdict so the theorem string prints only if all checks pass.

Every construction, exact expression, check name, input firewall, and status
boundary remains unchanged. No tolerance, numerical value, alternative
factorization, native-substrate claim, or neuroscience claim is introduced.

## 3. Implementation and execution

Implementation:

```text
scripts/proofs/proof_bilateral_self_dual_quartic_clock_v2.py
```

Script SHA-256:
`BE35B4B8125F969ABC9D1615E41E8C8039D25B6E03038DD515EB9419B6354C07`

After locking the script and protocol hashes in the preregistration manifest,
run exactly once:

```text
python scripts/proofs/proof_bilateral_self_dual_quartic_clock_v2.py
```

## 4. Outcomes

- **Outcome A — repaired exact certificate valid:** all 17 unchanged checks
  pass. The conditional coordinate theorem and imposed feedback ledger may be
  booked under FTD-0836.
- **Outcome B — repair invalid:** any check fails or either permitted edit is
  exceeded. No theorem is booked.

Neither outcome derives the quartic Hamiltonian, radial controller, bounded
clock hardware, prime-indexed dynamics, Born weights, or brain dynamics from
the substrate.

## 5. Recorded outcome

The hash-matching repaired script ran once and returned:

```text
FTD-0836 bilateral self-dual quartic clock: 17/17 PASS
BILATERAL_SELF_DUAL_QUARTIC_CLOCK_COORDINATE_THEOREM
QUARTIC_HAMILTONIAN_STATUS=SELECTED_INPUT
RADIAL_STABILIZER_STATUS=IMPOSED_REFERENCE_WITH_EXPLICIT_LEDGER
NATIVE_SUBSTRATE_REALIZATION=OPEN
```

Outcome A applies. The result licenses the exact conditional coordinate
theorem and the algebraic feedback ledger only within their frozen inputs.
