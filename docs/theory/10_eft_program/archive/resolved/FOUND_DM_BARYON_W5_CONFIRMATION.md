# FOUND — W5 Moore-Shell DM Weighting Cosmological Verification

**Status:** [Outcome B — UNDERDETERMINED]
**Date:** 2026-05-27
**Campaign ID:** FTD-0211
**Pre-registration Tag:** `preregister-w5-confirmation-v1`
**Execution Commit:** `ae9996e` (pre-reg) / Current (exec)

---

## §1 · Executive Summary

This document presents the final results of the independent cosmological confirmation campaign for the **W5 Moore-shell dark matter weighting scheme (FTD-0211)**. W5 is a per-site weighting scheme where the 12 cuboctahedron sites carry weight $N_{\text{base}} = 4$ and all other shells carry weight 1.

Following the strict methodological protocol locked in `PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md`, we evaluated the W5 predictions for the primordial Helium-4 mass fraction $Y_p$ and the CMB acoustic-peak scale $\ell_1$ against Planck 2018 parameters.

We find that **W5 achieves a dramatic improvement** over the uniform voxel-counting baseline W1. However, because the predicted primordial Helium abundance $Y_p$ deviates from the observed central value by $2.84\%$, the campaign terminates in **Outcome B (UNDERDETERMINED)**. The results are summarized below:

*   **W1 Uniform Baseline:** Deviates from $Y_p$ by $35.08\%$ ($+28.65\sigma$ exclusion) and from $\ell_1$ by $7.29\%$ ($-16.04\sigma$ exclusion). It is strongly excluded.
*   **W5 Active Hypothesis:** Deviates from $Y_p$ by $2.84\%$ ($+2.32\sigma$) and from $\ell_1$ by $0.91\%$ ($+1.99\sigma$). While W5 is in the correct physical family and matches both observables within the $5.0\%$ family threshold, it fails to sharply satisfy the strict $1.5\%$ threshold for both, leading to an **underdetermined** classification.

---

## §2 · Numerical Results

The calculations were executed via `scripts/exploration/verify_w5_cosmology.py` at 50-digit precision. The physical parameters and residuals are tabulated below:

| Parameter | Observed Target | W1 Uniform Baseline | W1 Residual | W5 Cuboctahedron | W5 Residual |
|---|---|---|---|---|---|
| **Baryon fraction $\Omega_b/\Omega_m$** | — | $10/27 \approx 0.370370$ | — | $10/63 \approx 0.158730$ | — |
| **Physical density $\Omega_b h^2$** | — | $0.056083$ | — | $0.024036$ | — |
| **BBN Parameter $\eta_{10}$** | — | $15.3613$ | — | $6.5834$ | — |
| **Helium fraction $Y_p$** | $0.245 \pm 0.003$ | $0.330951$ | $35.08\%$ ($+28.65\sigma$) | $0.251951$ | $2.84\%$ ($+2.32\sigma$) |
| **CMB scale $\ell_1$** | $220.0 \pm 1.0$ | $203.9584$ | $7.29\%$ ($-16.04\sigma$) | $221.9930$ | $0.91\%$ ($+1.99\sigma$) |

---

## §3 · Physical and Ontological Interpretation

### §3.1 · Comparison of W1 vs W5
The uniform voxel-counting scheme (W1) assigns equal weight to every site in the 27-site Moore neighborhood. This yields $\Omega_b/\Omega_m = 10/27$, which translates to a physical baryon density $\Omega_b h^2 \approx 0.0561$. This value is vastly too high for a standard $\Lambda\text{CDM}$ universe with $\Omega_m = 1/3$, forcing BBN to synthesize far too much Helium-4 ($Y_p \approx 0.331$ vs $0.245$ observed). W1 is mathematically and observationally untenable as a cosmological description.

The W5 scheme weights the 12 cuboctahedron sites by $N_{\text{base}} = 4$ based on the framework's internal-multiplicity of fermions. Because the cuboctahedron sites are entirely partitioned into dark matter, this weighting selectively boosts the dark matter density. The resulting baryon-to-matter ratio is:
$$\frac{\Omega_b}{\Omega_m}\bigg|_{\text{W5}} = \frac{10}{63} \approx 0.15873$$

This yields $\Omega_b h^2 \approx 0.02404$, bringing the baryon density down by a factor of 2.3 and aligning it closely with modern precision cosmology.

### §3.2 · Underdetermined Resolution
While W5 represents a massive structural leap, it does not achieve the perfect $1.5\%$ threshold required for a definitive **Outcome A (FOUND)** classification:
1.  **CMB Scale $\ell_1$ (PASS):** The W5 prediction $\ell_1 \approx 222.0$ matches the observed first acoustic peak position ($220.0 \pm 1.0$) to within $0.91\%$ ($1.99\sigma$). This is an extraordinary match for an analytical, zero-parameter prediction.
2.  **Helium Abundance $Y_p$ (FAIL):** The predicted Helium fraction $Y_p \approx 0.2520$ deviates from the observed value ($0.245 \pm 0.003$) by $2.84\%$ ($2.32\sigma$). 

Because $2.84\% > 1.5\%$, **F-a (BBN mismatch)** fires, preventing Outcome A. However, since both residuals are well below $5.0\%$, the weighting is not retired as a post-hoc coincidence (which would be Outcome C). The campaign remains **Outcome B (UNDERDETERMINED)**. W5 is a highly promising candidate that requires either a more detailed multi-fluid acoustic treatment (beyond the pre-registered first-order approximation) or a deeper coupling constant correction.

---

## §4 · Epistemic Status & Integrity

Per CLAUDE.md anti-laundering rules, this campaign maintains absolute epistemic transparency:
*   **No Post-hoc Adjustments:** No parameters ($h = 0.674$, $\Omega_m = 1/3$) were adjusted to force a better fit for $Y_p$.
*   **Outcome B Locked:** The classification is left as underdetermined rather than being "cleaned" or polished into an artificial success.
*   **Provenance Integrity:** This result matches the pre-registered criteria exactly, and the verification script remains committed as an audit trail.
