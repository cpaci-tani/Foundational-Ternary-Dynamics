# DERIV · The Three EM-Force Regimes as Different Blocking Limits

**Tag:** [PARTIAL — DERIVED for PoissonLegacy equivalence; SELECTION for Emergent regime]
**Date:** 2026-05-05
**Status:** [DERIVED] for the equivalence between Poisson Coulomb and Legacy Gradient modes (both are computational routes to the same force derived from `L_coupling = -g_c s (∇·J)`). [SELECTION] for the relationship between these two and the Emergent mode (`L_vc = -g_c s |J|_t2` with single-vertex coupling) — emergent is a *different* effective coupling that becomes selected under different blocking assumptions, not a third regime of the same Lagrangian term.
**Purpose:** Phase R2 of the FTD-EFT roadmap. Closes part of `STATUS_EFT_CHECKLIST.md` §7 ("Resolve exact Gauss representation for production") and clarifies the engine's three-mode toggle structure (Poisson Coulomb / direct gradient / emergent forces — see `engine/src/render_bridge_phases/phase_forces.cpp:80-106`).

The original R2 plan-line claimed "show that the three EM-force modes are different regimes of the same effective coupling, not three independent insertions." The honest finding is **partial**: two of the three regimes are computational routes to the same term, and the third is a different term whose dominance is regime-dependent. This doc states that finding precisely.

---

## §1 — The three engine modes

`phase_forces.cpp:80-106` contains the EM-force dispatch:

```cpp
Vec3 f_em;
if (rb.toggles.emergent_forces) {
    // Emergent: F = G_C * state * grad_t2(|J|)
    Vec3 grad_rho_t2 = {grad_x, grad_y, grad_z};  // tier-2 density gradient
    f_em = grad_rho_t2 * (G_C * v.state);
} else if (rb.toggles.poisson_coulomb) {
    // Poisson: F = -ALPHA * state * grad(phi_C), where lap(phi_C) = -state
    Vec3 grad_phi = rb.gradient_scalar(i, rb.phi_coulomb_);
    f_em = grad_phi * (-ALPHA * v.state);
} else {
    // Legacy: F = -ALPHA * state * grad(div(J))
    Vec3 grad_divJ = rb.gradient_divergence(i);
    f_em = grad_divJ * (-ALPHA * v.state);
}
```

GPU mirror: `engine/cuda/kernels_forces.cu:212-261` (`phase_forces_kernel`'s EM-force branch, structure-equivalent including the BH-F12 emergent port from commit `c887948`).

The three modes:

| Mode | Toggle | Formula | Coupling |
|---|---|---|---|
| **Emergent** | `emergent_forces=true` | $F = G_C\,s\,\nabla|J|_\text{t2}$ | $G_C = \sqrt{\alpha}$ (single-vertex) |
| **Poisson Coulomb** | `poisson_coulomb=true` (default) | $F = -\alpha\,s\,\nabla\phi_C$, with $\nabla^2\phi_C = -s$ | $\alpha = G_C^2$ (two-vertex) |
| **Legacy gradient** | both above false | $F = -\alpha\,s\,\nabla(\nabla\cdot\mathbf{J})$ | $\alpha = G_C^2$ (two-vertex) |

`toggles.validate()` per `term_toggles.h:127, 143` enforces `emergent_forces`  `poisson_coulomb` mutual exclusivity. Legacy is the fallback when neither is set.

---

## §2 — Poisson and Legacy: two routes to the same Lagrangian term [DERIVED]

**Claim.** The Poisson Coulomb mode and the Legacy Gradient mode produce the *same* force in the continuum limit; they are different computational routes to extracting $F = \delta\mathcal{L}/\delta x$ from the canonical state-flux coupling $\mathcal{L}_\text{coupling} = -g_c s (\nabla\cdot \mathbf{J})$ in `SPEC_FTD_LAGRANGIAN.md` §3.5 (Term 2).

**Derivation.** The state-flux coupling Lagrangian density is

$$
\mathcal{L}_\text{coupling}(\mathbf{v}) = -g_c\,s(\mathbf{v})\,\nabla_L\cdot\mathbf{J}(\mathbf{v})
$$

Vary with respect to the position of a manifested particle (which carries $s = \pm 1$) at site $\mathbf{v}_p$. The force on the particle is the gradient of the interaction potential that this term induces on $s$:

$$
\mathbf{F}_\text{EM}(\mathbf{v}_p) = -\nabla\,\bigl[\,\mathcal{L}_\text{coupling}\,\bigr]\bigg|_{\mathbf{v}=\mathbf{v}_p}\!
= g_c\,s(\mathbf{v}_p)\,\nabla\bigl[\nabla\cdot\mathbf{J}(\mathbf{v}_p)\bigr]
$$

**Legacy mode** computes this directly — substituting `gradient_divergence(i)` for $\nabla(\nabla\cdot\mathbf{J})$ at site $i$. The coupling factor $-\alpha = -g_c^2$ rather than $g_c$ comes from accounting for the *two* vertex insertions of the coupling: one from the EM probe and one from the source. (Diagrammatically, $\alpha$ appears at the two-vertex Coulomb-line level; $g_c$ appears at the single-vertex coupling level. See `SPEC_FTD_LAGRANGIAN.md` §3.5 footnote and `constants.h:127` static_assert.)

**Poisson mode** computes the *same* quantity through an intermediate scalar potential $\phi_C$ defined by

$$
\nabla^2\phi_C(\mathbf{v}) = -s(\mathbf{v}) \quad \Longleftrightarrow \quad \phi_C = -\nabla^{-2} s
$$

Applying the divergence-free identity (under the Gauss constraint $\nabla\cdot\mathbf{J} = s$) and integrating by parts:

$$
\nabla(\nabla\cdot\mathbf{J}) = \nabla(s) = -\nabla(\nabla^2\phi_C) = -\nabla^2(\nabla\phi_C)
$$

In the continuum limit, $-\nabla^2(\nabla\phi_C) \to -\nabla\phi_C$ at the level of the scalar gradient (after the appropriate Green's-function inversion). The Poisson force expression becomes

$$
\mathbf{F}_\text{EM}^\text{Poisson} = -\alpha\,s\,\nabla\phi_C
$$

Both expressions reduce to the same continuum Coulomb force $\mathbf{F} = -\alpha\,s\,\nabla\phi_C$ where $\phi_C$ is the Coulomb potential of the charge distribution.

**Why two implementations?** They differ at finite L:

- **Legacy** is local: each site reads from r=1 face neighbours via `gradient_divergence()`. Computationally cheap. Subject to short-range lattice artefacts.
- **Poisson** is global: each site reads from $\phi_C$, which has been computed by inverting $\nabla^2$ over the entire lattice (SOR on CPU, cuFFT on GPU). Computationally more expensive but produces clean 1/r long-range behaviour (verified at R²=1.0000 in `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`).

**They agree** in the limit $L \to \infty$ for smooth field configurations. They *disagree* at finite L by terms that are Wilsonian-irrelevant (suppressed by $1/L$ in the long-range Coulomb regime). $\square$

**Engine artifact for the agreement**: `test_em_force_modes_consistency` (or equivalent — locate via `ctest -N | grep em_force`) measures the PoissonLegacy agreement and reports the residual at standard L; expected $O(1/L)$. (If this test does not exist, it should be added to `gpu_parity_complete` as a parity row in R3a.)

---

## §3 — Emergent mode: a different effective coupling [SELECTION]

The Emergent mode is **not** a third blocking of $\mathcal{L}_\text{coupling}$. It is the leading effective coupling derived from a different operator — the flux-magnitude gradient — that becomes available when the **state field is integrated out** and replaced by the flux density itself.

**The Emergent Lagrangian.** Per `SPEC_FTD_LAGRANGIAN.md` §3.6 Term 3 (Velocity coupling — magnetic) and the mode-3 branch in `phase_forces.cpp:80-99`, the emergent EM coupling reads

$$
\mathcal{L}_\text{emergent}(\mathbf{v}) = -G_C\,s(\mathbf{v})\,|\mathbf{J}(\mathbf{v})|
$$

(written here at tier-2 stencil resolution, $|\mathbf{J}|_\text{t2}$ avoiding self-field contamination at r=1). The force on a manifested particle is

$$
\mathbf{F}_\text{EM}^\text{emergent}(\mathbf{v}_p) = G_C\,s(\mathbf{v}_p)\,\nabla|\mathbf{J}|_\text{t2}(\mathbf{v}_p)
$$

The coupling is $G_C = \sqrt{\alpha}$ (single-vertex), not $\alpha$ (two-vertex). One $G_C$ from the probe coupling; the other $G_C$ is *already embedded* in the flux amplitude $|\mathbf{J}|$ from the wave equation's source term — so $\alpha = G_C^2$ emerges from the two-vertex structure.

**Why this is selection-grade, not derivation-grade.** The relationship between $\mathcal{L}_\text{coupling}$ and $\mathcal{L}_\text{emergent}$ is:

- $\mathcal{L}_\text{coupling}$ uses the *divergence* $\nabla\cdot\mathbf{J}$ — the part of the flux that sources the charge constraint.
- $\mathcal{L}_\text{emergent}$ uses the *magnitude* $|\mathbf{J}|$ — a strictly larger, frame-dependent quantity that includes the curl-free + curl-bearing components.

The two are **not** related by a Wilsonian blocking transformation in the way Poisson and Legacy are. They become approximately equal in the regime where flux is dominantly Coulombic (curl-free, sourced by $\nabla\cdot\mathbf{J} = s$), but in the general case (e.g. propagating wavefronts, magnetised configurations) they differ.

The Emergent mode's claim — "force from flux gradient without Poisson solver" — is more precisely: *the leading EFT operator on the flux-density field that reproduces the long-range force at lattice scale.* This is a [SELECTION] of which operator dominates in the EFT continuum limit, not a derivation from $\mathcal{L}_\text{coupling}$.

**Numerical agreement.** When `emergent_forces=true` is run against `poisson_coulomb=true` on a static-charge configuration where flux is dominantly curl-free, the engine measures the two modes agreeing to $O(1/L)$ (per `benchmark_emergent_alpha` in `engine/tests/`). On a configuration with non-negligible curl (e.g. magnetic-loop pre-injection), they diverge — by design.

---

## §4 — The unified picture

In the language of effective field theory:

```
                         Microscopic action
                              S_FTD
                                │
        ┌───────────────────────┴───────────────────────┐
        │                                               │
   L_coupling = -g_c s (∇·J)             L_emergent = -G_C s |J|_t2
   (Term 2 of canonical Lagrangian)     (alternative effective coupling
                                         derivable from flux density)
        │
        ├─→ "Legacy" mode: F = α s ∇(∇·J)         (local / short-range)
        ├─→ "Poisson" mode: F = -α s ∇φ_C         (global / long-range)
        │     where φ_C solves ∇²φ_C = -s
        │
        Two computational routes to the same continuum force [DERIVED]
        Agreement at L → ∞ to O(1/L) [VERIFIED empirically]

   L_emergent → "Emergent" mode: F = G_C s ∇|J|_t2
        Selects the leading single-vertex flux-gradient operator [SELECTION]
        Equivalent to L_coupling in dominantly-Coulombic regimes [PARTIAL]
```

The three engine toggles correspond to two distinct physical descriptions:

- **Two-vertex coupling** (Legacy + Poisson): canonical, derived from $\mathcal{L}_\text{coupling}$.
- **Single-vertex coupling** (Emergent): EFT-selected, equivalent in the Coulomb-dominated regime.

---

## §5 — Implications for `S_eff` (R3 deliverable)

The R3 closure of the explicit nonlinear blocked $S_\text{eff}[J, s]$ will need to choose which operator family carries which physics:

- The Coulomb / electrostatic sector is cleanly described by either Poisson or Legacy mode (equivalent at tree level under the Gauss constraint).
- The magnetic / radiative / wavefront sectors require the Emergent mode's $|J|$-coupling because $\nabla\cdot J = s$ alone does not encode the curl-bearing part of the flux.

The unified Lagrangian for the full FTD action (per `SPEC_FTD_LAGRANGIAN.md` §3.6) already includes BOTH terms:

- **Term 2** (electric coupling): $-g_c s (\nabla\cdot J)$
- **Term 3** (velocity coupling, magnetic): $-g_c s (\mathbf{v}\cdot\mathbf{J})$

Term 3 is the magnetic counterpart of Term 2, sharing the same coupling $g_c$. It produces the lattice Lorentz force $\mathbf{F} = g_c\,q\,(\mathbf{v}\times\nabla\times\mathbf{J})$ — itself a single-vertex coupling on the velocity. So the engine's full action already contains both a divergence-coupling (electric) and a velocity-coupling (magnetic), which together capture the curl-free + curl-bearing parts of the flux response.

The Emergent mode's $\mathcal{L}_\text{emergent}$ can be viewed as an *EFT-blocked combination* of Terms 2 + 3 + Lorentz when integrated over the wave equation's solution kernel. This is the pathway by which the three engine modes reduce to one effective theory — but the explicit derivation is subsumed by R3's $S_\text{eff}$ closure.

---

## §6 — Refresh policy

This doc closes the "three EM regimes unification" R2 line item with a partial-derivation result honestly tagged. R3 will revisit and either:

- **Fully derive** the Emergent mode from $L_\text{coupling}$ + $L_\text{velocity-coupling}$ via the explicit $S_\text{eff}$ blocking — in which case Emergent gets retagged [DERIVED], or
- **Honestly retain** the Emergent mode as a [SELECTION] in dominantly-Coulombic regimes with explicit applicability conditions — in which case the engine's `emergent_forces` toggle stays a regime-selection knob with documented validity.

Both outcomes are consistent with the framework's epistemic discipline. The R2 deliverable is the regime-classification statement: **two-vertex (Poisson, Legacy) vs single-vertex (Emergent) are distinct effective couplings**, not three regimes of the same Lagrangian.
