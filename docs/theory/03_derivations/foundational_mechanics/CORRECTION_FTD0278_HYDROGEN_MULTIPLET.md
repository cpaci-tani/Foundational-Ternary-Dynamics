# CORRECTION — FTD-0278 hydrogen: the "n=2 multiplet / Rydberg ladder" was overclaimed

**Tag:** `[CORRECTION]` (honesty correction to a merged result; narrows the claim, retracts one falsifier)
**Date:** 2026-06-12
**Trigger:** a 4-lens adversarial audit workflow (run on the committed FTD-0278 + FTD-0279
artifacts) flagged, at high confidence from two independent lenses (physics + numerics),
that the hydrogen "n=2 multiplet" is dominated by free-torus continuum modes, not bound
2s/2p orbitals. **Independently verified here.** This corrects the
[`ANALYSIS_HYDROGEN_LATTICE_SPECTRUM_v1.md`](ANALYSIS_HYDROGEN_LATTICE_SPECTRUM_v1.md)
F-C claim and rescopes the verdict; it does **not** retract the genuine result (a
Coulombic 1s ground state). FTD-0279 (helium) is **unaffected** (ground-state SCF only).

---

## 1 · The finding, independently verified

Running the locked `derive_hydrogen_lattice_spectrum.py` operator at the record cells:

| q (a₀) | L | n_bound | E[0..4] |
|---|---|---|---|
| 1.1170 (2.5) | 48 | **2** | −0.0260, −0.0011, +0.0006, +0.0006, +0.0006 |
| 1.1170 (2.5) | 64 | **5** | −0.0274, −0.0022, −0.0007, −0.0007, −0.0007 |
| 0.9308 (3.0) | 48 | **2** | −0.0124, −0.0000, +0.0011, … |
| 0.9308 (3.0) | 64 | **2** | −0.0135, −0.0006, +0.0001, … |
| 0.6981 (4.0) | 48 | **1** | −0.0046, +0.0004, +0.0016, … |
| 0.6981 (4.0) | 64 | **1** | −0.0054, +0.0001, +0.0006, … |

**Two record cells (a₀=4) have only n_bound=1 — only the 1s is bound; E[1:5] are positive
(continuum).** A genuinely bound n=2 quadruple appears **only** at the deepest cell
(a₀=2.5, L=64, n_bound=5), and even there the n=2 states are severely under-bound
(E[1]/E[0] = 0.08, vs the Rydberg 2s/1s = 0.25) — the finite lattice does not reproduce
the 1/n² ladder.

**F-C is non-probative (symmetry-protected).** Re-running the operator with a *repulsive*
core (V → −V, zero bound states), the "T1u triple" internal spread is 7.3×10⁻⁵ —
essentially as degenerate (2.2×10⁻¹⁶ for the attractive case). The exact T1u degeneracy
is the **cubic-symmetry degeneracy of the periodic box** for *any* central potential, not
evidence of Coulomb-bound 2p orbitals. **F-C as written passes even with no binding.**

## 2 · What is RETRACTED

- The headline "the n=2 quadruple appears in every cell as A1g + an EXACTLY degenerate
  T1u triple — the hydrogen n=2 multiplet, split precisely as O_h representation theory
  requires" is **WITHDRAWN.** The triple's degeneracy is torus momentum degeneracy, not
  a Coulomb 2p multiplet; in most record cells those states are unbound.
- **Falsifier F-C is retracted as non-probative.** It cannot fail for a central potential
  and is removed from the evidence basis.
- The "Rydberg LADDER" framing (multiple levels in a 1/n² pattern) is **withdrawn** — the
  finite lattices bind at most the 1s cleanly; the n≥2 ladder is not reproduced.

## 3 · What STANDS (the genuine, narrowed result)

The substance survives, correctly relabeled as a **ground-state** result:

- **The 1s ground state is genuinely bound in every cell** (E[0] < 0, the continuum edge
  is at 0), deepening monotonically with well depth q.
- **F-A stands, reinterpreted:** `gap12 = mean(E[1:5]) − E[0]` is **1s-dominated** (E[0] is
  by far the most negative term), so `gap12 ≈ |E_1s|`. F-A (lattice/1/r ratio of gap12 ∈
  [0.952, 1.021]) therefore says **the engine's own Gauss potential binds the 1s as
  deeply as the ideal 1/r at the same lattice** — a valid Coulombic-binding statement
  about the ground state.
- **F-B stands, reinterpreted:** the monotone "Rydberg ratio" 1.975 → 1.447 → 1.138 is the
  **1s binding energy approaching the continuum-Rydberg scale** as a₀ grows off the
  lattice — a ground-state convergence statement, **not** a 1s→2p line.
- **F-E (causal control) stands.**

**Corrected verdict: HYDROGEN-1s-CONFIRMED** — GIVEN the clock + the scalar-potential
coupling, the engine's exact lattice Coulomb well binds a **1s ground state** whose energy
is Coulombic (binds like ideal 1/r, F-A) and approaches the continuum Rydberg scale as the
well/box grow (F-B). The excited (n≥2) hydrogen states are **not** bound on these finite
lattices (a real BOUNDARY: finite-box discretization under-binds the diffuse n≥2 orbitals;
a genuine n=2 multiplet needs larger L / shift-invert eigensolves — queued).

## 4 · FTD-0279 (helium) is unaffected

Helium's screening σ and ionization ratio are **ground-state SCF observables** (the closed
1s² shell + the Hartree repulsion); they use no excited bound states. The 1s ground state
is robustly bound (§3), so HELIUM-CONFIRMED stands. (Separately noted: the helium
ionization target 0.4519 is the *experimental* He ratio; the HF-level ratio is ~0.459 —
within the frozen (0.30, 0.60) band either way, so the verdict is unchanged.)

## 5 · Minor doc corrections (also from the audit)

- `derive_hydrogen_lattice_spectrum.py` docstring: "V(r) = −q·φ_G" contradicts the code
  (`coulomb_well: V = +q·φ`); the code is right (φ_G(0) < 0 ⇒ +q·φ_G is the attractive
  core). Docstring to be fixed.
- Tachyon-guard constant: "|φ_G(0)| ~ 0.22 ⇒ q < 2.27" is numerically wrong; measured
  |φ_G(0)| ≈ 0.30–0.31 (grows with L) ⇒ q < ~1.6 at ω₀=1 (the runtime guard is the exact
  eigenvalue test, so no result is affected). Docstring to be fixed.

## 6 · Process note

This correction is the adversarial-audit discipline working as intended: a multi-lens
workflow caught an overclaim in a *merged* result, the finding was independently
re-verified before action, and the claim was narrowed to exactly what the data support.
The Number-One-Goal second clause ("establish what we cannot derive") is served: the
finite-lattice **n≥2 binding boundary** is now mapped. **Nothing was promoted; one
headline and one falsifier are withdrawn; FTD-0270's unconditional verdict and FC-1
stand; the engine Leg-2 demonstration (in progress) targets the 1s ground state — the part
that survives.** A v2 pre-registration that adds a binding/localization gate and pushes to
larger L (to test whether a real n=2 multiplet emerges) is the honest next step, queued.
