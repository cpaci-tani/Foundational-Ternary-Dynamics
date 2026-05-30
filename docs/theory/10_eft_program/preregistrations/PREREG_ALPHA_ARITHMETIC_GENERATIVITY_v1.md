# PRE-REGISTRATION -- Alpha Arithmetic Generativity Test (Test 4), v1

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-20
**Hash-lock target tag:** `preregister-alpha-arithmetic-generativity-v1`
**LEDGER row reservation:** FTD-0185
**Supersedes:** an earlier, uncommitted draft of this pre-registration. This redone v1 retains that draft's strict rule-set (Sections 2-4), adds the structural grounding that makes the candidate space concrete (Section 2.1), and records the first desk-audit pass (Section 5).
**Companion docs:** `SPEC_FQCR.md` Section 6 (Test 4); `SPEC_ALPHA_READOUT_CONTRACT.md` (ARC / FTD-0152 -- the operational alpha-readout problem MC-T4.3); `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (FTD-0143, Test 2); `SPEC_ALGEBRAIC_SPINE.md` Section 4 (coefficient 16 = |Aut(E)|^2, FTD-0006); LEDGER FTD-0013 (master quadratic single-root readout; FTD-0014 retired per v1.4 §5 — see annotation below), FTD-0157 (equianharmonic dichotomy), FTD-0163 (chi character-unification), FTD-0175/0182 (Sym^2-Sym^3 (2,3)-uniqueness), FTD-0181 (integer-4 unification); `scripts/exploration/gstar_equianharmonic.py`, `scripts/exploration/sym23_uniqueness_proof.py`.

**2026-05-22 v1.4 cleanup-taxonomy annotation (outside frozen scope).** Hash-locked references to FTD-0014 (`x_- ↔ N_c`) and the "Forbidden: `x_- ~ N_c` dual-root match as generativity prize" rule below refer to the framework state at registration time (2026-05-20). On 2026-05-22, **after this pre-registration was locked**, FTD/FQCR Cleanup Taxonomy v1.4 §5 retired the `x_- ↔ N_c` identification (LEDGER FTD-0014 removed in commit `ca7eb61`). Per pre-registration discipline (§0), the frozen §§2-4 content is preserved verbatim. The retirement strengthens the prohibition's intent (the dual-root match is now not merely "already advertised" but **structurally not a live physics identification at all**); the rule's force is unchanged. `N_c = 3` in FTD is independently sourced via `DERIV_NC_FROM_TOPOLOGY.md`.

> **Pre-registration discipline.** The rules in Sections 2-4 are committed before any generativity candidate is numerically evaluated. Section 5 records the first desk-audit pass, reached by structural reasoning only -- no numerical comparison. After commit, this document's SHA256 is recorded in `REF_PREREGISTER_MANIFEST.md` and the git tag is applied. Any post-hoc edit to Sections 2-4 invalidates v1; a v2 must be issued before a new candidate is evaluated.

---

## 1. Thesis: the Balmer-to-Bohr gate

Current canonical alpha posture:

- The master quadratic and the FQCR transfer structure are mathematical spine content ([THEOREM]).
- `x_+` matches physical `1/alpha` at 1.26 ppm -- [STRONGLY MOTIVATED CONJECTURE] (FTD-0013).
- Every dynamical / action-level recovery route has closed negative or structurally decoupled.
- `SPEC_ALPHA_READOUT_CONTRACT.md` defines the operational readout problem (ARC / MC-T4.3), which remains [OPEN].

The reframe this pre-registration tests:

> If the alpha match is real, alpha is not recovered from substrate dynamics. It is an arithmetic / readout invariant of the lemniscatic CM / FQCR structure. The correct falsifier is therefore not "can the engine flow to alpha?" but "does the arithmetic architecture generate a second independent dimensionless observable with no new tuned freedom?"

One precise formula is Balmer: a fit. A second independent observable from the same rigidity is Bohr: a mechanism. Test 4 is that gate.

---

## 2. Admissible mechanism and forbidden operations

### 2.1 The candidate space is structurally constrained

The reframe in Section 1 says alpha is an invariant of the lemniscatic CM structure. The most direct generativity candidate is therefore: apply the identical master-quadratic construction to a *different* CM elliptic curve and ask whether its roots read a different physical observable.

This avenue is finite and tiny. Over the complex numbers, exactly two elliptic curves carry an automorphism group larger than {+1, -1}:

- the **lemniscatic** curve E: y^2 = x^3 - x  (j = 1728, |Aut| = 4, CM by Z[i]) -- already used; root x_+ conjecturally equals 1/alpha;
- the **equianharmonic** curve E_rho: y^2 = x^3 - 1  (j = 0, |Aut| = 6, CM by Z[rho]).

This is classical and is reconfirmed in `gstar_equianharmonic.py` (line 295: "the lemniscatic case (m=4) and equianharmonic case (m=6) are the only non-trivial instances over Q with |Aut| > 2"). The curve-generativity avenue therefore has **exactly one untried instance** -- the equianharmonic curve. There is no third curve and no tunable family. Any admissible candidate beyond the equianharmonic curve must come from a *new theorem in the existing architecture* (allowed, Section 2.3) -- not a new curve, and not a retuned recurrence.

**The coincidence-break.** In the lemniscatic case the relevant integers collapse to a single value: |Aut(E)| = |Z[i]^x| = |disc Q(i)| = (chi_-4 modulus) = 4. FTD-0181 documents this and proves Q(i) is the unique imaginary quadratic field with |units| = |discriminant|. Because the integers coincide, the lemniscatic master quadratic alone cannot identify which integer is the structural source of its coefficient 16. The equianharmonic case breaks the degeneracy: |Aut(E_rho)| = 6, |Z[rho]^x| = 6, |disc Q(rho)| = 3, (chi_-3 modulus) = 3. A candidate built on the equianharmonic curve therefore inherits a small, enumerable set of structurally-arguable forms; Section 5 records which choices are forced and which are not.

### 2.2 What counts as "same mechanism"

An admissible Test-4 candidate may reuse only the following fixed structure:

- `G* = Gamma(1/4)/Gamma(3/4)` and the `chi_-4` / `Q(i)` quarter-conjugacy spine -- or its structural equianharmonic mirror (Section 2.1, Section 5).
- The coefficient `16 = |Aut(E_lemn)|^2` and the canonical Sym^2 / Sym^3 (2,3)-uniqueness chain (FTD-0175/0182).
- FQCR Models I-V as currently specified in `SPEC_FQCR.md`, unless a change is proved independently of any target value.
- The ARC discipline (`SPEC_ALPHA_READOUT_CONTRACT.md`, FTD-0152) when an operational physics readout is claimed.

### 2.3 Allowed and forbidden operations

**Allowed:**

- Prove a new theorem within the existing CM / FQCR architecture.
- State a new readout map *before* any numerical comparison.
- Derive a target-independent symbolic expression and only then compare it to data.

**Forbidden:**

- Numerical search for near-misses, coincidence scans, or target-list matching.
- Adding a free integer, exponent, scale, threshold, or normalization after looking at a target value.
- Substituting FTD numbers into a standard physics formula and calling the result a derivation.
- Retuning FQCR response terms (R, t, exponent quadruples, observer terms) to hit a target.
- Counting the already-advertised `x_- ~ N_c` dual-root match as the generativity prize.
- Using dimensional calibrations (`a_phys`, `K_B`, `ell_P`, measured SI masses) to manufacture a dimensionless-looking target.

---

## 3. Target declaration rule

Before any numerical comparison, a Test-4 candidate must publish a target declaration containing:

1. **Target observable** -- the physical dimensionless observable to be predicted.
2. **Why this observable is in scope** -- why the same CM / FQCR architecture should speak to it, argued target-independently.
3. **Symbolic output** -- the exact expression the architecture produces.
4. **No-freedom audit** -- every integer, exponent, branch choice, normalization, and map used, each marked "already canonical" or "newly introduced".
5. **Comparison protocol** -- the experimental value or dimensionless relation to be used and the tolerance accepted.

The declaration must exist before the numerical residual is evaluated. If the residual is known before the declaration, the attempt is not Test 4; it is exploratory and cannot promote the alpha conjecture.

---

## 4. Locked pass/fail criteria

**Outcome A -- generativity pass.** All hold: the candidate uses only the Section 2 mechanism; the Section 3 declaration is complete before numerical comparison; the output predicts one independent physical dimensionless observable not already encoded by the master-quadratic dual-root story; no new tunable freedom is introduced; the comparison lands inside the pre-declared tolerance; the derivation explains *why* that observable reads the arithmetic quantity. Claim impact: `x_+ <-> 1/alpha` may be considered for upgrade from [STRONGLY MOTIVATED CONJECTURE] to a "generative arithmetic conjecture" status, subject to separate LEDGER review; the new observable receives its own tag by proof quality.

**Outcome B -- arithmetic-only pass.** The architecture produces a new theorem or invariant but no operational physical readout. Claim impact: pure mathematics may strengthen; the alpha physics claim does not promote.

**Outcome C -- near-miss / fit failure.** The output is generated honestly but misses the target, or multiple comparable targets can be chosen post hoc. Claim impact: preserve as exploratory or closed-negative for that target; the alpha claim remains [STRONGLY MOTIVATED CONJECTURE].

**Outcome D -- exclusion failure.** The attempt violates any forbidden operation in Section 2.3. Claim impact: archive as closed-negative provenance if documented; no tag promotion.

---

## 5. Desk audit -- first pass (completed 2026-05-20)

Per Section 2.3 the first executable task is a desk audit, not a scan: inventory the CM / FQCR structures that produce target-independent symbolic outputs, and for each ask whether a physical observable can be attached *before* numbers are looked up. This pass uses structural reasoning only -- no numerical comparison is performed.

### 5.1 Candidate: the equianharmonic master quadratic

Section 2.1 establishes that the curve-generativity avenue has exactly one untried instance. By structural transfer from the lemniscatic `x^2 - 16 G*^2 x + 16 G*^3`, the equianharmonic analog has the form `x^2 - c R^2 x + c R^3`, with:

- **exponents (2, 3): FORCED.** The FTD-0175/0182 Sym^2-Sym^3 uniqueness case analysis is value-agnostic -- verified against the logic of `sym23_uniqueness_proof.py`: admissibility reduces to the exponent inequality 2a > b, and the one value-dependent step (discriminant positivity) is automatically satisfied for any base constant > 1 and prefactor > 4. (2,3) is the unique minimal pair for every form considered here.
- **prefactor c and constant R: principled but NOT forced.** The coincidence-break (Section 2.1) means no theorem fixes them. The principled primary is `c = 36 = |Aut(E_rho)|^2` and `R = R_3 = Gamma(1/3)/Gamma(2/3)` (the chi_-3 Gamma-ratio, FTD-0163); the structurally-arguable alternatives are `c = 9 = |disc Q(rho)|^2` and `R = Gamma(1/6)/Gamma(5/6)`. `SPEC_ALGEBRAIC_SPINE.md` Section 4 establishes 16 = |Aut(E_lemn)|^2 only as an arithmetic identity *for the lemniscatic curve*, with an explicit "what it does NOT claim" clause -- it is not a general prefactor law.

This candidate produces a target-independent symbolic output (the polynomial and its exact roots). It clears the first inventory test.

### 5.2 The candidate fails the Section 3 target-declaration gate

The candidate must declare a physical target observable *before* numerical comparison, with a target-independent reason it is in scope (Section 3, items 1-2). It cannot:

- The lemniscatic readout `x_+ <-> 1/alpha` itself has **no derived mechanism** -- this is exactly MC-T4.3, which is [OPEN]. There is no established "why" for the k = 4 case.
- With no "why" for k = 4, there is no "why" to transfer to the equianharmonic (k = 3) case. No principled argument names a physical observable for the equianharmonic roots in advance.
- The one structural echo available -- the equianharmonic small root playing an `x_- ~ N_c`-type role -- is explicitly excluded as a prize by Section 2.3.

Any physical target for the equianharmonic roots would therefore be visible only *after* numerical evaluation. By Section 3 the candidate is **rejected** for Test-4 purposes. (It remains legitimate exploratory mathematics; nothing here closes the equianharmonic polynomial as a mathematical object.)

### 5.3 First-pass verdict: no admissible candidate -- Test 4 is blocked

The equianharmonic curve is the only untried curve-generativity candidate, and it fails the target-declaration gate. The curve-generativity avenue is therefore **exhausted with no admissible candidate**.

This pass produces a **no-candidate report** -- the outcome Section 5 of the rule-set explicitly anticipates. **Test 4 is currently blocked**, and the blocker is precise: **MC-T4.3**. Without an alpha-injection mechanism there is no "why" for the lemniscatic readout; without a "why" no physical target can be declared for a second curve; and generativity-by-prediction is, by Section 3, exactly target-declaration-by-prediction. Test 4 is gated behind the same missing mechanism that gates the rest of the alpha program.

This is not a failure of FTD or of Test 4. It is the disciplined process reporting, correctly, that the test cannot yet be run -- and locating, precisely, what would unblock it.

### 5.4 What remains open

One admissible avenue remains: a **new theorem within the existing architecture** that yields a target-independent symbolic output *and* carries a pre-declarable physical observable (Section 2.3, "allowed"). No such theorem is currently in hand. The rules in Sections 2-4 stand locked for any future candidate of this kind; producing a complete Section 3 declaration for one is the open Test-4 work.

---

## 6. Relation to Test 2 and ARC

Test 2 (`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`, FTD-0143) asks whether the FQCR Model IV quotient choice is unique -- a robustness test. Test 4 asks whether the architecture is generative. A positive Test 2 without Test 4 leaves alpha at a refined Balmer-like status; a positive Test 4 would be the first Bohr-like step.

ARC (`SPEC_ALPHA_READOUT_CONTRACT.md`, FTD-0152) remains the operational requirement for a *physical* alpha derivation. Test 4 does not replace ARC; it tests whether the arithmetic spine has independent physical reach. A generativity pass without an ARC-grade readout still does not license the phrase "FTD derives alpha".

---

## 7. Hash-lock

After owner review and commit:

```
git tag preregister-alpha-arithmetic-generativity-v1 <commit-sha>
sha256sum docs/theory/10_eft_program/PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md
```

Record the SHA256 in `REF_PREREGISTER_MANIFEST.md` (replacing the superseded draft's entry) and add the LEDGER FTD-0185 row. The git tag is local-only per project policy. Sections 2-4 are the pre-registered rules; the Section 5 first-pass verdict is part of the locked record. Any post-hoc edit to Sections 2-4 requires a v2.

---

## 8. External wording

Use:

> The next alpha test is generativity: can the same lemniscatic CM / FQCR rigidity that produces the alpha candidate produce one further independent dimensionless observable with no new tuned freedom? As of the first desk-audit pass (2026-05-20), Test 4 has no admissible candidate -- the one other extra-automorphism curve cannot declare a target in advance -- so Test 4 is blocked behind the open alpha-readout mechanism (MC-T4.3).

Do not use:

> FTD has derived alpha.
> Test 4 confirms generativity.

---

## 9. Status

**v1 -- redone 2026-05-20**, replacing an uncommitted earlier draft. The strict rule-set (Sections 2-4) is kept; the structural grounding (Section 2.1) and the executed first desk-audit pass (Section 5) are new.

- Rules, Sections 2-4: **pre-registered.**
- Section 5, first desk-audit pass: **complete -- no admissible candidate; Test 4 blocked behind MC-T4.3.**

**Not yet hash-locked.** Pending owner review, then: commit this file -> `git tag` -> SHA256 to `REF_PREREGISTER_MANIFEST.md` (refresh the FTD-0185 row; the prior SHA `b222c2a0...` was the superseded draft) -> add the LEDGER FTD-0185 row. The superseded equianharmonic draft (`PREREG_FQCR_EQUIANHARMONIC_GENERATIVITY_v1.md`) is withdrawn; its structural analysis is folded into Sections 2.1 and 5.
