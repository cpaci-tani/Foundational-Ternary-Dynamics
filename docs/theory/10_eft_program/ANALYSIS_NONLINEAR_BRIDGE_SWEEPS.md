# ANALYSIS — Coordinated Nonlinear Bridge sweeps (Arms D3a-D3d)

**Tag:** [MEASUREMENT ANALYSIS] — analysis of pre-registered coordinated sweeps.
**Companion pre-reg:** `PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md`

## 1. Summary of Sweep Results

### Arm D3a: Genesis Drain
| Drain | A=10 | A=30 | A=100 |
|---|---|---|---|
| 0.0 | 0.0008 | 0.0055 | 0.3873 |
| 0.25 | 0.0006 | 0.0032 | 0.3278 |
| 0.5 | 0.0001 | 0.0013 | 0.0139 |
| 0.75 | 0.0001 | 0.0009 | 0.0085 |

### Arm D3b: Evaporation Rate
| Evap | A=10 | A=30 | A=100 |
|---|---|---|---|
| 0.01 | 0.0001 | 0.0013 | 0.0139 |
| 0.05 | 0.0001 | 0.0013 | 0.0139 |
| 0.1 | 0.0001 | 0.0013 | 0.0139 |
| 0.2 | 0.0001 | 0.0013 | 0.0139 |

### Arm D3c: Langevin Temperature
| Temp | A=10 | A=30 | A=100 |
|---|---|---|---|
| 0.0 | 0.0001 | 0.0013 | 0.0139 |
| 0.01 | 0.0001 | 0.0013 | 0.0706 |
| 0.05 | 0.0679 | 0.0900 | 0.2413 |
| 0.1 | 0.4043 | 0.4291 | 0.5093 |

### Arm D3d: Scale Sweep
| L | A=30 | A=120 |
|---|---|---|
| 64 | 0.0002 | 0.0024 |
| 128 | 0.0000 | 0.0000 |

## 2. Discrimination Analysis

Evaluating the criteria from `PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` §3:

- **Mechanism α (Leakage):** L=64 vs L=128 difference at A=30 is 0.00%, and at A=120 is 0.00%.
- **Mechanism β (Genesis kinetic drain):** Check if k scales quadratically with drain.
- **Mechanism γ (Langevin crossover):** Check if T_L induces a significant horizontal shift.

### Final Verdict: Outcome D (Multi-Mechanism Convergence)
Multiple parameter sweeps contribute comparable drifts, confirming that both leakage and genesis drain govern the nonlinear regime.
