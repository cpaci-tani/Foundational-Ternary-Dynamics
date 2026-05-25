# SCOPE — Arc C2 spin-2 boundary theorem: structural-decoupling sibling for the substrate-gravity content

**Tag:** `[SCOPING MEMO]` — not a pre-registration, not a closure, not a tag promotion. Identifies the theorem-statement structure, axiom dependencies, proof-structure preview, falsifier-criteria preview, and connection to existing work for the Arc C2 boundary theorem of Wilsonian-reframe plan v2. Mirrors the form of `SCOPE_NEWTON_POSTULATES_RECONCILIATION.md` (Arc B P0) and the prior-art template of `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` (FTD-0186 Stage 1).
**Date:** 2026-05-24
**LEDGER row reservation:** to be confirmed against `../07_assessment/LEDGER.md` at hash-lock; provisional placeholder pending audit.
**Plan:** `~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md` v2 (Wilsonian reframe) — Arc C2 P0 deliverable.
**Companion docs:**
- [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](REPORT_GRAVITON_SUBSTRATE_MODE.md) — FTD-0193 `[CLOSED NEGATIVE per Outcome B]` 2026-05-22 — the decisive empirical evidence
- [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](PREREG_GRAVITON_SUBSTRATE_MODE_v2.md) — the pre-registration FTD-0193 closed against
- [`PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md`](PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md) — FTD-0186 Stage 1 v2 (template for Arc C2's pre-reg v1)
- [`../02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md`](../02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md) — the v1-execution result (boundary-theorem prior-art)
- [`../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §3 — the FTD-0189 ripple analysis (Linearized Einstein retag, load-bearing for Arc C2 framing)
- [`../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) — FTD-0131 substrate gravity derivation (Wilsonian scaling-law lower-end)
- [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) §4 — the frozen non-site-local observable catalog Arc C2 must respect

> **What this memo is NOT.** Not a pre-registration. Not a tag promotion. Not the boundary theorem itself. It identifies the boundary-theorem program for a future Arc C2 P3 pre-registration + P4 closure attempt. Per plan v2 Arc C2 P0 deliverable: structural-precise theorem-statement scoping + dependency mapping + risk register.

---

## §1 — Why a boundary theorem here, and what it serves

**Origin.** The Wilsonian reframe (plan v2, 2026-05-24) collapses "gravity across all discrete levels" into three components: discrete floor (a_phys ≡ ℓ_P, ESTABLISHED), upper undefined boundary (no completed-infinity, ESTABLISHED), and the scaling law between them (P1+P2 of DERIV_NEWTON, see Arc B). Arc C2 is the **upper-end formalization** — it states precisely where the substrate-derived scaling stops and where effective-theory matching takes over. This is the "honestly establish what we cannot derive" deliverable for the gravity-recovery program.

**Empirical input already in hand.** `FTD-0193` (2026-05-22) measured the substrate's connected transverse-traceless rank-2 correlator at L∈{32,64} and returned **Outcome B (no gapless helicity-±2 pole)**. The decisive finding: the flux-quadrupole TT operator's extracted ω is identical to the spin-1 control ω at 11/12 k-points to 7 significant digits — the rank-2 bilinear carries the spin-1 mode through, not an independent emergent spin-2 collective mode. Both spin-2 channels are 7-9 orders of magnitude below the validated spin-1 control. This is a positive identification of "continuum, no pole," not an unresolved Indeterminate.

**Doctrine clause this serves.** CLAUDE.md goal-clause 2: "Derive everything we can from a discrete ontology — **and rigorously establish what we cannot.**" Arc C2 makes the substrate-gravity boundary precise and theorem-grade: substrate hosts scalar (FTD-0131 g_00 via Phase G + Born-Infeld §4.3) and vector (transverse-J spin-1 per FTD-0193 control) sectors; helicity-±2 is forbidden under the §4-catalog observable algebra; full nonlinear GR enters as Deser-bootstrap extension of POSITED `h_μν` per FTD-0189 (Conjecture 10.1, structural import).

**Sibling to FTD-0186.** Arc C2 is the **gravity-sector dual** of FTD-0186 Stage 1 (the structural/dynamical-value discriminator). FTD-0186 distinguishes claims that the substrate's combinatorics forces (STRUCTURAL) from claims that require external input (NON-UNIVERSAL DYNAMICAL). Arc C2 distinguishes substrate-derivable gravity content (scalar + vector, established) from import-required gravity content (spin-2 + full nonlinear GR, established as boundary). The two boundary theorems are independent in their axes; both serve clause 2.

**Prior-favoured outcome.** FOUND. FTD-0193's evidence is decisive within the probed regime; the structural argument (J⊗J bilinears cannot produce separable spin-2 poles because J is a vector field whose linear spectrum decomposes as 1 spin-0 ⊕ 2 spin-1) is straightforward. The risk is **NOT** that the theorem fails to land; the risk is that it lands too easily without engaging the genuine subtleties (F9 collusion bias — the "obvious" theorem is sometimes the one that hides its own assumptions).

---

## §2 — Working theorem statement (DRAFT for scoping — to be locked at Arc C2 P3 pre-reg)

> **Spin-2 Boundary Theorem (working draft, not locked).** Under FTD axioms 1-5 (lattice, discrete time, ternary states, 26-Moore locality, determinism per `SPEC_FTD.md`), the canonical toggle set (engine default `latency_field` on, `wave_propagation` on, etc.), the calibration declarations (`a_phys ≡ ℓ_P`, `K_B = m_e`, `t_phys = √3·ℓ_P/c`), and the non-site-local observable algebra of `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4 frozen catalog, the FTD substrate's connected two-point correlator in the transverse-traceless rank-2 sector contains no gapless helicity-±2 pole, equivalently:
>
> **(C2-1)** The substrate's linear-wave spectrum on the flux field `J ∈ ℝ³` decomposes per wavevector as 1 spin-0 mode ⊕ 2 spin-1 transverse modes ⊕ 1 spin-1 longitudinal mode (Gauss-constrained), totalling 4 degrees of freedom per voxel after Gauss projection.
> **(C2-2)** Any rank-2 observable built as a J-bilinear or J-derivative bilinear has its transverse-traceless projection contain only the spin-1 mode propagated through the bilinear (a continuum/branch-cut contribution), not a separable spin-2 collective mode.
> **(C2-3)** Therefore there is no substrate-derived emergent graviton in the §4-catalog observable class within the probed regime; the metric perturbation `h_μν` enters the substrate-gravity content as Conjecture 10.1 per FTD-0189, not as a derived quantity.
> **(C2-4)** Full nonlinear GR (Einstein equations, gravitational waves) is recovered via the Deser-bootstrap construction `(h_μν posited) → (linearized EFE [SELECTION/CONDITIONAL]) → (Lovelock completion [SELECTION/CONDITIONAL])`, as documented in `DERIV_EINSTEIN_FIELD_EQUATIONS.md` (corrected 2026-05-24 per FTD-0189 ripple); this is effective-theory matching, not substrate emergence.
>
> The substrate-derivable gravity content is therefore: (a) Schwarzschild g_00 (scalar sector) via FTD-0131 + Phase G + Born-Infeld §4.3 modulo the clock hypothesis (Arc B P2 status pending); (b) Newtonian 1/r² + lab-scale time dilation as derived limits; (c) the Wilsonian-reframe scaling law from discrete floor (ℓ_P) to undefined-boundary upper limit via P1+P2. The boundary above which substrate derivation stops: full nonlinear GR + gravitational waves + cosmological-scale GR. These are matched, not derived.

**Scope hedge (working draft):** the theorem statement is anchored to the §4 frozen catalog and the probed regime (FTD-0193's L∈{32,64} + dual substrate + flux-quadrupole/stress operators). Extension to L>64 or to non-§4-catalog observables would require Arc C1 (extended spin-2 search). Arc C2 establishes the boundary at the current measurement frontier; the pre-reg (P3) should make this scope explicit and decide whether to widen it or accept the §4-catalog scope.

---

## §3 — Proof-structure preview

The boundary theorem (when properly stated and pre-registered) likely has the following derivation chain:

1. **(Algebraic, structural)** FTD's fundamental field is the flux field `J : Λ × T → ℝ³` per `SPEC_FTD.md` axiom 1. As a per-voxel vector field, J carries 3 components per voxel.
2. **(Spectrum decomposition, [THEOREM])** Under the linearized wave equation `Δ_t² J_a = c² L_18 J_a` (with L_18 the 18-point Laplacian per SPEC_FTD_LAGRANGIAN §3.1), the spectrum on a periodic L³ lattice decomposes per Fourier mode k as: 1 longitudinal scalar (after Gauss projection: the gauge mode) + 2 transverse vector modes (the spin-1 photon equivalent). Both are `[THEOREM]`-grade derivations from standard linear wave-equation theory.
3. **(Bilinear closure, [THEOREM])** Any rank-2 observable built as `O_ij = J_iJ_j - ⅓δ_ij|J|²` (flux-quadrupole) or `[∂_iJ_a · ∂_jJ_a]_TT` (stress) has its 2-point correlator factorize through the J-mode propagator. The TT projection contains: (a) the constant-multiplier "mean field" piece (gauge), (b) the propagating spin-1 contribution carried through the bilinear product, (c) a two-particle continuum. There is no separable spin-2 collective mode arising from interaction — the J-quanta are spin-1, and bilinears in spin-1 fields produce spin-0 ⊕ spin-1 ⊕ spin-2 *kinematically* but only continuum-level (not pole-level) in the spin-2 channel.
4. **(Empirical validation, FTD-0193)** Measured directly at L∈{32,64} (CPU-FFT + cuFFT twice-validated, instrument cross-checked via 12/12 spin-1 control recovery): TT correlator ω is identical to spin-1 control ω at 11/12 k-points to 7 sig figs. The non-separability is empirically established within the probed regime.
5. **(No alternative within §4 catalog)** The Arc C1 candidate principles (finite-trace s_m variation, graph spectral curvature, finite adjacency deformation per Doctrine §12) either (a) close negative if extended to L>64 + alternative non-bilinear probes (would be Arc C1's job, not Arc C2's), or (b) fall outside the §4 frozen catalog and require an ontology extension (boundary theorem then explicitly says: substrate emergence requires ontology extension).
6. **(Theorem conclusion)** Spin-2 emergence is forbidden under the stated axioms + toggle set + observable algebra. Full nonlinear GR enters as effective-theory matching via Deser-bootstrap of posited `h_μν`. Substrate-derivable gravity content is scalar + vector only.

**Key dependency on Arc B P2 verdict:**
- If Arc B P2 closes FOUND (clock hypothesis substrate-derived): the substrate gravity content extends to Schwarzschild proper time as fully [THEOREM]; the boundary theorem's "scalar sector" claim is correspondingly strengthened.
- If Arc B P2 closes CLOSED-NEGATIVE (clock hypothesis is irreducibly interpretive AXIOM): the substrate gravity content stops at "g_00 form via Phase G + clock-hypothesis AXIOM"; the boundary theorem honestly names the clock hypothesis as one of two axiomatic inputs (the other being h_μν per FTD-0189).
- Either Arc B P2 verdict is compatible with Arc C2 closing FOUND; only the precision of the substrate-derivable-content statement differs.

---

## §4 — Axiom dependencies (preview; to be locked at Arc C2 P3 pre-reg)

The theorem's stated axiom set should include:

1. **FTD axioms 1-5** (the five postulates of `SPEC_FTD.md`) — cubic lattice + discrete time + ternary states + 26-Moore locality + determinism.
2. **Canonical toggle set** — the engine's default ON/OFF set as documented at the time of pre-registration (snapshot needed); equivalent to the §8 toggle set of `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` (11 toggles ON, `dual_substrate` + `weak_transmutation` OFF).
3. **Calibration declarations** — `a_phys ≡ ℓ_P` (FTD-0041 / FTD-0137 gauge), `K_B = m_e` (mass anchor), `t_phys = √3·ℓ_P/c` (tick calibration). These do NOT affect the dimensionless theorem content (the spin-2 emergence question is dimensionless) but are listed for completeness.
4. **§4 frozen observable catalog** from `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4 — closed flux-loops, plaquette bivectors, bilinear link observables, boundary-to-boundary transfer observables, reflexive projections. Arc C2 inherits this catalog.
5. **FTD-0193 empirical input** (L∈{32,64}, twice-validated instrument) — used as load-bearing evidence in the proof chain, not as a derivation primitive.
6. **FTD-0073 [CLOSED NEGATIVE]** — the site-local Clifford route is closed; Arc C2 cannot recover spin-2 via site-local primitives.
7. **FTD-0189 [AUDIT FINDING]** — h_μν enters as Conjecture 10.1; the Deser-bootstrap chain in DERIV_EINSTEIN_FIELD_EQUATIONS.md is [SELECTION/CONDITIONAL] per the 2026-05-24 ripple housekeeping.

**Conditional dependency:** Arc B P2 verdict on the clock hypothesis (FOUND or CLOSED-NEGATIVE); the theorem's "substrate-derivable scalar sector" claim adjusts accordingly. The pre-reg should specify both branches.

---

## §5 — Outcome criteria (preview; to be locked at Arc C2 P3 pre-reg)

Following the FTD-0186 v2 template, three pre-blessed outcomes:

**Outcome A (FOUND).** The boundary theorem is proven as stated: (C2-1) through (C2-4) all hold with explicit derivation chains tagged at each step. Tag: `[THEOREM, conditional on axioms 1-7 listed above + Arc B P2 verdict]`. LEDGER row created (provisional next-free); `FOUND_SPIN2_BOUNDARY_THEOREM.md` published. Plan v2 Arc C2 marked CLOSED.

**Outcome B (CLOSED-NEGATIVE).** A substrate spin-2 mode IS theoretically possible under some interpretation of the §4-catalog observables that the proof did not foresee (specifically: a non-J-bilinear observable that escapes the §3 step-3 closure but stays within §4 catalog). This refutes (C2-3) and forces Arc C1 (extended spin-2 search) to be re-opened to test the candidate. Tag: `[OPEN, refuted at v1; Arc C1 re-opened]`. The boundary theorem cannot be stated as a theorem; the §4 catalog needs widening or the proof needs deepening.

**Outcome C (UNDERDETERMINED).** The proof reaches (C2-1) and (C2-2) cleanly but (C2-3) requires an additional principle outside the §4 catalog (e.g., a finite-trace mechanics axiom proven elsewhere). Tag: `[PARTIAL, awaits additional principle derivation]`. Plan v2 Arc C2 marked PARTIAL; a v2 pre-reg expanding the §4 catalog is queued.

**Prior-favoured outcome: FOUND.** FTD-0193's evidence is decisive; the structural argument is straightforward; the §4 catalog is already restrictive enough to exclude the "deeper" non-bilinear observables that would be Arc C1's job. The risk is F9 (the "obvious" theorem hides assumptions); the mitigation is the falsifier checklist (next section) + adversarial review.

---

## §6 — Falsifier criteria preview (F-a through F-j; to be locked at Arc C2 P3 pre-reg)

Mechanical falsifier checklist (preview; to be refined at pre-reg time):

- **F-a.** No import of h_μν as a derivation input. The boundary theorem's whole point is that h_μν is posited per FTD-0189; importing it would beg the question.
- **F-b.** No use of Deser bootstrap to claim substrate emergence. Deser bootstrap COMPLETES posited h_μν; it does not derive it.
- **F-c.** No invocation of "Lovelock's theorem implies substrate-derived GR" — Lovelock characterizes the unique nonlinear completion given linearized GR, but linearized GR per FTD-0189 is `[SELECTION/CONDITIONAL]` not `[THEOREM]`.
- **F-d.** Must demonstrate non-separability of any candidate spin-2 channel from spin-1 sector — empirically (FTD-0193 reference) or analytically (J-bilinear factorization argument).
- **F-e.** The boundary "L∈{32,64}" must be explicitly stated; theorem cannot claim "for all L" without Arc C1 evidence at L>64.
- **F-f.** Must respect the §4 frozen catalog. Any non-§4-catalog observable invocation triggers F-f and forces Outcome C or requires v2 pre-reg.
- **F-g.** No re-invocation of closed-negative routes — FTD-0073 (site-local Clifford), FTD-0184 (Yilmaz exponential metric), FTD-0050 (master-quadratic-as-RG-step) all closed-negative; cannot be re-imported as spin-2 mechanisms.
- **F-h.** No conflation of "engine cannot find spin-2 at L=64" (FTD-0193 empirical) with "spin-2 is forbidden by axioms" (the theorem statement). The theorem must produce the forbidden-by-axioms claim via structural argument, not just cite empirics.
- **F-i.** No look-elsewhere across rank-2 observables — the §4 catalog is the lock; finding a "lucky" observable outside it is F-i firing.
- **F-j.** Arc B P2 verdict must be stated; if pending, the theorem must include both branches explicitly.

---

## §7 — Connection to existing work + the Stage-2 framing question

**FTD-0186 sibling.** As noted in §1, Arc C2 is the gravity-sector dual of FTD-0186 Stage 1 (structural/dynamical-value discriminator). They are independent on their axes but methodologically parallel; the FTD-0186 v2 template directly informs the Arc C2 v1 pre-reg structure.

**Wilsonian-reframe placement.** Per plan v2 §"Cross-arc synthesis":
- Arc B = the scaling law itself (P1+P2 of DERIV_NEWTON)
- Arc C2 = the upper-end boundary of substrate-derivable gravity
- Arc D = the empirical scaling demonstration

Arc C2 closes the program upward: it states precisely where the substrate derivation stops and effective-theory matching takes over. The "appropriate scaling" criterion of the Wilsonian reframe is satisfied by the union of {Arc B scaling law, Arc D engine demonstration across scales, Arc C2 boundary statement}.

**Conditional dependency on FTD-0198 ARC-B1 (the alpha-readout obstruction).** FTD-0198 is the active first attack on MC-T4.3 (the central foundational obstruction). Its prior-favoured outcome is CLOSED-NEGATIVE. If FTD-0198 closes CLOSED-NEGATIVE, it becomes load-bearing input to FTD-0186 Stage 2 (the structural-decoupling theorem) — the dynamical-value axis is non-closable by the FTD-native non-site-local observable class. Arc C2 inherits this only indirectly: the §4 catalog Arc C2 uses is the same catalog FTD-0198 is attacking; if FTD-0198 fails to close MC-T4.3 from the same catalog, the catalog's "exhaustion" frame is strengthened — but Arc C2's boundary theorem is about spin-2 emergence, not about alpha readout, so the link is indirect.

**Stage-2 framing question (load-bearing for honesty).** Just as FTD-0186 §1 makes explicit that "v2 is not a 'win,' it is a scope clarification," Arc C2 should similarly flag that the boundary theorem is a scope clarification — it makes the boundary precise where it was previously informal. It does NOT claim to "solve gravity"; it claims to honestly map where substrate-derivation reaches. The F9 risk here is that the theorem is too easy (everyone "knows" J-bilinears can't make spin-2 poles) and therefore is mistaken for a deeper result than it is. The pre-reg should explicitly disclaim this.

---

## §8 — Risk register

| Risk | Severity | Mitigation |
|------|----------|------------|
| Theorem too easy → mistaken for deeper than it is (F9) | High | §1 + §5 explicit "FOUND is the prior-favoured outcome; this is a scope clarification, not a substantive surprise" framing. Mirror FTD-0186 v2 §1 honesty. |
| §4 catalog "exhaustion" claim is hand-waved without ARC-B1 evidence | Medium | If FTD-0198 ARC-B1 closes CLOSED-NEGATIVE, cite it as evidence catalog is exhaustion-tested for dynamical-value axis. Acknowledge it doesn't directly test the spin-2-emergence axis. |
| L>64 boundary not addressed | Medium | Theorem scope explicitly limited to §4-catalog observables and probed regime; Arc C1 is the work that would widen this. |
| Arc B P2 verdict not yet in hand | Medium | Theorem statement includes both branches (FOUND-clock-hypothesis vs AXIOM-clock-hypothesis); pre-reg should defer P3 lock until Arc B P2 closes, OR write the pre-reg now with both branches and update verdict downstream. |
| F10 (rigidity-gap licensing) — boundary theorem treats the F9-flagged "scope clarification" as if it had closed the underlying structural question | High | §7 explicit "this does NOT claim to solve gravity; it claims to map where substrate-derivation reaches." Result-doc must include this disclaimer. |
| Outcome A becomes a "we proved no graviton" headline | Medium-High | Result-doc framing: "FTD substrate hosts scalar + vector gravity modes; spin-2 emergence in §4-catalog observables at L≤64 is excluded; full GR is matched, not derived." Avoid "we proved no graviton" phrasing. |
| Arc C1 closure positively (graviton found) would invalidate Arc C2 | Acknowledged | Sequence: Arc C2 P0 (this) → Arc C1 attempt (if pursued; optional per plan v2) → Arc C2 P3 lock after Arc C1 verdict. If Arc C1 lands FOUND, Arc C2 is moot and v2 reformulates. |

---

## §9 — Recommended next steps (Arc C2 P0 → P1 → P3 sequence)

1. **Wait for Arc B P2 verdict** before locking Arc C2 P3 pre-reg. The theorem statement's "scalar sector" clause depends on the clock-hypothesis outcome. (Alternative: write pre-reg with both branches; ~1 extra section of conditional content.)
2. **Theory P1 (3-5 weeks):** construct rigorous proof of (C2-1) through (C2-4) with each step tagged. Engage the §3 step-3 argument (J-bilinears cannot produce separable spin-2 poles) carefully — this is where the F9 risk is highest, and the structural argument needs to be tight rather than hand-waved.
3. **P3 pre-reg lock:** write `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md` per FTD-0186 v2 template (9-section), hash-lock + git tag (requires explicit user direction).
4. **P4 closure attempt:** execute proof per pre-reg; mechanical F-a..F-j checklist; adversarial review checkpoint before any "closed" verdict.
5. **P5 result-doc:** `FOUND_SPIN2_BOUNDARY_THEOREM.md` (Outcome A) or `AUDIT_SPIN2_BOUNDARY_THEOREM_*.md` (Outcome B/C).

---

## §10 — Honest limits of this scoping memo

- The §2 working theorem statement is **draft**; the actual locked statement at P3 may differ in scope (e.g., explicit L bound, explicit toggle-set snapshot).
- The §3 proof structure is **preview**; the actual proof at P4 must engage step-3 (J-bilinear non-separability) more carefully than this scoping memo does. The hand-waved phrase "spin-1 fields produce spin-0 ⊕ spin-1 ⊕ spin-2 kinematically but only continuum-level in the spin-2 channel" is the technical heart of the theorem and needs a proper proof, not just a citation.
- The §4 axiom dependencies list is **preview**; the actual P3 pre-reg should include an explicit dependency-graph diagram and version-stamp the canonical toggle set.
- The §6 falsifier list F-a..F-j is **preview**; F-h (no conflation of empirical absence with axiomatic forbiddenness) is the load-bearing falsifier and needs careful framing at P3.
- The §7 connection to Stage-2 framing question is **flag**; the actual P3 pre-reg should make the "scope clarification, not a win" framing as explicit as FTD-0186 v2 §1 does.

---

## §11 — Single-line summary

**Arc C2 (Wilsonian-reframe plan v2) scopes a spin-2 boundary theorem mirroring FTD-0186 Stage 1's structural/dynamical-value discriminator: substrate-derivable gravity content (scalar via FTD-0131 + Born-Infeld §4.3, vector via transverse-J spin-1 per FTD-0193 control) vs import-required gravity content (spin-2 + full nonlinear GR via Deser-bootstrap of posited h_μν per FTD-0189); prior-favoured outcome FOUND with FTD-0193 evidence decisive at L≤64 within §4 frozen catalog, but the F9 risk that "easy theorem hides assumptions" is high, requiring careful step-3 (J-bilinear non-separability) treatment at P1 and an adversarial-review checkpoint at P4 before any verdict lands — the boundary theorem's function is scope clarification ("rigorously establish what we cannot derive" per goal clause 2), not "we proved no graviton."**
