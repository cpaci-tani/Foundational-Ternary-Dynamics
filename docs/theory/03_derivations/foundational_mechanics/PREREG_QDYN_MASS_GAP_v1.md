# PRE-REGISTRATION — The nonlinear-loop native mass gap (FTD-0270 closure swing, P2)

**Status:** PROTOCOL — to be hash-locked (SHA256 + git tag `preregister-qdyn-mass-gap-v1`) **before** the measurement run.
**Date:** 2026-06-26 · **LEDGER (on verdict):** FTD-0333 · **Arc:** FTD-0270 quantum-dynamics-ceiling closure, phase **P2** (the swing) — design `docs/superpowers/specs/2026-06-26-qdyn-ceiling-closure-design.md`.
**Frozen artifact (§1):** `engine/tests/campaign_mass_gap.cpp` — SHA256 `44a20d76a53779297a8bbf96d84e23a4cd15573be1cc094469ea017bc2fdd21d`.

---

## 0 · Purpose and honesty ceiling

The FTD-0270 ceiling is that the substrate's flux dispersion is linear (`s≈1`, massless), not the Schrödinger-quadratic the Rydberg needs. The closure swing (P2) asks whether the framework can **force a native rest-mass gap `ω₀ > 0`** for manifested matter, *without* the imposed de Broglie clock term (`de_broglie_clock` OFF). A nonzero native `ω₀` would make the non-relativistic envelope dispersion quadratic natively (FTD-0271's already-measured `E_env ∝ L^{−1.843}`), moving atomic spectra toward `[DERIVED]`.

**Honesty ceiling — what is already settled and is NOT the question here.** The *linear* native flux is established massless at `k=0` (`ω₀ = 0`): `test_de_broglie_clock.cpp` clock-OFF rest mode is flat to `~4×10⁻¹⁵`; FTD-0270 measures the massless dispersion `s = 0.944`; FTD-0271 §1 states it outright. Re-deriving that is a banned move (§7). The **only** non-duplicative, genuinely `[OPEN]` slice this pre-reg targets is:

> Does the **full nonlinear genesis↔Gauss back-reaction loop** (genesis firing + kinetic drain + Gauss projection all live) dynamically generate a `k=0` restoring oscillation of a manifested resting cluster's flux that the **linear operator analysis structurally cannot see**?

Prior across the nearby commutative-substrate closures and the L¹/L² no-go (FTD-0208) is **CLOSED-NEGATIVE** (no native gap). A null **hardens** FTD-0270; the swing is taken as an honest clause-1 attempt with the boundary as the guaranteed clause-2 landing. **Zero promotions** under any outcome (§5).

---

## 2 · The measurement (physics criteria)

**Engine config (native dynamics, clock OFF), frozen:**
- New campaign `engine/tests/campaign_mass_gap.cpp`. Backend: CPU reference (`OMP_NUM_THREADS=1`, bit-reproducible) is canonical for the verdict; WSL2/`build_wsl` CUDA cross-checks at CPU↔GPU parity. Golden-neutral (observation-only; `de_broglie_clock`/`db_clock_coulomb`/`symplectic_leapfrog` default OFF, dead branches; golden hash `0xb604d81a3d79366e` must be unchanged).
- Toggles ON: `wave_propagation`, `coupling`, `genesis`, `gauss_projection`. Toggles OFF: `dual_substrate`, `de_broglie_clock`, `langevin` (T=0, deterministic rest frame).
- Lattice `L = 32` (verdict), `L = 48` (finite-size cross-check). Deterministic `seed_rng`.
- Seed one resting cluster: `inject_flux(center, A·K_GENESIS, 0, 0)` (zero velocity). **Amplitude sweep** `A ∈ {6, 8, 10, 12, 16}` — `A` sets the manifested cluster size `N` (the FTD-0273 mass = voxel count), so this is the mass sweep.

**Observable.** The rest-frame flux autocorrelation `C(t) = Σ_{i∈probe} J_i(0)·J_i(t)` over a probe ball (radius `5`) centred on the cluster, sampled every tick for `T = 4096` ticks, DC-removed, FFT'd (`ftd::power_spectrum`), peak-picked with the leapfrog frequency correction `ω_phys = (2/dt)·sin(ω_raw/2)`. `ω₀` ≡ the lowest coherent spectral peak (above `10⁻³·PSD_max`). The cluster is at rest, so all spectral content is rest-frame (`k=0`).

**Two windows (the quiescence discriminator).** Record the genesis firing-rate per tick. Measure `ω₀` in (a) the **forming window** (genesis active) and (b) the **quiescent window** (after the firing rate falls below 10% of its peak). A genuine rest-mass gap persists into quiescence; a genesis-relaxation oscillation vanishes with the firing rate.

**Linear control.** Identical readout with `genesis` + `gauss_projection` **OFF** (pure linear wave on the same seeded flux). Must reproduce the established `ω₀ ≈ 0` — this validates the readout and is the null baseline.

---

## 3 · Gates (must all pass for a run to count)

- **G1 — readout validity (causal control):** the linear control yields `ω₀_ctrl < 0.01` (in lattice angular-frequency units). If the control shows a spurious peak, the readout is broken → run INVALID.
- **G2 — instability rejection:** the per-tick amplitude growth `ρ = ⟨|J|⟩(t+1)/⟨|J|⟩(t)` over the FFT window satisfies `ρ < 1.0005`. A leapfrog blow-up (`ρ ≳ 1.002`, the FTD-0308 signature) → run INVALID, **not** a positive gap.
- **G3 — cluster formed:** a manifested cluster of `N ≥ 3` voxels exists and persists through the measurement windows.
- **G4 — determinism:** CPU run bit-reproducible across two seeds-of-record; GPU within parity tolerance.

---

## 4 · Frozen discriminators and verdict map

Let `ω₀^q` = the gap in the quiescent window, `ω₀^f` = in the forming window, `ω₀^ctrl` = the linear control, `g_rate` = genesis firing rate.

| Discriminator | Genuine mass gap | Genesis-relaxation artifact | No gap |
|---|---|---|---|
| `ω₀^q` (quiescent) | `> 0.02`, coherent | `≈ ω₀^ctrl` (vanishes) | `≈ ω₀^ctrl` |
| persistence `ω₀^q / ω₀^f` | `≈ 1` (persists) | `≪ 1` (decays with `g_rate`) | n/a |
| scaling vs `A` | tracks `N`/`M_REST` (Compton-like, monotone) | tracks `g_rate`, not `N` | flat at 0 |
| phase coherence | coherent flux-phase rotation | incoherent `|J|`-relaxation | — |

**Verdict map:**
- **FORCED (CONFIRMED)** → `[OPEN → candidate]` (NOT a promotion): `ω₀^q > 0.02`, **coherent**, **persists** into quiescence (`ω₀^q/ω₀^f ≈ 1`), **G1–G4 all pass**, and scales with `N`/`M_REST` rather than `g_rate`. Requires a **fresh pre-registration + adversarial red-team** before any tag on FTD-0270/0271 moves. Prior-disfavoured.
- **SELECTION** → import shrinks, stays `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]`: a coherent persistent `ω₀^q > 0.02` whose value is motivated but not forced (e.g. requires a chosen coupling/identification).
- **CLOSED-NEGATIVE (NULL)** → **hardens FTD-0270** `[MEASURED — BOUNDARY]`, feeds P4: `ω₀^q ≈ ω₀^ctrl` (within the control band) **or** `ω₀` is a genesis-relaxation artifact (`ω₀^q/ω₀^f ≪ 1`, scales with `g_rate`). Prior-favoured.
- **INVALID** → re-scope: any gate G1–G4 fails (notably G2 instability → needs the E1 stable integrator first).

---

## 5 · Tags and priors

Prior probabilities (pre-registered): CLOSED-NEGATIVE ~70%, INVALID (instability) ~15%, SELECTION ~10%, FORCED ~5%. **Standing invariants held under every outcome:** FTD-0270 `[MEASURED — BOUNDARY]`, FTD-0271 `[CONDITIONAL]`, FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; no α derived; golden gate untouched. A FORCED outcome does **not** itself promote anything — it opens a candidate that a separate arc must red-team.

---

## 6 · Scope

In scope: the `k=0` rest-frame flux oscillation of a manifested resting cluster under the full nonlinear native loop, clock OFF. Out of scope: the imposed-clock spectra (FTD-0271/0278/0279, already `[CONDITIONAL]`); the `k≠0` dispersion (FTD-0270, already measured); the envelope-frame question (P3); guidance (FTD-0271, declined).

---

## 7 · Banned moves

1. **Do NOT** report "native flux is massless at `k=0`" as the result — that is the established FTD-0270/0271 baseline, here used only as the linear control (G1).
2. **Do NOT** conflate FTD-0044 (the *manifestation-energy* per-voxel gap, `spec(H) ⊂ {0}∪[K_B,∞)`) with a flux `ω(k=0)` oscillation — orthogonal objects.
3. **Do NOT** read a leapfrog instability (G2 fail) as a discovered gap.
4. **Do NOT** enable `de_broglie_clock` or `db_clock_coulomb` — those *impose* the very `ω₀` whose native emergence is the question.
5. **Do NOT** tune `ω₀`, the coupling, or the genesis threshold to manufacture a peak; the sweep is fixed in §2.
6. **Do NOT** promote any tag on a CONFIRMED outcome without a fresh pre-reg + red-team (§4/§5).

---

## 8 · Hash-lock

`campaign_mass_gap.cpp` SHA256 `44a20d76a53779297a8bbf96d84e23a4cd15573be1cc094469ea017bc2fdd21d`; this file committed and git-tagged `preregister-qdyn-mass-gap-v1` at the lock commit, **before** any measurement run (no `--sweep` invocation precedes the tag; only the ~2.7 s no-arg smoke was run, which produces no §4 discriminator reading). Run-of-record, analysis, and verdict are recorded in a separate `ANALYSIS_QDYN_MASS_GAP_v1.md` (FTD-0333) after the lock.
