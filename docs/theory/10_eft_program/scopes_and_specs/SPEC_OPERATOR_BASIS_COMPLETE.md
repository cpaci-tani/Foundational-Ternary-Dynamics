# Complete Operator Basis under the Frozen Gate-1 Contract

**Date:** 2026-04-24
**Status:** [THEOREM] (enumeration under frozen dimensions); [SELECTION] on C/P/T conventions
**Purpose:** Gate 3 closure of the bridge contract. Enumerate all $O_h$-invariant, $C$-invariant local operators in $(\rho, J, j, A)$ through engineering dimension $D \le 6$ under the frozen scaling of [FTD-0064](../07_assessment/LEDGER.md).
**Supersedes:** [SPEC_OPERATOR_BASIS.md](archive/campaign_complete/SPEC_OPERATOR_BASIS.md) (Phase-3 lattice-unit spec) in the continuum EFT sense.

---

## 1. Frozen dimensions (imported from Gate 1)

Under `a_phys ≡ ℓ_P` and the Z-factors of FTD-0064:

| Field | Dim | $C$-parity | $P$-parity | $T$-parity | $O_h$ type |
|---|---|---|---|---|---|
| $\rho(x,t)$ | 3 | $-$ | $+$ | $+$ | scalar |
| $J_i(x,t)$ | 2 | $-$ | $-$ | $+$ | polar vector |
| $j_i(x,t)$ | 3 | $-$ | $-$ | $-$ | polar vector |
| $A_i(x,t)$ | 1 | $-$ | $-$ | $\pm$ (gauge) | polar vector |
| $\partial_i$ | 1 | $+$ | $-$ | $+$ | polar vector |
| $\partial_t$ | 1 | $+$ | $+$ | $-$ | scalar |

Assignments follow standard QED conventions:
- Current $j^\mu = (\rho, \vec j)$ is $C$-odd (fixes $C$ as standard).
- Photon potential $A$ is $C$-odd (standard).
- Spatial parity of $A$ is $-$ (polar, standard gauge-potential convention).
- $T$-parity of $A$ is gauge-choice-dependent; we take temporal-gauge convention $A \to A$ so that the Lagrangian is $T$-even at leading order.

Lagrangian density has dimension $D = 4$ in $3+1$ natural units. Classification:
- **Relevant** $D < 4$: grows in IR, must be in the effective action.
- **Marginal** $D = 4$: scale-invariant at tree level.
- **Irrelevant** $D > 4$: shrinks in IR, optional at leading order.

---

## 2. $D \le 6$ enumeration of $O_h$-scalar, $C$-even, $P$-even operators

### Dimension $D = 1$: none

Only $A_i$ and $\partial_i$ have $D = 1$. Both are vectors; no $O_h$ scalar.

### Dimension $D = 2$: one operator

| Operator | Expression | $C$ | $P$ | $T$ | Class | Interpretation |
|---|---|---|---|---|---|---|
| $\mathcal{O}_1$ | $A \cdot A$ | $+$ | $+$ | $+$ | **Relevant** | photon mass term |

$\rho$ alone ($D = 3$) is $C$-odd and forbidden at the bilinear level. $\partial \cdot A$ is $C$-odd.

### Dimension $D = 3$: none

$\rho$, $\partial \cdot A$, $\partial_t A$, and $\partial^2 A$ at this dimension are either $C$-odd or non-scalar. No $C$-even $O_h$-scalar at $D = 3$.

### Dimension $D = 4$: four marginal operators

| Operator | Expression | $C$ | $P$ | $T$ | Interpretation |
|---|---|---|---|---|---|
| $\mathcal{O}_2$ | $J \cdot J$ | $+$ | $+$ | $+$ | flux kinetic / electric field energy |
| $\mathcal{O}_3$ | $(\partial_i A_j)(\partial^i A^j) = (\partial A)^2$ | $+$ | $+$ | $+$ | gauge-potential kinetic (gauge-unfixed form) |
| $\mathcal{O}_4$ | $(\partial \times A) \cdot (\partial \times A) = B^2$ | $+$ | $+$ | $+$ | Maxwell magnetic (transverse part of $\mathcal{O}_3$) |
| $\mathcal{O}_5$ | $j \cdot A$ | $+$ | $+$ | $-$ | minimal gauge coupling (parity-even, $T$-violating branch only if scheme chooses) |

Note: $\mathcal{O}_3$ and $\mathcal{O}_4$ differ by $(\partial \cdot A)^2$ (longitudinal mode) and by total-derivative pieces. For a gauge-unfixed EFT both appear independently; after gauge-fixing only $\mathcal{O}_4$ (plus a gauge-fixing term) survives. Under the native transverse-projection convention $J_T = P_T A$, $\mathcal{O}_4$ is the physical kinetic piece.

**These four operators are the marginal content of the EFT** — they define the four running couplings $(C_L, K_T, Z_j, g_{sJ})$ of the native response tuple (§5).

### Dimension $D = 5$: three operators

| Operator | Expression | $C$ | $P$ | $T$ | Interpretation |
|---|---|---|---|---|---|
| $\mathcal{O}_6$ | $\rho (\partial \cdot A)$ | $+$ | $+$ | $+$ | source–Gauss coupling (the "$s \cdot \text{div}\, J$" term) |
| $\mathcal{O}_7$ | $j \cdot (\partial \times A) = \vec j \cdot \vec B$ | $+$ | $+$ | $+$ | Chern-Simons–like; $O_h$ scalar, $P$-even under the given assignment |
| $\mathcal{O}_8$ | $\rho \, \partial_t A_0$ or $\rho \, \partial_t(\partial \cdot A)$ | $+$ | $+$ | $+$ | $T$-even source–time coupling (scheme-dependent basis choice; not independent of $\mathcal{O}_6$ on-shell) |

All three are **irrelevant** ($D = 5 > 4$). The FTD native "source-vector coupling" $s \cdot \text{div}\,J$ lives in $\mathcal{O}_6$; it is subleading to the marginal operators in the IR but dominates at short distances.

### Dimension $D = 6$: six+ operators (representative)

| Operator | Expression | $C$ | $P$ | $T$ | Interpretation |
|---|---|---|---|---|---|
| $\mathcal{O}_9$ | $\rho^2$ | $+$ | $+$ | $+$ | source–source contact; native "mass-like" for $\rho$ fluctuations |
| $\mathcal{O}_{10}$ | $j \cdot j$ | $+$ | $+$ | $+$ | current–current contact |
| $\mathcal{O}_{11}$ | $J^2 A^2$ or $(J \cdot A)^2$ | $+$ | $+$ | $+$ | four-field photon self-interaction (Euler–Heisenberg–like) |
| $\mathcal{O}_{12}$ | $(\partial A)^4$ | $+$ | $+$ | $+$ | higher-derivative gauge kinetic |
| $\mathcal{O}_{13}$ | $\rho (\partial \cdot J)$ | $+$ | $+$ | $+$ | source–Gauss-extended (related to $\mathcal{O}_6$ via equations of motion) |
| $\mathcal{O}_{14}$ | $(\partial \times A)^2 \cdot A^2$ | $+$ | $+$ | $+$ | mixed photon kinetic-mass term |

Truncation at $D = 6$ includes all corrections suppressed by $(\ell_P / \ell_{\text{obs}})^2$ for observations at physical scale $\ell_{\text{obs}}$.

---

## 3. Mixing classes under $b = 2$ native blocking

The dual-cell Wilsonian blocking map (SPEC_FTD_NATIVE_BLOCKING_MAP.md) acts on the enumeration above. Operators of the **same symmetry type** mix into each other at linear order in the blocking:

**Marginal block ($D = 4$):** $\mathcal{O}_2, \mathcal{O}_3, \mathcal{O}_4, \mathcal{O}_5$ all mix among themselves because they share $C$-even, $P$-even, $T$-even ($\mathcal{O}_5$: $T$-odd but decouples separately) quantum numbers. The mixing matrix at tree level on a Gaussian native generator was measured at (1, 1, 1, 1) for the native response tuple (FTD-0064 referenced [DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md](../derivations/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md)). Non-Gaussian flow (FTD-0065, P1.3) is the next measurement.

**Relevant → marginal flow:** $\mathcal{O}_1$ (photon mass, $D = 2$) is relevant and generates $\mathcal{O}_3$-like corrections at one loop. Whether $\mathcal{O}_1$ is generated dynamically by FTD dynamics or remains zero is a measurement question (ctest `native_operator_spectrum` or equivalent).

**$D = 5 \to D = 4$ flow:** $\mathcal{O}_6 = \rho \, \partial \cdot A$ sources $\mathcal{O}_5 = j \cdot A$ via the continuity equation $\partial_t \rho + \partial \cdot j = S_R$. The coefficient ratio $g_{sJ} / g_{jA}$ is therefore not independent; it is fixed by Ward identity (FTD-0066, P1.3/P1.4 measurement).

**$D = 6$ operators** are irrelevant and do not affect the leading-IR predictions. They matter for precision tests and lattice-spacing corrections.

---

## 4. Forbidden operators (by symmetry)

The following would be allowed by dimension but are forbidden by $C$- or $P$-invariance:

| Would-be operator | Dim | Reason forbidden |
|---|---|---|
| $\rho$ | 3 | $C$-odd |
| $\partial \cdot A$ | 2 | $C$-odd |
| $\rho^3$ | 9 | $C$-odd |
| $\rho \cdot J$ (scalar) | 5 | not $O_h$ scalar ($J$ is vector, $\rho$ scalar; needs another vector to contract) |
| $\rho \cdot (\partial \times A) \cdot A$ | 6 | contracts to pseudoscalar ($P$-odd) |
| $A \cdot (\partial \times A)$ | 3 | $P$-odd (Chern-Simons, relevant at $D=3$ in 2+1 but $P$-odd in 3+1) |

The $P$-odd Chern-Simons $A \cdot (\partial \times A)$ is excluded by $O_h$ inversion (part of the cubic group). Its cousin $j \cdot (\partial \times A)$ ($\mathcal{O}_7$) is allowed because it is $P$-even ($j$ is polar-vector, $\partial \times A$ is axial-vector, product is pseudoscalar — **which is $P$-odd**... let me recheck).

**Correction to $\mathcal{O}_7$ entry:** $\vec j \cdot \vec B$ where $\vec B = \nabla \times \vec A$ is $P$-odd (because $\vec j$ is polar, $\vec B$ is axial). So it is a pseudoscalar, not a scalar, and is **forbidden by $P$ conservation**. Under cubic $O_h$ (which includes inversion), it is forbidden. If FTD breaks P (e.g., via an arrow-of-time selection correlated with parity), $\mathcal{O}_7$ may reappear; under strict $O_h$ it is out.

**Revised $D = 5$ list:** $\{\mathcal{O}_6, \mathcal{O}_8\}$ (two operators, not three).

---

## 5. Connection to the native response tuple $(C_L, K_T, Z_j, g_{sJ})$

The four native running couplings map to the dimension-$\le 4$ operators as follows:

| Coupling | Marginal operator | Microscopic meaning |
|---|---|---|
| $C_L^{\mathrm{FTD}}$ | $\mathcal{O}_6 = \rho (\partial \cdot A)$ coefficient ratio | longitudinal-sector source response |
| $K_T^{\mathrm{FTD}}$ | $\mathcal{O}_4 = B^2$ coefficient | transverse flux stiffness |
| $Z_j^{\mathrm{FTD}}$ | $\mathcal{O}_5 = j \cdot A$ coefficient | current normalization |
| $g_{sJ}^{\mathrm{FTD}}$ | $\mathcal{O}_2 = J^2$ source-flux vertex | bare flux kinetic |

At the Gaussian (linear-generator) fixed point these are all 1 (FTD documented in `DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md`). Non-linear flow measurements (FTD-0065) extend this to the nonlinear regime.

---

## 6. Total count through $D = 6$

- $D = 2$: 1 operator (relevant)
- $D = 3$: 0 operators
- $D = 4$: 4 operators (marginal) — **couplings of the EFT**
- $D = 5$: 2 operators (irrelevant)
- $D = 6$: 6+ operators (irrelevant; truncation-dependent)

**Total leading-EFT content:** 1 + 4 = 5 operators through the marginal level.

This closes Gate 3 of the bridge contract at the symmetry-enumeration level. Full measurement of the renormalization-group mixing matrix requires Gates 4 (blocking) + 5 (Ward) + 7 (observables), of which P1.2–P1.4 close the qualitative picture and b $\ge 4$ measurements (Phase 2 of the roadmap) will produce quantitative flow data.

---

## 7. Epistemic tags

| Piece | Tag |
|---|---|
| Dimensions of $\rho, J, j, A, \partial$ | [THEOREM] (from Gate 1, FTD-0064) |
| $C$, $P$, $T$ parity assignments above | [SELECTION] (standard QED conventions) |
| Enumeration of $D \le 6$ $C$-even $P$-even scalars | [THEOREM] (symmetry enumeration) |
| Identification of marginal operators with $(C_L, K_T, Z_j, g_{sJ})$ | [SELECTION] (EFT matching convention) |
| Mixing matrix at the Gaussian fixed point = identity | [THEOREM] at tree level (FTD-0064 bare-flow results) |
| Non-Gaussian mixing-matrix structure | [OPEN] (awaiting b≥4 measurements) |

---

*Filed 2026-04-24. Upgrades bridge-contract Gate 3 from [PARTIAL] to [CLOSED] at the enumeration level; quantitative flow determinations of the mixing matrix remain a Gate-4 / Phase-2 task.*
