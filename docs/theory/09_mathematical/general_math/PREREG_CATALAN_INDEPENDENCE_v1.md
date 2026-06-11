# PRE-REGISTRATION -- Algebraic independence of Catalan's constant over $\overline{\mathbb{Q}}(G^*, \pi)$, v1

**Tag:** [PRE-REGISTRATION] -- this document locks the *frontier-documentation discipline* for Conjecture 19.2 of the G\* paper (`docs/papers/PAPER_GSTAR_INTRODUCTION.tex`). It is **not a proof attempt** -- per the multi-session arc plan §R3 + Session D1 scoping, proving the algebraic independence of Catalan's constant requires Baker-type lower bounds or Deligne-period machinery beyond the FTD program's current reach. The PREREG documents (a) the conjecture statement (frozen), (b) the strongest currently-feasible numerical evidence (PSLQ 80 digits over a degree-≤8 basis), (c) the falsification criterion (an integer relation found at any precision), (d) what would strengthen the evidence without closing it, and (e) what would close it positively (a transcendence-theoretic proof, FO-difficulty).
**Date:** 2026-05-23 (Phase D Session D1 of `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
**Hash-lock target tag:** `preregister-catalan-independence-v1`.
**LEDGER row reservation:** FTD-0206.
**Supersedes:** none -- first pre-registration documenting the Catalan algebraic-independence frontier.
**Companion docs:**
- [`docs/papers/PAPER_GSTAR_INTRODUCTION.tex`](../../papers/PAPER_GSTAR_INTRODUCTION.tex) Conjecture 19.2 (the conjecture statement + PSLQ evidence as stated in the paper).
- [`docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md`](REF_GUILLERA_CORPUS_MAP.md) (the surrounding AGM/period framework).
- [`docs/theory/09_mathematical/ROADMAP_IDENTITY_PRIORITIES.md`](ROADMAP_IDENTITY_PRIORITIES.md) (Bundle 1 in the synonymy-graph roadmap -- Catalan  {G_G, π, x_+, x_-} -- FO-blocked, this PREREG is the boundary documentation).
- `scripts/verification/verify_gstar_paper.py` (the verified-identity corpus the conjecture lives at the edge of).

> **Pre-registration discipline.** Sections §§2-6 are committed before any *further* PSLQ campaign runs. After commit: SHA256 -> `REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §§2-6 invalidates v1; a v2 is required before any extended-precision PSLQ campaign or alternative-basis scan is run. **No closure attempt is run as part of this pre-registration.** The closure attempt -- proving the conjecture, or finding the falsifying integer relation -- is downstream work, gated on either (a) new transcendence-theory machinery becoming available to the project, or (b) substantial GPU/CPU time being committed to extended-precision PSLQ.

---

## §1 -- Context: why pre-register a frontier conjecture

Per CLAUDE.md goal-clause 2 ("rigorously establish what we cannot derive"), the project's commitment is to map the boundary of what discreteness fixes in both directions: forward derivations *and* honest closures. Conjecture 19.2 of the G\* paper sits at the frontier of analytic / transcendence theory -- not at the discreteness boundary, but at the **mathematical-foundations boundary** that supports the G\* paper's algebraic-independence claims about quasi-modular value algebras.

The pre-registration discipline that has been load-bearing for the FTD-0186 boundary theorem (v1 → v2 in Session A2) and the FTD-0198 ARC-B1 closure attempt (Sessions C1 + C3 + C4) is here applied to a **frontier-documentation** task: lock the conjecture statement, lock what currently constitutes the strongest feasible evidence, lock the falsification criterion, and document the boundary cleanly so that (a) future agents do not relitigate the conjecture as if it were open-or-closed, (b) any further evidence (extended precision, richer basis) is checked against the locked baseline rather than against ad-hoc post-hoc criteria, and (c) the conjecture's status as a transcendence-theory frontier is honestly preserved.

**The conjecture is genuinely open.** It is not "almost proved" or "essentially known." The Beilinson-Deligne conjectural framework provides structural motivation (non-critical L-values are conjecturally outside the period ring of $\mathbb{Q}(i)$) but no proof. PSLQ to 80 digits is strong numerical evidence but, by the nature of PSLQ, does not rule out integer relations with coefficient magnitude > $10^{12}$ -- and never can. The conjecture's *closure* requires methods (Baker-type linear-forms-in-logs lower bounds; Deligne period machinery; modular-form / Eisenstein-series transcendence theory) that are beyond what FTD as a program is currently positioned to develop. **The PREREG is therefore a boundary-documentation deliverable, not a closure scaffold.**

---

## §2 -- The conjecture (frozen, verbatim from G\* paper §19)

**Conjecture 19.2 (G\* paper, Catalan independence).** The Catalan constant
$$ G_{\mathrm{Catalan}} = L(\chi_{-4}, 2) = \sum_{n \geq 0} \frac{(-1)^n}{(2n+1)^2} \approx 0.9159655941\ldots $$
is **algebraically independent of $\{G_G, \pi\}$** (equivalently, of $\{G^*, \pi\}$) over the algebraic closure $\overline{\mathbb{Q}}$.

Equivalent formulations:
- **(F1) Polynomial-relation formulation.** No polynomial $P \in \mathbb{Z}[X, Y, Z] \setminus \{0\}$ satisfies $P(G_{\mathrm{Catalan}}, G^*, \pi) = 0$.
- **(F2) Field-extension formulation.** $[\overline{\mathbb{Q}}(G_{\mathrm{Catalan}}, G^*, \pi) : \overline{\mathbb{Q}}(G^*, \pi)] = \infty$, i.e. $G_{\mathrm{Catalan}}$ is transcendental over the field $\overline{\mathbb{Q}}(G^*, \pi)$.
- **(F3) Period-theoretic formulation.** $L(\chi_{-4}, 2)$ is not in the period ring of $\mathbb{Q}(i)$ as defined by Kontsevich-Zagier.

The three formulations are equivalent up to standard transcendence-theory identifications. The G\* paper uses (F1) operationally (as the PSLQ test target) and cites (F3) via Beilinson-Deligne for structural motivation.

**What the conjecture does NOT claim:** it does not claim that $G_{\mathrm{Catalan}}$ is irrational (that is open in its own right but weaker), nor that the digits of $G_{\mathrm{Catalan}}$ are normal, nor that any specific functional equation links $G_{\mathrm{Catalan}}$ to the $\zeta$-function. The claim is specifically about algebraic independence from $\{G^*, \pi\}$.

**Why this conjecture matters for the G\* paper.** The G\* paper's compendium of identities (Theorems 7-17 + the closing remarks) repeatedly uses the dichotomy between **G\*-shadow values** (in the value ring $\overline{\mathbb{Q}}(G^*, \pi)$) and **non-G\*-shadow values** (outside it, e.g. Catalan, $W^{(4)}_{\mathrm{BCC}}$). Conjecture 19.2 is the load-bearing instance of "Catalan sits outside" -- if it were proven false (an integer relation found), several identities in §§14-18 would need restatement.

---

## §3 -- Current evidence (frozen baseline)

### §3.1 -- The PSLQ baseline (G\* paper §19)

**Setup (frozen):**
- Precision: 80 decimal digits.
- Basis: $\{1, G_{\mathrm{Catalan}}, G_G^k \pi^\ell, G_{\mathrm{Catalan}} \cdot G_G^k \pi^\ell\}$ for $|k|, |\ell| \leq 8$.
- Basis size: $1 + 1 + 2 \cdot 17 \cdot 17 = 580$ basis elements after de-duplication.
- Algorithm: `mpmath.pslq` with default tolerance (relative to 80-digit precision).
- Coefficient bound: $10^{12}$ (PSLQ's reported "no relation up to" threshold at this precision).
- Result: no integer relation found.

**Interpretation (frozen):**
- The negative result rules out integer relations with coefficient magnitude $\leq 10^{12}$ across the 580-element basis.
- It does **not** rule out relations with larger coefficients (PSLQ's resolution is inherently bounded by precision × condition-number considerations).
- It does **not** rule out relations over a *richer* basis (higher exponent ranges, additional generators like $\Gamma(1/8)$ or $W^{(4)}_{\mathrm{BCC}}$).
- It does **not** prove the conjecture. PSLQ is a *necessary* but not sufficient evidence test for algebraic independence.

**Reproducibility:** the PSLQ scan is reproducible from `scripts/verification/verify_gstar_paper.py` and `scripts/verification/investigate_p2_cubic_agm.py` (both committed at commit `e6a6553`, the G\* paper polish commit). The conjecture's statement and evidence claim live at `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §19 / Conjecture 19.2 (uncommitted at hash-lock time per the user's arXiv upload workflow; the PREREG references the paper at its draft state).

### §3.2 -- The structural motivation (Beilinson-Deligne)

The Beilinson-Deligne conjectural framework gives the structural reason to *expect* the conjecture is true:

- $L(\chi_{-4}, 1) = \pi/4$ is the **critical value** of the $L$-function associated to the Kronecker character $\chi_{-4}$ of $K = \mathbb{Q}(i)$. By the Beilinson conjectures (proven in this rank-1 case), critical L-values are periods of the underlying motive -- and the motive of $\chi_{-4}$ has $\mathbb{Q}(i)$-period ring, which contains $\pi$ (and hence $\pi/4$).
- $L(\chi_{-4}, 2) = G_{\mathrm{Catalan}}$ is the **non-critical** L-value (one step past the critical line). Beilinson's conjectures predict that non-critical L-values are *not* in the standard period ring -- they live in a regulator-shifted extension involving the Deligne cohomology.
- Therefore, conjecturally, $G_{\mathrm{Catalan}} \notin \mathcal{P}_{\mathbb{Q}(i)}$ (the Kontsevich-Zagier period ring of $\mathbb{Q}(i)$). Since $G^*$ and $\pi$ are *in* $\mathcal{P}_{\mathbb{Q}(i)}$ (both are CM-period-related values), $G_{\mathrm{Catalan}}$ is conjecturally outside $\overline{\mathbb{Q}}(G^*, \pi)$.

**The Beilinson-Deligne motivation is itself conjectural.** The full Beilinson conjectures are open except in low rank; the period-ring statement for $L(\chi_{-4}, 2)$ specifically is unresolved. The structural motivation gives **strong heuristic support** to Conjecture 19.2 without proving it.

### §3.3 -- What the evidence is, in plain terms

- **Strong:** the PSLQ negative is robust at 80 digits over a substantial basis; the Beilinson-Deligne framework predicts the conjecture should hold for structural reasons.
- **Not a proof:** PSLQ cannot prove independence (only refute it by finding a relation); the Beilinson-Deligne framework is itself conjectural; no transcendence-theoretic proof exists.
- **Posterior probability (heuristic):** the conjecture is *very likely true* -- both the empirical and structural evidence point the same way -- but the gap between "very likely" and "proven" requires transcendence theory that does not currently exist in published form for this specific case.

---

## §4 -- Falsification criterion (LOCKED)

The conjecture is **falsified** -- moves from [STRONGLY MOTIVATED CONJECTURE] to [CLOSED NEGATIVE] -- if any of the following holds:

- **F-CAT-1.** An integer relation is found over the §3.1 basis at *any* precision (including 80, 100, 200, or 500 digits). A non-zero integer vector $(c_0, c_1, \{c_{k\ell}\}, \{c'_{k\ell}\})$ with $\sum |c_i| \leq B$ (for any finite bound $B$) such that the linear combination evaluates to zero at the working precision.

- **F-CAT-2.** A polynomial identity $P(G_{\mathrm{Catalan}}, G^*, \pi) = 0$ with $P \in \mathbb{Z}[X, Y, Z] \setminus \{0\}$ is derived analytically (e.g. via a quasimodular identity, a regulator computation, or an explicit motive-period evaluation).

- **F-CAT-3.** A proof that $G_{\mathrm{Catalan}} \in \mathcal{P}_{\mathbb{Q}(i)}$ (the Kontsevich-Zagier period ring of $\mathbb{Q}(i)$) is published. By (F3) this would imply the conjecture is false.

Falsification by F-CAT-1 (PSLQ at finite precision) requires reporting the precision, the basis, the algorithm parameters, and the coefficient vector; F-CAT-2 and F-CAT-3 require a referee-grade proof. Falsification by F-CAT-1 is the most likely path *if* the conjecture is false; F-CAT-2 and F-CAT-3 are how it would close *if* the conjecture is true (closure-positive via a separate transcendence-theoretic result that incidentally implies the period-ring claim).

---

## §5 -- Evidence-strengthening (not closure) -- pre-registered criteria

The following actions would **strengthen the evidence** for Conjecture 19.2 without closing it. They are pre-registered here so that future agents do not over-claim "the conjecture is essentially proven" if these strengthenings are achieved.

| Strengthening | What it adds | Difficulty | Comments |
|---|---|---|---|
| **S-CAT-1.** Extended-precision PSLQ at 200 digits over the §3.1 basis. | Rules out integer relations with coefficient magnitude $\leq 10^{50}$ (approximate; actual bound depends on basis condition number at 200 digits). | D (≈3-5 days CPU + analysis). | Standard mpmath / pari-gp infrastructure suffices. Does not close the conjecture. |
| **S-CAT-2.** Extended-precision PSLQ at 500 digits over the §3.1 basis. | Rules out integer relations with coefficient magnitude $\leq 10^{200}$ (approximate). | D-W (≈1-2 weeks CPU + analysis, basis condition number becomes a serious concern). | The marginal evidence gain beyond S-CAT-1 is small (PSLQ saturates against the same conjecture). Does not close. |
| **S-CAT-3.** PSLQ at 80 digits over an extended basis adding $\{\Gamma(1/3), \Gamma(1/5), W^{(4)}_{\mathrm{BCC}}\}$ as additional generators. | Tests whether $G_{\mathrm{Catalan}}$ might have a relation with the broader $\Gamma$-product / higher-Watson family that the original basis missed. | W (≈1-2 weeks; basis size grows from 580 to ≈3000). | Negative result narrows the conjecture's domain. Positive result (finding a relation) would be a partial falsification -- the relation might not be with $\{G^*, \pi\}$ alone, but with the extended set. |
| **S-CAT-4.** Direct numerical evaluation of the Deligne regulator for $L(\chi_{-4}, 2)$ to verify it is non-trivial. | Direct test of the Beilinson-Deligne motivation. | M (≈1 month; requires implementing Deligne cohomology computations). | Would either confirm the structural motivation numerically or expose a gap. |

**None of S-CAT-1 through S-CAT-4 closes the conjecture.** They each move the negative-result coefficient bound or extend the basis or test the structural framework -- they do not constitute proofs.

---

## §6 -- Closure criteria (LOCKED)

The conjecture is **closed** -- moves from [STRONGLY MOTIVATED CONJECTURE] to [THEOREM] -- only if **one** of the following is published in referee-grade form:

- **CLOSE-CAT-1 (positive closure).** A transcendence-theoretic proof of Conjecture 19.2 via Baker-type linear-forms-in-logarithms bounds applied to $G_{\mathrm{Catalan}}$. Baker's theorem (rational linear-independence of $\log p_1, \log p_2, \ldots$ for distinct primes) has analogues for L-values that are sufficient *in principle* for cases like this; making the analogue effective for $L(\chi_{-4}, 2)$ specifically would be a significant transcendence-theory contribution. **FO-difficulty** -- requires extending Baker's framework, well beyond FTD's program scope.

- **CLOSE-CAT-2 (positive closure via Deligne).** A proof via Deligne's period machinery that $L(\chi_{-4}, 2)$ is the regulator of a non-trivial Deligne 1-extension, hence outside the period ring of $\mathbb{Q}(i)$. Requires effective implementation of Beilinson-Deligne for this specific L-value. **FO-difficulty.**

- **CLOSE-CAT-3 (positive closure via Eisenstein-series transcendence).** A proof exploiting transcendence properties of Eisenstein series at $\tau = i$ (where $G_{\mathrm{Catalan}}$ appears as a specific L-value evaluation). Connected to the Nesterenko / Chudnovsky transcendence framework. **FO-difficulty.**

- **CLOSE-CAT-4 (negative closure).** Per §4, a counter-example to the conjecture (integer relation found via PSLQ, polynomial identity derived analytically, or proof that the period-ring statement is false). Would close Conjecture 19.2 as `[CLOSED NEGATIVE]`.

**The pre-registration's default expectation is that CLOSE-CAT-1 through CLOSE-CAT-3 will NOT be achieved within the FTD program's reach.** The PREREG documents the boundary so that any future work does not inadvertently mis-cite "the Catalan conjecture is proven in FTD" -- it is not, and the PREREG locks the standard for what would constitute closure.

---

## §7 -- Pre-registered consequences

- **No falsification (default expectation):** the conjecture stays at `[STRONGLY MOTIVATED CONJECTURE]`. Evidence-strengthening per §5 may be carried out without further pre-reg revision; results are recorded in the G\* paper §19 evidence subsection or its successor.
- **Falsification (F-CAT-1 / 2 / 3):** the conjecture moves to `[CLOSED NEGATIVE]`. The relation / proof is documented in a successor document (`AUDIT_CATALAN_INDEPENDENCE_CLOSED_NEGATIVE.md` or similar). The G\* paper §19 and any downstream sections that depend on Conjecture 19.2 (notably §§14-18 identities that use the G\*-shadow / non-G\*-shadow dichotomy) require restatement. **This would be a substantial result and would advance the project's understanding of the G\*-arithmetic.**
- **Closure-positive (CLOSE-CAT-1 / 2 / 3):** the conjecture moves to `[THEOREM]`. The proof is documented in a successor document (`FOUND_CATALAN_INDEPENDENCE.md` or external publication). The G\* paper §19 becomes a theorem statement. **This would be a major transcendence-theory contribution beyond the FTD program's current scope.**

In every case: the spine (FTD-0001 / 0002 / 0006 / 0007 / 0013) is untouched. The conjecture is mathematical-foundations content, not physics content.

---

## §8 -- Hash-lock protocol

To lock this pre-registration before any extended-precision PSLQ campaign or alternative-basis scan runs:

1. Finalise this document. Compute `sha256sum docs/theory/09_mathematical/PREREG_CATALAN_INDEPENDENCE_v1.md`.
2. Record the SHA256 in `docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md` (new section).
3. `git commit` the pre-registration. Create a lightweight tag:
   ```
   git tag preregister-catalan-independence-v1 \
       -m "Pre-reg for Conjecture 19.2 (Catalan algebraic independence over Q(G*, pi))"
   ```
4. The strengthening / closure attempts (§§5-6) run against the tagged commit. Results land in successor documents (`FOUND_*`, `AUDIT_*`, `AUDIT_*_CLOSED_NEGATIVE.md`) per the §§4-6 verdict -- never by editing this file.
5. To verify the tag's commit has not drifted: `git rev-list -n1 preregister-catalan-independence-v1`.

---

## §9 -- What this pre-registration does NOT do

- **Does not prove or disprove Conjecture 19.2.** This is frontier documentation, not a closure attempt.
- **Does not commit the FTD program to pursuing the closure.** Per §6 the closure is FO-difficulty and beyond current scope. The PREREG locks the boundary so that future work does not over-claim.
- **Does not affect any spine claim.** FTD-0001 / 0002 / 0006 / 0007 / 0013 unchanged. Conjecture 19.2 is supporting infrastructure for the G\* paper's value-algebra dichotomy, not a load-bearing claim for the physics spine.
- **Does not run any extended-precision PSLQ.** The 80-digit PSLQ baseline (§3.1) is the current evidence; extending to 200 or 500 digits or to richer baseis is §5 evidence-strengthening work, separately pre-registered if pursued.
- **Does not promote FTD-0013 or any other LEDGER claim.** No tag movement occurs.

---

## §10 -- Status

**DRAFT v1 -- authored 2026-05-23 as Phase D Session D1 of the multi-session coordinated arc `.claude/plans/let-s-proceed-on-the-eager-rocket.md`.** Pending owner review of §2 (the conjecture statement), §3 (the current-evidence baseline), §4 (the falsification criterion), and §6 (the closure criteria), then: commit → `git tag preregister-catalan-independence-v1` → SHA256 to `REF_PREREGISTER_MANIFEST.md` → no further work runs until either (a) extended-precision PSLQ is committed (§5 strengthening), (b) a transcendence-theory proof is published (§6 closure), or (c) an integer relation is found (§4 falsification). **The default expectation is that none of (a)-(c) occurs within the FTD program's reach**; the PREREG is the boundary documentation deliverable for this conjecture.

---

*Pre-registration authored 2026-05-23. **No result.** The conjecture's status is exactly as the G\* paper §19 reports it: [STRONGLY MOTIVATED CONJECTURE] supported by 80-digit PSLQ over a degree-≤8 basis with no integer relation at coefficient bound $10^{12}$, structurally motivated by the Beilinson-Deligne framework, awaiting either a transcendence-theoretic proof (FO-difficulty) or a falsifying integer relation (which the current evidence makes unlikely). The PREREG locks the boundary; closure -- in either direction -- is downstream work.*
