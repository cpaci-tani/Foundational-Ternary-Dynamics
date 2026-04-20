# Engine Audit — Undefined-Boundary Reframe (2026-04-19)

Scope: `engine/include/`, `engine/src/`, `engine/cuda/`, `engine/wasm/`, `engine/web/js/`.
Method: targeted grep for trigger phrases (infinity tokens, "all sites", "L → ∞",
literal α, hidden couplings), spot-read the hits plus immediate call-sites.
**Flag, do not fix.**

## Executive summary

- Files audited (direct reads + grepped hit-lists): **~70 files** across the five roots;
  grep triggers touched ~400 files.
- HIGH risk findings: **3**
- MEDIUM: **6**
- LOW: **9**
- Parameter-free claim status: **CONDITIONAL**.
  - `ALPHA_EFT = G_C * G_C` identity is preserved everywhere in update-rule code
    (CPU + CUDA). `include/ftd/constants.h:127-132` holds, and `src/render_bridge.cpp`
    force-law sites use `ALPHA` with the explicit `// ALPHA == ALPHA_EFT (G_C² identity)`
    comment. No literal `1/137.036` sneaks into any update rule — all literal uses are
    in CODATA-comparison benchmarks, comments, or `test_falsifiability.cpp` /
    `test_gpu_parity_complete.cpp` assertions (valid external reference use).
  - CONDITIONAL, not SUPPORTED, because:
    (a) the EFT-program benchmark `benchmark_dynamical_sm.cpp` explicitly reports an
    `α_eff(L → ∞)` extrapolation whose semantics are no longer well-posed under the
    reframe (HIGH-1);
    (b) `ALPHA_S = 1.0` (strong-sector), `M_YUKAWA = 1.0`, `Q_LATTICE = 2.0`, and
    `LAMBDA_G = 100.0` are tagged `[IMPOSED]` in comments but show up in live update
    rules (MED-1…MED-3) — acceptable under reframe if they stay labeled, worth
    re-auditing as ontic inputs;
    (c) `K_B = 0.511` is a calibration anchor to `m_e` (MeV) — explicitly acknowledged
    in `ontic/particle_masses.h:31-32` — compliant, but callers that treat
    `m_e = K_B √(2π)(16/3)α¹¹` as a derivation need to be audited separately (out of
    scope here; flagged as LOW-8).

## HIGH risk findings

### HIGH-1 — `α_eff(L → ∞)` extrapolation is load-bearing under the reframe
- **File:** `engine/tests/benchmark_dynamical_sm.cpp:166-212, 289-308`
- **Excerpt:**
  ```cpp
  std::cerr << "  α_eff(L→∞) extrapolation: α_inf = " << cf.alpha_inf
            << "  (b/L² coefficient = " << cf.b_coeff << ")\n";
  …
  std::cout << "continuum,alpha_inf," << std::setprecision(10) << cf.alpha_inf << "\n";
  std::cout << "continuum,ratio_to_alpha_ref," << std::setprecision(6)
            << (cf.alpha_inf / ftd::ALPHA) << "\n";
  ```
  The struct holds `double alpha_inf = 0.0;` and `cf.alpha_inf = (sy - cf.b_coeff * sx) / n;`
  — a linear fit in `1/L²` extrapolated to `L → ∞`.
- **Issue:** Under undefined-boundary ontology the expression `L → ∞` is not a valid
  limit — only ε-L statements ("for every ε there exists L such that |α(L) − α*| < ε")
  are well-posed. The benchmark emits a CSV row `continuum,alpha_inf,…` and a ratio
  `α_inf / α_ref` that is the headline number cited in the EFT recovery program
  ("α_∞ plateau at ~3.6× α_ref"). The current script presents this as a point estimate
  of a completed-limit quantity.
- **Recommended action:** rename `alpha_inf` → `alpha_extrap` (or `alpha_largeL`),
  emit the fit plus its empirical-uncertainty band across the measured `L ∈ {32,48,64}`
  (or the Phase-F `{64,128,256,384}` GPU run), and re-state the headline claim as
  "α at the largest measured L was X, with 1/L² residual band [a,b]." Paper
  `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` text will need parallel
  revision.

### HIGH-2 — "infinite lattice" cited as justification for Green's-function form
- **File:** `engine/tests/test_einstein_equations.cpp:305-309`
- **Excerpt:**
  ```cpp
  // Theory: for a point mass M at origin on an infinite lattice,
  //   nabla^2 phi = 4*pi*G_N * rho
  // The Green's function gives:
  //   phi(r) = G_N * M_total / r   (for r >> lattice spacing)
  ```
- **Issue:** The comment derives the test's expected profile from the infinite-volume
  Green's function, then applies it on a periodic `L = 64` torus. Under the reframe
  the infinite-volume Green's function is a tool, not a background reality; the
  comment should say so, and the `r >> lattice spacing` qualifier should be joined
  by `and r << L/2` (periodic-image cutoff). The test itself already uses
  `r = 8..20` on `L = 64` so the code is correct; only the framing is off.
- **Recommended action:** reword comment to "on a sufficiently large finite lattice,
  for 1 ≪ r ≪ L/2, the free-space Green's function approximates phi(r) ≈ G_N M/r;
  periodic-image contributions dominate for r ≳ L/2." Note that lines 317-319
  already acknowledge periodic images — HIGH-2 is the doc string, not the logic.

### HIGH-3 — "lambda_G → infinity is exact constraint" as load-bearing claim
- **File:** `engine/include/ftd/lagrangian.h:50-52`
- **Excerpt:**
  ```cpp
  // Term 4: Gauss constraint  -lambda_G * (div J - rho_charge)^2
  // Enforces charge conservation. lambda_G -> infinity is exact constraint.
  inline constexpr double LAMBDA_G = 100.0;
  ```
- **Issue:** The comment references a completed-infinity limit ("lambda_G → infinity
  is exact constraint") as the justification for why a finite `100.0` is acceptable.
  Under the reframe the correct framing is a residual bound: "for every ε > 0 a
  sufficiently large finite LAMBDA_G drives ‖div J − ρ‖ < ε; LAMBDA_G = 100 gives
  the engine's measured residual."
- **Recommended action:** replace the comment with an ε-finite statement and cite
  the measured Ward-floor residual (Day-2 campaign: matched-stencil CG drives the
  floor from 1% → 1e-8). Value `100.0` is not itself flagged.

## MEDIUM risk findings

- **MED-1** — `engine/include/ftd/constants.h:205` `ALPHA_S = 1.0` is tagged
  `[IMPOSED]` in comments; confirm it is referenced only in Yukawa short-range
  strong-force update code (`cuda/kernels_forces.cu:486`, `src/atom/*`) and never
  appears as a "derived" value in a benchmark assertion.
- **MED-2** — `engine/include/ftd/constants.h:209` `M_YUKAWA = 1.0` and **MED-3**
  `engine/include/ftd/constants.h:214` `Q_LATTICE = 2.0` — both `[IMPOSED]` in
  comments; these are the two knobs tuning the strong sector to lattice scale.
  The reframe does not change their status but their pairing should be explicit
  (the three together form the "strong-sector toy regime").
- **MED-4** — `engine/src/atom/atom_forces.cpp:132-133, 161-162` literal `60.0` in
  LJ 10-12 force derivative: `F = eps * 60 * (sr12 - sr10) / r`. This is geometric
  (from d/dr of `5·sr¹² − 6·sr¹⁰` which gives `60·(sr¹²−sr¹⁰)/r`) — not a hidden
  coupling, but it would read more defensibly as a named constant
  `LJ_10_12_FORCE_COEFF = 60.0` with the derivation in the comment.
- **MED-5** — `engine/tests/campaign_dispersion.cpp:128, 182-184, 267-268` uses
  phrases "Continuum limit" and "Fill ENTIRE lattice uniformly in y,z". The
  second is a finite-L statement (fine), but DISP-6's "ω ≈ c·k within 5%" is
  an ε-L statement at a single L; under the reframe the printed header
  "Continuum limit" should be "Long-wavelength regime at L=" so readers don't
  infer a completed limit.
- **MED-6** — `engine/include/ftd/hilbert.h:165` "The continuum limit recovers the
  standard Schrodinger equation" — in a docstring describing engine code. Add
  "(ε-L: for a ≪ de-Broglie-wavelength on a box of linear size L ≫ spread of |ψ|²)"
  or at minimum cite the ε-L derivation doc.

## LOW risk findings

- **LOW-1** — `engine/include/ftd/scale.h:37` `BULK = 4, // Thermodynamic limit` as
  a scale-tag enum label; harmless tag but "Thermodynamic limit" now wants
  "Bulk regime (large-L asymptotics)".
- **LOW-2** — `engine/web/js/backgrounds/beyond.js:3` "an infinite lattice, with
  sparse flickering void points" — pure visual-aesthetic comment, no load bearing.
- **LOW-3** — `engine/web/js/ui/components/faq/data.js:356,378` FAQ text uses
  "continuum limit" in `[SELECTION]`-tagged answers; already appropriately tagged,
  but text should cite the ε-L form once for consistency with the audit.
- **LOW-4** — `engine/web/js/ui/components/faq/data.js:221-224` "The specific
  continuum limit that recovers QFT perturbation theory from FTD is a work in
  progress." — already hedged; fine.
- **LOW-5** — `engine/include/ftd/correlations.h:26` "Averaged over all directions
  and all sites for isotropy" (docstring). Code itself bounds by `lat.total_sites()`
  — change to "all sites of the finite lattice" for cleanliness.
- **LOW-6** — `engine/include/ftd/hilbert.h:107` `// Full Born probability
  distribution over all sites.` Code iterates `i < num_sites` which is finite;
  docstring wording only.
- **LOW-7** — `engine/include/ftd/eft/ward_identities.h:52` `rms_violation = …
  over all voxels` — bounded in code; wording only.
- **LOW-8** — `K_B = 0.511` in `include/ftd/ontic/particle_masses.h:32` is an
  explicit "practical simulation value" for `m_e` in MeV. Out-of-scope here, but
  call-sites that phrase `m_e = K_B · √(2π) · (16/3) · α¹¹` as a theorem need a
  separate theory-side audit; engine side is clean.
- **LOW-9** — `engine/src/ws_server.cpp:432` `while (true)` is a network `accept`
  loop; outside the reframe scope. `engine/tools/test_runner/src/NdjsonParser.cpp:26`
  `while (true)` is a CSV stream reader; also outside scope.

## Parameter-free check

Constants that appear in live update rules (CPU or GPU), with provenance:

| Constant | Value | Uses | Provenance | Status |
|----------|-------|------|------------|--------|
| `G_STAR` | 2.95868… | derived chain | Γ(1/4)/Γ(3/4) (lemniscatic) | DERIVED |
| `X_PLUS_PRECISION` | 137.035999177 | `ALPHA` seed | 4-term master-quadratic series | DERIVED |
| `ALPHA` | 1/X_PLUS_PRECISION | force code, Rayleigh | identity `= 1/x₊` | DERIVED |
| `G_C` | 0.0854245… | wave equation source, `ALPHA_EFT` | `sqrt(ALPHA)` by definition (not an independent coupling — `include/ftd/constants.h:111-115` says so explicitly) | IDENTITY |
| `ALPHA_EFT` | `G_C*G_C` | `particle_engine.cpp:139,198,275`, `kernels_forces.cu`, `ftd_wasm.cpp:278` | algebraic identity w/ `ALPHA` (static_assert guards drift) | DERIVED |
| `C_WAVE` | 1/√3 | stencil, Leapfrog | CFL on cubic lattice | GEOMETRIC |
| `DAMPING` | `ALPHA` | Rayleigh dissipation | `[IMPOSED]` tag at `gauge_couplings.h:194` | IMPOSED (acknowledged) |
| `G_N` | 0.01 | Poisson latency, `particle_engine.cpp` gravity | `1/(B_3+N_C)² = 1/100` + explicit TOY banner `gauge_couplings.h:69-97` | IMPOSED (toy, acknowledged) |
| `ALPHA_S` | 1.0 | Yukawa strong force (CUDA + atom) | `[IMPOSED]` at `constants.h:202-205` ("α_s ~ O(1) at Planck") | IMPOSED (acknowledged) |
| `M_YUKAWA` | 1.0 | strong-force range | `[IMPOSED]` at `constants.h:207-209` | IMPOSED (acknowledged) |
| `Q_LATTICE` | 2.0 | `alpha_s_lattice(r)` running | `[IMPOSED]` at `constants.h:211-214` | IMPOSED (acknowledged) |
| `LAMBDA_G` | 100.0 | Gauss penalty (Lagrangian-only) | comment says "→∞ exact"; see HIGH-3 | NEEDS REFRAMING |
| `LAMBDA_QCD` | (ontic) | CUDA `alpha_s_running` | from ontic chain | DERIVED |
| `H_BOND_EPSILON` | `K_B·α³` | H-bond force | comment `[DERIVED from α perturbation theory]` | DERIVED (via `K_B`) |
| `K_ANGLE` | `ALPHA*K_B` | angle strain | comment `[DERIVED]` | DERIVED (via `K_B`) |
| `R_EFF_DEFAULT` | 2.48 | Scale-1 particle radius | `constants.h:195-196` "from Phase 6 engine data" | EMPIRICAL (acknowledged) |
| `60.0` (LJ coeff) | 60 | `atom_forces.cpp:133,162` | geometric LJ 10-12 derivative | GEOMETRIC (MED-4: rename) |
| `OMEGA_LAMBDA` | 2/3 | cosmic scale (JS only) | `[THEOREM]` (Moore decomposition) | DERIVED |

No literal `137` or `1/137` detected in any update rule — all hits live in
benchmarks (`campaign_novel_predictions.cpp`, `test_constants.cpp`,
`campaign_integer_sweep.cpp`), CODATA comparison comments, or assertion tolerances.

## Cross-file observations

1. **Reframe-vulnerable "continuum limit" language is concentrated in the EFT and
   dispersion families.** `campaign_dispersion.cpp`, `benchmark_dynamical_sm.cpp`,
   `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (theory), and the docstrings of
   `spectral.h` and `hilbert.h` all use the phrase. Code is bounded; text is not.
   A single editorial sweep replacing "in the continuum limit" with the ε-L form
   (or marking it as "for all L ≥ L_ε such that…") would close MED-5, MED-6,
   and LOW-1/5/6/7 in one pass.

2. **`ALPHA_EFT = G_C * G_C` identity is enforced by `static_assert` and honored by
   every force path audited** (CPU `render_bridge.cpp`, CPU `particle_engine.cpp`,
   CUDA `particle_engine_gpu.cu`, CUDA `kernels_forces.cu`, WASM export
   `ftd_wasm.cpp`). JS `constants.js:61-62` mirrors it (`ALPHA = ALPHA_EFT`). No
   drift detected.

3. **Literal `137.036` or `137.035999177` appears only in benchmarks and audits.**
   Every occurrence grepped is either (a) a CODATA reference for a tolerance
   check, (b) an inline comment citing `1/α` for the reader, or (c) the declaration
   of `X_PLUS` / `X_PLUS_PRECISION` themselves. No silent calibration leaks
   detected.

4. **All lattice iteration is bounded** via `Lattice::size()` / `total_sites()`
   (`lattice.h:27-55`). The phrases "all sites" / "all voxels" are docstring
   shorthand for "all sites of the finite lattice" — rephrase for cleanliness,
   no correctness impact.

5. **`while(true)` loops** are confined to the WebSocket server and a test-runner
   NDJSON parser — both are network/IO, outside the physics engine, and out of
   scope for the reframe.

6. **Toy-banner read-through is weak.** `G_N = 0.01` carries its TOY banner in
   `gauge_couplings.h:69-97`, but callers (`test_einstein_equations.cpp`,
   `particle_engine.cpp`, the BH thermo benchmark, `campaign_gravity_hierarchy.cpp`)
   do not re-cite the toy framing in their own comments. Worth a follow-up
   pass: each consumer should print a one-line "TOY-regime gravity: G_N = 0.01"
   to stdout so benchmark readers see it.

## Recommendations (prioritized)

1. **Fix HIGH-1 first.** Rename `alpha_inf` / `cf.alpha_inf` across
   `benchmark_dynamical_sm.cpp` and any consumer scripts
   (`scripts/benchmarks/analyze_convergence.py`, the EFT paper) to `alpha_largeL`
   or `alpha_extrap`. Emit the fit *with* its 1/L² residual band. Update the
   EFT paper (`dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`) headline
   claim to "α at the largest measured L" rather than "α_∞".
2. **Rewrite the HIGH-3 comment** (`lagrangian.h:50-52`) as an ε-finite statement;
   no code change.
3. **Rewrite HIGH-2 comment** (`test_einstein_equations.cpp:305-309`) to make
   the finite-L regime explicit.
4. **Editorial sweep:** replace "continuum limit" with the ε-L form (or tag with
   a footnote citing `AUDIT_INFINITY_REFRAME.md`) in the six files flagged
   in MED-5/6 and LOW-1/3/5/6/7. Mechanical, low-risk, knocks out ~8 findings.
5. **Rename `60.0` → `LJ_10_12_FORCE_COEFF = 60.0`** in `atom_forces.cpp`
   (MED-4) with a one-line derivation comment. Geometric, but it currently
   reads as a magic number.
6. **Toy-banner read-through:** add a one-line `std::cout` or log entry to each
   gravity-consuming test/benchmark noting `G_N = 0.01` is toy-regime.
7. **(Out of scope, flagged for a separate audit.)** `K_B = 0.511` is a
   calibration anchor on the engine side; theory-side call-sites that phrase the
   chain `m_e = K_B · √(2π)·(16/3)·α¹¹` as a derivation need the parametric-
   insertion status checked against `CATALOG_PARAMETRIC_INSERTIONS.md`.

---
*Grep triggers used: `INFINITY`, `infinity`, `HUGE_VAL`, `numeric_limits<…>::infinity`,
`Infinity`, `isFinite`, `L -> infinity`, `thermodynamic limit`, `continuum limit`,
`infinite (lattice|torus|volume|space|domain)`, `all sites`, `all voxels`,
`every site`, `entire lattice`, `137.03`, `1/137`, `0.00729`, `7.2973`,
`ALPHA_EFT`, `G_C * G_C`, `ALPHA =`, `G_N`, `TOY`, `toy.regime`, `K_B =`,
`ALPHA_S =`, `exchange_force`, `H_BOND_EPSILON`, `LAMBDA_G`, `while\s*\(true\)`,
`for\s*\(;;\)`.*
