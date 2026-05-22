# Structure-2 Ward-Valid Gauge Completion Audit

**Date:** 2026-04-22  
**Status:** Negative cross-check for universal ppb alpha closure  
**Scope:** Fixed verification calculation, not a numerical search  
**Script:** `scripts/exploration/gpu_plan_priority4_structure2.py`  
**Raw outputs:** `scripts/exploration/outputs/priority4_periodic_strict_cases_N1024.jsonl`, `scripts/exploration/outputs/priority4_periodic_strict_cases_N1024_strict_rows.csv`

---

## Executive summary

The Structure-2 two-U(1) BCC scalar-loop test was implemented as a Ward-valid gauge calculation with both bubble and seagull terms included. The result does **not** reproduce the Structure-1 one-loop alpha closure.

For the natural handoff matter content,

```text
scalar q = (1,0)
M = sqrt(134.012207541816) = 11.576364176278
```

the strict periodic-BZ result at `N=1024` is:

```text
unit-charge delta_K_PP = +7.793246508116e-07
x_S2_primary           = 137.036171847817
residual vs x_S1       = +1259.797 ppb
Ward max |Pi_ii(0)|    = 1.053e-18
```

The project threshold was:

```text
abs(residual_ppb) < 30       confirmed
30 <= abs(residual_ppb) <= 300 ambiguous
abs(residual_ppb) > 300      does not reproduce Structure-1 closure
```

Therefore the tested Structure-2 scalar gauge completions do **not** reproduce the Structure-1 ppb correction. This is a claim-boundary result, not a recovery target.

---

## Claim impact

| Claim | Status after this audit |
|---|---|
| CM arithmetic / master quadratic gives `x_+ = 137.036171...` | Unchanged. Arithmetic core remains intact. |
| Tree-level `x_+` matches `1/alpha` at about `1.258 ppm` | Unchanged empirical match. |
| Structure-1 one-loop scalar EFT gives a ppb-scale correction | Still measured within that specific scheme. |
| Structure-1 ppb correction is scheme-independent physical output | Not supported. Downgrade to scheme-specific unless a unique matching principle is derived. |
| Natural Structure-2 two-U(1) scalar gauge completion reproduces Structure-1 | Falsified under tested scalar matter assumptions. |

Recommended tag language:

- CM/master quadratic: **[THEOREM]** where already proven.
- `x_+ ↔ 1/alpha` tree-level match: **[STRONGLY MOTIVATED CONJECTURE]** or empirical arithmetic match, not theorem.
- Structure-1 one-loop ppb correction: **[SELECTION]** or **[CONJECTURE]**, scheme-conditional.
- Structure-2 scalar gauge completion tested here: **negative audit result**.
- FTD-to-EFT matching principle: **[OPEN]**.

---

## What was tested

The script implements the BCC Peierls-link scalar loop:

```text
delta = (a/2) * (sx, sy, sz), sx, sy, sz in {-1,+1}
a = 2/3
D(k) = M^2 + (8/a^2) * [1 - cos(kx a/2) cos(ky a/2) cos(kz a/2)]
```

The strict result uses a transverse gauge mode `A_x(Q z)` and computes:

```text
Pi_xx(Q) = integral [
    Re(W_xx(k)) / D(k)
    - Re(V_x(k,Q) * V_x(k+Q,-Q)) / (D(k) * D(k+Q))
]
```

with the BCC external lattice momentum:

```text
qhat_BCC^2(Q) = (8/a^2) * [1 - cos(Q a/2)]
delta_K_PP(Q) = Pi_xx(Q) / qhat_BCC^2(Q)
```

The seagull term is essential. Bubble-only `q=0` is retained only as a handoff diagnostic.

---

## Validation checks

The physically decisive runs used the periodic Peierls-link integration cell:

```text
k_i in [-2*pi/a, 2*pi/a)
```

This cell passes the Ward check. The older framework-BZ cell,

```text
k_i in [-pi/a, pi/a)
```

is retained only for reproducing the handoff's literal bubble-only diagnostic. It does not pass the strict Ward check for this Peierls-link gauge calculation.

Validation status:

| Check | Result |
|---|---|
| GPU available | RTX 5090 via WSL2 / CuPy |
| CPU/GPU cross-check | Passed at `N=16` for all tested cases |
| Ward identity | Passed in periodic BZ, max `|Pi_ii(0)| ~ 1e-18` to `4e-17` |
| Isotropy | `Pi_xx(Q_z)` and `Pi_yy(Q_z)` agree within convergence tolerance |
| Small-q stability | Stable plateau / linear extrapolation in `qhat_BCC^2` |
| N convergence | Stable through `N=1024`; `N=2048` not needed because results are far from decision boundary |
| Numerical search | None. Only fixed cases from the handoff follow-up were run. |

---

## Strict matter-case results

Command:

```powershell
wsl.exe -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/cpaci/Desktop/ftd && python3 scripts/exploration/gpu_plan_priority4_structure2.py --mode strict --bz periodic --N 1024 --q-list 1,2,3,4 --cpu-check 16 --cases S2-A,S2-B,S2-C,S2-D,S2-E --run-name priority4_periodic_strict_cases_N1024"
```

Results:

| Case | Matter content | Unit `delta_K` | `x_+` factor | `x_S2` | Residual vs Structure-1 | Verdict |
|---|---:|---:|---:|---:|---:|---|
| S2-A | scalar `q=(1,0)`, `M=sqrt(M^2)` | `+7.793246508116e-07` | `0.5` | `137.036171847817` | `+1259.797 ppb` | Does not reproduce |
| S2-B | scalar `q=(1,1)`, `M=sqrt(M^2)` | `+7.793246508116e-07` | `2.0` | `137.036173016804` | `+1268.328 ppb` | Does not reproduce |
| S2-C | scalar `q=(1,-1)`, `M=sqrt(M^2)` | `+7.793246508116e-07` | `0.0` | `137.036171458155` | `+1256.954 ppb` | Does not reproduce |
| S2-D | scalar `q=(1,0)`, `M=x_-` | `+1.350626545993e-03` | `0.5` | `137.036846771428` | `+6184.947 ppb` | Does not reproduce |
| S2-E | scalar `q=(1,0)`, `M=sqrt(x_+ x_-)` | `+1.175228241950e-08` | `0.5` | `137.036171464031` | `+1256.997 ppb` | Does not reproduce |

Interpretation:

The scalar matter scan does not rescue the Structure-2 gauge interpretation. Cases S2-A, S2-C, and S2-E are all near the tree-level residual because the Ward-valid scalar correction is too small or mapped away from the `x_+` mode. Case S2-D overcorrects in the wrong direction. S2-B also fails.

---

## Handoff literal diagnostic

The original handoff formula was bubble-only and omitted the seagull term. That quantity is not gauge-invariant, but the script reports it to reproduce the handoff shorthand.

Framework-BZ literal diagnostic:

```text
Pi_bubble_xx  = +7.091094244023e-04
Pi_bubble_yy  = +7.091094244023e-04
Pi_bubble_zz  = +7.091094244023e-04
Pi_bubble_sum = +2.127328273207e-03
Pi_bubble_avg = +7.091094244023e-04
```

Mapped with the handoff factor `(1 + coth(theta))/2 = 1.022564839217`:

```text
handoff residual from sum = +17131.092 ppb
handoff residual from avg =  +6548.333 ppb
```

This diagnostic should not be used as the Structure-2 physics result.

---

## Path B conclusion

The goal is not to recover the Structure-1 ppb number by changing Structure-2 inputs. The goal is to classify which claims are robust.

This audit supports the following Path B position:

1. Preserve the CM arithmetic and master quadratic as the hard core.
2. Present the `1.258 ppm` tree-level alpha match honestly as the robust numerical match.
3. Treat the Structure-1 ppb correction as scheme-specific unless a unique FTD-to-EFT matching principle is derived.
4. Record the Ward-valid Structure-2 scalar gauge completion as a negative cross-check.
5. Do not run open-ended charge, mass, regulator, or discretization searches for a near-miss.

The next theoretical target is not a better numerical fit. It is a first-principles FTD-to-EFT matching rule:

```text
FTD local ontology -> unique continuum fields -> unique matter content -> unique regulator/counterterm prescription
```

Until that bridge exists, ppb-level EFT corrections should be labeled conditional rather than universal.

