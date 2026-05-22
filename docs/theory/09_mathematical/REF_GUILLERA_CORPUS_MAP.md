# REF — Jesús Guillera's corpus, mapped to the FTD algebraic spine

**Tag:** [REFERENCE] / external-literature map. Introduces no FTD claim, no derivation, no tag change.
**Date:** 2026-05-21.
**Purpose:** a single map of Jesús Guillera's body of work — Ramanujan-type series, the Wilf–Zeilberger (WZ) method, Borwein-like algorithms, self-replication — cross-referenced to the parts of FTD it genuinely touches, and explicitly to the parts it does not.

---

## §0 — Scope (read first)

**This document IS:** a map of *external mathematics* that FTD's **algebraic-spine and computational** work draws on, or could draw on. Guillera (Universidad de Zaragoza) is a number theorist whose two-decade body of work is the mathematics of computing and proving closed-form identities for π, 1/π², and Γ-function values. Two FTD computational results verified in May 2026 — the Landen log-derivative route to `G*` (`scripts/proofs/proof_landen_gstar_compression.py`) and the quartic quarter-constant route (`scripts/proofs/proof_quartic_quarter_constants.py`) — sit *inside* his framework.

**This document IS NOT:** a claim that Guillera's work supports, endorses, or bears on FTD's **physics**. Guillera is not an FTD collaborator. His corpus concerns the computation and proof of identities for transcendental constants; it has nothing to say about `x₊ = 1/α` (FTD-0013), the boundary theorem (FTD-0186), the lattice ontology, or any FTD physics claim. **Citing Guillera in FTD work is scholarly attribution for specific mathematical tools FTD uses — it is not, and must never be presented as, third-party validation of the framework.** This sentence is the load-bearing content of §0.

---

## §1 — Why the corpus is relevant at all

FTD's algebraic spine occupies one specific place in the mathematical landscape: the lemniscatic CM point τ = i, the Kronecker character χ₋₄ of Q(i), and master-quadratic / AGM-type recurrences (`SPEC_ALGEBRAIC_SPINE.md`; `docs/papers/PAPER_GSTAR_INTRODUCTION.tex`). Guillera spent two decades on the mathematics of *exactly that neighbourhood*: Ramanujan-type series (which arise from CM points), Borwein-type algorithms (AGM-family recurrences), and the elementary proof of such identities.

The resonance is therefore real — but it is **neighbourhood resonance**: FTD's spine and Guillera's corpus are in the same room of mathematics. `[OBSERVATION]` It deepens and could rigorise the *spine*; it does not reach the *physics*. The discipline of this document is to keep that distinction visible on every row.

---

## §2 — The corpus, six threads (relevance-tagged)

| Thread | Representative work | FTD relevance |
|---|---|---|
| **1. Ramanujan-type series for 1/π and 1/π²** — Guillera is credited with the first Ramanujan-type series for 1/π², a category beyond Ramanujan's own 1/π work. | "About a new kind of Ramanujan-type series" (Exp. Math. 2003); "A new method to obtain series for 1/π and 1/π²" (Exp. Math. 2006); "A new Ramanujan-like series for 1/π²" (2011, arXiv:1003.1915); "Bilateral Ramanujan-like series for 1/πᵏ" (2020). Many remain conjectural (found by PSLQ). | **Adjacent, not a bridge.** Same mathematical world as Paper A; computes 1/π², not `G*`. |
| **2. The WZ-method as a proof engine** — proving hypergeometric / Ramanujan-type identities by the Wilf–Zeilberger algorithm: a rational certificate `C(n,k)`, mechanically checkable, **no modular-form theory required**. | "On WZ-pairs which prove Ramanujan series" (2010, arXiv:0904.0406); "Proofs … using a program due to Zeilberger" (2018); "The WZ method and flawless WZ pairs" (2026, arXiv:2503.00570). | **HIGH — methodological.** Paper A's identities are exactly the kind of object WZ proves elementarily. See §5.1. |
| **3. Borwein-like algorithms & self-replication** — self-replicating functional identities generate modular forms and yield AGM-type algorithms. | "Easy proofs of some Borwein algorithms for π" (2008, arXiv:0803.0991); "New proofs of Borwein-type algorithms" (2016, arXiv:1604.00193); **"Crouching AGM, Hidden Modularity"** (Cooper–Guillera–Straub–Zudilin 2018, arXiv:1604.01106); **"Self-replication and Borwein-like algorithms"** (2018, arXiv:1702.05378). | **HIGH — computational.** The verified quartic route is *from* arXiv:1702.05378; the Landen route is the same family. This is the rigorous home of the "G\* compression" work. |
| **4. Divergent / p-adic / supercongruences** (Guillera–Zudilin) | "'Divergent' Ramanujan-type supercongruences" (2012, arXiv:1004.4337); "Mosaic supercongruences" (2012); q-analogues; "Heuristic derivation of Zudilin's supercongruences" (2025). | **NON-BRIDGE.** Deep number theory, but "p-adic" is not FTD's "discrete substrate" — unrelated structures. See §4. |
| **5. The lemniscate constant & quarter neighbourhood** | A fast direct-hypergeometric formula for the lemniscate constant ϖ (samples catalogue, post-2010, used in record computations); "Ramanujan series upside-down" (2014, with M. Rogers, arXiv:1206.3981) — the χ₋₄ value yields Catalan's constant. | **HIGHEST — read precisely.** Since `G* = 2ϖ/√π`, a fast ϖ formula *is* a fast `G*` computation. The χ₋₄ overlap with Paper A is *the same classical character* — confirmation that Paper A sits in respectable territory, not new content. |
| **6. Chudnovsky-class series & modular polynomials** | "Proof of Chudnovsky's series for 1/π using Weber modular polynomials" (2021, arXiv:2003.06668); "The fastest series for 1/π due to Ramanujan" (2025). | **Informative bound.** Record-class — but about 1/π. No Chudnovsky-class series *for `G*` specifically* exists in the corpus. |

Full publication list: `https://anamat.unizar.es/jguillera/publications.html` (≈40 papers, 2002–2026).

---

## §3 — Where FTD cites Guillera (citation discipline)

- **`scripts/proofs/proof_quartic_quarter_constants.py`** — the quartic iteration **is** Guillera's (arXiv:1702.05378); citation is **mandatory**. Attribution confirmed by literature search 2026-05-21.
- **`scripts/proofs/proof_landen_gstar_compression.py`** — the Landen log-derivative route is classical AGM/Landen mathematics; Guillera's Borwein-algorithm papers (2008, 2016) are the modern context, cite as "see also."
- **Paper A (`PAPER_GSTAR_INTRODUCTION.tex`)** — *if* the Landen and/or quartic routes are added (e.g. as a computational remark in §4), citing Guillera (arXiv:1702.05378) and Cooper–Guillera–Straub–Zudilin (arXiv:1604.01106) is **mandatory attribution**. As of 2026-05-21 those routes are **not** in Paper A; this is a conditional, not a present obligation.
- **Anywhere else:** cite Guillera only where FTD genuinely uses a Guillera result. Do not cite him to lend the framework legitimacy by association (§0).

---

## §4 — The two non-bridges (explicit)

Two threads *look* like FTD bridges and are not. Recording them prevents the pattern-matching overreach (GTCA failure mode F1):

- **"Ramanujan-like series for 1/π² and string theory"** (Almkvist–Guillera 2012) and the Calabi–Yau-differential-equations machinery. "String theory" here means these series appear in the mathematics of Calabi–Yau periods — a statement about the *series*, not a derivation of physics. It is **not** an FTD physics bridge.
- **The p-adic / supercongruence thread** (§2 row 4). "p-adic" is a different structure from FTD's discrete lattice. The superficial word-overlap ("discrete," "p-adic") is **not** a structural connection.

---

## §5 — Genuinely actionable

### §5.1 — The WZ-method could give Paper A elementary, self-contained proofs
Paper A currently proves its identities through classical elliptic-integral / modular-form arguments and cites external theorems (Chudnovsky 1976, Rubin 1991, Deligne). Several of its identities — the Watson identity `W₃ = G*²/(2π)`, the χ₋₄ four-level relations, the new Landen log-derivative form — are hypergeometric-type identities of exactly the kind the WZ-method proves *mechanically, with a checkable certificate, without modular-form theory*. Re-deriving them by WZ would make Paper A more **self-contained and machine-verifiable** for a referee. `[OPEN — a rigour upgrade, not a new result]` Honest caveat: WZ proves *identities between sums*; it does not, and cannot, prove `x₊ = 1/α`.

### §5.2 — Guillera's fast lemniscate formula is already a fast `G*` computation
Via `G* = 2ϖ/√π`, Guillera's fast ϖ formula computes `G*` at the same speed. The earlier project interest in "a fast series for `G*`" is substantially answered by existing literature; it need not be re-derived.

### §5.3 — The self-replication papers are the parent framework
If the Landen and quartic `G*`-computation routes are written up (in Paper A or elsewhere), they should be presented as instances of Guillera's self-replication framework (arXiv:1702.05378, arXiv:1604.01106), not as novel algorithms.

---

## §6 — Cross-references

- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — the FTD algebraic spine (τ=i, χ₋₄, master quadratic).
- `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` — Paper A; the χ₋₄ unification.
- `scripts/proofs/proof_landen_gstar_compression.py`, `scripts/proofs/proof_quartic_quarter_constants.py` — the two verified `G*`-computation routes.
- Key Guillera papers: arXiv:1702.05378 (self-replication); arXiv:1604.01106 (Crouching AGM, Hidden Modularity); arXiv:0803.0991 (easy Borwein proofs); arXiv:1604.00193 (new Borwein proofs); arXiv:0904.0406 (WZ-pairs for Ramanujan series); arXiv:2503.00570 (the WZ method and flawless WZ pairs); arXiv:1004.4337 (divergent supercongruences); arXiv:2003.06668 (Chudnovsky via Weber polynomials).
- Homepage: `https://anamat.unizar.es/jguillera/publications.html` (publications) and `https://anamat.unizar.es/jguillera/samples.html` (formula catalogue).

---

*This map records adjacent mathematics. It does not move any FTD claim. The framework's open problem — the physics identification of the master-quadratic roots — is untouched by anything catalogued here.*
