# Scale Ownership and Recovery Boundary

**Status:** [SYNTHESIS] of implementation ownership; cross-scale physical recovery is [OPEN].
**Authority:** The [active v3 constitution](../theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md) defines the primitive tuple and recovery contract. This page inventories software; it does not add a postulate or certify emergence.

## Current dashboard owners

| Scale | State and tick owner | Present relationship to v3 `Phi` |
| --- | --- | --- |
| 0, lattice | C++ `RenderBridge`, exposed through the active WASM/worker bridge | Finite simulation/experimental substrate, not a proof that the selected v3 law recovers every effective sector |
| 1, particles | Separate native `ParticleEngine` instance in the [WASM adapter](../../engine/web/js/bridge/native-particle-engine.js) | Effective particle dynamics; no state-complete pushforward from Scale 0 |
| 2-3, atoms and molecules | JS `AtomEngine` hosted by [WasmBridge](../../engine/web/js/bridge/wasm-bridge.js) | Effective atom/molecule dynamics; the native AtomEngine binding is disabled pending a Planck-to-Bohr unit conversion, and neither backend is a v3 recovery map |
| 4, planetary | Independent [PlanetaryMockBridge](../../engine/web/js/scales/scale4/controller.js) | Effective N-body presentation, not a Scale 0 readout |
| 5, cosmic | Independent [CosmicMockBridge](../../engine/web/js/scales/scale5/controller.js) | Effective cosmological presentation, not a Scale 0 readout |

The shared `ScaleBridge` API makes modes navigable; it does not imply shared state, a common time step, or a derivation. Mock/effective engines should remain labeled as such in code and UI. Moving their modules into one directory would not change this boundary.

## Recovery acceptance contract

For a claimed v3-to-effective transition, identify all of the following before labeling it [EMERGENT]:

1. One state-complete microscopic record `X`, one tick law `Phi`, and the finite prepared-history domain on which the claim is made.
2. An explicit readout `B_{R,T}` from finite histories on region `R` over `T` ticks to the proposed effective variables, including units, resolution, boundary treatment, and any ensemble definition.
3. A quantitative evolution comparison between `B_{R,T}(Phi^n X)` and the effective model's update, with a stated domain, error/stability bound, and causal-support check. Agreement of constants alone is insufficient.
4. Accounting for transferred work, recoil, heat/radiation, and boundary flow where an energy or force claim is made. Diagnostic ledgers alone are not physical conservation proofs.
5. A reproducible test fixture and failure criterion that are independent of the target experimental value. Label imported equations, calibration, and selected initial conditions separately.

Until these are supplied, the dashboard modes are distinct effective demonstrations. The [Scale 0 fluid scenario contract](../../engine/web/js/scales/scale0/fluid-scenario-contract.js) is explicitly prospective, and the [record panel](../../engine/web/js/scales/scale0/ui/controls/record-panel-model.js) names missing particle/observable contracts. Neither is a completed cross-scale bridge.

## Ownership rule for new work

Keep effective-model changes in their present scale owners. Put a proposed recovery readout beside the microscopic owner and give it a separate, versioned contract/test; do not silently feed an effective engine back into `Phi` or call independent reinitialization a coarse-graining. A downstream scale may consume a readout only after its units, time mapping, validity domain, and error contract are explicit. The theory [ledger](../theory/07_assessment/core_ledgers/LEDGER.md) remains the claim-status authority.
