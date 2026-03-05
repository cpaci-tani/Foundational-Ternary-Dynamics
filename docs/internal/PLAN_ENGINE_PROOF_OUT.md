> [!IMPORTANT]
> **STATUS: COMPLETE** — All 10 phases pass with zero failures (2026-03-03).
> 125+ individual checks across 40 campaign test files. See `engine/SPEC_ENGINE.md` §21-22 for results.

# FTD Engine Proof-Out: Master Plan

## Context

The FTD engine (v2.8, ~5700 lines C++, 77 tests, 363x GPU acceleration) implements 6 axiomatically-derived rules on a ternary lattice. The ontic derivation chain (D=3 + varpi -> all physics) produces ~30 genuine derivations including alpha=1/137.036 (1.26 ppm). However, a gap exists between theoretical claims (~180 items across 93 theory documents) and what the engine actually validates. This plan closes those gaps systematically.

**Current engine state:** Maxwell equations verified (4/4), Coulomb 1/r^2 confirmed (-2.067 exponent), charge conservation exact, hydrogen-like orbits observed, 100% test pass rate. Spin and color fields exist in voxel struct but are NOT used in dynamics.

**Goal:** Transform FTD from "internally consistent simulation with nice constants" into "physics engine that demonstrably reproduces known phenomena from first principles."

---

## Phase 1: Measurement Infrastructure (Foundation for Everything)

**Why first:** Every subsequent phase requires ensemble statistics, correlation functions, and spectral analysis. The engine currently has per-tick diagnostics but no multi-run statistical framework.

### 1.1 Ensemble Runner
- **File:** `engine/include/ftd/ensemble.h`, `engine/src/ensemble.cpp`
- Add `EnsembleRunner` class wrapping `RenderBridge`
- Support N independent runs with different RNG seeds
- Collect per-run observables into statistical moments (mean, variance, skewness, kurtosis)
- Output: mean +/- stderr for all `EnergyAudit` fields across ensemble

### 1.2 Correlation Function Infrastructure
- **File:** `engine/include/ftd/correlations.h`
- Spatial correlation `C(r) = <J(x) . J(x+r)>` averaged over all x
- Temporal autocorrelation `C(tau) = <J(x,t) . J(x,t+tau)>`
- Two-point charge correlation `G(r) = <s(x) s(x+r)>`
- Structure factor `S(k) = FFT of C(r)` (momentum space)

### 1.3 Spectral Analysis Tools
- **File:** `engine/include/ftd/spectral.h`
- Time-series FFT for dispersion relation omega(k)
- Power spectral density of flux field
- Mode decomposition: transverse vs longitudinal vs spin-wave

### 1.4 Particle Tracking & History
- **File:** `engine/include/ftd/tracker.h`
- Use existing `particle_id` to track particles across ticks
- Record trajectory `(x,y,z,t)` for each tracked particle
- Compute: mean free path, lifetime distribution, binding time

### 1.5 Observable Framework
- Generic `Observable` base class with `measure(RenderBridge&)` and `reduce()` methods
- Built-in observables: energy density, charge density, flux magnitude, Gauss violation
- User-defined observables via lambda or subclass

### Tests
- `test_ensemble.cpp`: 5-run ensemble produces correct statistics
- `test_correlations.cpp`: Known field configuration gives expected C(r)
- `test_spectral.cpp`: Monochromatic wave gives single peak in FFT
- `campaign_statistical_convergence.cpp`: Moments converge as N_runs -> infinity

---

## Phase 2: Continuum Limit Validation

**Why second:** This is the foundational proof that the lattice recovers known physics. Everything else builds on this.

### 2.1 Maxwell Convergence Study
- Run identical wave propagation at L = 16, 32, 64, 128, 256 (GPU for large)
- Measure: wave speed, dispersion, energy conservation, Gauss violation
- Compute convergence rate: error ~ h^p where h = 1/L
- **Target:** p >= 2 (second-order convergence to Maxwell)
- Compare lattice dispersion omega(k) vs continuum omega = c|k|

### 2.2 Coulomb Convergence
- Single charge at center, measure phi(r) at increasing L
- Compare with theoretical 1/(4*pi*r) Coulomb potential
- Extract convergence rate for force law exponent toward -2.0
- **Target:** Exponent within 0.01 of -2.0 at L=256

### 2.3 Schrodinger Limit
- Initialize Gaussian wavepacket in flux field (no manifested particles)
- Track spreading: sigma(t) should grow as sqrt(t) (free particle)
- Add potential well (manifested charge): measure bound state energy levels
- Compare with E_n = -alpha^2 * K_B / (2n^2) hydrogen spectrum
- **Target:** Ground state energy within 5% of Bohr model

### 2.4 Lorentz Invariance Recovery
- Boosted wave packets at various velocities
- Measure energy-momentum relation E^2 = p^2*c^2 + m^2*c^4
- Quantify anisotropy: ratio of wave speeds along (100), (110), (111) directions
- **Target:** Isotropy ratio > 0.99 at L >= 64

### 2.5 Scaling Report
- Produce publication-quality plots of all convergence rates
- Identify the universality class of the lattice field theory
- Document any anomalous scaling or phase transitions

### Tests
- `campaign_continuum_maxwell.cpp`: Wave speed convergence at 4 lattice sizes
- `campaign_continuum_coulomb.cpp`: Force exponent convergence
- `campaign_continuum_schrodinger.cpp`: Wavepacket spreading rate
- `campaign_continuum_lorentz.cpp`: Isotropy measurements
- `campaign_dispersion_relation.cpp`: Full omega(k) curve vs theory

---

## Phase 3: Bell Test & Quantum Mechanics

**Why third:** This is the make-or-break test. If ensemble averaging over lattice states can produce S > 2, FTD has a genuine quantum mechanics story. If not, the framework is classical.

### 3.1 EPR Pair Creation
- Create correlated pair via annihilation-in-reverse (high flux -> pair production)
- Track entanglement via `pair_id` — both particles share origin
- Verify anti-correlation: pair always has opposite state (+1, -1)

### 3.2 Bell Measurement Protocol
- Define "measurement basis" as flux gradient direction at detector location
- Implement two separated "detectors" (manifested structures at distance D)
- Measure correlation E(a,b) = <s_A(a) * s_B(b)> where a,b are detector orientations
- Compute CHSH parameter S = |E(a,b) - E(a,b') + E(a',b) + E(a',b')|

### 3.3 Substrate-Level Bell Test
- Run N >> 1000 pair productions with random detector orientations
- Measure S from raw lattice correlations
- **Expected result:** S <= 2 (lattice is local deterministic)
- Verify this matches theory prediction

### 3.4 Aggregate/Ensemble Bell Test (Critical Experiment)
- Hypothesis: S > 2 emerges from ensemble averaging over many configurations
- Protocol: For each detector angle pair (a,b), run M >> 100 independent lattice instances
- Compute ensemble-averaged correlation <E(a,b)>_ensemble
- Test whether ensemble S exceeds 2
- If YES: characterize the mechanism (complexification? sLoop? shared substrate?)
- If NO: document the failure honestly and reassess theory

### 3.5 sLoop Implementation
- Add `is_sloop` flag activation when observer structure is embedded in measured system
- Implement self-referential coupling: observer's flux participates in measured field
- Compare Bell parameter with and without sLoop coupling
- Test the three-level hierarchy: S_substrate=2, S_independent=sqrt(2), S_sloop=2*sqrt(2)

### 3.6 Born Rule Verification
- Generate many (N > 10000) manifestation events in flux field with known |J|^2 profile
- Measure: spatial distribution of manifestation positions
- Compare with Born rule P(x) = |psi(x)|^2 / ||psi||^2
- Chi-squared test for goodness of fit
- **Target:** p-value > 0.05 (cannot reject Born rule)

### Tests
- `test_epr_pair.cpp`: Pair creation produces anti-correlated states
- `test_bell_measurement.cpp`: Detector protocol returns valid correlations
- `campaign_bell_substrate.cpp`: 10000-pair substrate S measurement
- `campaign_bell_ensemble.cpp`: Ensemble-averaged S (the critical test)
- `campaign_born_rule.cpp`: Chi-squared Born rule test (10000 events)
- `campaign_sloop_bell.cpp`: Three-level hierarchy verification

---

## Phase 4: Emergent Mass Spectrum

**Why fourth:** If the lattice produces bound structures whose mass ratios match the ontic derivations, that's powerful evidence the mathematics is physical, not numerological.

### 4.1 Structure Detection
- Scan lattice for stable bound configurations (>1000 ticks persistence)
- Classify by: particle count, geometry, charge, spin, color
- Measure effective mass via: (a) total flux energy, (b) gravitational response, (c) inertial response to force

### 4.2 Triad Mass Measurement
- Create triads (3 same-sign particles in equilateral triangle, r ~ sqrt(2))
- Measure total energy vs 3x single-particle energy -> binding energy
- Compare binding energy with K_B * PHI prediction
- Compute mass ratio m_triad / m_single -> should approach m_p / m_e ~ 1836

### 4.3 Lepton Mass Hierarchy
- If different bound structures exist, measure mass ratios
- Search for states with mass ratios near 207 (muon) and 3477 (tau)
- Test whether these ratios emerge from dynamics or must be imposed
- **Critical question:** Does the lattice NATURALLY produce these ratios, or only when parameters are tuned?

### 4.4 Meson-Like Structures
- Search for particle-antiparticle bound states (quark-antiquark analog)
- Measure: binding energy, orbital period, decay lifetime
- Compare with pion mass prediction from ontic.h

### 4.5 Large-Scale Structure Formation
- Run very long simulations (100K+ ticks) on large lattices (128^3+ GPU)
- Seed with random flux perturbations above K_GENESIS
- Observe: do hierarchical structures form? (triads -> nuclei -> atoms?)
- Measure: size distribution, lifetime distribution, interaction cross-sections

### Tests
- `test_structure_detection.cpp`: Known triad is correctly identified
- `campaign_triad_mass.cpp`: Triad binding energy and mass measurement
- `campaign_mass_hierarchy.cpp`: Search for mass ratios near predictions
- `campaign_meson_search.cpp`: Particle-antiparticle bound states
- `campaign_structure_formation.cpp`: Long-running hierarchical assembly (GPU)

---

## Phase 5: Color Dynamics & SU(3)

**Why fifth:** The engine already assigns color at genesis (flux axis dominance -> Z_3) but doesn't use it in forces. Activating color-dependent dynamics tests whether SU(3) truly emerges from geometry.

### 5.1 Color-Dependent Coupling
- Modify `phase_forces()`: add color-color interaction term
- Color force: attractive between different colors, repulsive between same
- Force form: F_color = g_s * color_factor(c_i, c_j) * gradient
- Color factor: +1 (different colors, attractive), -1/2 (same color, repulsive)
- Coupling g_s from ontic.h: strong coupling at confinement scale

### 5.2 Confinement Test
- Place two different-color particles at separation r
- Measure force vs r
- **Target:** Linear potential V(r) ~ sigma * r at large r (string tension)
- Contrast with Coulomb: V(r) ~ 1/r at short r
- Crossover distance should emerge from lattice dynamics

### 5.3 Color Neutrality
- Test that color-neutral composites (3 particles, one of each color) have zero color force
- "Baryon" = {red, green, blue} triad -> net color = white -> no color force at distance
- Measure: force between two color-neutral triads vs two colored particles
- **Target:** Color-neutral force << colored force at same distance

### 5.4 Asymptotic Freedom
- Measure effective color coupling g_eff(r) at various distances
- At short r (high energy): g_eff should decrease (asymptotic freedom)
- At long r (low energy): g_eff should increase (confinement)
- Compare running with QCD beta function: dg/d(ln mu) = -b_0 * g^3 / (16*pi^2)
- b_0 = 11 - 2*N_f/3 = 7 (matching ontic b_3!)

### 5.5 Gluon-Like Flux Modes
- Identify flux configurations that carry color but not charge (s=0, color != 0)
- Test: do these modes propagate? At what speed? With what dispersion?
- Measure: gluon-gluon interaction (self-coupling of non-Abelian gauge field)
- **Critical:** Do 8 independent color-flux modes exist? (SU(3) has 8 generators)

### 5.6 Toggle Integration
- New toggle: `color_forces` (default: false, to preserve backward compatibility)
- All existing tests must pass with `color_forces = false`
- New campaign tests verify color dynamics when enabled

### Tests
- `test_color_force.cpp`: Same-color repulsion, different-color attraction
- `test_color_neutral.cpp`: White composite has zero color force
- `campaign_confinement.cpp`: Linear potential at large r
- `campaign_asymptotic_freedom.cpp`: Running coupling g_eff(r)
- `campaign_gluon_modes.cpp`: Color-carrying flux propagation
- `campaign_baryon_formation.cpp`: Three quarks -> color-neutral bound state

---

## Phase 6: Weak Sector & SU(2)

**Why sixth:** Weak interactions require transmutation (state flipping), parity violation (chirality from dual substrate), and the Higgs mechanism. This builds on Phase 5 (color) and the dual-substrate infrastructure.

### 6.1 Transmutation Dynamics
- Implement stress-threshold state flip: +1 <-> -1 when field stress > WEAK_THRESHOLD
- Stress = |div J| + |curl J| + |grad rho|
- WEAK_THRESHOLD derived from ontic: ~ K_B / sin^2(theta_W) ~ 2.2 * K_B
- Toggle: `weak_transmutation` (default: false)

### 6.2 Parity Violation from Dual Substrate
- In dual-substrate mode: J_L and J_R evolve independently
- Weak force couples only to J_L (left-chiral component)
- Implement: transmutation probability depends on |J_L|^2, not |J|^2
- Test: do left-handed particles transmute more readily than right-handed?
- **Target:** Maximal parity violation (only left-handed weak interaction)

### 6.3 W/Z Boson Analogs
- Search for massive flux modes that mediate transmutation
- W boson: charged flux packet that carries state change
- Z boson: neutral flux packet from interference of L/R modes
- Measure: effective mass from dispersion relation omega^2 = k^2 + m_W^2
- Compare with ontic prediction: m_W = v_Higgs * g_W / 2

### 6.4 Higgs Mechanism
- Implement scalar condensate field phi(v) on lattice
- Mexican hat potential: V(phi) = lambda(phi^2 - v^2)^2
- Symmetry breaking: phi acquires VEV = v_Higgs = 246 GeV (from ontic)
- Mass generation: particle masses proportional to Yukawa coupling * v_Higgs
- Test: do particle effective masses change when Higgs condensate is present?

### 6.5 Electroweak Unification
- At high energy (short distance): EM and weak forces should merge
- Measure: coupling ratio alpha_EM / alpha_W as function of energy scale
- **Target:** Unification at ~ 100 GeV scale (v_Higgs)
- Weinberg angle: sin^2(theta_W) = N_c / N_eff = 3/13 ~ 0.231 (ontic prediction)

### Tests
- `test_transmutation.cpp`: State flip occurs above stress threshold
- `test_parity_violation.cpp`: Left-handed particles transmute preferentially
- `campaign_weak_decay.cpp`: Transmutation rate vs stress level
- `campaign_w_boson.cpp`: Massive flux mode carrying charge change
- `campaign_higgs_condensate.cpp`: Symmetry breaking and mass generation
- `campaign_electroweak_unification.cpp`: Coupling merging at high energy

---

## Phase 7: Gravitational Sector & GR

**Why seventh:** The engine has a simple density-gradient gravity force. This phase upgrades it to demonstrate convergence toward general relativity.

### 7.1 Metric Emergence
- Define effective metric: g_mu_nu(x) = eta_mu_nu + h_mu_nu(x)
- h_mu_nu proportional to flux energy density: h_00 ~ 2*G_N*rho/c^2
- Measure: local speed of light c_eff(x) = c * sqrt(1 - 2*G_N*rho/c^2)
- Test: light rays (flux waves) deflect around massive objects
- **Target:** Deflection angle matches GR prediction theta = 4GM/(rc^2)

### 7.2 Geodesic Motion
- Place test particle in density gradient
- Compare trajectory with geodesic equation: d^2x/dt^2 + Gamma * dx/dt * dx/dt = 0
- Compute Christoffel symbols from g_mu_nu
- **Target:** Particle trajectory matches geodesic to within 5% for weak fields

### 7.3 Gravitational Waves
- Create oscillating mass distribution (binary system)
- Measure: transverse flux oscillations at large distance
- Verify: quadrupole radiation pattern
- Compare wave speed with c_wave
- **Target:** Gravitational wave speed = electromagnetic wave speed (both = 1/sqrt(3))

### 7.4 Schwarzschild Limit
- Place large mass (many locked particles) at center
- Measure radial density profile
- Compare with Schwarzschild: g_00 = 1 - 2GM/r
- Identify "horizon" where g_00 -> 0 (if lattice large enough)
- **Target:** Qualitative agreement with Schwarzschild geometry

### 7.5 Einstein Equation Verification
- Compute discrete Ricci tensor R_mu_nu from g_mu_nu
- Compute stress-energy T_mu_nu from flux field
- Test: R_mu_nu - 1/2 g_mu_nu R = 8*pi*G * T_mu_nu
- **Target:** Relation holds to within discretization error ~ h^2

### 7.6 Gravitational Hierarchy
- Measure ratio alpha_G / alpha_EM in simulation
- Compare with ontic prediction: alpha_G = 2*pi*(16/3)^2*(N_eff+3/7)^2*alpha^20
- This ratio should be ~ 10^-39 (gravitational hierarchy)
- **Target:** Correct order of magnitude (may require very large lattice)

### Tests
- `test_metric_emergence.cpp`: Light bending near massive object
- `test_geodesic.cpp`: Free fall matches geodesic prediction
- `campaign_gravitational_wave.cpp`: Quadrupole radiation from binary
- `campaign_schwarzschild.cpp`: Radial density profile comparison
- `campaign_einstein_equation.cpp`: Discrete R_mu_nu vs T_mu_nu
- `campaign_hierarchy_ratio.cpp`: alpha_G / alpha_EM measurement

---

## Phase 8: Flavor Physics & Particle Zoo

**Why eighth:** With gauge structure (U(1), SU(2), SU(3)) and gravity in place, test whether the full particle spectrum emerges.

### 8.1 Quark Composites
- With color forces active, create {u,u,d} triad (proton) and {u,d,d} triad (neutron)
- u quark: state=+1, charge=+2/3, color in {R,G,B}
- d quark: state=+1, charge=-1/3, color in {R,G,B}
- Measure: proton mass, neutron mass, binding energy
- **Target:** m_p/m_e ~ 1836.15 (ontic prediction 0.017% accuracy)

### 8.2 Decay Processes
- Neutron beta decay: d -> u + e + nu_bar (via weak transmutation)
- Measure: decay lifetime
- Compare with tau_n ~ 880 s (in lattice time units)
- Pion decay: pi -> mu + nu_mu
- **Target:** Correct decay channels and approximate lifetime ratios

### 8.3 CKM Mixing Verification
- Create quark flavor transitions via weak interaction
- Measure: transition amplitudes |V_ud|, |V_us|, |V_ub|, etc.
- Compare with ontic CKM matrix predictions (1-6% accuracy)
- **Target:** Correct mixing hierarchy |V_ud| >> |V_us| >> |V_ub|

### 8.4 Neutrino Oscillations
- Implement neutrino as state=0, charge=0 flux packet
- Three flavors with masses from ontic seesaw mechanism
- Propagate over distance L, measure flavor change probability
- **Target:** P(nu_e -> nu_mu) oscillation matches PMNS predictions

### 8.5 CP Violation
- Measure asymmetry between particle and antiparticle decay rates
- Jarlskog invariant J = Im(V_us V_cb V_ub* V_cs*) ~ 3.9e-5
- **Target:** Correct sign and order of magnitude of CP asymmetry

### Tests
- `test_proton_neutron.cpp`: Color-neutral composites form and persist
- `campaign_nucleon_mass.cpp`: Proton/neutron mass ratio measurement
- `campaign_beta_decay.cpp`: Neutron transmutation dynamics
- `campaign_ckm_mixing.cpp`: Flavor transition amplitudes
- `campaign_neutrino_oscillation.cpp`: Oscillation probability vs distance
- `campaign_cp_violation.cpp`: Matter-antimatter decay asymmetry

---

## Phase 9: Cosmological Validation

**Why ninth:** With all particle physics in place, test cosmological predictions at the largest accessible scales.

### 9.1 Expanding Lattice
- Implement scale factor a(t) that stretches lattice spacing over time
- Friedmann equation: (da/dt)^2 / a^2 = 8*pi*G*rho / 3
- Hubble parameter H(t) = da/dt / a
- **Target:** Correct Friedmann dynamics for radiation-dominated and matter-dominated eras

### 9.2 Inflation Dynamics
- Initialize sub-threshold flux field (0 < |J| < K_GENESIS) as inflaton
- Slow-roll dynamics: d^2phi/dt^2 + 3H*dphi/dt + V'(phi) = 0
- Measure: number of e-folds N_e (target: 56.3 from ontic n_eff^2/N_c)
- Spectral index: n_s = 1 - 2/N_e (target: 0.9645)
- Tensor-to-scalar: r = 4*alpha*(3/4) (target: 0.0219)

### 9.3 Baryogenesis
- Start with equal matter/antimatter from pair production
- CP violation from Phase 8 -> slight asymmetry
- Measure: baryon-to-photon ratio eta = (n_B - n_Bbar) / n_gamma
- **Target:** eta ~ 10^-10 (correct order of magnitude)

### 9.4 Dark Matter Candidates
- Sub-threshold flux (0 < |J| < K_B): gravitationally active but non-manifested
- Measure: total sub-threshold flux energy vs manifested energy
- Ratio should be ~ 5:1 (dark matter:baryonic matter)
- Test: does sub-threshold flux form halos around manifested structures?

### 9.5 Cosmological Constant
- Vacuum energy density from flux zero-point fluctuations
- Measure: <|J|^2> in true vacuum (no particles)
- Compare with Lambda = 8*pi*G*rho_vac
- FTD prediction: Lambda from discrete lattice regularization (finite, not infinite)

### Tests
- `campaign_friedmann.cpp`: Scale factor evolution matches Friedmann
- `campaign_inflation.cpp`: Slow-roll dynamics and e-fold count
- `campaign_baryogenesis.cpp`: Matter-antimatter asymmetry emergence
- `campaign_dark_matter.cpp`: Sub-threshold flux gravitational dynamics
- `campaign_vacuum_energy.cpp`: Zero-point energy measurement

---

## Phase 10: Novel Predictions & Falsifiability

**Why last:** Only after validating known physics can we credibly make predictions.

### 10.1 Planck-Scale Lorentz Violation Signature
- Characterize exact form of dispersion relation: omega(k) = c*k * (1 + epsilon * (k/k_P)^n + ...)
- Determine n and epsilon from lattice simulations
- Predict: photon time-of-flight difference for GRB observations
- **Target:** Specific numerical prediction for Fermi/GLAST observations

### 10.2 Fourth Generation Exclusion
- The master quadratic gives N_gen = floor(x_-) = floor(3.024) = 3
- Predict: NO fourth generation fermions with standard gauge couplings
- Quantify: maximum mass for hypothetical 4th generation (exclusion bound)
- **Falsification:** Discovery of sequential 4th generation at any mass

### 10.3 Alpha Precision Test
- Current: 1/alpha = 137.0360 (1.26 ppm from CODATA)
- Compute higher-order corrections from lattice dynamics
- Predict: next digit of alpha from FTD (beyond current 6-digit match)
- **Falsification:** Future precision measurements disagree beyond stated uncertainty

### 10.4 Proton Decay
- If baryon number violated at Planck scale (GUT-like mechanism)
- Predict: proton lifetime tau_p from FTD integers
- Compare with Super-Kamiokande bounds (tau_p > 10^34 years)
- **Target:** Specific lifetime prediction (or proof of stability)

### 10.5 Gravitational Wave Dispersion
- Discrete lattice predicts: gravitational waves have (tiny) dispersion
- Predict: frequency-dependent speed delta_c/c ~ (f/f_P)^2
- Compare with LIGO/Virgo observations of binary mergers
- **Falsification:** Wrong sign of dispersion (superluminal high-frequency)

### 10.6 Neutrino Mass Prediction
- FTD seesaw: specific values for m_1, m_2, m_3
- Predict: lightest neutrino mass (currently unmeasured)
- Compare with KATRIN, cosmological bounds
- **Falsification:** Measured mass disagrees with prediction

### Tests
- `campaign_lorentz_violation.cpp`: Dispersion relation high-k coefficients
- `campaign_alpha_precision.cpp`: Higher-order corrections to alpha
- `campaign_proton_stability.cpp`: Baryon number violation search
- `campaign_gw_dispersion.cpp`: Gravitational wave speed vs frequency

---

## Implementation Priority & Dependencies

```
Phase 1 (Measurement Infrastructure)
  |
  +---> Phase 2 (Continuum Limit) ------+
  |                                       |
  +---> Phase 3 (Bell Test) ------+      |
                                   |      |
                                   v      v
                            Phase 4 (Mass Spectrum)
                                   |
                    +--------------+---------------+
                    |                              |
              Phase 5 (SU(3) Color)          Phase 7 (GR/Gravity)
                    |                              |
              Phase 6 (SU(2) Weak)                 |
                    |                              |
                    +-----------+------------------+
                                |
                         Phase 8 (Particle Zoo)
                                |
                         Phase 9 (Cosmology)
                                |
                        Phase 10 (Predictions)
```

### Estimated Effort

| Phase | New Code (est.) | New Tests | GPU Work | Depends On |
|-------|----------------|-----------|----------|------------|
| 1. Measurement | ~800 lines | 4 tests | Minimal | Nothing |
| 2. Continuum | ~400 lines | 5 campaigns | Yes (scaling) | Phase 1 |
| 3. Bell | ~1200 lines | 6 campaigns | Yes (ensemble) | Phase 1 |
| 4. Mass Spectrum | ~600 lines | 5 campaigns | Yes (long runs) | Phases 1,2 |
| 5. SU(3) Color | ~1000 lines | 6 campaigns | Yes (kernels) | Phase 4 |
| 6. SU(2) Weak | ~1200 lines | 6 campaigns | Yes (kernels) | Phase 5 |
| 7. GR/Gravity | ~800 lines | 6 campaigns | Yes (large L) | Phase 2 |
| 8. Particle Zoo | ~1500 lines | 6 campaigns | Yes | Phases 5,6,7 |
| 9. Cosmology | ~1000 lines | 5 campaigns | Yes (GPU-only) | Phase 8 |
| 10. Predictions | ~600 lines | 4 campaigns | Yes | All |

**Total estimated:** ~9,100 new lines of engine code, ~53 new test/campaign files

### Verification Strategy

Each phase ends with a **Verification Gate** before proceeding:

| Phase | Gate Criterion |
|-------|---------------|
| 1 | Ensemble statistics converge; correlation functions match known configurations |
| 2 | Convergence rate >= h^2; Maxwell recovery demonstrated at 4 lattice sizes |
| 3 | Born rule chi-squared p > 0.05; Bell S measured and documented (pass or fail) |
| 4 | At least one emergent mass ratio matches ontic prediction within 10% |
| 5 | Linear confinement observed; color-neutral composites verified |
| 6 | Parity violation demonstrated; transmutation follows predicted threshold |
| 7 | Light bending within 10% of GR; gravitational waves detected |
| 8 | Proton/neutron mass ratio within 5% of 1836; decay channels correct |
| 9 | Friedmann evolution qualitatively correct; baryogenesis order of magnitude |
| 10 | At least one pre-observation prediction with stated uncertainty |

### Critical Decision Points

- **After Phase 3:** If Bell ensemble test gives S <= 2 with high confidence, FTD is classical. The project pivots to "best classical lattice simulation" rather than "quantum mechanics from first principles." This is honest and important.
- **After Phase 4:** If NO emergent mass ratios match predictions, the ontic constants may be numerological coincidences rather than physics. Reassess the derivation chain.
- **After Phase 5:** If SU(3) doesn't emerge from color dynamics, the "3 flux components = 3 colors" claim is falsified. Document and move on.

---

## Files to Create/Modify

### New Headers (engine/include/ftd/)
- `ensemble.h` — EnsembleRunner class
- `correlations.h` — Spatial/temporal correlation functions
- `spectral.h` — FFT analysis tools
- `tracker.h` — Particle trajectory tracking
- `observable.h` — Generic observable framework
- `color_force.h` — SU(3) color-dependent coupling
- `weak_force.h` — SU(2) transmutation dynamics
- `metric.h` — Effective metric and Christoffel symbols
- `cosmology.h` — Scale factor and Friedmann evolution

### New Source Files (engine/src/)
- `ensemble.cpp`, `correlations.cpp`, `spectral.cpp`, `tracker.cpp`
- `color_force.cpp`, `weak_force.cpp`, `metric.cpp`, `cosmology.cpp`

### Modified Files
- `engine/include/ftd/render_bridge.h` — New measurement APIs
- `engine/src/render_bridge.cpp` — Color force, weak force, metric hooks in tick cycle
- `engine/include/ftd/term_toggles.h` — New toggles: `color_forces`, `weak_transmutation`, `metric_evolution`
- `engine/include/ftd/voxel.h` — Activate spin/color in dynamics; add flavor field
- `engine/cuda/kernels_forces.cu` — GPU color force, weak force kernels
- `engine/cuda/gpu_engine.cu` — New phase routing for color/weak/metric

### New Test Files (~53)
- See individual phase sections above for complete list
