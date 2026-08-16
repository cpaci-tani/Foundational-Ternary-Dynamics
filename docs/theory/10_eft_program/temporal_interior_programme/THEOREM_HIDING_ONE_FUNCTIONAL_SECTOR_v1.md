# THEOREM — The hiding order of the one-functional sector v1

**Date:** 2026-08-15
**Status:** `[DERIVED — EXACT SERIES COMPUTATION, VERIFIER 4/4]` +
`[CORRECTION — REFUTES THE PROGRAMME'S DRAFTED (ka)⁴ EXPECTATION]` +
`[OPEN — matter-sector observables; general arm orientations]`
**Programme:** Universality Programme P2
(`SCOPE_UNIVERSALITY_PROGRAMME_v1.md`, DRAFT — whose P2 statement this
computation amends against the programme's interest).
**Verifier:** `scripts/experiments/temporal_interior/derive_hiding_internal_interferometer.py`
(exact Fraction truncated-series arithmetic, total order 8; 4/4).
**Parents:** FTD-0815 (the exact M18 symbol machinery and the (ka)⁴/3240
anisotropy), the Functional Census (`AUDIT_FUNCTIONAL_CENSUS.md` — the
wave island CF1–CF2 is the sector this theorem covers).

## 1. The observable

The two-color Michelson ratio: round-trip times of signals at two comoving
source frequencies along a boost-parallel and a boost-perpendicular
comoving arm (boost u along ⟨100⟩, arms ⟨100⟩ and ⟨010⟩, k₃ = 0),

> O = [t_∥(A)/t_⊥(A)] / [t_∥(B)/t_⊥(B)].

Arm lengths cancel **exactly** in O — no rod model, no contraction
assumption enters. For any covariant dispersion, massless or massive,
O = 1 identically at every boost; every deviation of O from 1 is pure
non-covariance, read by an internal, apparatus-free procedure. Legs are
parametrized by the comoving-frequency condition ω(k) − u·kₓ = w (comoving
sources and mirrors preserve the lab frequency), group-velocity
kinematics, exact Newton-series solves.

## 2. The computed result

With b = u/C and s = ka (the comoving frequency scale), the single-color
ratio is, exactly through total order 8:

> **R(b, s) = γ(b) · [ 1 + (3/4) b²s² + (7/18) b²s⁴ + (11/4) b⁴s² + … ]**

**T-A (the positive half — kinematic hiding is exact).** The s-independent
tower of R is the γ-series *term by term*: ½b², ⅜b⁴, 5/16 b⁶ (verifier
V1). Dispersion-free internal kinematics in the one-functional sector is
exactly special-relativistic: at zeroth order in (ka) the substrate frame
is perfectly hidden, to all computed orders in the boost.

**T-B (the leak — the hiding order).** The leading internal
frame-detecting term is

> **O − 1 ⊃ (3/4) · (u/C)² · (ka)²  ·  [(k_A a)² − (k_B a)² structure]**,

and its cause is the **isotropic k⁴ dispersion curvature**, not the
anisotropy: removing the k⁴ term of the symbol surgically kills the (2,2)
coefficient (verifier V4), and the geometry uses symmetry-identical
⟨100⟩-class arms, so no anisotropy enters at this order at all. The
celebrated (ka)⁴/3240 anisotropy (FTD-0815) is a *subleading* frame leak.

**Corollary (the correction of record).** The hiding order of the M18
one-functional sector for propagation observables is **(ka)² relative,
not (ka)⁴**. The charter's drafted P2 statement ("invariant through
O((ka)²), breaking at O((ka)⁴)") is refuted by the programme's own first
computation and must be amended at ratification. A dispersive medium
betrays its rest frame through curvature two orders before it does through
anisotropy — Fizeau before Michelson, in classical language.

## 3. Magnitude (context, not a prediction)

At a = ℓ_P the leak is (u/C)²·(E/E_P)²-suppressed: for optical photons
(E/E_P ~ 10⁻²⁸) the two-color signal is ~10⁻⁵⁶·(u/C)² — as unobservable
as every recorded free-sector figure, but the *order* is the honest
statement, and it is weaker than the programme hoped.

## 4. Scope

Exact within: the M18 wave island (census CF1–CF2), k₃ = 0 plane,
⟨100⟩-class arms, group-velocity signal kinematics, comoving-frequency
parametrization, series through total order 8 (order-8 edge coefficients
truncation-contaminated; the quoted terms sit well inside the reliable
region). Not covered: matter/soliton observables (rod-based experiments),
general arm orientations (which add anisotropy terms at higher relative
order), interacting sectors, and everything the Functional Census marks
second-category — each SC term is expected to leak at *lower* order than
this sector's (ka)², which is now the benchmark all of them must beat.
