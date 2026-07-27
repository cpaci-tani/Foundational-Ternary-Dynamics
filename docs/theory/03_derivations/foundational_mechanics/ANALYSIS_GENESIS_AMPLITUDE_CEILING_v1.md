# Analysis — Genesis ignition threshold and dynamical crest regulation (FTD-0316; corrected by FTD-0567)

**Tag:** `[EMERGENT / MEASURED — DERIVED-FROM-RULE]` for the registered ignition threshold and dynamical crest regulation; the *numeric* threshold is `[IMPOSED]`-calibration-conditional. The former exact-amplitude-ceiling interpretation is `[RETRACTED — FTD-0567]`. **NOT** a `[THEOREM]`, **NOT** a new physics derivation.
**Lock (in-source pre-registration):** the three outcomes + the two numeric predictions were registered in the campaign header *before* running — `engine/tests/campaign_genesis_amplitude_ceiling.cpp:27-34` (outcomes 1/2/3), `:20` ("threshold: exactly A = K_GENESIS"), `:23` ("wavelength-invariance: NO"). Golden gate **certified green** — the three runtime override knobs (`genesis_threshold_override` / `manifest_scale_override` / `manifest_use_temperature`) are golden-neutral by construction (defaults reproduce compile-time `K_GENESIS`/`K_MANIFEST` byte-for-byte); `test_render_bridge_golden` **rebuilt from source + PASS** (`0xb604d81a3d79366e`). Read-only on physics phases.
**Controlling correction (2026-07-26):** FTD-0567 proves from the exact accepted-event map that `|J|'=|J|-K_GENESIS`. The rule subtracts one threshold unit; it does not set the residual to the threshold. For `|J|>2K_GENESIS`, one accepted event leaves a supercritical field. This document's measured ignition cliff survives, while every statement calling the local map an exact amplitude ceiling is superseded.
**Precedence:** LEDGER > FTD-0567 > this historical run report.

---

## 0 · Verdict

> **CORRECTED OUTCOME.** The registered arms establish a **sharp, wavelength-invariant genesis ignition threshold at `|J| = K_GENESIS`**: genesis is *identically zero* for antinode amplitude `A ≤ K_GENESIS` and ignites for `A > K_GENESIS`, with the same cliff at every wavelength tested. Above it, the tested dynamical waves show crest regulation and matter accumulation. FTD-0567 proves that this is not an exact local amplitude cap; “breaking” is a descriptive analogy for the measured collective behavior, not a derived Schwinger-limit equivalence.

**The honest reading (mandatory, not buried):** the ignition cliff is the engine's own genesis predicate (`|J| > K_GENESIS`) **made visible** under a coherent drive — it is `[DERIVED-FROM-RULE]`/`[EMERGENT]`, **not an independent dynamical surprise and not a theorem.** The historical run's number 1.533 was **`[IMPOSED]`-conditional**, and the override control showed that the cliff tracks whatever `K_GENESIS` is set to. The forward content is the sharp, wavelength-invariant **eligibility threshold** in those arms. No universal post-event amplitude follows.

**Nothing promoted.** `x₊ = 1/α` stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; golden gate green.

---

## 1 · The experiment + pre-registration

Light in FTD is assigned to a wave in the **continuous** flux field `J` (FTD-0298), which carries no axiom-level amplitude cap. Genesis fires when a void site reaches `|J| > K_GENESIS`, and an accepted single-substrate event applies `flux *= 1-K_GENESIS/|J|`. The correct consequence is `|J|'=|J|-K_GENESIS`: subtraction by one threshold unit, not reset to or below the threshold. The campaign drives a **coherent transverse plane wave** `J = (0, A·sin(kx), 0)` (divergence-free ⇒ survives Gauss projection; antinodes reach `|J| = A`), Langevin OFF, dual-substrate OFF, stack = wave-propagation + Gauss + genesis. Its historical pre-stated outcomes are preserved as run provenance; FTD-0567 narrows Outcome 1 from “ceiling” to “ignition threshold plus measured dynamical regulation.”

The **`--static`** mode (wave-propagation OFF, the field held at amplitude `A`) isolates the **clean gate**: onset is then exactly at `|J| = K_GENESIS`, with no propagation softening.

---

## 2 · The run of record

**Clean gate — `genesis_amp_static` (L=32, ticks=25, 3 seeds, nodes ∈ {2,4,8}):** `genesis_total` is **EXACTLY 0** for every seed at `A ∈ {1.0, 1.2, 1.4, 1.45, 1.5, 1.52, 1.53, 1.533}`, then a **hard cliff** to ~10⁴ events at `A = 1.540`:

| nodes (wavelength) | genesis at A≤1.533 | genesis at A=1.540 (3 seeds) |
|---|---|---|
| 2 (λ=16) | **0** | 9549 / 10389 / 9987 |
| 4 (λ=8)  | **0** | 11573 / 10732 / 10691 |
| 8 (λ=4)  | **0** | 22683 / 22162 / 21518 |

The cliff sits at **exactly `K_GENESIS = N_c·K_MANIFEST = 3·0.511 = 1.533`** (the gate is `|J| > K_GENESIS`, so `A = 1.533` does *not* fire and `A = 1.540` does), and it is **identical across all three wavelengths** ⇒ amplitude ⊥ wavelength, exactly as pre-registered. (Higher node count → more antinodes → more events above threshold, but the *cliff position* is unmoved.)

**Wave breaking — `genesis_amp_wave` (propagation ON):** onset slightly *softened* to ~1.50–1.533 (a propagating antinode can transiently exceed `A`); above it `genesis_total` rises monotonically while `max_void_J` stays pinned in the ~1.5–2.0 band and manifested matter accumulates to lattice saturation — the **breaking-wave signature**.

**Calibration control — `genesis_kg_kb` (override `kg = K_MANIFEST = 0.511`, dropping `N_c`):** the cliff **moves to exactly 0.511** (zero through A=0.511, ignites at 0.520). This is the load-bearing honesty check: **the cliff is the genesis gate, and its position is whatever `K_GENESIS` is set to** — confirming the *value* is calibration-conditional while the *structure* (sharp, wavelength-invariant ceiling at the gate) is robust. `genesis_override_sanity` (no override) reproduces the 1.533 cliff.

**L-independence:** the threshold is a **local** gate on `|J|`, so it is L-independent by construction; an L=64 static sweep confirms the same cliff (`engine/results/genesis_amp_static_L64`, local/gitignored).

---

## 3 · Why this is `[EMERGENT]`, not a theorem (the discipline)

- **Rule-made-visible.** The cliff is exactly the manifestation eligibility rule `|J| > K_GENESIS`. The campaign demonstrates that the registered coherent drive activates at that local threshold and then exhibits dynamical crest regulation. It does **not** establish a hard amplitude cap, an independent light law, or a QED-equivalent Schwinger mechanism.
- **The number is `[IMPOSED]`.** `K_GENESIS = N_c·K_MANIFEST` is an engine calibration; the kg-override run proves the cliff tracks it. The *dimensionless structural* statement (a sharp ceiling exactly at the genesis threshold, wavelength-invariant) is what the framework forward-predicts; `1.533` is conditional on the `M_REST = m_e` / `K_GENESIS = N_c·K_MANIFEST` register.
- **The wave onset is softer than the static gate** (propagation transient) — the static mode is the clean measurement; the wave mode is the dynamical "breaking."
- **Scope.** This is the genesis/manifestation sector (FTD-0274/0298 lineage), not the α or QM sectors. The secondary campaign modes (`--ladder` FTD-0110 convention test; `--thermal` EWSB `m(T)`; `--info` information-creation = FTD-0317; `--stamp`) are separate sub-questions.

---

## 4 · Non-promotion

`[EMERGENT / MEASURED — DERIVED-FROM-RULE]`. The surviving deliverable is a sharp, wavelength-invariant **genesis ignition threshold** in the registered coherent-wave arms, followed by measured collective crest regulation; the numeric threshold is calibration-conditional. The exact-amplitude-ceiling and Schwinger-equivalence readings are withdrawn. **No new theorem, no α, no promotion.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, and the algebraic spine are unchanged.
