# AUDIT — κ_ψ = 4π: convention, conditional theorem, or physical law?

**Tag:** [AUDIT]. Findings: the FQCR source-law coupling `κ_ψ` is a **[DEFINITION]** (a normalization convention); it takes the value `4π` only inside one specific convention triple, and is at most a **[CONDITIONAL THEOREM]** given that triple together with `D = 3`. The proposition *"the finite / lattice closure law forces `κ_ψ = 4π`"* is **[CLOSED NEGATIVE]**. `κ_ψ = 4π` is **not** theorem-forceable as a physical law. No FTD LEDGER claim is promoted or demoted by this audit — it confirms the v1.4-taxonomy Layer-E label and supplies the structural reason that label can never be upgraded.
**Date:** 2026-05-21
**LEDGER:** FTD-0188
**Audit context:** FTD/FQCR Cleanup & Taxonomy v1.4 — Path Forward §8 ("first task in the new thread"); Frontier 3.
**Depends on:**
- FTD-0004 (Phase G discrete Poisson Green's function, `G_+(r) → 1/(4πr)`) — [THEOREM]
- FTD-0059 (No-Go: no SI length is derivable from Axiom Zero) — [THEOREM]
- FTD-0041 (`a_phys ≡ ℓ_P` calibration declaration)
- FTD-0131 (Newton from substrate; `G_N = −K_B^grav/(4π m_e)`) — [DERIVED given 2 postulates]
- FTD-0137 (lattice spacing is a gauge degree of freedom)
- FTD-0184 (FQCR gravity red-team; `ℓ_F` defined by `G_N = c³ℓ_F²/ℏ` is the substitution identity `ℓ_F ≡ ℓ_P`)

**Closes:** Frontier 3 of the v1.4 taxonomy ("can finite source flux / Gauss closure theorem-force `κ_ψ = 4π`?") — **as posed**. Recommends re-pointing the freed effort at Frontier 4 (massless spin-2 substrate mode).

---

## 0 · Verdict

The audited proposition (Path Forward §8):

> *"For a finite spherical / closed flux readout, does the unit source require `κ_ψ = 4π`? Or is `4π` only a continuum Gauss normalization imported from spherical flux?"*

Answer, in two parts:

- **A finite / closed lattice flux readout does NOT require `κ_ψ = 4π`.** The discrete divergence theorem — the lattice's own intrinsic closure law — has coefficient **exactly 1** and contains no `4π`. [THEOREM]
- **The `4π` IS the continuum Gauss normalization.** It re-enters FTD only through the asymptotic `1/(4πr)` tail of the 3-D lattice Green's function — and it enters **by theorem** (Green's-function universality, FTD-0004), not by hand. [CONDITIONAL THEOREM]

$$\boxed{\;\kappa_\psi = 4\pi \;\text{ is a }\textbf{[DEFINITION]}\text{ — a normalization convention — at most a }\textbf{[CONDITIONAL THEOREM]}\;}$$

promotable to "`κ_ψ = 4π` **given** `D = 3` and the unrationalized convention triple," and **never** to an unconditional [THEOREM] or a physical prediction.

The v1.4 taxonomy already records this correctly (Layer E: "STRONG SELECTION / not theorem-forced"; §5: "DEMOTED to strong selection"). **This audit confirms that label and closes the Path-Forward §2 / Frontier-3 attempt to re-promote it.** Where the taxonomy and the Path Forward disagree, the taxonomy is right.

---

## 1 · The source law and the question

FQCR taxonomy v1.4, Layer E, states the source law

$$D_\sigma^\dagger\,W\,\sinh(D_\sigma U) \;=\; \kappa_\psi\,J_\chi \qquad\text{(strong field)}$$
$$\Delta_\sigma U \;=\; \kappa_\psi\,J_\chi \qquad\text{(weak field)}$$

with clock readout `dτ = e^{−U} dt`, `a = −c²∇_Γ U`, and the gravity bridge `G_N = (κ_ψ/4π)·c³ℓ_F²/ℏ`. The Frontier-3 question is whether `κ_ψ = 4π` is *theorem-forced* or merely an imported continuum normalization.

Committed FTD already contains the equivalent structure under different notation. [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) (FTD-0131) solves the discrete Poisson equation for a point source and obtains `G_N = −K_B^grav/(4π m_e)` — the `4π` sitting in the **denominator**, taken directly from the Green's-function tail of §1.1. The FQCR `κ_ψ` and the committed `K_B^grav` are the same bookkeeping slot. The audit below is therefore not about novel FQCR machinery; it is about a coefficient FTD has already placed correctly once.

---

## 2 · The decisive distinction — two "Gauss" structures

The question conflates two objects that must be kept separate.

**(a) The lattice / finite closure law — the discrete divergence theorem.** For a field `U` on lattice sites, with discrete gradient `D_σ U` on edges and `Δ_σ = D_σ†D_σ`, and any finite site-set `V` with edge-boundary `∂V`:

$$\sum_{x\in V}\bigl(D_\sigma^\dagger F\bigr)(x) \;=\; \sum_{e\in\partial V} F(e)$$

This is exact telescoping — a combinatorial identity with coefficient **exactly 1**. Integrating the weak-field law over `V`:

$$\sum_{\partial V}\bigl(D_\sigma U\bigr) \;=\; \kappa_\psi\sum_{V} J_\chi \;=\; \kappa_\psi\,Q_{\rm enc}$$

The coefficient on the enclosed source is `κ_ψ`. **No `4π` appears anywhere in the finite closure law.** [THEOREM] The nonlinear law changes nothing: `Σ_∂V W·sinh(D_σ U) = κ_ψ Q_enc`, still coefficient `κ_ψ`, still no `4π`.

**(b) The continuum radial readout.** The `4π` is *not* in the divergence theorem either — Gauss's theorem is coefficient-1 in the continuum too. The `4π` enters at exactly one step: imposing spherical symmetry and integrating over the *actual 2-sphere*, whose area is `4πr²`:

$$\oint_{S^2_r}\!\nabla U\cdot d\mathbf A \;=\; \frac{dU}{dr}\,(4\pi r^2) \;=\; \kappa_\psi\,Q \;\Longrightarrow\; U(r) = -\frac{\kappa_\psi Q}{4\pi r}$$

So the `4π` is **the surface area of `S²`**, nothing else: `4π = |S²| = ∫_{S²} dΩ`.

**The category slip.** The phrase "finite spherical … flux readout" cannot be satisfied on a lattice. A finite closed lattice surface enclosing a source is **cubical / staircase**, not spherical; its face-count scales as `r²` with a cube coefficient (≈ 24 for a cube of half-width `r`), never `4π`. *Finite* and *spherical* are mutually exclusive at finite lattice resolution. The false 4π-hope lives precisely in that slip.

---

## 3 · Where the 4π actually lives — the Green's-function tail

The `4π` re-enters FTD at one place only: the **asymptotic tail of the 3-D lattice Green's function**, which FTD already owns as a theorem.

[`DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) §1.1 (FTD-0004, **[THEOREM]**, `SPEC_ALGEBRAIC_SPINE.md §6`, classical Glasser–Zucker, verified to 0.07 % at L = 384):

$$G_+(r) \;\to\; \frac{1}{4\pi r}\qquad\text{at large }r$$

"A classical lattice-Green's-function fact, independent of any physics interpretation."

The point that decides the audit: **this `4π` is the continuum `D = 3` value, recovered by universality — not a new lattice `4π`.** For *any* consistent local Laplacian on a 3-D lattice the Green's function flows to `1/(4πr)`, because

$$4\pi \;=\; |S^2| \;=\; \frac{2\pi^{3/2}}{\Gamma(3/2)} \;=\; \frac{2\pi^{3/2}}{\tfrac12\sqrt\pi}$$

is fixed by `D = 3` alone. The lattice has no sphere; it *inherits* the continuum solid angle in the `r → ∞` limit, with anisotropic cube corrections suppressed by extra powers of `1/r` and the leading isotropic coefficient exactly `1/(4π)`. [THEOREM]

Concretely: the number of `ℤ³` sites in a thin shell at radius `r` is `≈ 4πr²·dr` (3-D Gauss-circle problem). The `4π` *is* the asymptotic lattice-shell density — `4π = lim_{r→∞}(\text{sites at radius }r)/r²`. It is the continuum limit of a discrete count, recovered, not imposed.

This `4π` belongs to **product-channel / circular closure** (taxonomy §8: "`4π` belongs to closed phase-loop normalization, NOT `G*`-arithmetic"). It is a `Γ(3/2)` half-integer object, not a `Γ(1/4)` quarter-integer object — disjoint from Layer B. The taxonomy's layer separation is correct, and this audit sharpens it: `4π` is a Layer-E continuum normalization that the lattice *recovers*.

---

## 4 · κ_ψ is a three-slot bookkeeping coefficient

`κ_ψ` carries no independent physical content. Its numerical value depends on **three** conventions, not one:

| Slot | Convention | Effect on κ_ψ |
|---|---|---|
| 1 | Rationalized vs unrationalized field normalization (`U = −Gm/r` vs `U = −Gm/(4πr)`) | moves `4π` between `κ_ψ` and the propagator |
| 2 | Normalization of the source current `J_χ` (unit-Dirac vs carrying its own factor — `J_χ = ΔS/ℏ`, `J_χ^{(γ)} = E_γℓ_F/(ℏc)` are already not canonically unit-normalized) | moves `4π` between `κ_ψ` and `J_χ` |
| 3 | Sign / scale convention of `Δ_σ` | trivial rescaling of `κ_ψ` |

`κ_ψ = 4π` is the value the coupling takes for **one** triple: unrationalized field + unit-Dirac `J_χ` + standard `Δ_σ`. Inside that triple it is forced (a [CONDITIONAL THEOREM] via the `D = 3` Green's-function tail of §3); outside it, it is not. The conditions are conventions, so the conditional theorem is not a physical prediction.

**Steelman, and why it fails.** *"`U = −G_N m/r` is physical — it is a real clock's redshift `dτ = e^{−U}dt` near a real mass — so the unrationalized form is privileged, hence `κ_ψ = 4π` is forced."*

What is physical is the **observable**: `dτ/dt(r) ≈ 1 − G_N m/(rc²)`. That is convention-independent. What is *not* physical is the *split* of its coefficient across the three slots above. The identical measured redshift is reproduced by (1) `κ_ψ = 4π` with unit-Dirac `J_χ`; (2) `κ_ψ = 1` with `4π` absorbed into `J_χ`; (3) `κ_ψ = 1` with `4π` in the readout map `Φ_phys = 4π·U`. All three give the same physical redshift. The steelman smuggles in "`J_χ` is unit-Dirac normalized" (slot 2) as if it were forced. The observable alone does not determine `κ_ψ`; only the observable **plus a full convention triple** does. [CLOSED NEGATIVE for the steelman]

The nonlinearity does not rescue it. `D_σ†W·sinh(D_σ U)` fixes the *structure of the strong-field completion*, not the coupling normalization — exactly as in GR, where the nonlinear bootstrap determines the completion but the coupling `8πG/c⁴` is set by the Newtonian-limit correspondence (§6).

---

## 5 · The downstream chain `κ_ψ = 4π ⟹ ℓ_F = ℓ_P` is independently blocked

Path Forward §2 hopes: `κ_ψ = 4π ⟹ G_N = c³ℓ_F²/ℏ ⟹ ℓ_F = √(ℏG_N/c³) = ℓ_P`. This fails for a **second, independent** reason, even granting the convention.

[`THEOREM_A_PHYS_NO_GO.md`](../10_eft_program/THEOREM_A_PHYS_NO_GO.md) (FTD-0059, **[THEOREM]**): *no quantity with SI dimension of length is derivable from Axiom Zero* — extended (Corollary 3.1/4.1) to mass, time, energy. `ℓ_F` is a length; its value is therefore **100 % calibration**.

The bridge `G_N = (κ_ψ/4π)·c³ℓ_F²/ℏ` is a single equation. The prefactor `(κ_ψ/4π)` is a pure convention ratio (§4); given empirical `{G_N, c, ℏ}` and a *chosen* `κ_ψ`, the bridge simply **defines** `ℓ_F`. Choosing `κ_ψ = 4π` is choosing the convention in which that definition reads `ℓ_F = ℓ_P` exactly; any other choice gives `ℓ_F = ℓ_P·(4π/κ_ψ)^{1/2}`.

$$\boxed{\;\kappa_\psi = 4\pi \;\text{ and }\; \ell_F = \ell_P \;\text{ are one calibration declaration written twice, not two results.}\;}$$

That declaration already exists: `a_phys ≡ ℓ_P` (FTD-0041). FTD-0184 reached the same conclusion by an independent route — "`ℓ_F`, defined by `G_N = c³ℓ_F²/ℏ`, is identically the Planck length … the substitution identity (reduced Compton)·(half-Schwarzschild) = `ℏG/c³` = `ℓ_P²` … not tagged THEOREM (would violate the anti-substitution-identity discipline)." And FTD-0059 Corollary 4.4 catalogs the *identical* error pattern — row "γ-SUCCESS (retracted)": "`ℏ_lat = 1` silently introduced … calibration shuffle." **Path Forward §2 is poised to re-run a closed-negative.** [CLOSED NEGATIVE — re-derivation of FTD-0059 / FTD-0184 closed work]

---

## 6 · Cross-domain confirmation

A coefficient equal to the measure of a sphere is geometry, never dynamics.

**Complex analysis.** Nobody asks whether the `2πi` of the residue theorem `∮ f dz = 2πi·ΣRes` is "derivable" — `2πi` *is* the measure of `S¹` with orientation; demanding a deeper derivation is a category error. The hierarchy `|S⁰| = 2`, `|S¹| = 2π`, `|S²| = 4π` is forced by dimension and forced to be nothing else, but it is never a dynamical quantity. `κ_ψ = 4π` is the `|S²|` of `D = 3`; treating it as a *physics frontier* is the residue-theorem category error.

**General relativity.** `G_{μν} = (8πG/c⁴)T_{μν}` — the `8π` is universally understood as `4π` (Poisson/Gauss) `× 2` (the linearized trace-reversal factor — the *same* "2" as FTD's Postulate 2 `tick_rate = 1 + 2φ/c²` in `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4). It is fixed by the **Newtonian-limit correspondence**, not derived from nonlinear consistency. GR never claimed to derive `8πG`. FTD's `κ_ψ` sits in exactly that seat — and that is the standard, honest status, not a weakness.

**Electromagnetism.** The `4π` placement in Maxwell's equations is the textbook example of pure convention: Gaussian units `∇·E = 4πρ` (unrationalized), Heaviside–Lorentz `∇·E = ρ` (rationalized), SI `∇·E = ρ/ε₀`. A century of physics established that the `4π` is bookkeeping (Jackson, appendix on units). The `κ_ψ` question is the same question, already settled in another sector.

---

## 7 · What the κ_ψ investigation genuinely established

The investigation does yield a real, non-trivial result — it is just a *recovery* theorem, not a derivation of `4π`:

$$\boxed{\;\text{FTD's finite, sphere-free substrate recovers the continuum inverse-square law}\;}$$
$$\text{with the correct }4\pi = |S^2|\text{ solid-angle normalization, via the universal }1/(4\pi r)$$
$$\text{tail of the 3-D lattice Green's function.}\quad\textbf{[THEOREM]}\ \text{(FTD-0004)}$$

This is worth banking as *what the κ_ψ work actually showed*. The substrate contains no sphere, yet `D = 3` discreteness + local causality is enough to reproduce both the `1/r` law and the `4π` of continuum space. The `4π` is not "imported by hand" — it is imported *by theorem*. But it is the continuum `4π` recovered; the lattice does not generate an independent one, and `κ_ψ` is not where new physics lives.

---

## 8 · Recommendations

| # | Action | Rationale |
|---|---|---|
| 1 | Relabel `κ_ψ = 4π` in the v1.4 taxonomy (Layer E status table, constant table, §5) from "strong selection" to **[DEFINITION]** (normalization convention). | "Selection" implies a choice among physically-distinct options; the options here are physically identical (§4). The precise label is DEFINITION. Keep it, declare it, never promote it. |
| 2 | **Close Frontier 3 as posed.** | "Can finite source flux / Gauss closure theorem-force `κ_ψ = 4π`?" asks to derive a convention. The finite closure law is provably `4π`-free (§2); the `4π` is only the continuum limit (§3). [CLOSED NEGATIVE] |
| 2′ | *Optional re-pose:* "Is the `J_χ` normalization (slot 2) fixed by an independent FTD principle — e.g. an action principle for `J_χ = ΔS/ℏ`?" | A real but narrower question. If `J_χ` is canonically pinned, slots 1 + 3 remain free, so `κ_ψ` is still convention — but the *family* of conventions shrinks. Low priority. |
| 3 | Mark Path Forward §2's "`ℓ_F = ℓ_P` from `κ_ψ = 4π`" as **[CLOSED NEGATIVE]**; `ℓ_F = ℓ_P` stays **[CALIBRATION]** (= `a_phys ≡ ℓ_P`, FTD-0041). | Blocked by FTD-0059 (no length from Axiom Zero) and already closed by FTD-0184. Re-running it would repeat the retracted γ-SUCCESS error (§5). |
| 4 | Re-point the freed effort at **Frontier 4** (massless spin-2 substrate mode). | Convention-free, FTD-native, and the genuine open gravity deliverable. Unlike Frontier 3 it is not a category error. |

No FTD LEDGER tag is promoted or demoted. The FQCR source law's *structure* (`U`, `dτ = e^{−U}dt`, the `sinh` strong-field law) is untouched by this audit; only the false hope of theorem-forcing the normalization coefficient is closed.

---

## 9 · Single-line summary

**`κ_ψ = 4π` is a normalization [DEFINITION], at most a [CONDITIONAL THEOREM] given `D = 3` plus a convention triple — never a theorem-forced physical law: the finite/lattice closure law is provably `4π`-free (coefficient 1, [THEOREM]), the `4π` is the continuum `|S²|` recovered only in the `1/(4πr)` Green's-function tail (FTD-0004 [THEOREM]), and the downstream `ℓ_F = ℓ_P` chain is independently blocked by the FTD-0059 no-go and already closed by FTD-0184; the v1.4 taxonomy's "strong selection, not theorem-forced" label is confirmed, Frontier 3 is closed as posed, and the genuine result to bank is that FTD's sphere-free substrate recovers the continuum inverse-square law with the correct `4π` solid angle.**

---

## 10 · Provenance

Audit performed 2026-05-21 as the first task of the FTD/FQCR Cleanup & Taxonomy v1.4 "new thread" (Path Forward §8). Method: first-principles derivation of the lattice vs continuum closure structure, cross-checked against committed FTD theorems — FTD-0004 (Green's-function tail, [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md) §1.1), FTD-0059 ([`THEOREM_A_PHYS_NO_GO.md`](../10_eft_program/THEOREM_A_PHYS_NO_GO.md)), FTD-0131, FTD-0137, FTD-0184. No numerical search was run; the only constant involved (`4π = 2π^{3/2}/Γ(3/2)`) is elementary and exact. LEDGER row: FTD-0188.
