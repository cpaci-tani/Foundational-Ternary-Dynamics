# ANALYSIS — Dynamical Time Dilation of a Moving Lattice Clock (Campaign 2 result)

**Tag:** `[OBSERVATION]` — pre-registered verdict **OTHER** (PREREG §6). **Nothing promoted; FTD-0013 and α unchanged; FTD-0208 clarified, not refuted.**
**Date:** 2026-06-07
**Pre-registration:** [`PREREG_DYNAMICAL_TIME_DILATION_v1.md`](PREREG_DYNAMICAL_TIME_DILATION_v1.md) (design + analysis locked before the verdict)
**Runner (frozen):** `engine/tests/campaign_time_dilation.cpp` SHA256 `ea29260b…`
**Analysis (frozen):** `scripts/exploration/analyze_time_dilation.py` SHA256 `323b2e7d…`
**Data:** `engine/results/time_dilation_2026-06-07/wave_clock_dilation.csv` + `verdict.md`
**Independent adversarial review:** RedTeamAuditor (fresh-context subagent, PREREG §S5 / B-10) — **PASS, rigor 8.5/10**, agreed with OTHER; its reframe is adopted as the headline below.
**LEDGER:** FTD-0252.

---

## 0 · One-paragraph result (honest framing)

For the chosen observable — a coherent flux wave's **co-moving (proper) carrier frequency** — the dilation `D(v) = √(1−v²)` is an **algebraic identity of the construction**, not an empirical contest. The bare-wave dispersion is a sum of squares (`ω² = c²k²+m²`) *by construction* (second-order dynamics), and on such a dispersion the kinematic definitions `v_g=dω/dk_z`, `ω_proper=ω−k_z v_g`, `D=ω_proper/ω₀` give `D=√(1−v²)` identically. So the genuine empirical content is **not** "L² beats L¹"; it is **(i) the lattice *realizes* this relativistic identity to <0.06% at low velocity, and (ii) calculable UV lattice corrections bend `D` *below* exact γ at higher velocity** (the clock over-dilates), the deviation growing monotonically with `v`/`k`. The pre-registered verdict is **OTHER [OBSERVATION]**: √(1−v²) is not cleanly isolated from its own Taylor truncation `1−v²/2` at the sampled velocities (ratio 0.47 > 0.30), the global median residual (0.022) sits a hair above the 0.02 bar, and — critically — **the IR limit was never probed** (a sweep flaw, §5). FTD-0208's linear `1−v` budget is a claim about a *different* observable (single-event coordinate budget); it is not in contest with the wave co-moving frequency.

## 1 · What was measured

The lattice flux is **massless** (`∂²_t J = c²∇²J`). An effective rest-mass is induced by a fixed **transverse** wavevector `k⊥` (factoring `J ∝ e^{ik⊥·r⊥}·φ` makes `φ` a 1+1-D massive field along the motion axis, `m_eff = c·k⊥`). For each mode `k = n_z·m̂ + n⊥·t̂` the lattice frequency `ω(k)` is measured by the **single-tick Rayleigh-quotient eigenvalue** (`ω² = −Σ(wave_vel·J)/Σ(J²)` from rest, bare wave only). The group velocity `v_g=dω/dk_z` (central difference), `v=v_g/C_WAVE` (`c_lat=1/√3`), and the co-moving frequency `ω_proper=ω−k_z v_g` give `D=ω_proper/ω₀`. **No `voxel.tau` is ever read** (it hardcodes the relativistic formula — verified by the adversarial review: `tau` appears only in a comment). Sweep: `L∈{33,49,65,97,129}` × motion ⟨100⟩/⟨110⟩/⟨111⟩ × 2 masses, 287 moving-clock points; 120/120 runner sanity checks PASS; golden gate `0xc13713f0e11a96da` unchanged (read-only).

## 2 · Results

Representative clean config (⟨100⟩, L=129, n⊥=16):

| v | D measured | √(1−v²) [L²] | 1−v [L¹] |
|---|---|---|---|
| 0.058 | 0.99816 | 0.99834 | 0.942 |
| 0.274 | 0.95737 | 0.96175 | 0.726 |
| 0.476 | 0.86275 | 0.87939 | 0.524 |
| 0.639 | 0.71701 | 0.76550 | 0.358 |

- **Low v (≲0.15):** `D` matches √(1−v²) to **<0.06%** (residual ≈ −0.0006). The lattice realizes the relativistic identity essentially exactly where it is soft.
- **Higher v:** `D` falls **below** √(1−v²), the gap growing monotonically to ≈ −0.04 at `v≈0.64` — the **UV lattice correction** (the discrete dispersion departs from `c²k²+m²` as `k` grows).
- **L¹ (`1−v`)** is wrong everywhere (e.g. 0.358 vs measured 0.717 at v=0.64) — but see §3: it was never a live competitor for this observable.
- **Isotropy (T3):** median |resid_L2| = 0.0198 ⟨100⟩, 0.0208 ⟨110⟩, 0.0265 ⟨111⟩ — a mild ⟨111⟩ excess, the expected O_h signature (the body diagonal samples larger `k` per `n_z`).
- **Pre-registered verdict: OTHER [OBSERVATION]** — WIN_RATIO (0.47>0.30) and FOUND_TOL (0.022>0.02) both fail on their own; the verdict is robust to the §5 T2 bug.

## 3 · The crucial caveat (adopted from the adversarial review)

`D=√(1−v²)` is an **identity** of D4–D6 on any gapped sum-of-squares dispersion: `ω_proper=ω−k_z·(c²k_z/ω)=m²/ω` and `ω=m/√(1−v²)`, so `D=ω_proper/ω₀=m²/(ω·m)=m/ω=√(1−v²)`. The bare-wave dispersion is sum-of-squares **by construction** (the dynamics is second-order). Therefore "L² wins" is essentially forced; **the measurement is a test of *lattice fidelity to* that identity and of the *UV-correction law*, not a contest L² won over L¹.** FTD-0208's `1−v` is a single-event coordinate-budget claim about a different construct; the 16× numerical "rejection" of L¹ is unsurprising and should not be read as an empirical horse race. The honest takeaways are §0(i) and §0(ii).

## 4 · Relation to FTD-0208 (clarified, not refuted)

FTD-0208 `[CLOSED NEGATIVE]` proved the *single-event* discrete budget is linear (`v+dτ/dt≤1`). This campaign measures a *different* observable (a coherent wave's co-moving carrier frequency), which is relativistic by construction. **There is no contradiction:** the two describe different things. What this adds is that the substrate's **wave sector carries the relativistic (L²) identity exactly in the continuum and realizes it to <0.06% at low v on the lattice** — consistent with the dispersion argument from the "why exactly γ" thread, but *not* a derivation of γ and *not* a demotion of the clock-hypothesis `[AXIOM]` (the IR limit was not demonstrated — §5).

## 5 · Defects found (this run) and the v2 plan

Found by the author and/or the independent review; none invalidate the OTHER verdict (which holds on WIN_RATIO + FOUND_TOL alone):

1. **IR limit not probed (design flaw).** The mass quantum `n⊥ ∝ L` pins `k⊥` near-constant across L (0.381→0.390, 2.3% drift), so increasing L gave finer velocity sampling at *fixed* k — the per-config residuals are flat across L. Falsifier **F-d correctly fired**; **no "γ emerges in the IR" claim is made.** **v2:** hold `n⊥` fixed and grow `L` so `k⊥→0`.
2. **T2 dtype bug.** The CSV `direction` parses as `int64`; the analysis filtered `=="100"` (string) → empty IR band → T2 always False. (Independently confirmed: int filter → 125 rows.) Affects only the T2 sub-check, not the verdict. **v2:** compare on int / `astype(str)`.
3. **Windows Unicode crash.** The frozen analysis prints `√` and aborts on a cp1252 console; it completes only under `PYTHONIOENCODING=utf-8` (used here). Reproducibility-on-declared-OS is broken. **v2:** `sys.stdout.reconfigure(encoding="utf-8")` / ASCII console.
4. **Non-independent runner-up.** `1−v²/2` is γ's own 2nd-order Taylor truncation, so "L² vs taylor" is L² losing to its own approximation; the 0.47 ratio is less meaningful than a true competitor. **v2:** drop/relabel, and sample higher v (where √(1−v²) and 1−v²/2 separate) — though see §3.
5. **Process:** git tag `preregister-dynamical-time-dilation-v1` owner-deferred (SHA256 content-hash is the substantive lock).

## 6 · Epistemic ledger

- Verdict: **OTHER [OBSERVATION]** (pre-registered, adversarially reviewed).
- `D=√(1−v²)` for this observable: **identity of the construction**, not a measurement (review §B).
- Lattice realizes it to <0.06% at low v; UV corrections bend it below γ at higher v: **[MEASURED]**.
- "γ emerges in the IR": **[OPEN]** — not demonstrated (the IR limit was not probed).
- **Nothing promoted.** FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`; the clock hypothesis stays an `[AXIOM]`; α is not derived; FTD-0208 stands (clarified).

## 7 · What this is NOT

- **Not** a derivation of γ (FTD-0208 closed that; this measures lattice fidelity to a constructed identity).
- **Not** a refutation of FTD-0208 (different observable).
- **Not** anything about α, FTD-0013, Born, or non-commutativity.

---

## v2 (2026-06-07) — the IR limit, SCOPED `[MEASURED]`

**Pre-reg:** [`PREREG_DYNAMICAL_TIME_DILATION_v2.md`](PREREG_DYNAMICAL_TIME_DILATION_v2.md) (runner SHA `28c99f87…`, analysis `scripts/exploration/analyze_time_dilation_v2.py` SHA `9a755904…`). **Independent review:** RedTeamAuditor v2 (fresh context) — **accept IR_CONFIRMED in scoped form, 7.5/10.** Data: `engine/results/time_dilation_v2_2026-06-07/`.

v2 fixed the v1 IR-flaw: holding `n⊥` **fixed** (`--nperp-fixed=3`) and growing `L∈{33,65,97,129,193}` makes `k⊥=2π·n⊥/L → 0`, so a matched `(dir, n⊥, n_z)` point genuinely **softens** with L. Result:

- **On the principal ⟨100⟩ axis, the lattice's departure from exact γ vanishes as a power law — `R = |D−√(1−v²)| ∝ L⁻¹·⁹⁸ ≈ L⁻²` (i.e. `∝ k²`, the leading UV-discretization error).** 9/9 matched groups monotone; the residual shrinks 34–94× from L=33 to L=193 (e.g. v≈0.29: 0.0024→0.00003; v≈0.67: 0.0218→0.00065). Independently reproduced by the reviewer; the velocity-drift confound is refuted (R tracks `k`, not `v`). **This is the genuine result: the substrate's wave dispersion converges to the relativistic sum-of-squares form as k→0 — emergent Lorentz invariance, measured.** `[MEASURED — γ emerges in the IR on ⟨100⟩ + moderate v]`
- **Honest scope (review must-fix):** 22/27 matched groups converge (ratio < 0.5); the **5 that do not are ultra-relativistic diagonal modes** (⟨110⟩/⟨111⟩, `n_z≥7`, `v>0.9`, near the `k≈1.2` turnover) — 3 of the 5 are sign-crossing artifacts of the ill-conditioned `R_hi/R_lo` ratio (`R_lo≈0`), the others non-monotone wobble. The blanket pre-registered `IR_CONFIRMED` clears the median thresholds (ratio 0.153<0.5; R_max 0.0046<0.005) but only by an **8% margin** that averages over these; it survives every honest sub-scope with large headroom (⟨100⟩-only R_max 0.0025; moderate-v 0.0006). **So the IR result is `[MEASURED]` for ⟨100⟩ + `v≲0.85`; the ultra-relativistic diagonal regime is `[OBSERVATION/OPEN]`** (finite-L vs fundamental unresolved — needs L≳257). Runner sanity: 2 configs (⟨110⟩ L=65, ⟨111⟩ L=97 — 4 checks) flagged, both exactly in that near-turnover regime, verdict-neutral.
- **Relation to FTD-0208 + the clock hypothesis.** FTD-0208 (no *exact* continuous γ from the *discrete* substrate) **stands** — the lattice carries `O(k²)` corrections; exact γ is only the `k→0` limit. v2 measures that those corrections **vanish as L⁻²**, i.e. the clock-hypothesis γ scaling is **IR-emergent with a measured power law**, not substrate-exact. The clock hypothesis may be annotated `[AXIOM with measured IR-emergent dynamical support (⟨100⟩, R∝L⁻²)]` — nothing stronger. The §3 caveat still holds: `D=√(1−v²)` is the construction's identity *given* a relativistic dispersion; what v2 adds is that the lattice dispersion *becomes* relativistic in the IR.
- **Nothing promoted.** FTD-0013, α unchanged.
- **v3 follow-ups (review):** scope-correct any downstream prose (done here); replace the `R_hi/R_lo` median with a per-group `R∝Lᵖ` exponent fit (sign-crossing-robust) and report the `p≈−2` directly; fix the pooled-trend table population (it currently *understates* convergence — conservative); add `L=257` on the diagonals to resolve whether the high-v non-convergence is finite-L or fundamental; create the git tags.
