# FTD-0406 — Strong stress–energy contract v1

**Frozen outcome:** **CPU-SCOPED-CONTRACT**

**Scope/tag:** **[SELECTED IMPLEMENTATION CONTRACT — isolated flat collision-free CPU colour domain]**

**Lock:** `preregister-strong-stress-energy-contract-v1` at commit `2692405ff189659e106fb3eef6ceb391ec08e9c1`; preregistration SHA256 `10725372b020756d82f823c909384a45cc942f539c5c0a2e329311ee90faf2d7`.

## Verdict

The owner-authorized choices remove both FTD-0405 obstructions on the frozen CPU v1 domain.

1. **One selected Hamiltonian.** The existing colour-force profile now has the explicit pair energy

   \[
   U_{ij}(r)=-c_f\int_1^r g(s)\,ds,
   \]

   with no coloured pairs and `r=1` selected as the zero-energy reference. The force proposal, post-movement energy projection, EnergyAudit, energy ledger, local stress allocation and gravity source use this same production implementation. This is an owner-selected convention, not a theorem fixing the additive zero from the substrate.

2. **Exact scoped energy and momentum.** The existing force/movement phases still propose the positions. A deterministic post-movement projection scales only momenta relative to their mean, so total physical momentum is preserved while `K+U` returns to its tick-start value. On the frozen two-body arm:

   ```text
   legacy residual      = -0.07641037710505616
   selected residual    =  0 at printed precision
   projection lambda    =  1.374382480446247
   final separation     = 15.253499324949958
   ```

   The proposal positions are bit-identical to the legacy arm, individual motion is nonzero, and `lambda != 1`, so the result is not a frozen/no-motion or no-correction artifact. A separate three-particle zero-total-momentum arm closes at `-8.8817841970012523e-16`.

3. **Local selected stress-energy.** Each pair energy is deposited along the shortest periodic segment using midpoint sampling and periodic trilinear cloud-in-cell weights. The same samples carry the symmetric central-force Irving–Kirkwood spatial stress. Integrated local `T00` equals the pair Hamiltonian within `1e-12`; integer translation and endpoint exchange preserve the integrated energy and stress. This is a selected microscopic localization, not a uniquely derived continuum tensor.

4. **Strong energy gravitates using FTD's derived speed.** The CPU latency Poisson source consumes the selected interaction energy as gravitational mass density

   \[
   \rho_{g,\mathrm{strong}}=T^{00}_{\mathrm{strong}}/C_{\rm SPEED}^2.
   \]

   Since `C_SPEED^2=1/3`, the frozen static pair has

   ```text
   integrated strong T00       = 2.1284287974850993
   gravitational mass-equivalent = 6.3852863924552983
   ```

   The selected source changes the actual CPU latency potential relative to the identical source-off control. No implicit `c=1` conversion is used.

## What this answers physically

Within this selected engine extension, the strong force does not manufacture an independent mass substance. Separation stores interaction energy in the confining pair Hamiltonian. That energy is represented locally along the string, and gravity receives its mass-equivalent density through `E/C_SPEED^2`. This is the explicit bridge that was previously missing.

It is still not a first-principles electron or hadron mass derivation. The absolute zero and localization were owner-authorized choices, the existing force profile contains imposed strong-sector inputs, and FTD-0096 still blocks a native MeV unit. A stable excitation would also need a target-blind invariant energy across histories and lattice sizes before it could be called a native rest mass.

## NCEMC disposition

| Requirement | FTD-0406 v1 result |
|---|---|
| NCEMC-1 — one Hamiltonian | **CLOSED ON SCOPE by selection:** one pair energy is shared by the force contract, audit, ledger, projection, localization and source |
| NCEMC-2 — exact work/energy | **CLOSED ON SCOPE:** two- and three-body `K+U` residuals are within `1e-12` |
| NCEMC-3 — momentum/stress | **CLOSED ON SCOPE:** total particle momentum is preserved and a selected local central stress is installed |
| NCEMC-4 — same energy sources gravity | **CLOSED ON SCOPE:** the same local `T00` enters CPU latency as `T00/C_SPEED^2` |
| NCEMC-5 — target-blind invariant | **NOT EXECUTED:** requires a separate lock after domain extension |
| NCEMC-6 — calibration boundary | **UNCHANGED:** FTD-0096 still forbids a first-principles MeV scale |

The scoped closure does not erase FTD-0405. FTD-0405 remains the no-go for the unmodified direct-force tick and for any claim that the zero/localization were already forced. FTD-0406 records what follows only after the owner explicitly supplies those choices.

## Correctness and verification

- Exact recomputing verifier: A1–A8 and S1–S13, **21/21 PASS**.
- New native target: **35/35 PASS twice** with bit-identical observation lines.
- Legacy non-vacuity: contract-off FTD-0405 work residual remains finite and nonzero.
- Two-body selected residual: zero at printed precision; total momentum closes.
- Three-body selected residual: `-8.8817841970012523e-16`; total momentum closes.
- Local integration, translation and endpoint-exchange gates pass.
- Static strong-gravity source/control gate passes with `U/C_SPEED^2`.
- Topology-change and invalid mixed-force controls are surfaced explicitly.
- Targeted neighboring CTests: **10/10 PASS** (`strong_stress_energy_contract`, `ncemc_feasibility`, `confinement_test`, `asymptotic_freedom`, `force_diag_parity`, `genesis_energy_ledger`, `latency_field`, `causal_normalization`, `toggle_matrix`, `strict_validation`).
- Goldens: **7/7 PASS** with no accepted hash change.
- No full CTest, GPU strong-contract implementation/parity, WASM/web build, mass campaign, numerical near-miss search or substitution search ran.

The preregistration used the shorthand `energy_ledger`; the registered existing neighbor is `genesis_energy_ledger`. That intended ledger campaign was built and passed; no test definition or physics was changed to reconcile the name.

## Explicit remaining boundary

`strong_stress_energy` is default-off and CPU-scoped. When selected on a CUDA-backed RenderBridge, the bridge explicitly falls back to the CPU before the tick. The energy projection supports only an unchanged coloured-particle topology in the isolated flat sector. Collision, annihilation, creation/evaporation, locked-cluster dynamics, moving latency, mixed forces and a native GPU implementation remain open.

The local object is a selected localization of a direct pair interaction; it is not a propagating local Yang–Mills field action. Extending that claim would require a new lock and new evidence.

Therefore the next legitimate choices are:

1. extend this contract through topology-changing events and mixed forces;
2. port the selected contract to GPU with independent parity gates;
3. only after those domains close, run a target-blind particlehood/invariant test before discussing rest mass.

## Reproduction record

- Implementation commit: `808bf27264ed2d0503313d556a31cfb2b60aadd1`
- Platform: WSL2 Ubuntu 22.04, Linux `6.6.87.2-microsoft-standard-WSL2`, CPU-forced Ryzen 9 9950X3D; RTX 5090 driver `610.47` present but not used for v1 physics
- Header SHA256: `32fa1fef11fee23138e8da41e5b950810ebcdd0e3ec058fb8b8ea9f06b16d5a5`
- Source SHA256: `022614da328b632866650a7efa3665a03183b2c0ab17144f7844d0beb1204760`
- Test SHA256: `37b27312000b4c2ebed2966837b19fbcf1da8e422c9b2f41a8015681779ac57a`
- Verifier SHA256: `98750acf07c709fecad7b899678953ee355c0b454e23459ba3509c92f5c74d3e`
- Binary SHA256: `14030f3ac2c34e349837cfca1d32854ee5d82b316927b884a1b1816630c7fb14`
- Raw record: `engine/results/strong_stress_energy_contract_2026-07-21/verification.txt`
