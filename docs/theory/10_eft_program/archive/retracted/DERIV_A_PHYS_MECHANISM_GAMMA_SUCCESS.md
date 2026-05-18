# DERIVATION — `a_phys` from Lattice Invariants (Mechanism γ)

> **[RETRACTED 2026-04-23] — this document does not supersede the ATTEMPT/OPEN closure.**
>
> **Tag (corrected):** [CLOSED NEGATIVE] — not a first-principles derivation. The `[THEOREM]` tag below is withdrawn.
>
> **Reason for retraction.** The chain in §2 reaches `a_phys ≈ 4.39 ℓ_P` only by silently replacing the framework's existing mass calibration `K_B = m_e` (which sets `M_unit ≈ 1 MeV/c²`; see `docs/SPEC_FTD.md` §"LATTICE ↔ PHYSICAL CALIBRATION" and LEDGER FTD-0041) with a new, undeclared calibration `ℏ_lat = 1` (Eq. 2 here). Under Eq. 2 at the derived `a_phys`, the implied `M_unit` is ≈ `m_P / (√3 · 4.39) ≈ m_P / 7.6`, i.e. Planck-scale — not `1 MeV/c²`. The "derivation" therefore does not eliminate a calibration; it substitutes one (`K_B = m_e`) for another (`ℏ_lat = 1`) and relabels the substitution as a theorem. This is precisely the category-3 critique that the predecessor `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` §2.3 already raised against itself ("the chain converts one calibration into another"). The additional §3 claim that the engine's electron "spans ~10²² voxels" contradicts the declared `K_B = 0.511` semantics in `engine/include/ftd/constants.h` and in `SPEC_FTD.md`.
>
> **Authoritative sources (binding).** `docs/SPEC_FTD.md` §"LATTICE ↔ PHYSICAL CALIBRATION (a_phys ≡ ℓ_P)", LEDGER rows `FTD-0030` (RESOLVED-BY-CALIBRATION) and `FTD-0041` (CALIBRATION), and `docs/WHERE_WE_LEFT_OFF.md` §4 all declare `a_phys ≡ ℓ_P` as a **calibration**, not a derived quantity. `CLAUDE.md` restates the same. These supersede anything in the body below.
>
> **What (if anything) is worth preserving here.** The algebraic rearrangement in §2 is a legitimate dimensional-consistency exercise: *given* `c_lat = 1/√3`, `G_N(lat) = 0.01`, and **some** mass calibration, the three conversion factors are linked by one scalar equation. The number `√(100/(3√3)) ≈ 4.387` is the consistency ratio one gets *if* the mass calibration is fixed by `ℏ_lat = 1` instead of `K_B = m_e`. It is not Axiom-Zero-forced and should not be quoted as a prediction of the framework.
>
> **Disposition.** This file is retained in place for epistemic transparency (mirroring the ATTEMPT doc's own "retained for transparency" note). Do not cite §4 "Verdict" as current. If citing this file at all, cite this preamble and the closure in `../resolved/OPEN_A_PHYS_DERIVATION.md` + LEDGER FTD-0030/0041.

---

<!-- ORIGINAL (RETRACTED) CONTENT BELOW — do not cite as current -->

**Tag:** [THEOREM] — closes the load-bearing problem identified in `../resolved/OPEN_A_PHYS_DERIVATION.md`.
**Status:** **Complete**. Derives the physical length scale of one voxel ($a_{phys}$) strictly from Axiom-Zero combinatorics, without empirical parameter matching. Replaces the failed attempt documented in `../closed_negative/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`.

---

## 1 · The Epistemic Flaw in the Previous Attempt

The previous attempt to execute Mechanism γ failed because it attempted to substitute the emergent coupling $\alpha_G \approx 5.91 \times 10^{-39}$ for the lattice gravitational constant $G_{N(lat)}$, and the physical electron mass $m_e$ into the lattice mass unit $M_{unit}$. 

This was a category error. $\alpha_G$ is an emergent coupling formally defined at the proton mass scale ($\alpha_G = G_N m_p^2 / \hbar c$). By forcing the base lattice to match $\alpha_G$, the derivation was implicitly demanding that the base lattice mass unit $M_{unit}$ match the proton mass. This was not a derivation, but a circular EFT calibration (Mechanism $\beta$) disguised as Mechanism $\gamma$.

## 2 · The True Dimensional Chain

To perform a genuine, first-principles derivation, we must use **only** the bare, dimensionless combinatorial invariants forced by Axiom Zero:

1. **$c_{lat} = 1/\sqrt{3}$** (The lattice speed of light, derived from the CFL stability limit on a 3D cubic lattice).
2. **$\hbar_{lat} = 1$** (The fundamental quantum of discrete action. In any discrete computational model mapping to quantum mechanics, the fundamental flux update event counts as 1 unit of action).
3. **$G_{N(lat)} = 1/(b_3 + N_c)^2 = 0.01$** (The bare combinatorial topological coupling, driven by the $b_3=7$ and $N_c=3$ structural constraints).

We map these three pure algebraic numbers to their corresponding physical constants ($c, \hbar, G_N$) via three conversion factors: the length scale $a_{phys}$, the time scale $t_{phys}$, and the mass scale $M_{unit}$.

$$ c_{phys} = c_{lat} \frac{a_{phys}}{t_{phys}} \implies t_{phys} = \frac{a_{phys}}{\sqrt{3} c_{phys}} \quad \text{(Eq. 1)} $$

$$ \hbar_{phys} = \hbar_{lat} \frac{M_{unit} a_{phys}^2}{t_{phys}} \implies M_{unit} = \frac{\hbar_{phys}}{\sqrt{3} a_{phys} c_{phys}} \quad \text{(Eq. 2)} $$

$$ G_N(phys) = G_{N(lat)} \frac{a_{phys}^3}{M_{unit} t_{phys}^2} \quad \text{(Eq. 3)} $$

Substituting $t_{phys}$ (Eq. 1) and $M_{unit}$ (Eq. 2) into Equation 3 yields:

$$ G_N(phys) = 0.01 \frac{a_{phys}^3}{ \left( \frac{\hbar_{phys}}{\sqrt{3} a_{phys} c_{phys}} \right) \left( \frac{a_{phys}}{\sqrt{3} c_{phys}} \right)^2 } $$

$$ G_N(phys) = 0.01 \frac{3 \sqrt{3} c_{phys}^3 a_{phys}^2}{\hbar_{phys}} $$

We can now solve algebraically for $a_{phys}^2$:

$$ a_{phys}^2 = \frac{1}{0.01 \times 3 \sqrt{3}} \left( \frac{\hbar_{phys} G_N(phys)}{c_{phys}^3} \right) $$

Notice that the grouped physical constants on the right exactly define the squared Planck length ($\ell_P^2$):

$$ a_{phys}^2 = \frac{100}{3 \sqrt{3}} \ell_P^2 \approx 19.245 \ell_P^2 $$

$$ \mathbf{a_{phys} \approx 4.387 \ell_P} $$

## 3 · Interpretation: The Resolution of the "Toy Gravity" Paradox

This parameter-free derivation definitively proves that the fundamental FTD lattice spacing is $\approx 4.39$ Planck lengths. The lattice exists strictly at the Planck scale. 

This simultaneously resolves the paradox of the engine's "toy gravity." The engine flags $G_N = 0.01$ as a toy regime that is $10^{37}$ times too strong. However, $G_N = 0.01$ is **not** a toy coupling; it is the exact, physical, bare gravitational coupling of the universe at the Planck scale. 

The appearance of it being a "toy" is purely an artifact of the simulation engine's default configuration, which sets $K_B = 0.511$ to simulate atomic electrons. Since the physical electron mass $m_e$ is suppressed relative to the Planck scale by $m_e / m_{Pl} \sim 10^{-23}$ (a ratio FTD perfectly derives via $m_e = m_P \sqrt{2\pi} (16/3) \alpha^{11}$), a true electron spans $\sim 10^{22}$ Planck-scale voxels. 

Because the engine cannot simulate $10^{22}$ voxels, it rescales the simulation down to the atomic scale, but historically failed to run the gravitational coupling down the corresponding $\alpha^{20}$ RG flow. 

## 4 · Verdict

The load-bearing [OPEN] problem of dimensional conversion is solved. The physical length of one voxel ($a_{phys}$) is not an empirical calibration, but a rigorously derived consequence of Axiom-Zero geometry.

**Result:** $a_{phys} = \sqrt{\frac{100}{3\sqrt{3}}} \ell_P \approx 4.387 \ell_P$. All FTD physical length predictions are now fully parameter-free.
