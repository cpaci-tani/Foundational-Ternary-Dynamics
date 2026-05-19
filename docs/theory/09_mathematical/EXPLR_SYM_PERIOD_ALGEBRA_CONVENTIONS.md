# Conventions: Symmetric Period Algebra of E_lemn

**Status:** [EXPLR] — working conventions for G* opus follow-up Phase 0+ infrastructure.
**Spec:** `docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md`
**Date locked:** 2026-05-19

## Curve

E_lemn: y² = x³ − x over Q. CM by Z[i]; j-invariant 1728; |Aut_geom(E)| = 4; conductor 32 (LMFDB 32.a3 for L-function; 32.a2 for the elliptic curve).

## de Rham basis of H¹_dR(E_lemn)/Q

Two Q-rational classes:
- ω := dx/y (holomorphic)
- η := x·dx/y (algebraic of the second kind; not holomorphic)

These span H¹_dR(E_lemn)/Q as a 2-dimensional Q-vector space.

## Z[i]-action via CM endomorphism

The CM endomorphism [i] : E → E is defined over Q(i) by (x, y) ↦ (−x, iy). Pullback to forms:
- [i]^*(ω) = [i]^*(dx/y) = d(−x)/(iy) = (−dx)/(iy) = i · dx/y = **i · ω**
- [i]^*(η) = [i]^*(x·dx/y) = (−x)·(−dx)/(iy) = x·dx/(iy) = **−i · η**

**Convention C1 (Z[i]-eigenvalues):** ω is in the +i-eigenline of [i]^*; η is in the −i-eigenline.

## Period values at τ = i

The canonical CM point is τ = i. Periods of (ω, η) over the standard real cycle γ_1 (the lemniscate arc segment from x = −1 to x = 0, doubled):
- **ω_period := ∫_{γ_1} ω = Γ(1/4)²/√(2π) = G* · √π** (Paper A Thm 5.3, equation 12–13)
- **η_period := ∫_{γ_1} η = q**, where q is the quasi-period

The quasi-period is fixed by the Legendre relation. With the imaginary cycle γ_2 = i · γ_1:
- ω(γ_2) = i · ω(γ_1) = i · G*·√π
- η(γ_2) = −i · η(γ_1) = −i · q

Legendre relation: det of period matrix = ±2πi:
```
| G*√π    i G*√π |
| q       −i q   |  =  G*√π · (−i q)  −  i G*√π · q  =  −2i · G*√π · q
```

Setting −2i · G*√π · q = 2πi (positive-orientation convention):
**Convention C2 (Legendre normalisation):**
**q = −π / (G*·√π) = −√π / G***.

(The negative sign reflects the orientation; verified numerically to ≥80 digits in Task 4.)

## Specialisation map Φ

Define Φ : Q[ω, η] → R by
- Φ(ω) := G* · √π
- Φ(η) := −√π / G*
- Φ extended multiplicatively

For arbitrary k:
- Φ(ω^k) = (G*√π)^k = G*^k · π^{k/2}
- Φ(η^k) = (−√π/G*)^k = (−1)^k · π^{k/2} / G*^k
- Φ(ω^a η^b) = (−1)^b · G*^{a−b} · π^{(a+b)/2}

## Complex conjugation involution c

The involution c : H¹_dR ⊗ Q[i] → H¹_dR ⊗ Q[i] is the **Q[i]/Q Galois conjugation**: c(α + βi) = α − βi for α, β ∈ Q, and c acts trivially on Q-rational forms.

**Convention C3 (c-action on Sym^k):** Since ω and η are Q-rational, c(ω) = ω and c(η) = η. On Sym^k(H¹) ⊗ Q[i], c acts by conjugating the Q[i]-coefficients only:
- c(α · ω^a η^b) = ᾱ · ω^a η^b for α ∈ Q[i].

**Interaction with Z[i]-action:** The Z[i]-eigenline of ω^a η^b has eigenvalue i^{a−b} ∈ Q[i]. Under c-conjugation of *coefficients*, an element c_1 · ω^a η^b + c_2 · ω^b η^a (with c_1, c_2 ∈ Q[i]) is c-invariant iff its Q[i]-coefficients are c-fixed, i.e. in Q (not properly in Q[i]).

**Convention C4 (c-invariant subspace):** Sym^k(H¹)^c (the c-fixed subspace) consists of elements with Q-rational (not Q[i]-rational) coefficient expansion in the (ω^a η^b) basis. For Sym²(H¹), this gives 3 real degrees of freedom (3 monomials × 1 dimension each); intersected with the constraint that the element commute with the Z[i]-action up to the eigenline-parity rule (Convention C5), the dimension reduces.

**Convention C5 (eigenline-parity rule, the LOAD-BEARING convention).** A Sym^k element x = Σ_{a+b=k} α_{a,b} · ω^a η^b is "eigenline-parity-consistent" if for every pair (a, b), α_{a,b} ∈ Q[i] satisfies α_{b,a} = c̄(α_{a,b}) · σ_{a,b} where σ_{a,b} ∈ Q[i] is an explicit Legendre-derived scaling factor. The explicit form of σ_{a,b} for k ≥ 3 is **deferred to Phase 1 (spec lemma L2)**; for Phase 0 verification we only need the k = 2 case, where σ_{1,1} = 1 and the convention reduces to: α_{2,0} = α_{0,2} (both real) and α_{1,1} ∈ Q, giving a 2-real-dimensional space. Intersection with the Q[i]-coefficient structure on the (a,b) = (1,1) center-line reduces to 1-dimensional over Q[i] — this is H3 from the pre-registration.

## Numerical reference values (80 digits, computed in mpmath)

To be populated in Task 5. Expected values:
- G* ≈ 2.9586751191886388923 ...
- √π ≈ 1.7724538509055160273 ...
- G*·√π (= ω_period) ≈ 5.2441151085...
- −√π/G* (= η_period) ≈ −0.5990701173...

## What this document does NOT lock

- The Phase 1+ extension of Convention C5 to k ≥ 3 (additional Legendre scaling factors). These are explored in spec lemma L2.
- The connection between Convention C5 and the joint-root match (spec lemma L4). That requires Phase 1 development.

## Change log

- 2026-05-19: initial draft, Phase 0.
