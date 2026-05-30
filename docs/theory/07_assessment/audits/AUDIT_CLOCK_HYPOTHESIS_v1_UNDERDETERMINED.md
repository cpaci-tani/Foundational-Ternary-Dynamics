# AUDIT — Clock-hypothesis substrate-derivation v1 closure attempt: UNDERDETERMINED

**Tag:** `[AUDIT FINDING — UNDERDETERMINED per pre-reg §6 Outcome B]`. v1 closure attempt is **incomplete** (not closed-negative): two §4-admissible routes were declined; the §5.3 transition-rate identification has F-d/F-f firings; the right verdict at v1 scope is UNDERDETERMINED. A v2 pre-reg is required with sharpened admissibility + the two queued routes added.
**Date:** 2026-05-25 (Step 2+3 execution Phase B per Wilsonian-reframe plan v2 Strategic Decision)
**LEDGER row:** FTD-0208 (Arc B P2 closure verdict)
**Pre-registration:** [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`](../../03_derivations/foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md) — git tag `preregister-clock-hypothesis-derivation-v1`, commit `4c15ba1`, SHA256 `9feb9d57ee53709ca419a6d068ed183b4b1426186bdaf662fad84061438ee4a5`. Hash-lock verified 2026-05-25.
**Closure-attempt executor:** FTD lead session.
**Adversarial reviewer (per pre-reg §9 step 9):** independent `general-purpose` agent. Verdict: **FAIL → UNDERDETERMINED**. Reviewer verdict + reasoning cited verbatim §11 below.
**Honest framing:** the F9 risk register's HIGHEST-risk arc behaved as predicted — the executor under-claimed (CLOSED-NEGATIVE looked epistemically humble but was actually incorrect at the operational level given untested routes + falsifier firings the executor missed). The adversarial review checkpoint caught this. **The framework works.** This audit document records the correct UNDERDETERMINED verdict + queues v2 pre-reg work.
**Companion docs:**
- [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §2 — identified the clock-hypothesis gap
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.3, §3.7, §4.3 — Born-Infeld action, bandwidth constraint, proper-time identification
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../../03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) §1.4 — original POSTULATE 2 framing
- [`AUDIT_FINITE_NEUTRAL_LOCK.md`](../08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md) — format precedent for UNDERDETERMINED result-docs

---

## §0 — Executive summary (UNDERDETERMINED verdict)

**The v1 closure attempt is INCOMPLETE, not closed-negative** per pre-reg §6 Outcome B. The executor's substrate-derivation via the SPEC §3.7 bandwidth-constraint route was initially proposed as Outcome C (CLOSED-NEGATIVE) on the grounds that the identification of the substrate-clock rate with the GR `dτ/dt` is irreducibly interpretive. **Independent adversarial review (general-purpose agent) identified that this verdict is UNDER-CLAIMED for two structural reasons:**

1. **§5.3 has F-d + F-f firings** that the executor missed. The identification `Ω_clock ~ |ℒ_matter|/S_trans` rests on an asserted "per-transition action quantum `S_trans ~ K_B · (constant)`" without per-voxel per-tick operational form (F-d fires: vague invocation) and without citing which §4 primitive justifies the transition-rate-equals-Lagrangian-over-S_trans identification (F-f fires: no §4 primitive cited).
2. **Two §4-admissible routes were declined, not exhausted.** Per pre-reg §6, CLOSED-NEGATIVE requires *demonstration that no derivation chain from §4 primitives produces dτ/dt = √(f - v²/f) without F-falsifier firing*. The executor's own §13 honest-limits acknowledges the calibration-declaration route (FTD-0041, T_U ≡ √3·ℓ_P/c — §4 catalog item 4) was "not pursued as a separate route" — that is *declared non-pursuit*, not exhaustion. Additionally, the SPEC §3.7 bandwidth-internal-time framing ("v and ℒ draw from same bandwidth budget" — a substrate-internal definition of proper time as the fraction of per-tick bandwidth budget spent on internal-clock transitions) was never enumerated in §7.2 at all.

**Verdict per pre-reg §6: Outcome B (UNDERDETERMINED)** — derivation chain reaches the substrate-clock rate `Ω_clock/Ω_0 = √(f-v²/f)` but with F-d/F-f firings at §5.3; two §4-admissible routes remain unattempted; the right verdict at v1 scope is UNDERDETERMINED with named candidate routes queued for v2 pre-reg.

**Tag consequences (Outcome B, NOT Outcome C):**
- SPEC §4.3 + §8 L-1 retain `[THEOREM modulo clock hypothesis]` — the clock hypothesis is NOT promoted to explicit `[AXIOM]` per v1; v2 attempts are queued first.
- LEDGER FTD-0208 created with tag `[UNDERDETERMINED, v1 incomplete; v2 with calibration-declaration + bandwidth-internal-time routes queued]`.
- Plan v2 Arc B P5 marked **PARTIAL**, NOT CLOSED.
- Arc C2 boundary theorem (FTD-0209, Outcome A FOUND) inherits D7 Branch C (dual-branch statement) since Arc B P2 verdict remains pending substantive verdict.
- v2 pre-reg `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` queued as follow-up work.

---

## §1 — Purpose

Record the v1 closure-attempt verdict per pre-reg §6. Document the adversarial-review FAIL → UNDERDETERMINED finding. Queue v2 pre-reg with the two named candidate routes + sharpened §5.3 admissibility.

---

## §2 — Substrate primitives stated operationally (pre-reg §9 step 1)

Per pre-reg D2 and §4 frozen catalog — content as in the original draft (no change required at this level):

| Primitive | Operational form | Source |
|---|---|---|
| **P1** SPEC §3.7 bandwidth constraint | `v < f` with `v = |Δ_tJ(v,t)|/K_B` and `f = 1 - ℒ(v)²` per voxel per tick | SPEC §3.7 [derived from Born-Infeld] |
| **P2** Born-Infeld action measure | `S = -K_B ∫√((f²-v²)/f) dt` — Lagrangian density of matter sector | SPEC §3.3 [THEOREM] |
| **P3** Substrate manifestation rate | Rate at which threshold-crossing events occur per universal tick `T_U` | Engine implementation (per phase_write) |
| **P4** Engine tick `T_U` | Universal discrete tick; calibrated `T_U ≡ √3·ℓ_P/c` per FTD-0041 | SPEC_FTD.md axiom 2 + FTD-0041 calibration |
| **P5** D4 substrate clock | Counting process on manifested-site transitions | Per pre-reg D4 |

**Critical note (adversarial review finding):** P4 (calibration declaration) is §4-catalog item 4 — admissible as a derivation primitive. The v1 closure attempt declined to pursue this as a route (§13 honest limits); v2 pre-reg should explicitly attempt this route.

---

## §3 — D4 substrate clock definition (pre-reg §9 step 2)

Per pre-reg D4: `Ω_clock(v, t) := ⟨ΔN_clock(v, t)/ΔT_U⟩` — expected manifested-site transition count per universal tick at the voxel's local (v, ℒ) configuration. Substrate-level counting process; not yet identified with proper time (the closure question).

---

## §4 — D5 bandwidth constraint operational form (pre-reg §9 step 3)

Per pre-reg D5 + SPEC §3.7: per universal tick `T_U`, a voxel can update its J by at most `K_B·f`. The bandwidth constraint is a *budget* constraint on per-voxel field updates.

**Critical note (adversarial review finding):** SPEC §3.7's "v and ℒ draw from same bandwidth budget" framing suggests a substrate-internal definition of proper time that the v1 §7.2 catalog did not enumerate: **proper time = the time experienced by a voxel whose `v < f` budget is fully spent on internal-clock transitions rather than spatial motion.** This is a substrate-internal definition (not GR-imported) and may not fire F-a / F-e / F-b. Whether it fires other falsifiers OR closes the gap is **OPEN** at v1; v2 pre-reg should enumerate and attempt.

---

## §5 — Transition-rate scaling derivation (pre-reg §9 step 4) — F-d/F-f FIRING IDENTIFIED

Attempt to derive `Ω_clock(v, t)` as a function of `(v, ℒ)` from primitives P1-P5 using §4-catalog primitives only.

### §5.1 — Action measure as energy-per-tick

`ℒ_matter(v, t) = -K_B · √((f²-v²)/f)` per SPEC §3.3 [THEOREM]. This is honest extraction from §4 admissibility item 2. ✅

### §5.2 — Per-tick action quantum

At rest: `ℒ_matter|_{v=0} = -K_B · √f`. General: `ℒ_matter(v, ℒ) = -K_B · √(f - v²/f)`. Mathematical unpacking. ✅

### §5.3 — Transition rate identification [F-d + F-f FIRE per adversarial review]

The v1 draft asserted:

> "A substrate transition (manifestation, evaporation, sign-flip) costs a fixed action quantum `S_trans ~ K_B · (constant)`. The transition rate is therefore: `Ω_clock(v, t) ~ |ℒ_matter(v, ℒ)| / S_trans = √(f - v²/f) · (K_B / S_trans)`."

**Adversarial review finding (verbatim):**

> "**The physical identification `Ω_clock ~ |ℒ_matter|/S_trans` has no §4-catalog warrant.** The executor asserts 'a substrate transition costs a fixed action quantum `S_trans ~ K_B · (constant)`' without deriving this from FTD axioms or operational engine behaviour. No §4 primitive (P1–P5) specifies that the manifestation/evaporation/sign-flip rate equals the Lagrangian magnitude divided by an action quantum. Per **F-f**, every step 'must cite which §4 primitive it uses.' §5.3 cites no primitive for this identification — it is asserted on a dimensional plausibility argument.
>
> **F-d violation.** Per pre-reg §7 F-d: 'vague invocations like "the bandwidth constraint implies" without site-level operational unpacking fire F-d.' The phrase 'a substrate transition costs a fixed action quantum `S_trans ~ K_B · (constant)`' is precisely such a vague invocation — `S_trans` is not given a per-voxel per-tick operational form, no engine-implementation reference, no derivation from §4 catalog. The `~` (asymptotic) and 'constant' placeholder are exactly the unpacking-failure F-d catches. **F-d should fire.**
>
> **The 'result' `Ω_clock/Ω_0 = √(f-v²/f)` is a definitional shadow of §5.1, not an independent derivation.** Since `Ω_clock` was *defined* as `|ℒ_matter|/S_trans` and `ℒ_matter` already contains `√((f²-v²)/f)`, getting `√(f-v²/f)` out is algebra, not physics."

**Honest acknowledgment:** the v1 §5.3 identification "transition rate equals Lagrangian magnitude divided by per-transition action quantum" is an unstated assumption tagged `[UNDERIVED IN v1 CATALOG]`. Whether this principle is substrate-derivable from §4 primitives is itself an open sub-question for v2.

**F-d and F-f fire at §5.3.** The v1 derivation chain is incomplete at this step; the executor's 0/10 falsifier claim was incorrect.

---

## §6 — Mapping to tick-rate scaling (pre-reg §9 step 5)

Given §5.3's F-d/F-f firings, the mapping to tick-rate scaling does not proceed cleanly from §4 primitives. The chain stops at the §5.3 unstated-assumption step.

---

## §7 — Check reduction to √(f - v²/f) — §7.2 CATALOG IS NON-EXHAUSTIVE per adversarial review

### §7.1 — The reduction at limits (mathematical, conditional on §5.3 assumption)

IF the §5.3 unstated assumption is accepted (transition rate = Lagrangian magnitude / S_trans), then the substrate-clock rate is `Ω_clock/Ω_0 = √(f - v²/f)`, which matches SPEC §4.3 form at limits:
- `v = 0`: `√f` ✓ (gravitational time dilation form)
- `ℒ = 0`: `√(1-v²)` ✓ (SR form)
- General: `√(f - v²/f)` ✓ (Schwarzschild proper time form)

**But this is conditional on the §5.3 unstated assumption** (per §5.3 acknowledgment); without that assumption, the reduction does not hold from §4 primitives alone.

### §7.2 — Catalog of candidate justifications for `Ω_clock/Ω_0 ↔ dτ/dt` (EXPANDED per adversarial review)

The v1 draft listed candidates (a)-(e) all firing F-a/F-e/F-b. **Adversarial review identified two missing candidates (c)+(d) below that should have been enumerated:**

| Route | Status | F-rule status |
|---|---|---|
| (a) "Substrate clock measures proper time because clocks do" | ❌ F-a fires (ideal-clock postulate) | Excluded |
| (b) Reparametrization-invariance argument | ❌ F-e fires (standard relativistic-particle theory) | Excluded |
| (c) **CALIBRATION-DECLARATION route via FTD-0041** | ⚠️ **NOT ATTEMPTED at v1** | OPEN — admissible via §4 catalog item 4 (engine tick T_U ≡ √3·ℓ_P/c is in catalog); whether the calibration provides an independent operational definition of proper time without invoking GR clock postulate is UNTESTED at v1. **v2 pre-reg should attempt.** |
| (d) **BANDWIDTH-INTERNAL-TIME route via SPEC §3.7** | ⚠️ **NOT ENUMERATED at v1** | OPEN — proper time = fraction of per-tick bandwidth budget spent on internal-clock transitions rather than spatial motion. This is substrate-internal (not GR-imported); whether it fires F-rules or closes the gap is UNTESTED. **v2 pre-reg should attempt.** |
| (e) "Substrate-clock rate IS dτ/dt because both have same functional form" | ❌ F-b fires (target-formula matching as justification) | Excluded |

**Adversarial review verdict on §7.2:** the v1 catalog was **non-exhaustive**. The CLOSED-NEGATIVE verdict requires demonstration of exhaustion (per pre-reg §6 Outcome C wording: "No derivation chain from §4 primitives produces dτ/dt = √(f - v²/f) without F-falsifier firing"). Declared non-pursuit of routes (c) and (d) is NOT exhaustion. The honest verdict is UNDERDETERMINED with routes (c) and (d) queued for v2 attempt.

---

## §8 — F-a..F-j falsifier checklist (CORRECTED per adversarial review)

Per pre-reg §7. v1 draft self-claim was 0/10 fire. **Independently verified by adversarial reviewer: 2/10 fire (F-d, F-f at §5.3).**

| Rule | Description | v1 self-claim | Adversarial verdict | Justification |
|---|---|---|---|---|
| F-a | No ideal-clock postulate import | PASS | ✅ PASS | Honest; §7.2 (a) correctly identified as F-a firing |
| F-b | No insertion of target formula | PASS | ⚠️ PASS-WITH-CONCERN | `Ω_clock` derivation rests on §5.3 identification which is itself the GR-Lagrangian structure reflected back; not strictly insertion but adjacent |
| F-c | No fitted constants | PASS | ✅ PASS | `Ω_0` defined structurally |
| **F-d** | **Operational bandwidth-constraint unpacking** | PASS | ❌ **FIRES** | **§5.3's `S_trans ~ K_B · (constant)` is vague invocation without per-voxel per-tick operational form; precisely the unpacking-failure F-d catches** |
| F-e | No "standard relativistic-particle theory" appeals | PASS | ✅ PASS | Honest; §7.2 (b) correctly identified as F-e firing |
| **F-f** | **Each step cites §4 primitive** | PASS | ❌ **FIRES** | **§5.3 does not cite which §4 primitive justifies "transition rate = Lagrangian magnitude / S_trans." P3 (substrate manifestation rate) is named but is itself the quantity being characterized, not a derivation source for the characterization.** |
| F-g | No Born-Infeld with hidden GR import | PASS | ⚠️ PASS-WITH-CONCERN | §5 uses Born-Infeld; F-d/F-f firings at §5.3 are gap, not smuggle |
| F-h | No Schwarzschild comparison before §10 | PASS | ✅ PASS | §7.1 is mathematical limit check, not numerical comparison |
| F-i | No look-elsewhere | PASS | ✅ PASS | Single mechanism; no mid-derivation switch |
| F-j | No SPEC §4.3 as scaffold | PASS | ✅ PASS | §5 derives independently |

**Corrected falsifier summary: 2/10 fire (F-d, F-f).** Per pre-reg §7: "Any single firing → at most Outcome B or Outcome C; not Outcome A." Adversarial-review verdict: **Outcome B (UNDERDETERMINED)** given the F-rule firings AND the unexhausted §4-admissible routes (c), (d).

---

## §9 — B-1..B-8 banned-moves checklist

All 8 banned moves: PASS per both executor self-check and adversarial review. Executor's B-checklist accurate.

---

## §10 — Adversarial review verdict (pre-reg §9 step 9) — FAIL → UNDERDETERMINED

**Adversarial reviewer:** independent `general-purpose` agent dispatched 2026-05-25 with: pre-reg path, AUDIT §2 reconciliation context, SPEC §3.3/§3.7/§4.3 context, draft result-doc path, F-/B-checklist verification mandate including specific attention to over-claim and under-claim risks.

**Reviewer's verdict (verbatim):** **FAIL → verdict should be UNDERDETERMINED instead**

**Reviewer's reasoning (excerpted verbatim from agent output):**

> "This is an **UNDER-CLAIM masquerading as honesty**. The executor has produced a result-doc that looks epistemically humble (CLOSED-NEGATIVE, 'we cannot derive'), but the humility is doing double duty: it (a) avoids confronting the F-d/F-f issues in §5.3 (the gap appears earlier than §7 claims), and (b) declines to attempt §4-admissible routes (calibration-declaration, bandwidth-internal time) that the §13 honest-limits section itself acknowledges. The CLOSED-NEGATIVE verdict is *easier* than the rigorous UNDERDETERMINED verdict that the pre-reg's three-outcome scheme actually supports.
>
> The executor is right that the ideal-clock / reparametrization-invariance / target-matching routes fire falsifiers. The executor is wrong that this exhausts the §4 catalog. The honest verdict, given that (i) the §5.3 derivation has F-d/F-f issues, (ii) the calibration-declaration route was admitted but not attempted, and (iii) the bandwidth-internal-time route was not enumerated, is **UNDERDETERMINED with named candidate routes for v2**.
>
> ...The over-claim risk (executor smuggled GR into §5 and called it derivation) is real but the executor's F-checklist handling of ideal-clock / reparametrization candidates in §7.2 is honest. The F-d/F-f issue at §5.3 is a *gap*, not a *smuggle* — the executor did not realize they needed to derive the transition-rate-equals-Lagrangian-over-S_trans identification from §4 primitives, and quietly bridged it dimensionally. That is a v2-pre-reg item, not a CLOSED-NEGATIVE.
>
> ...This is exactly the kind of 'looks closed-negative because the work is hard' pattern the pre-reg §10 risk register HIGHEST F9 risk was calibrated against. The reviewer's job is to push back on it, and I am pushing back."

**Reviewer's 5 caveats (all incorporated in this finalized AUDIT doc per pre-reg §6 Outcome B):**

1. **§5.3 honest re-statement** ✅ — addressed in §5.3 above with `[UNDERIVED IN v1 CATALOG]` tag and F-d/F-f firing acknowledgment
2. **§7.2 catalog expansion** ✅ — addressed in §7.2 above with routes (c) calibration-declaration and (d) bandwidth-internal-time added as UNTESTED
3. **Tag consequences updated to UNDERDETERMINED, not CLOSED-NEGATIVE** ✅ — SPEC §4.3 + §8 L-1 retain `[THEOREM modulo clock hypothesis]`; clock hypothesis NOT promoted to explicit `[AXIOM]`; LEDGER FTD-0208 tagged `[UNDERDETERMINED]` per §0 + §12 below
4. **v2 pre-reg scope-out** ✅ — queued in §13 below as follow-up work; will be authored as `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` per the reviewer's recommended scope
5. **Honest mid-paragraph acknowledgment in §0** ✅ — §0 leads with "The v1 closure attempt is INCOMPLETE, not closed-negative"

---

## §11 — Numerical comparison (pre-reg §9 step 10) — NOT REACHED

Per pre-reg §9 step 10: "only after steps 1–9 close cleanly." Since the closure attempt's adversarial-reviewed verdict is UNDERDETERMINED, the derivation chain does NOT close at v1 scope. Numerical comparison is not reached. v2 pre-reg + closure attempt is the path to numerical comparison.

---

## §12 — Verdict assignment per §6 (pre-reg §9 step 11) — Outcome B UNDERDETERMINED

**§6 Outcome B (UNDERDETERMINED):** the derivation chain reaches the substrate-clock rate `Ω_clock/Ω_0 = √(f-v²/f)` conditionally (modulo the §5.3 unstated assumption that itself fires F-d/F-f at v1 scope); two §4-admissible routes (calibration-declaration via FTD-0041; bandwidth-internal-time via SPEC §3.7) remain unattempted. A v2 pre-reg with sharpened §5.3 admissibility + the two queued routes added is required.

**Tag consequences per pre-reg §6 Outcome B:**
- **SPEC §4.3 + §8 L-1 retain** `[THEOREM modulo clock hypothesis]` — clock hypothesis NOT promoted to explicit `[AXIOM]` per v1 (that would be Outcome C consequence); status unchanged pending v2.
- **LEDGER FTD-0208 row created** with tag `[UNDERDETERMINED, v1 incomplete; v2 pre-reg queued with calibration-declaration + bandwidth-internal-time routes added + §5.3 sharpened admissibility]`.
- **Plan v2 Arc B P5 marked PARTIAL**, NOT CLOSED.
- **LEDGER FTD-0131 row** unchanged — the substrate-derivation chain still rests on the clock hypothesis as "1 flagged interpretive step" per the 2026-05-24 reconciliation; v1 closure attempt did not change this status.
- **Arc C2 boundary theorem (FTD-0209, FOUND)** inherits Branch C (dual-branch statement for the scalar sector) per pre-reg D7, since Arc B P2 verdict is now formally PARTIAL/UNDERDETERMINED.
- **`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §2.4** updated to record v1 closure attempt UNDERDETERMINED; the "1 flagged interpretive step" status stands pending v2.

---

## §13 — Honest limits + v2 pre-reg scope-out (queued follow-up)

- **The v1 closure attempt is INCOMPLETE,** per adversarial-review FAIL → UNDERDETERMINED. This audit document honestly records this and queues v2 work.
- **v2 pre-reg scope-out** (queued for follow-up session — separate work, not part of this audit's deliverable):
  - **Add to §4 admissibility:** explicit catalog inclusion of route (c) calibration-declaration via FTD-0041 calibration `T_U ≡ √3·ℓ_P/c` as a derivation primitive, AND route (d) bandwidth-internal-time via SPEC §3.7 "v and ℒ draw from same bandwidth budget" framing.
  - **Sharpen F-d:** add explicit catch for the `S_trans ~ K_B · (constant)` placeholder pattern that fired F-d at v1. Specifically: any per-transition action quantum used in transition-rate-equals-Lagrangian-over-quantum identification must be derived from §4 primitives operationally per voxel per tick (no asymptotic `~` or "constant" placeholders).
  - **§5.3 derivation requirement:** the transition-rate identification must be derived from §4 primitives, not asserted dimensionally. If the §4 catalog (including the v2 additions for routes (c) and (d)) does not support this derivation, the v2 closure attempt may land CLOSED-NEGATIVE legitimately at that scope.
  - **Outcomes (per pre-reg §6):** FOUND (clock hypothesis substrate-derived via one of the new routes); UNDERDETERMINED (closure requires additional principle beyond v2 catalog); CLOSED-NEGATIVE (v2 catalog with routes (c)+(d) added genuinely cannot support the identification — only then is the irreducible-axiom verdict honest).
- **F9 risk acknowledged + handled:** the v1 closure attempt over-confidently declared CLOSED-NEGATIVE based on declared non-pursuit rather than demonstrated exhaustion. The adversarial review caught this. The pre-registration discipline + adversarial review checkpoint WORKED. This UNDERDETERMINED verdict is the honest outcome at v1 scope.
- **F10 risk acknowledged:** the UNDERDETERMINED tag is recognition that the v1 closure attempt is incomplete — NOT a fix to the substrate-derivation question more broadly. The clock hypothesis remains an open interpretive step; whether it can be substrate-derived (via routes (c) or (d) or some other v2 mechanism) or must be tagged honestly as `[AXIOM]` is the v2 closure question.

---

## §14 — Single-line summary

**The v1 closure attempt of `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md` (Arc B P2 of Wilsonian-reframe plan v2) is INCOMPLETE per pre-reg §6 Outcome B (UNDERDETERMINED): the executor's CLOSED-NEGATIVE provisional verdict was caught by adversarial review (independent `general-purpose` agent, verdict FAIL → UNDERDETERMINED) as under-claiming due to (a) F-d + F-f firings at §5.3's `S_trans ~ K_B · (constant)` vague invocation that lacks per-voxel per-tick operational form and §4-primitive citation, and (b) two §4-admissible routes — calibration-declaration via FTD-0041 (catalog item 4) and bandwidth-internal-time via SPEC §3.7 "v and ℒ draw from same bandwidth budget" — declined or never enumerated at v1; CLOSED-NEGATIVE requires demonstrated exhaustion (per §6), not declared non-pursuit, so the honest v1 verdict is UNDERDETERMINED with the two routes named for v2 pre-reg work and the §5.3 admissibility sharpened to catch placeholder patterns; SPEC §4.3 + §8 L-1 retain `[THEOREM modulo clock hypothesis]` (clock hypothesis NOT promoted to explicit `[AXIOM]` per v1); LEDGER FTD-0208 tagged `[UNDERDETERMINED, v1 incomplete; v2 queued]`; Plan v2 Arc B P5 marked PARTIAL (not CLOSED); Arc C2 boundary theorem FTD-0209 inherits Branch C dual-branch statement per D7 pending Arc B P2 v2 verdict; the pre-registration discipline + adversarial review checkpoint WORKED — this UNDERDETERMINED finding is the framework operating as designed, catching the F9 "under-claim masquerading as humility" pattern the §10 risk register HIGHEST F9 risk was calibrated against; v2 pre-reg `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md` queued as separate follow-up work.**
