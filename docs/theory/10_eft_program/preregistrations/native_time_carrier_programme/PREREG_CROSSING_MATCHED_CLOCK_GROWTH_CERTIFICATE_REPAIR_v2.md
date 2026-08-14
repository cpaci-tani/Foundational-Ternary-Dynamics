# FTD-0996 — Preregistration: crossing-matched clock-growth certificate repair v2

**Identifier:** `FTD-0996`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY REPAIR]`  
**Parent:** `FTD-0995`, first execution `84/88`, Outcome D

## 1. Immutable parent record

- Parent protocol SHA-256:
  `B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F`.
- Parent proof SHA-256:
  `17DE90F5BBEFD1BDEFC22AACB236C024FBE8446BD5DE765AA7F95B79EDD87574`.
- All source hashes, necessity/sufficiency equations, energy identities,
  causal-front tests, quartic inheritance tests, mismatch boundary, and scope
  firewalls passed.
- Four verifier representations failed: one source marker used the plain-text
  spelling `E_join-E_cut` instead of the frozen theorem's LaTeX spelling; two
  energy predicates left `sigma^2=1` unstated to SymPy; and one exact zero
  matrix was compared before elementwise simplification.

The parent protocol and proof remain byte-preserved and execution-invalid.

## 2. Authorized in-memory repairs

The wrapper may apply exactly these four substitutions, in two categories:

**Representational-only (leave the tested expression unchanged):**

1. replace the plain source marker `E_join-E_cut` by the exact frozen marker
   `E_{\\rm join}-E_{\\rm cut}`; and
4. apply `sp.simplify` elementwise to `K * uniform_q` before comparison with
   the zero matrix (an evaluation-order change, not a change to the compared
   quantities).

**Domain-completion (change the tested expression):**

2. in the off-seam energy predicate, substitute `sigma_symbol**2 -> 1` before
   simplification; and
3. in the seam-energy predicate, substitute `sigma_symbol**2 -> 1` before
   simplification.

Items 2 and 3 are NOT cosmetic. The parent predicates leave `sigma_symbol`
an unconstrained nonzero real, so each predicate reduces to a nonzero
residual proportional to `(sigma_symbol**2 - 1)` and is false as coded for
generic `sigma_symbol` (e.g. `sigma_symbol = 2`). The substitution completes
a domain restriction — `sigma_symbol` is the retained orientation sign
`sigma in {-1, +1}` used throughout the parent theorem (Eq. 1) — that the
frozen predicate omitted; applying it turns a false-as-coded check into a
true one on the restricted domain. This is authorized as a domain-completion
consistent with the theorem's own stated hypotheses, not as a formula
change: no source hash, expected value, classifier, physical claim, or
scope statement changes, and no equation, tolerance, or threshold anywhere
in the parent theorem changes. But readers auditing this repair should not
take "predicate-only substitution" to mean "the tested expression is
unchanged" for items 2-3 — only for items 1 and 4. The repaired source
exists only in memory during the wrapper execution.

## 3. Integrity gates

The wrapper must verify:

- both parent hashes before execution;
- this repair protocol exists;
- every old fragment occurs exactly once and every replacement is absent;
- exactly four authorized substitutions occur;
- the repaired inherited certificate exits zero with `88/88` and Outcome B;
- the parent protocol and proof bytes remain unchanged; and
- the wrapper reports its own integrity count and fails closed.

## 4. Classifier

- **Outcome B:** all integrity gates pass and the inherited result remains
  exact compliance-surface growth with autonomous matching open.
- **Outcome D:** any hash, occurrence, inherited check, byte-preservation, or
  scope gate fails.

No engine mutation, numerical search, parameter scan, fit, or formula
substitution is authorized.
