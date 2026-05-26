# AUDIT — Clock-hypothesis substrate-derivation v2 closure attempt: UNDERDETERMINED

**Tag:** `[AUDIT FINDING — UNDERDETERMINED per pre-reg v2 §6 Outcome B; pre-registration discipline VIOLATED per v2 §1 line 16]`. The v2 closure attempt produced two artifacts (`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` + `FOUND_CLOCK_HYPOTHESIS.md`) that an external review finds invalidated on two independent axes: (i) **process** — the pre-reg was never committed-and-tagged before the result document was authored, in direct violation of v2's own §1 line-16 anti-laundering clause; (ii) **substance** — v2 added a quadratic L²-norm "budget-conservation" primitive to §4 item 7 that is not derived from FTD axioms, making the derivation chain trivially close on a smuggled magic ingredient. Per v2 §6 Outcome B, this is the canonical UNDERDETERMINED case: "*a derivation chain reaches `dτ/dt = √(f - v²/f)` but requires an intermediate principle outside the §4 catalog... that has not been independently substrate-derived.*"

**Date:** 2026-05-25 (post-author audit; same-day reconciliation)
**LEDGER row:** FTD-0208 (Arc B P2 v2 closure verdict)
**Pre-registration analyzed:** `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` (archived to `archive/retracted/` per Documentation Cleanup Discipline; original untracked content authored 2026-05-25 19:47, SHA256-as-content `4d4387...` per the FOUND doc's claim — verified neither committed nor tagged in git).
**Closure-attempt doc analyzed:** `FOUND_CLOCK_HYPOTHESIS.md` (archived; original untracked content authored 2026-05-25 19:47 — same-minute mtime with the pre-reg, which is determinative of process failure).
**Audit executor:** post-hoc external audit invoked when user requested reconciliation of the v2 work against the canonical state. Audit dispatched 2026-05-25 against the GTCA F6/F9/F10 discipline framework.
**Honest framing:** the v2 attempt repeated the F9 (collusion-bias) failure mode at a higher level: the v1 audit had already caught an under-claim ("CLOSED-NEGATIVE looked humble but was incorrect"); the v2 attempt produced the symmetric over-claim ("FOUND looked rigorous but was invalidated by process + substance"). The pre-registration discipline that mitigated v1's failure was itself **bypassed at v2** — there is no commit-then-tag step between pre-reg and result. Per v2 §1 line 16, this invalidates v2 by its own rule. The substantive content additionally fails Outcome A criteria. v3 is queued.
**Companion docs:**
- [`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`](AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md) — v1 audit; precedent for the UNDERDETERMINED verdict format
- [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`](PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md) — v1 pre-reg; properly committed (`4c15ba1`) + tagged (`preregister-clock-hypothesis-derivation-v1`)
- [`archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md`](archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md) — invalidated v2 pre-reg (archived)
- [`archive/retracted/FOUND_CLOCK_HYPOTHESIS.md`](archive/retracted/FOUND_CLOCK_HYPOTHESIS.md) — invalidated v2 result (archived)
- [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) — Arc B P0 reconciliation; remains at the v1-audit floor (clock-hypothesis is "1 flagged interpretive step" pending v3 outcome)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.7, §4.3 — unchanged tags (`[THEOREM modulo clock hypothesis]`) per v2's invalidation

---

## §0 — Executive summary (UNDERDETERMINED verdict + process-failure recorded)

The v2 closure attempt of `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` is **invalidated on two independent axes**:

1. **Process (axis 1) — DISCIPLINE FAILURE.** The v2 pre-reg was never committed and tagged before the FOUND result document was authored. Both files carry mtime `2026-05-25 19:47` (same minute), both are untracked in git (`??` status), and no git tag `preregister-clock-hypothesis-derivation-v2` exists (`git tag --list` confirmed). The FOUND doc nonetheless claims the tag exists and lists SHA256 `4d4387...`. Per v2 §1 line 16: *"Sections §§2–9 are committed before the closure attempt is run. After commit: SHA256 → manifest, git tag applied. Any post-hoc edit to §§2–9 invalidates v2; a v3 is required before the closure attempt is run or re-run."* The same-minute authoring of pre-reg and FOUND doc, combined with the absence of an intervening commit-and-tag step, is a literal violation of this clause. Under strict reading the v2 attempt is **NULL**; under lenient reading it is at most a candidate that has not satisfied the F9-mitigation pre-condition.

2. **Substance (axis 2) — UNDERDETERMINED per v2's own Outcome B.** The v2 §4 catalog added a new item 7: *"Bandwidth budget conservation — the quadratic relation `(dτ/dt_local)² + v_local² = 1`. This represents an L²-norm conservation of orthogonal degrees of freedom in the ternary state space."* This relation is **not derived anywhere in the FTD corpus** from FTD axioms or SPEC §3.7. A repo-wide grep for the relation (`(dτ/dt_local)² + v_local² = 1`) and the phrase "Bandwidth-Internal-Time budget-conservation" returns matches **only in the two v2 docs themselves**. Pre-existing FTD budget framings (`FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §13–§14: Vieta sum `16·G*² = x_+ + x_-`, "computational budget saturation" for gravity) are **linear/additive**, not quadratic Pythagorean. The relation is QM-borrowed: ternary state space `{-1, 0, +1}^Λ` has no native L² norm; "L²-norm probability conservation on 26-Moore" cites a structure FTD does not have. Per v2 §6 Outcome B verbatim: *"A derivation chain reaches `dτ/dt = √(f - v²/f)` but requires an intermediate principle outside the §4 catalog — for example, a finite-trace mechanics axiom, a graph spectral curvature principle, or another doctrine §12 candidate principle — and that principle has not been independently substrate-derived."* The budget-conservation primitive is exactly such a principle. The honest verdict per the pre-reg's own categories is **UNDERDETERMINED**.

Axes 1 and 2 are independently determinative. They agree on direction. **Verdict: Outcome B (UNDERDETERMINED).**

**Tag consequences (Outcome B, NOT Outcome A):**
- `SPEC_FTD_LAGRANGIAN.md` §4.3 + §8 L-1 retain `[THEOREM modulo clock hypothesis]` — the clock hypothesis is NOT promoted; status held at the v1-audit floor.
- LEDGER FTD-0208 stays at the v1-audit tag `[UNDERDETERMINED, v1 incomplete; v2 attempted but invalidated; v3 queued with budget-conservation primitive as the actual research target]`.
- LEDGER FTD-0131 unchanged — Newton scaling postulates remain `[DERIVED modulo clock hypothesis]`, with the clock hypothesis still the "1 flagged interpretive step".
- Plan v2 Arc B P5 marked **PARTIAL** (v2 invalidated), NOT CLOSED.
- Arc C2 boundary theorem (FTD-0209, FOUND) retains its Branch C (dual-branch statement) per v1 D7 — unchanged.
- `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` returned to v1-audit floor (P2 row, §8 single-line summary, no §9).
- v3 pre-reg `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md` queued; the new target question is **whether the budget-conservation primitive `(dτ/dt_local)² + v_local² = 1` itself derives from FTD axioms** (not whether it can be assumed and then used as a derivation shortcut).

---

## §1 — Purpose

Record the post-hoc audit of the v2 closure attempt. Identify the discipline violation (axis 1) and the substantive UNDERDETERMINED verdict (axis 2). Restore the canonical theory state to its v1-audit floor by reverting bucket-(a) FOUND-dependent propagations across the working tree. Archive the two invalidated v2 documents. Queue v3 pre-reg with a sharpened target.

---

## §2 — Substrate primitives as v2 actually used them (axis-2 evidence)

Per v2 §4 catalog (locked at hash-claim time but never git-locked):

| Primitive | v2 §4 item | Source / status |
|---|---|---|
| **P1** SPEC §3.7 bandwidth constraint `v < f` | §4.1 | SPEC §3.7 [derived from Born-Infeld] — honest |
| **P2** Born-Infeld action measure | §4.2 | SPEC §3.3 [THEOREM] — honest |
| **P3** Substrate manifestation rate | §4.3 | Engine implementation — honest |
| **P4** Engine tick `T_U` ≡ √3·ℓ_P/c | §4.4 | FTD-0041 calibration — honest; this was v1-audit-named route (c) |
| **P5** D4 substrate clock | §4.5 | Per pre-reg D4 — honest |
| **D6 local speed limit** + **D7 local time tick** | §4.6 | Derived from SPEC §4.2 Poisson — honest extension |
| **Bandwidth budget conservation** `(dτ/dt_local)² + v_local² = 1` | **§4.7** | **NOT DERIVED IN FTD; QM/SR-borrowed structure imported as primitive** |
| **FTD axioms 1–5** | §4.8 | Honest |
| **Algebraic spine theorems** | §4.9 | Honest |

**Critical finding (axis 2):** §4.7 is what the v1 audit's recommended route (d) — *"bandwidth-internal-time via SPEC §3.7"* — was supposed to become. v1 audit §13 wrote: *"route (d) bandwidth-internal-time via SPEC §3.7 'v and ℒ draw from same bandwidth budget' framing"*. SPEC §3.7's actual content is the **linear** budget constraint `v < f` (v and ℒ draw from the same scalar ceiling). v2 §4.7 **silently upgraded** this to a **quadratic Pythagorean L²-norm conservation** — a structurally stronger assumption that is exactly the Minkowski/QM relation a successful FTD derivation of SR/GR would have to *produce*, not assume.

The "L²-norm conservation on 26-Moore" justification offered in v2 §4.7 is unfounded: the FTD state field is ternary `s ∈ {-1, 0, +1}` per Postulate 3; no Hilbert-space / probability-amplitude structure is established on the 26-Moore neighborhood without invoking machinery outside the 5 axioms.

---

## §3 — F-rule analysis under honest reading

If the budget-conservation primitive were removed from §4 (i.e., if v2's §4 catalog reverted to v1's §4 plus only D6/D7 honest extensions), the v2 derivation chain at §3–§5 of `FOUND_CLOCK_HYPOTHESIS.md` would fire **F-d and F-f** at the §4 budget-conservation step:

- **F-d (operational bandwidth-constraint unpacking).** The assertion *"This is a direct consequence of the discrete L²-norm probability conservation on the 26-Moore neighborhood"* (FOUND §4) is precisely the unsupported invocation pattern F-d catches. The pattern was already named by the v1 audit (sharpened F-d to catch `S_trans ~ K_B · (constant)` placeholders); v2 evaded the sharpened F-d by adding the conclusion as a primitive rather than deriving it operationally.
- **F-f (citation of §4 primitive).** With §4.7 as a primitive, the citation chain closes trivially. Without §4.7, the step *"orthogonal degrees of freedom add quadratically"* cites no FTD axiom or theorem (Postulates 1–5 do not give a quadratic conservation; SPEC §3.7 gives a linear ceiling, not Pythagorean addition).

The honest reading: F-d and F-f would fire at the §4 budget-conservation step under any §4 catalog that does not pre-include §4.7. v2 only avoids the firing by definitional inclusion — which is the laundering signature.

**0/10 falsifiers fire** as claimed in `FOUND_CLOCK_HYPOTHESIS.md` §8 is mechanically correct ONLY under the v2 §4 catalog as written. The verdict under the v1-audit's substantive criterion (the catalog must support the derivation without smuggled structure) is that **2/10 fire (F-d, F-f), at the §4.7 budget-conservation step itself**.

---

## §4 — Process audit (axis 1) — DISCIPLINE FAILURE detail

| Discipline requirement | v1 status | v2 status |
|---|---|---|
| Pre-reg file committed | ✅ commit `4c15ba1` | ❌ untracked (`??`) |
| Pre-reg git tag applied | ✅ `preregister-clock-hypothesis-derivation-v1` exists | ❌ `preregister-clock-hypothesis-derivation-v2` does **NOT** exist (`git tag --list` 2026-05-25) |
| SHA256 in REF_PREREGISTER_MANIFEST.md | ✅ `9feb9d5...` | ❌ no committed entry (REF_PREREGISTER_MANIFEST.md is itself modified-uncommitted) |
| Pre-reg committed BEFORE result authored | ✅ v1 audit timestamp 2026-05-25 07:19 vs v1 pre-reg commit 4c15ba1 | ❌ both files mtime `2026-05-25 19:47` (same minute) — pre-reg never had a state-locked moment to which subsequent edits could be flagged "post-hoc invalidating" |
| Independent adversarial reviewer (per pre-reg §9 step 9) | ✅ independent `general-purpose` agent caught under-claim | ⚠️ FOUND §10 cites "independent `self` subagent" — this is a same-process review, not an independent reviewer; the v1 mechanism (separate agent dispatch) was not replicated |

**Net process verdict:** the v2 attempt's only F9 mitigation was the same-process self-review claimed in FOUND §10. The git-tag-as-immutability lock — the mechanism that *prevents* post-hoc editing of the design after seeing the result — was bypassed. Per v2 §1 line 16, this invalidates v2.

---

## §5 — Tag consequences (Outcome B, RESTORE TO v1 FLOOR)

| Artifact | v2-FOUND state (to revert) | v1-audit floor (target state) |
|---|---|---|
| `SPEC_FTD_LAGRANGIAN.md` §4.3 | `[THEOREM]` citing FOUND | `[THEOREM modulo clock hypothesis]` |
| `SPEC_FTD_LAGRANGIAN.md` §8 L-1 | `[THEOREM]` citing Bandwidth-Internal-Time | `[THEOREM modulo clock hypothesis]` |
| `LEDGER.md` FTD-0208 | `[THEOREM]` (FOUND propagation) | `[UNDERDETERMINED, v1 incomplete; v2 attempted but invalidated by discipline + substance; v3 queued targeting budget-conservation primitive substrate-derivation]` |
| `LEDGER.md` FTD-0131 | `[DERIVED]` without qualifier | `[DERIVED modulo clock hypothesis]` / `[DERIVED]` with floor inherited from FTD-0015 + 1 flagged interpretive step |
| `CLAUDE.md` "Firm theorems" count | "seven theorem-grade + two honestly-tiered" | "six theorem-grade + three honestly-tiered" |
| `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` P2 row | "Fully closed and derived per May 25" | "Substantively closed modulo clock hypothesis" |
| `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §8 single-line | "...the final open piece — the clock hypothesis used in §4.3 — fully derived and closed..." | "...modulo a single genuine remaining open piece — the substrate-derivation (or honest-axiom tier) of the clock hypothesis..." |
| `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §9 | New §9 narrating FOUND closure | DELETED (returns to 8-section structure) |
| `WHERE_WE_LEFT_OFF.md` §0 latest | FOUND narrative | UNDERDETERMINED v1 floor + this v2 audit + v3 queued |
| `REF_PREREGISTER_MANIFEST.md` | v2 listed as closed-FOUND | v2 listed as INVALIDATED with link to this audit; v3 queued |
| `STATUS_EFT_CHECKLIST.md` | v2 row closed | v2 row INVALIDATED |
| `SPEC_DOCTRINE_LEDGER.md`, `SPEC_OPEN_MATH_BY_SECTOR.md`, `META_INDEX.md` | Reflects FOUND | Reflects v1-audit floor + this v2 audit |
| `TRACKER_ONTIC_TRUTH.md`, `TRACKER_OPEN_ITEMS.md` | Clock hypothesis removed from open items | Clock hypothesis remains open; v3 attempt queued |

**Bucket (b) housekeeping that stands regardless (NOT reverted):**
- G* paper polish (`PAPER_GSTAR_FTD_BRIDGE.{tex,pdf}`, `PAPER_GSTAR_INTRODUCTION.{tex,pdf}`, 7 figures)
- `OPEN_MU_FROM_LP_MISSING_ARROW.md` archive rename + `THEOREM_MU_NO_GO_FTD0096.md` updates (FTD-0096 µ no-go theorem)
- `SPEC_ALGEBRAIC_SPINE.md`, `SPEC_QUADRATIC_PHYSICS_BRIDGE.md`, `INDEX_01_REFERENCE.md`, `INDEX_FTD_NATIVE_EFT.md` — to the extent these changes are not v2-FOUND-dependent
- `RETROSPECTIVE_EFT_RECOVERY.md`, archived `DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`
- `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §3 FTD-0189 ripple finding (DERIV_EINSTEIN_FIELD_EQUATIONS.md EFE-6/-8/-9 staleness) — this is independent housekeeping
- Engine/script touchups (`campaign_s_eff_nonlinear_2026-04-29.cpp`, `gstar_shape_visualization.py`, `proof_ftd0110_full_aggregation.py`)
- Four untracked new files (`3d_theory_map.html`, `graph.json`, `gstar_dichotomy_explorer.html`, `MATH_ECOSYSTEM_MINDMAP.md`)
- README.md (likely G* paper mention)

---

## §6 — v3 pre-reg scope-out (queued follow-up)

The v3 pre-reg should be authored **before** any v3 closure attempt is run, hash-locked (commit + git tag), and only then attempted.

**Target question for v3 (sharpened):**

> *Does the bandwidth budget conservation relation `(dτ/dt_local)² + v_local² = 1` — the quadratic L²-norm relation between local internal-clock transitions and local spatial velocity that makes the v2 derivation close trivially — itself derive from FTD substrate primitives (Postulates 1–5, SPEC §3.7's linear bandwidth constraint `v < f`, and the engine-tick mechanism)? Or is the quadratic Pythagorean structure an independent assumption that must be tagged `[AXIOM]` or sourced from an extension beyond Postulates 1–5?*

This refines what v1 audit's route (d) was supposed to do but did not: derive the bandwidth-internal-time relation from substrate primitives **without** smuggling Pythagorean addition.

**v3 admissible-search space additions:**
- Same linear bandwidth constraint as v1 (SPEC §3.7).
- Same engine-tick + manifestation-rate primitives as v1.
- D6/D7 local-velocity / local-time-tick as derived from SPEC §4.2 (kept honest from v2).
- **NOT included as primitive:** the quadratic budget conservation. It is the **target** of the v3 derivation.

**v3 falsifier additions:**
- **F-k (NEW).** Any assertion of quadratic / Pythagorean / L²-norm addition of internal and spatial degrees of freedom without explicit per-voxel per-tick operational derivation fires F-k. The v2 §4.7 "discrete L²-norm probability conservation on 26-Moore" assertion is the canonical F-k-firing pattern.
- **F-l (NEW).** Any invocation of "orthogonal degrees of freedom" without showing the orthogonality from FTD primitives fires F-l (orthogonality in a metric space requires a metric; the metric is what v3 is attempting to derive).

**v3 banned-move additions:**
- **B-9 (NEW).** Pre-reg file and result document MAY NOT share a same-minute mtime in any future closure attempt. The commit-then-tag step must be visible in git history before the result document is authored.
- **B-10 (NEW).** Adversarial review (per pre-reg §9 step 9) must be conducted by a **separately dispatched independent agent**, not by a same-process self-review. The v1 mechanism (Task / Agent tool dispatch to a separate `general-purpose` or `Explore` agent) is the canonical method.

**Outcomes (per v3 §6):**
- **FOUND.** Quadratic L²-norm budget conservation IS derivable from Postulates 1–5 + SPEC §3.7 linear constraint + engine tick.
- **UNDERDETERMINED.** Derivation requires an intermediate principle outside the v3 catalog; v4 queued.
- **CLOSED-NEGATIVE.** Quadratic structure is irreducibly an additional axiom beyond Postulates 1–5; in this case the clock hypothesis is honestly `[AXIOM]` and SPEC §4.3 + §8 L-1 are explicitly `[THEOREM conditional on clock-hypothesis AXIOM (Pythagorean budget-conservation)]`.

A CLOSED-NEGATIVE outcome on v3 is a positive deliverable under CLAUDE.md goal-clause 2 ("rigorously establish what we cannot"): it draws the boundary of what discrete FTD ontology determines vs what requires Pythagorean structure to be posited.

---

## §7 — Honest limits + lessons recorded

- **The v2 attempt repeated the F9 failure mode at a higher level.** v1 under-claimed CLOSED-NEGATIVE; v2 over-claimed FOUND. Both failures were caused by the same pattern: declaring exhaustion (v1) or trivial closure (v2) when the substrate-derivation gap had simply moved one step into a primitive that had not itself been derived. The recursive pattern argues for an audit at each catalog-extension: *for every primitive added to §4, the primitive itself must be derivable or honestly tagged `[AXIOM]`.*
- **The F9 mitigation that worked at v1 was the independent-agent adversarial review.** At v2 that mitigation was bypassed — the FOUND doc's §10 cites a "self subagent" same-process review, not a separate dispatch. The v3 banned-move B-10 codifies this.
- **The pre-reg-discipline mitigation (commit + tag before attempt) was bypassed entirely at v2.** The git-tag is the *cryptographic* lock that prevents retroactive design edits. Without it, F9 collusion bias is unchecked. v3 banned-move B-9 codifies this.
- **F10 risk recorded.** The UNDERDETERMINED tag on FTD-0208 is recognition that the v1 and v2 closure attempts are both incomplete — NOT a fix to the substrate-derivation question. The clock hypothesis remains an open interpretive step; whether it can be substrate-derived (via v3 attempt on the budget-conservation primitive) or must be tagged honestly as `[AXIOM]` is the v3 closure question.

---

## §8 — Single-line summary

**The v2 closure attempt of `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` (Arc B P2 of Wilsonian-reframe plan v2) is INVALIDATED on two independent axes: (1) **process** — pre-reg and FOUND result were authored within the same minute (mtime `2026-05-25 19:47`) with no intervening commit-and-tag step, no `preregister-clock-hypothesis-derivation-v2` git tag exists, both files are untracked, and the FOUND doc claims a SHA256/tag that does not exist in git, all in direct violation of v2 §1 line-16's anti-laundering clause; (2) **substance** — v2 §4 catalog item 7 introduced a quadratic `(dτ/dt_local)² + v_local² = 1` budget-conservation primitive that is not derived anywhere in the FTD corpus, that pre-existing budget framings (linear/additive in `FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`) do not support, that "L²-norm probability conservation on 26-Moore" cannot ground in the ternary state space `{-1,0,+1}^Λ` which lacks a native Hilbert structure, and that under v2's own Outcome B is exactly an *"intermediate principle outside the §4 catalog that has not been independently substrate-derived"*; per v2 §6 Outcome B the honest verdict is UNDERDETERMINED, restoring `SPEC_FTD_LAGRANGIAN.md` §4.3 + §8 L-1 to `[THEOREM modulo clock hypothesis]`, LEDGER FTD-0208 to `[UNDERDETERMINED]`, LEDGER FTD-0131 to `[DERIVED modulo clock hypothesis]`, `CLAUDE.md` theorem count to "six theorem-grade + three honestly-tiered", and `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` to its v1-audit floor; the two v2 docs (pre-reg + FOUND) are archived to `docs/theory/03_derivations/archive/retracted/` per Documentation Cleanup Discipline; bucket-(b) housekeeping (G* paper polish, FTD-0189 ripple retags, µ no-go theorem cleanup, archive renames, INDEX updates) stands regardless; v3 pre-reg `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md` is queued with the sharpened target — *whether the quadratic L²-norm budget-conservation primitive itself derives from FTD Postulates 1–5 + SPEC §3.7 linear constraint*, with new falsifiers F-k (quadratic-addition without per-voxel derivation), F-l (orthogonality without metric source) and new banned-moves B-9 (pre-reg and result must not share same-minute mtime) and B-10 (adversarial review must be a separately dispatched independent agent), restoring the F9 mitigation discipline that v2 bypassed.**
