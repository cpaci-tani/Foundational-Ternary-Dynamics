# MATH — Reflection flow parity: the product and ratio branches as first-order flows, split by differential algebraicity

**Tag:** [THEOREM — classical identities assembled (differentiated Euler reflection; Gauss digamma; Hölder 1887)] for §1–§2; [coherent-interpretation] for every reading in §3. Introduces no new theorem of FTD's — the assembly and the frontier reading are the contribution.
**LEDGER id:** FTD-0367.
**Verification:** `scripts/proofs/proof_reflection_flow_parity.py` (16/16 PASS, <1 s; symbolic sympy + 50-digit mpmath, two independent numeric routes for the slope).
**Closes:** — (adds the exact stratum beneath `EXPLR_EULER_RATIO_RICCI_FLOW.md`'s [CONJECTURE] layer; promotes nothing there).
**Audience:** project owner + agents working on the Ratio-and-the-Arrow thread, the modulus/argument frontier's algebraic face, or FTD-0127's boundary-transcendental identities.

---

## §0 — Scope

Both branches of the Euler reflection formula satisfy first-order linear ODEs. This note records the exact, classical fact that their flow *coefficients* split along the same modulus/argument line as everything else in the G\*-neighborhood — by digamma parity, by differential algebraicity, and by value-class at the lemniscatic point. Every identity below is classical mathematics, machine-verified in the companion script; nothing here is an FTD theorem, an α-route, or a promotion. The one standing temptation is named in §4.

## §1 — The two flow equations [THEOREM — classical]

For P(z) = Γ(z)Γ(1−z) and R(z) = Γ(z)/Γ(1−z):

$$P'(z) = c_P(z)\,P(z), \qquad c_P(z) = \psi(z) - \psi(1-z) = -\pi\cot(\pi z)$$
$$R'(z) = c_R(z)\,R(z), \qquad c_R(z) = \psi(z) + \psi(1-z)$$

The first coefficient identity is the differentiated reflection formula (verified symbolically, F1); the second is term-by-term differentiation (F2). Note the parity cross-over: the **product** (the χ-even Γ-combination) is driven by the **odd** digamma combination, and the **ratio** (χ-odd) by the **even** one.

## §2 — The three-level split [THEOREM — classical]

| level | product branch (modulus side) | ratio branch (argument side) | check |
|---|---|---|---|
| coefficient form | c_P = −π·cot(πz) — elementary, closes in the π/trig world | c_R = 2ψ(z) − c_P — digamma-valued, not elementary | F1/F2/F5 |
| differential algebraicity | c_P satisfies its **own autonomous first-order algebraic ODE**: c_P′ = π² + c_P² (Riccati form) | c_R is **hypertranscendental** — it satisfies *no* algebraic differential equation over ℂ(z) | F4 + Hölder |
| value at z = 1/4 | c_P(1/4) = **−π** | c_R(1/4) = ψ(1/4) + ψ(3/4) = **−2(γ + 3 ln 2)** = −5.31331441316… | F3 |

The hypertranscendence step: differentially algebraic (DA) functions form a field closed under differentiation, antidifferentiation, and exp. If c_R were DA, then ψ = (c_R + c_P)/2 would be DA (F5 verifies the reduction; c_P is DA by F4), hence log Γ (an antiderivative of ψ) and Γ = exp(log Γ) would be DA — contradicting **Hölder's theorem (1887)**: Γ satisfies no algebraic differential equation. Hölder is cited classical mathematics, not machine-checked; every algebraic step around it is verified.

The value-level split lands in already-charted territory: −π is the π-world; −2(γ + 3 ln 2) is exactly the γ/log **boundary-transcendental class** of FTD-0127's L′(s, χ₋₄) identities (L′(0, χ₋₄) = log(G\*/2); L′(1, χ₋₄) ∋ γ + log(2π/G\*²)). The ratio branch's slope at the lemniscatic point is a new member of that family, not a new kind of object.

## §3 — Readings `[coherent-interpretation]` — none of these is a theorem

- **The frontier's algebraic face, one derivative down.** The modulus half (product) is a flow whose law is *self-closing*: its coefficient satisfies its own autonomous algebraic ODE. The argument half (ratio) is a flow whose law **cannot be written in any differentially-algebraic world**: to state R's flow you must import the digamma — the flow's own trajectory data can never close over it. This is the modulus/argument split (`FOUND_MODULUS_ARGUMENT_FRONTIER.md` §2, algebraic face) restated at the level of flow *laws* rather than values.
- **The Arrow, sharpened.** `PAPER_RATIO_AND_THE_ARROW.tex` and `EXPLR_EULER_RATIO_RICCI_FLOW.md` read the ratio branch as the time-asymmetric one. This note adds the exact stratum: the symmetric branch's dynamics is differentially algebraic; the asymmetric branch's dynamics is hypertranscendental. The [CONJECTURE]-grade flow readings in the Euler-ratio doc gain one exact statement beneath them — their tag does not move.
- **G\* now carries a canonical first-order datum.** Beyond its value, the lemniscatic point has an exact flow rate under z-deformation: (log R)′(1/4) = −2(γ + 3 ln 2). Any future deformation-theoretic treatment of the spine's objects inherits this slope for free.

## §4 — The named temptation (sentence-level, no section of caveats)

The hypertranscendence of c_R will look, to some future session, like a *mechanism* for why α-class values resist derivation — "the ratio branch's flow law is unwritable, hence…". Resist the promotion: MC-T4.3 is a statement about FTD's substrate operations, not about differential algebra over ℂ(z), and no implication in either direction has been established. The correct reading is shape-consonance only: the wall's silhouette recurs one derivative down. [coherent-interpretation, deflationary] Likewise, building any first-order ODE "whose solution is a physical constant" from these ingredients is a substitution identity under the standing Epistemic Discipline.

## §5 — Cross-references

- `scripts/proofs/proof_reflection_flow_parity.py` — the 16-check verifier (F1–F6).
- `docs/theory/02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md` §2 — the algebraic face this note deepens (row annotated with FTD-0367).
- `docs/theory/09_mathematical/number_theory/EXPLR_EULER_RATIO_RICCI_FLOW.md` — the [CONJECTURE] interpretive layer above this note.
- `docs/papers/src/PAPER_RATIO_AND_THE_ARROW.tex` — the product/ratio dichotomy at value level.
- `docs/theory/09_mathematical/number_theory/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md` (FTD-0127) — the γ/log boundary-transcendental family the slope value joins.
- `docs/theory/09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` (FTD-0366) — the ensemble realization of the same parity split at value level; this note is its ODE-level sibling.
- Classical sources: O. Hölder, *Ueber die Eigenschaft der Gammafunction keiner algebraischen Differentialgleichung zu genügen*, Math. Ann. 28 (1887) 1–13; Gauss digamma theorem (values of ψ at rationals) — see `docs/reference/REF_BIBLIOGRAPHY.md`.

---

*Every identity here is classical; the reading is FTD's and is tagged as such. No FTD claim moves.*
