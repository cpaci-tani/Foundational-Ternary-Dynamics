# PL-4 blind extension — locked predictions for L=257 (<100>, n_perp=3)

Fit: per-group log10 R vs log10 L over L in [np.int64(33), np.int64(65), np.int64(97), np.int64(129), np.int64(193)]; 95% PI half-width = max(t(0.975,3)*RMSE*leverage, 0.05 dex).

| n_z | p_fit | rmse(dex) | R(193) | R_pred(257) | 95% PI lo | 95% PI hi |
|---|---|---|---|---|---|---|
| 1 | 2.608 | 0.0870 | 0.000026 | 0.000013 | 0.000006 | 0.000032 |
| 2 | 2.328 | 0.0938 | 0.000149 | 0.000098 | 0.000038 | 0.000247 |
| 3 | 1.980 | 0.0346 | 0.000651 | 0.000400 | 0.000284 | 0.000564 |
| 4 | 1.810 | 0.0376 | 0.001431 | 0.000921 | 0.000634 | 0.001338 |
| 5 | 1.603 | 0.0639 | 0.002517 | 0.001805 | 0.000957 | 0.003402 |
| 6 | 1.225 | 0.1262 | 0.003962 | 0.003559 | 0.001018 | 0.012445 |
| 7 | 1.544 | 0.0361 | 0.005790 | 0.003934 | 0.002276 | 0.006800 |
| 8 | 1.249 | 0.0615 | 0.007971 | 0.006124 | 0.002410 | 0.015560 |
| 9 | 0.723 | 0.1170 | 0.010407 | 0.010083 | 0.001712 | 0.059395 |

median R_pred(257) over groups = 0.001805
median R(193)      over groups = 0.002517

## Scoring against the locked bands

| n_z | R_obs(257) | inside 95% PI? | R_obs(257) < R(193)? |
|---|---|---|---|
| 1 | 0.000061 | False | False |
| 2 | 0.000005 | False | True |
| 3 | 0.000309 | True | True |
| 4 | 0.000777 | True | True |
| 5 | 0.001415 | True | True |
| 6 | 0.002263 | True | True |
| 7 | 0.003355 | True | True |
| 8 | 0.004704 | True | True |
| 9 | 0.006294 | True | True |

groups inside PI: 7/9   (CONFIRM needs >= 7)
median R_obs(257) = 0.001415   median R(193) = 0.002517

**VERDICT: PREDICTION_CONFIRMED** — the locked extrapolation of the FTD-0252 residual law held at the unmeasured L=257. [MEASURED — blind extension]
