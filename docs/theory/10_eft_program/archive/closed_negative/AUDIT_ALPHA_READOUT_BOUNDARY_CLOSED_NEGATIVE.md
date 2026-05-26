# AUDIT · Candidate A Boundary-Condition Readout Closed-Negative Report (ARC-A1)

**Tag:** [CLOSED NEGATIVE] / canonical
**Date:** 2026-05-26
**LEDGER row:** FTD-0214 (new closed-negative audit claim)
**Depends on:** `docs/theory/10_eft_program/archive/closed_negative/PREREG_ALPHA_READOUT_BOUNDARY_v1.md` ([PRE-REGISTRATION])
**Status:** [CLOSED NEGATIVE] for Candidate A; the boundary spectral ratio $\theta(L)$ is proven to flow to $0$ as $L \to \infty$.

---

## 0 · Executive summary

This audit records the definitive **CLOSED-NEGATIVE** resolution of the **Candidate A (Boundary-Condition Readout)** track under the Alpha Readout Contract (`SPEC_ALPHA_READOUT_CONTRACT.md`), executing the decisive checks pre-registered in [`PREREG_ALPHA_READOUT_BOUNDARY_v1.md`](PREREG_ALPHA_READOUT_BOUNDARY_v1.md).

By utilizing a Fourier-reduced 1D momentum representation of the 18-point discrete Laplacian with open $z$-boundaries, we calculated the exact boundary and bulk spectral gaps up to $L = 128$. The boundary spectral gap $\Delta_{\text{boundary}}$ is proven to be exactly $1.0$ for all $L$, while the bulk spectral gap $\Delta_{\text{bulk}}$ scales quadratically as $O(L^2)$. The ratio $\theta(L) = \Delta_{\text{boundary}} / \Delta_{\text{bulk}}$ asymptotically approaches $0$ as $L \to \infty$, failing the 1% convergence check to the master-quadratic root $x_+ \approx 137.036$. Candidate A is therefore formally archived as **closed-negative**.

---

## 1 · Mathematical proof of the flow to zero

Let the 3D discrete Laplacian be periodic in $x, y$ and open in $z$. Fourier transforming along the lateral coordinates block-diagonalizes the system into independent $L \times L$ tridiagonal systems indexed by 2D momentum $(q_x, q_y)$. 

For the slow-mode sector ($q_x = q_y = 0$), the negative Laplacian tridiagonal matrix $-H$ is:

$$-H = \begin{pmatrix} 
 2 & -1 &  0 & \dots &  0 \\ 
-1 &  2 & -1 & \dots &  0 \\ 
 0 & -1 &  2 & \dots &  0 \\ 
\vdots & \vdots & \vdots & \ddots & \vdots \\ 
 0 &  0 &  0 & \dots &  2 
\end{pmatrix}$$

This is the standard 1D tridiagonal Dirichlet Laplacian.

### 1.1 Bulk spectral gap
The smallest eigenvalue of $-H$ is classically:
$$\lambda_{\text{min}} = 2 - 2 \cos\left( \frac{\pi}{L+1} \right) \approx \frac{\pi^2}{(L+1)^2}$$
So the bulk spectral gap (largest eigenvalue of the Green's function $G = (-H)^{-1}$) is:
$$\Delta_{\text{bulk}} = \frac{1}{\lambda_{\text{min}}} \approx \frac{(L+1)^2}{\pi^2}$$

### 1.2 Boundary spectral gap
The inverse matrix $G = (-H)^{-1}$ has exact analytical entries:
$$G_{j, k} = \frac{\min(j+1, k+1)(L - \max(j, k))}{L+1}$$
At the boundary corners $j, k \in \{0, L-1\}$:
$$G_{0, 0} = \frac{L}{L+1}, \quad G_{0, L-1} = \frac{1}{L+1}$$

The boundary spectral gap is the largest eigenvalue of the $2 \times 2$ boundary Green's function:
$$\Delta_{\text{boundary}} = G_{0, 0} + G_{0, L-1} = \frac{L}{L+1} + \frac{1}{L+1} = 1.0$$
This is an exact algebraic identity for all $L \ge 2$.

### 1.3 Asymptotic ratio
The ratio $\theta(L)$ is:
$$\theta(L) = \frac{\Delta_{\text{boundary}}}{\Delta_{\text{bulk}}} = 2 - 2 \cos\left( \frac{\pi}{L+1} \right) \approx \frac{\pi^2}{L^2}$$
As $L \to \infty$, the ratio flows strictly to $0$:
$$\lim_{L \to \infty} \theta(L) = 0$$

---

## 2 · Numerical verification results

The exact numerical sweep results verified by `explore_boundary_readout_spectrum.py` confirm the proof:

| $L$ | Bulk Gap $\Delta_{\text{bulk}}$ | Boundary Gap $\Delta_{\text{boundary}}$ | Ratio $\theta(L)$ |
|---|---|---|---|
| 8 | 8.290859 | 1.000000 | 0.12061476 |
| 16 | 29.365298 | 1.000000 | 0.03405380 |
| 24 | 63.409139 | 1.000000 | 0.01577060 |
| 32 | 110.422140 | 1.000000 | 0.00905615 |
| 48 | 243.355512 | 1.000000 | 0.00410921 |
| 64 | 428.165344 | 1.000000 | 0.00233555 |
| 128 | 1686.169153 | 1.000000 | 0.00059306 |

---

## 3 · Verdict and archiving consequence

Per the pre-registration criteria:
* The ratio $\theta(L)$ flows to $0$ as $L \to \infty$.
* **Verdict:** Closed-negative (Failure outcome).
* **Consequence:** Candidate A is archived under `docs/theory/10_eft_program/archive/closed_negative/`.

This closes the boundary-condition self-consistency track. The search space for MC-T4.3 narrows to Candidate C and B2.
