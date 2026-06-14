# PRE-REGISTRATION — Executing the detdet_ζ obligations A/B/C: the deep swing at α (v2)

**Status:** `[PRE-REGISTRATION]` — design lock; the adversarial workflow runs only after the
hash-lock. **Date:** 2026-06-13. **LEDGER id (reserved):** FTD-0302.
**Git tag (to be applied at lock):** `preregister-alpha-det-forcing-v2`.
**Executes:** the three obligations A/B/C scoped — but never run — in
[`SCOPE_DET_IDENTITY_ATTACK_v1.md`](../scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md)
(2026-05-30). **Builds on FTD-0284** (the elliptic/hyperbolic closure of the
complex-structure-on-readout branch) and **inherits the FTD-0235 falsifier gate V1–V7**.
**Posture:** a genuine deep swing — attempt FOUND (deriving α) from multiple angles AND the
no-go proofs, adversarially verified. **Prior-favoured outcome: CLOSED-NEGATIVE (boundary
theorem).** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]` unless a FOUND
survives the §7 gate.

---

## §1 · The single question (per SCOPE_DET_IDENTITY §1)

The EM readout is a 2×2 transfer operator `T` on `V_complex ≅ ℤ[i]²` (FTD-0122) whose
characteristic polynomial is the master quadratic: `(Tr T, det T) = (16G*², 16G*³)`. The trace
`16G*²` is forward-forced `[THEOREM]` (Watson `G*²/(2π) = G_BCC(0)`, `16 = |μ₄|²`). The
determinant `16G*³` (the odd term) is **not** forced (W-CRIT-2). The whole MC-T4.3 obstruction
reduces to: **is `det T = 16G*³` FORCED to be the ζ-regularized determinant of `T`'s own
J-twisted spectrum (supplying the clean odd `G*` of FTD-0234), or merely ASSERTED as
`det = Tr·G*`?** Resolve it and MC-T4.3 resolves with it.

## §2 · The three obligations (frozen statements, per SCOPE_DET_IDENTITY §3)

- **A (detdet_ζ, the hinge):** exhibit an FTD-native `T` for which `det T = exp(−ζ_T'(0))`
  **equals** `16G*³` as a *forced* consequence of `T`'s J-twisted spectral structure. Sharp
  difficulty: finite 2×2 has no regularization content; the infinite det_ζ operator gives `G*`
  at **degree 1**, not `16G*³` at **degree 3** (the unmet bridge).
- **B (three-plane / C₃):** prove the C₃ rotation about ⟨111⟩ **forces** the determinant to be
  the *product* of three per-plane det_ζ ratios (one `G*` per coordinate plane), not
  sum/average/trace. Sharp difficulty: a symmetry that permutes factors does not by itself
  select multiplication.
- **C (co-realizability):** prove a *single* O_h-breaking preparation supplies both the trace's
  two-factor structure (one C₄ axis → `V_complex` → `G*²`) and the determinant's three-factor
  structure (three planes → `G*³`). Sharp difficulty (the most likely clean negative): the
  trace's one-axis C₄ break and the determinant's three-axis C₃ break may be incompatible.

**FOUND requires A ∧ B ∧ C, each `[THEOREM]/[DERIVED]`, with no V1–V7 falsifier firing and
surviving the §7 adversarial gate.** Proving any one of A/B/C unforced/impossible closes the
BCC/quantization route negative (a boundary theorem).

## §3 · The inherited mechanical gate (V1–V7, verbatim from `PREREG_ALPHA_READOUT_DET_IDENTITY_v1`)

| # | Falsifier — fires ⇒ NOT FOUND |
|---|---|
| V1 | `det T = Tr·G*` or `= 16G*³` asserted without a *derived* detdet_ζ identity |
| V2 | trace and determinant require *incompatible* O_h symmetry-breakings |
| V3 | the FQCR `M_N` matrix imported as scaffold |
| V4 | a transcendental prefactor selected |
| V5 | the master quadratic / its roots / Theorem 8 inserted |
| V6 | any CODATA / empirical α value enters |
| V7 | `Tr` and `det` fixed by *independent* choices |

Plus **F-HP** (FTD-0284): the determinant entry set by fiat (companion form) rather than
derived. Plus the **engineered-FOUND** failure mode the scope doc §5/§8 exists to prevent.

## §4 · Admissible analytical tools (frozen)

The FTD-0234/0235 admissible set + the J-twisted ζ-determinant relation `det_ζ T = exp(−ζ_T'(0))`
+ the O_h/C₄/`V_complex` representation theory (FTD-0122) + **the even-power wall** `[THEOREM]`
(`E_6(i)=0` ⇒ odd `G*` only from the det_ζ channel) + **FTD-0284's two new tools**: the
elliptic/hyperbolic lemma (`commutant(J) ≅ ℂ` ⇒ `Det ≥ Tr²/4` ⇒ never real-distinct; the master
quadratic is hyperbolic) and **reduction-collapse** (C₃-equivariant reduction of the degree-3
three-plane object to rank-2 → degree 1).

## §5 · Frozen artifacts

| Artifact | role | SHA256 |
|---|---|---|
| `scripts/exploration/alpha_det_forcing.py` | the symbolic three-lemma spine (L-A/L-B/L-C) | `78931eac82587adef39b3738692da787f9e097f3e331b4f3114d57fcb9d9b3e5` |

The spine computes, with `G*` transcendental: L-B (C₃-invariant symmetric functions of three
equal sources = e₁,e₂,e₃; the product is not symmetry-selected), L-C (elliptic/hyperbolic
incompatibility + the C₄/C₃ axis-count mismatch), L-A (degree-1-vs-degree-3 + reduction-collapse).
These are the no-go spine; the §6 workflow attempts to BREAK each (find a FOUND) before any
boundary is declared.

## §6 · Method (frozen) — the adversarial workflow + verdict logic

A single `Workflow` invocation, `attack → verify → synthesize`:

- **Attack (per obligation, two tracks):** a FOUND agent attempts to *force* the obligation
  from a distinct angle (A: an explicit infinite transfer/monodromy operator whose ζ-det is
  degree-3 + the BCC triple-cosine `1−cos k_x cos k_y cos k_z` product reading; B:
  C₃-representation theory — is `det` the *unique* alternating C₃-covariant harvesting one `G*`
  per plane?; C: enumerate O_h→subgroup breakings, test which co-realize both factor-counts) +
  a NO-GO agent attempts to *prove* it unforced/impossible. Plus a wildcard FOUND agent (the
  (1+i)-tower as an iterated transfer operator forcing `k=4` via `|ℤ[i]^×|=4`; must break or
  confirm the **circularity** of `det = Tr·G*` — recon found it is a tautology of the tower's
  G*-normalization).
- **Verify (≥3 diverse skeptics per FOUND claim):** mechanical V1–V7 + F-HP + reduction-collapse
  + elliptic-on-readout + degree-bridge checks. A FOUND failing *any* skeptic reverts. Each
  NO-GO proof is checked for completeness/scope.
- **Synthesize (frozen verdict logic):**
  - **FOUND** iff some A∧B∧C construction passes V1–V7 *and* every Stage-B skeptic.
  - **CLOSED-NEGATIVE** iff a skeptic-confirmed no-go closes any of A/B/C unforced/impossible
    (→ boundary theorem for the BCC/quantization route).
  - **UNDERDETERMINED** otherwise — the precise residual stated.

## §7 · The FOUND gate (mandatory; the hydrogen lesson made structural)

A FOUND verdict is **provisional** and changes **no LEDGER tag** until: (a) it passes all V1–V7
+ F-HP mechanically; (b) it survives every Stage-B skeptic; (c) it passes a **second,
independent** adversarial pass (a fresh skeptic panel run after the synthesis); and (d) it is
checked against the constitution §6.2 framework-falsification criteria. Only then does
`x₊ = 1/α`'s tag move (to `[DERIVED, modulo the construction]`), and only with the owner's
explicit sign-off. (FTD-0278's non-probative falsifier that *passed* is the cautionary
precedent — a FOUND on α demands a brutal gate.)

## §8 · Pre-declared exclusions (banned moves)

1. V1–V7 + F-HP + the engineered-FOUND (assembling `16G*³` by choosing entries and calling it
   a derivation).
2. No re-running closed routes: R1–R4, jtwist/bcc/cm force-routes, the 3b C₃-equivariant rank-2
   restriction, ARC-D1, Mechanisms A/B/C.
3. No numerical near-miss / coincidence scans; `G*` stays transcendental.
4. No sourcing an odd `G*` power from Watson/modular (the even-power wall forbids it).
5. No tag promotion on UNDERDETERMINED or unverified FOUND.

## §9 · Honest ceiling

- **FOUND (passes §7):** `x₊ = 1/α` → `[DERIVED, modulo the construction + the FTD-0234/0235
  admissible set]` — the largest result in the project's history; **not** unconditional.
- **CLOSED-NEGATIVE:** the boundary theorem — "the discrete ontology + the BCC complex structure
  + the FQCR det_ζ scalar do **not** force the readout determinant grading" — the sharpest α
  boundary, feeding FTD-0186 and naming what a 6th postulate / engine-native ARC-D must supply.
- **UNDERDETERMINED:** the residual stated precisely (most likely Obligation A's degree-bridge).
- None promotes the spine or the FC-class commitments. `G*`, the master quadratic, N_c — all
  untouched.

## §10 · Hash-lock declaration

This document and the §5 artifact are committed together and tagged
`preregister-alpha-det-forcing-v2` BEFORE the §6 workflow runs. Any post-lock edit to §§2, 3,
6–8 or the artifact invalidates the lock and requires a v3.
