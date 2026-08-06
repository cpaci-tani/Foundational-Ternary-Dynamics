# PREREG — W5 Moore-Shell DM Weighting Independent Cosmological Confirmation Campaign

**Status:** [PRE-REGISTRATION — hash-locked before execution]
**Date:** 2026-05-27
**Campaign ID:** FTD-0211
**Funder / Context:** Wilsonian-reframe plan v2 Arc B P1 / P1 Cosmology Priority

---

## §1 — Objective & Process Protocol

This document pre-registers and lock-secures the design of the cosmological independent confirmation campaign for the **W5 Moore-shell dark matter weighting scheme (FTD-0211)**. 

Per CLAUDE.md anti-laundering rules and epistemic discipline, the W5 weighting scheme was identified as a post-hoc match in `docs/theory/10_eft_program/EXPLR_DM_BARYON_W5_WEIGHTING.md` and carries the `[SELECTION]` tag. To validate W5 as a genuine structural feature of FTD's lattice ontology, we must test it against **independent cosmological observables** that were not used to construct or calibrate it.

### Methodological Protocol:
1. This pre-registration document (including §§2–9) is committed and git-tagged before the verification script is executed.
2. The SHA256 of this file is recorded in the manifest `REF_PREREGISTER_MANIFEST.md` and the campaign is reserved in `LEDGER.md`.
3. The verification script `scripts/exploration/verify_w5_cosmology.py` is written and executed to compute the numerical predictions.
4. The final verdict (Outcome A, B, or C) is reported in the analysis result document `FOUND_DM_BARYON_W5_CONFIRMATION.md`.

---

## §2 — The Central Question

**Q-DM-W5:** Does the per-site weighting scheme W5 (cuboctahedron sites weight = $N_{\text{base}} = 4$, other shells weight = 1) predict both the primordial Helium-4 mass fraction $Y_p$ and the CMB acoustic-peak scale $\ell_1$ within a $1.5\%$ residual deviation threshold under Planck 2018 parameters, while the uniform voxel-counting scheme W1 is excluded by $> 5\sigma$ of experimental uncertainty?

---

## §3 — Definitions & Formalisms

*   **Definition D1 (W1 Scheme):** Uniform voxel-counting where every site in the 27-site Moore neighborhood carries weight 1. The baryonic fraction of matter is $\Omega_b / \Omega_m = 10/27 \approx 0.3704$.
*   **Definition D2 (W5 Scheme):** Per-site weighting where the 12 cuboctahedron sites carry weight $N_{\text{base}} = 4$, and the remaining 15 sites (center, octahedron, stella octangula) carry weight 1. The baryonic fraction of matter is:
    $$\frac{\Omega_b}{\Omega_m}\bigg|_{\text{W5}} = \frac{6 \cdot 1 + 4 \cdot 1}{(1 \cdot 1 + 12 \cdot 4 + 4 \cdot 1) + (6 \cdot 1 + 4 \cdot 1)} = \frac{10}{63} \approx 0.15873$$
*   **Definition D3 (Target Baryon density):** The physical baryon density $\Omega_b h^2$ computed from the matter density $\Omega_m = 1/3$ (from FTD's canonical $\Omega_{\Lambda} = 2/3$ selection) and $h = 0.674$ (Planck 2018 Hubble parameter):
    $$\Omega_b h^2 = \frac{\Omega_b}{\Omega_m} \cdot \Omega_m h^2 = \frac{\Omega_b}{\Omega_m} \cdot \frac{1}{3} \cdot (0.674)^2$$
*   **Definition D4 (primordial helium mass fraction $Y_p$):** The standard BBN prediction for the Helium-4 abundance as a function of the baryon-to-photon ratio $\eta_{10} = 273.9 \cdot \Omega_b h^2$:
    $$Y_p \approx 0.2467 + 0.009 (\eta_{10} - 6.0)$$
    against the observed primordial abundance $Y_p^{\text{obs}} = 0.245 \pm 0.003$ (Aver et al. 2015).
*   **Definition D5 (CMB acoustic peak position $\ell_1$):** The first acoustic peak in the angular power spectrum of the CMB, approximated by the acoustic angular scale:
    $$\ell_1 \approx \frac{\pi}{\theta_*} \approx 220 \cdot \left( \frac{\Omega_b h^2}{0.0224} \right)^{-0.1} \left( \frac{\Omega_m h^2}{0.142} \right)^{0.25}$$
    against the observed value $\ell_1^{\text{obs}} = 220 \pm 1$ (Planck 2018).

---

## §4 — Admissible Search Space

The search is strictly confined to the two weighting schemes:
1. **W1 (Uniform)**: baseline / control.
2. **W5 (Cuboctahedron-weighted)**: active hypothesis.

### Excluded moves (Banned as post-hoc tuning):
*   No adjusting the Hubble parameter $h = 0.674$ or matter density $\Omega_m = 1/3$.
*   No introducing any other arbitrary per-shell weightings outside the 9 natural weightings cataloged in `EXPLR_DM_BARYON_W5_WEIGHTING.md`.
*   No adjusting the physical BBN or CMB equations to force a match.

---

## §5 — Three Pre-Blessed Outcomes

*   **Outcome A (FOUND):** The W5 weighting predicts both $Y_p$ and $\ell_1$ within a $1.5\%$ deviation threshold of the observed values, while W1 deviates by $> 5\%$ (equivalent to $> 5\sigma$ exclusion). W5 is confirmed as a valid structural Selection.
*   **Outcome B (UNDERDETERMINED):** The W5 weighting predicts both observables within a $5.0\%$ deviation, but does not sharply satisfy the $1.5\%$ threshold, or W1 is not strongly excluded.
*   **Outcome C (CLOSED-NEGATIVE):** The W5 weighting deviates by $> 5\%$ on either observable. W5 is retired as a post-hoc coincidence.

---

## §6 — Falsifiers

*   **F-a (BBN mismatch):** The W5-predicted $Y_p$ deviates from $Y_p^{\text{obs}}$ by $> 1.5\%$.
*   **F-b (CMB mismatch):** The W5-predicted $\ell_1$ deviates from $\ell_1^{\text{obs}}$ by $> 1.5\%$.
*   **F-c (W1 overlap):** The uniform voxel-counting scheme W1 predicts $Y_p$ or $\ell_1$ within $1.5\%$, meaning W5 does not have unique confirmatory power.

---

## §7 — Banned Moves

*   **B-1 (Posterior selection):** Altering the $1.5\%$ deviation threshold after the script is run.
*   **B-2 (Calibration laundering):** Changing the PDG/Planck experimental values cited in `docs/reference/REF_EXTERNAL_CONSTANTS.md`.
*   **B-3 (AI attribution):** Adding AI co-author or generator trailers to any commit in this campaign.

---

## §8 — Verification Procedure

1. Read experimental parameters from `docs/reference/REF_EXTERNAL_CONSTANTS.md`.
2. Compute $\Omega_b$ and $\Omega_b h^2$ under W1 and W5.
3. Compute BBN $\eta_{10}$ and predicted $Y_p$.
4. Compute CMB acoustic peak $\ell_1$.
5. Evaluate residuals $| \text{predicted} - \text{observed} | / \text{observed}$.
6. Assign verdict and write the final result document.
