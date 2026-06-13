# PRE-REGISTRATION — Hydrogen-like bound-state spectroscopy on the FTD lattice (FTD-0278 Leg 1)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-12
**LEDGER id (reserved):** FTD-0278 (Leg 1 — operator spectroscopy)
**Git tag (to be applied at lock):** `preregister-hydrogen-lattice-spectrum-v1`
**Program:** the "hydrogen in the engine" arc (the biggest-win target identified from the
emergent-physics roadmap; Leg 0 = guidance no-go, committed `fe05c473`).
**Result class (declared in advance):** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` +
lattice correctness. KG-in-a-Coulomb-well is textbook; the FTD content is that **every
ingredient except the imposed register is the engine's own exact machinery**. This is
NEVER to be headlined as "FTD derives QM/Schrödinger/hydrogen" (FC-1 stands; the
FTD-0270 unconditional dispersion boundary stands).

---

## §1 · Question

GIVEN the de Broglie clock (FTD-0271) and a scalar-potential coupling of the clock to
the engine's own Gauss potential, does the flux field bound in the engine's exact
lattice Coulomb well produce a hydrogen-like spectrum — the right level structure, the
right O_h multiplets, and a level spacing that (i) matches an ideal 1/r reference at the
same lattice and (ii) approaches the continuum Rydberg as the Bohr radius grows?

## §2 · The imposed register (motivated; nothing silently promoted)

| Input | Value | Motivation |
|---|---|---|
| clock scalar ω₀ | 1.5 rad/tick | FTD-0271 `[IMPOSED]`; rest mass exists (M_REST); covariant rate FTD-native (FTD-0252/0271-A5); 1.5 < 2/dt stability; chosen (declared) for tachyon-guard headroom |
| scalar-potential coupling | ω_eff²(r) = ω₀² + 2ω₀V(r) | the engine's own structural move for gravity (latency modulates the local clock rate, `transmutation_phases.cpp`); applied to the Gauss φ. Schrödinger limit: E = c²k²/(2ω₀) + V |
| well charges q | {1.1170, 0.9308, 0.6981} | Schrödinger-limit Bohr radii a₀ = {2.5, 3, 4} lattice units (α=1/137 would put a₀ off-lattice; hydrogen-like-ion scaling, declared); all obey the tachyon guard q < ω₀/(2\|φ_G(0)\|) |

## §3 · Engine-exact ingredients (theorems; not imposed)

- 18-pt O_h Laplacian `L₁₈` (the engine `phase_read` stencil); symbol `M(k)` verified to machine precision (gate G-1).
- Lattice Coulomb potential `V = +q·φ_G`, with `φ_G` the **mean-free periodic Green's function** of L₁₈ — the engine's own FFT Gauss solution for a unit point charge (OT-1.4 / Phase-G `[THEOREM]`). No offset manipulation; **all falsifier observables are energy GAPS (offset-invariant)**.
- The spectroscopy operator: `A = −c²L₁₈ + 2ω₀V` (sparse symmetric); engine carrier frequencies `ω_n = √(ω₀² + a_n)`; envelope energies `E_n = ω_n − ω₀`.
- Reference (diagnostic, not FTD): periodized continuum `−q/(4πr)` with identical core regularization + identical mean-free convention at the **same L** — isolating "engine potential vs ideal 1/r" from torus truncation (which cancels in the ratio).

## §4 · Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/derive_hydrogen_lattice_spectrum.py` | `8e953fac6b7dc251c21290f6e21d416c6e2a9d0e78d923a94e8953c73654573f` |

The script's `record_run()` encodes the §6 verdict logic and the frozen §5 grid; the
run of record is `--record` (mechanical; no parameter choices at run time).

## §5 · Run of record (frozen)

```
python scripts/exploration/derive_hydrogen_lattice_spectrum.py --record \
    --out scripts/exploration/results/hydrogen_spectrum_2026-06-12.csv
```

Grid: ω₀ = 1.5; q ∈ {1.1170, 0.9308, 0.6981}; L ∈ {48, 64}; k = 10 lowest eigenvalues
per cell; both lattice and 1/r-reference potentials; control grid L ∈ {12,16,20,24,32}
massless Dirichlet (the FTD-0270 protocol).

**Prior information (disclosed):** development runs (this session, recorded in git
history before this lock) measured at (q ∈ {1.1170, 0.9308}, L ∈ {48, 64}):
lattice/ref gap ratios {1.021, 1.016, 0.989, 0.996} and Rydberg ratios 1.96 (a₀=2.5)
→ 1.44 (a₀=3.0). The a₀ = 4.0 cells and the F-B monotonicity/endpoint band have NOT
been run under the final (mean-free) convention — F-B is the genuinely blind leg. The
F-A tolerance (±0.05) is set ~2.5× the largest observed deviation, declared before the
record run.

## §6 · Frozen falsifiers and verdict logic (encoded in `record_run()`)

- **G-1 (operator correctness):** periodic eigenvalues match M(k) to < 1e-10.
- **F-A (the engine potential is Coulombic):** lattice/1/r-reference gap12 ratio within
  **1 ± 0.05** in **all 6** grid cells, where gap12 = mean(E₂..E₅) − E₁ (the n=2
  multiplet gap; offset-invariant).
- **F-B (Rydberg approach — the blind leg):** at L = 64, the Rydberg ratio
  gap12/(¾R) is **strictly decreasing across a₀ = {2.5, 3, 4}** AND the a₀ = 4 value
  lies in **(1.0, 1.40)** — i.e. the spectrum approaches the continuum Rydberg from
  above as discretization weakens.
- **F-C (O_h multiplet structure):** in every cell, the T1u triple (states 3–5) is
  internally degenerate to ≤ 5% of gap12, and the A1g–T1u splitting is ≤ 50% of gap12
  (the n=2 quadruple is a recognizable hydrogen multiplet, lattice-split as O_h requires).
- **F-E (causal control):** the massless Dirichlet ground mode reproduces FTD-0270's
  linear scaling, s ∈ [0.8, 1.2].

**Verdict:** HYDROGEN-CONFIRMED iff G-1 ∧ F-A ∧ F-B ∧ F-C ∧ F-E; PARTIAL if
G-1 ∧ F-A ∧ F-C but ¬F-B; CLOSED-NEGATIVE otherwise.

## §7 · Pre-declared exclusions (banned moves)

1. No post-hoc band adjustment; no re-running with a different ω₀/q/L grid to move a
   verdict (protocol changes require v2).
2. No absolute-energy comparisons (offset-convention-dependent); gaps only.
3. No claim of unconditional derivation: the verdict conditions on the §2 register.
   FTD-0270's "atomic spectra NOT substrate-derivable" stands for *unmodified* FTD.
4. HYDROGEN-CONFIRMED does not promote FTD-0013, MC-T4.3, or any LEDGER row; it lands
   as a new `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` row (FTD-0278).
5. The q² continuum scaling is **not** a falsifier (the lattice folds discretization
   running into the apparent exponent — measured in development, disclosed); the same-L
   reference ratio (F-A) carries that comparison honestly.
6. Leg 2 (engine time-series spectroscopy with the `db_clock_coulomb` toggle) is a
   separate deliverable under the same FTD-0278 row with its own cross-check falsifier
   (engine FFT peaks vs this leg's ω_n = √(ω₀² + a_n) at the same (L, q)); it is not
   gated by this lock.

## §8 · Honest ceiling

Even HYDROGEN-CONFIRMED yields: "GIVEN the clock + the scalar-potential coupling, the
engine's exact lattice machinery produces a hydrogen-like bound-state spectrum." It
does NOT derive ω₀ (no ℏ in the substrate), does NOT make the coupling native (the
flux wave does not feel φ without it — that absence is measured engine fact), and does
NOT touch the lab-atomic-physics identification (no calibration of R to eV is claimed).

## §9 · Hash-lock declaration

This document and the §4 artifact are committed together; the commit is tagged
`preregister-hydrogen-lattice-spectrum-v1` BEFORE the §5 run executes. Any post-lock
edit to §§2, 4–7 or the artifact invalidates the lock and requires a v2.
