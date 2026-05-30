# SCOPE — Attacking the det↔det_ζ Identity (MC-T4.3's single load-bearing hinge), v1

**Tag:** `[SCOPING / OPEN]` — this document scopes the highest-leverage open problem in the α-readout program. **It contains no result. It does not claim FOUND. It does not close MC-T4.3.** Its purpose is to state precisely what a genuine attempt would have to establish, what is already in hand, and what the prior-favoured outcome is — so that a future attempt is rigorous in whichever direction it lands.
**Date:** 2026-05-30
**LEDGER:** proposed new row **FTD-0240** (see §7; confirm next-free against `../../07_assessment/core_ledgers/LEDGER.md` at registration. Canonical LEDGER max is FTD-0237, but **FTD-0238 and FTD-0239 are already provisionally claimed by concurrent working-tree docs** — the ARC-A1-v2 boundary trio `PREREG_ALPHA_READOUT_BOUNDARY_v2.md` / `SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md` (both 0238) and `DERIV_ALPHA_READOUT_BOUNDARY.md` (0239) — so the next genuinely free id is **0240**. Confirm at registration; this is exactly the duplicate-id pattern the 2026-05-30 ledger-cleanup campaign closed, so it must be resolved before any of 0238/0239/0240 is canonized.).
**Scopes:** the determinant-grading hinge first localized by `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` (FTD-0234) and ruled **UNDERDETERMINED** by `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` (FTD-0235). This memo does **not** supersede those audits; it lays out the work that would be needed to move FTD-0235's verdict in either direction.
**Builds on (all `[THEOREM]`/`[UNDERDETERMINED]`, none re-derived here):** FTD-0233 (parity scoping), FTD-0234 (J-twisted det_ζ ratio = G\* is a clean odd scalar), FTD-0235 (det↔det_ζ identity UNDERDETERMINED), FTD-0122 (BCC complex structure $V_{\text{complex}}\cong\mathbb{Z}[i]^2$), Watson identity (`THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md`), FTD-0237 (Gaussian/Eisenstein dichotomy — why no Eisenstein forcing can supply the odd term).

> **GTCA discipline note.** This is a P5/P4 scoping memo, run wide-aperture. The central FOUND-vs-UNDERDETERMINED tension is held open deliberately (Tension Register), not collapsed. Every forward-looking statement is tagged. The prior-favoured outcome (UNDERDETERMINED) is stated as a prior, not a result. Engineering any step toward FOUND inside this memo would invalidate the next real attempt — so no construction is performed here.

---

## 1 · Why this is the single highest-leverage open problem

The entire MC-T4.3 obstruction — the reason `x₊ = 1/α` (FTD-0013) cannot be promoted past `[STRONGLY MOTIVATED CONJECTURE]` via the BCC/quantization observable route — reduces to **one** question.

The EM readout is modelled as a $2\times2$ transfer operator $T$ on the BCC complex plane $V_{\text{complex}}\cong\mathbb{Z}[i]^2$ (FTD-0122), and the master quadratic is its characteristic polynomial:
$$\det\!\big(xI - T\big) = x^2 - (\operatorname{Tr}T)\,x + (\det T), \qquad (\operatorname{Tr}T,\ \det T) = \big(16G^{*2},\ 16G^{*3}\big).$$

- **The trace $\operatorname{Tr}T = 16G^{*2}$ is forced** `[THEOREM]/[DERIVED]`. It is the Watson Green's function at the origin: $G_{\text{BCC}}(0) = \Gamma(1/4)^4/(4\pi^3) = G^{*2}/(2\pi)$, and $16 = |\mu_4|^2$ (the $\mathbb{Z}[i]$ unit group squared). Both are forward-derived FTD-native scalars.
- **The determinant $\det T = 16G^{*3}$ is the odd term, and it is *not* forced** `[UNDERDETERMINED]` (W-CRIT-2). Watson supplies $G^{*2}$, not $G^{*3}$. A clean **odd** scalar source exists — the J-twisted ζ-regularized determinant ratio $\det_\zeta(D_{3/4})/\det_\zeta(D_{1/4}) = G^*$ (FTD-0234, `[THEOREM]`, no prefactor) — and so $\det T = 16G^{*3} = \operatorname{Tr}T \cdot G^*$ is *assemblable* from forward scalars. But for a $2\times2$ operator $\operatorname{Tr}$ and $\det$ are **independent invariants**: nothing yet *forces* the determinant to carry exactly one extra factor of the det_ζ scalar relative to the trace. That extra-factor dependence $\det = \operatorname{Tr}\cdot G^*$ **is** the imposed master-quadratic Vieta structure (FTD-0001) — the target, not an output.

So the obstruction is exactly: **is $\det T = 16G^{*3}$ FORCED to equal a ζ-regularized determinant of $T$'s own J-twisted spectrum (which would supply the odd $G^*$ from the clean FTD-0234 source by a genuine operator identity), or is it merely ASSERTED as $\det = \operatorname{Tr}\cdot G^*$?** Resolve this and MC-T4.3 resolves with it: a forced identity closes it positive (ARC-3 eligibility for FTD-0013), a proof that the structure is unforced/impossible closes the BCC/quantization route negative and yields a boundary theorem.

This is one question, with a clean numerical target, sitting on top of two `[THEOREM]`-grade scalars (Watson; det_ζ ratio). That is what makes it the highest-leverage open problem in the program.

---

## 2 · (a) What is in hand

Three load-bearing facts are established and carry their canonical tags. **None is re-derived here; they are the admissible starting set.**

| # | Fact | Tag | Source |
|---|---|---|---|
| H1 | $\operatorname{Tr}T = 16G^{*2}$, with $G^{*2}/(2\pi) = G_{\text{BCC}}(0) = \Gamma(1/4)^4/(4\pi^3)$ (Watson) and $16 = |\mu_4|^2$. The **even** term is forced. | `[THEOREM]/[DERIVED]` | Watson bridge; FTD-0122; FTD-0235 §0 |
| H2 | A clean, forward, FTD-native **odd** scalar $= G^*$ exists: $\det_\zeta(D_{3/4})/\det_\zeta(D_{1/4}) = G^*$ (Model I, no prefactor). | `[THEOREM]` | FTD-0234 (FQCR `SPEC_FQCR.md` §2 Prop 1) |
| H3 | The **even-power wall**: every elliptic-modular route evaluated at the lemniscatic point $\tau = i$ yields even powers of the period, because $E_6(i) = 0$ (verified $|E_6(i)| < 10^{-15}$, machine-zero; classically $E_6$ vanishes at $i$ since $j(i)=1728$). Odd powers of $G^*$ cannot come from the weight-6 modular generator at $\tau=i$. | `[THEOREM]` (classical) | This memo §4; FTD-0233 parity scoping |

H1 and H2 together mean **the scalar $16G^{*3}$ is forward-computable** ($16G^{*3} = |\mu_4|^2 \cdot 2\pi\,G_{\text{BCC}}(0) \cdot G^*$, all forward scalars; verified $16G^{*3} = 414.39243772\ldots$ and $\operatorname{Tr}\cdot G^* = 16G^{*3}$ to 45 dp). H3 explains **why the odd factor must come from the det_ζ channel** (H2) and not from the modular/Watson channel (H1) — the even-power wall blocks the latter. The framework is therefore *not* short of an odd scalar; it is short of a *reason the operator's determinant must consume exactly one of them*.

**What is decisively NOT in hand:** a derived reason that the readout operator $T$ has the specific pair $(\operatorname{Tr},\det) = (16G^{*2}, 16G^{*3})$ rather than any other pair assembled from the same forward scalars. This is the W-CRIT-2 "master quadratic imposed not derived" gap (`SPEC_OPEN_MATH_BY_SECTOR.md`), and it is the operative obstruction (FTD-0235 §1–2).

---

## 3 · (b) The precise proof obligation

A genuine FOUND requires an **operator $T$** and a proof that all three of the following hold simultaneously, each `[THEOREM]`/`[DERIVED]`:

**Obligation A — det↔det_ζ (the hinge).** Exhibit an FTD-native operator $T$ for which $\det T$ **is** the ζ-regularized determinant of $T$'s own J-twisted spectrum,
$$\det\nolimits_\zeta T \;=\; \exp\!\big(-\zeta_T'(0)\big), \qquad \zeta_T(s) := \sum_{\lambda \in \operatorname{spec}(T)} \lambda^{-s},$$
and for which this ζ-regularized determinant **equals** $16G^{*3}$ as a *forced* consequence of $T$'s structure — not as an imposed match. The proof obligation is to show the equality $\det T = 16G^{*3}$ follows from $\zeta_T'(0)$, i.e. that the odd $G^*$ is *produced by* the determinant's spectral structure, not inserted.

> **The sharp difficulty (held open, not resolved).** A *finite* $2\times2$ $T$ with eigenvalues $\{x_+, x_-\}$ has $\det T = x_+ x_-$, an ordinary product, and its "ζ-regularized" determinant is just that product — there is no regularization content, and no mechanism forces $x_+ x_- = 16G^{*3}$ beyond *choosing* the entries. So Obligation A cannot be met by the finite readout matrix as literally stated. It would require $T$ to be (or to descend from) an **infinite** operator whose J-twisted spectrum is the FQCR shifted spectrum $\{n+\tfrac14\}/\{n+\tfrac34\}$ — but *that* operator's det_ζ ratio is $G^*$ at **degree 1** (FTD-0234), not $16G^{*3}$ at degree 3. Bridging "degree-1 det_ζ ratio on the infinite operator" to "degree-3 determinant on the finite readout" is the unmet core of Obligation A. **This memo does not bridge it; it names it as the obligation.**

**Obligation B — the three-plane / $C_3$ structure (does $D=3$ force three factors?).** The owner's hint and FTD-0237 §4 read $16G^{*3} = |\mu_4|^2 \cdot \prod_{\text{3 planes}}(\text{per-plane det}_\zeta\ \text{ratio} = G^*)$: the determinant carries **three** $G^*$ factors, one per coordinate plane of $\mathbb{Z}^3$, organized (cyclically permuted) by the $C_3$ rotation about the body diagonal $\langle111\rangle$. The proof obligation is to show that the $C_3$ structure of the BCC/$\langle111\rangle$ stencil **forces** the determinant to be a product of exactly three per-plane det_ζ ratios — i.e. that $D=3$ (three coordinate planes, each a square $\mathbb{Z}[i]$ lattice per FTD-0237) is what supplies the exponent $3$ in $G^{*3}$.

> **The sharp difficulty (held open).** FTD-0237 §3.1–§3.2 establishes that the planes are genuinely Gaussian ($\mathbb{Z}[i]$) and genuinely organized by $C_3$ — but it also establishes (Theorem GE-1 + corollary) that the *organizing 3-fold rotation* is a real symmetry while an *Eisenstein forcing* ($\mathbb{Z}[\omega]$) is ruled out. So $C_3$ is present as a rotation, but it is **not yet shown** that a rotation that *permutes* three planes *forces the determinant to multiply* their three det_ζ ratios rather than, say, average or trace them. A symmetry that permutes factors does not by itself dictate the arithmetic operation combining them. Closing Obligation B means proving the determinant (an antisymmetric, multiplicative functional) is the *correct* invariant for the three-plane structure and that it harvests one $G^*$ per plane. The trace harvests two (Watson, $G^{*2}$, two-component $\mathbb{Z}[i]^2$); the claim is the determinant harvests three. **Why two for the trace and three for the determinant, from one preparation, is exactly the gap.**

**Obligation C — symmetry-breaking co-realizability (one preparation, both invariants).** The FTD-0231 winding-index charge quantization needs a localized charge to break $O_h \to C_4$ about **one** axis, giving **one** $V_{\text{complex}}$ and **one** det_ζ ratio $= G^*$ (degree 1). A degree-3 determinant (Obligation B) needs three axes' worth of det_ζ ratios. The obligation is to show that a **single** physical preparation supplies both the trace's two-factor structure and the determinant's three-factor structure — i.e. that the trace's and determinant's required symmetry-breakings are **co-realizable**, not mutually exclusive.

> **The sharp difficulty (held open).** This is the cleanest candidate for a *negative* resolution. If the charge quantization that gives the readout its $\mathbb{Z}[i]$ structure requires breaking $O_h \to C_4$ about one axis (degree-1 det_ζ), while the determinant's $G^{*3}$ requires three axes' det_ζ ratios (degree-3), and these two symmetry-breakings cannot coexist in one preparation, then trace and determinant require **incompatible** preparations — and the master-quadratic structure is provably unforced (a boundary theorem, §5). FTD-0235 already found the assembly *possible* (the scalars exist); Obligation C asks whether it is *co-realizable from one state*. This is the most likely place a real attempt closes the question — in the negative.

**FOUND requires A ∧ B ∧ C, all derived, with no falsifier firing** (the FTD-0235 pre-reg falsifiers V1–V7 remain the mechanical gate: V1 assertion-of-det, V2 incompatible symmetry-breaking, V5 master-quadratic insertion, V7 Tr/Det independence). A real attempt that meets A∧B∧C closes MC-T4.3 positive; an attempt that proves any one of A, B, C unforced-or-impossible closes the BCC/quantization route negative.

---

## 4 · The even-power wall, stated precisely `[THEOREM]` (classical input)

This is the structural reason the odd term is hard, and it is worth stating cleanly because it scopes *where* the odd $G^*$ may legitimately come from.

At the lemniscatic CM point $\tau = i$ (the curve $E: y^2 = x^3 - x$, $j = 1728$), the weight-6 Eisenstein series **vanishes**: $E_6(i) = 0$. (Verified in-session: $|E_6(i)| < 10^{-15}$, machine-zero; this is the classical statement that $E_6$ has a simple zero at $i$.) Periods and quasi-periods of $E$ at $\tau=i$ are built from $\Gamma(1/4)$ and $\pi$, and every $SL_2(\mathbb{Z})$-modular combination that survives at $\tau=i$ enters at **even** weight (the surviving generators are powers of $E_4(i)$ and of the period $\varpi$; the weight-6 generator is killed). Consequently:

> **Even-power wall.** Any quantity assembled purely from the elliptic-modular data of $E$ at $\tau=i$ is an even power of the lemniscatic period $G^*$ (equivalently, lives in the ring generated by $G^{*2}$). The odd power $G^{*3}$ **cannot** be produced by the modular/Watson channel; it must come from a genuinely different functional — and the only clean such functional in hand is the J-twisted **ζ-regularized determinant ratio** (H2, FTD-0234), which produces $G^*$ at degree 1 by spectral (not modular) means.

This is why FTD-0237's "no Eisenstein twin" result matters here: the natural place one might hope to find an odd-degree forcing — an Eisenstein ($\mathbb{Z}[\omega]$) structure — is both (i) ruled out as a *ring* in $\mathbb{Z}^3$ and (ii) shown by Theorem GE-1 to have no canonical coefficient analog. The even-power wall (Gaussian side) plus the no-Eisenstein-twin result (FTD-0237) together say: **the odd term is structurally exceptional, and the det_ζ channel is the only admissible supplier.** The proof obligation of §3 is precisely to show the determinant *must* draw on that channel.

---

## 5 · (c) The prior-favoured outcome, and why even that is a result

**Prior-favoured outcome: UNDERDETERMINED** (consistent with FTD-0235's standing verdict, and with the independent review FTD-0232). The basis for the prior:

- The scalars are all forward-computable (H1, H2), so the obstruction is **not** parity and **not** a kind-mismatch (FTD-0235 explicitly corrected an earlier over-hardened CLOSED-NEGATIVE on exactly this point — the det_ζ ratio, $G_{\text{BCC}}(0)$, and the finite-$N$ approximant are all scalars).
- A $2\times2$ operator's $\operatorname{Tr}$ and $\det$ are independent; assembling the master-quadratic pair from forward scalars is *possible* but, on present evidence, *chosen* (the imposed Vieta target). Obligation A's sharp difficulty (finite matrix has no regularization content; the infinite operator gives degree 1, not degree 3) suggests A is **natural-yet-unforced** rather than outright impossible.
- Obligation C is the one place a *clean negative* could emerge (incompatible symmetry-breakings). If a real attempt proves C impossible, the verdict upgrades from UNDERDETERMINED to **CLOSED-NEGATIVE** for the BCC/quantization route.

**Why UNDERDETERMINED is still a deliverable — the boundary theorem.** Per the project's number-one goal (*derive what we can; rigorously establish what we cannot*), a rigorous UNDERDETERMINED is a **boundary result**: it would establish that

> the discrete ontology (five postulates + the BCC complex structure + the FQCR det_ζ scalar) **does not force** the readout operator's determinant grading — i.e. the master-quadratic operator structure is *not* determined by discreteness alone.

That is a map of how far discreteness reaches into the EM sector. It would feed directly into the FTD-0186 boundary-theorem program and would make precise what a **sixth postulate** (or **engine-native dynamics**, the surviving ARC-D route) would have to supply: a forced det↔det_ζ correspondence, i.e. a dynamical reason the determinant carries exactly one extra det_ζ factor relative to the trace. Either verdict — FOUND, UNDERDETERMINED-as-boundary, or CLOSED-NEGATIVE — advances the goal. **The one outcome that would be a failure is an *engineered* FOUND that assembled $16G^{*3}$ by choosing entries and called the assembly a derivation.** This memo exists to prevent that.

---

## 6 · What a real attempt would do (method sketch, not executed)

Ordered, so a future attempt has a checklist. **No step is performed here.**

1. **Pin the operator.** State the candidate $T$ explicitly as an FTD-native object (descended from the BCC/$\langle111\rangle$ stencil + C₄ winding preparation), with its J-twist defined. Decide whether it is finite (then Obligation A is at risk of V1) or an explicit infinite operator with a named spectrum.
2. **Compute $\zeta_T(s)$ and $\zeta_T'(0)$** for that operator; determine whether $\exp(-\zeta_T'(0))$ is $16G^{*3}$, $G^*$, or neither. If it is $G^*$ (degree 1), Obligation A as stated fails and the attempt must either reformulate the target or close negative.
3. **Test the three-plane forcing (Obligation B).** Determine whether the $C_3$ action on the three coordinate planes forces a *product* of three per-plane det_ζ ratios. Specifically: is the determinant the unique $C_3$-invariant multiplicative functional that harvests one $G^*$ per plane, or do averaging/trace alternatives survive the same symmetry? (A symmetry that permutes factors does not by itself select multiplication — this must be proven.)
4. **Test co-realizability (Obligation C).** Determine the axis-count each invariant needs (trace: the two-component $\mathbb{Z}[i]^2$; determinant: three planes) and whether one $O_h$-breaking preparation supplies both, or whether they demand incompatible breakings. This is the most promising route to a clean *negative*.
5. **Apply FTD-0235 falsifiers V1–V7 mechanically.** Numerical confirmation only after all pass. Verdict per the FTD-0235 pre-reg's three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE).

**Admissible ingredients** (frozen, per FTD-0234/0235): the FTD-0234 set + the J-twisted ζ-regularized determinant relation $\det_\zeta T = \exp(-\zeta_T'(0))$ + the $O_h/C_4/V_{\text{complex}}$ representation theory (FTD-0122). **Out of scope:** asserting $\det = \operatorname{Tr}\cdot G^*$ or $\det = 16G^{*3}$ without the det↔det_ζ derivation; importing the FQCR $M_N$ matrix as scaffold; inserting the master quadratic / its roots / Theorem 8; a transcendental prefactor; any CODATA value.

---

## 7 · LEDGER / provenance (proposed; not written here)

This memo does not edit the LEDGER (per task constraint). The proposed registration:

- **Row:** **FTD-0240** (next genuinely free; FTD-0237 is canonical max in `core_ledgers/LEDGER.md`, but 0238 and 0239 are already claimed by concurrent working-tree boundary docs — see the LEDGER line at the top of this memo). **Confirm 0240 is still free at registration time.**
- **Title:** "det↔det_ζ identity attack scope (MC-T4.3 hinge) — v1".
- **Tag:** `[SCOPING / OPEN]`.
- **Status:** open; prior-favoured outcome UNDERDETERMINED; depends on FTD-0234 (H2), FTD-0235 (standing verdict), FTD-0122, FTD-0237 (even-power wall / no-Eisenstein-twin), Watson bridge.
- **Relation to ARC-A1 v2 (FTD-0238/0239):** that boundary-torus route reaches UNDERDETERMINED via the **same even-power wall** ($E_6$ generates only even powers at $\tau=i$) but for the **boundary readout** (modular filtering of the bulk spectrum), whereas this memo scopes the **operator-determinant identity** (FTD-0235). The two are **complementary, non-overlapping** routes to the same odd-term obstruction; the shared even-power wall is mutually corroborating. Neither supersedes the other.
- **Note:** this row records a *scoping* of the FTD-0235 hinge; it does **not** move FTD-0235's verdict and does **not** touch FTD-0013 (`x₊=1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`).

---

## 8 · Scope, caveats, and what is NOT claimed

- **No FOUND.** Nothing here closes MC-T4.3 positive. The three obligations A/B/C are stated as obligations, with their sharp difficulties held open.
- **No tag promotion.** FTD-0013 unchanged; FTD-0235 verdict unchanged; the spine is untouched.
- **No construction.** No operator is built, no determinant assembled, no number engineered. Doing so would be the V1/V5/V7 failure the FTD-0235 pre-reg exists to catch, and would invalidate the next real attempt (F9/F10 discipline).
- **The even-power wall (§4) and Watson/det_ζ facts (§2) are the only `[THEOREM]`-grade content**; they are imported, not new. Everything forward-looking is `[SCOPING / OPEN]`.
- The prior-favoured UNDERDETERMINED outcome is a **prior**, not a result; the value of a real attempt is to make whichever verdict lands rigorous, and to extract the boundary theorem if it lands UNDERDETERMINED.

---

## 9 · Cross-references

- The hinge & standing verdicts: [`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md) (FTD-0234), [`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md) (FTD-0235), [`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](../preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md) (the locked V1–V7 falsifiers).
- The readout operator & quantization: [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](../derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md) (FTD-0230), [`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`](../derivations/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md) (FTD-0231), [`AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`](../../07_assessment/audits/AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md) (FTD-0232).
- The even-power wall & no-Eisenstein-twin: [`EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md`](../../09_mathematical/number_theory/EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md) (FTD-0237, incl. Theorem GE-1).
- Complementary boundary route (same even-power wall, different mechanism): [`PREREG_ALPHA_READOUT_BOUNDARY_v2.md`](../preregistrations/PREREG_ALPHA_READOUT_BOUNDARY_v2.md) + [`DERIV_ALPHA_READOUT_BOUNDARY.md`](../derivations/DERIV_ALPHA_READOUT_BOUNDARY.md) (ARC-A1 v2, FTD-0238/0239, UNDERDETERMINED); [`SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md`](../../07_assessment/SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md).
- The clean odd source: `SPEC_FQCR.md` §2 Prop 1 (Model I); G\* arithmetic identities ([`EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`](../../09_mathematical/number_theory/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md)).
- Watson trace: [`THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md`](../../09_mathematical/general_math/THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md).
- The imposed-structure gap (W-CRIT-2): `SPEC_OPEN_MATH_BY_SECTOR.md`.
- Surviving route if this closes negative: ARC-D engine-native measurement ([`SCOPE_ALPHA_READOUT_NEXT_STEPS.md`](SCOPE_ALPHA_READOUT_NEXT_STEPS.md)); boundary-theorem program (`FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md`, FTD-0186).
- Central conjecture untouched: [`CONJ_ALPHA_FROM_CM.md`](../../09_mathematical/general_math/CONJ_ALPHA_FROM_CM.md) (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`).
