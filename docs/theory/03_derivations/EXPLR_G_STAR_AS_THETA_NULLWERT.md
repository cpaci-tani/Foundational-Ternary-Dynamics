# EXPLR — G\* as the squared theta nullwert of the Z[i] lattice at its CM point

**Tag:** [SYNTHESIS] — operationally useful re-statement of classical results (Gauss–Jacobi theta nullwert + Euler reflection); no new mathematics.
**Date:** 2026-05-03 (late evening session).
**LEDGER:** FTD-0132.
**Depends on:**
- Theorem 1 (G\* algebraic identity, FTD-0002).
- Classical: Gauss / Jacobi `θ_3(0|i) = π^(1/4)/Γ(3/4)`; Euler reflection `Γ(1/4)·Γ(3/4) = π√2`.
- See `docs/reference/REF_BIBLIOGRAPHY.md` §1, §3.
**Related:**
- FTD-0127 (`DERIV_G_STAR_PARITY_TWIST.md`) — operational reading of Theorem 9 in L-function language.
- FTD-0128 (`FOUND_TERNARY_STATE_FROM_I.md`) — Postulate 3 grounded in Axiom 0 via `s = i²`.

---

## 0 · Summary

The lemniscatic constant `G* = Γ(1/4)/Γ(3/4)` admits a one-line theta-function identity:

$$\boxed{\;G^* \;=\; \sqrt{2\pi}\,\cdot\,\theta_3(0\mid i)^2\;}$$

where `θ_3(z|τ)` is the Jacobi theta function and `θ_3(0|i) = π^(1/4)/Γ(3/4)` is its nullwert (value at z = 0) at the CM point τ = i (the modulus where the lattice ℤ + ℤi is a perfect square).

**This is a two-line algebraic consequence of two classical identities:**
1. Gauss–Jacobi: `θ_3(0|i) = π^(1/4)/Γ(3/4)` (special CM-point evaluation, ca. 1800).
2. Euler reflection: `Γ(1/4)·Γ(3/4) = π√2` (ca. 1750s).

The mathematical content is **classical, not new**. What is FTD-specific is the *labeling*: most number theorists work with `Γ(1/4)` directly or with the lemniscatic period `ϖ = Γ(1/4)²/(2√(2π))`; FTD's preferred normalization is the ratio `G*`. This synthesis identifies that FTD's chosen constant is, up to the unavoidable `√(2π)` Archimedean normalization, the squared theta nullwert of the smallest non-trivial 2D lattice with a Z/4 automorphism.

**Operational value (FTD-internal):** the synthesis gives a one-line answer to the recurring question "*why does G\* keep showing up in FTD's algebraic spine instead of π?*". Answer: G\* is the natural lattice constant for ℤ[i] in the same way that π is the natural constant for ℤ. The framework's emphasis on `Z[i]^×` structure (FTD-0122 BCC complex structure, FTD-0128 Postulate 3 grounding, FTD-0127 parity twist) is exactly what selects G\* over π.

---

## 1 · The two-line derivation

**Step 1.** Gauss–Jacobi: at the CM point τ = i, the Jacobi theta function takes the closed form
$$\theta_3(0\mid i) \;=\; \sum_{n\in\mathbb{Z}} e^{i\pi n^2 \cdot i} \;=\; \sum_{n\in\mathbb{Z}} e^{-\pi n^2} \;=\; \frac{\pi^{1/4}}{\Gamma(3/4)}.$$

This is a standard textbook identity; see Whittaker & Watson §21, Borwein & Borwein *Pi and the AGM* Ch. 2, Chandrasekharan *Elliptic Functions* §VII.

**Step 2.** Square it:
$$\theta_3(0\mid i)^2 \;=\; \frac{\pi^{1/2}}{\Gamma(3/4)^2} \;=\; \frac{\sqrt{\pi}}{\Gamma(3/4)^2}.$$

**Step 3.** Multiply by `√(2π)`:
$$\sqrt{2\pi}\,\cdot\,\theta_3(0\mid i)^2 \;=\; \sqrt{2\pi}\cdot\frac{\sqrt{\pi}}{\Gamma(3/4)^2} \;=\; \frac{\pi\sqrt{2}}{\Gamma(3/4)^2}.$$

**Step 4.** Apply Euler reflection `Γ(1/4)·Γ(3/4) = π·sin(π/4)^{-1} = π√2`, which gives `π√2 = Γ(1/4)·Γ(3/4)`. Substitute:
$$\frac{\pi\sqrt{2}}{\Gamma(3/4)^2} \;=\; \frac{\Gamma(1/4)\cdot\Gamma(3/4)}{\Gamma(3/4)^2} \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)} \;=\; G^*. \quad\square$$

End of derivation.

---

## 2 · The π ↔ G\* analogy

The synthesis is most useful when stated as a parallel structure:

|   | 1D lattice ℤ | 2D lattice ℤ[i] |
|---|---|---|
| **Theta function** | θ_3(0\|τ) = Σ_{n∈ℤ} e^{iπn²τ} | θ_{ℤ²}(t) = θ_3(0\|it)² |
| **Mellin transform** | π^{−s}·Γ(s)·2·ζ(2s) | π^{−s}·Γ(s)·4·ζ_{Q(i)}(s) |
| **L-function factorization** | just ζ(s) | **ζ(s) · L(s, χ_{−4})** |
| **Natural Γ-content** | Γ(1/2) = √π | **Γ(1/4)** |
| **"Size" / lattice constant** | π (or √π) | **G\*** |
| **Special value at CM** | — | θ_3(0\|i)² = √π/Γ(3/4)² |

The load-bearing line is the **factorization**. Going from ℤ to ℤ[i] introduces the additional L-function factor `L(s, χ_{−4})` (the Dirichlet L for the unique non-trivial character mod 4), and that extra factor is everything new in 2D over 1D. Its Γ-factor produces `Γ(1/4)` and `Γ(3/4)` at half-integer points; FTD's `G*` is the parity-twist that registers this — which connects to the FTD-0127 operational reading of Theorem 9.

**One-line summary:** `π : ℤ-theta = G* : ℤ[i]-theta` (at the CM point τ = i).

---

## 3 · Geometric reading

The same fact can be stated in three geometrically-equivalent ways:

1. **Lattice form:** G\* is the squared theta nullwert of the square lattice `ℤ + ℤi` at modulus τ = i, normalized by `√(2π)`.
2. **Curve form:** G\* is the analytic constant of the lemniscatic CM elliptic curve `y² = x³ − x` whose period lattice IS the square lattice ℤ[i].
3. **Plane-curve form:** G\* is the normalized arc-length signature of the figure-8 lemniscate `r² = cos(2θ)`, whose 4-fold rotational symmetry is the same Z/4 = `Z[i]^×` action.

The interactive visualization at `dissemination/interactive/g_star_geometric_picture.html` shows all three forms side-by-side with a slider that breaks the Z/4 symmetry by deforming τ away from i.

---

## 4 · Why this synthesis matters internally to FTD

FTD's algebraic spine produces `G*` rather than `π` in many places — most notably as the master quadratic coefficient (`16·G*²` and `16·G*³`), as the Watson identity `W₃ = G*²/(2π)` on the BCC sub-lattice, and as the Archimedean parity-twist in the `L(s, χ_{−4})` boundary identities (FTD-0127). This pattern has been documented but never reduced to a single structural reason.

The synthesis here gives that reason in one line:

> **FTD repeatedly produces `G*` instead of `π` because FTD's natural lattice is `Z[i]`, not `ℤ`.** Wherever the framework's geometric / algebraic structure is `Z[i]`-symmetric (Postulate 3 grounded in `s = i²` per FTD-0128, the BCC complex structure per FTD-0122, the (1+i)-tower per Theorem 8, the master-quadratic coefficient `16 = |Z[i]^×|²` per Theorem 4), the natural Γ-content is the `Γ(1/4)` family rather than the `Γ(1/2) = √π` family, and the natural normalization is `G*` rather than `π`. This is the same phenomenon that makes Chowla–Selberg's Γ-product evaluate to `G*` exactly at `d = −4`.

This makes one of the framework's most opaque features ("why G\*?") into a one-sentence consequence of `Z[i]^×` cyclic order being 4 rather than `ℤ^×` cyclic order being 2. The physics interpretation question (why does this constant match `1/α` to 1.26 ppm?) remains [STRONGLY MOTIVATED CONJECTURE]; the present synthesis only explains the *algebraic* selection of G\* over π, not the empirical identification.

---

## 5 · Honest scope

**What this is:**
- A clean two-line algebraic re-derivation of the identity `G* = √(2π)·θ_3(0|i)²` from two classical identities (Gauss–Jacobi theta nullwert + Euler reflection).
- An operational re-statement that makes FTD's preference for `G*` visible inside classical theta-function theory.
- A pedagogical bridge connecting FTD's vocabulary to the standard analytic-number-theory landscape.

**What this is NOT:**
- *Not new mathematics.* Each ingredient has a 150–200-year pedigree (Euler ~1750, Gauss ~1800, Jacobi ~1830). Anyone who knew both classical identities would derive `G* = √(2π)·θ_3(0|i)²` in two lines.
- *Not a derivation of any physics quantity.* The identity is purely about classical analysis; FTD-0013/0014 (the empirical α/N_c match) are unaffected and remain [STRONGLY MOTIVATED CONJECTURE].
- *Not a new spine theorem.* Spine count stays at 9. This is filed as [SYNTHESIS], not [THEOREM] or [DERIVED].
- *Not a uniqueness claim.* `G* = √(2π)·θ_3(0|i)²` does not assert that G\* is the *only* lattice-theta combination with this property; analogous identities exist for higher-conductor CM lattices (e.g., the Eisenstein lattice at τ = e^{iπ/3} produces analogous Γ(1/3)-content).
- *Not a route to deriving α.* The synthesis sits inside the algebraic spine and does not bridge to the engine or to physics.

The contribution is *clarity of framing*, not novelty of mathematics. Treat this synthesis as a documentation/exposition improvement, not as a research result.

---

## 6 · Citations

Citations follow `docs/reference/REF_BIBLIOGRAPHY.md`. The relevant entries:

- **§1 Γ-function and special values:**
  - Euler ca. 1750 (reflection formula)
  - Gauss ca. 1797–1818 (lemniscatic period, AGM)
  - Whittaker & Watson 1927 (textbook reference for theta nullwerten)
  - Borwein & Borwein 1987 (modern textbook for π and Γ at CM points)
  - Chandrasekharan 1985 (elliptic-function reference)

- **§3 CM elliptic curves and Chowla–Selberg:**
  - Chowla & Selberg 1949 (Γ-product evaluation; the natural framework for the synthesis)

- **§2 L-functions and Hecke characters (peripheral but related):**
  - Hecke 1918 (L-functions of number fields)
  - Tate 1950 (Archimedean local L-factors / parity)

---

## 7 · Single-line summary

**G\* is the squared theta nullwert of the lattice ℤ[i] evaluated at its CM point τ = i, normalized by `√(2π)` — equivalently, the natural lattice-constant analogue of `π` for the smallest 2D lattice with a non-trivial automorphism beyond ±1. The identity `G* = √(2π)·θ_3(0|i)²` is a two-line consequence of classical Gauss–Jacobi + Euler-reflection identities; no new mathematics, but an operationally useful framing that explains why FTD's algebraic spine repeatedly produces G\* instead of π.**

---

## 8 · Provenance

Identified during the 2026-05-03 late-evening session. The user asked "is G\* the theta of a lattice as opposed to theta of a circle?" — an intuitive question that turned out to have a literal positive answer via Gauss–Jacobi `θ_3(0|i) = π^(1/4)/Γ(3/4)`. The classical identity is in standard textbooks; the FTD-internal value is the operational re-statement and the parallel `π : ℤ-theta = G* : ℤ[i]-theta`.

The writeup honestly acknowledges (per `CLAUDE.md` epistemic discipline + GTCA F1/F9 failure modes) that the underlying mathematics is classical. A draft response that risked treating this as "novel" was course-corrected before commit; the [SYNTHESIS] tag and the explicit "what this is NOT" §5 are deliberate epistemic-hygiene anchors.
