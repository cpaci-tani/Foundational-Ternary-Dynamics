# PRE-REGISTRATION — Frontier 4: does the FTD substrate carry an emergent massless spin-2 mode?

**Tag:** [PRE-REGISTRATION] — locks the hypothesis and decision criteria for the Frontier 4 engine campaign **before** any measurement. No claim is made or promoted by this document.
**Date:** 2026-05-21
**Status:** REGISTERED (hash-locked — see §9). Pending measurement.
**Version:** v1
**Pre-registers:** Step 4a of Frontier 4 (FTD/FQCR Cleanup & Taxonomy v1.4) — the engine test for whether the FTD substrate carries a propagating massless spin-2 (graviton) mode.
**Depends on:**
- FTD-0189 (Step-0 graviton-provenance audit — `h_μν` is posited, not derived; Frontier 4 open from scratch)
- FTD-0184 (FQCR gravity red-team — substrate strong-field gravity is [OPEN])
- FTD-0131 (Newton from substrate — the scalar `h_00` / clock sector *is* derived)
- FTD-0004 (Phase G discrete Poisson Green's function) — [THEOREM]
- FTD-0044 (per-voxel mass gap) — relevant to the gaplessness criterion

---

## 1 · Purpose and background

The Step-0 audit (LEDGER FTD-0189) established that the rank-2 metric perturbation `h_μν` — the carrier of the spin-2 graviton — is **posited** in the FTD corpus (`DERIV_RELATIVITY_DERIVATION.md` §10.1 Conjecture 10.1; spin-2 spatial part is §10.3 Gap 10.1), not constructed from the substrate. The Deser-bootstrap "derivation of the Einstein equations" therefore *completes* a posited graviton; it does not produce one. **Frontier 4 — does the FTD substrate carry a massless spin-2 mode? — is open from scratch.**

This document locks the test before measurement, so the result cannot be reinterpreted post-hoc. It is registered, not claimed.

## 2 · The question

> Does the FTD substrate's effective long-wavelength dynamics contain a **propagating, gapless (massless), transverse-traceless rank-2 (helicity ±2, spin-2) mode** — either as an independent linear mode or as an emergent collective pole of the interacting dynamics?

A positive answer is the missing input the Deser bootstrap assumes; a negative answer means FTD's effective gravity is at most scalar+vector and the graviton must be imported.

## 3 · Why the test is two-part

FTD's fundamental continuous field is the flux 3-vector `J` (the state field `s ∈ {−1,0,+1}` is discrete and carries no small-oscillation mode below the genesis threshold). The linear small-oscillation spectrum of a 3-vector field decomposes as **1 longitudinal (spin-0) ⊕ 2 transverse (spin-1)** per wavevector — there is no room in a 3-vector for the 5-component spin-2 representation. Therefore:

- A spin-2 mode **cannot** be a fundamental linear mode of `J` alone.
- If FTD has a graviton, it must be **emergent / composite** — a collective mode of the interacting substrate, in the class of lattice emergent-gravity programs (Wen string-nets, Volovik) that evade the Weinberg–Witten no-go via the discrete preferred frame.

The test is accordingly two-part: a linear census (sanity check) and an emergent-pole test (decisive).

## 4 · Test 4a-i — linear vacuum-spectrum census (sanity check)

Linearize the substrate dynamics around the vacuum (`s = 0`, `J = 0`). Perturb with small-amplitude plane-wave `J`-modes; measure the dispersion `ω(k)`; classify each branch by `O_h` irrep and continuum spin.

**Pre-declared expectation:** exactly **3 gapless branches** per `k` — 1 longitudinal (spin-0) + 2 transverse (spin-1). This confirms `J` carries no fundamental tensor degree of freedom and that any graviton must be emergent (→ Test 4a-ii).

**Pre-declared surprise condition:** if **more than 3** independent propagating branches appear per `k`, the substrate has a hidden degree of freedom — a result in its own right, to be investigated before 4a-ii.

4a-i does **not**, by itself, decide Frontier 4. A 3-branch result is the *expected* premise, not a negative outcome.

## 5 · Test 4a-ii — emergent transverse-traceless pole (decisive)

In the **interacting** substrate (genesis + nonlinear flux dynamics active; toggle set fixed in §8), measure the connected, transverse-traceless-projected two-point function of a rank-2 composite operator built from `J`.

**Pre-registered probe operators** (declared now, fixed; no post-hoc operator scanning):
- (i) strain-rate type: `O_ij = ∂_(i J_j) − (1/3) δ_ij (∂·J)` — symmetric traceless part of the flux velocity-gradient;
- (ii) stress type: `O_ij = [T_ij]_TT`, the transverse-traceless part of the Noether stress tensor `T_ij = (∂_iJ_a)(∂_jJ_a) − δ_ij L` (`DERIV_RELATIVITY_DERIVATION.md` §14.4 Theorem 14.2 — a genuine derived rank-2 J-bilinear).

**Decisive observable:** the connected correlator `⟨O^TT(k,ω) O^TT(−k,−ω)⟩_c`, transverse-traceless-projected, resolved in the helicity-±2 channel.

**Decision criterion:** does this correlator exhibit a **gapless pole** — `∝ 1/(ω² − c² k²)` with `c` the lattice signal speed, `ω → 0` as `k → 0` — carrying **helicity ±2** (verified by the transformation law `e^{±2iθ}` under rotation by `θ` about `k`, equivalently `O_h` content `E_g ⊕ T_2g`), separable from the spin-0 and spin-1 sectors of Test 4a-i?

## 6 · Pre-declared outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — positive** | A gapless helicity-±2 pole in `⟨O^TT O^TT⟩_c`, separable from the spin-0/spin-1 sectors, for at least one declared probe operator. | Emergent-graviton candidate. Frontier 4 advances to Step 4c (confirm masslessness across `L`) and Step 4d (stress-energy coupling). **No claim that GR is derived** — only that the spin-2 input exists. |
| **B — negative** | No gapless helicity-±2 pole: the TT correlator shows only the two-particle continuum / branch cut expected from spin-1 constituents. | Frontier 4 **[CLOSED NEGATIVE]** in the probed regime. FTD's effective gravity is at most scalar+vector; the Einstein-chain graviton is imported, not derived — confirms and sharpens FTD-0184 / FTD-0189. A genuine, valuable boundary result (project goal clause 2: "rigorously establish what we cannot derive"). |
| **Indeterminate** | The engine cannot resolve a pole vs continuum at accessible `L` / statistics. | **Not** Outcome B. The campaign is extended (larger `L`, more statistics) or re-registered as v2. A noisy null is never laundered into a clean negative. |

## 7 · Pre-declared exclusions (anti-gaming)

Per FTD epistemic discipline:

- **The polarization count is not the criterion.** "2 transverse modes" or "matches the 2 GW polarizations" does **not** count as Outcome A — that is exactly the spin-1/spin-2 representation error corrected in FTD-0189. The criterion is the helicity-±2 transformation law (`O_h` content `E_g ⊕ T_2g`), nothing less.
- **No post-hoc operator scanning.** The probe operators are fixed in §5. A pole found only by some operator constructed after seeing the data does not count.
- **No parameter tuning to manufacture a pole.** The toggle/parameter set is fixed in §8.
- **No near-miss or coincidence scanning.**
- **A noisy result is Indeterminate, not B.** Outcomes are not reinterpreted post-hoc.
- **Scalar success does not transfer.** The already-derived scalar `h_00` sector (FTD-0131) is not evidence for a spin-2 mode.

## 8 · Method specification

- **Engine / backend:** FTD C++/CUDA engine; WSL2/CUDA backend for all measurement campaigns (per project rule); CPU build for correctness checks only.
- **Test 4a-i:** vacuum `s=0, J=0`; small-amplitude plane-wave `J` perturbations along `[100]`, `[110]`, `[111]`; measure `ω(k)` by tracking the perturbation; sweep `|k|`; `L ∈ {32, 64}`.
- **Test 4a-ii:** interacting substrate with genesis active; toggle set = the default logic-first six rules **plus** the nonlinear flux coupling, **no** phenomenological toggles beyond those (the exact toggle list to be enumerated in the campaign source and hash-referenced against this registration); measure `⟨O^TT O^TT⟩_c`; `L ∈ {32, 64, 128}`.
- **New instrumentation:** the TT-projected composite-correlator measurement is expected to require a new engine campaign module. This pre-registration locks the hypothesis, the probe operators, and the decision criteria; the instrumentation is built to this spec **after** lock.
- **Analysis:** branch classification by `O_h` irrep (spin-2 ⊃ `E_g ⊕ T_2g`); helicity by the rotation-transformation law about `k`; pole-vs-continuum by the `ω`-dependence of the spectral function.

## 9 · Hash-lock declaration

This document is hash-locked, in the FTD pre-registration discipline, by its git commit and the tag **`preregister-graviton-substrate-mode-v1`**. The file SHA256 is recorded in the locking commit message and alongside LEDGER FTD-0189. No edit is permitted after lock; any change requires a fresh v2 registration with its own hash. Measurement (Step 4a-i, then 4a-ii) proceeds only after this lock.

**Registered by:** Frontier 4, Step 4a — FTD/FQCR Cleanup & Taxonomy v1.4, Path Forward §4.
