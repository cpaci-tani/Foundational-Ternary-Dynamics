# FTD-0763 — Fractional-Center Gauss Observer

**Status:** `[DERIVED OBSERVER EXTENSION; SELECTED PIECEWISE CHART]`

## Need

The ternary constituents already have continuous effective positions

\[
 x_a=n_a+r_a,
\]

with integer anchor `n_a` and subcell remainder `r_a`.  A state-only dressing
observer that exists only when the pair centroid is an integer is therefore
not defined on ordinary moving histories.

## Construction

For physical centroid

\[
 c=\frac{x_++x_-}{2},
\]

select the deterministic support chart

\[
 C=\operatorname{round}(c).
\]

The compact induced-site graph remains the integer cube
`C+[-R,R]^3`. The polarity density is evaluated at the actual fractional
constituent positions. On that graph solve the same zero-crossing,
minimum-energy Gauss problem used by FTD-0739. Nothing is translated,
interpolated, or added to the physical field.

The resulting record carries both centers:

- `physical_center = c`, used for radial characteristic directions;
- `support_center = C`, used for integer support membership and its boundary
  ledger.

Shells remain integer Chebyshev shells about `C`; their radial unit vectors
point from `c`. This preserves the old observer exactly when `c=C`.

## Exact properties

Within a fixed chart the construction retains:

1. the supplied fractional polarity density;
2. exact neutrality and density containment;
3. the finite-graph minimum-energy Gauss solution;
4. zero support-boundary crossing;
5. exact integer-translation and proper-cubic covariance;
6. polarity conjugation;
7. state-only dependence and no mutation of dynamics.

At a half-cell chart seam, `round(c)` changes discontinuously. Exact continuous
fractional-translation covariance is not claimed. The two adjacent selected
finite supports may return different representatives of the same global Gauss
affine space. Seam sensitivity must be measured rather than hidden.

## Epistemic limit

Observer validity proves only that an instantaneous field can be decomposed
against the selected fractional-density Gauss representative. It does not by
itself prove co-motion, binding, particle identity, or momentum balance.
Those require time-relative morphology and stress/momentum gates.

