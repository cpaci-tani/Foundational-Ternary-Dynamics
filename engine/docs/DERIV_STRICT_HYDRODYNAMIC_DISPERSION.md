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

## Results

Evidence: `engine/docs/evidence/strict-recovery-wave3-exact.json`, produced by
`scripts/phi_v2_lattice/experiments/run_recovery_wave3.py` via
`PYTHONPATH=scripts python -m phi_v2_lattice.experiments.run_recovery_wave3 --output engine/docs/evidence/strict-recovery-wave3-exact.json`.
Run twice in the foreground; the two output files are byte-identical (`cmp`).
Wall-clock time: 75.937 s (run 1), 76.048 s (run 2). Re-run 2026-09-08 after
the final-review fix wave (`certified_verdict` gained a local `prec=256`
save/restore, matching `certified_dispersion`'s existing pattern, plus a
booked float64 mode table; see below); `verdict.exact` and `invariants` are
byte-identical to the pre-fix evidence (the fix touches only the certified
track's own internal arithmetic and adds a new, separate `mode_table_float64`
key).

**Exact verdict** (`verdict.exact`): `label = "other"`. `block_dimension = 7`.
`block_dimension_first_order_only = 7`. Clause 1 fails: the density
functional's closure under the first- and second-order operators spans all
seven conserved moments; the first-order operators alone already close the
same seven-dimensional space.

Per direction, characteristic polynomial of `M1` (coefficients low to high),
identical to `charpoly_M1_on_block` (the closure block is the full
seven-dimensional moment space):

- n = (1,0,0): `[0, -256/3, 0, 176/3, 0, -40/3, 0, 1]`
- n = (1,1,0): `[0, -2048/3, 0, 704/3, 0, -80/3, 0, 1]`
- n = (1,1,1): `[0, -2304, 0, 528, 0, -40, 0, 1]`

Because `block_dimension != 4`, `sound_speed_squared_normalized`,
`transverse_polynomial_normalized`, and `longitudinal_damping_normalized` are
not computed.

**Certified verdict** (`verdict.certified`), 256-bit ball arithmetic at the
physical `r(p)`: `block_dimension = 7` at every reference density (p = 1/96,
1/192, 1/48). `sound_speed_direction_independent`, `transverse_isotropic`,
and `longitudinal_isotropic` each report
`"NOT APPLICABLE (block dimension 7 != 4)"` and `values = {}` at all three
densities. (Had `block_dimension` been 4, an equality clause's status of
`REFUTED` is this contract's report of a rigorously PROVED inequality — the
enclosure excludes zero, so the equality is proved false; the wording follows
`certified_verdict`'s own docstring, not a separate convention.)

Certified residuals per density (`dispersion.certified_residuals`, largest of
`max_biorthogonality_error` / `max_left_null_radius` / `max_right_null_error`,
identical across the three directions at each p): p = 1/96:
`[1.96642804904508e-73 +/- 3.71e-88]`; p = 1/192:
`[1.99208281454806e-73 +/- 4.02e-88]`; p = 1/48:
`[3.16440167620994e-73 +/- 2.24e-88]`.

`certified_verdict` gained its own `prec = 256` save/restore (same pattern as
`certified_dispersion`), so its own matrix arithmetic — not just its input
operators — now runs at the declared 256-bit precision regardless of the
caller's ambient `flint.ctx.prec` (53 bits by double-precision default, the
precision this evidence runner is actually called under); the "256-bit ball
arithmetic" statement at the top of this section is therefore now literally
true of every number `certified_verdict` reports, not only of the input
`M1`/`M2` enclosures it receives. The verdict's own block-invariance residual
(`block_invariance_max_residual`, worst over the three directions, at
n = (1,1,1) for every density) is now p = 1/96:
`[9.9957251106...e-69 +/- 3.45e-145]`; p = 1/192:
`[1.0125474129...e-68 +/- 3.80e-144]`; p = 1/48:
`[1.6087671241...e-68 +/- 4.08e-144]` — down from the pre-fix
`[1.01326852046694e-11 +/- 1.58e-26]` (identical across densities only
because that value was a double-precision rounding artifact of this
function's own arithmetic, not a property of the certified operators). The
largest single charpoly-coefficient radius reported anywhere in
`verdict.certified` (`max_charpoly_radius`, over `charpoly_M1_certified` and
`charpoly_M2_certified`, both directions and all seven coefficients) is
p = 1/96: `2.75...e-45`; p = 1/192: `2.75...e-46`; p = 1/48: `1.06...e-40`.

Unit-modulus spectrum of `P0` (float64 report, never an acceptance
criterion): 7 eigenvalues with `|lambda| > 1 - 1e-9`, matching the seven
conserved modes.

## Mode table (float64 report)

Because `block_dimension = 7` (label `"other"`), the density functional is
not an eigenfunctional of `M1` and no NS-class 4-dimensional block exists, so
the sound speed / transverse-damping / longitudinal-damping outputs of
section 3.2 and the density-mode diffusion coefficient are not defined for
this law. `verdict.mode_table_float64` (`recovery_hydro_verdict.mode_table_float64`)
is the booked substitute the spec's "complete mode table booked either way"
requires: a float64, non-certified degenerate-perturbation-theory report on
the exact `M1`, `M2` for each direction. `M1` is eigendecomposed (numpy
`eig`), eigenvalues are grouped into clusters (tolerance `1e-9`), and within
each cluster `M2` is projected through a biorthonormalized left/right
eigenvector basis; the cluster's `mu2` eigenvalues give `damping =
Re(mu2) - Re(mu1)^2/2`. This is a double-precision report for booking
purposes, never an acceptance value.

| Direction | mu1 (normalized) | damping (normalized) | multiplicity |
|---|---:|---:|---:|
| (1,0,0) | -2.309401 | 205.484909 | 1 |
| (1,0,0) | +2.000000 | 211.866253 | 2 |
| (1,0,0) | +2.000000 | 211.866253 | 2 |
| (1,0,0) | +2.309401 | 205.484909 | 1 |
| (1,0,0) | -2.000000 | 211.866253 | 2 |
| (1,0,0) | -2.000000 | 211.866253 | 2 |
| (1,0,0) | +0.000000 | 76.571037 | 1 |
| (1,1,0) | -2.309401 | 152.395597 | 1 |
| (1,1,0) | -2.000000 | 149.125461 | 2 |
| (1,1,0) | -2.000000 | 264.955565 | 2 |
| (1,1,0) | -0.000000 | 202.052621 | 1 |
| (1,1,0) | +2.309401 | 152.395597 | 1 |
| (1,1,0) | +2.000000 | 264.955565 | 2 |
| (1,1,0) | +2.000000 | 149.125461 | 2 |
| (1,1,1) | -0.000000 | 243.879816 | 1 |
| (1,1,1) | +2.000000 | 205.431933 | 2 |
| (1,1,1) | +2.000000 | 205.431933 | 2 |
| (1,1,1) | +2.309401 | 134.699160 | 1 |
| (1,1,1) | -2.000000 | 205.431933 | 2 |
| (1,1,1) | -2.000000 | 205.431933 | 2 |
| (1,1,1) | -2.309401 | 134.699160 | 1 |

`mu1` is real to float64 precision at every row (imaginary parts <= 1e-15 in
magnitude) for all three directions; `damping (normalized)` differs between
the two rows of a `mu1 = -2.0`/`+2.0`, multiplicity-2 cluster along (1,1,0)
(149.125461 vs. 264.955565), so that cluster's second-order splitting is not
itself degenerate even though its first-order eigenvalue is.

## What this does and does not establish

This is a linear-response (Boltzmann, product-closure) prediction at the
declared reference couplings r(p) for p in {1/96, 1/192, 1/48}, evaluated on
the exact finite operators of the wave-3 staged candidate; correlation
leakage beyond the product closure is not bounded here. Gate H2 tests the
closure assumption against the full tangent dynamics. The result carries no
physical units and constitutes no Clay Millennium claim; it is a statement
about the finite operators of this law, not an adopted physical dispersion
relation.
