# SCOPE — Arc B / Newton-postulates reconciliation: SPEC_FTD_LAGRANGIAN.md [THEOREM] vs DERIV_NEWTON_FROM_SUBSTRATE.md [POSTULATE]

**Tag:** `[SCOPING MEMO]` — not a closure, not a tag promotion, not a new derivation. Records a cross-doc tag-reconciliation question that materially affects what Arc B (Newton-postulate substrate derivation) needs to attempt.
**LEDGER row reservation:** to be confirmed against `../07_assessment/core_ledgers/LEDGER.md` at hash-lock; provisional placeholder pending audit.
**Date:** 2026-05-24
**Plan:** `~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md` v2 (Wilsonian reframe) — Arc B P2 P0 deliverable.
**Sources read:**
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](DERIV_NEWTON_FROM_SUBSTRATE.md) (2026-05-03, FTD-0131) — flags Postulate 1 (§1.2) + Postulate 2 (§1.4)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) (v3.2, 2026-03-16) — §4.2 [THEOREM] Poisson; §4.3 [THEOREM] Born-Infeld → Schwarzschild proper time; §8 claims table L-1 through L-11
- [`../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`](../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) §14.1, §14.2 — `[THEOREM]` tick-rate-variation
- [`../01_reference/SPEC_DOCTRINE_LEDGER.md`](../01_reference/SPEC_DOCTRINE_LEDGER.md) §13.5, §14 — Phase-2 priority 6 (gravity beyond Newtonian); FTD-0189 Step-0 Deser correction context

> **What this memo is NOT.** Not Arc B P1 (theory attempt). Not a pre-registration. Not a tag move. It records the reconciliation question so Arc B P1 starts from the correct gap rather than a premature one.

---

## §1 — The reconciliation question

`DERIV_NEWTON_FROM_SUBSTRATE.md` (2026-05-03) flags two postulates:

| Tag in DERIV_NEWTON | Statement | Doc section |
|---|---|---|
| `[POSTULATE 1, flagged]` | `ρ_g(x) = K_B^grav · 𝟙_manifested(x)` | §1.2 |
| `[POSTULATE 2, flagged]` | `dτ/dT_U(x) = 1 + 2φ_g(x)/c²` (linearized tick-rate) | §1.4 |

`SPEC_FTD_LAGRANGIAN.md` (v3.2, 2026-03-16) appears to tag the same substrate-physics content as `[THEOREM]`:

| Tag in SPEC_FTD_LAGRANGIAN | Statement | Doc section |
|---|---|---|
| **L-7 `[THEOREM]`** | Poisson `∇²ℒ = 4πGρ` follows from variation of S w.r.t. ℒ; matter contribution gives `ρ_mass = K_B · n` (number density of manifested sites) in weak field | §4.2 |
| **L-1 `[THEOREM]`** | Born-Infeld core exactly reproduces Schwarzschild proper time `dτ/dt = √((f²-v²)/f)` for all `f ∈ (0,1]` | §4.3 |
| **L-7, gravity table row** | "Poisson eq ∇²ℒ = 4πGρ" — Variation of S w.r.t. ℒ | §6.2 |
| **§6.2 gravity table** | "Exact Schwarzschild — Born-Infeld core (§4.3) — DERIV_LATTICE_SCHWARZSCHILD" | §6.2 |

**Two possible readings:**

- **Reading A — tag mismatch / DERIV_NEWTON outdated.** The substrate derivations exist in SPEC_FTD_LAGRANGIAN.md §4.2-§4.3; DERIV_NEWTON's `[POSTULATE]` flags are residue from before those THEOREMs were established (or from a deliberate choice to flag conditional dependencies that the SPEC doc treats as accepted). If correct: Arc B is mostly closed already; the work is doc-reconciliation + propagating the THEOREM tags into DERIV_NEWTON's §4 honest-tagging table + LEDGER FTD-0131 row update.
- **Reading B — substantive distinction.** The SPEC_FTD_LAGRANGIAN.md `[THEOREM]`s are conditional on assumptions DERIV_NEWTON is correctly flagging at the postulate level (clock hypothesis for L-1; `K_B^grav = K_B` identification for L-7). If correct: Arc B's real target narrows to those underlying assumptions, not the derivation chain itself.

Both readings make Arc B substantively smaller than v1 of the plan implied. The reconciliation must land before any Arc B P3 pre-registration locks.

---

## §2 — The two specific reconciliation sub-questions

### §2.1 — Postulate 1 (source coupling): is `K_B^grav = K_B`?

DERIV_NEWTON §1.2 writes `ρ_g = K_B^grav · 𝟙_manifested` with `K_B^grav` (superscript-grav) as "a coupling constant to be determined".

SPEC_FTD_LAGRANGIAN.md §4.2 derives `ρ_mass = K_B · n` in the weak-field limit, with `K_B` from §3.4 = manifestation threshold = `M_P√(2π)(16/3)α¹¹` = `m_e` = 0.511 MeV `[THEOREM]`.

**Sub-question.** Is `K_B^grav` distinct from the manifestation-threshold `K_B`, or are they the same constant?

- **If same:** Postulate 1 is closed by SPEC §4.2 [THEOREM]; DERIV_NEWTON's superscript is misleading and `[POSTULATE 1]` flag is residue.
- **If distinct:** there is a genuine open question about why `K_B^grav` has its specific value relative to `K_B`. Engine evidence (FTD-0131's α_G(e,e) = 0.38% match using `K_B = m_e`) suggests they ARE the same — otherwise FTD-0131's numerical match would be a coincidence rather than a derivation.

**Provisional reading.** Same constant. DERIV_NEWTON's superscript is bookkeeping for "the coupling constant entering the gravity term", not a claim that it's a different number from K_B. Arc B P1 P1 then reduces to: verify that the §4.2 THEOREM-derived coupling form is identical to DERIV_NEWTON's flagged Postulate 1, and propagate the [THEOREM] tag into DERIV_NEWTON §4 + LEDGER FTD-0131.

### §2.2 — Postulate 2 (linearized tick-rate response): is the clock hypothesis substrate-derived?

DERIV_NEWTON §1.4 writes `dτ/dT_U = 1 + 2φ_g/c²` as `[POSTULATE 2]`, with the note "FTD postulates it to match GR's linearization."

SPEC_FTD_LAGRANGIAN.md §4.3 [THEOREM]: action `S = -K_B ∫√((f²-v²)/f) dt`. **"By the clock hypothesis, dτ ∝ √((f²-v²)/f) dt"** → `dτ/dt = √(f - v²/f)`. With `f = 1 - r_s/r`, this is exactly Schwarzschild proper time for all f. §5.2 specializes to v=0: `dτ/dt = √f`, giving gravitational time dilation.

Linearizing §4.3's result: `√f = √(1 - r_s/r) ≈ 1 - r_s/(2r) = 1 - GM/(rc²) = 1 + φ_g/c²` (with `φ_g = -GM/r`). **Coefficient is `1`, not `2`.**

DERIV_NEWTON §1.4 writes `1 + 2φ_g/c²`. This **either**:
- (a) is a transcription that conflates `g_00 = -(1 + 2Φ/c²)` (the metric component) with `dτ/dt = √(-g_00) ≈ 1 + Φ/c²` (the proper-time ratio), in which case `[POSTULATE 2]`'s coefficient is wrong by a factor of √, OR
- (b) is using a convention where "dτ/dT_U" means `(dτ/dt)²` or `g_00` directly, in which case the coefficient `2` is correct under that convention, OR
- (c) is an honest acknowledgement that even though §4.3 [THEOREM] gives `√f` for the full form, the LINEARIZED relationship between `φ_g` and the tick-rate response is what's postulated, and the full-form theorem doesn't automatically license the linearized form.

Most likely (a) or (b) — convention/transcription artifact. The honest reading: **§4.3's [THEOREM] subsumes the substrate-physics content of DERIV_NEWTON §1.4's [POSTULATE 2]**.

**Sub-question.** Is the clock hypothesis (substrate ticks correspond to proper time via the Born-Infeld action measure) substrate-derived, or is it an interpretive axiom?

- If substrate-derived: §4.3 [THEOREM] fully closes Postulate 2.
- If interpretive axiom: §4.3 [THEOREM] is conditional on the clock hypothesis; Postulate 2 is closed modulo this axiom.

**Provisional reading.** The clock hypothesis is an interpretive step — it asserts that the Born-Infeld action measure IS proper time, rather than just being a Lagrangian. This is a standard step in relativistic-particle theory (the proper-time parameter is what makes the action reparametrization-invariant). In FTD it can be either (i) **derived** if substrate manifestation rate scales with the Born-Infeld core (which has independent physical content via the bandwidth constraint v < f), or (ii) **axiomatic** if treated as definitional.

Arc B P2's genuine target then becomes: substrate-derive (or honestly tier-flag) the clock hypothesis. This is a much narrower target than "derive the coefficient 2/c²".

---

## §3 — The FTD-0189 Step-0 Deser audit context

`SPEC_DOCTRINE_LEDGER.md` §14 priority 6 cites FTD-0184 (branch-compliance/Yilmaz route [CLOSED NEGATIVE]) and FTD-0131 (partial closure with 2 postulates flagged). The 2026-05-21 FTD-0189 Step-0 audit retagged some related claims in `DERIV_RELATIVITY_DERIVATION.md`:

- Theorem 14.1 "linearized Einstein [THEOREM]" → `[SELECTION/CONDITIONAL]` (rests on hJ correspondence)
- Theorem 15.3 "graviton = transverse flux modes [THEOREM]" → `[CONJECTURE] + spin-count flag` (spin-content error: 2 transverse J-modes = helicity-±1, not ±2)

**Does FTD-0189's audit apply to SPEC_FTD_LAGRANGIAN.md §4.2 (Poisson) or §4.3 (Born-Infeld → Schwarzschild)?**

- §4.2 (Poisson from action variation): does not invoke `h_μν` correspondence; the derivation runs through `ℒ` (the latency scalar) variation, not metric-perturbation theory. **Provisional reading: FTD-0189 audit does NOT bear on §4.2.** Confirmation requires reading FTD-0189 directly + the audit's scope.
- §4.3 (Born-Infeld → Schwarzschild proper time): does not invoke `h_μν` correspondence either; the derivation runs through Born-Infeld action + clock hypothesis. **Provisional reading: FTD-0189 audit does NOT bear on §4.3.** Same confirmation needed.
- §6.2 table row "Nonlinear completion: G_μν = 8πG·T_μν/c⁴ — Lovelock's theorem [1] — DERIV_EINSTEIN_FIELD_EQUATIONS §5": **this MAY be subject to FTD-0189 audit** since the Lovelock-based extension to full Einstein equations is precisely what FTD-0189 §10.1 audited.

**Net.** §4.2 + §4.3 should survive the FTD-0189 audit unchanged. §6.2's nonlinear-completion row may already be retagged. Arc C2 (boundary theorem) handles the upper end; Arc B P0's reconciliation only needs §4.2 + §4.3.

---

## §4 — Implications for the Arc B plan

If §1-§3 readings hold:

| Plan v2 Arc B item | Updated state under reconciliation |
|---|---|
| **Arc B P1 (Postulate 1 derivation)** | Substantively closed by SPEC §4.2 [THEOREM] under `K_B^grav = K_B` identification. Arc B P1 reduces to **tag-reconciliation** (~1-2 sessions) + LEDGER row update, not a multi-week derivation. |
| **Arc B P2 (Postulate 2 derivation)** | Substantively closed by SPEC §4.3 [THEOREM] under the clock hypothesis assumption. Arc B P2's genuine target narrows to: **substrate-derive the clock hypothesis** OR **honest-tier the clock hypothesis as an interpretive axiom**. This is a narrower, sharper target than "derive coefficient 2/c²". |
| **Arc B horizon** | Drops from 6-10 weeks to **2-4 weeks** (tag reconciliation ~1-2 weeks + clock-hypothesis attempt ~1-2 weeks). |
| **Critical path** | Reordered. Tag reconciliation is the new immediate work product. Clock hypothesis attempt comes after reconciliation lands. |
| **Arc D gap (iv)** ("Newton-from-substrate derivation test") | Probably **already passes** under the reconciliation reading — the engine's `render_bridge.cpp:709` Poisson implementation IS the §4.2 [THEOREM]. Verification reduces to: does the engine code implement the §4.2-derived equation, or a parallel boundary-condition imposition? Quick audit. |

If §1-§3 readings DO NOT hold (i.e., reconciliation reveals genuine substantive gap between SPEC and DERIV_NEWTON): Arc B P1/P2 stay at v2's full scope and horizon.

---

## §5 — Recommended Arc B P0 → P1 next steps

1. **Direct tag reconciliation desk session (~1 day):**
   - Read SPEC_FTD_LAGRANGIAN.md §4.2-§4.3 in full + DERIV_NEWTON_FROM_SUBSTRATE.md §1.2-§1.5 side-by-side
   - Read DERIV_EINSTEIN_FIELD_EQUATIONS.md to check L-7 / linearized-Einstein THEOREM provenance
   - Verify `K_B^grav = K_B` from engine code (`engine/include/ftd/ontic.h`) or from numerical match in `scripts/proofs/proof_newton_from_substrate.py`
   - Verify the clock hypothesis treatment — is it tagged anywhere, or implicit?
   - Determine which reading (A: tag mismatch / B: substantive distinction) holds for each postulate
2. **Reconciliation memo (~½ day desk):**
   - Publish `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` recording the verdict (Reading A or B per postulate; specific gaps identified)
   - Update DERIV_NEWTON_FROM_SUBSTRATE.md §4 honest-tagging table if needed
   - Update LEDGER FTD-0131 row if tag changes
3. **Branch into Arc B P1 / P2 based on reconciliation outcome:**
   - If Reading A on both: Arc B reduces to LEDGER housekeeping + clock-hypothesis treatment
   - If Reading B on either: pursue the narrow remaining gap per pre-registration discipline (PREREG_NEWTON_POSTULATE_*_v1.md)
4. **Defer Arc C2 P3 (boundary theorem pre-reg lock) until Arc B reconciliation lands** — Arc C2's framing of "substrate carries scalar + vector modes only; full GR enters as Deser-bootstrap extension of posited h_μν" depends on whether §4.2+§4.3 are correctly tagged as [THEOREM] for the scalar/Schwarzschild portion. If those THEOREMs hold, C2's structure is unchanged. If reconciliation downgrades them, C2 widens.

---

## §6 — What this memo does NOT claim

- **NOT a closure of Arc B.** This memo identifies the reconciliation question; the verdict (Reading A or B) must be established by Arc B P0 → P1's actual side-by-side read.
- **NOT a tag promotion.** DERIV_NEWTON_FROM_SUBSTRATE.md §1.2-§1.4 keeps `[POSTULATE 1, flagged]` and `[POSTULATE 2, flagged]` until the reconciliation memo lands a verdict.
- **NOT a finding that Arc B is trivial.** Even under Reading A, the clock hypothesis substrate-derivation is a real open question; even Reading A leaves the FTD-0015 prefactor (`√(2π)·(16/3)`) [OPEN], which is the gating step for upgrading α_G(e,e) from `[SMC]` to `[DERIVED]`.
- **NOT a rejection of FTD-0189's audit scope.** The provisional reading that §4.2+§4.3 survive FTD-0189 unchanged needs explicit confirmation by reading FTD-0189's audit doc directly.

---

## §7 — Honest limits of this scoping memo

- It rests on grep + spot-read of SPEC_FTD_LAGRANGIAN.md §4.2-§4.3 + §6.2 + §8 claims table, plus full read of DERIV_NEWTON_FROM_SUBSTRATE.md. It has NOT read DERIV_EINSTEIN_FIELD_EQUATIONS.md, DERIV_RELATIVITY_DERIVATION.md, or the FTD-0189 audit LEDGER row directly.
- The factor-of-2 issue in §2.2 (DERIV_NEWTON §1.4 coefficient vs §4.3 linearization) is identified as transcription-or-convention, but not formally diagnosed. A careful linearization audit is in the Arc B P0 P1 desk session above.
- The Wilsonian-reframe plan v2 (`~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md`) should be updated after Arc B P0 reconciliation lands, to reflect actual Arc B horizon and content.

---

## §8 — Single-line summary

**SPEC_FTD_LAGRANGIAN.md §4.2 + §4.3 contain `[THEOREM]`-tagged derivations of the substrate-physics content that DERIV_NEWTON_FROM_SUBSTRATE.md §1.2 + §1.4 flag as `[POSTULATE 1]` + `[POSTULATE 2]`; the two-doc tag mismatch is the load-bearing finding of Arc B P0; reconciliation reduces Arc B to ~2-4 weeks of tag-reconciliation desk work + clock-hypothesis treatment, rather than v2's 6-10 weeks of independent substrate-physics derivation.**
