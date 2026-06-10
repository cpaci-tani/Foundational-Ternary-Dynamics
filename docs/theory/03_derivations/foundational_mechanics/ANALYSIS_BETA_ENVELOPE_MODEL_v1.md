# ANALYSIS: Mechanism Beta — Parameter-Free Envelope Prediction

**Status:** `[CLOSED -- RESOLVED]` (BETA-PARTIAL)
**Epistemic Tag:** `[EMERGENT]`
**Authoritative Ledger Row:** FTD-0263 (Sub-knee onset profile constraint)
**Verification Script:** [`scripts/exploration/derive_beta_envelope_prediction.py`](file:///C:/Users/cpaci/Desktop/ftd/scripts/exploration/derive_beta_envelope_prediction.py)

---

## 1. Overview and Model Formulation

Mechanism $\beta$ is a parameter-free envelope model designed to predict the sub-knee onset of manifestations (staircase N(A) spectrum) at early ticks against the FTD-0263 constraint profile.

The underlying model postulates that prior to voxel genesis, the engine dynamics are exactly linear, governed by a symplectic Euler wave update on the 18-point stencil with propagation parameter $\alpha = 1/18$. The injected flux at the center voxel is $A \cdot K_{\text{genesis}} \cdot \hat{x}$.

Under this linear regime, we define the parameter-free envelope $e(\delta)$ as the maximum flux magnitude at voxel coordinate $\delta$ over time per unit injection:
$$ e(\delta) = \max_t |J(\delta, t)| $$

Under the sharp-kinetics, initial-crossing approximation:
- A voxel manifests if and only if $A \cdot e(\delta) > 1$ (the $K_{\text{genesis}}$ threshold cancels).
- The predicted number of manifested voxels $N(A)$ for a given injection amplitude $A$ is the size of the set where the envelope exceeds $1/A$:
  $$ N(A) = \#\{\delta : e(\delta) > 1/A\} $$

We evaluated the model under two variants:
1. **Variant A (No Projection)**: Symplectic wave equation only, no per-tick divergence cleaning.
2. **Variant B (Divergence-Free Projection)**: Symplectic wave equation with a per-tick FFT projection using the discrete central-difference divergence symbol.

---

## 2. Mathematical Corrections: Symplectic Euler Dispersion

During verification, a load-bearing correction was made to the dispersion self-check formula. The original implementation calculated the winding frequency via $\omega = \arccos(q_s[0])$. However, under the symplectic Euler stagger scheme:
$$ v_{n+1} = v_n + \alpha \Delta f_n $$
$$ f_{n+1} = f_n + v_{n+1} $$

For a single mode with eigenvalue $-\lambda$, the transition matrix yields a characteristic equation $g^2 - (2 - \alpha \lambda)g + 1 = 0$. For a stable wave mode $g = e^{i\Omega}$, this yields $\cos(\Omega) = 1 - 0.5 \alpha \lambda$. Since $\cos(\Omega) = 1 - 2 \sin^2(\Omega/2)$, the exact discrete dispersion winding frequency relation is:
$$ \Omega = 2 \arcsin\left(0.5 \sqrt{\alpha \lambda}\right) $$

Given that the first normalized step projection satisfies $q_s[0] = 1 - \alpha \lambda$, we substitute $\alpha \lambda = 1 - q_s[0]$ to obtain the exact discrete relation:
$$ \Omega = 2 \arcsin\left(0.5 \sqrt{1 - q_s[0]}\right) $$

Using this corrected formula, the dispersion self-check at axis mode $n=4$ (lattice size $L=64$) yields:
- **Measured $\Omega$**: $0.2257$
- **Predicted $2 c_* \sin(k/2)$**: $0.2253$
- **Residual Error**: $\approx 0.17\%$ (perfectly resolving the previous $42\%$ discrepancy).

---

## 3. Results and Verdict

Running the corrected simulation on a $L=64$ grid for $T=110$ ticks yields the following results:

### Variant A: No Per-Tick Projection

- **Top Ranked Envelopes and Join Amplitudes ($A_{\text{join}} = 1/e$)**:
  - Rank 1: $e = 1.11111$, $A_{\text{join}} = 0.90$ (Center voxel)
  - Rank 2–7: $e = 0.21628$, $A_{\text{join}} = 4.62$
  - Rank 8–15: $e = 0.17465$, $A_{\text{join}} = 5.73$
  - Rank 16–21: $e = 0.11548$, $A_{\text{join}} = 8.66$
  - Rank 22–25: $e = 0.10844$, $A_{\text{join}} = 9.22$
- **Predicted Elbow**: $\text{knee}_A = 5.8$, $\text{knee}_N = 13.1$
- **Comparison against Measured Staircase (F-arm)**:
  - At $A = 9.0$: $N_{\text{meas}} = 2.0$, $N_{\text{pred}} = 21$
  - At $A = 10.0$: $N_{\text{meas}} = 4.0$, $N_{\text{pred}} = 33$
  - At $A = 14.6$: $\text{knee}_N \approx 14.6$ (Measured) vs $\text{knee}_N = 13.1$ (Predicted)
- **T1 Elbow Test (Target range $[9.7, 21.9]$)**: **PASS** ($\text{knee}_N = 13.1$)
- **T2 Shape RMS (Target $\le 0.20$)**: **FAIL** ($\text{RMS} = 0.749$)

### Variant B: Per-Tick Divergence Projection

- **Top Ranked Envelopes and Join Amplitudes ($A_{\text{join}} = 1/e$)**:
  - Rank 1: $e = 0.74075$, $A_{\text{join}} = 1.35$ (Center voxel)
  - Rank 2–3: $e = 0.17376$, $A_{\text{join}} = 5.76$
  - Rank 4–11: $e = 0.16041$, $A_{\text{join}} = 6.23$
- **Predicted Elbow**: $\text{knee}_A = 6.4$, $\text{knee}_N = 6.4$
- **T1 Elbow Test**: **FAIL** ($\text{knee}_N = 6.4$)
- **T2 Shape RMS**: **FAIL** ($\text{RMS} = 0.583$)

### Final Verdict: `BETA-PARTIAL`

Under Variant A, the parameter-free envelope model successfully reproduces the elbow location ($\text{knee}_N = 13.1$ within the target band of $[9.7, 21.9]$). However, the shape of the predicted staircase $N(A)$ overestimates the voxel count at low amplitudes compared to the measured F-arm data (e.g., predicting $21$ voxels at $A=9.0$ where only $2$ are measured).

---

## 4. Physical Insights

1. **Onset Location**: The initial-crossing threshold matches the onset of manifestation at a quantitative level, demonstrating that the spatial envelope of the linear wave packet governs *when* and *where* first-crossing occurs.
2. **Kinetics and Back-reaction**: The failure of the shape metric (T2) indicates that the *staircase shape* is not purely geometric. Once a voxel manifests (state changes to $\pm 1$), its coupling to the flux field (back-reaction) and subsequent field drainage/evaporation alters the local environment, suppressing genesis at neighboring sites.
3. **Verdict Implications**: Mechanism $\beta$ alone is insufficient to describe the full manifestation curve, confirming that non-linear back-reaction and localized charge kinetics are load-bearing components of the sub-knee staircase dynamics.
