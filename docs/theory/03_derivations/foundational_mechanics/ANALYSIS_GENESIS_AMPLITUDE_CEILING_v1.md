# Analysis — Is there a maximum amplitude for coherent light in FTD? The genesis amplitude ceiling (FTD-0316)

**Tag:** `[EMERGENT / MEASURED — DERIVED-FROM-RULE]` for the phenomenon; the *numeric* ceiling is `[IMPOSED]`-calibration-conditional. **NOT** a `[THEOREM]`, **NOT** a new physics derivation.
**Lock (in-source pre-registration):** the three outcomes + the two numeric predictions were registered in the campaign header *before* running — `engine/tests/campaign_genesis_amplitude_ceiling.cpp:27-34` (outcomes 1/2/3), `:20` ("threshold: exactly A = K_GENESIS"), `:23` ("wavelength-invariance: NO"). Golden gate **certified green** — the three runtime override knobs (`genesis_threshold_override` / `manifest_scale_override` / `manifest_use_temperature`) are golden-neutral by construction (defaults reproduce compile-time `K_GENESIS`/`K_MANIFEST` byte-for-byte); `test_render_bridge_golden` **rebuilt from source + PASS** (`0xb604d81a3d79366e`). Read-only on physics phases.
**Precedence:** LEDGER > this doc.

---

## 0 · Verdict

> **OUTCOME (1) — CONFIRMED.** A coherent flux wave has a **sharp, wavelength-invariant amplitude ceiling at exactly `|J| = K_GENESIS`**: genesis is *identically zero* for antinode amplitude `A ≤ K_GENESIS` and ignites for `A > K_GENESIS`, with the same cliff at every wavelength tested. Above it the wave "breaks" — the void-field peak pins near `K_GENESIS` while manifested matter accumulates. This is the **lattice analogue of the QED Schwinger limit** (field too strong ⇒ the vacuum sheds it into matter).

**The honest reading (mandatory, not buried):** the ceiling is the engine's own genesis rule (`|J| > K_GENESIS ⇒ manifestation drain`) **made visible** under a coherent drive — it is `[DERIVED-FROM-RULE]`/`[EMERGENT]`, **not an independent dynamical surprise and not a theorem.** And the *number* 1.533 is **`[IMPOSED]`-conditional**: `K_GENESIS = N_c·K_MANIFEST = 3·0.511`, and the override control shows the cliff simply **tracks whatever `K_GENESIS` is set to** (drop `N_c` → `kg=0.511` → cliff at 0.511). So the **structure** (a sharp, wavelength-invariant ceiling at exactly the genesis threshold) is the forward content; the **value** inherits the genesis calibration.

**Nothing promoted.** `x₊ = 1/α` stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; golden gate green.

---

## 1 · The experiment + pre-registration

Light in FTD is a wave in the **continuous** flux field `J` (FTD-0298), which carries no axiom-level amplitude cap. But genesis fires when a void site reaches `|J| > K_GENESIS`, and manifestation **drains** the flux back to threshold (`flux *= max(0, 1 − K_GENESIS/|J|)`). So a coherent flux wave should be capped at `|J| = K_GENESIS`. The campaign drives a **coherent transverse plane wave** `J = (0, A·sin(kx), 0)` (divergence-free ⇒ survives Gauss projection; antinodes reach `|J| = A`), Langevin OFF, dual-substrate OFF, stack = wave-propagation + Gauss + genesis. **Three pre-stated outcomes** (`:27-34`): (1) sharp `K_GENESIS`-locked, wavelength-invariant threshold → a forward prediction; (2) smeared/unlocked crossover → a boundary; (3) no triggering at all → a sharper boundary.

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

- **Rule-made-visible.** The cliff is exactly the manifestation rule `|J| > K_GENESIS ⇒ drain`. The campaign *demonstrates* that this local rule implies a coherent-light amplitude ceiling and breaking-wave dynamics — a real, clean `[EMERGENT]` consequence — but it is **not** an independent dynamical law and **not** a `[THEOREM]`. Calling it a derivation of a new constant would be the overclaim to avoid.
- **The number is `[IMPOSED]`.** `K_GENESIS = N_c·K_MANIFEST` is an engine calibration; the kg-override run proves the cliff tracks it. The *dimensionless structural* statement (a sharp ceiling exactly at the genesis threshold, wavelength-invariant) is what the framework forward-predicts; `1.533` is conditional on the `M_REST = m_e` / `K_GENESIS = N_c·K_MANIFEST` register.
- **The wave onset is softer than the static gate** (propagation transient) — the static mode is the clean measurement; the wave mode is the dynamical "breaking."
- **Scope.** This is the genesis/manifestation sector (FTD-0274/0298 lineage), not the α or QM sectors. The secondary campaign modes (`--ladder` FTD-0110 convention test; `--thermal` EWSB `m(T)`; `--info` information-creation = FTD-0317; `--stamp`) are separate sub-questions.

---

## 4 · Non-promotion

`[EMERGENT / MEASURED — DERIVED-FROM-RULE]`. The deliverable is a clean, honestly-scoped forward prediction of the **genesis sector**: FTD's manifestation rule implies a sharp, wavelength-invariant coherent-light amplitude ceiling at `K_GENESIS` (the lattice Schwinger-limit analogue), with the numeric value calibration-conditional. **No new theorem, no α, no promotion.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, the algebraic spine — all unchanged. Golden gate certified green (`0xb604d81a3d79366e`).
