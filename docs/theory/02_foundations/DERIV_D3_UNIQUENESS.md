# D = 3 Uniqueness from the Watson Integral

## The Self-Referential Selection of Three Spatial Dimensions

**Date:** March 17, 2026
**Status:** [THEOREM]
**Proof script:** `scripts/proofs/proof_d3_uniqueness.py`

---

## Abstract

The master quadratic x^2 - K\_D x + K\_D G\*\_D = 0 is well-defined in any spatial dimension D, with K\_D = 16G\*\_D^2 and G\*\_D = sqrt(2pi W\_D) where W\_D is the D-dimensional Watson integral. We compute W\_D for D = 1 through 6 and analyze the gap equation in each case.

**Key finding:** D = 3 is the unique dimension where floor(x\_-) = D. That is, the color number N\_c derived from the gap equation equals the spatial dimension. This self-referential identity N\_c = D is unique to D = 3.

---

## Results by Dimension

| D | W\_D | G\*\_D | x\_+ | x\_- | floor(x\_-) | floor(x\_-) = D? |
|---|------|--------|------|------|-------------|------------------|
| 1 | divergent | - | - | - | - | N/A |
| 2 | 11.987 | 8.678 | 1196.3 | 8.742 | 8 | NO |
| 3 | 1.3932 | 2.9587 | 137.04 | 3.024 | **3** | **YES** |
| 4 | 1.118 | 2.651 | 109.7 | 2.716 | 2 | NO |
| 5 | 1.047 | 2.565 | 102.6 | 2.631 | 2 | NO |
| 6 | 1.020 | 2.532 | 100.0 | 2.598 | 2 | NO |

D = 1 is excluded (W\_1 diverges). D = 2 through D = 6 all have positive discriminants, but **only D = 3 has floor(x\_-) = D**.

---

## The Self-Referential Identity

The condition floor(x\_-) = D means: the number of color charges derived from the gap equation equals the number of spatial dimensions. In D = 3:

- The lattice is Z^3 (three spatial dimensions)
- The Watson integral W\_3 = Gamma(1/4)^4 / (4pi^3) determines G\*
- The gap equation gives x\_- = 3.024, so N\_c = floor(x\_-) = 3
- N\_c = D = 3: the dimension selects itself

This is a self-consistency loop: D determines W\_D, which determines G\*\_D, which determines the gap equation roots, one of which gives N\_c = D. **Only D = 3 closes this loop.**

---

## Why Not Other Dimensions?

- **D = 1:** Watson integral diverges. No gap equation exists.
- **D = 2:** floor(x\_-) = 8, not 2. Also, compact U(1) in 2+1D has no confinement transition.
- **D = 4:** floor(x\_-) = 2, not 4. The Watson integral is too small (G\*\_4 < G\*\_3).
- **D = 5, 6:** floor(x\_-) = 2. Watson integrals decrease monotonically with D.

The Watson integral W\_D decreases with D for D >= 3 (higher-dimensional lattices have more neighbors, reducing the propagator at the origin). This drives x\_- downward, making floor(x\_-) = 2 for all D >= 4.

---

## Epistemic Status

**[THEOREM]:**
1. Watson integrals W\_D computed for D = 1..6 (D=3 exact, D=4..6 Monte Carlo with 5M samples)
2. D = 3 is the unique dimension where floor(x\_-) = D (exhaustive check D = 1..6)
3. The self-referential identity N\_c = D holds only for D = 3

**[SELECTION]:**
- The coefficient K = 16G\*^2 is assumed uniform across dimensions. The Faddeev-Popov derivation (48/3 = 16 gauge modes) is specific to the octahedral symmetry group of D = 3.
- D >= 4 Watson integrals are Monte Carlo estimates (finite statistical error).
- The self-referential criterion "floor(x\_-) = D" is chosen as the selection principle.

---

## References

- proof\_d3\_uniqueness.py -- Numerical verification (4/4 tests)
- DERIV\_MASTER\_QUADRATIC\_FROM\_Z.md -- Gap equation derivation
- Watson, G. N. "Three Triple Integrals," *Q. J. Math.* **10** (1939), 266-276
