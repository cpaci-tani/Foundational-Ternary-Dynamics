# SCOPE -- FTD-0110 nonlinear bridge: desk-analytical vs engine-resourced classification

**Tag:** [SCOPING MEMO] -- not a derivation, not a new theorem, not a tag promotion. Classifies the FTD-0110 closure work into session-scoped desk tracks and multi-week engine campaigns.
**LEDGER row:** FTD-0203.
**Date:** 2026-05-23 (Path IV Session B1 of `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
**Owner-question this answers:** *"For each remaining FTD-0110 sub-investigation, is it desk-analytical work I can land in a session, or engine-resourced campaign work that needs WSL2/CUDA time?"*
**Sources read:**
- [`docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) -- the linear-level closure (`k = 1/N_base = 1/4` from O_h character-table multiplicity, [THEOREM]).
- [`docs/theory/03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](../03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) -- the Phase A/B/C sharpening that ruled out the 1/√d law and the Langevin-equipartition framing as per-block efficiencies (FALSIFIED in Phase B and C respectively).
- [`engine/tests/test_langevin_equipartition.cpp`](../../../engine/tests/test_langevin_equipartition.cpp) -- operational Langevin infrastructure (FTD-0051; CPU single-substrate OU update; equipartition verified to ~4%).

> **What this scope memo is NOT.** Not a closure attempt; not a pre-registration; not a tag move. It classifies the remaining work so the user can decide *what fits a session* and *what is a campaign*. Per the plan: "Engine-campaign pre-reg sketch (not the full pre-reg; that's Session B1+1 if pursued)."

---

## §1 -- Where the FTD-0110 bridge stands today (verbatim from EXPLR §6 + post-Phase-C)

**[DERIVED at linear level, [THEOREM]-grade]:** `k_linear = 1/N_base = 1/4` from `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`. The 27-block A_{1g} subspace has dimension 4 (Burnside-character formula); δ_center is A_{1g}-pure; the 18-point Laplacian preserves A_{1g}; the mean of A_{1g} eigenmode energies `{3/8, 1/8, 3/8, 1/8}` is exactly `1/4`. Direction-invariant. **Unchanged.**

**[STRONGLY MOTIVATED CONJECTURE, [OPEN] for the full nonlinear regime]:** the empirical k(A) drift in the engine matches the linear theorem at small A (k = 0.25 at A = 2) and drifts logarithmically toward k ≈ 0.21 at A ≈ 120. Empirical fit `k(A) ≈ (1/4)·(1 − 0.030·ln(A/2))`.

**Three candidate mechanisms (EXPLR §2):**
- **α** -- multi-block irrep mixing (most consistent with log-A functional form per EXPLR §3); the naive 1/√d per-block-efficiency reading was **FALSIFIED in Phase B** (commit `proof_ftd0110_langevin_steady_state.py`); per-block `f_slow` does not match 1/√d either as a function of `d` or position.
- **β** -- genesis kink-induced non-A_{1g} energy redistribution. Untested.
- **γ** -- Langevin amplitude-crossover at A\* ≈ 13. Untested. The crossover scale matches the empirical drift range and remains plausible.

**Also falsified (Phase C):** the Langevin-equipartition extension of the linear theorem (over-counted by ≈8× at large A, predicting `k > 1` which is unphysical). The per-block summation has structural over-counting (each voxel belongs to 27 blocks).

**Net status:** bridge gap is *sharper* than before, but two of the natural representation-theoretic frameworks have been ruled out. Mechanisms β and γ require different machinery.

---

## §2 -- The classification

Each remaining work item is classified by **track** (desk-analytical D vs engine-resourced E), **bounded-effort estimate** (in calendar units), and **prerequisite dependency** (what must land first).

### §2.1 -- Desk-analytical (session-scoped)

These can be drafted and (provisionally) executed in a single focused session of 4-8 hours each. No engine time required; mpmath/sympy suffices.

| ID | Work item | Track | Effort | Prerequisites | Closure verdict |
|---|---|---|---|---|---|
| **M-α-perturb** | Mechanism α: per-shell A_{1g} leakage prefactor calculation | D | ~1 week (multiple sessions) | None | If prefactor matches empirical `−0.030/ln-unit` slope, α is the dominant mechanism; if not, α is not the closure mechanism in this form |
| **M-β-est** | Mechanism β: genesis kink-induced non-A_{1g} energy estimate (Gaussian-flux + erfc threshold-crossing rate) | D | ~3-5 days (1-2 sessions) | None | Bounds the size of the genesis-kink contribution; if small, β is sub-dominant |
| **M-γ-balance** | Mechanism γ: Langevin equilibrium with A-dependent crossover at A\* ≈ 13 | D | ~3-5 days (1-2 sessions) | None | Predicts the k(A) curve from Langevin-only balance; compare to empirical engine data |
| **AP-no-over-count** | Per-voxel aggregation: derive a clean multi-block aggregation rule that does **not** over-count voxels (each voxel belongs to 27 blocks; Phase C used per-block summation and over-counted by ≈8×) | D | ~1 week (multiple sessions) | None | If a clean aggregation rule exists, retest Phase C's "Langevin equipartition extension" without the over-counting bug -- may rehabilitate or terminally close this candidate |

**Classification reasoning.** All four are perturbation-theory / representation-theory calculations on the existing 27-block + lattice Green's function machinery. They produce closed-form (or near-closed-form) predictions for the empirical slope or curve. None of them require engine measurements *to execute*; they all require engine measurements *to verify* their predictions against empirical k(A).

### §2.2 -- Engine-resourced (multi-week campaigns)

The four D3a/b/c/d engine experiments listed in `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §6.5 and re-stated in `EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` §3 are all engine-resourced parameter sweeps. Each requires a full pre-registration before execution per the project's pre-registration discipline (CLAUDE.md "NEW INFRASTRUCTURE 2026-04-27").

| ID | Work item | Track | Effort | Prerequisites | Discrimination value |
|---|---|---|---|---|---|
| **D3a** | Vary `K_GENESIS_KINETIC_DRAIN` at fixed A in {10, 30, 100}; measure k(A, drain) | E | ~2-3 days GPU + 1-2 days analysis | WSL2/CUDA backend (`engine/build_wsl`); pre-reg hash-lock | If k ∝ drain² → Mechanism β dominant |
| **D3b** | Vary `K_EVAP_RATE` at fixed A in {10, 30, 100}; measure k(A, evap) | E | ~2-3 days GPU + 1-2 days analysis | WSL2/CUDA backend; pre-reg hash-lock | If k scales monotonically with evap → cluster-balance dynamics (Mechanism γ-adjacent) dominant |
| **D3c** | Vary `T_L` (Langevin temperature) at fixed A in {10, 30, 100}; measure k(A, T_L) | E | ~2-3 days GPU + 1-2 days analysis | WSL2/CUDA backend; pre-reg hash-lock; baseline confirm against `test_langevin_equipartition.cpp` (currently passes equipartition to ~4%) | If k(A) curve shifts with T_L → Mechanism γ significant; if unaffected → γ sub-dominant |
| **D3d** | Vary `L` (lattice size) at fixed A in {10, 30, 100}; compare L=64 vs L=128 | E | ~3-5 days GPU (large) + 1-2 days analysis | WSL2/CUDA backend; L=128 large-mem path; pre-reg hash-lock | Mechanism α predicts saturation when cluster fills lattice (`L < R_cluster`) -- L=64 vs L=128 at A=30 (R ≈ 9) should agree, at A=120 (R ≈ 24) should disagree if α is dominant |

**Classification reasoning.** All four are **parameter sweeps over engine knobs**; none of them are "measure a single quantity once" jobs. Each is at least 3 amplitudes × 2-3 parameter values × 5 seeds = 30-45 engine runs. At L=64 each run is ≤10 minutes on WSL2/CUDA RTX 5090; at L=128 each run is ≈1 hour. Engine wall-time budgets are dominated by D3d (L=128).

### §2.3 -- The bridge campaign sketch (Session B1+1 = pre-reg, Session B1+2 = execute)

If the user pursues the engine-resourced track after this scoping memo:

**Pre-reg (Session B1+1, ~4-6 hours desk):** author `PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` covering D3a-D3d as a coordinated 4-arm parameter sweep. Locked content per the project's pre-reg discipline (PREREG_FINITE_NEUTRAL_LOCK_v1 + PREREG_COLOUR_SINGLET_RANK_v1 template):
- §2 question Q-FTD-0110-EXEC -- which of {α, β, γ} dominates the empirical k(A) drift?
- §3 definitions D1-D6 -- arm A/B/C/D parameter ranges; what "k" means at each (A, drain, evap, T_L, L); how seeds are aggregated; the discrimination criterion per arm.
- §4 frozen catalog of admissible diagnostics (the four arms above; no post-hoc arm additions).
- §5 four pre-blessed outcomes per arm (Mechanism α / β / γ / NONE-OF-THE-ABOVE) with explicit tag consequences.
- §7 falsifier rules F-a..F-h covering CODATA-input ban, free-parameter ban, look-elsewhere, post-hoc arm addition, calibration-dependence, RNG-portability, GPU-CPU parity.
- §8 banned moves (no engine-knob tuning to fit the empirical curve; no spine tag moves before closure).
- §9 locked 11-step method with numerical comparison only at step 10 after admissibility + falsifier + banned-moves checklists.

**Execute (Session B1+2 = multi-week campaign):** run D3a + D3b + D3c at L=64 first (≈3-5 days WSL2/CUDA), then D3d L=128 (≈1 week). Analyses against the pre-reg. Per-arm verdict per §5.

**Engine-time budget:** ≈2 weeks GPU wall-time on RTX 5090 via WSL2, dominated by D3d. Pre-reg authoring is ~6 hours desk; execution is ≈2 weeks engine time but only ≈3-4 days human review time.

---

## §3 -- What is *session-scoped today* vs *campaign-territory*

**Session-scoped (could be next session of Path IV / B2 / a one-off):**
- M-α-perturb -- the most-likely-dominant mechanism; if it lands the closure, no engine campaign needed.
- M-β-est -- short bound; cheap insurance against β contaminating the M-α reading.
- M-γ-balance -- ditto; the A\* ≈ 13 crossover is suggestive.
- AP-no-over-count -- methodologically critical (fixes the Phase C error pattern); enables Phase C-style multi-block aggregation tests without the over-counting bug.

**Campaign-territory (Session B1+1 + Session B1+2, ≈2 weeks engine time):**
- D3a + D3b + D3c at L=64 -- the discrimination triple.
- D3d at L=128 -- the lattice-size sensitivity test.

**Recommended next-session order (if user pursues Path IV):**
1. **M-α-perturb** (1-2 desk sessions) -- if closure lands here, the campaign is moot.
2. **M-β-est + M-γ-balance** (1 desk session each) -- bound the other two mechanisms; pre-cleared if α is dominant.
3. **Pre-reg D3a-D3d** (1 desk session) -- only if M-α-perturb does not land a clean closure.
4. **Execute D3a + D3b + D3c at L=64** (≈1 week engine + 1-2 days analysis).
5. **Execute D3d at L=128** (≈1 week engine + 1-2 days analysis).

If steps 1+2 close the bridge analytically, steps 3-5 are redundant (the perturbation calculation IS the closure; the engine measurements would only confirm). The plan §B1 deliverable as the user requested is the scoping memo; the next-session decision is the user's.

---

## §4 -- Honest limits of this scoping memo

- This memo does not predict which mechanism will close the bridge. The Phase B + C falsifications (in `EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` §8.6 + §8.7) ruled out two natural representation-theoretic frameworks; there is no guarantee that Mechanism α in the corrected (multi-shell + per-shell A_{1g} leakage) form will land either.
- This memo does not pre-register anything; pre-registration is a separate Session B1+1 if pursued.
- This memo does not move FTD-0110 off `[STRONGLY MOTIVATED CONJECTURE]`. Closure -- if it lands -- moves the LEDGER row; the analytical work or the engine campaign is the closure mechanism, not this memo.
- This memo does not promise the closure is achievable. Per `EXPLR §5` ("Why this is genuinely hard"): multiple mechanisms at play, nonequilibrium dynamics, discrete-continuous mismatch. The bridge may end up scoped down (e.g. closure only in the asymptotic large-A regime) rather than universally closed.

---

## §5 -- LEDGER + cross-refs

LEDGER row FTD-0203 [SCOPING MEMO] records this classification.

Cross-refs:
- FTD-0110 [STRONGLY MOTIVATED CONJECTURE] (main bridge claim).
- FTD-0119 [BRIDGE-ANALYZED] (the EXPLR doc that this memo classifies).
- FTD-0051 (Langevin thermostat infrastructure that D3c builds on).
- FTD-0110-α / -β / -γ / -D3 (the four sub-investigation tags in EXPLR §6).
- `engine/tests/test_langevin_equipartition.cpp` (D3c prerequisite baseline; currently equipartition to ~4%).
- `.claude/plans/let-s-proceed-on-the-eager-rocket.md` Session B1 (this work) + downstream Session B1+1 (pre-reg) + Session B1+2 (engine campaign).

---

*End of scoping memo.*
