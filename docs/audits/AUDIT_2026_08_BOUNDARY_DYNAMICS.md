# Audit: Scale-0 Boundary Dynamics

**Date:** 2026-08-30
**Scope:** CPU RenderBridge, native CUDA, WASM/WebSocket configuration, Scale-0 scenario defaults, toolbar, viewport orientation, and the global ordinal-clock readout.
**Epistemic status:** implementation audit and imposed finite-box contract. This document does not promote any boundary rule to a substrate theorem.

## Outcome

The three public boundary choices now control the same local transported state on CPU and CUDA: manifested sites plus the observable, dual, strong, and weak field registers.

| Mode | Field rule | Manifested-site rule | Qualified interpretation |
|---|---|---|---|
| Dispersal | Before stencil reads and after local writers, every non-field face record is reset to void. Each transported field pair is reconstructed only from its corresponding strictly interior record by `J_face = J_interior - h*wave_vel_interior/C_WAVE`, `wave_vel_face = wave_vel_interior`. No strictly interior cell is graded. | The first attempted hop from the interior into any face retires the complete manifested record; it never occupies the face or enters at the opposite face. | Computed window into an otherwise uncomputed void, with an imposed first-order outward trace rather than an evolved wall. It guarantees no storage-level opposite-face re-entry, but is not an exact all-angle transparent-boundary theorem. |
| Reflective | Every face is a discrete Neumann ghost shell: the exterior cell copies the first interior cell, giving the implemented zero-normal-difference mirror. | The crossed normal velocity component reverses; tangential components are unchanged and the site remains in the box. | Sealed mirror for the audited local transported state. This is not a perfect-conductor claim and does not imply exact total-energy conservation with arbitrary dissipative/stochastic terms enabled. |
| Periodic | All three pairs of opposite faces identify through the lattice neighbour tables. No face is converted to an open exhaust by the orientation choice. | A crossing through any of the six faces wraps to the opposite face. | Full-domain periodic finite box. Dashboard orientation remains X=lateral, Y=vertical, Z=forward/aft, but it does not alter the boundary operator. |

## Defects found and closed

1. The former Dispersal pass was a one-cell multiplier applied after a fully periodic stencil read, and an intermediate replacement damped an `L`-dependent interior sponge. Both violated the intended computed-window contract. A subsequent exact-zero shell closed wraparound but acted as a Dirichlet wall, reversing 77% of the fixed packet's launch momentum. Dispersal now excises non-field face records and reconstructs a one-way characteristic field trace from strict-interior values; no strictly interior record is graded.
2. The public flux-boundary selector and manifested-particle boundary were split. The selector changed fields while a separate legacy Boolean changed particle crossings. `flux_boundary` is now authoritative for both. The Boolean remains only as an old native-profile compatibility override.
3. An intermediate directional-periodic implementation coupled orientation to physics by exhausting the four faces outside the selected axis. That coupling is retired: Periodic now covers all six faces, and the historical `periodic_axis` profile field carries orientation/provenance metadata only.
4. Boundary helpers omitted the strong and weak substrate registers. CPU and CUDA passes now mirror or reconstruct those field registers with the observable and dual fields, while face excision covers the manifested, identity, motion, potential, and diagnostic record families.
5. WASM and native WebSocket profiles had no periodic-axis control or readback. Both transports now set, validate, acknowledge, and roll back `fluxPeriodicAxis` transactionally.

## Scenario compatibility

Every scenario receives the same six-face operator for its selected boundary mode. The orientation defaults to Z (forward/aft) unless a scenario records another view orientation; this metadata does not change transport, stencil topology, or field preparation.

## Orientation and clock visualization

The Scale-0 viewport now renders six arrows outside the cubic boundary. X is red, Y is green, and Z is blue. The selected orientation pair is highlighted for every boundary mode; XYZ highlights all pairs. The Orientation control remains enabled for Dispersal, Reflective, and Periodic because it describes the simulation frame rather than selecting boundary faces.

The external clock ring and toolbar readout display the settled global tick. One tick means one completed deterministic engine transaction through the ordered phase groups. The ten visual sectors are pedagogical phase groups; the hand is not a live sub-phase probe. Playback speed changes wall-clock pacing only. It does not change the ordinal meaning of a tick, and it is distinct from recovered local proper time.

The Scene > View menu exposes independent, presentation-only `Arrows` and `Clock` toggles. They preserve their selected visibility across lattice-size decoration rebuilds; hiding the 3D clock face does not hide or freeze the authoritative toolbar tick readout.

## Verification evidence

- `boundary_scenario_physics`: exact six-face mirror/outflow operators across the audited record families, strictly interior Dispersal invariance, all-axis periodic seam coupling, finite causal cone, reflective momentum reversal, periodic Hamiltonian control, and a fixed 90-tick Dispersal regression with reverse momentum and residual field norm below 1%.
- `boundary_movement`: first-contact Dispersal retirement at all six faces, normal-only reflection, and periodic wrapping both along and across the recorded orientation.
- `gpu_movement_transaction`: CPU/CUDA parity for all three complete-boundary modes, including first-contact Dispersal retirement, orientation-independent periodic movement, local field shells, and observable/dual/strong/weak outflow traces.
- `boundary_modes_golden`: explicit characterization pins for Reflective, Dispersal, the separate absorbing sponge, and the legacy mirror override. The Dispersal pin was intentionally re-baselined to the one-way outflow contract; the optional `absorbing_boundary` toggle retains the imposed interior sponge as a distinct operator.
- Browser toolbar audit: boundary/orientation separation, axis arrows, orientation highlighting in every mode, and global-clock readout.

## Limits and non-claims

- Manifested and non-field face records are deleted, and the field trace is reconstructed without reading an opposite face. The fixed normal packet has no macroscopic reverb under the recorded 1% regression gate, but a finite first-order local closure can still create small numerical backscatter for other wavelengths and incidence angles; this is not a theorem of exactly zero wave reflection.
- The Gauss, Coulomb, and latency Poisson solvers and gauge-link relaxation have their own operator topology. This audit does not claim that their constraint potentials satisfy open or Neumann Green-function boundary conditions. Runs that depend sensitively on those nonlocal solvers still need a dedicated boundary-qualified solver campaign.
- The physical Scale-0 domain is the cubic lattice. Alternative decorative boundary shapes do not redefine the native stencil domain.
- The historical API/profile name `PeriodicAxis` is retained for wire compatibility. X/Y/Z/All now records or displays orientation only; Z is the direct-sandbox default.
