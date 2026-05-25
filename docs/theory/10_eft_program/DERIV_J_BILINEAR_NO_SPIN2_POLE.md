# DERIV — J-bilinear two-point correlator has no separable helicity-±2 pole (free-theory + tree-level)

**Tag:** `[THEOREM at free-theory level + tree-level interactions in the canonical Gauss-only toggle subset]` + `[STRONGLY MOTIVATED CONJECTURE for full canonical toggle set with all interactions]` + `[OPEN for general non-canonical toggle configurations]`. Load-bearing step 3 of the Arc C2 spin-2 boundary theorem program per [`SCOPE_SPIN2_BOUNDARY_THEOREM.md`](SCOPE_SPIN2_BOUNDARY_THEOREM.md) §3.
**Date:** 2026-05-24 (Arc C2 P1 deliverable a, Wilsonian-reframe plan v2)
**LEDGER row reservation:** provisional, confirm against `../07_assessment/LEDGER.md` at hash-lock; this derivation is referenced by the future `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`.
**Companion docs:**
- [`SCOPE_SPIN2_BOUNDARY_THEOREM.md`](SCOPE_SPIN2_BOUNDARY_THEOREM.md) — Arc C2 scoping (parent doc; this derivation is its §3 step 3)
- [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](REPORT_GRAVITON_SUBSTRATE_MODE.md) — FTD-0193 empirical validation (11/12 k-points identical to spin-1 control at L=64)
- [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](PREREG_GRAVITON_SUBSTRATE_MODE_v2.md) — the pre-registration FTD-0193 closed against
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.1 (lattice derivatives) + §3.3 (action) + §6.1 (lattice Green's function `G_L(k) = 1/k̂²`)
- [`../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) — Phase G scalar gravity (the substrate-derivable content above which Arc C2 establishes the boundary)

**What this document does:** rigorously establishes — at the free-theory + Gauss-only-interaction level — that the connected two-point correlator of the symmetric traceless rank-2 J-bilinear contains **no separable helicity-±2 pole**; the transverse-traceless projection produces a two-particle continuum (bubble-diagram structure) carrying the spin-1 J-mode dispersion through the bilinear product. This makes rigorous the "spin-1 fields produce spin-0 ⊕ spin-1 ⊕ spin-2 kinematically but only continuum-level (not pole-level) in the spin-2 channel" claim flagged as hand-waved in the parent scope memo.

**What this document does NOT do:** prove no-pole for the full interacting canonical toggle set (state-flux coupling, velocity coupling, evaporation, manifestation threshold-crossing nonlinearities). It conjectures, motivated by FTD-0193's empirical validation across 11/12 k-points and by the structural argument that interactions shift continuum but do not create separable poles, that the full canonical toggle set preserves the free-theory result. That conjecture is `[STRONGLY MOTIVATED CONJECTURE]`. Full-interaction extension is queued as separate work.

---

## §0 — Notation and conventions

- Lattice: `Λ = ℤ³` with no defined boundary per `SPEC_FTD.md` axiom 1; lattice spacing `a` (set to 1 in lattice units; calibrated `a ≡ ℓ_P` per FTD-0041 for SI translation).
- Flux field: `J : Λ × ℕ → ℝ³` — three real-valued components per voxel per discrete tick `t`.
- Lattice forward differences: `(Δ_μ J)(v,t) = J(v + ê_μ, t) - J(v, t)` for spatial directions μ ∈ {1,2,3}; `(Δ_t J)(v,t) = J(v, t+1) - J(v, t)` for time. Per SPEC §3.1.
- 18-point isotropic Laplacian: `L_18` per SPEC §4.4 (1st + 2nd shell of Moore neighborhood, properly weighted for O_h isotropy).
- Lattice wave equation (linearized about J ≡ 0, with no state-flux coupling and no constraint): `(Δ_t² J_a) = C² L_18 J_a` for each Cartesian component a ∈ {1,2,3}, with `C² = 1/3` (lattice CFL stability constant).
- Brillouin zone: `BZ³ = [-π, π]³` for k-modes; lattice-Fourier convention `J_a(k, ω) = Σ_{v,t} J_a(v, t) exp(-i(k·v + ωt))`.
- Lattice Green's function: per SPEC §6.1, `G_L(k) = 1 / k̂²` with `k̂_μ = 2 sin(k_μ/2)` (the "lattice momentum"), and similarly for the 18-point operator: `G_L^{18}(k) = 1/ω̂²(k)` where `ω̂(k)` is the L_18 spectrum (per SPEC §3.1).
- Helicity-h subspace: for a transverse vector mode at wavevector k, helicity ±1 corresponds to the two transverse polarizations rotating under the SO(2) little group of k. For a rank-2 symmetric traceless tensor at k, helicity ±2 corresponds to the two TT modes; helicity ±1 corresponds to the "vector" subspace (one transverse index, one longitudinal-like contraction); helicity 0 corresponds to the scalar trace + longitudinal subspaces.

---

## §1 — Setup: free-theory J-field spectrum

### §1.1 — Linear lattice wave equation

In the free-theory limit (state field `s ≡ 0`, no Gauss constraint applied, no Langevin noise, no velocity coupling), the flux field obeys

$$ \Delta_t^2 J_a(v, t) = C^2\, L_{18}\, J_a(v, t), \qquad a \in \{1, 2, 3\}, $$

per `SPEC_FTD_LAGRANGIAN.md` §6.1 (the matter-Lagrangian weak-field expansion). The three Cartesian components decouple at this level. Fourier transforming:

$$ -\omega^2\, J_a(k, \omega) = -C^2\, \omega_L^2(k)\, J_a(k, \omega) $$

with `ω_L²(k) = sum over 18-point stencil coefficients × (1 - cos(k_μ + ...))`-style expression (the exact form depending on the 18-point weights; see SPEC §4.4). The dispersion relation is `ω(k) = C · ω_L(k)`; for small `|k|·a`, this reduces to `ω ≈ C|k|` (the continuum massless dispersion). **Tag: [THEOREM]** — direct linear-wave-equation manipulation per standard lattice field theory.

### §1.2 — Mode count per wavevector

For each `k ∈ BZ³`, the three components `J_a(k, ω)` give 3 polarization degrees of freedom. **Before Gauss projection:** all 3 components propagate at the same dispersion `ω(k) = C · ω_L(k)`.

**After Gauss projection** (per `SPEC_FTD_LAGRANGIAN.md` §3.3 + §3.6 term 4): the constraint `∇_L · J = ρ` (where ρ is the substrate state-field density, vanishing in the free-theory vacuum) projects out the longitudinal component. Two transverse modes remain: `J_T^{(1)}(k)` and `J_T^{(2)}(k)`. Both propagate at `ω = C · ω_L(k)`. The longitudinal mode is gauge (non-propagating).

**Mode count per k:** 1 longitudinal (gauge, non-propagating) + 2 transverse (propagating, spin-1 each in the little-group SO(2) representation). This matches FTD-0193 §2 spin-1 control finding: the engine's spin-1 channel returns the transverse-J dispersion at 12/12 k-points to 0.02-3% precision. **Tag: [THEOREM]** — standard lattice gauge-theory spectrum analysis applied to the FTD vector flux + Gauss constraint.

### §1.3 — Free-theory J-propagator

The connected two-point function in the free theory is

$$ \langle J_a(k, \omega)\, J_b(-k, -\omega) \rangle^{(0)}_c = \frac{i\, P^T_{ab}(k)}{\omega^2 - C^2\, \omega_L^2(k) + i\epsilon} $$

where `P^T_{ab}(k) = δ_{ab} - k_a k_b / |k|²` is the transverse projector (longitudinal mode projected out by Gauss constraint; the i-epsilon prescription gives the Feynman propagator). **Tag: [THEOREM]** — direct Gaussian-integral evaluation of the free-theory partition function in Euclidean / Wick-rotated form, standard lattice QFT.

---

## §2 — The rank-2 bilinear observable

### §2.1 — Definition

The symmetric traceless rank-2 J-bilinear is

$$ O_{ij}(x) := J_i(x) J_j(x) - \tfrac{1}{3}\delta_{ij} |J(x)|^2 $$

per `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` §5 observable (i') (flux-quadrupole). The trace-removed structure ensures `δ^{ij} O_{ij} = 0`, isolating the candidate spin-2 channel from any scalar (trace) component.

The "stress" observable

$$ \tilde O_{ij}(x) := \bigl[(\partial_i J_a)(\partial_j J_a)\bigr]_{TT} $$

per PREREG_GRAVITON_SUBSTRATE_MODE_v2.md §5 observable (ii) is similar in structure (derivative bilinears in J, projected TT). The analysis below applies to either; for concreteness we use `O_{ij}`.

### §2.2 — Why a bilinear is the candidate spin-2 observable

A rank-2 tensor field that could carry helicity ±2 must transform as the symmetric traceless representation of the rotation group at fixed k. From a 3-vector J-field, the only local rank-2 observables built without introducing new fundamental fields are bilinears `J ⊗ J` (or their derivative analogs `∂J ⊗ ∂J`). The symmetric traceless part of `J ⊗ J` decomposes under SO(2) (the little group at fixed k) into helicity {0, ±1, ±2}; the TT projection isolates the ±2 sector. **This is the *only* candidate non-site-local rank-2 observable in the §4 frozen catalog** (per `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` §4) that could carry spin-2 dynamics if a spin-2 pole existed. **Tag: [THEOREM]** — direct group-theoretic decomposition of `Sym²(V) − Tr(V ⊗ V)/3` for V the 3-vector representation.

---

## §3 — Two-point correlator of the bilinear

### §3.1 — Wick contraction (free theory)

In the free theory (Gaussian J-field), the connected two-point function of `O_ij` factorizes via Wick's theorem:

$$ \begin{aligned}
\langle O_{ij}(x)\, O_{kl}(y) \rangle_c &= \langle J_i J_j J_k J_l \rangle_c \;\;(\text{minus trace pieces}) \\
&= \langle J_i(x) J_k(y) \rangle \langle J_j(x) J_l(y) \rangle + \langle J_i(x) J_l(y) \rangle \langle J_j(x) J_k(y) \rangle \\
&\quad - \tfrac{1}{3}\delta_{ij}(\ldots) - \tfrac{1}{3}\delta_{kl}(\ldots) + \tfrac{1}{9}\delta_{ij}\delta_{kl}(\ldots)
\end{aligned} $$

where the disconnected piece `⟨J_i J_j⟩⟨J_k J_l⟩` cancels in the connected correlator. **Tag: [THEOREM]** — standard Wick contraction; the bilinear correlator is a sum of two products of J-propagators (a "bubble diagram" in QFT language).

### §3.2 — Momentum-space expression

Fourier transforming and using §1.3:

$$ \langle O_{ij}(k, \omega) O_{kl}(-k, -\omega) \rangle_c = \int \frac{d^4 p}{(2\pi)^4}\, [\text{tensor structure from }P^T] \cdot G_L^J(p) \cdot G_L^J(k - p) $$

where `G_L^J(p) = i / (ω_p² - C² ω_L²(p) + iε)` is the J-propagator from §1.3, the tensor structure is the contraction of two `P^T(p) ⊗ P^T(k-p)` projectors with the symmetric-traceless structure of `O_{ij}`, and the integration is over the loop momentum `p` (4-momentum including frequency, restricted to BZ³).

**The integrand is a convolution of two J-propagators**, each of which has a single-particle pole at `ω_p = ±C ω_L(p)` and `ω_{k-p} = ±C ω_L(k - p)`. The convolution integral is the standard "two-particle bubble" of QFT. **Tag: [THEOREM]** — direct momentum-space evaluation of §3.1's Wick contraction.

### §3.3 — The branch-cut / continuum structure

The bubble integral

$$ \Pi(k, \omega) := \int \frac{d^4 p}{(2\pi)^4}\, \frac{1}{(\omega_p² - C² ω_L²(p) + iε)(\omega_{k-p}² - C² ω_L²(k-p) + iε)} $$

has its analytic structure dictated by the two propagator poles. Standard analysis (textbook Peskin-Schroeder §10.2 for continuum; lattice analog straightforward via the same residue arguments at BZ-restricted poles):

- For `|ω|² < 4C² ω_L²(k/2)` — below the two-particle threshold — `Π(k, ω)` is real and smooth.
- For `|ω|² ≥ 4C² ω_L²(k/2)` — above threshold — `Π(k, ω)` develops an imaginary part: a **branch cut**, NOT a pole. The branch cut corresponds to the two-particle continuum (kinematic region where energy ω can be carried by two on-shell J-quanta at relative momentum sufficient for the kinematic constraint).
- **There is no isolated pole at any `(k, ω)`.** A pole would correspond to a single-quantum state (a propagating particle) in the spin-2 channel; the bubble structure gives only continuum.

**Tag: [THEOREM]** — standard QFT result for a bubble diagram from two propagating constituents; the lattice version preserves the result via finite-BZ residue calculus.

### §3.4 — TT projection

The TT projector at wavevector k is

$$ P^{TT}_{ijkl}(k) = \tfrac{1}{2}(P^T_{ik} P^T_{jl} + P^T_{il} P^T_{jk}) - \tfrac{1}{2} P^T_{ij} P^T_{kl} $$

(symmetric in (ij)↔(kl), traceless on each pair). Contracting `P^{TT}` with the bilinear correlator §3.2 yields the **TT-projected bilinear correlator**:

$$ \langle O_{ij} O_{kl} \rangle_c^{TT} = P^{TT}_{ijkl}(k) \cdot \Pi^{TT}(k, \omega) $$

where `Π^{TT}(k, ω)` is the scalar form factor (the bubble integral §3.3 with the appropriate tensor contractions). Per §3.3, `Π^{TT}` has only branch-cut structure — no isolated pole.

**Conclusion of §3 (the load-bearing theorem):** the free-theory two-point correlator of the rank-2 J-bilinear, projected onto the helicity-±2 (TT) subspace, contains no isolated pole. The TT spectral function is a two-particle continuum across the kinematically allowed region; the spin-2 channel is "continuum, no separable mode." **Tag: [THEOREM at free-theory level]**.

---

## §4 — FTD-0193 empirical confirmation

The free-theory result of §3 makes a sharp empirical prediction: when the engine measures `⟨O_{ij} O_{kl}⟩^{TT}` at wavevector k, the dominant frequency response should be **the spin-1 J-dispersion**, not an independent spin-2 mode. Specifically, the bubble integral's peak (the kinematic edge of the two-particle continuum) follows the spin-1 dispersion `ω = 2 C ω_L(k/2) ≈ C|k|` (in the long-wavelength limit).

**FTD-0193 measured this directly** (REPORT_GRAVITON_SUBSTRATE_MODE §4): at L=64, the flux-quadrupole TT operator's extracted ω is **identical to the spin-1 control ω to 7 significant digits at 11/12 k-points** (the lone exception lands in a neighboring FFT bin). The stress TT operator shows fixed two-particle beat frequencies (0.27, 0.54, 0.81, …, identical across [100] and [111] — textbook continuum). Both spin-2 channels are 7-9 orders of magnitude below the validated spin-1 control.

**Empirical match:** the engine's observed behavior is precisely what the §3 free-theory theorem predicts. The bilinear carries the spin-1 mode through. **Tag: [VERIFIED]** — FTD-0193 + REPORT_GRAVITON_SUBSTRATE_MODE.md §4 explicitly recorded.

---

## §5 — Extension to the canonical toggle set (with interactions)

### §5.1 — What interactions could change

The FTD canonical toggle set (per `engine/include/ftd/term_toggles.h` defaults at the time of FTD-0193's pre-registration) includes:
- **State-flux coupling** `-g_c · s · (∇_L · J)` (SPEC §3.6 term 2)
- **Velocity coupling** `-g_c · s · (v · J)` (SPEC §3.6 term 3)
- **Manifestation threshold** (genesis): voxel state transitions when `|J|² > K_GENESIS²` (SPEC §3.3 + engine implementation)
- **Evaporation threshold**: voxel state transitions back when `|J|² < K_EVAP²`
- **Gauss constraint** (already accounted for in §1.2)
- **Langevin thermostat** (per FTD-0051, when toggled)

The free-theory §3 result was derived under: Gauss constraint ON, all other interactions OFF. Extending to the canonical toggle set means asking whether the interactions can produce a new spin-2 pole.

### §5.2 — Structural argument that interactions preserve the no-pole result

A spin-2 pole at `(k, ω)` would require a **propagating spin-2 mode**: a state in the substrate Hilbert space (or its lattice analog) that carries helicity ±2 and propagates with a definite dispersion. The substrate's fundamental field is J (a 3-vector); no spin-2 propagating mode is built in at the kinematic level.

The interactions in §5.1 are:
- (a) **Cubic in J × s** (state-flux + velocity couplings). These do not change J's kinematic representation; they add interaction vertices that couple J-modes to substrate state transitions. They cannot create a new propagating mode without introducing a new field; they only renormalize existing modes (J-self-energy from s-loops, etc.).
- (b) **Threshold-nonlinear** (genesis + evaporation). These are not perturbative interactions; they are state-transition rules that change which voxels are manifested. They do not create new propagating modes either; they modify the substrate background (the set of manifested voxels) that the J-field propagates on.
- (c) **Langevin thermostat**. A noise term adding `+ξ` per tick; cannot create a propagating mode (it's a forcing term, not a dynamical equation for a new field).

None of (a), (b), (c) introduces a new dynamical rank-2 field. Therefore the spin-2 channel remains, at the kinematic level, populated only by bilinears of the spin-1 J-field. The bubble structure of §3 should persist; interactions shift the continuum (renormalize ω, shift threshold, broaden peaks) but do not create a separable pole.

**Tag: [STRONGLY MOTIVATED CONJECTURE]** — the argument is structural, not constructive; the conjecture is empirically validated by FTD-0193 within the probed regime (canonical toggle set + Gauss-constraint + manifestation dynamics ON, with 11/12 k-points consistent with the bubble prediction).

### §5.3 — What would refute §5.2

The §5.2 conjecture would be refuted if any of the following were demonstrated:
- A propagating rank-2 mode is identified in some toggle-set extension that emerges as a collective excitation rather than a fundamental field. (Candidate principles: finite-trace `s_m` variation per Doctrine §12; graph spectral curvature; finite adjacency deformation. None has been substrate-derived; pursued by Arc C1 if/when activated.)
- An interaction vertex couples bilinears to a previously-hidden sector that mediates spin-2 exchange. (No such sector identified.)
- The lattice's finite-L effects at L > 64 produce a separable spin-2 pole. (FTD-0193 §5 documents L=128 deferral for engineering reasons, not methodological; Arc C1 GPU port would test this.)

The §5.2 conjecture is therefore falsifiable in principle; FTD-0193 + this derivation establish it within the probed regime + canonical toggle set + §4-catalog observable algebra.

---

## §6 — Tag summary

| Step | Content | Tag | Source |
|---|---|---|---|
| 1.1 | Free-theory J wave equation `Δ_t² J_a = C² L_18 J_a` | **[THEOREM]** | SPEC_FTD_LAGRANGIAN.md §6.1 |
| 1.2 | Mode count: 1 longitudinal (gauge) + 2 transverse (spin-1 each) | **[THEOREM]** | Standard lattice gauge-theory; FTD-0193 §2 control empirical match |
| 1.3 | Free-theory J propagator `G_L^J(k, ω) = iP^T/(ω² - C²ω_L²(k) + iε)` | **[THEOREM]** | Standard Gaussian-integral evaluation |
| 2.1 | Bilinear observable `O_ij = J_iJ_j - (1/3)δ_ij|J|²` definition | **[DEFINITION]** | PREREG_GRAVITON_SUBSTRATE_MODE_v2.md §5 (i') |
| 2.2 | `O_ij` is the unique candidate non-site-local rank-2 spin-2 observable in §4 catalog | **[THEOREM]** | Group-theoretic decomposition Sym²(V)−Tr(V⊗V)/3 |
| 3.1 | Wick contraction: `⟨O O⟩_c` is a sum of `⟨JJ⟩⟨JJ⟩` products (bubble) | **[THEOREM]** | Standard Wick's theorem on Gaussian J |
| 3.2 | Momentum-space bubble integral form | **[THEOREM]** | Direct Fourier transform of §3.1 |
| 3.3 | Bubble integral has branch-cut structure, no isolated pole | **[THEOREM]** | Standard QFT bubble analysis; lattice analog via BZ-restricted residues |
| 3.4 | TT projection of `⟨O O⟩_c^{TT} = P^{TT} · Π^{TT}` with `Π^{TT}` only continuum | **[THEOREM at free-theory level]** | Combines §3.3 + §3.4 tensor algebra |
| 4 | FTD-0193 empirical confirmation: 11/12 k-points identical to spin-1 control at L=64 | **[VERIFIED]** | REPORT_GRAVITON_SUBSTRATE_MODE.md §4 |
| 5.1 | Canonical toggle set interaction structure | **[REFERENCE]** | engine term_toggles.h + SPEC_FTD_LAGRANGIAN.md §3.6 |
| 5.2 | Interactions in canonical toggle set preserve no-pole result | **[STRONGLY MOTIVATED CONJECTURE]** | Structural argument + FTD-0193 empirical validation |
| 5.3 | Refutation conditions for §5.2 conjecture | **[REFERENCE]** | Arc C1 candidate principles (Doctrine §12); L=128 deferred per REPORT §5 |

**Net tag for the J-bilinear no-spin2-pole result:**

- **Free-theory + Gauss constraint only:** `[THEOREM]` (rigorous chain of standard lattice-QFT steps).
- **Full canonical toggle set (interactions ON):** `[STRONGLY MOTIVATED CONJECTURE]` (structural argument + FTD-0193 empirical validation at 11/12 k-points L≤64).
- **General toggle configurations + L > 64:** `[OPEN]` (Arc C1 territory if pursued).

---

## §7 — Implications for Arc C2 boundary theorem

This derivation makes rigorous step 3 of the proof preview in `SCOPE_SPIN2_BOUNDARY_THEOREM.md` §3. The boundary theorem statement (C2-2) — "Any rank-2 observable built as a J-bilinear or J-derivative bilinear has its TT projection contain only the spin-1 mode propagated through the bilinear (a continuum/branch-cut contribution), not a separable spin-2 collective mode" — is now backed by:

- **At free-theory level + Gauss-only interactions:** `[THEOREM]`-grade derivation per §3.
- **At canonical toggle set level:** `[STRONGLY MOTIVATED CONJECTURE]`-grade structural argument + FTD-0193 empirical validation per §5.

The Arc C2 boundary theorem at P3 pre-reg should therefore state (C2-2) with this **dual tag**: free-theory `[THEOREM]` + canonical-toggle-set `[SMC]`. The boundary theorem's prior-favoured Outcome A (FOUND) is supported at both tag levels; the F9 risk noted in SCOPE §8 (theorem too easy → mistaken for deeper than it is) is mitigated by the honest two-tag structure here, which makes the free-theory rigor explicit and the interacting extension's `[SMC]` status explicit rather than buried.

---

## §8 — Honest limits

- **§5.2 conjecture is structural, not constructive.** A rigorous proof that no canonical-toggle-set interaction can create a spin-2 pole would require either (a) a full interaction-vertex enumeration with explicit no-pole verification per vertex, or (b) an axiom-level argument that no new propagating mode can emerge from the canonical toggle set. (a) is tedious but tractable; (b) requires deeper substrate-physics theorems that are themselves Arc C1 / Doctrine §12 territory. The `[SMC]` tag honestly reflects this gap.
- **L > 64 not addressed.** The free-theory §3 result is L-independent in form (the bubble integral has the same analytic structure at any L). But the empirical validation §4 is at L ∈ {32, 64}; FTD-0193 §5 documents L=128 as deferred for engineering reasons (host↔device transfer + CPU-side operator cost). The boundary theorem's scope claim must respect this — extending to "for all L" requires Arc C1's GPU-port work.
- **Derivative bilinears `[∂J · ∂J]_TT` not separately verified.** §2.1 noted the same analysis applies to "stress" observable; this needs explicit verification (probably trivial, but should be done at P1 (b) deliverable).
- **§3.3 lattice analog of bubble-integral analysis.** The continuum textbook result (Peskin-Schroeder §10.2) translates straightforwardly to the lattice via finite-BZ residue calculus, but a careful lattice-QFT reference would strengthen this. Candidate references: Montvay & Münster *Quantum Fields on a Lattice*, §3. Cite at P3 pre-reg time.

---

## §9 — What this derivation does NOT claim

- **NOT a proof that gravity is impossible on FTD's substrate.** Only that the helicity-±2 channel of J-bilinears (within the §4 frozen catalog at free-theory + Gauss-only level rigorously, and at canonical-toggle-set level as SMC) cannot support a separable pole.
- **NOT a refutation of substrate-derived emergent gravity in general.** Doctrine §12 candidate principles (finite-trace `s_m` variation, graph spectral curvature, finite adjacency deformation) remain `[CANDIDATE PRINCIPLE]` and could, if substrate-derived, produce emergent gravity via non-J-bilinear mechanisms outside the §4 catalog.
- **NOT a closure of Arc C2.** This is step 3 of the proof; Arc C2 also requires the (C2-1), (C2-3), (C2-4) clauses of the working theorem statement. P3 pre-reg + P4 closure attempt are the closure mechanism.
- **NOT a new spine theorem.** Spine count unchanged. This derivation is subsidiary to the Arc C2 boundary-theorem program; it backs up step 3 of the proof, not a standalone result.

---

## §10 — Single-line summary

**The free-theory two-point correlator of the symmetric traceless rank-2 J-bilinear, projected onto the helicity-±2 (TT) subspace, is a bubble integral of the spin-1 J-propagator whose analytic structure is a two-particle branch-cut continuum, NOT an isolated pole — making rigorous, at `[THEOREM]`-grade for free-theory + Gauss-only and at `[STRONGLY MOTIVATED CONJECTURE]`-grade for the full canonical toggle set (with FTD-0193 empirical validation at 11/12 k-points L≤64), the load-bearing step 3 of the Arc C2 spin-2 boundary theorem's proof preview, thereby supporting the boundary-theorem clause (C2-2) and the program's prior-favoured Outcome A (FOUND) with honest two-tag scope.**
