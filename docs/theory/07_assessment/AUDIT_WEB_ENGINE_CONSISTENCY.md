# FTD Web Engine Scale and Telemetry Consistency Audit

**Version:** 1.0 (2026-06-11)  
**Status:** [CLOSED -- RESOLVED]  
**Category:** Assessment & Epistemic Audit (Category 7)  
**ID:** FTD-0250  

---

## 1. Executive Summary

This audit establishes mathematical and programmatic consistency across the FTD web-dashboard dashboard engines (Scales 0 through 5). Prior versions suffered from duplicate literal values of constants, inconsistent gravity telemetry display bindings, missing Scale 4 (Planetary) telemetry, and static/stale dark matter fraction estimations. 

All identified issues have been resolved. The web engine constants are anchored strictly on `engine/web/js/constants.js`. Scale 4 (Planetary N-body) now computes real-time Kinetic Energy, Potential Energy, Total Energy, Energy Drift, Total Momentum, Virial Ratio, and System Radius, mapping them directly to the diagnostics panel. A programmatic verification suite (`engine/web/tests/verify_web_consistency.js`) has been deployed to enforce no duplicate literal constant leaks and verify telemetry channel bindings.

---

## 2. Discrepancy Resolution Ledger

| ID | Component / File | Description of Discrepancy | Resolution Method | Status |
|----|------------------|----------------------------|-------------------|--------|
| **D-01** | `ws-bridge.js` | Hardcoded `0.511` default parameter for `createEntangledPair`. | Imported and substituted `K_B` from `./constants.js`. | **RESOLVED** |
| **D-02** | `cosmic-physics.js` | Hardcoded `0.577` in stellar radiation pressure loop representing speed of light $c = 1/\sqrt{3}$. | Imported and substituted `C_SPEED` from `../constants.js`. | **RESOLVED** |
| **D-03** | `template.js` | Static gravity display string `<span>0.01</span>` in Scale 4 controls panel. | Substituted with `<span id="planetary-ctrl-gravity">0.01</span>` to allow dynamic updates. | **RESOLVED** |
| **D-04** | `scale4/controller.js` | Gravity value display did not update dynamically; Scale 4 N-body loop did not dispatch telemetry. | Bound `planetary-ctrl-gravity` to `bridge.G` (formatted by physical/decorative mode); injected `telemetryHub.collectScale4` call in rAF callback loop. | **RESOLVED** |
| **D-05** | `telemetry-hub.js` | Scale 4 telemetry buffers completely missing; Scale 5 dark matter fraction was static. | Initialized pl buffers and `_plInitialEnergy`; implemented `collectScale4(bridge)` mathematical routines; updated `collectScale5` to calculate DM fraction dynamically from body types. | **RESOLVED** |
| **D-06** | `component.js` | Scale 4 telemetry channels missing in `CHANNELS` grid mapping. | Added 8 standard telemetry channels mapping pl buffers to the uPlot metrics grid. | **RESOLVED** |

---

## 3. Telemetry Formulations for Scale 4 (Planetary N-Body)

Scale 4 N-body diagnostics are now computed at the frame tick level using the following physical formulations:

1. **Kinetic Energy ($KE$)**:
   $$KE = \frac{1}{2} \sum_{i} m_i v_i^2 = \frac{1}{2} \sum_{i} m_i (v_{x,i}^2 + v_{y,i}^2 + v_{z,i}^2)$$

2. **Potential Energy ($PE$)**:
   $$PE = -G \sum_{i < j} \frac{m_i m_j}{\sqrt{r_{ij}^2 + \epsilon}}$$
   Where $G$ is the active gravitational constant (`bridge.G`), and $\epsilon = 10^{-6}$ is the softening parameter to avoid division by zero.

3. **Total Energy ($E_{\text{total}}$)**:
   $$E_{\text{total}} = KE + PE$$

4. **Energy Drift ($\text{Drift}\%$)**:
   $$\text{Drift}\% = \frac{E_{\text{total}} - E_{\text{initial}}}{\|E_{\text{initial}}\|} \times 100$$
   Where $E_{\text{initial}}$ is anchored on the first non-zero total energy computed after scenario initialization.

5. **Total Momentum ($\|\vec{P}\|$)**:
   $$\|\vec{P}\| = \sqrt{\left(\sum_{i} m_i v_{x,i}\right)^2 + \left(\sum_{i} m_i v_{y,i}\right)^2 + \left(\sum_{i} m_i v_{z,i}\right)^2}$$

6. **System Radius ($R_{\text{sys}}$)**:
   $$R_{\text{sys}} = \max_{i} \|\vec{r}_i - \vec{r}_{\text{CoM}}\||$$
   Where $\vec{r}_{\text{CoM}}$ is the Center of Mass:
   $$\vec{r}_{\text{CoM}} = \frac{\sum_{i} m_i \vec{r}_i}{\sum_{i} m_i}$$

7. **Virial Ratio ($V$)**:
   $$V = \begin{cases} \frac{2 \cdot KE}{\|PE\|} & \text{if } PE \neq 0 \\ 0 & \text{otherwise} \end{cases}$$

---

## 4. Scale 5 Dynamic Dark Matter Fraction

The Scale 5 (Cosmic) telemetry now dynamically assesses the dark matter fraction instead of relying on a static value from diagnostics:
$$\text{DM}\% = \frac{\text{Counts}[3]}{\text{Total Bodies}} \times 100$$
Where index `3` corresponds to `DARK_MATTER` bodies inside the cosmic body registry.

---

## 5. Verification and Compliance Status

Programmatic verification is enforced via `engine/web/tests/verify_web_consistency.js`. This script performs:
1. **Constant Leaks Check**: Scans all Javascript modules recursively (excluding comments and string literals) to verify that no duplicate literal representations of fine structure ($137.036$), mass anchors ($0.511$), or universal constants ($2.95867$) exist.
2. **Telemetry Binding Check**: Programmatically instantiates `TelemetryHub` and matches its properties against the buffer keys defined in `telemetry-grid/component.js` to ensure zero runtime binding failures.

The verification script completed with exit code `0`, confirming complete system alignment.
