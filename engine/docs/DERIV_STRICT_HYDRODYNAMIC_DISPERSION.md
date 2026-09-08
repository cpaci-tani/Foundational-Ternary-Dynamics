# Exact small-wavevector dispersion of the staged field sector

Date: 2026-09-08. Law: `phi-v2-staged-candidate-1`. Gate: H1 of
[the hydrodynamics design](../../docs/superpowers/specs/2026-09-07-phi-hydrodynamics-design.md).
Status before computation: **[PREREGISTRATION — EXACT LINEAR-RESPONSE CONTRACT]**.
Nothing above the results marker changes after results exist.

## Objects

Per polarity, channel space Q^192. Reference density p (rational). Linearized
collision at layer q: `J_q = I + r K_q`, `r = p (1-p)^189`, `K_q` the integer
correction of `recovery_kinetic_response.collision_operator(q)`. Streaming at
wavevector `k = kappa * n` for an integer direction n: with the formal variable
`eps = -i kappa`, `S(eps) = P_U (I + eps D1 + eps^2 D2)`, `D1 = diag(n . d(c))`,
`D2 = diag((n . d(c))^2 / 2)`. Stage t applies `J_{(l0 - t) mod 3}` then `S`;
the period map over 12 stages (48 physical microticks) is
`P(eps) = P0 + eps A1 + eps^2 A2` with real rational coefficients.

Conserved weights W (7 rows) are the stage-0 locked kernel of gate H0.
Equilibrium modes V (192 x 7) are the right null vectors of `I - P0` with
`W V = I`. The reduced resolvent R satisfies `(I - P0) R = I - V W`, `W R = 0`,
`R V = 0`. Effective operators: `M1 = W A1 V`, `M2 = W (A2 + A1 R A1) V`.
Eigenvalues near 1 read `lambda = 1 - i kappa mu1 - kappa^2 mu2`; per-period
damping is `mu2 - mu1^2/2`.

## Two arithmetic tracks

Exact track: python-flint `fmpq_mat` at the declared proxy coupling
`r0 = 1/695` (relative offset -0.049 % from `r(1/96)`). Certified track:
python-flint `arb_mat` at 256-bit precision at the exact physical `r(p)` for
p in {1/96, 1/192, 1/48}; every clause is reported PROVED, CONSISTENT, or
UNDECIDED from enclosures. The certified track at `r0` must enclose the exact
track (a test). No float enters any clause.

## Directions and normalization

n in {(1,0,0), (1,1,0), (1,1,1)}, unnormalized. First-order quantities are
divided by |n|^2 where they scale as |k|^2 (sound speed squared), second-order
transverse coefficients by |n|^2 and |n|^4, longitudinal damping by |n|^2.

## Verdict rule (fixed)

Let a be the density functional (constant weight in the W basis) and B the
smallest row space containing a and invariant under every M1(n), M2(n).

1. `dim B = 4`. Its 3-dimensional complement within B is "momentum-like"; no
   other definition is admitted.
2. For every n, `charpoly(M1|_B) = x^4 - s x^2` with s > 0, `(M1|_B)^3 = s M1|_B`,
   and `s/|n|^2` equal across the three directions.
3. Transverse second-order polynomial (charpoly of `Pi_T Y Pi_T` divided by
   x^2, `Pi_T = I - X^2/s`), normalized, equal across directions; longitudinal
   damping `tr(Pi_L Y Pi_L)/2 - s/2`, normalized, equal across directions.

All three hold exactly at r0: **NS-class isotropic**. Clause 1 and 2 hold and 3
fails: **anisotropic momentum hydrodynamics**. `M1 = 0` for every n:
**diffusive only**, with the density diffusion polynomial reported. Anything
else: **other**. The full 7x7 operators and their characteristic polynomials
are reported in every case. No fourth label and no reinterpretation.

## Tag

**[DERIVED — linearized Boltzmann (product) closure at the declared reference;
correlation leakage not bounded here].** The wave-2 full-tangent bounds are
loose; this is a prediction for gate H2, not a theorem about Phi.

<!-- RESULTS MARKER: nothing above this line changes after results exist -->
