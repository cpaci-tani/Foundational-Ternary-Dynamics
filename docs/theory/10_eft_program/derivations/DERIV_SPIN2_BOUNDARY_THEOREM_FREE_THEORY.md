# DERIV — Spin-2 boundary theorem (free-theory + canonical-toggle scope)

**Tag:** `[THEOREM at free-theory + Gauss-only level, for clauses (C2-1), (C2-2 free), (C2-3 within §4 catalog)]` + `[STRONGLY MOTIVATED CONJECTURE for full canonical toggle set per FTD-0193 empirical validation]` + `[REFERENCE for clause (C2-4)]` (effective-theory matching via Deser bootstrap of posited h_μν per FTD-0189 + AUDIT_NEWTON_POSTULATES_RECONCILIATION §3 + DERIV_EINSTEIN_FIELD_EQUATIONS [SELECTION/CONDITIONAL] retags 2026-05-24).

This document consolidates Arc C2 P1 substantive derivation into a unified free-theory + canonical-toggle-set version of the boundary theorem. It is the load-bearing pre-pre-reg derivation underlying the future `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md` (Arc C2 P3). Tag promotion to a full closed-form theorem requires P3 pre-reg + P4 closure attempt against the locked design.

**Date:** 2026-05-24 (Arc C2 P1 consolidated deliverable, Wilsonian-reframe plan v2)
**LEDGER row reservation:** provisional, confirm next-free against `../07_assessment/core_ledgers/LEDGER.md` at hash-lock; expected to be cited by `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`.
**Plan:** `~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md` v2 Arc C2 P1 deliverable.
**Companion docs:**
- [`SCOPE_SPIN2_BOUNDARY_THEOREM.md`](../scopes_and_specs/SCOPE_SPIN2_BOUNDARY_THEOREM.md) — Arc C2 P0 scoping (parent; this is its P1 closure)
- [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) — load-bearing C2-2 derivation (J-bilinear bubble integral has no isolated pole)
- [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](../reports_and_audits/REPORT_GRAVITON_SUBSTRATE_MODE.md) — FTD-0193 `[CLOSED NEGATIVE]` 2026-05-22 empirical validation
- [`../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §3 — FTD-0189 ripple establishing (C2-4) framing
- [`../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md`](../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md) — retagged 2026-05-24 (EFE-6/8/9 → [SELECTION/CONDITIONAL]); the Deser-bootstrap chain cited by (C2-4)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) — §3 action functional, §4.2 [THEOREM] Poisson on ℒ, §4.3 [THEOREM modulo clock hypothesis] Born-Infeld Schwarzschild
- [`../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) — FTD-0131 substrate gravity (the scalar-sector lower-end of the boundary statement)
- [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) §4 — frozen non-site-local observable catalog Arc C2 inherits

> **What this document is.** A consolidated derivation of the four clauses of the spin-2 boundary theorem (C2-1 through C2-4) at free-theory + canonical-toggle scope. The free-theory clauses receive [THEOREM]-grade rigor; the canonical-toggle extension carries [SMC] tag per the FTD-0193 empirical validation; the effective-theory matching clause (C2-4) is established by reference to the already-retagged Deser-bootstrap chain.

> **What this document is NOT.** A pre-registration (that is the next P3 deliverable, `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`). A closure attempt against pre-registered falsifiers (that is P4). A new spine theorem (the spine count is unchanged). A claim that "we proved no graviton" — per the F9/F10 discipline in `SCOPE_SPIN2_BOUNDARY_THEOREM.md` §7, this is **scope clarification**, not a substantive surprise.

---

## §0 — Conventions

Inherit from [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) §0 (lattice Λ = ℤ³ undefined-boundary, flux field J : Λ × ℕ → ℝ³, state field s : Λ × ℕ → {−1, 0, +1}, latency field ℒ : Λ → [0, 1), 18-point Laplacian L_18, lattice momentum k̂_μ = 2 sin(k_μ/2), Brillouin zone BZ³ = [−π, π]³, lattice CFL constant C² = 1/3). Add: the **canonical toggle set** is the engine's default toggle configuration at the time of `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` (specifically the 11 toggles ON + `dual_substrate`/`weak_transmutation` OFF set explicitly enumerated there; will be re-snapshotted at Arc C2 P3 pre-reg lock).

---

## §1 — The boundary-theorem statement (this doc's content)

> **Spin-2 boundary theorem, free-theory + canonical-toggle scope.** Under FTD axioms 1-5 (`SPEC_FTD.md`), the calibration declarations (`a_phys ≡ ℓ_P`, `K_B = m_e`, `t_phys = √3 ℓ_P/c`), and the non-site-local observable algebra of `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4 frozen catalog, the FTD substrate's connected two-point correlator in the transverse-traceless rank-2 sector contains no gapless helicity-±2 pole. Specifically:
>
> **(C2-1) Linear spectrum.** The substrate's linearized dynamics support two independent propagating sectors: (a) the flux-field vector sector — after Gauss projection, the J-field carries 2 transverse spin-1 modes per wavevector k, with dispersion `ω(k) = C · ω_L(k)`; (b) the latency-field scalar sector — the latency `ℒ` is a quasi-static scalar satisfying the discrete Poisson equation `∇²_L ℒ = 4πG ρ_mass` per `SPEC_FTD_LAGRANGIAN.md` §4.2 [THEOREM], with no independent propagating dynamics in the free-theory limit. Total propagating mode count per k = 2 (transverse vector). No rank-2 propagating mode.
>
> **(C2-2) J-bilinear non-separability.** The connected two-point correlator of the symmetric traceless rank-2 J-bilinear `O_ij = J_iJ_j − ⅓δ_ij|J|²`, projected onto the transverse-traceless (helicity-±2) subspace, is a bubble-integral convolution of J-propagators with branch-cut analytic structure — no isolated pole. Rigorous derivation: [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md). Empirical validation: FTD-0193 measured TT correlator at L=64 identical to spin-1 control at 11/12 k-points to 7 significant digits.
>
> **(C2-3) No substrate-derived emergent graviton in §4 catalog.** Since (i) the only candidate non-site-local rank-2 observables in the §4 frozen catalog are J-bilinears or J-derivative bilinears (`O_ij`, `Õ_ij` per `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` §5; see uniqueness argument in [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) §2.2), and (ii) per (C2-2) these observables produce no isolated pole in the TT channel, there is no substrate-derived emergent graviton within the §4 catalog. Extension outside the §4 catalog (e.g., to finite-trace `s_m` variation per Doctrine §12 candidate principles) is Arc C1 territory and is `[OPEN]` if pursued.
>
> **(C2-4) Full nonlinear GR is matched, not derived.** The metric perturbation `h_μν` enters the FTD gravity content as Conjecture 10.1 per FTD-0189 [AUDIT FINDING] (2026-05-21) — *posited*, not substrate-constructed. The Deser-bootstrap chain `(h_μν posited) → (linearized Einstein equations [SELECTION/CONDITIONAL]) → (Lovelock-completion [SELECTION/CONDITIONAL])` documented in [`../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md`](../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md) (EFE-6/8/9 retagged 2026-05-24 per FTD-0189 ripple) recovers full GR as effective-theory matching of imported scaffold. Gravitational-wave dynamics, perihelion precession, light bending, and other strong-field GR phenomena are matched via this chain; they are not substrate-derived from FTD axioms 1-5.
>
> **Net statement.** The substrate-derivable gravity content within the §4 catalog at free-theory + Gauss-only level is **scalar (latency ℒ, via Phase G Poisson, [THEOREM]) + transverse vector (J spin-1, propagating, [THEOREM])**. Helicity-±2 is forbidden by (C2-3) within the catalog at this level. Full nonlinear GR enters by (C2-4) as effective-theory matching of imported h_μν, conditional on Conjecture 10.1 (which FTD-0193 [CLOSED NEGATIVE] establishes as falsified in the probed regime for substrate emergence).

---

## §2 — Setup: substrate field content + canonical toggle set

### §2.1 — Field content

Per `SPEC_FTD_LAGRANGIAN.md` axiom 1 (= `SPEC_FTD.md` axiom 1, undefined-boundary cubic lattice), each vertex `v ∈ Λ` carries:

1. **Flux field** `J(v, t) ∈ ℝ³` — three real-valued components per voxel per discrete tick (vector field).
2. **State field** `s(v, t) ∈ {−1, 0, +1}` — ternary, per voxel per tick.
3. **Latency field** `ℒ(v) ∈ [0, 1)` — scalar, quasi-static (changes on slower timescales than J).

The candidate "rank-2 substrate content" is built from these three fields. No fundamental rank-2 field is built into the substrate by axiom; any rank-2 observable must be constructed from bilinears, derivative bilinears, or other higher-rank combinations of J/s/ℒ. **Tag: [THEOREM]** — direct restatement of SPEC §1 axiom 1.

### §2.2 — Canonical toggle set (snapshot reference)

The canonical toggle set at the time of `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` lock (commit `bb354b6`, 2026-05-22) is the engine's default with 11 toggles ON, `dual_substrate` + `weak_transmutation` OFF. This is the regime in which FTD-0193 measured the [CLOSED NEGATIVE] verdict. Arc C2's boundary theorem statement at P3 pre-reg lock will re-snapshot this set explicitly; the present derivation uses it as inherited.

**Free-theory limit (used in §3, §4):** s ≡ 0, no state-flux coupling, no Gauss constraint applied (or Gauss applied as the only interaction), no Langevin noise, no manifestation thresholds active. This is the regime in which the [THEOREM]-grade results of §3 + §4 are rigorous.

**Canonical-toggle-set extension (used in §5, §6):** state-flux coupling ON, Gauss ON, manifestation thresholds active, velocity coupling ON, evaporation ON, but ground state has J ≡ 0 perturbed by small fluctuations. This is the regime in which FTD-0193 measured. The extension carries [SMC] tag per the structural argument + empirical validation.

---

## §3 — Clause (C2-1): linear spectrum

### §3.1 — J-sector spectrum (2 spin-1 transverse modes)

Per [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) §1.1 + §1.2: the linearized lattice wave equation `Δ_t² J_a = C² L_18 J_a` with Gauss constraint `∇_L · J = ρ` (vacuum: ρ = 0) gives:

- **3 J components per voxel.** Pre-constraint dof count = 3.
- **Gauss removes 1 dof** (longitudinal mode is gauge, non-propagating).
- **2 transverse modes propagate**, both with dispersion `ω(k) = C · ω_L(k)`. These are the spin-1 modes (helicity ±1 under the SO(2) little group of k).

**Tag: [THEOREM]** — direct lattice gauge-theory spectrum analysis. Empirically validated by FTD-0193 §2 (spin-1 control returned 12/12 k-points at 0.02-3% precision).

### §3.2 — ℒ-sector spectrum (1 scalar quasi-static mode)

Per `SPEC_FTD_LAGRANGIAN.md` §4.2 [THEOREM]: the latency field satisfies the discrete Poisson equation `∇²_L ℒ = 4πG ρ_mass` with `ρ_mass = K_B · n` (number density of manifested sites). In the free-theory vacuum (s ≡ 0 ⟹ n = 0), the equation gives `∇²_L ℒ = 0`, whose solutions are harmonic functions on the lattice. With periodic BC (engine convention) and mean subtraction (per `engine/src/poisson_solvers.cpp:206-220`), the unique solution is `ℒ ≡ 0`. With a localized mass source, ℒ falls off as `1/(4π·r)` at large r per Phase G [THEOREM] (FTD-0004 + `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.1).

**Critical point for spectrum analysis:** the latency Poisson equation is **first-order in time** (or zeroth-order; ℒ is quasi-static). It has no `Δ_t²` term. Therefore ℒ has **no propagating dispersion** — it is a constrained scalar that responds instantaneously (in the SOR-equilibrated sense, after enough ticks for convergence) to the source. The "spectrum" of ℒ has only one mode per wavevector k: the static Green's function response `ℒ(k) = (4πG / k̂²) · ρ_mass(k)`. No propagating spin-0 graviton.

**Tag: [THEOREM]** — direct from SPEC §4.2 [THEOREM] + Phase G [THEOREM] + standard analysis of static Poisson on a periodic lattice.

### §3.3 — Combined spectrum count

Total propagating modes per wavevector k under the free theory + Gauss constraint:

| Sector | Field | dof/voxel pre-constraint | Constraints removed | Propagating dof/k |
|---|---|---|---|---|
| J vector | J ∈ ℝ³ | 3 | 1 (Gauss longitudinal gauge) | 2 (transverse spin-1) |
| ℒ scalar | ℒ ∈ [0, 1) | 1 | (none; quasi-static) | 0 (non-propagating) |
| s ternary | s ∈ {−1, 0, +1} | 1 | (none; discrete state, not a propagating field) | 0 (non-propagating) |

**Total propagating modes per k = 2** (two transverse spin-1 modes of J).

**No rank-2 (spin-2) propagating mode exists in the linear spectrum.** This is the (C2-1) statement made rigorous. **Tag: [THEOREM]** at free-theory + Gauss-only level.

### §3.4 — Extension to canonical toggle set

Interactions in the canonical toggle set (state-flux coupling, velocity coupling, manifestation thresholds, evaporation, Langevin):

- None of them introduce a new fundamental field beyond {J, s, ℒ}.
- None of them changes J's kinematic representation (still 3-vector).
- None of them changes ℒ's first-order constrained character (still quasi-static).
- Interactions modify the J self-energy (renormalize the dispersion ω(k)), shift threshold-crossing dynamics, and add Langevin noise — but do not produce a new propagating mode in the rank-2 channel.

**Tag: [SMC]** — structural argument (no new fundamental field) + FTD-0193 empirical validation (spin-1 control at 12/12 k-points still recovers J-dispersion under canonical toggles).

---

## §4 — Clause (C2-2): J-bilinear non-separability

Per [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) §3:

1. **Wick contraction** of `⟨O_ij(x) O_kl(y)⟩_c` factors into a sum of `⟨J J⟩⟨J J⟩` products (bubble diagram). [THEOREM]
2. **Momentum-space bubble integral** `Π(k, ω) = ∫(d⁴p) G_J(p) G_J(k − p)` is the standard two-particle convolution. [THEOREM]
3. **Analytic structure**: `Π(k, ω)` has a branch cut starting at the two-particle threshold `|ω|² = 4 C² ω_L²(k/2)`, with **no isolated pole anywhere in the analytic plane**. [THEOREM]
4. **TT projection**: contracting `P^{TT}_{ijkl}(k)` with the bilinear correlator preserves the branch-cut-only structure. [THEOREM]
5. **Empirical validation**: FTD-0193 §4 measured the TT correlator at L=64 identical to spin-1 control ω at 11/12 k-points to 7 sig figs. [VERIFIED]

The full derivation is in [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md); not reproduced here. The net for (C2-2): **the J-bilinear's TT projection has no isolated pole** — the spin-2 channel is "continuum, no separable mode."

**Tag for (C2-2):** [THEOREM] at free-theory + Gauss-only level; [SMC] for canonical toggle set with FTD-0193 empirical validation; [OPEN] for L > 64 / non-§4-catalog observables.

---

## §5 — Clause (C2-3): no substrate-derived graviton in §4 catalog

### §5.1 — Uniqueness of the J-bilinear candidate

Per [`DERIV_J_BILINEAR_NO_SPIN2_POLE.md`](DERIV_J_BILINEAR_NO_SPIN2_POLE.md) §2.2: the only candidate non-site-local rank-2 observables in the §4 frozen catalog are J-bilinears (`O_ij`) or J-derivative bilinears (`Õ_ij = [(∂J)(∂J)]_TT`). This is a direct group-theoretic consequence: from a 3-vector field J without introducing new fundamental fields, the only local rank-2 observables built without higher-derivative or non-local operators are bilinears in J or its derivatives.

**Tag: [THEOREM]** — group-theoretic decomposition of `Sym²(V) − Tr(V ⊗ V)/3` for V the 3-vector representation, restricted to the local-bilinear sector of the §4 frozen catalog.

### §5.2 — (C2-3) follows from (C2-1) + (C2-2) + §5.1 uniqueness

The argument:

- By (C2-1) §3, no rank-2 propagating mode exists in the linear spectrum.
- By §5.1 uniqueness, the only candidate emergent rank-2 observables in the §4 catalog are J-bilinears.
- By (C2-2) §4, J-bilinears' TT projection has no isolated pole — only continuum.
- Therefore: no substrate-derived emergent graviton exists in the §4 catalog at free-theory + Gauss-only level.

**Tag: [THEOREM]** at free-theory + Gauss-only level (composition of [THEOREM]-grade C2-1 + C2-2 + §5.1).
**Tag: [SMC]** at canonical-toggle-set level (inherits C2-2 [SMC] for interactions; structural argument that interactions don't introduce new fundamental fields per §3.4).
**Tag: [OPEN]** for non-§4-catalog observables — finite-trace `s_m` variation, graph spectral curvature, finite adjacency deformation (Doctrine §12 candidate principles). Arc C1 territory if pursued.

### §5.3 — What this clause does NOT claim

(C2-3) **does not claim** "spin-2 emergence is impossible on FTD's substrate." It claims: spin-2 emergence is impossible **within the §4 frozen observable catalog** at the probed regime (free-theory + Gauss-only rigorously, canonical-toggle-set SMC, L ≤ 64 empirically). Extension beyond the §4 catalog (e.g., to non-bilinear observables, or to L > 64 with new instrumentation per Arc C1 GPU port) could change the verdict; this is the Arc C1 territory documented as parallel-permitted in plan v2.

---

## §6 — Clause (C2-4): full GR via Deser bootstrap of posited h_μν

### §6.1 — The Deser-bootstrap chain (per FTD-0189 + AUDIT §3 ripple)

Per [`../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md`](../03_derivations/DERIV_EINSTEIN_FIELD_EQUATIONS.md) Steps 1-5 (retagged 2026-05-24 per FTD-0189 ripple):

- **Step 1 (Metric emergence):** `g_μν = η_μν + h_μν(ℒ)` — h_μν is *posited*, not substrate-constructed. Per FTD-0189: Conjecture 10.1.
- **Step 2 (Stress-energy via Noether):** [THEOREM]; survives FTD-0189 unchanged (no h_μν dependence).
- **Step 3 (Linearized Einstein eqs):** [SELECTION/CONDITIONAL] (retagged from [THEOREM] per FTD-0189; cites Theorem 14.1 of `DERIV_RELATIVITY_DERIVATION.md` which was also retagged).
- **Step 4 (Newton's G from α_G hierarchy):** [THEOREM] via α^20 derivation per `DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md`; survives FTD-0189.
- **Step 5 (Nonlinear completion via Lovelock):** [SELECTION — conditional on Step 3].

**Net:** the Deser-bootstrap chain recovers full Einstein equations as `G_μν = 8πG/c⁴ · T_μν`, but **conditional on Conjecture 10.1** (h_μν posited). This is effective-theory matching of imported scaffold, not substrate emergence.

### §6.2 — Conjecture 10.1 status: [CLOSED NEGATIVE] for substrate emergence (FTD-0193)

Per FTD-0193 [CLOSED NEGATIVE per Outcome B] at L ∈ {32, 64} on dual substrate with J-bilinears (flux-quadrupole, stress): no gapless helicity-±2 pole. Conjecture 10.1's "h_μν exists in substrate as an emergent rank-2 mode" is empirically falsified in the probed regime. The Deser-bootstrap chain's input (h_μν) is therefore **NOT** substrate-derivable within the probed regime / §4 catalog; it is genuinely imported.

### §6.3 — Net for (C2-4)

Full nonlinear GR (Einstein equations, gravitational waves, Mercury perihelion, light bending, GPS time corrections, Shapiro delay) is **matched, not derived**. The matching mechanism is the Deser bootstrap of POSITED h_μν per `DERIV_EINSTEIN_FIELD_EQUATIONS.md` (chain retagged [SELECTION/CONDITIONAL] per FTD-0189 ripple). The empirical agreement of FTD with GR observables at strong-field regime is via this matching, not via substrate emergence.

**Tag: [REFERENCE]** for this clause — the substantive work is already documented in `DERIV_EINSTEIN_FIELD_EQUATIONS.md` + FTD-0189 + FTD-0193 + `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §3. This boundary theorem cites those as established context; it does not re-derive the chain.

---

## §7 — Net boundary statement

**Substrate-derivable gravity content** at free-theory + Gauss-only + §4-catalog scope:

| Sector | Substrate-derivable? | Tag | Source |
|---|---|---|---|
| Scalar gravity (latency ℒ Poisson) | YES | [THEOREM] (FTD-0004 Phase G + SPEC §4.2) | `DERIV_NEWTON_FROM_SUBSTRATE.md` |
| Newton's 1/r tail | YES | [THEOREM] (Phase G + classical Glasser-Zucker asymptote) | `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.1 |
| Schwarzschild g_00 form | YES (modulo clock hypothesis) | [THEOREM modulo clock hypothesis] (SPEC §4.3 Born-Infeld) | `SPEC_FTD_LAGRANGIAN.md` §4.3 + Arc B P2 verdict pending |
| Coupling G_N (engine-internal) | YES (operational, not physical-G_N identification) | [DERIVED] (per AUDIT §3.5 gap (iv) closure) | `engine/src/poisson_solvers.cpp:190-228` |
| α_G(e,e) hierarchy prediction | YES (0.38% match) | [SMC] (inherits FTD-0015) | FTD-0131 §2.1 |
| Vector gravity (transverse spin-1) | YES (propagating modes) | [THEOREM] (linear lattice wave equation per §3.1) | `DERIV_J_BILINEAR_NO_SPIN2_POLE.md` §1.2 |
| **Spin-2 emergent graviton** | **NO** (within §4 catalog at free-theory + Gauss-only rigorously; SMC for canonical toggles per FTD-0193) | **[THEOREM]** at free-theory level for (C2-3) | This document §5 |
| Full nonlinear GR (Einstein eqs, GW) | NO (matched, not derived) | [SELECTION — conditional on Conjecture 10.1] | `DERIV_EINSTEIN_FIELD_EQUATIONS.md` retagged 2026-05-24 |
| Mercury perihelion, light bending, GPS | NO (matched via Deser bootstrap) | [SELECTION — inherited] | `DERIV_EINSTEIN_FIELD_EQUATIONS.md` Step 3 chain |

**Wilsonian reframe placement:** this boundary theorem caps the upper end of the substrate-derived scaling law. The scaling law from discrete floor (ℓ_P, FTD-0041 ESTABLISHED) through scalar gravity (Phase G ESTABLISHED) and vector gravity (J spin-1 modes ESTABLISHED) reaches the Schwarzschild scalar sector (FTD-0131 [DERIVED modulo clock hypothesis]). Above that, gravity content is matched via Deser-bootstrap of posited h_μν per (C2-4). The boundary is **precisely**: substrate-derivation reaches scalar + vector; emergent spin-2 is forbidden in §4 catalog; full GR is matched.

This serves CLAUDE.md project-goal clause 2 ("rigorously establish what we cannot derive") at theorem-grade for the free-theory part and at SMC + empirical-validation grade for the canonical-toggle extension.

---

## §8 — Arc B P2 verdict matrix (conditional scaling-law branch)

The boundary statement's "scalar gravity sector" depends on Arc B P2 verdict (clock-hypothesis substrate-derivation attempt; pre-reg `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md` authored 2026-05-24, hash-lock pending). Two branches:

**Branch A: Arc B P2 closes FOUND** (clock hypothesis substrate-derived).
- SPEC §4.3 promoted to fully [THEOREM] (no qualifier).
- Schwarzschild proper time `dτ/dt = √(f − v²/f)` is fully substrate-derived.
- The boundary statement's "scalar sector" extends to full Schwarzschild proper time.
- Net: substrate scaling reaches scalar Schwarzschild + transverse J spin-1; spin-2 + full nonlinear GR still excluded per (C2-3) + (C2-4).

**Branch B: Arc B P2 closes CLOSED-NEGATIVE** (clock hypothesis tagged as [AXIOM]).
- SPEC §4.3 tag remains [THEOREM, conditional on clock hypothesis AXIOM].
- The boundary statement's "scalar sector" stops at "g_00 form via Phase G + clock-hypothesis AXIOM".
- Net: substrate scaling reaches scalar gravity via Phase G + AXIOM-tier clock-hypothesis + transverse J spin-1; spin-2 + full nonlinear GR still excluded per (C2-3) + (C2-4).

**Either branch is compatible with this document's Outcome A (FOUND) tag for (C2-3).** The (C2-3) clause does NOT depend on the clock hypothesis; it depends only on the linear spectrum (C2-1) + bilinear non-separability (C2-2) + §4-catalog uniqueness (§5.1).

---

## §9 — Tag summary

| Clause | Content | Tag (free-theory + Gauss only) | Tag (canonical toggle set) | Tag (general / extended) |
|---|---|---|---|---|
| C2-1 | Linear spectrum: 2 transverse spin-1 + 1 quasi-static scalar; no rank-2 propagating mode | **[THEOREM]** | **[SMC]** (interactions don't add new fundamental field per §3.4) | OPEN at L > 64 / non-canonical toggles |
| C2-2 | J-bilinear TT projection has no isolated pole | **[THEOREM]** (per DERIV_J_BILINEAR_NO_SPIN2_POLE.md §3) | **[SMC]** (per §5 + FTD-0193 empirical 11/12 k-points L=64) | OPEN at L > 64 / non-bilinear observables |
| C2-3 | No substrate-derived emergent graviton in §4 catalog | **[THEOREM]** (composition C2-1 + C2-2 + §5.1 uniqueness) | **[SMC]** (inherits C2-2 [SMC]) | OPEN for non-§4-catalog observables (Arc C1) |
| C2-4 | Full nonlinear GR via Deser bootstrap of posited h_μν | **[REFERENCE]** — `DERIV_EINSTEIN_FIELD_EQUATIONS.md` retagged 2026-05-24 per FTD-0189; chain is [SELECTION/CONDITIONAL] not [THEOREM] | inherited | inherited |

**Net tag for the boundary theorem statement** (at free-theory + Gauss-only + §4-catalog scope): **[THEOREM]**. At canonical-toggle-set scope: **[SMC]** with FTD-0193 empirical floor. At general / extended scope: **[OPEN]** pending Arc C1 work.

**No new spine theorem.** Spine count unchanged. This is a boundary theorem characterizing where substrate derivation stops and effective-theory matching takes over; it adds rigor to existing claims, not new theorems.

---

## §10 — Honest limits + scope hedges (per F9/F10 discipline)

- **F9 mitigation (theorem too easy / hides assumptions):** the two-tag structure (free-theory [THEOREM] vs canonical-toggle [SMC]) makes the conditional structure explicit. The free-theory result IS easy (bubble integral with no pole — standard QFT); the canonical-toggle extension is structural argument + empirical validation, not closed-form proof.
- **F10 mitigation (rigidity-gap licensing):** [THEOREM] for (C2-3) at free-theory level is recognition of a structural fact (linear spectrum + bilinear analytic structure forbid spin-2 pole in §4 catalog); it does NOT fix the substrate-derived-gravity question more broadly. Substrate gravity content remains scalar (FTD-0131 modulo clock hypothesis) + vector (J spin-1); the upper end remains effective-theory matching per (C2-4). The boundary is mapped honestly, not enlarged.
- **Scope hedge 1:** the §4 frozen catalog is inherited from `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`. Any observable outside this catalog (e.g., Doctrine §12 candidate principles) is outside the boundary theorem's scope. Arc C1 (extended search) is the work that would widen this scope.
- **Scope hedge 2:** L ∈ {32, 64} is the empirically probed regime. L > 64 is `[OPEN]` empirically (FTD-0193 §5 documented L=128 deferral for GPU-port engineering, not methodological). Theorem's "for all L" claim relies on the L-independent analytic structure of the free-theory bubble integral (§4); empirical extension to L=128 would be Arc C1 GPU-port work.
- **Scope hedge 3:** Arc B P2 verdict determines whether the scalar gravity sector reaches full Schwarzschild proper time ([THEOREM]) or stops at "g_00 form modulo clock-hypothesis AXIOM." Per §8, both branches are compatible with (C2-3) Outcome A.
- **Scope hedge 4:** (C2-4) is [REFERENCE], not derivation. The Deser-bootstrap chain's [SELECTION/CONDITIONAL] tags per FTD-0189 ripple are load-bearing input; if those tags change (e.g., a future audit reverses the FTD-0189 retags), (C2-4) framing would need to update.

---

## §11 — Implications for Arc C2 P3 pre-reg + plan v2

**For Arc C2 P3 pre-registration:**
- This document provides the load-bearing derivation chain. The P3 pre-reg should:
  - State the boundary theorem as §1 here (with §2.2 canonical toggle set re-snapshotted at lock time)
  - State the axioms as `SPEC_FTD.md` 1-5 + the calibration declarations + the §4 frozen catalog
  - Lock the proof-structure preview as Sections §3-§6 here, with explicit dual-tag (free-theory [THEOREM] vs canonical-toggle [SMC])
  - Three outcomes per SCOPE §5: FOUND (boundary theorem holds), CLOSED-NEGATIVE (some §4-catalog observable escapes the closure), UNDERDETERMINED (additional principle needed)
  - Falsifiers F-a through F-j per SCOPE §6
  - Conditional dependency on Arc B P2 verdict per §8

**For plan v2:**
- Arc C2 P1 substantive derivation work is now complete via this document + companion `DERIV_J_BILINEAR_NO_SPIN2_POLE.md`. Plan v2 Arc C2 P1 marked CLOSED.
- Arc C2 P2 (instrumentation): NONE — boundary theorem is desk work per SCOPE §1.
- Arc C2 P3 (pre-reg lock): READY TO AUTHOR after Arc B P2 verdict in hand, OR with both branches per §8. Requires hash-lock (git ops; explicit user direction needed per CLAUDE.md commit policy).
- Arc C2 P4 (closure attempt): would proceed against P3-locked pre-reg.
- Arc C2 P5 (result-doc): would land as `FOUND_SPIN2_BOUNDARY_THEOREM.md` (Outcome A) or `AUDIT_SPIN2_BOUNDARY_THEOREM_*.md`.

**Plan v2 horizon update under this completion:** Arc C2 P1 was estimated at 3-5 weeks; landed in 1 session (~2 substantial documents authored). Arc C2 net horizon shrinks from 4-6 wk to ~2-3 wk (P3 pre-reg + P4 attempt + P5 result-doc, dominated by adversarial review checkpoint and P4 careful checking against falsifiers).

---

## §12 — What this document does NOT claim

- **NOT a proof that gravity is impossible on FTD's substrate.** Scalar gravity (Phase G) and vector gravity (J spin-1 modes) are substrate-derived; helicity-±2 emergent graviton is forbidden in §4 catalog; full GR is matched via Deser bootstrap of posited h_μν.
- **NOT a closure of Arc C2.** This is the consolidated Arc C2 P1 derivation; the closure requires Arc C2 P3 pre-reg + P4 closure attempt against the locked falsifiers.
- **NOT a refutation of Arc C1 (extended spin-2 search).** Arc C1 explores non-§4-catalog observables (Doctrine §12 candidate principles); if Arc C1 finds an emergent spin-2 mode outside the catalog, the boundary theorem's scope would update.
- **NOT a new spine theorem.** Spine count unchanged; this is subsidiary to the boundary-theorem program.
- **NOT a tag promotion of any existing LEDGER claim.** Existing tags stand; this document characterizes the **structural relationship** between substrate-derivable and import-required gravity content, not new derivations.

---

## §13 — Single-line summary

**The FTD substrate hosts scalar gravity (latency ℒ Poisson per Phase G [THEOREM] + SPEC §4.2 [THEOREM]) and vector gravity (transverse J spin-1 modes per linear lattice wave equation [THEOREM]); helicity-±2 emergent graviton is forbidden in the §4 frozen observable catalog at free-theory + Gauss-only level [THEOREM] and at canonical-toggle level [SMC] with FTD-0193 empirical validation at 11/12 k-points L≤64; full nonlinear GR (Einstein equations, gravitational waves, Mercury perihelion, light bending) is matched via the Deser bootstrap of POSITED h_μν per Conjecture 10.1 [CLOSED NEGATIVE per FTD-0193 for substrate emergence in probed regime] with the bootstrap chain [SELECTION/CONDITIONAL] per FTD-0189 ripple — this is the precise statement of "where substrate-derivation stops and effective-theory matching takes over" for FTD's gravity content, serving CLAUDE.md project-goal clause 2 ("rigorously establish what we cannot derive") at theorem-grade for the free-theory part and SMC + empirical-validation grade for the canonical-toggle extension, with Arc B P2 verdict determining whether the scalar sector reaches full Schwarzschild proper time ([THEOREM]) or stops at "g_00 modulo clock-hypothesis AXIOM" — either branch compatible with the (C2-3) Outcome A result.**
