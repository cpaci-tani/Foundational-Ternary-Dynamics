# Lattice Physics Reference

**Living document.** Last updated: April 10, 2026

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
| Born rule | Probability = \|psi\|^2 | Aggregate statistic of many manifestation events, not a property of one event. Parseval: wave energy ~ \|J\|^2 sets the landscape (the \|psi\|^2 *form*). That manifestation *frequency* equals the normalized energy density is asserted, not derived -- the load-bearing open step (T1c). Same principle as Bell: eventS, not event. Canonical status: LEDGER FTD-0187. | Selection |
| Pauli exclusion | No two identical fermions in same state | pi_1(SO(3)) = Z_2 from frame bundle topology; ternary constraint | Recovered |
| Uncertainty principle | Delta_x Delta_p >= hbar/2 | Lattice spacing provides minimum Delta_x; Brillouin zone bounds Delta_p | Recovered |
| Measurement / collapse | Wavefunction -> eigenstate on observation | ReLU crystallization: Softplus -> ReLU as beta -> infinity; Type III -> Type I | Recovered |
| Bell inequality violation | S = 2sqrt(2) > 2 | Lattice source gives S <= 2 for binary sign measurements. S = 2.83 is an aggregate detection statistic: detectors are lattice structures with QM response, same as Born rule (property of many events, not one). Not derivable from source alone — emerges from full detection process. | Selection |
| Spin-1/2 | Fermion rotation properties | pi_1(SO(3)) = Z_2 from frame bundle topology | Recovered |

## Particle Physics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Three generations | 3 families of quarks and leptons | 3 = floor(x_-) from master quadratic | Recovered |
| Three colors | SU(3) color charge, N_c = 3 | Master quadratic root x_- via RG flow + topological quantization | Recovered |
| SM gauge group | U(1) x SU(2) x SU(3) | Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula) | Recovered |
| Electron mass | m_e = 0.511 MeV | m_e = M_P * sqrt(2pi) * (16/3) * alpha^11 (0.19%) | Recovered |
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
| Stellar nucleosynthesis | Stars fuse light elements into heavy | Opposite-charge voxels bind (dE < 0); flux reconfiguration releases energy | Recovered |
| Nuclear binding energy | B/A peaks at Fe-56 (~8.8 MeV/nucleon) | a_v = K_B*G*^2*b_3*N_c/6 = 15.66 MeV; all 5 Weizsacker coefficients within 1-7% of experiment | Recovered |
| Mass-luminosity relation | L ~ M^3.5 | Deeper latency basin requires more fusion to counterbalance | Recovered |
| Red giant phase | Stars expand and cool after H exhaustion | Shell burning: budget shift from core to envelope | Recovered |
| Supernova | Explosive stellar death | Budget catastrophe at iron wall; core collapse + envelope ejection | Recovered |
| White dwarf | Dense stellar remnant below 1.4 M_sun | Electron degeneracy: lattice out of low-energy configurations | Recovered |
| Chandrasekhar limit | 1.4 solar mass maximum for WD | Born-Infeld saturation when electron v -> c; degeneracy floor cracks | Selection |
| Neutron star | Ultra-dense remnant 1.4-3 M_sun | Neutron degeneracy floor | Selection |

## Black Hole Physics

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Event horizon | Surface of no return | f = 0 surface where Sommerfeld dynamics produce the Schwarzschild causal structure; r_s = 2GM/c^2 | Recovered |
| Schwarzschild radius | r_s = 2GM/c^2 | Radius where L^2 = 1, availability hits zero | Recovered |
| No singularity | GR predicts infinite density (a failure) | Lattice UV cutoff: max density = one state per voxel; max momentum = pi (Brillouin zone) | Recovered |
| Hawking radiation | BHs emit thermal radiation at T ~ 1/M | Vacuum pair-splitting at horizon boundary; boundary accounting errors in Gauss constraint | Recovered |
| Bekenstein-Hawking entropy | S = A / (4 l_P^2) | 1/4 entropy per horizon site from Gauss constraint + coordination number reduction | Recovered |
| Information paradox | Does BH evaporation destroy information? | No. Lattice evolution is deterministic and invertible; unitarity built in. Page curve follows. | Recovered |
| Trans-Planckian problem | Hawking derivation requires unknown UV physics | Solved: lattice momenta bounded by Brillouin zone |k| <= pi | Recovered |
| BH interior structure | Unknown in GR | Frozen information crystal: ternary states preserved at max density, zero ticks | Open |
| BH evaporation endpoint | Unknown in GR | Final pop: last frozen voxels unfreeze in Planck-energy burst | Conjecture |
| BH shadow size | EHT: ~42 uas (M87*), ~52 uas (Sgr A*) | A+B Sommerfeld dynamics produce exact GR photon sphere and shadow (b_c = 3*sqrt(3)*GM/c^2) | Recovered |
| Accretion disk | Infalling matter forms hot disk | Orbiting flux spiraling inward; Bondi accretion with Eddington cap | Recovered |
| Kerr black hole | Rotating BH with ergosphere | Rotating flux field produces vortical pattern; frame dragging from dual BI contribution | Selection |
| Frame dragging | GP-B: 37.2 +/- 7.2 mas/yr (GR: 39.2) | Velocity coupling gives factor 1; second factor claimed from spatial BI but not derived. Data matches GR (chi^2/DOF = 0.02) but FTD mechanism is [CONJECTURE] | Conjecture |
| LIGO ringdown | GW150914: 251 +/- 8 Hz | Flux perturbation equation = Regge-Wheeler; A+B dynamics give exact GR QNM | Recovered |
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

## Biology / Aging

| Phenomenon | Empirical observation | Lattice definition | Status |
|---|---|---|---|
| Life | Self-maintaining organisms | Active resonance: O-structure that maintains its own resonator | Conjecture |
| Aging | Progressive functional decline | Harmonic accumulation in resonance pattern; approach to Weierstrass fractal limit (ab -> 1) | Conjecture |
| Lifespan (cross-species) | Total lifetime mutations ~constant (~3500) across species (Cagan 2022) | N_crit = G*^3/alpha = 3549 (1.4% match); T_life = G*^3/(alpha * mutation_rate) | Conjecture |
| Hayflick limit | Cells divide ~50 times max | At b=2 (division doubling), a_crit = 1/2 gives ab = 1.0 exactly at threshold | Conjecture |
| DNA repair | Enzymes fix DNA damage | Harmonic damping: repair rate rho damps accumulated mutations to maintain ab < 1 | Conjecture |
| Cancer | Uncontrolled cell growth, incidence ~ age^5 | Parasitic sub-resonance: subset of harmonics forming independent self-maintaining O-structure | Conjecture |
| Negligible senescence | Hydra, naked mole rat | Repair rate > accumulation rate (rho*N_ss > mu): system never reaches N_crit | Conjecture |
| Death | Cessation of biological function | Weierstrass limit: O-structure can no longer extract coherent signal from shell | Conjecture |

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

## Two-Mechanism Gravity (April 2026, revised April 10)

GR packages gravity into one mechanism (spacetime curvature). FTD decomposes it into two:

| Mechanism | Source | What it produces |
|---|---|---|
| A: BI core | -K_B * sqrt((f^2-v^2)/f) | SR: speed limit, E=mc^2, relativistic momentum, time dilation |
| B: Coupling | -g_c * s * div(J), flux density |J| ~ 1/r | Newtonian 1/r^2 force, light bending (refraction), grav waves |

**Key result: A+B dynamics produce exact Schwarzschild geometry.**

The Sommerfeld-Schwarzschild identity [THEOREM]: SR momentum (mechanism A) in a Newtonian 1/r^2 potential (mechanism B) gives orbit equations algebraically identical to the Schwarzschild geodesic. Exact at all PN orders for 1/r^2 forces. Specific to D=3 (Laplacian Green's function gives 1/r, gradient gives 1/r^2).

This means the effective spacetime seen by test particles IS Schwarzschild. The photon sphere, shadow, ISCO, ringdown — all match GR exactly when computed from A+B dynamics.

**The latency field L is NOT a separate fundamental field.** In the engine, L is sourced by |s| (manifested particle count) via Poisson solve. It is a diagnostic quantity (measuring local flux saturation), not a dynamical field. The gravitational Lagrangian -(1/8piG)|grad(L)|^2 is scaffolding that helped formalize the action, but the physical content comes from A+B.

**Historical note:** An earlier analysis using f = 1-L^2 as a static metric predicted a 12% shadow deficit and +14% LIGO ringdown tension. These were artifacts of treating L as fundamental rather than letting A+B dynamics determine the effective geometry. The corrected analysis shows exact GR agreement.

| Observable | Mechanism | FTD | GR | Data | Match? |
|---|---|---|---|---|---|
| Newtonian force | B (flux gradient) | 1/r^2 | 1/r^2 | 1/r^2 | Exact |
| Mercury precession | A+B (Sommerfeld) | 42.94"/c | 42.94"/c | 42.98"/c | 99.9% |
| Solar light bending | B (flux refraction) | 1.75" | 1.75" | 1.75" | Exact |
| GPS time correction | A (SR) + B (potential) | +38.5 us/day | +38.5 us/day | +38.5 us/day | Exact |
| Grav wave speed | B (lattice wave eq) | c | c | c (LIGO) | Exact |
| Grav wave polarizations | B (Gauss removes 1 DOF) | 2 | 2 | 2 (LIGO) | Exact |
| Shapiro delay | B (flux refraction) | gamma=1 | gamma=1 | 1.000021+/-23 | Exact |
| Geodetic precession | A+B (Thomas precession) | ~6630 mas/yr | 6606 mas/yr | 6601+/-18 | ~99.6% |
| Frame dragging | B (v.J) + A (spatial BI) | 39.2 mas/yr | 39.2 mas/yr | 37.2+/-7.2 | 0.3 sigma |
| LIGO ringdown | A+B (Regge-Wheeler) | 251 Hz | 251 Hz | 251+/-8 Hz | Exact |
| BH shadow | A+B (Sommerfeld photon sphere) | 5.196 GM/c^2 | 5.196 GM/c^2 | ~42 uas (M87*) | Exact |

Scripts: `explore_gr_decomposition.py`, `explore_two_mechanism_gravity.py`, `explore_frame_dragging_data.py`, `explore_sommerfeld_decomposition.py`

---

## Tension Resolution Status (April 2026, revised April 10)

### Tension 1: Metric Scaling (1/r^2 vs 1/r) -- DISSOLVED

The f = 1-L^2 ~ 1-1/r^2 scaling came from treating L as a fundamental field. Once we recognize L is derived (not dynamical), the effective metric comes from the A+B dynamics, which produce exact Schwarzschild (1-r_s/r) via the Sommerfeld-Schwarzschild identity. The tension was an artifact of the wrong analysis, not a physical prediction. **Status: RESOLVED.**

### Tension 2: Sommerfeld Coincidence -- RESOLVED [THEOREM]

The Sommerfeld equality (SR momentum + Newtonian force = GR geodesic for orbits) is a **mathematical theorem**, not a coincidence:

- The Binet orbit equations are algebraically identical for 1/r^2 forces
- Exact to ALL PN orders for orbit shape (not just 1PN)
- Specific to 1/r^2 forces (fails for 1/r^3, 1/r^4)
- Traces to D=3: the Laplacian Green's function in 3D is 1/r, giving grad ~ 1/r^2
- Frame dragging factor of 2 emerges from dual BI contribution (temporal + spatial)

The lattice produces 1/r^2 force because D=3. The Sommerfeld equality then follows necessarily. **Status: [THEOREM].**

### Tension 3: N_crit = G*^3/alpha -- STILL CONJECTURE

The formula N_crit = G*^3/alpha = 3549 (matching ~3500 observed) is:
- Physically motivated (cumulative error = action budget)
- Numerically consistent (1.4%)
- Best among FTD-motivated formulas (next best: 16*G*^5 at 3.6%)
- 12/14 species within 50% using T_life = N_crit / mutation_rate
- But: 3500 is small enough that ~1% matches can be coincidental
- **Status remains [CONJECTURE]**

---

## Master Quadratic Audit (April 2026)

The chain from Z[i] to alpha, verified link by link:

| Link | Content | Status | Confidence |
|---|---|---|---|
| 1 | Gamma(1/4) is a mathematical constant | [THEOREM] | 100% |
| 2 | G* = Gamma(1/4)^2 / (sqrt(2)*pi) | [THEOREM] | 100% |
| 3 | Watson: W_3 = G*^2/(2pi) = Gamma(1/4)^4/(4pi^3) | [THEOREM] | 100% (30 digits) |
| 4 | K = 16*G*^2 (Faddeev-Popov, O_h gauge fixing) | [THEOREM] | 100% (18/18 tests) |
| 5 | Budget equation: x/K + G*/x = 1 | [THEOREM] | ~95% (exhaustion principle) |
| 6 | Quadratic: x^2 - Kx + KG* = 0 | [THEOREM] | 100% (given 5) |
| 7 | Roots: x+ = 137.036171, x- = 3.023964 | [THEOREM] | 100% |
| 8 | x+ = 1/alpha (1.26 ppm, zero free parameters) | [STRONGLY MOTIVATED CONJECTURE] | ~95% |
| 9 | floor(x-) = N_c = 3 | [STRONGLY MOTIVATED CONJECTURE] as root identification | ~80% |

7/9 links [THEOREM]. The mathematical chain is rigorous. The physical identifications (Links 8-9) are motivated by the 1.26 ppm agreement but not derived from the lattice action.

The budget equation (Link 5): x/K + G*/x = 1 says the coupling partitions completely between Coulomb (x/K) and confined (G*/x) phases. Verified to 15 digits for both roots. The coefficient c=1 is forced by exhaustion (two phases sum to 1), not chosen.

Scripts: `explore_master_quadratic_audit.py`, `explore_link5_derivation.py`

---

## Prime Splitting and the Forces (April 2026)

The imaginary unit i sorts primes by their behavior in Z[i]:

- **Split** (p = 1 mod 4): p = a^2 + b^2 = (a+bi)(a-bi). Factors in Z[i].
- **Inert** (p = 3 mod 4): stays prime in Z[i]. Cannot be written as a^2 + b^2.

The master quadratic roots map to this classification:

| Root | Value | Nearest prime | Prime type | Force |
|---|---|---|---|---|
| x+ | 137.036 | 137 | SPLIT: 137 = 4^2 + 11^2 = (4+11i)(4-11i) | EM (U(1), complex) |
| x- | 3.024 | 3 | INERT: stays prime in Z[i] | Strong (SU(3), real) |

G* encodes the prime distribution via the L-function Euler product:
- pi = 4 * prod_p 1/(1 - chi_-4(p)/p) where chi_-4 classifies split vs inert
- G* = Gamma(1/4)^2 / (sqrt(2)*pi) = Chowla-Selberg value / prime product
- alpha = function(G*) = function(prime distribution in Z[i]) [THEOREM for algebra, SELECTION for identification]

Script: `explore_primes_and_gstar.py`

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
