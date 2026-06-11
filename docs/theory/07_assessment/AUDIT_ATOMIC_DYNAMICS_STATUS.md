# Audit — Atomic Dynamics in FTD: Honest Derivation-Status Map

**Tag:** `[SYNTHESIS]` (consolidation; promotes nothing)
**Date:** 2026-06-11
**Scope:** every ingredient required to compute an atom's quantum dynamics, and FTD's actual derivation status for each.
**Why this exists:** an external "derive the helium spectrum" challenge prompted a full audit. This document is the canonical, reviewer-facing answer to *"how much of atomic physics does FTD derive from the substrate?"* — so the question can never again be answered by an over-tagged benchmark comment.

---

## 0 · One-line verdict

**FTD derives ~0% of atomic quantum dynamics from its substrate, and this is by declared design, not by oversight.** The framework's constitution (FC-1) formally **declines** recovering the Schrödinger equation, the continuous Born rule, and Hilbert space. What exists in the codebase under "atoms" is classical mechanics + classical electromagnetism + parametrized chemistry + empirical lookup. The honest framing of every atomic result is *"standard physics formula, with FTD's constants inserted,"* never *"derived from the lattice."*

---

## 1 · The status table

| Ingredient an atom's dynamics needs | FTD status | Evidence |
|---|---|---|
| Coulomb potential (electrostatics) | `[THEOREM]` for the lattice form; classical | Phase-G geometric Coulomb = lattice Poisson Green's function (FTD-0004); `poisson_solvers.cpp` |
| **Quantum of action ℏ** | **ABSENT / `[DECLINED]`** | No ℏ constant anywhere (`constants.h`); appears only in a units comment. FC-1 declines deriving it. |
| **Quantum kinetic operator** `−ℏ²/2m ∇²` | **ABSENT** | Kinetic energy is classical `½mv²` (`particle_engine.cpp`, `diagnostics_compute.cpp`). The flux wave equation is classical (2nd-order in time, `phase_write.cpp`). |
| **Discrete / quantized energy levels** | **ABSENT** | No Bohr model, no Rydberg derivation, no spectral lines anywhere in the corpus. |
| Hydrogen "spectrum" benchmark | classical reproduction, **not** a derivation | `campaign_hydrogen_spectrum.cpp` places classical Kepler orbits at `r_n=n²a₀` and checks `1/n²` scaling (generic to any `1/r` force); the Rydberg scale is calibrated via `ALPHA`/`K_B`. |
| Born rule (probabilities) | `[SELECTION]` (form) + `[OPEN]` (proportionality) | `DERIV_QM_FROM_LATTICE.md`; LEDGER FTD-0187. The `\|ψ\|²` *form* is selected; the step `probability = energy density` is asserted, not derived. FC-1 declines the continuous Born rule. |
| Electron spin = ½ | `[SELECTION]` | `DERIV_SPIN_STATISTICS_BRIDGE.md`: ℤ₂ topology of lemniscates motivates spin-½, but the value (vs 3/2, …) is not forced. |
| **Pauli exclusion (multi-electron)** | `[CONJECTURE]` / ABSENT | The "ternary forbids s=±2" point is single-site saturation, not multi-electron antisymmetry. Quantitative Fermi-Dirac is `[CONJECTURE]` (`DERIV_SPIN_STATISTICS_BRIDGE.md` §5.3). |
| **Exchange energy** (ortho/para splitting) | **ABSENT** | `exchange_force` toggle is a **no-op on CPU** (`phase_forces.cpp:200`) and a parametrized stub on GPU. No derivation. |
| Fine structure / spin-orbit | **ABSENT** | `spin_orbit` toggle is a stub (`particle_engine.h`); not computed. |
| Hyperfine | **ABSENT** | Not addressed anywhere. |
| Lamb shift "0.23%" | `[PARAMETRIC]` | Standard QED one-loop (Mohr + Uehling VP) with FTD's α inserted (`CATALOG_PARAMETRIC_INSERTIONS.md`; `proof_complete_sm.py`). The formula is textbook QED; α is itself `[STRONGLY MOTIVATED CONJECTURE]`. |
| Orbital shapes (s,p,d,f) | EMPIRICAL / visualization | `orbitals.js`: real hydrogenic wavefunctions + Slater screening, applied universally to all Z, **tuned for display**. Not a multi-electron physics claim. |
| Slater shielding / effective nuclear charge | EMPIRICAL | Slater's 1930 rules verbatim (`quantum-chemistry.js`); hard-coded constants. |
| Atomic radius | EMPIRICAL | `R = R_BOHR / Z^{1/3}` (Thomas-Fermi screening, borrowed). |
| Periodic table / electron configuration / valence / bonds | EMPIRICAL lookup | 118-element hard-coded tables (`elements.js`, `atom_engine.h`); Aufbau/Hund not computed. |
| Inter-atomic forces (ionic/vdW/covalent) | `[PARAMETRIC]` | Classical Coulomb / Lennard-Jones / harmonic-spring with FTD constants (`atom_forces.cpp`). |
| Multi-electron atomic structure (Li, C, …) | ABSENT | No Hartree-Fock / SCF / configuration interaction. |

**Tag legend:** DERIVED (from axioms by an explicit chain) · SELECTION (argued, not forced) · PARAMETRIC (standard formula + FTD numbers) · CONJECTURE (proposed, unvalidated) · OPEN (unresolved) · ABSENT (not present) · DECLINED (FC-1 explicitly opts out).

---

## 2 · The structural reason (not just "ℏ is missing")

FTD's flux field `J` obeys a **classical wave equation, 2nd-order in time** (leapfrog integration in `phase_write.cpp`): `∂²J/∂t² = c²∇²₁₈J`, giving **linear dispersion ω ∝ |k|** (EM/cavity-like). The hydrogen Rydberg `1/n²` comes from the **Schrödinger operator's quadratic dispersion ω ∝ k²** (1st-order in time). These are structurally different. So even setting ℏ aside, FTD's substrate dynamics has the **wrong dispersion law** to produce atomic spectra. This is the precise boundary that FTD-0270 (`PREREG_QUANTIZATION_LATTICE_MODES_v1`) is designed to measure and pin.

Equally decisive: in FTD the electron is a **manifested cluster** (state field `s ∈ {−1,0,+1}`), not a wavefunction. A bound electron orbits **classically** (the Kepler benchmark). There is no electron wave whose standing modes could be the orbital energy levels.

---

## 3 · What this is, and is not

- **It is not a scandal.** FTD never targeted quantum chemistry; its north star is a discrete-ontology + rigorous-algebra program with an *emergent/statistical* reading of QM (`DERIV_QM_FROM_LATTICE.md`, CLOSED DECLINED under FC-1). The corpus tags everything atomic honestly already; this document just makes the absence explicit in one place.
- **It is the correct answer to the helium challenge.** "Derive the helium spectrum from your lattice" cannot be met, because helium needs a quantum kinetic operator + fermionic exchange + multi-electron correlation — none of which the substrate generates. Producing a helium number would require importing the quantum machinery wholesale, which is exactly the move that loses the argument.
- **The winnable fronts are elsewhere:** the algebraic spine (theorem-grade math), the honestly-mapped boundaries (α dynamical / MC-T4.3; the N(A) calibration / FTD-0269; and now the atomic-dispersion boundary / FTD-0270), the QM-foundations critique (FC-1, the Bell/measurement-independence line), and the structural deviation-prediction ledger (FTD-0258). Atomic-spectra precision is QM's home turf and is not a front FTD can win.

---

## 4 · Corrections applied alongside this audit

To keep the codebase framings consistent with this map, the following were de-overclaimed in the same pass (Track A of the FTD-0270 arc):
- `campaign_hydrogen_spectrum.cpp` — header clarifies it is a classical Kepler/virial `1/n²` check, not an eigenvalue derivation.
- `engine/web/js/orbitals.js` — docstring clarifies the clouds are a hydrogenic-wavefunction visualization, not a multi-electron physics claim.
- `CLAUDE.md` / `README.md` — the "hydrogen 1/n²" and "Lamb shift" key-results entries carry the classical-reproduction / `[PARAMETRIC]` qualifier.
- `CATALOG_PARAMETRIC_INSERTIONS.md` — atomic-chemistry rows added (previously absent).

This document is the canonical reference; if a code comment or README line conflicts with it, this map wins.
