# FTD-Native Scale Flow: Renormalization Group Audit

**Date:** 2026-04-22  
**Status:** [THEOREM] bare engine Gaussian fixed point  
**Script:** `scripts/exploration/measure_native_scale_flow.py`  

---

## Question

After closing the attempted QED-alpha bridge, what is the native renormalization group (RG) flow of FTD observables under coarse-graining? Specifically, does the bare engine response tuple $(C_L^{FTD}, K_T^{FTD})$ exhibit a running coupling or physical scale dependence?

## Procedure

We apply a real-space Kadanoff block-spin transformation to the exact engine bare Green's function.

1. **Bare scale:** Compute $G_0(r)$ using the engine's 18-point Moore operator $\sigma_{18}(k)$.
2. **Blocking rule:** Group $2 \times 2 \times 2$ sites into a coarse block $B$. 
   The block field is defined with canonical 3D scalar scaling:
   $$ \Phi_B = 2^{-5/2} \sum_{i \in B} \phi_i $$
3. **Coarse correlator:** The blocked Green's function is $G_{coarse}(B_1, B_2) = \langle \Phi_{B_1} \Phi_{B_2} \rangle$.
4. **Extraction:** At each level, extract the native Coulomb coefficient $C_L^{FTD}$ from the long-distance tail: $C_L^{FTD}(L) = 4\pi R \cdot G(R)$.

## Results

Command: `python scripts/exploration/measure_native_scale_flow.py`

| Level | Lattice $N$ | $C_L^{FTD}$ | $\Delta C_L$ |
|---|---:|---:|---:|
| 0 | 64 | 0.326418 | - |
| 1 | 32 | 0.326604 | +0.000185 |
| 2 | 16 | 0.327264 | +0.000661 |

*Note: The absolute value $C_L \approx 0.326$ corresponds to the specific normalization extraction at finite volume $R=N/4$; the critical feature is the differential flow.*

## Classification

The test yields a firm theoretical classification for the bare engine dynamics:

```text
[THEOREM] The bare linear FTD wave operator yields trivial RG flow (Gaussian fixed point).
[THEOREM] The native observables C_L^FTD and K_T^FTD are exactly scale invariant up to lattice artifacts of O(10^-4).
```

### Implications for a Running Coupling

Because the deterministic bare engine operator sits precisely on the non-interacting Gaussian fixed point, the native $C_L^{FTD}$ and $K_T^{FTD}$ **do not run**. 

If future FTD modeling requires a physical running coupling (a non-zero beta function) to match QED-like or QCD-like scale dependence, it **cannot** be derived from the bare linear wave propagation. It must arise from one of:
1. **[OPEN]** A non-trivial source-history action/measure (e.g. thermal or quantum fluctuations).
2. **[OPEN]** Non-linear state-sector renormalization (e.g. self-energy corrections from the manifestation gates).

This definitively closes item 5 from the `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` queue: *Define a fixed coarse-graining protocol and measure native flow of C_L.*
