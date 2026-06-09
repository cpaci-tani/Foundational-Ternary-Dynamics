# PRE-REGISTRATION — Dynamical Time Dilation of a Moving Lattice Clock (v1)

**Tag:** `[PRE-REGISTRATION]` (design + analysis LOCKED before the verdict)
**Date:** 2026-06-07
**LEDGER row (reserved):** FTD-0252
**Runner (frozen):** `engine/tests/campaign_time_dilation.cpp`
SHA256 `ea29260b20dcbcadbaeec5e79125d099bad77313f346fe28df74454f07fff331`
**Analysis (frozen):** `scripts/exploration/analyze_time_dilation.py`
SHA256 `323b2e7d4dce9a2a0211007a0bc39492f48acdba43813bdd93e99a655a513bad`
**Git tag:** `preregister-dynamical-time-dilation-v1` — *owner-deferred* (working tree holds
unrelated in-flight session work; the SHA256 content-hashes above are the substantive lock,
and the independent adversarial review of §S5 is the integrity check). The verdict run and
result doc post-date this file.
**Plan:** `.claude/plans/plan-an-intuitive-path-twinkling-gizmo.md` (Campaign 2).

---

## §1 · Context & doctrine

The "why exactly γ" thread produced a **testable disagreement inside FTD**. FTD-0208
`[CLOSED NEGATIVE, AXIOM-LEVEL]` proved that the discrete substrate's *single-event*
coordinate budget is **linear (L¹)**: `v + dτ/dt ≤ 1` ⇒ `dτ/dt = 1−v` (no γ), and tagged the
Pythagorean clock hypothesis an `[AXIOM]` (the engine's `voxel.tau` hardcodes
`dτ/dt = √(f²−v²)/√f`). The **wave-dispersion** argument says the dynamics is second-order, so
the dispersion is a sum of squares **(L²)**: `ω² = c²k² + m²` ⇒ `dτ/dt = √(1−v²)` (γ) in the IR.
These differ by **36% at v=0.5**. FTD-0208 reasoned about single events; the dispersion argument
is about coherent waves. **This pre-registration locks a measurement of which law a real,
counted oscillation obeys.** It is a MEASUREMENT that *refines* FTD-0208, **not** a re-derivation
of γ (FTD-0208 closed that derivation). Prior-favoured but genuinely open: **L²/γ in the IR with
calculable lattice corrections.** Nothing about α or FTD-0013 moves under any outcome.

## §2 · The Question (LOCKED)

For a moving "wave clock" (defined in §3), measure the dilation `D(v) = ω_proper(v)/ω₀` of its
co-moving oscillation as a function of velocity `v`, and decide which **parameter-free** law it
follows:

- **L²/γ:** `D(v) = √(1 − v²)` (wave-dispersion prediction); OR
- **L¹:** `D(v) = 1 − v` (FTD-0208 discrete single-event budget); OR
- **OTHER:** neither, within the §6 thresholds.

Sub-questions: **(T2)** does the L²/γ residual shrink toward 0 as the mode softens (IR)?
**(T3)** is `D(v)` isotropic across motion directions ⟨100⟩, ⟨110⟩, ⟨111⟩?

## §3 · Definitions (LOCKED)

- **D1 — Wave clock.** A flux plane-wave with wavevector `k = n_z·m̂ + n⊥·t̂` (`m̂` the integer
  motion direction, `t̂` an integer transverse direction with `t̂·m̂ = 0`). The fixed transverse
  quantum `n⊥` supplies an **effective rest-mass** `m_eff = c·k⊥` (factoring `J ∝ e^{ik⊥·r⊥}·φ`
  reduces the massless 3-D wave to a 1+1-D massive field `φ` along `m̂`). The lattice is otherwise
  **massless** (`∂²_t J = c²∇²J`).
- **D2 — ω(k).** The single-tick Rayleigh-quotient eigenvalue: inject `J ∝ sin(k·r)` at rest,
  one bare-wave tick, `ω² = −Σ(wave_vel·J)/Σ(J²)`. Exact for a periodic eigenmode.
- **D3 — Rest frequency.** `ω₀ = ω(n_z = 0)` (pure transverse mode; the clock at rest).
- **D4 — Group velocity / velocity.** `v_g = dω/dk_z` by central difference of measured `ω`;
  `v = v_g / C_WAVE` (`c_lattice = C_WAVE = 1/√3`).
- **D5 — Co-moving (proper) frequency.** `ω_proper = ω − k_z·v_g` (the carrier frequency seen at
  the co-moving wave-packet centre — a kinematic identity, **not** the `tau` formula).
- **D6 — Dilation.** `D = ω_proper / ω₀`.
- **D7 — IR limit.** Softer mode / larger `L`: `k → 0`, lattice corrections → 0.
- **D8 — Clean regime.** `k_motion < 1.2` (pre-turnover): the lattice group velocity rises and
  `D(v)` is monotone; beyond it the lattice dispersion turns over (excluded).

## §4 · Admissible search space (LOCKED)

- **Allowed:** the bare wave dynamics (`wave_propagation` only); the single-tick Rayleigh
  eigenvalue (D2); central-difference `v_g`; `ω_proper = ω − k_z v_g`; the two **parameter-free**
  laws of §2.
- **EXCLUDED:** the `voxel.tau` accumulator (circular — it hardcodes the relativistic formula);
  any insertion of a Lorentz / clock-hypothesis / Born-Infeld formula as a derivation step; any
  free-parameter fit; any `v`/`L`/direction sub-selection chosen after seeing residuals.

## §5 · Benchmark (LOCKED)

- **L²/γ:** `D(v) = √(1−v²)` — emitted by the runner as `dilation_L2`.
- **L¹:** `D(v) = 1−v` — emitted as `dilation_L1`.
- **Sanity limits:** `D(0)=1` (rest clock undilated); `D` monotone decreasing in `v` (clock slows);
  `v < 1` (subluminal). (These are the runner's verdict-neutral sanity checks; they pass for
  either law.)

## §6 · Three pre-blessed outcomes (LOCKED)

Thresholds (frozen in the analysis): `WIN_RATIO = 0.30`, `FOUND_TOL = 0.02`.

- **L2_FOUND** — median `|D − √(1−v²)|` < `FOUND_TOL`, **and** < `WIN_RATIO ×` the runner-up law's
  median, **and** the L² residual decreases with `L` in the IR band (T2). → tag
  `[MEASURED — γ emerges in the IR]`; refines FTD-0208 (its L¹ budget governs single events, not
  wave-dynamical clocks). **Does NOT promote FTD-0013 or derive α.** The clock hypothesis may be
  annotated `[AXIOM with IR-emergent dynamical support]` — nothing stronger.
- **L1_FOUND** — symmetric, with `1−v` winning. → `[MEASURED — linear budget]`; vindicates
  FTD-0208 dynamically.
- **OTHER** — neither law wins cleanly. → `[OBSERVATION]`; report the measured form.

## §7 · Falsifier rules (LOCKED)

- **F-a.** The runner reads `voxel.tau` anywhere → INVALID (verify by source inspection).
- **F-b.** Any free-parameter fit to the data → at most OTHER (the laws are parameter-free).
- **F-c.** Every reported point carries its own measured `v_norm`; no extrapolation past `D8`.
- **F-d.** An IR claim (T2) requires `|resid_L2|` measurably decreasing with `L`; otherwise the
  γ result is stated only at the measured `L`, not "in the IR".
- **F-e.** An isotropy claim (T3) requires all three directions measured; a single direction → no
  isotropy claim.
- **F-f.** The analyzed CSV must come from the frozen-SHA runner; any code edit voids the lock and
  requires a v2.

## §8 · Banned moves (LOCKED)

- No post-hoc tuning of `WIN_RATIO` / `FOUND_TOL` / `K_MAX` to flip a verdict.
- No selecting only the `(v, L, direction)` points where a law fits.
- No reading or citing `voxel.tau`.
- No claim of *deriving* γ — FTD-0208 closed that; this MEASURES and refines.
- No tag promotion of FTD-0013 or any α claim under any outcome.
- (B-9) result doc must post-date this file; (B-10) the verdict is gated on an **independent**
  `RedTeamAuditor` subagent (fresh context) — see §9 S5.

## §9 · Method (LOCKED)

- **S1.** Build the frozen runner; confirm 72/72 verdict-neutral sanity checks PASS and golden gate
  unchanged. *(done 2026-06-07)*
- **S2.** Run the canonical sweep `--Llist=33,49,65,97,129` × {⟨100⟩,⟨110⟩,⟨111⟩} × 2 masses →
  `wave_clock_dilation.csv` in the canonical results dir (CPU, deterministic; re-run ⇒ bit-identical).
- **S4.** Run the **frozen** analysis on that CSV; apply the §6 thresholds mechanically; record the
  verdict.
- **S5.** Dispatch a fresh, independent `RedTeamAuditor` subagent (no working-context) to audit the
  runner physics, the analysis, the falsifiers (§7), and the verdict before any claim is made.
- **S6.** Write the result doc (`ANALYSIS_DYNAMICAL_TIME_DILATION.md`) + finalize LEDGER FTD-0252,
  tagged strictly per the landed §6 outcome.
