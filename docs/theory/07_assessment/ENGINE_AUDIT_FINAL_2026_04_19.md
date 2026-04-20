# Engine Final Audit — Session 4 (2026-04-19)

**Scope:** Residual sweep for completed-infinity language and reframe-stale framing in
engine subsystems NOT covered by Session 2 (`ENGINE_AUDIT_REFRAME.md`).

**Targets:** `engine/cuda/`, `engine/web/js/`, `engine/tests/*.cpp`, `engine/README.md`,
`engine/SPEC_ENGINE.md`, `engine/web/docs/USER_GUIDE.md`, `engine/CHECKLIST_PHYSICS.md`.

**Special concern:** verify whether engine-side documentation acknowledges the new
project-level calibration declaration `a_phys ≡ ℓ_P` (LEDGER FTD-0041, declared in
`docs/SPEC_FTD.md`).

---

## Summary

- **Files scanned:**
  - `engine/cuda/` 8 files (kernels + GPU engine + buffers)
  - `engine/web/js/` ~270 JS modules
  - `engine/tests/` 169 `.cpp` test files
  - 4 top-level docs (README, SPEC_ENGINE, USER_GUIDE, CHECKLIST_PHYSICS)
- **Residual completed-infinity hits worth flagging:** 7
  - HIGH: 0
  - MEDIUM: 4
  - LOW: 3
- **Calibration-acknowledgment status:** **NOT ACKNOWLEDGED** in any engine-side doc.
  See dedicated section below; recommend a one-paragraph banner in `SPEC_ENGINE.md`
  and a sentence in `engine/README.md`.

**Headline:** the CUDA layer is clean. The JS layer is clean (the previous audit was
thorough). Residual hits are concentrated in (a) one cosmetic-but-visible piece of
copy in the web dashboard ("infinite vortex line"), (b) one background-art docstring
("infinite lattice"), (c) FAQ entries that use "continuum limit" without an ε-L
restatement, and (d) test-file comments using "all sites" / "infinite mass" /
"continuum limit" as informal shorthand. None block correctness; the most
externally-visible item is the dashboard scenario tooltip.

---

## HIGH risk findings

**None.** Session 2 plus the load-bearing fixes already shipped have removed every
finding that I would call HIGH. The "in the L → ∞ limit" in
`benchmark_dynamical_sm.cpp:172,205` already explicitly disclaims itself ("NOT an
L → ∞ claim", "see undefined-boundary ontology"). Session 2 fixed the actual claim.

---

## MEDIUM risk findings

### M1. `engine/web/docs/USER_GUIDE.md:131` — "Infinite vortex line"
> `s0-field-vortex-line` — **Infinite** vortex line → watch (1/r) azimuthal flux circulation.

User-visible documentation. The scenario stamps a finite-cylinder approximation; the
"(1/r) circulation" pedagogy is fine but "Infinite" should be "Long" or
"Lattice-spanning" to match the undefined-boundary ontology. Same wording propagates
through `web/js/scales/scale0/scenario-registry.js:102` ("Vortex line"), but only the
USER_GUIDE.md copy says "Infinite" verbatim.

**Action:** rename "Infinite vortex line" → "Lattice-spanning vortex line" in
USER_GUIDE.md. No code change needed.

### M2. `engine/web/js/backgrounds/beyond.js:1-3` — "infinite lattice" in docstring
> /\*\*
>  \* "The Beyond" theme — fading grid extending outward, suggesting
>  \* an **infinite** lattice, with sparse flickering void points between lines.
>  \*/

Background-art module docstring. The phrase is rhetorical (it's literally describing a
fade-out gradient drawn into the renderer), but it is still verbatim "infinite
lattice" in a JS file shipped to the browser.

**Action:** rephrase to "an unbounded lattice" or "an indefinitely-extending lattice"
to match the project ontology. Trivial.

### M3. `engine/web/js/ui/components/faq/data.js:221,378` — "continuum limit" used as a load-bearing concept
- L221: "The specific continuum limit that recovers QFT perturbation theory from FTD is a work in progress."
- L378: "Integer-valued observables in the continuum limit are not a mystery..."

These FAQ entries treat "continuum limit" as a well-defined regime without the ε-L
restatement now mandated by the foundational reframe. L221 is honest about being WIP,
but the phrasing implies that the limit itself is well-defined; per the reframe it
needs to be stated as "a sequence of finite-L approximations satisfying ε-L
convergence bounds."

**Action:** soft rephrase. Replace "the continuum limit" with "the large-L
coarse-graining" or "the L → continuous-EFT regime (ε-L sense)" in both FAQ entries.

### M4. `engine/SPEC_ENGINE.md` and `engine/README.md` — no acknowledgment of `a_phys ≡ ℓ_P`
SPEC_ENGINE.md §391 documents `ALPHA_G_APPROX = 5.9e-39` (physical) vs `G_N = 0.01`
(engine), but nowhere notes that this discrepancy is a *calibration choice* declared
at the project level (LEDGER FTD-0041). README.md L150 says "FTD-derived constants
throughout (G_N=0.01, ...)" — calling G_N "FTD-derived" is technically correct (it is
derived from `1/(b_3+N_c)^2`) but does not mention that the engine's lattice spacing
is calibrated such that `a_phys ≡ ℓ_P`, which is what makes the dimensionless `0.01`
play the role of physical Newton's constant in the simulation.

**Action:** add a single calibration-acknowledgment paragraph (see "Calibration-
acknowledgment status" section below for proposed text).

---

## LOW risk findings

### L1. `engine/tests/campaign_dispersion.cpp:128` — "Continuum limit" as test-section title
> std::printf("\n--- DISP-6: **Continuum limit** ---\n");
> // DISP-6: Long-wavelength limit ω ≈ c·k

Test section heading. The actual check is "long-wavelength limit ω ≈ c·k within 5%"
on a finite L=N lattice — a perfectly legitimate finite-L observation. The header
prints "Continuum limit" to console, which is shorthand. Comment-level only.

**Action (optional):** rename console banner to "Long-wavelength regime" to match the
assertion text on L131.

### L2. `engine/tests/campaign_hydrogen_spectrum.cpp:1093` and `engine/tests/test_atomic_energy.cpp:329` — "infinite mass" in proton-locked comment
> // Inject proton at origin (locked -- **infinite** mass)

Standard atomic-physics shorthand for "fixed nucleus / Born-Oppenheimer". Not a
load-bearing claim about lattice infinities.

**Action (optional):** rephrase as "(locked — treated as fixed/non-recoiling)" for
ontology consistency. Cosmetic.

### L3. `engine/tests/test_larmor.cpp:351` — "Classical dipole predicts ratio → ∞"
> std::cout << "    (Classical dipole predicts ratio → ∞; we require > 1.5)\n";

Console message describing a continuum-theory prediction (the *theory* being compared
against, not an FTD claim). Acceptable as-is — this is a comparison to a non-FTD
theory's prediction, and the FTD assertion correctly states "we require > 1.5"
(finite).

**Action:** none. Acceptable.

---

## Items deliberately NOT flagged

The following were noticed but should not be flagged or fixed:

- All `Infinity` / `-Infinity` JS sentinel values for min/max reductions, distance
  searches, decay-rate "stable particle" markers (decay-rates.js), and CSS animation
  durations (`pulse 2s infinite`). These are software-engineering primitives, not
  ontological claims.
- `consciousness/walkthrough-steps.js:61-67` — already explicitly reframes von Neumann
  infinite regress as "terminates — not at infinity, but at a finite algebraic locus."
  This is a *good* reframe and serves as a model for other panels.
- `tests/campaign_novel_predictions.cpp:125` — uses `ℓ_P` correctly as the lattice
  spacing in a Lorentz-violation bound calculation. This is the *right* notation per
  FTD-0041.
- `benchmark_dynamical_sm.cpp:172,205` — already disclaims itself ("NOT an L → ∞
  claim, see undefined-boundary ontology"). Session 2 fix is in place.
- README.md L286 "Proton lifetime: Infinite" — this is the FTD prediction (proton is
  stable), not a lattice-extent claim. Fine.

---

## Calibration-acknowledgment status

Per LEDGER FTD-0041, the project now declares `a_phys ≡ ℓ_P` (engine lattice spacing
identified with the Planck length) as the calibration that makes engine constants
dimensionally meaningful. This converts `K_B = m_e ≈ 0.511` and `G_N = 0.01` from
"unexplained numbers" to "calibration outputs of a fixed unit choice."

**Status of engine docs:**

| Doc | Acknowledges `a_phys ≡ ℓ_P`? | Comment |
|-----|------------------------------|---------|
| `engine/SPEC_ENGINE.md` | **No** | §391 mentions `ALPHA_G_APPROX` vs `G_N=0.01` discrepancy but does not name the calibration |
| `engine/README.md` | **No** | L150 says "FTD-derived constants throughout (G_N=0.01, ...)" without unit-system caveat |
| `engine/CHECKLIST_PHYSICS.md` | Partial | L68 has `[IMPOSED]` row in epistemic-tags table but no `K_B`/`G_N`-specific mention |
| `engine/web/docs/USER_GUIDE.md` | **No** | No mention |
| `engine/include/ftd/ontic.h` (header doc) | **No** | (Per Session 2 audit, header has been split; theme-headers do not currently include calibration prose) |

**Recommended one-paragraph banner** for `SPEC_ENGINE.md` (insert near §107 "FTD
constants" line):

> **Calibration discipline (LEDGER FTD-0041, 2026-04-19):** the engine identifies its
> lattice spacing with the Planck length, `a_phys ≡ ℓ_P`. Under this identification
> `K_B = m_e ≈ 0.511` (manifestation amplitude → electron rest energy) and
> `G_N = 1/(b_3+N_c)^2 = 0.01` (effective gravitational coupling) are calibrations of
> the discrete dynamics to physical units, not free parameters. The dimensionless
> values are derived from the framework integers; the unit choice is `[IMPOSED]`.
> See `docs/SPEC_FTD.md` and `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`.

A single sentence pointing to this banner from `engine/README.md` L150 and
`engine/web/docs/USER_GUIDE.md` is sufficient.

---

## Recommended actions (prioritized)

1. **(MEDIUM, cosmetic-but-visible)** Rename "Infinite vortex line" → "Lattice-
   spanning vortex line" in `engine/web/docs/USER_GUIDE.md:131`. **5 minutes.**

2. **(MEDIUM, ontology hygiene)** Add the calibration-acknowledgment banner to
   `engine/SPEC_ENGINE.md` near §107 and a one-line pointer in `engine/README.md`
   near L150. **10 minutes.**

3. **(MEDIUM, ontology hygiene)** Rephrase `engine/web/js/backgrounds/beyond.js:3`
   docstring: "infinite lattice" → "unbounded lattice". **2 minutes.**

4. **(MEDIUM, FAQ pedagogy)** Soft-rephrase the two FAQ entries
   `engine/web/js/ui/components/faq/data.js:221,378` to use "large-L coarse-graining"
   instead of "continuum limit", or to attach an explicit ε-L caveat. **10 minutes.**

5. **(LOW, optional)** Rename `campaign_dispersion.cpp:128` test banner
   "Continuum limit" → "Long-wavelength regime" to match its own assertion text. **2
   minutes.**

6. **(LOW, optional)** Rephrase "infinite mass" → "fixed/non-recoiling" in two test
   files (`campaign_hydrogen_spectrum.cpp:1093`, `test_atomic_energy.cpp:329`). **2
   minutes each.**

---

## Conclusion

The engine layer is in good shape after the previous two audits. The only externally-
visible reframe-stale string is the "Infinite vortex line" tooltip in the dashboard
USER_GUIDE. The most important *missing* artifact is calibration-discipline
acknowledgment (`a_phys ≡ ℓ_P`) in any engine-side document. Recommend bundling all
six items above into a single small commit (~30 minutes total).

No HIGH-risk residue remains.

---

**Files referenced (absolute paths):**
- `C:/Users/cpaci/Desktop/ftd/engine/web/docs/USER_GUIDE.md`
- `C:/Users/cpaci/Desktop/ftd/engine/web/js/backgrounds/beyond.js`
- `C:/Users/cpaci/Desktop/ftd/engine/web/js/ui/components/faq/data.js`
- `C:/Users/cpaci/Desktop/ftd/engine/SPEC_ENGINE.md`
- `C:/Users/cpaci/Desktop/ftd/engine/README.md`
- `C:/Users/cpaci/Desktop/ftd/engine/CHECKLIST_PHYSICS.md`
- `C:/Users/cpaci/Desktop/ftd/engine/tests/campaign_dispersion.cpp`
- `C:/Users/cpaci/Desktop/ftd/engine/tests/campaign_hydrogen_spectrum.cpp`
- `C:/Users/cpaci/Desktop/ftd/engine/tests/test_atomic_energy.cpp`
- `C:/Users/cpaci/Desktop/ftd/engine/tests/test_larmor.cpp`
- `C:/Users/cpaci/Desktop/ftd/engine/tests/benchmark_dynamical_sm.cpp` (already-fixed reference)
- `C:/Users/cpaci/Desktop/ftd/engine/web/js/consciousness/walkthrough-steps.js` (good-reframe reference)
