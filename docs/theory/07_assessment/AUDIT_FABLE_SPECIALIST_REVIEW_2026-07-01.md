# AUDIT — Fable Two-Specialist Review (Mathematics + Physics), 2026-07-01

**Tag:** `[CRITIC SYNTHESIS]` — adversarial review record, not a theory claim.
**LEDGER id:** FTD-0348.
**Relation to prior passes:** the third same-day adversarial pass (after the 21-agent red-team,
FTD-0345/0346, and the Opus provisional review, FTD-0347), run on the two specialist reviewer
agents (`.claude/agents/ftd-math-redteam.md`, `ftd-physics-redteam.md`) **on their intended
`model: fable` for the first time**, against HEAD `85c4be23`. Both were instructed: form
impressions from primary sources first; anything already recorded FIXED/FLAGGED in the two
prior capture docs is not a new finding; the job is what those passes missed or got wrong.

## §0 · Mandatory disclosure

AI-simulated review, not external human validation (`TRACKER_OPEN_ITEMS.md` §0 stands). The
physics reviewer's shell was classifier-blocked mid-run — its numerics were hand arithmetic,
subsequently machine/read-verified by the controller before any fix (all key claims held).
The math reviewer had shell access: 78-check mpmath harness (dps 50–100), all seven cited
proof scripts re-run, Lean artifacts compiled (`lean/Standalone.lean`: bare Lean 4.30.0,
`#print axioms = [propext, Quot.sound]`).

## §1 · Headline

**Both reviewers overturned the prior passes' "surviving defects are bookkeeping" verdict.**
The core held again — Theorems 1/2/5/8/9 and the tracker's T1 layer attacked and survived,
"frequently to 60 digits"; the K-BIND chain's load-bearing core (FTD-0243 + FTD-0314's
squarefree argument) verified sound; `x₊=1/α` nowhere promoted; Phase-G, Λ=0, and the spin-2
scoping survived. But the serious defects sat exactly where earlier passes spent least time:
an invalid live `[THEOREM]` contradicting the flagship PROVEN boundary, a false central lemma,
a false subsidiary theorem, and a wrong-by-2× arithmetic step in the Einstein chain. Bonus
discovery: the FQCR convergence constant "C ≈ 0.046" is analytically **G\*/64 = 0.046229**.

## §2 · Findings — FIXED same day (all under FTD-0348; per-fix commits on `main`)

| Sev | Domain | Finding | Fix |
|---|---|---|---|
| CRITICAL | Phys | `DERIV_NONCOMMUTATIVE_EMERGENCE.md` — live `[THEOREM]` deriving "emergent non-commutativity" via an invalid proof (denying-the-antecedent at step 7; unsupported Type III₁ claim contradicting FTD-0225), the exact falsifier §2a's PROVEN row names; cited live in 3 indexes | **RETRACTED** with full notice; archived to `03_derivations/archive/retracted/`; all 3 citations corrected (META_INDEX 3.61, INDEX_03, MONOGRAPH_EFFECTIVE_EQUATIONS §5.5/§7.2) |
| HIGH | Phys | Einstein doc Step 4: α_G defined with m_e² but proton formula applied ((m_p/m_e)²≈3.4×10⁶ mismatch); α²⁰ stated 2× wrong (3.647e-43 vs true 1.834e-43); result 6.7M× its own definition; false whitepaper citation; EFE-7 `[THEOREM]` | Definition, arithmetic (5.909e-39 vs measured 5.906e-39, +0.05%), citation, and tag all corrected; two honesty caveats added (spelling-dependence; the substrate-derived quantity is α_G(e,e) per FTD-0131) |
| HIGH | Math | FTD-0244 Lemma 1 false as stated: generator 3 (`G_BCC(0)·I`, G\*²/2π) has trace G\*²/π ∉ ℚ(G\*) unless π ∈ ℚ(G\*) — contradicting the corpus's own Theorem 9 | **Partially fixed** — see §3 (the full field-enlargement rewrite is flagged; the reviewer confirmed the conclusion is recoverable: δ ∉ ℚ̄(π,Γ(1/4)) by FTD-0314 §3-C3's squarefree argument, independently re-derived) |
| HIGH | Phys | μ/τ mass-ratio demotion of record never propagated: catalog `[DERIVED]` rows (with a mis-stated formula evaluating to 819 and a citation to nonexistent `proof_mass_ratios.py`), `DERIV_LEPTON_MASS_GEOMETRY.md` `[THEOREM]`, 4 script prints, a tracker line | All retagged `[STRUCTURALLY MOTIVATED PARAMETRIC]`; headline accounting corrected ~23→~21 derived / ~129→~131 parametric (CLAUDE.md, AGENTS.md, catalog) |
| MED-HIGH | Phys | α_G(p,p) `[THEOREM]`/`[DERIVED]` tags corpus-wide (SPEC_SM_REPLACEMENT ×2 — self-contradicting, SPEC_NOVEL_PREDICTIONS ×2, SPEC_SIX_ALGORITHMS, SPEC_FTD ×2, SPEC_FTD_REFERENCE "0.01%", script); precision manufactured by two inconsistent spellings canceling; **no catalog row existed** | All demoted `[SMP]` with the spelling-dependence caveat (+0.05% vs −0.33%); new catalog §13b covering α_G(p,p) and α_G(e,e) |
| MED | Phys | m_H "0.24% error" reproduces only vs superseded PDG-2020; vs canonical PDG 2024 (125.20±0.11) it is −0.36% = **−4.1σ** — excluded as exact | CLAUDE.md, AGENTS.md, catalog, script comparator all corrected + edition-tagged |
| MED | Phys | Confinement σ: script computed corrupted −ln(x₋/(x₋+1))=0.2857 vs its own 0.209 comparator under `[THEOREM]`; catalog copied it at `[DERIVED]` | Canonical −ln(I₁(β)/I₀(β)) restored (0.2086, 2‰); tag → constitution's `[SELECTION]`; FTD-0210 caveat noted |
| MED | Math | Spine §12–§14 tail: retired x₋=N_c presented live under FTD-0017 (the Higgs row); FTD-0097 called "not yet run" (MEASURED 2026-04-27); §9→§11 pointers; "permille" claim | All corrected with dated notes |
| MED | Math | OT-2.2 proof sketch a non-sequitur ("Γ(1/4) transcendental ⇒ G\* transcendental") + phantom "Schneider 1949" | Corrected to the Chudnovsky-1976 independence route the load-bearing docs already use |
| MED | Math | OT-3.3 conflated the 2.87M extended scan with FTD-0319's 2.65M adversarial scan (sizes + pre-reg tags crossed) | Disambiguated |
| MED | Math | Theorem 3's stated scan domain wrong: "63 fundamental discriminants (h∈{1..4}, \|d\|≤907)" is an 86-element set; h=4 truncated at \|d\|≤312 (23 skipped) | **Flagged, not fixed** — see §3 (touches a frozen pre-registration's complement count) |
| LOW | Both | Tracker 7-theorem list typo (OT-2.4 for OT-1.4) + Tier-2 header "(3)" over 7 rows; spine §1 FTD-0001→FTD-0002 + stale Chowla-Selberg dependency line; stale paths (tower doc under `electromagnetism/`, session-synthesis under `campaigns/`, +5 in CLAUDE.md); §13 table missing Theorem 8/9 rows; tower doc's stale "spine §1 drift" note + wrong §10 pointer + 1.258→1.2572 ppm; PMNS `[THEOREM]` prints (FTD-0320 demotion unpropagated) + catalog §7.1 header; dead PROTON_RATIO formula (=3519.97 under a "173 ppm" comment); script verdict banners outrunning their checks (PSLQ-linear vs field statement); `AUDIT_ALPHA_EXTRACTION` "NO ½ prefactor" false at HEAD + the Coulomb doc's two unreconciled conventions | All fixed; scripts re-run clean |

## §3 · Flagged open — NOT fixed (owner/specialist judgment required)

1. **OT-2.7 / FTD-0175 / "Theorem 16.5.1" is a false theorem as proved** (math F1, HIGH):
   the Case-A/Case-C distinction in `PAPER_GSTAR_INTRODUCTION.tex` §16.5 is purely notational
   (the roots are identically `x± = 4G*^a(2 ± √(4 − G*^{b−2a}))` in every case — verified
   symbolically and at 25 digits), so (1,3) survives every stated criterion and the claimed
   minimal-a uniqueness of (2,3) fails; the enumeration script encodes the flawed classification
   by fiat. **Repair requires a choice** (add a criterion; demote to `[SELECTION]`; or restrict
   b < 2a by definition with the selection acknowledged) that changes a T2 tracker entry and a
   paper theorem — owner decision, not a mechanical propagation. Until repaired, do not cite
   OT-2.7/FTD-0175 as `[THEOREM]`.
2. **FTD-0244 Lemma 1's field-enlargement rewrite** (math F2 residue): the false bullet
   (generator 3's invariants "in ℚ(G\*)") needs the honest repair — invariant field ℚ(G\*, π),
   with δ still outside by the squarefree argument — plus the abstract's "insurmountable Galois
   obstruction" logic description corrected (the companion matrix realizes P(x) over the base
   field; the obstruction is to root-distinguishing beables, and the non-forcing proof lives in
   FTD-0243 §5). Interlocks with the standing generator-representativeness flag (FTD-0347) —
   one specialist rewrite should do both.
3. **Theorem 3's scan-domain restatement** (math F3): honest statement is "all h ≤ 3 complete
   (43 fields) + the 20 smallest h = 4 (\|d\| ≤ 312)"; the queued `PREREG_DAMERELL_SCAN_v1.md`
   should fix its "54" complement count (→77 under its own rule) **before running** — pre-reg
   edits need owner sign-off under the frozen-registration discipline.
4. **Two PROVEN-row justification rewrites** (phys F7): §2b's row cites the π/G\* split
   (interpretive garnish) instead of the model-exhibition proof; the frontier row-2 slogan
   compresses a proven substrate-level claim and an open emergent-level claim (W-CRIT-4) into
   one implication. Wording-level, but framing-sensitive — same class as FTD-0346's owner-scoped
   work.
5. **Frontier row-4 falsifier scoping** (phys F8): as written it is nominally met by FTD-0252's
   own wave-clock; needs "exact at the ternary-state primitive level, not IR-emergent in the
   wave sector."
6. **The MONOGRAPH_EFFECTIVE_EQUATIONS sibling cluster** (controller observation during the F1
   retraction, outside both reviewers' briefs): the same document carries `[THEOREM]` rows for
   the Born rule, Schrödinger, and Dirac (§§5.1–5.3, citing `DERIV_BORN_PROPORTIONALITY_
   RESOLUTION.md` "closes FTD-0187" etc.) that appear to contradict the canon (Born `[OPEN]`
   per FTD-0187, FTD-0200 closed-negative; "never headline FTD derives Schrödinger" per
   FTD-0271). Same vintage as the retracted non-commutativity doc. **Needs its own review pass**
   — flagged, not adjudicated here.
7. **`PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex`** older "seven theorems total" framing (pre-8/9);
   ~20-doc census fixed the 6+3 count, this different formulation remains owner judgment.

## §4 · Attacked and survived (representative)

All spine numerics at 50–100 digits (74/78 harness; 4 fails were the reviewer's own methods);
Theorem 9's two subtle steps + all three companion L-value identities (one by exact
Hurwitz-ζ derivation); Theorem 8 end-to-end including the level-3 cyclotomic identity; the
K-BIND chain's genuine core (consistency witnesses recomputed; C3 squarefree argument
re-derived); FTD-0341's four carrier closures + magnitude/phase theorem (η, θ-null, weight-4
homogeneity at 59 digits); Watson closed form corroborated against the raw defining series
within a provable tail bound; the η-tower at both ℚ(i) and ℚ(√−3); Chudnovsky/Nesterenko
bookkeeping; FTD-0131's numbers (stated errors slightly conservative — the honest direction);
m_p/m_e 173 ppm; the §0.5 PROVEN/ATTEMPTED table; the spin-2 split; Phase-G's "not a QED
deviation"; Λ=0 `[DERIVED]`.

**Zero promotions across the entire pass:** `x₊=1/α` `[SMC]`; MC-T4.3 `[FOUNDATIONAL
OBSTRUCTION]`; golden `0xb604d81a3d79366e` untouched; every fix is a demotion, correction,
disambiguation, or disclosure.
