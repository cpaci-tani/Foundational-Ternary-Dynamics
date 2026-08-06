# PRE-REGISTRATION v2 — Frontier 4: does the FTD substrate carry an emergent massless spin-2 mode?

**Tag:** [PRE-REGISTRATION] — locks the hypothesis and decision criteria for the Frontier 4 decisive engine campaign **before** the canonical measurement. Supersedes v1 §5 and §8 (see §0). No claim is made or promoted by this document.
**Date:** 2026-05-22
**Status:** REGISTERED (hash-locked — see §9). Pending the canonical measurement.
**Version:** v2 — supersedes `PREREG_GRAVITON_SUBSTRATE_MODE_v1.md` (retained, unmodified, as provenance).
**Pre-registers:** Step 4a-ii of Frontier 4 (FTD/FQCR Cleanup & Taxonomy v1.4) — the engine test for whether the FTD substrate carries an emergent massless spin-2 (graviton) mode.
**Depends on:**
- FTD-0189 (Step-0 graviton-provenance audit — `h_μν` is posited, not derived; Frontier 4 open from scratch)
- FTD-0184 (FQCR gravity red-team — substrate strong-field gravity is [OPEN])
- FTD-0131 (Newton from substrate — the scalar `h_00` / clock sector *is* derived)
- FTD-0004 (Phase G discrete Poisson Green's function) — [THEOREM]
- FTD-0044 (per-voxel mass gap) — relevant to the gaplessness criterion

---

## 0 · Why v2 — changelog from v1

PREREG v1 (`preregister-graviton-substrate-mode-v1`, commit `bdf7883`) declared two probe operators in §5. During the build + smoke-test of the campaign instrument — a build check at L=16, **not** the canonical measurement — probe operator (i), the **strain-rate operator** `O_ij = ½(∂_iJ_j+∂_jJ_i) − ⅓δ_ij(∂·J)`, was found **analytically degenerate in the transverse-traceless channel**:

> In Fourier space `O^strain_ij(k) = (i/2)(k_i J_j + k_j J_i) − (i/3)δ_ij(k·J)` — every term carries a free `k` index. The TT projector annihilates any tensor with a free `k` index (`P(k)·k̂ = 0`). Hence `Λ·O^strain ≡ 0` for all `J`, at every wavevector along the measured high-symmetry directions. A strain tensor — the symmetrized gradient of a *vector* field — is "k-reducible": it carries spin-0 and spin-1 content but **no helicity-±2 (spin-2) content**. It was never a valid spin-2 probe.

This is a design error in v1 §5. It is harmless to validity — a degenerate probe can neither produce a false pole nor mask a real one — but it leaves the decisive test on a single probe, losing the intended two-probe cross-check. **It was caught before the canonical measurement; no measurement was run under v1.**

v2 corrects it:
- **Operator (i) replaced** — the strain-rate operator is dropped; operator (i′) is now the **flux-quadrupole bilinear** (§5), a genuine rank-2 composite with real TT content.
- **Operator (ii) unchanged** — the stress bilinear stands.
- **§8 toggle/solver configuration is now enumerated explicitly** — the v1 §8 "hash-reference" obligation is discharged inside the registration itself.
- All other content — the question, the two-part structure, Outcomes A/B/Indeterminate, the exclusions, the scale — is unchanged from v1.

---

## 1 · Purpose and background

The Step-0 audit (LEDGER FTD-0189) established that the rank-2 metric perturbation `h_μν` — the carrier of the spin-2 graviton — is **posited** in the FTD corpus (`DERIV_RELATIVITY_DERIVATION.md` §10.1 Conjecture 10.1; spin-2 spatial part is §10.3 Gap 10.1), not constructed from the substrate. The Deser-bootstrap "derivation of the Einstein equations" *completes* a posited graviton; it does not produce one. **Frontier 4 — does the FTD substrate carry a massless spin-2 mode? — is open from scratch.** This document locks the decisive test before measurement.

## 2 · The question

> Does the FTD substrate's effective long-wavelength dynamics contain a **propagating, gapless (massless), transverse-traceless rank-2 (helicity ±2, spin-2) mode** — as an emergent collective pole of the interacting dynamics?

## 3 · Why the test is two-part

FTD's fundamental continuous field is the flux 3-vector `J`. The linear small-oscillation spectrum of a 3-vector decomposes as **1 longitudinal (spin-0) ⊕ 2 transverse (spin-1)** per wavevector — a 3-vector cannot carry the 5-component spin-2 representation. So a spin-2 mode, if any, must be **emergent / composite** (Wen string-net class; the discrete preferred frame evades the Weinberg–Witten no-go). The test is two-part: a linear census (4a-i, sanity) and an emergent-pole test (4a-ii, decisive).

## 4 · Test 4a-i — linear vacuum-spectrum census: **COMPLETE**

Established by the engine code audit (FTD-0189 follow-up): linearizing the substrate around vacuum (`s=0, J=0`) is exact — every nonlinear term is `∝ s = 0`. The surviving dynamics are 3 componentwise-decoupled, identical scalar wave equations (`Jⁿ⁺¹ − 2Jⁿ + Jⁿ⁻¹ = C²∇²₁₈Jⁿ`, `C²=1/3`). The linear vacuum spectrum is therefore **exactly 3 gapless branches = 1 longitudinal (spin-0) ⊕ 2 transverse (spin-1)** — dispersion `ω(k) = 2C|sin(k/2)|`, numerically confirmed to <0.1% by the existing `campaign_dispersion` suite. **No fundamental spin-2 degree of freedom; the "surprise condition" (>3 branches) was not triggered.** The premise stands: any FTD graviton must be emergent. v2 registers the decisive Test 4a-ii.

## 5 · Test 4a-ii — emergent transverse-traceless pole (decisive)

In the **interacting** substrate (toggle set fixed in §8), measure the connected, transverse-traceless-projected two-point function of a rank-2 composite operator built from `J`.

**Pre-registered probe operators** — both genuine symmetric-traceless rank-2 **bilinears** of `J`; neither is k-reducible, so both carry real TT content:

- **(i′) flux-quadrupole:** `O_ij = J_i J_j − ⅓δ_ij|J|²`. In Fourier `O_ij(k) = Σ_q J_i(q) J_j(k−q) − trace` — index `i` on `J(q)`, index `j` on `J(k−q)`, independent momenta; the TT projection is generically non-zero.
- **(ii) stress:** `O_ij = [T_ij]_TT`, the transverse-traceless part of the Noether stress tensor `T_ij = (∂_iJ_a)(∂_jJ_a) − δ_ij L`, `L = ½|J̇|² − ½C²|∇J|²` (Theorem 14.2, `docs/theory/03_derivations/DERIV_RELATIVITY_DERIVATION.md` §14.4). The `δ_ij L` term is pure-trace and drops under traceless projection, so `O_ij = [(∂_iJ_a)(∂_jJ_a)]_TT`.

The two operators are independent constructions (`J⊗J` vs `∂J⊗∂J`); a result on both is a genuine cross-check.

**Decisive observable:** the connected scalar correlator `C_TT(k,τ) = ⟨O^TT_ij(k,t+τ) · O^TT_ij(k,t)*⟩_c` (sum over i,j; connected = subtract the time-mean ⟨O^TT⟩), where `O^TT_ij(k) = Λ_ij,lm(k) O_lm(k)` with the 3D transverse-traceless projector `Λ_ij,lm = ½(P_il P_jm + P_im P_jl) − ½P_ij P_lm`, `P_ij = δ_ij − k̂_i k̂_j`. In 3D, `Λ` isolates exactly the 2-dimensional helicity-±2 subspace.

**Decision criterion:** does `C_TT(k,τ)` exhibit a **gapless pole** — a clean single-frequency propagating mode `C_TT ~ A cos(ω(k)τ)·e^{−Γτ}` consistent with `∝ 1/(ω²−c²k²)`, `ω → 0` as `k → 0` — in the helicity-±2 channel, separable from the spin-0 and spin-1 sectors?

**Control (mandatory self-validation):** the campaign also measures the spin-1 sector — the connected transverse-vector correlator of `J` itself, a known propagating mode with `ω(k) = 2C|sin(k/2)|`, `C = 1/√3`. The machinery must recover this control; if it cannot, the instrument is broken. The control also confirms any spin-2 pole sits at a *different* `ω` than the spin-1 branch.

## 6 · Pre-declared outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — positive** | A gapless helicity-±2 pole in `⟨O^TT O^TT⟩_c`, separable from the spin-0/spin-1 sectors, for at least one declared probe operator. | Emergent-graviton candidate. Frontier 4 advances to Step 4c (confirm masslessness across `L`) and Step 4d (stress-energy coupling). **No claim that GR is derived** — only that the spin-2 input exists. |
| **B — negative** | No gapless helicity-±2 pole: the TT correlator shows only the two-particle continuum / branch cut expected from spin-1 constituents. | Frontier 4 **[CLOSED NEGATIVE]** in the probed regime. FTD's effective gravity is at most scalar+vector; the Einstein-chain graviton is imported, not derived — confirms and sharpens FTD-0184 / FTD-0189. A genuine boundary result (project goal clause 2: "rigorously establish what we cannot derive"). |
| **Indeterminate** | The engine cannot resolve a pole vs continuum at accessible `L` / statistics. | **Not** Outcome B. The campaign is extended (larger `L`, more statistics) or re-registered as v3. A noisy null is never laundered into a clean negative. |

## 7 · Pre-declared exclusions (anti-gaming)

- **The polarization count is not the criterion.** "2 transverse modes" or "matches the 2 GW polarizations" does **not** count as Outcome A — that is the spin-1/spin-2 representation error corrected in FTD-0189. The criterion is the helicity-±2 transformation law (`O_h` content `E_g ⊕ T_2g`), nothing less.
- **No post-hoc operator scanning.** The two probe operators are fixed in §5. A pole found only by some operator constructed after seeing the data does not count.
- **No parameter tuning to manufacture a pole.** The toggle/solver configuration is fixed and enumerated in §8.
- **No near-miss or coincidence scanning.**
- **A noisy result is Indeterminate, not B.** Outcomes are not reinterpreted post-hoc.
- **Scalar success does not transfer.** The already-derived scalar `h_00` sector (FTD-0131) is not evidence for a spin-2 mode.

## 8 · Method specification (explicit — discharges the hash-reference obligation)

- **Engine / backend:** FTD C++/CUDA engine; WSL2/CUDA backend (`engine/build_wsl`) for all measurement campaigns, per project rule.
- **Instrument:** `engine/tests/campaign_graviton_tt_correlator.cpp`, locked in the same commit as this registration.
- **Toggle set** (on a freshly-constructed `RenderBridge` after `toggles.disable_all()`): **ON (11)** — `wave_propagation`, `coupling`, `gauss_projection`, `genesis`, `forces`, `gravity`, `poisson_coulomb`, `lorentz_force`, `movement`, `damping`, `selective_damping`. **OFF** — every other toggle, including `dual_substrate`, `weak_transmutation`, `color_forces`, `strong_force`, `larmor_radiation`, `triad_binding`, `pair_production`, `exchange_force`, `latency_field`, `emergent_forces`, `langevin`, `exact_dual_gauss`, `confinement`, `strict_validation`. This is SPEC_ENGINE §1's six logic-first rules + the nonlinear fluxstate coupling + `selective_damping` (which makes the vacuum lossless); no phenomenological extensions.
- **Solver:** SOR iterations = 20 (a Gauss-constraint solver-accuracy setting, not a physics parameter; engine default is 6).
- **Initial state:** vacuum + a fixed-seed deterministic broadband `J` perturbation (seed `0x4A21B7`, per-mode amplitude `0.02`, plane waves `n=1..4` per axis plus body-diagonal modes). FTD is deterministic; the run is exactly reproducible.
- **Wavevectors:** small `|k|` (`n=1..4`) along the high-symmetry directions `[100]`, `[110]`, `[111]`.
- **Scale:** `L ∈ {32, 64, 128}`; `L` is a command-line parameter. A short equilibration precedes the measurement window.
- **Analysis:** per `k` and per operator — the dominant `(ω, Γ)`, the pole-vs-continuum diagnostic, the connected variance, and the spin-1 control `ω(k)`. The campaign emits measurement data only; the Outcome A/B/Indeterminate verdict is applied afterward against §6.

## 9 · Hash-lock declaration

This document is hash-locked, in the FTD pre-registration discipline, by its git commit and the tag **`preregister-graviton-substrate-mode-v2`** — committed together with the instrument (`campaign_graviton_tt_correlator.cpp`) so the spec and the instrument are locked as one unit before measurement. The file SHA256 is recorded in the locking commit message. No edit is permitted after lock; any change requires a fresh v3 registration. The canonical `L ∈ {32,64,128}` measurement proceeds only after this lock.

**Registered by:** Frontier 4, Step 4a-ii — FTD/FQCR Cleanup & Taxonomy v1.4, Path Forward §4.
