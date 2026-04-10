# Lattice Physics Reference

**Living document.** Last updated: April 9, 2026

Physics phenomena to recover from empirical observation, paired with their lattice-based definitions and current status.

---

## Mechanics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Mass | Objects have inertia proportional to their "stuff" | Per-tick budget cost of maintaining a non-void voxel (K_B per tick) | Recovered |
| Inertia | Resistance to acceleration scales with mass | Cost of reconfiguring the flux wake; more mass = deeper well = more to rearrange | Recovered |
| Momentum | p = mv, conserved in collisions | p = K_B * v / sqrt(1 - v^2); conserved by lattice translation symmetry (Noether) | Recovered |
| Kinetic energy | Energy of motion, (1/2)mv^2 at low v | Budget consumed by spatial traversal; fraction of G*^2 per tick devoted to velocity | Recovered |
| Newton's laws | F=ma, action/reaction, inertia | Emerge from Born-Infeld Lagrangian in weak-field limit | Recovered |

## Gravity

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Gravitational attraction | Masses accelerate toward each other | Each mass creates a flux density halo; the other mass sinks along the gradient (dense knot in lighter medium) | Recovered |
| Inverse square law | F = GMm/r^2 | Gradient of overlapping 1/r density halos naturally gives 1/r^2 force | Recovered |
| G_N value | G = 6.67e-11 m^3/kg/s^2 | G_N = 1/(b_3 + N_c)^2 = 0.01 on lattice; physical value via renormalization | Recovered |
| Gravitational time dilation | Clocks tick slower near mass | Reduced availability f = 1 - L^2; fewer ticks processed per universal tick | Recovered |
| Equivalence principle | Inertial mass = gravitational mass | Same flux wake causes both: push it = inertia, sit in gradient with it = gravity | Recovered |
| Weightlessness in freefall | Orbiting astronauts float despite 88% surface gravity | No resistance to gradient = no conflict = no felt weight | Recovered |
| Schwarzschild metric | ds^2 = f dt^2 - dr^2/f - r^2 dOmega^2 | Exact from Born-Infeld proper time: dtau/dt = sqrt(f - v^2/f) | Recovered |
| Einstein field equations | R_uv - (1/2)g_uv R = 8piG T_uv | Noether stress-energy + Lovelock uniqueness theorem; 8piG from 2pi * N_base | Recovered |
| Frame dragging (Kerr) | Rotating masses drag spacetime | Latency shadow of rotating flux configuration | Partial |
| Gravitational waves | Ripples in spacetime from accelerating masses | Propagating perturbations in the latency field | Partial |

## Special Relativity

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Speed of light limit | Nothing exceeds c | c = 1/sqrt(3), CFL condition on cubic lattice; max information propagation speed | Recovered |
| Lorentz contraction | Moving objects appear shorter | Flux wake compressed along direction of motion | Recovered |
| Time dilation (kinematic) | Moving clocks tick slower | Budget consumed by spatial traversal reduces budget available for proper time | Recovered |
| E = mc^2 | Rest energy equals mass times c^2 | K_B (rest budget cost) = m_e * c^2; budget statement | Recovered |
| Relativistic momentum | p diverges as v -> c | Born-Infeld nonlinearity: p = K_B * v / sqrt(1 - v^2); wake backlog at c | Recovered |

## Electromagnetism

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Electric charge | Two kinds, attract/repel | Ternary state polarity: s = +1 or s = -1 | Recovered |
| Coulomb's law | F = kqq/r^2 | Flux gradients from charged voxels; Gauss constraint enforcement | Recovered |
| Maxwell's equations | Unified EM field theory | Continuum limit of lattice wave equation + Gauss constraint | Recovered |
| Fine structure constant | alpha = 1/137.036 | Master quadratic root x_+ from Z^3 + CM elliptic curve E_i | Recovered |
| U(1) gauge symmetry | Phase invariance of EM | Gauss constraint div(J) = rho on the lattice | Recovered |
| Photon (massless boson) | EM radiation | Flux wave propagating at c through lossless vacuum | Recovered |

## Quantum Mechanics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Wave-particle duality | Particles show interference patterns | Flux field (wave) + manifestation events (particle) = two-layer ontology | Recovered |
| Born rule | Probability = |psi|^2 | Multiple derivations: threshold crossing, max entropy, Gleason-style | Recovered |
| Pauli exclusion | No two identical fermions in same state | pi_1(SO(3)) = Z_2 from frame bundle topology; ternary constraint | Recovered |
| Uncertainty principle | Delta_x Delta_p >= hbar/2 | Lattice spacing provides minimum Delta_x; Brillouin zone bounds Delta_p | Recovered |
| Measurement / collapse | Wavefunction -> eigenstate on observation | ReLU crystallization: Softplus -> ReLU as beta -> infinity; Type III -> Type I | Recovered |
| Bell inequality violation | S = 2sqrt(2) > 2 | Gauss constraint -> 2D transverse flux; statistical aggregate behavior | Recovered |
| Spin-1/2 | Fermion rotation properties | pi_1(SO(3)) = Z_2 from frame bundle topology | Recovered |

## Particle Physics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Three generations | 3 families of quarks and leptons | 3 = floor(x_-) from master quadratic | Recovered |
| Three colors | SU(3) color charge, N_c = 3 | Master quadratic root x_- via RG flow + topological quantization | Recovered |
| SM gauge group | U(1) x SU(2) x SU(3) | Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula) | Recovered |
| Electron mass | m_e = 0.511 MeV | m_e = M_P * sqrt(2pi) * (16/3) * alpha^11 (0.27%) | Recovered |
| Proton mass | m_p = 938.3 MeV | m_p/m_e = N_eff/alpha + N_base*N_eff + N_c = 1836.47 (174 ppm) | Recovered |
| Higgs mass | m_H = 125.1 GeV | m_H = (N_eff/alpha^2) * m_e = 124.8 GeV (0.24%) | Recovered |
| Confinement | Quarks never free | Area-law Wilson loops at x_- ; sigma = 0.209 | Recovered |
| Electron g-2 | a_e = 0.00115965... | alpha/(2pi) to 5-loop = 2.55 ppb agreement | Recovered |
| Lamb shift | 1057.8 MHz | 1055.4 MHz from lattice calculation (0.23%) | Recovered |

## Stellar Physics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Jeans instability | Gas clouds above critical mass collapse | Budget tipping point: gravitational drain > thermal redistribution | Recovered |
| Hydrostatic equilibrium | Stars maintain stable structure | Fusion budget income = gravitational budget expense | Recovered |
| Main sequence | Stars burn hydrogen for most of their life | Balanced budget era; self-regulating fuel economy | Recovered |
| Stellar nucleosynthesis | Stars fuse light elements into heavy | High-energy ternary state rearrangements releasing stored flux energy | Conjecture |
| Mass-luminosity relation | L ~ M^3.5 | Deeper latency basin requires more fusion to counterbalance | Recovered |
| Red giant phase | Stars expand and cool after H exhaustion | Shell burning: budget shift from core to envelope | Recovered |
| Supernova | Explosive stellar death | Budget catastrophe at iron wall; core collapse + envelope ejection | Recovered |
| White dwarf | Dense stellar remnant below 1.4 M_sun | Electron degeneracy: lattice out of low-energy configurations | Recovered |
| Chandrasekhar limit | 1.4 solar mass maximum for WD | Born-Infeld saturation when electron v -> c; degeneracy floor cracks | Selection |
| Neutron star | Ultra-dense remnant 1.4-3 M_sun | Neutron degeneracy floor | Selection |

## Black Hole Physics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Event horizon | Surface of no return | f = 0 surface; inward flux current exceeds max swim speed c. FTD horizon at GM/c^2 (half Schwarzschild) | Prediction |
| Schwarzschild radius | r_s = 2GM/c^2 | Radius where L^2 = 1, availability hits zero | Recovered |
| No singularity | GR predicts infinite density (a failure) | Lattice UV cutoff: max density = one state per voxel; max momentum = pi (Brillouin zone) | Recovered |
| Hawking radiation | BHs emit thermal radiation at T ~ 1/M | Vacuum pair-splitting at horizon boundary; boundary accounting errors in Gauss constraint | Recovered |
| Bekenstein-Hawking entropy | S = A / (4 l_P^2) | 1/4 entropy per horizon site from Gauss constraint + coordination number reduction | Recovered |
| Information paradox | Does BH evaporation destroy information? | No. Lattice evolution is deterministic and invertible; unitarity built in. Page curve follows. | Recovered |
| Trans-Planckian problem | Hawking derivation requires unknown UV physics | Solved: lattice momenta bounded by Brillouin zone |k| <= pi | Recovered |
| BH interior structure | Unknown in GR | Frozen information crystal: ternary states preserved at max density, zero ticks | Open |
| BH evaporation endpoint | Unknown in GR | Final pop: last frozen voxels unfreeze in Planck-energy burst | Conjecture |
| BH shadow size | EHT: ~42 uas (M87*), ~52 uas (Sgr A*) | Two-mechanism model: ~88% of GR shadow. FTD predicts 34.9 uas for M87* | Prediction |
| Accretion disk | Infalling matter forms hot disk | Orbiting flux spiraling inward; Bondi accretion with Eddington cap | Recovered |
| Kerr black hole | Rotating BH with ergosphere | Modified latency field from angular momentum; frame dragging | Partial |
| Penrose process | Energy extraction from rotating BH | Budget extraction from ergosphere region where f < 0 but horizon not yet reached | Open |

## Cosmology

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Dark matter | 27% of universe, gravitationally active, EM invisible | Lossless far-field flux halos (~15-23 voxels); free scaffolding | Selection |
| Dark energy | Accelerating expansion | Net energy leak from coupling injection exceeding near-field damping | Selection |
| Cosmic inflation | n_s = 0.965, r < 0.06 | Sub-threshold flux dynamics; n_s = 0.966, r = 0.022 | Recovered |
| Baryogenesis | Matter-antimatter asymmetry eta ~ 10^-10 | CP violation + Sakharov conditions from lattice dynamics | Recovered |
| Large-scale structure | Filaments, voids, clusters | Dark halos create potential wells; baryons fall in; voids are "balloons" rising | Selection |
| Cosmological constant | Lambda ~ 10^-122 in Planck units | rho_Lambda = m_e^4 * alpha^16 * G*^2 | Selection |
| Holographic principle | Entropy bounded by area, not volume | Interior determined by boundary data on lattice (deterministic + local + finite c) | Recovered |

## Thermodynamics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Temperature | Average kinetic energy of constituents | Average velocity budget consumption per manifested voxel | Recovered |
| Entropy | Disorder, irreversibility | Microstate counting over ternary configurations | Recovered |
| Second law | Entropy increases | Lattice dynamics are reversible but coarse-graining produces effective irreversibility | Selection |
| Boltzmann distribution | Probability ~ exp(-E/kT) | Emerges from microstate counting over lattice configurations | Recovered |

---

## The 2^3 / 3^3 Distinction: Matter vs Life (April 2026)

Two minimal cubic lattices encode two fundamentally different modes of being:

| Structure | Definition | Properties | Models |
|---|---|---|---|
| F = {0,1}^3 | 2x2x2 cube, 8 vertices | No center, all boundary, extensional | Matter as occupancy: being there |
| O = {-1,0,+1}^3 | 3x3x3 cube, 27 sites | Unique center, 26-shell, interior/exterior | Life as centered integration: being there for itself |

F sits inside O as the 8 corner vertices. But F has no center. The step from F to O is the step from extension to interiority.

**Three layers:**

| Layer | Definition | Criterion |
|---|---|---|
| Matter | F-structure | Occupancy, exclusion, extension |
| Organic matter | F configured into stable boundary-bearing assemblies | Membrane closure, local energy handling, recursive coupling |
| Life | O emerging when such assemblies sustain center-shell integration | Persistent lambda_x^{t+1} = F(lambda_x^t, Sigma_x^t) maintaining centered organization |

**Formal mapping to FTD:**
- F = the state field (s in {-1,0,+1} at each site) — discrete occupancy
- O = the tick cycle (center voxel reads 26 neighbors, integrates, writes) — centered integration
- The tick IS the O-operation: M_x = Phi(lambda_x, Sigma_x)
- Life = patterns that persistently maintain their own O-structure against perturbation

**Key claim:** Life is not a new substance. It is a new topology of relation — matter organized so that occupancy becomes interiority.

---

## Two-Mechanism Gravity (April 2026 Finding)

GR has one gravitational mechanism: spacetime curvature. FTD has two:

| Mechanism | Source | What it produces |
|---|---|---|
| Flux gradient | Coupling term g_c * s * div(J), |J| ~ 1/r | Newtonian 1/r^2 force, light bending (refraction), Keplerian orbits |
| BI metric | Born-Infeld core, f = 1 - L^2, L ~ 1/r | Time dilation, speed limit, relativistic corrections |

The two mechanisms together recover all weak-field observations. The ~12% strong-field deficit in shadow size is a testable lattice prediction.

| Observable | Mechanism | FTD value | GR value | Data | Match? |
|---|---|---|---|---|---|
| Newtonian force | Flux gradient | 1/r^2 | 1/r^2 | 1/r^2 | Exact |
| Mercury precession | Sommerfeld (SR momentum) | 42.94"/c | 42.94"/c | 42.98"/c | 99.9% |
| Solar light bending | Flux refraction | 1.75" | 1.75" | 1.75" | Exact |
| GPS time dilation | BI core f = 1 - L^2 | Matches | Matches | Matches | Yes |
| Grav wave speed | Lattice wave eq | c | c | c | Exact |
| Photon sphere | Both mechanisms | 1.769 GM/c^2 | 3.000 GM/c^2 | ? | Prediction |
| BH shadow (b_c) | Both mechanisms | 4.569 GM/c^2 | 5.196 GM/c^2 | ? | 88% of GR |
| M87* shadow | Both mechanisms | 34.9 uas | 39.7 uas | ~42 uas | -17% (within EHT error) |
| Horizon radius | BI metric f = 0 | 1.0 GM/c^2 | 2.0 GM/c^2 | ? | Prediction: half GR |

Key finding: the f = 1 - L^2 metric with L ~ 1/r falls off as 1/r^2 (not Schwarzschild's 1/r). The 1/r behavior of GR comes from gravitational self-energy, which the FTD latency field does not self-source. The flux mechanism provides the missing 1/r contribution for weak-field effects but cannot fully compensate at strong fields.

Scripts: `scripts/exploration/explore_ftd_vs_schwarzschild.py`, `explore_deser_bootstrap.py`, `explore_latency_field.py`

---

## Status Key

| Status | Meaning |
|---|---|
| **Recovered** | Phenomenon emerges from lattice dynamics with quantitative agreement where applicable |
| **Selection** | Argued from consistency / structure, not uniquely forced by axioms |
| **Conjecture** | Proposed mechanism, not yet derived or simulated |
| **Partial** | Framework exists but incomplete or not fully quantitative |
| **Open** | Not yet addressed; research opportunity |
| **Prediction** | FTD makes a specific, testable prediction that differs from GR |
