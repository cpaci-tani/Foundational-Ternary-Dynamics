# FOUND — Spin-2 boundary theorem (free-theory + canonical-toggle scope)

**Tag:** `[THEOREM at free-theory + Gauss-only level for clauses (C2-1), (C2-2), (C2-3)]` + `[STRONGLY MOTIVATED CONJECTURE for canonical toggle set per FTD-0193 empirical floor]` + `[REFERENCE for (C2-4)]` (effective-theory matching via Deser-bootstrap of POSITED `h_μν` per FTD-0189 + AUDIT_NEWTON_POSTULATES_RECONCILIATION §3 + DERIV_EINSTEIN_FIELD_EQUATIONS [SELECTION/CONDITIONAL] retags 2026-05-24). **Outcome A (FOUND)** per pre-reg §6 outcome-A criteria.

**Date:** 2026-05-25 (Step 2+3 execution per Wilsonian-reframe plan v2 Strategic Decision)
**LEDGER row:** FTD-0209 (Arc C2 P4 closure verdict)
**Pre-registration:** [`PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`](../preregistrations/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md) — git tag `preregister-spin2-boundary-theorem-v1`, commit `d8e016b`, SHA256 `c6bd0e182d85cf9027c4a1d54d0c16b83724c6a2bbd12a3b0b8391b0036440db`. Hash-lock verified 2026-05-25 via `git rev-list -n1 preregister-spin2-boundary-theorem-v1`.
**Closure-attempt executor:** FTD lead session.
**Adversarial reviewer (per pre-reg §9 step 10):** independent `general-purpose` agent (no project priors). Verdict cited verbatim in §11 below.
**Companion docs (load-bearing proof scaffold):**
- [`DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md`](DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md) — 4-clause consolidated derivation (commit `d2ec208`)
- [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) — load-bearing C2-2 bubble-integral analysis (commit `d2ec208`)
- [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](../reports_and_audits/REPORT_GRAVITON_SUBSTRATE_MODE.md) — FTD-0193 [CLOSED NEGATIVE] empirical floor

---

## §0 — Executive summary

The four-clause spin-2 boundary theorem of `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` §1 holds at the locked dual-tag scope. The closure attempt walked pre-reg §9 method 11-step: 0/10 F-a..F-j falsifiers fire; 0/8 B-1..B-8 banned moves invoked. Independent `general-purpose` agent adversarial review returned **PASS-WITH-CAVEATS** (verdict cited §11). The 4 caveats — all documentation-level scope clarifications, none structural — are incorporated inline in §5, §6, §7, §13 below.

**Per pre-reg §1 honest framing:** this is **scope clarification** ("the boundary is mapped at theorem-grade rigor at the locked dual-tag scope"), not the substantive surprise of "we proved no graviton." The DERIV docs (commit `d2ec208`) authored 2026-05-24 already established the chain rigorously; this closure attempt verifies + finalizes + addresses adversarial-review caveats.

**Verdict per pre-reg §6: Outcome A (FOUND).**

---

## §1 — Purpose

Walk the pre-registered §9 method 11-step against the locked theorem statement (pre-reg D1 = `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` §1) to produce a verdict per §6 outcomes, with adversarial-review-confirmed mechanical F-/B-checklist results and adversarial-review-caveats incorporated in the final result-doc.

---

## §2 — Theorem statement (pre-reg §9 step 1, verbatim from D1)

Per pre-reg D1 = `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` §1 verbatim:

> **Spin-2 boundary theorem, free-theory + canonical-toggle scope.** Under FTD axioms 1-5, the calibration declarations, and the non-site-local observable algebra of `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4 frozen catalog, the FTD substrate's connected two-point correlator in the transverse-traceless rank-2 sector contains no gapless helicity-±2 pole. Specifically:
>
> **(C2-1) Linear spectrum.** 2 transverse spin-1 J modes per wavevector + 1 quasi-static scalar ℒ; no rank-2 propagating mode.
> **(C2-2) J-bilinear non-separability.** TT projection of `O_ij = J_iJ_j − ⅓δ_ij|J|²` correlator is bubble-integral continuum, no isolated pole.
> **(C2-3) No substrate-derived graviton in §4 catalog.** Composition of (C2-1) + (C2-2) + §5.1 uniqueness argument.
> **(C2-4) Full nonlinear GR matched, not derived.** h_μν posited per Conjecture 10.1 [FTD-0189]; Deser-bootstrap chain [SELECTION/CONDITIONAL] per `DERIV_EINSTEIN_FIELD_EQUATIONS.md` retags 2026-05-24.

---

## §3 — Level distinction (pre-reg §9 step 2)

Per pre-reg D2 + D3:
- **Free-theory + Gauss-only level** (D2): state field s ≡ 0; no state-flux coupling; no manifestation thresholds; no Langevin noise; Gauss constraint is the only interaction.
- **Canonical toggle set level** (D3): engine's default toggle configuration at the time of `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` hash-lock (commit `bb354b6`, 2026-05-22) — 11 toggles ON, `dual_substrate` + `weak_transmutation` OFF. Re-snapshotted via this pre-reg's D3 reference. This is the regime in which FTD-0193 measured.

**Dual-tag structure** preserved throughout the walk-through and the final result-doc: [THEOREM] for free-theory clauses; [SMC] for canonical-toggle extension with FTD-0193 empirical floor; [REFERENCE] for the Deser-bootstrap clause (C2-4).

---

## §4 — Walk-through (C2-1) clause (pre-reg §9 step 3)

Verify DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY §3 linear-spectrum derivation:

| Sub-step | Source | Status |
|---|---|---|
| 3.1 J-sector spectrum: 3 components per voxel; Gauss removes 1 longitudinal; 2 transverse spin-1 modes propagate at `ω(k) = C·ω_L(k)` | DERIV §3.1 [THEOREM] |  verified; standard lattice gauge theory; empirically confirmed by FTD-0193 §2 spin-1 control (12/12 k-points at 0.02-3% precision) |
| 3.2 ℒ-sector spectrum: latency satisfies static Poisson `∇²_L ℒ = 4πG ρ_mass`; no `Δ_t²` term; quasi-static; 1 mode per k (Green's function response only); no propagating spin-0 graviton | DERIV §3.2 [THEOREM] (cites SPEC §4.2 + FTD-0004 Phase G) |  verified; first-order-in-time constrained scalar; matches engine's `solve_latency_poisson_cpu` (gap (iv) CLOSED POSITIVE per AUDIT §3.5) |
| 3.3 Combined: 2 transverse spin-1 (propagating) + 1 quasi-static scalar = **no rank-2 propagating mode** in linear spectrum | DERIV §3.3 [THEOREM] |  verified by composition |
| 3.4 Canonical-toggle extension: interactions in canonical set (state-flux coupling, velocity coupling, thresholds, Langevin) do not introduce new fundamental field beyond {J, s, ℒ} | DERIV §3.4 [SMC] |  structural argument; empirically validated FTD-0193 spin-1 control unchanged under canonical toggles |

**Tag (C2-1):** [THEOREM] at free-theory + Gauss-only; [SMC] at canonical-toggle. 

---

## §5 — Walk-through (C2-2) clause (pre-reg §9 step 4)

Verify DERIV_J_BILINEAR_NO_SPIN2_POLE.md §3 bubble-integral analysis (load-bearing for the theorem):

| Sub-step | Source | Status |
|---|---|---|
| 3.1 Wick contraction `⟨O_ij O_kl⟩_c` factorizes into sum of `⟨JJ⟩⟨JJ⟩` products | DERIV_J_BILINEAR §3.1 [THEOREM] |  standard Wick's theorem on Gaussian J |
| 3.2 Momentum-space bubble integral `Π(k,ω) = ∫(d⁴p) G_J(p)·G_J(k−p)` with two J-propagator poles | DERIV_J_BILINEAR §3.2 [THEOREM] |  direct Fourier transform |
| 3.3 Bubble integral analytic structure: branch cut at `|ω|² ≥ 4C²ω_L²(k/2)` — **no isolated pole** | DERIV_J_BILINEAR §3.3 [THEOREM] |  standard QFT (Peskin-Schroeder §10.2 continuum; Montvay-Münster §3 lattice analog) |
| 3.4 TT projection `P^{TT}_{ijkl}` preserves branch-cut-only structure in TT-form factor `Π^{TT}(k,ω)` | DERIV_J_BILINEAR §3.4 [THEOREM at free-theory level] |  tensor algebra; no pole introduced by projection |
| Empirical validation: FTD-0193 §4 — flux-quadrupole TT ω identical to spin-1 control ω at 11/12 k-points to 7 sig figs at L=64 | REPORT_GRAVITON_SUBSTRATE_MODE.md §4 [VERIFIED] |  canonical-toggle empirical floor |
| §5.2 Canonical-toggle extension argument: interactions don't add new propagating modes | DERIV_J_BILINEAR §5 [SMC] |  structural argument |

**Caveat-2 (per adversarial review Gap-2, addressed here):** the continuum bubble-integral branch-cut conclusion (DERIV_J_BILINEAR §3.3 [THEOREM]) strictly applies in the `L → ∞` (continuum-spectral) limit. At finite L, the propagator's "pole" is a discrete set of frequencies, so the strict continuum branch cut becomes a *dense set of two-particle resonance peaks whose envelope is the continuum spectral function*. **FTD-0193's L=64 empirical confirmation IS the lattice test** — the engine measurement directly validated this transfer, with 11/12 k-points showing the spin-1-frequency-dominated bilinear response exactly per the bubble-integral prediction. The [THEOREM] vs [SMC] split is operationally honest: [THEOREM] holds at the free-theory continuum-spectral level; [SMC] at the canonical-toggle finite-L realization is what FTD-0193 measured.

**Tag (C2-2):** [THEOREM] at free-theory + Gauss-only continuum-spectral level; [SMC] at canonical-toggle finite-L per FTD-0193 empirical floor. 

---

## §6 — Walk-through (C2-3) clause (pre-reg §9 step 5)

**Load-bearing composition argument.** F-h falsifier critical: (C2-3) [THEOREM]-grade tag must rest on the structural argument, NOT on FTD-0193 empirics.

Verify DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY §5 composition:

| Premise | Source | Status |
|---|---|---|
| (i) Linear spectrum has no rank-2 propagating mode | (C2-1) §4 above [THEOREM] |  verified |
| (ii) Only candidate non-site-local rank-2 observables in §4 catalog are J-bilinears (flux-quadrupole `O_ij` and stress `Õ_ij`) — group-theoretic uniqueness | DERIV_J_BILINEAR §2.2 + §5.1 [THEOREM via Sym²(V) − Tr(V⊗V)/3 decomposition for V=3-vector rep] |  verified (with sub-case walk per Caveat-1 below) |
| (iii) J-bilinears' TT projection has no isolated pole | (C2-2) §5 above [THEOREM] |  verified |
| **Conclusion:** No substrate-derived emergent graviton exists in §4 catalog at free-theory + Gauss-only level | Composition (i) ∧ (ii) ∧ (iii) [THEOREM] |  verified by logical composition |

**Caveat-1 (per adversarial review Gap-1, addressed here) — §5.1 uniqueness sub-case walk.** DERIV_J_BILINEAR §2.2 asserts "from a 3-vector J-field without introducing new fundamental fields, the only local rank-2 observables built without higher-derivative or non-local operators are bilinears in J or its derivatives." This excludes three candidate sub-cases that an adversarial reader could pick at:

- **(a) Higher J-multilinears constructed as rank-2** (e.g., `J_iJ_j|J|²`, `(J_iJ_j)(J·J)`): such constructions are reducible to bilinear-times-scalar combinations; at lowest spectral order the TT projection of a higher-multilinear gives the same branch-cut bubble structure as the bilinear, multiplied by a positive scalar form factor that cannot introduce an isolated pole (no new propagating mode). Resolution: subsumed by the bilinear analysis at lowest order.
- **(b) ℒ-derivative bilinears `(∂_iℒ)(∂_jℒ)`** symmetrized-traceless: per (C2-1) §3.2, ℒ is quasi-static (no `Δ_t²` term; first-order constrained scalar). ℒ-bilinears therefore inherit ℒ's non-propagating character — they cannot host a *propagating* rank-2 pole (propagation requires a `Δ_t²` mode equation, which ℒ does not have). Resolution: no new propagating spin-2 pole possible from ℒ-bilinears.
- **(c) J-ℒ cross-bilinears `J_i(∂_jℒ)`** symmetrized-traceless: these mix the J spin-1 propagating sector with the ℒ quasi-static scalar sector. The TT projection inherits one or the other's no-propagating-rank-2-pole property — either the J spin-1 carries through (bubble-continuum, no pole; per (C2-2)) or the ℒ quasi-static character dominates (per (b) above). Resolution: no new propagating spin-2 pole possible from cross-bilinears.

The §5.1 uniqueness should therefore read: **"uniqueness of *propagating-mode-candidate* rank-2 observables given (C2-1)'s mode-count restriction, with higher-multilinears reducing to bilinears at lowest order, ℒ-bilinears non-propagating per (C2-1) §3.2, and J-ℒ cross-bilinears inheriting one or the other's no-rank-2-pole property."**

**Critical F-h check (load-bearing):** the (C2-3) [THEOREM]-grade tag rests on the **structural argument** above (rep-theoretic uniqueness with sub-cases handled + bubble-integral analytic structure + linear-spectrum mode count), NOT on FTD-0193 empirics. FTD-0193 is the **empirical FLOOR for the canonical-toggle [SMC] tag**, not the [THEOREM] proof. The composition argument stands without empirics. **F-h passes.**

**Tag (C2-3):** [THEOREM] at free-theory + Gauss-only (composition, with sub-cases explicit per Caveat-1); [SMC] at canonical-toggle (inherits from C2-2). 

---

## §7 — Walk-through (C2-4) clause (pre-reg §9 step 6)

Verify DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY §6 + reference to FTD-0189 audit chain:

| Sub-step | Source | Status |
|---|---|---|
| 6.1 Deser-bootstrap chain in DERIV_EINSTEIN_FIELD_EQUATIONS.md Step 5: h_μν enters as Conjecture 10.1 (posited) per FTD-0189 audit | DERIV_EINSTEIN_FIELD_EQUATIONS §1 + Step 3 + Step 5 (retagged 2026-05-24 per FTD-0189 ripple per commit `9e85bd7`) |  EFE-6/8/9 now [SELECTION/CONDITIONAL] per ripple housekeeping |
| 6.2 Conjecture 10.1 status: **[CLOSED NEGATIVE for *substrate emergence* in the probed regime per FTD-0193]**; **importation as effective-theory scaffold per (C2-4) is *unaffected*** | FTD-0193 + REPORT_GRAVITON_SUBSTRATE_MODE.md §6 |  empirical floor confirmed (Caveat-4: scope explicit) |
| 6.3 Net for (C2-4): full nonlinear GR matched via Deser-bootstrap of posited h_μν; NOT substrate-derived | Composition 6.1 + 6.2 |  [REFERENCE] tag (not derivation) |

**Caveat-4 (per adversarial review Gap-3, addressed here) — Conjecture 10.1 scope-bounding.** The Conjecture 10.1 status is **explicitly limited**: `[CLOSED NEGATIVE for *substrate emergence* in the probed regime (L ≤ 64, J-bilinear observables, dual substrate per FTD-0193 measurement); importation as effective-theory scaffold per (C2-4) is *unaffected*]`. The conjecture is NOT globally falsified — h_μν remains importable as effective-theory scaffold within the Deser-bootstrap chain; that is precisely the matching/import statement (C2-4) makes precise. This caveat guards against a reader interpreting Conjecture 10.1 as globally falsified, which it is not.

**Tag (C2-4):** [REFERENCE] — established by retagged DERIV_EINSTEIN_FIELD_EQUATIONS chain + FTD-0189 audit + FTD-0193 empirical floor with scope-bounded Conjecture 10.1 status. NOT a new derivation; reference to already-established structural import status with scope explicit per Caveat-4.

---

## §8 — Arc B P2 verdict branch handling (pre-reg §9 step 7, D7)

Per pre-reg D7: Arc B P2 verdict determines the scalar-sector statement. Status at this closure attempt:

- **Arc B P2 status (2026-05-25):** pre-reg `preregister-clock-hypothesis-derivation-v1` hash-locked at commit `4c15ba1` (this session); closure attempt scheduled as Phase B of Step 2+3 execution (immediately after this Arc C2 P4 closure). **Verdict: PENDING.**
- **Branch handling per D7:** since Arc B P2 verdict is not yet in hand at this closure attempt time, this result-doc includes BOTH branch statements per D7 Branch C:
  - **Branch A (Arc B P2 = FOUND):** scalar sector extends to full Schwarzschild proper time [THEOREM]; this boundary theorem's "scalar sector" statement reads as "FTD substrate-derivable scalar gravity reaches full Schwarzschild proper time [THEOREM via SPEC §4.3]".
  - **Branch B (Arc B P2 = CLOSED-NEGATIVE):** scalar sector stops at "g_00 form via Phase G + clock-hypothesis AXIOM"; this boundary theorem's "scalar sector" statement reads as "FTD substrate-derivable scalar gravity reaches Schwarzschild g_00 form via Phase G + the clock-hypothesis AXIOM tagged in SPEC §4.3".
- **Verdict on (C2-3) is unaffected by Arc B P2 branch** per DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY §8 (the boundary theorem's spin-2 forbiddenness depends only on (C2-1) ∧ (C2-2) ∧ §5.1 uniqueness, all of which are independent of the clock hypothesis).

**Compliance check:** F-j (Arc B P2 verdict must be stated) —  satisfied via D7 Branch C dual-branch statement.

---

## §9 — F-a..F-j falsifier checklist (pre-reg §9 step 8, mechanical)

Per pre-reg §7. Each rule: fired (FAIL) or passed (PASS) with one-sentence justification. **Independently verified by adversarial reviewer (see §11).**

| Rule | Description | Status | Justification |
|---|---|---|---|
| **F-a** | No import of h_μν as derivation input |  PASS | No (C2-1), (C2-2), or (C2-3) prose uses "h_μν" as derivation primitive; only cited in (C2-4) as the posited input per FTD-0189 |
| **F-b** | No Deser bootstrap as substrate-emergence evidence |  PASS | (C2-4) §7 explicitly tags Deser as [REFERENCE] for effective-theory matching, NOT as substrate emergence |
| **F-c** | No invocation of "Lovelock implies substrate-derived GR" |  PASS | Lovelock not cited as positive substrate-derivation evidence anywhere in walk-through |
| **F-d** | Non-separability demonstrated empirically AND analytically |  PASS | §5 cites both FTD-0193 §4 empirical (11/12 k-points L=64) AND DERIV_J_BILINEAR §3 + §3.4 analytical (bubble-integral branch-cut structure) |
| **F-e** | Boundary scope explicitly states L ∈ {32, 64} + §4 catalog |  PASS | §3 + §13 explicitly scope to L ≤ 64 probed regime + §4 frozen catalog; theorem does NOT claim "for all L" without Arc C1 evidence |
| **F-f** | Respects §4 frozen catalog |  PASS | No non-§4-catalog observables invoked; §5.1 uniqueness argument is within catalog, with sub-cases per Caveat-1 |
| **F-g** | No re-invocation of closed-negative routes (FTD-0073, FTD-0184, FTD-0050) |  PASS | None cited as positive evidence; FTD-0073 is referenced as background context for site-local Clifford closure, not as a derivation primitive |
| **F-h** | No conflation of FTD-0193 empirics with axiomatic forbiddenness |  PASS | §6 (C2-3) walk-through explicitly grounds [THEOREM] tag on the structural composition argument with sub-cases per Caveat-1; FTD-0193 cited as empirical FLOOR for [SMC] canonical-toggle tag, not as proof of axiomatic forbiddenness. **Load-bearing falsifier; adversarial review confirmed correct handling.** |
| **F-i** | No look-elsewhere across rank-2 observables |  PASS | §5.1 uniqueness argument fixes the candidates (J-bilinear `O_ij` + `Õ_ij` plus sub-cases per Caveat-1); no observable-substitution mid-walk |
| **F-j** | Arc B P2 verdict stated |  PASS | §8 D7 Branch C explicit dual-branch statement (Arc B P2 verdict pending; both branches A and B documented) |

**Falsifier summary: 0/10 falsifiers fire.** Independently confirmed by adversarial reviewer (§11).

---

## §10 — B-1..B-8 banned-moves checklist (pre-reg §9 step 9, mechanical)

Per pre-reg §8. Each rule: invoked (FAIL) or not (PASS) with justification. **Independently verified by adversarial reviewer (see §11).**

| Rule | Description | Status | Justification |
|---|---|---|---|
| **B-1** | No tag-promotion of DERIV docs beyond current dual tags before result-doc lands |  PASS | DERIV docs' tags unchanged in this walk-through; result-doc promotes ONLY the boundary-theorem-as-a-whole tag (FTD-0209), not constituent DERIV-doc tags |
| **B-2** | No re-derivation of (C2-2) bubble structure beyond pre-reg authorization |  PASS | §5 walks DERIV_J_BILINEAR §3 mechanically; Caveat-2 (finite-L lattice acknowledgment) is scope clarification, not substantive amendment |
| **B-3** | No metaphysical priors ("graviton must exist somewhere") |  PASS | No such priors invoked |
| **B-4** | No LIGO/Virgo as substrate-spin-2 evidence |  PASS | LIGO/Virgo not cited in walk-through (consistent with (C2-4) matched-GR framing) |
| **B-5** | No [THEOREM]/[SMC] tag conflation in result-doc |  PASS | Each clause's dual tag (free-theory vs canonical-toggle) preserved explicitly throughout §4-§7 and in §12 finalization |
| **B-6** | No silent extension of §4 catalog |  PASS | All observable references within `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4 catalog; Caveat-1 sub-cases (higher J-multilinears, ℒ-bilinears, J-ℒ cross-bilinears) are within-catalog refinements |
| **B-7** | "Standard QFT result" claims cite canonical references |  PASS | DERIV_J_BILINEAR §3 explicitly cites Peskin-Schroeder §10.2 + Montvay-Münster §3 |
| **B-8** | (C2-3) [THEOREM] promotion with explicit dual-tag handling |  PASS | §6 explicit "[THEOREM] at free-theory + Gauss-only (composition with sub-cases); [SMC] at canonical-toggle"; not collapsed to single tag |

**Banned-moves summary: 0/8 invoked.** Independently confirmed by adversarial reviewer (§11).

---

## §11 — Adversarial review verdict (pre-reg §9 step 10) — PASS-WITH-CAVEATS

**Adversarial reviewer:** independent `general-purpose` agent dispatched 2026-05-25 with: pre-reg path, DERIV docs paths, draft result-doc path, REPORT_GRAVITON_SUBSTRATE_MODE path, F-/B-checklist verification mandate.

**Reviewer's verdict (verbatim):** **PASS-WITH-CAVEATS**

**Reviewer's reasoning (excerpted verbatim from agent output):**

> The closure attempt walks the §9 method 11-step correctly. The dual-tag structure (free-theory [THEOREM] + Gauss-only vs canonical-toggle [SMC] with FTD-0193 floor + [REFERENCE] for C2-4) is preserved explicitly throughout — including in the §12 finalization clause and the §14 single-line summary. The load-bearing F-h rule (which catches the F9 risk of resting the [THEOREM]-grade tag on empirics rather than structure) is correctly handled in §6 row 4: the (C2-3) [THEOREM] tag rests on the composition `(C2-1) ∧ (C2-2) ∧ §5.1 uniqueness`, with FTD-0193 explicitly demoted to "empirical FLOOR for the [SMC] canonical-toggle tag, not the [THEOREM] proof." This is the discipline-bearing test of the entire closure attempt and it is correctly passed.
>
> The F-checklist (0/10) and B-checklist (0/8) walks are mechanically correct and I confirm each line independently. The Arc B P2 branch-handling (D7 Branch C dual-statement) is correctly applied given the pending Arc B P2 verdict. All structural-chain elements ((C2-1) linear spectrum, (C2-2) bubble-integral analytic structure, (C2-3) composition, (C2-4) [REFERENCE] to retagged Deser chain) check out against the DERIV docs and the FTD-0193 empirical floor.
>
> The single residual concern is documentation-level: the §5.1 group-theoretic uniqueness argument in DERIV_J_BILINEAR §2.2 is informal about three sub-cases (higher J-multilinears, ℒ-bilinears, J-ℒ cross-bilinears). Each sub-case is structurally handled by the framework... but the closure attempt does not explicitly walk them. The result-doc should make this explicit. The conclusion stands either way.
>
> **The result-doc should land as `FOUND_SPIN2_BOUNDARY_THEOREM.md` per §6 Outcome A**, with the §5.1 uniqueness scope clarified per the caveats below.

**Reviewer's 4 caveats (all incorporated inline in this final result-doc):**

1. **§5.1 uniqueness scope** → addressed in §6 above as **Caveat-1 sub-case walk** (higher J-multilinears reducible; ℒ-bilinears non-propagating per (C2-1) §3.2; J-ℒ cross-bilinears inherit no-rank-2-pole property)
2. **Lattice bubble-integral finite-L caveat** → addressed in §5 above as **Caveat-2** (continuum branch-cut is `L → ∞` limit; finite-L is dense two-particle resonance peaks; FTD-0193 L=64 is the lattice test)
3. **L=128 deferral framing** → addressed in §13 below
4. **Conjecture 10.1 framing** → addressed in §7 above as **Caveat-4** (scope-bounded: [CLOSED NEGATIVE for substrate emergence in probed regime; importation as effective-theory scaffold unaffected])

**None of the caveats demands a v2 pre-reg or invalidates the closure attempt.** All are documentation-level clarifications for the final result-doc to inherit honestly. The closure attempt's structural argument and F-/B-checklist walks are correct and independently verified. **Outcome A (FOUND) is earned at the locked dual-tag scope.**

---

## §12 — Verdict assignment per §6 (pre-reg §9 step 11) — Outcome A FOUND

**§6 Outcome A (FOUND):** four-clause theorem holds; dual-tag scope ([THEOREM] free-theory + [SMC] canonical-toggle + [REFERENCE] for C2-4) confirmed; F-/B-checklists clean (0/10 falsifiers fire; 0/8 banned moves invoked); independent adversarial review PASS-WITH-CAVEATS (caveats addressed inline above).

**Tag consequences per pre-reg §6:**
- **LEDGER row FTD-0209 created** with tag `[THEOREM at free-theory + Gauss-only level for (C2-1), (C2-2), (C2-3); SMC for canonical-toggle extension per FTD-0193 empirical floor; REFERENCE for (C2-4) Deser-bootstrap matching]`.
- **DERIV docs** (`DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md`) confirmed as load-bearing proof scaffolds.
- **Plan v2 Arc C2 marked CLOSED with FOUND verdict.**
- **No FTD claim beyond the boundary-theorem tag itself is promoted or demoted.** Existing FTD-0131, FTD-0184, FTD-0189, FTD-0193 tags unchanged.
- **Arc E (MC-T4.3 alpha-readout boundary theorem) inherits methodology validated** by this Arc C2 P4 FOUND outcome per plan v2 Arc E cross-arc dependency.

---

## §13 — Honest limits + scope hedges

- **Scope limit:** §4 frozen catalog observables only. Doctrine §12 candidate principles (finite-trace `s_m` variation, graph spectral curvature, finite adjacency deformation) outside scope — Arc C1 territory if pursued.
- **L-scope (Caveat-3 per adversarial review Gap-3 framing):** L ∈ {32, 64} empirically probed (FTD-0193). Theorem at free-theory level is L-independent in form (bubble integral has same analytic structure at any L, per Caveat-2's continuum-spectral framing), but canonical-toggle [SMC] tier's empirical floor is at L ≤ 64. **L > 64 is [OPEN] empirically — but the [OPEN] tag is *instrumentation-bounded, not theorem-bounded*.** Specifically: L=128 was attempted twice and deferred for engineering reasons (hostdevice transfer + CPU-side operator computation at L³=2M voxels per REPORT_GRAVITON_SUBSTRATE_MODE §5), not for methodological reasons. The boundary theorem at the free-theory level extends predictively to L > 64 modulo Arc C1 GPU-port engineering work.
- **Arc B P2 verdict:** pending; D7 Branch C dual-branch statement covers both outcomes for the scalar-sector clause.
- **F9 risk handled:** prior-favoured Outcome A was "easy" because DERIV docs already did the substantive work. §9 F-h is the load-bearing falsifier specifically designed to catch the "structural argument vs empirical floor" conflation; adversarial reviewer independently verified F-h handling. The structural composition argument `(C2-1) ∧ (C2-2) ∧ §5.1 uniqueness` (with sub-cases per Caveat-1) stands independent of FTD-0193 empirics.
- **F10 risk handled:** the [THEOREM] tag at free-theory level is *recognition* of the structural fact (linear spectrum + bilinear analytic structure forbid spin-2 pole in §4 catalog); it does NOT fix the substrate-derived-gravity question more broadly. Substrate gravity content remains scalar (FTD-0131 modulo clock hypothesis pending Arc B P2) + vector (J spin-1); the upper end remains effective-theory matching per (C2-4). The boundary is mapped honestly, not enlarged.

---

## §14 — Single-line summary

**The spin-2 boundary theorem of `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` §1 lands as Outcome A (FOUND) per pre-reg §6: [THEOREM] for clauses (C2-1) linear spectrum, (C2-2) J-bilinear non-separability, (C2-3) no substrate-derived graviton in §4 catalog — all at free-theory + Gauss-only level via composition of linear-spectrum analysis (DERIV §3) + J-bilinear bubble-integral analytic-structure analysis (DERIV_J_BILINEAR §3, citing Peskin-Schroeder §10.2 + Montvay-Münster §3) + group-theoretic uniqueness with sub-case walk per Caveat-1 (higher J-multilinears reducible, ℒ-bilinears non-propagating, J-ℒ cross-bilinears inherit no-rank-2-pole); [SMC] for canonical-toggle extension with FTD-0193 empirical floor (11/12 k-points L=64 TT correlator identical to spin-1 control — continuum bubble-integral envelope at finite-L per Caveat-2); [REFERENCE] for (C2-4) Deser-bootstrap matching of POSITED h_μν per FTD-0189 + retagged DERIV_EINSTEIN_FIELD_EQUATIONS chain (Conjecture 10.1 scope-bounded per Caveat-4: substrate emergence [CLOSED NEGATIVE in probed regime]; effective-theory import unaffected); 0/10 F-falsifiers fire; 0/8 banned moves invoked; independent `general-purpose` agent adversarial review PASS-WITH-CAVEATS (all 4 caveats addressed inline); LEDGER row FTD-0209 created; plan v2 Arc C2 marked CLOSED; Arc E inherits validated methodology — scope clarification at theorem-grade rigor at the locked dual-tag scope, NOT "we proved no graviton."**
