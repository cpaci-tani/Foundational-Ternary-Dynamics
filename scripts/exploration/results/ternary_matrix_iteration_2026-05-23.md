# Ternary-Matrix BCC-Snap Test -- Results

**Date:** 2026-05-23
**Pre-registration:** docs/theory/09_mathematical/PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md
**Git tag:** preregister-ternary-matrix-bcc-snap-v1

---

## Outcome

**Outcome D.** Iteration fails to converge under at least one natural (A, B). [CLOSED NEGATIVE] on the construction as stated; would need a different iteration rule.

---

## Summary statistics

- Primary sweep runs: 40 (2 A x 4 B x 5 seeds)
- B=0 control runs: 10 (2 A x 5 seeds)
- Random-B control runs: 100 (2 A x 10 random B x 5 seeds)

- (A, B) pairs satisfying P1 (all 5 seeds snap to BCC at threshold 1e-06): **0** out of 8
- B=0 control snaps (threshold 0.001, loose): 0 out of 10
- Random-B BCC-snap rate (threshold 1e-06): 0 / 100 = 0.0%

---

## Per-(A, B) primary-sweep summary

| A | B | seeds converged | mean d_BCC | mean d_axis | snap rate (d_BCC < 1e-6) |
|---|---|---|---|---|---|
| A1_Gstar_powers | B1_pos_sym | 5/5 | 7.160e-01 | 2.242e-01 | 0/5 |
| A1_Gstar_powers | B2_asym_BCC | 5/5 | 7.198e-01 | 2.192e-01 | 0/5 |
| A1_Gstar_powers | B3_cyclic_antisym | 5/5 | 7.277e-01 | 2.168e-01 | 0/5 |
| A1_Gstar_powers | B4_toeplitz_sign | 5/5 | 7.166e-01 | 2.226e-01 | 0/5 |
| A2_Gstar_varpi_pi | B1_pos_sym | 5/5 | 1.425e-01 | 7.934e-01 | 0/5 |
| A2_Gstar_varpi_pi | B2_asym_BCC | 5/5 | 5.638e-01 | 4.409e-01 | 0/5 |
| A2_Gstar_varpi_pi | B3_cyclic_antisym | 0/5 | 4.648e-01 | 6.805e-01 | 0/5 |
| A2_Gstar_varpi_pi | B4_toeplitz_sign | 0/5 | 4.529e-01 | 6.599e-01 | 0/5 |

---

## Full data

See `ternary_matrix_iteration_2026-05-23.csv` (150 rows).
