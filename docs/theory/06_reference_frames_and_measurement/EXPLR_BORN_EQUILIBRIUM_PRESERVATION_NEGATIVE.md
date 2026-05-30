# EXPLR — Born-Equilibrium Preservation Test: Honest Mixed Result + Closed Negative

**Document type:** Exploratory test result (closed-negative on the DGZ-equilibrium route)
**Status:** `[CLOSED NEGATIVE]` on DGZ-equilibrium preservation in the 6-neighbour substrate; pre-registered formal outcome is `D_mixed`, but the underlying data is structurally clearer than that label conveys (see §3).
**Created:** 2026-05-23
**Pre-registration:** [`PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md`](PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md)
**Git tag (hash-lock):** `preregister-born-equilibrium-preservation-v1` (commit `16f10a2`, runner SHA256 `94b280f40c6ef69b2d6b1f964ca165cdaadc3fd975a56504bb54a9f519ff0732`)
**Runner:** `scripts/exploration/explore_born_equilibrium_preservation.py`
**Results:** `scripts/exploration/results/born_equilibrium_preservation_2026-05-23.{csv,md}`
**Sub-investigation of:** LEDGER FTD-0187 / T1c (the `[OPEN]` step *probability = normalized energy density*); follow-up to FTD-0200 closed-negative.
**LEDGER row:** FTD-0199.

---

## 0 · One-paragraph result

`[CLOSED NEGATIVE]` on the Dürr-Goldstein-Zanghì-equilibrium-preservation route to T1c in the 6-neighbour Python substrate. Three target `|ψ(v)|²` profiles (single Gaussian, uniform-with-envelope, two-bump) were initialized with `J(v) ~ Normal(0, σ²(v)·I_3)` ensemble-distributed so that `⟨|J(v)|²⟩ ∝ |ψ(v)|²`, then evolved deterministically for 80 ticks across 100 trials each. Long-run manifestation rate `freq_long(v)` was regressed against `|ψ(v)|²`. Result:

- **P2 (uniform envelope):** n = −0.0091, R² = 0.34 → equipartitioned (Class B). Substrate washes out the initial distribution into uniform.
- **P1 (Gaussian) and P3 (two-bump):** n = −0.07 to −0.09, R² = 0.97–0.99 → tight **anti-correlation**. Sites with high `|ψ(v)|²` get *fewer* long-run events, not more.

Neither preserves `|ψ|²`. The mechanism is structural: the deterministic ReLU threshold rule plus evaporation produces saturation at high-|J| sites (they manifest immediately, stay manifested, contribute no further events), while low-|J| sites cycle. This anti-correlates manifestation rate with `|ψ|²`. Both flavors are non-preservation. T1c is not closable via the DGZ route in this substrate; the engine-canonical version (v3) and the Softplus-regularized variant (v4) remain open paths.

---

## 1 · What was tested

Per (a)+(c) framing crystallized 2026-05-23 (a = law of large numbers, c = the right ensemble is initial conditions drawn from `|ψ|²`), the substantive T1c question is: **does the substrate preserve a Born-distributed initial ensemble under deterministic evolution?**

This mirrors DGZ 1992 for Bohmian mechanics: under the guidance equation, particles distributed as `|ψ|²` *stay* distributed as `|ψ|²`. Born is then the equilibrium of the dynamics, and LLN gives `freq → |ψ|²` because the equilibrium *is* `|ψ|²`. The FTD analog asks the same of substrate-distributed flux fields.

Construction (frozen in PREREG §2):

- Same 6-neighbour Python substrate as FTD-0200 v1.
- Three target profiles `|ψ(v)|²` (Gaussian, uniform-with-envelope, two-bump).
- Initial flux `J_{x,y,z}(v)` independently `~ Normal(0, σ²(v))` with `σ²(v) ∝ |ψ(v)|²` and `max σ² = 4/3` (energy-scale match to v1).
- 100 trials per profile × 80 ticks each = 8000 substrate samples per profile.
- Two measurements per profile: primary = long-run manifestation rate per voxel (ticks 20–80); secondary = first-event spatial distribution over the ensemble.

---

## 2 · What the run produced

Full data: [`born_equilibrium_preservation_2026-05-23.csv`](../../../scripts/exploration/results/born_equilibrium_preservation_2026-05-23.csv) (41 472 rows, 3 profiles × 24³ voxels).

**Primary measurement: `freq_long(v)` vs `|ψ(v)|²`, log-log power-law fit, equal-count bins.**

| Profile | n (slope) | R² | Pre-reg class |
|---|---|---|---|
| P1 single Gaussian | **−0.0898** | 0.9760 | D |
| P2 uniform envelope | −0.0091 | 0.3385 | **B** (equipartition) |
| P3 two-bump | **−0.0717** | 0.9870 | D |

**Aggregate outcome per PREREG §4.2:** `D_mixed` (no class has 2+ profiles).

**But the underlying physics is clearer than D_mixed conveys:**

- P2 cleanly equipartitions (B). Expected for a uniform initial profile evolving under a wave equation: nothing to spread, so it stays roughly uniform (modulo edge effects). The low R² indicates no spatial dependence, i.e., the substrate doesn't differentiate sites.
- P1 and P3 show **strong, statistically clean anti-correlation** (R² > 0.97). The slope magnitude is small (|n| ≈ 0.08) but the negative sign is robust and the regression is tight. The pre-registered classes did not anticipate "small-|n|, high-R² anti-correlation"; these profiles fall through to D *by the rules* even though the signal is real and structural.

**Secondary measurement: `hist_first(v)` (first-event spatial distribution) vs `|ψ(v)|²`.**

For P2, the secondary fit was `hist_first ∝ |ψ|^5.5` with R² = 0.9749. That is *much sharper than Born* (which would be n = 2 if anything). First events are highly concentrated on the very highest-|ψ|² sites because those sites cross threshold immediately and only one site can be "first". This is sharp-concentration behavior, not Born scaling. (P1 and P3 secondary fits were dominated by trivial first-event localization near the bump centres.)

---

## 3 · The structural explanation: saturation

`[STRUCTURAL OBSERVATION].` The deterministic ReLU manifestation rule has a built-in saturation: once a site satisfies `|J| > K_B`, the state transitions `s: 0 → ±1`, and it can only return to `s = 0` if `|J|` later drops below `K_B_evap = K_B/2 = 0.25`. For sites with high initial `|J|`, this drop is unlikely under wave-equation evolution (the amplitude doesn't decay fast enough; damping `γ = 0.001` is small). Such sites manifest in the first few ticks (which is why we skipped the first 20 ticks as burnin) and contribute *zero* manifestation events during the long-run window. Low-`|J|` sites, in contrast, repeatedly cross K_B (manifest) and fall below K_B_evap (evaporate) under wave fluctuations, contributing many events.

The net effect: `freq_long(v)` is **anti-correlated** with `|ψ(v)|²` because high-`|ψ|²` sites are *saturated* and low-`|ψ|²` sites *cycle*. This is not a noise artifact — R² ≈ 0.98 across two independent profiles confirms it as a structural feature of the manifestation rule.

`[STRUCTURAL OBSERVATION].` P2's equipartition + P1/P3's saturation anti-correlation together imply: **the deterministic threshold rule destroys spatial structure**. For uniform initials, structurelessness is preserved (P2: equipartition). For non-uniform initials, the threshold rule *inverts* the structure (P1, P3: anti-correlation), eventually washing it toward equipartition over very long times.

`[STRUCTURAL OBSERVATION].` The DGZ argument requires that the dynamics *preserve* `|ψ|²` as the equilibrium distribution. In Bohmian mechanics, the guidance equation is *measure-preserving* with respect to `|ψ|²` (by construction; it's a continuity equation for `|ψ|²`). The FTD substrate's discrete-time wave equation + threshold rule is **not** measure-preserving with respect to any spatial profile other than equipartition; the threshold rule introduces an irreversible step that biases toward cycling-around-K_B sites. So DGZ-equilibrium-preservation **cannot hold** in this substrate as a structural matter, not just a tuning matter.

`[CONJECTURE — testable in v4].` Replacing the sharp ReLU threshold with a Softplus regularization, `P(manifest) = σ(β·(|J| − K_B))`, removes the saturation discontinuity. At finite β, sites can spontaneously evaporate and re-manifest at rates determined by their local `|J|` distribution, which would re-couple manifestation rate to `|ψ|²`. Whether the resulting rate scales as `|ψ|²` (Born), `|ψ|` (linear), or something else is the v4 question. This is also the canonical FTD framework per `DERIV_COLLAPSE_MECHANISM.md` (Softplus → ReLU in the β → ∞ limit); the present v2 falsified the **β = ∞ deterministic limit**.

---

## 4 · What is **not** falsified by this run

- **Born rule itself.** Unaffected.
- **T1c (FTD-0187).** Sub-investigation; remains `[OPEN]`. Two routes closed-negative so far (FTD-0200 threshold-crossing-from-non-Born-ensemble; FTD-0199 DGZ-preservation-from-Born-ensemble in β=∞ regime). Open paths: Softplus v4, engine-canonical v3, EF-C3 algebraic-uniqueness.
- **The `|ψ|²` form question** (EF-C3): separate algebraic-uniqueness question.
- **The canonical engine version.** 26-neighbour Moore + full toggle dynamics not tested here.
- **The Lindblad/Softplus collapse framework** (`DERIV_COLLAPSE_MECHANISM.md`): the present test is the β = ∞ limit; the finite-β behavior is the natural follow-up.
- **`x₊ = 1/α`** (FTD-0013): unaffected.

---

## 5 · What this rules out

`[CLOSED NEGATIVE].` In the 6-neighbour Python substrate with a sharp ReLU manifestation rule (β = ∞), the substrate **does not preserve a `|ψ|²`-distributed initial ensemble**. The DGZ-equilibrium route to T1c does not work in this regime. The mechanism for the failure is identified: ReLU saturation at high-|J| sites + cycling at low-|J| sites + linear wave-equation equipartition. None of the three structural ingredients points toward Born preservation.

`[METHODOLOGICAL].` The pre-registered classification scheme (Class B requires R² < 0.5; Class C requires |n| > 0.30) did not anticipate **structured-but-small-|n|** outcomes. P1 and P3 produced clean structural signal (R² > 0.97) at small |n| (−0.07 to −0.09); they fell through to D by formal rule but the saturation reading is the honest interpretation. v3/v4 manifests should add a class for this signature: "structured-non-Born anti-correlation, R² > 0.90 AND |n| ∈ (0, 0.30)".

`[STRUCTURAL].` Any future Born-derivation attempt in FTD must address the saturation problem at the level of the manifestation rule. Three candidates:

1. **Softplus regularization** at finite β: removes saturation; potentially preserves `|ψ|²` (v4).
2. **First-event-only ensemble** (instead of long-run rate): each "measurement" is one trial returning one first-event location; the histogram across trials might reproduce Born without saturation. The secondary measurement in this run hinted at concentration but not Born scaling.
3. **Canonical engine** with manifestation + evaporation + Gauss projection + dual-substrate: the additional dynamics may break the saturation; testable in v3.

---

## 6 · Items still out of scope (per PREREG §4.4)

- EF-C3 (`|ψ|²` form): why quadratic vs `|ψ|` vs `|ψ|⁴`. Algebraic-uniqueness; separate workstream.
- Engine-canonical 26-neighbour version.
- Bell, interference, entanglement.
- Central conjecture `x₊ = 1/α`.

---

## 7 · Cross-references

- [`PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md`](PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md) — the locked manifest.
- [`LEDGER.md`](../07_assessment/LEDGER.md) — FTD-0187 (Born consolidation), FTD-0200 (threshold-crossing closed-negative), FTD-0199 (this test).
- [`EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md`](EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md) — v1 closed-negative (different question, same substrate).
- [`DERIV_COLLAPSE_MECHANISM.md`](DERIV_COLLAPSE_MECHANISM.md) — canonical Softplus → ReLU framework; v4 would test the finite-β case.
- Dürr, Goldstein, Zanghì 1992 — the analytical framework this test mirrored.

---

## 8 · Bookkeeping

- **Pre-registration discipline:** fully respected. Construction, prediction, outcome map, seeds frozen before runner executed.
- **Formal outcome per PREREG:** `D_mixed`. **Substantive verdict:** `[CLOSED NEGATIVE]` on DGZ-preservation route — see §3 for the structural reading not captured by the pre-registered class boundaries.
- **No FTD tag promoted.** Two more closed-negative paths added under FTD-0187 / T1c.
- **Engine touched:** none. Python-only.
- **Manuscript/paper touched:** none.
- **Result lives in:** this file + LEDGER FTD-0199 + the CSV/MD artifacts. Nowhere else.
- **Classification scheme update recommended:** future PREREG should add a "structured weak anti-correlation" class to avoid the D-by-default outcome when |n| is small but R² is high. Noted for future PREREG hygiene; does not retroactively change this run's outcome.
