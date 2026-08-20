# LEDGER index — categorised claim map

**Generated** by `scripts/theory/build_ledger_index.py` from `LEDGER.md` (941 claim rows). **Do not edit by hand** — regenerate instead.

This is a *navigation aid*, not a source of truth. `LEDGER.md` remains the single source of truth for claim status; where this index and the ledger disagree, **the ledger wins**.

**Reading the tag column.** It lists the row's *complete* canonical tag set — never truncated — so a row carrying both `[THEOREM]` and `[CLOSED NEGATIVE]` shows both. This corpus is majority negative results, and a claim's verdict is frequently the *last* tag in its cell. The `Claim` column is a verbatim (possibly truncated) substring of the row's short name; the verbatim tag prose is in `LEDGER.md`.

**How to use this file:** read it whole to see what has already been investigated, then open the cited `LEDGER.md` line for the full record. Searching the ledger directly is the failure mode this file exists to prevent — its rows run to 30 KB each.

## Contents

| Programme | Claims | Scope |
|---|---:|---|
| [Algebraic spine — master quadratic](#algebraic-spine-master-quadratic) | 18 | The polynomial x^2-16G*^2x+16G*^3, its roots, the coefficient 16, D=3, minimality/uniqueness scans, and the x+ = 1/alpha identification. |
| [Algebraic spine — G*, CM curves, modular](#algebraic-spine-g-cm-curves-modular) | 41 | G* itself, lemniscatic/CM-curve theory, Chowla-Selberg, modular and quasi-modular values, the chi_-4 character, FQCR, Sym^k period algebra. |
| [Algebraic spine — periods, Watson, transcendence](#algebraic-spine-periods-watson-transcendence) | 14 | Watson integrals, lattice Green-function periods, the native closure N, delta-independence, E1/E2 transcendence, the period-import frontier. |
| [Framework — postulates & constitution](#framework-postulates-constitution) | 14 | P1-P5, the Framework Commitments (FC-0/1/2/3/W), the axiom register, calibrations, adoption-pricing rules. |
| [Framework — boundary, imports, consumption](#framework-boundary-imports-consumption) | 55 | The modulus/argument frontier, type-priority, the priced-import ledger, the consumption programme, act-counts, what the ontology cannot self-set. |
| [Framework — audits, red-teams, reconciliation](#framework-audits-red-teams-reconciliation) | 25 | Adversarial audits, red-team remediation, retractions, tag-honesty and corpus-wide reconciliation passes, rigidity / look-elsewhere audits. |
| [Quantum foundations](#quantum-foundations) | 19 | Born rule, measurement and the declined map M, CHSH/Bell, Spekkens, the deviation-prediction ledger, frame-relative projection. |
| [SM constants — mass & flavour](#sm-constants-mass-flavour) | 33 | m_e, m_p/m_e, Higgs mass, mixing angles, PMNS/CKM, Yukawa prefactors, the cluster-size-mass identification and its N(A) law. |
| [Alpha readout programme (MC-T4.3)](#alpha-readout-programme-mc-t4-3) | 33 | The alpha-readout contract, the ARC-A/B/C campaigns, observable selection, FC-W and the carrier-narrowing theorem, engine alpha probes. |
| [QCD, colour & electroweak](#qcd-colour-electroweak) | 15 | Confinement, colour charge and singlets, SU(3)/Z3 structure, hadrodynamics, electroweak rank, generations, no-4th-generation. |
| [Gravity & cosmology](#gravity-cosmology) | 31 | Newton's law from the substrate, graviton/spin-2 provenance, Kerr-Newman, strong-field signatures, Lambda, dark matter. |
| [Engine infrastructure & RG](#engine-infrastructure-rg) | 39 | Langevin/thermostat, operator-mixing matrices, RG flow and blocking, the bridge-contract gates, Ward identities, GPU/CUDA ports and parity. |
| [Engine emergence campaigns](#engine-emergence-campaigns) | 37 | Fermion-emergence phases, genesis/evaporation and thermal phase maps, atomic and bound-state spectra, wave sectors and dispersion. |
| [Lorentz recovery & causal structure](#lorentz-recovery-causal-structure) | 37 | The discrete flux pole, anisotropy exponents, the common cone, CFL and causal normalisation, preferred-frame operators, anisotropic-QED RG. |
| [Charge, Gauss & native EM emergence](#charge-gauss-native-em-emergence) | 15 | Native additive charge, Gauss projection and dressing, face-current sidecars, longitudinal susceptibility, dressed hazards, monopoles. |
| [Common-action mechanics & reciprocity](#common-action-mechanics-reciprocity) | 146 | Forces, work and recoil for a hop; the worldline/Legendre action; charts, collisions and quotients; energy closure and Peierls barriers. |
| [Constituent-complete matter](#constituent-complete-matter) | 167 | Compact cores, trimers and connected blocks; rest states and Hessians; transport, gait, capture/binding, wakes, causal-horizon persistence. |
| [Native time & the carrier programme](#native-time-the-carrier-programme) | 185 | The quartic action-angle clock, G* as a temporal invariant, the C1/C2/C3 carrier conditions and every carrier candidate opened against them. |
| [Meta — papers, tooling, project process](#meta-papers-tooling-project-process) | 17 | Paper splits and referee rounds, monographs, node maps and synonymy graphs, trackers, pre-registration registries, project policy. |

---

## Algebraic spine — master quadratic

*The polynomial x^2-16G*^2x+16G*^3, its roots, the coefficient 16, D=3, minimality/uniqueness scans, and the x+ = 1/alpha identification.*

**18 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0001` | THEOREM | Master Quadratic Polynomial + Roots | L185 |
| `FTD-0002` | THEOREM | G* algebraic identity (Watson–Chowla–Selberg) | L186 |
| `FTD-0003` | THEOREM | CM-curve uniqueness across class-number-1 fields | L187 |
| `FTD-0006` | THEOREM | Coefficient 16 from \|Aut(E)\|² (Route A) | L190 |
| `FTD-0007` | SELECTION | Coefficient 16 from z_BCC × 2 (Route B) | L191 |
| `FTD-0008` | THEOREM | Moore neighbourhood integers {N_base=4, N_eff=13, b_3=7} | L192 |
| `FTD-0010` | THEOREM | D = 3 from \|Aut(E)\|² = 2^D · (D−1)! | L194 |
| `FTD-0012` | THEOREM | Discriminant trichotomy (bosons/critical/fermions) | L196 |
| `FTD-0013` | SMC | x₊ 1/α (1.26 ppm) | L197 |
| `FTD-0032` | RETRACTED | Master quadratic as L → ∞ limit of finite-L gap equation | L251 |
| `FTD-0050` | CLOSED_NEGATIVE | Master quadratic as characteristic polynomial of an RG step on the FTD engine | L269 |
| `FTD-0080` | FOUNDATION | Cogito–axiom bridge + full reverse-engineering trace: initial justification formalized | L342 |
| `FTD-0081` | THEOREM, SELECTION | Master quadratic unified motivation: two-route convergence (physics + L-values) narrows S1 to minimum-degree selection | L341 |
| `FTD-0082` | THEOREM | Master quadratic bare algebraic decomposition: $\alpha + 1/N_c = 1/G^*$ as single-line content | L340 |
| `FTD-0083` | THEOREM | Program E closure: uniqueness of the master quadratic as minimal polynomial in the bounded $G^*$-integer class | L339 |
| `FTD-0084` | THEOREM, SELECTION | Program A partial closure: ladder-walk step-size multiset $\{3,3,4,6\}$ from $O_h$ structure | L338 |
| `FTD-0210` | CLOSED_NEGATIVE, SMC | x_- physical-identification search — Arc B P1 of Wilsonian-reframe plan v2 | L368 |
| `FTD-0312` | CLOSED_NEGATIVE, OPEN, SMC, THEOREM | Is the master quadratic's smaller root x₋ = 3.024 (its residual δ_c = x₋−3 = 0.024) "the dimensionless pressure of the flux" (owner conjecture)?… | L458 |

---

## Algebraic spine — G*, CM curves, modular

*G* itself, lemniscatic/CM-curve theory, Chowla-Selberg, modular and quasi-modular values, the chi_-4 character, FQCR, Sym^k period algebra.*

**41 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0111` | THEOREM | Harmonic invariant of the master-quadratic (1+i)-tower (Theorem 8 of SPEC_ALGEBRAIC_SPINE) | L303 |
| `FTD-0112` | THEOREM | Field-theoretic characterization of `Q(G*)` as a `π`-free subfield of `Q(π, Γ(1/4))` (Theorem 9 of SPEC_ALGEBRAIC_SPINE; "maximal" struck 2026-06-24… | L319 |
| `FTD-0122` | DERIVED, NO_GO | BCC complex-structure theorem: dual-4 partial unification (Roles 1+3 via Z[i]) + honest no-go (Roles 2+4 are count coincidences) | L309 |
| `FTD-0123` | NUMERICAL_FACT | Chowla-Selberg Γ-product dual-match scan: ZERO h≥2 dual-matchers across 54 discriminants (class numbers 2-4) | L308 |
| `FTD-0124` | NUMERICAL_FACT | 9-Heegner CM-tower rigidity scan + criterion-bifurcation finding | L307 |
| `FTD-0127` | DERIVED | G\* as the parity-twist between ζ and L(s, χ_{−4}) at the critical-line center; complete boundary structure of L(s, χ_{−4}) closed-form in Q(G\*, γ… | L345 |
| `FTD-0132` | SYNTHESIS | G\* as the squared theta nullwert of the Z[i] lattice at its CM point τ = i, normalized by √(2π); operational re-statement explaining FTD's… | L347 |
| `FTD-0143` | CLOSED_NEGATIVE, SELECTION, SMC | FQCR Model-IV (4,6;3,2) quotient-uniqueness scan — pre-registered 2026-05-06, EXECUTED 2026-07-12 | L198 |
| `FTD-0154` | THEOREM | G* in P^exp (exponential periods, Kontsevich-Zagier sense); strict P-status conditional on KZ conjecture | L201 |
| `FTD-0155` | THEOREM | Level-one modular forms at τ=i: f(i) ∈ Q·E₄(i)^(k/4) = Q·(G*⁴/π²)^(k/4) for f ∈ M_k(SL₂(Z)) with 4∣k; f(i)=0 otherwise | L202 |
| `FTD-0156` | THEOREM | Generalised Watson identity: W^(D) = _DF_{D-1}(½,...,½; 1,...,1; 1) for all D ≥ 3 | L203 |
| `FTD-0157` | THEOREM | Equianharmonic dichotomy at τ=ρ: parallel framework for K=Q(ρ) with G_ρ := Γ(1/3)Γ(1/6)/(2π√π) playing the analog role of G*; vanishing pattern… | L204 |
| `FTD-0158` | THEOREM | Quasi-modular value algebra at τ=i: Q[E_2(i),E_4(i),E_6(i)] = Q[π⁻¹, G_G⁴] as polynomial ring in 2 transcendentally-independent generators… | L205 |
| `FTD-0159` | THEOREM | L(E_lemn, 1) closed form: L(E_lemn, 1) = ϖ/4 = πG_G/4 = G*√π/8, with corrected BSD accounting (full real period Ω=2ϖ, c_2=2 Tamagawa at p=2 Kodaira… | L206 |
| `FTD-0160` | SYNTHESIS | Closure of paper open-problems P3 (R_4 distinguishedness) + P5 (equianharmonic generalisation) | L207 |
| `FTD-0161` | CONJECTURE | Conjecture: W^(4)_BCC = (2/π)² · _4F_3(½⁴; 1³; 1) ≈ 0.4534 is algebraically independent of {G*, π, Γ(1/4)} over Qbar | L208 |
| `FTD-0162` | CONJECTURE | Conjecture: G_Catalan = L(χ_{-4}, 2) is algebraically independent of {G_G, π} (equivalently of {G*, π}) over Qbar | L209 |
| `FTD-0163` | SYNTHESIS | Character-unification theorem: G*/G_G dichotomy is the projection of the Kronecker character χ_{-4} of Q(i) at four arithmetic levels (lattice… | L210 |
| `FTD-0164` | CLOSED_NEGATIVE | Three structural candidates for closing the χ_{-4} → P_{G*} arrow (master quadratic from CM theory) all fail. (N1) Class polynomial: H_{-4}, H_{-16}… | L211 |
| `FTD-0165` | THEOREM | New auxiliary identity 2·η(2i)·η(i/2)³ = G_G² (clean low-weight η-product at K=Q(i)) | L212 |
| `FTD-0166` | THEOREM | Asymptotic-regime theorem for y² - 16 R^p y + 16 R^q = 0: only q = p+1 gives y_-(R) → R; among q=p+1, only (p,q)=(2,3) gives non-vanishing CONSTANT… | L213 |
| `FTD-0167` | OBSERVATION | Joint-matching uniqueness: (p,q)=(2,3) is the unique exponent pair among (p,q) ∈ Z² with \|p\|, \|q\| ≤ 5 such that BOTH roots of y² - 16 R^p y + 16… | L214 |
| `FTD-0168` | SYNTHESIS | χ_{-4}(n) = Im(i^n) = sin(πn/2); value set {χ_{-4}(n)} = {i², 0, \|i²\|} = {-1, 0, +1} coincides as a set with the FTD ternary voxel alphabet.… | L215 |
| `FTD-0169` | THEOREM | Conjecture: P_{G*}(x) = x² - 16G*²x + 16G*³ is the unique minimal-degree polynomial with coefficients in distinct graded pieces Sym^a, Sym^b of… | L216 |
| `FTD-0175` | THEOREM, SELECTION | Sym²⊕Sym³ uniqueness theorem (Paper A §16.5): (a, b) = (2, 3) is the unique minimal-a exponent pair such that the leading-period polynomial x² - 16… | L222 |
| `FTD-0176` | POSITIVE | chi_{-4} structure in engine: GPU campaign (WSL2/CUDA/L=32) testing whether the engine empirically manifests Z[i]-module structure. Three GPU tests… | L223 |
| `FTD-0177` | INFRASTRUCTURE, VERIFIED, OBSERVATION | Phase 0 of G* opus follow-up: symmetric period algebra infrastructure verified at 80 digits. Identities I1 (Φ(ω²)=G*²π), I2a (Φ(η²)=π/G*²), I2b… | L224 |
| `FTD-0178` | DERIVED | Phase 1 L2: J Hodge complex structure on Sym^k(H¹(E_lemn)) — implementation + verification. J(ω) = −i·η/G*, J(η) = i·G*·ω, semi-linear on… | L225 |
| `FTD-0179` | DERIVED | Phase 1 L3: J-eigenspace decomposition of Sym^k(H¹(E_lemn)) for k ∈ {2,3,4,5} + Sym⁴ Z[i]-trivial reconciliation. J-eigenspace decomposition computed… | L226 |
| `FTD-0180` | DERIVED, HYPOTHESIS | Phase 1 L4: H4 confirmed — (a,b) = (2,3) is the unique minimum-a admissible pair in Paper A Theorem 17.5's leading-period family for a ≤ 5, b ≤ 6.… | L227 |
| `FTD-0181` | THEOREM, CORRECTION | Phase 1 L6: integer-4 unification (corrected T-A2). The lemniscatic catalogue 4's classify into 3 classes; Q(i) is the unique imaginary quadratic… | L228 |
| `FTD-0182` | DERIVED | Phase 1 L5: Conjecture 16.5.2 closed — the Sym^a residual reduces to Theorem 17.5 via the reality-collapse lemma. G* opus follow-up target T-A3.… | L229 |
| `FTD-0183` | CLOSED_NEGATIVE | Phase 1 L7: G* opus follow-up Tier B (T-B1, T-B2) CLOSED NEGATIVE — N_base=4 is a crystallographic coincidence, not a Z[i] bridge. G* opus follow-up… | L230 |
| `FTD-0212` | CLOSED_NEGATIVE | Lemniscatic K_2-regulator closed-form derivation | L370 |
| `FTD-0237` | OBSERVATION, CONJECTURE, SELECTION | Gaussian–Eisenstein dichotomy & the $2^4=4^2$ coefficient uniqueness | L403 |
| `FTD-0321` | NUMERICAL_FACT, CLOSED_NEGATIVE, MEASURED, CORRECTION | Does the full per-ideal-class Damerell scan preserve `d = -4` as the unique dual-matcher, or do the h-1 extra components at `h >= 2` unlock new… | L467 |
| `FTD-0366` | SYNTHESIS, OBSERVATION, SMC, FOUNDATIONAL_OBSTRUCTION | G\* is the irreducible transcendental of the strongly-coupled quartic matrix model — CHPS 2018 mapped as an external construction site for ℚ(G\*)… | L511 |
| `FTD-0367` | THEOREM, COHERENT_INTERPRETATION, CONJECTURE, SMC, FOUNDATIONAL_OBSTRUCTION | Reflection flow parity — the product and ratio branches as first-order flows whose coefficients split by parity, differential algebraicity, and value… | L512 |
| `FTD-0381` | OBSERVATION | The parity twist as a superdeterminant (CHPS r=4 χ₋₄-graded moment module) | L526 |
| `FTD-0382` | SYNTHESIS, OBSERVATION, COHERENT_INTERPRETATION, SELECTION | The bilateral-symmetry criterion for an orientation carrier: C_s = Stab_{O(3)}(v,g) | L527 |
| `FTD-0803` | THEOREM, MEASURED, REFUTATION, CORRECTION | Why is the `G*` window crowded? Is FTD-0321's measured base rate an accident of the scan, or forced by where `G*` lives? | L965 |

---

## Algebraic spine — periods, Watson, transcendence

*Watson integrals, lattice Green-function periods, the native closure N, delta-independence, E1/E2 transcendence, the period-import frontier.*

**14 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0079` | MEASURED | Watson integral $W_{\rm Moore-18}$ computed numerically; bridge ratio $W_{\rm M18}/W_{\rm BCC} = 0.913$ — universal 27/8 bridge falsified | L343 |
| `FTD-0113` | THEOREM | Retarded/static lattice Green identity | L318 |
| `FTD-0116` | CLOSED_NEGATIVE, HYPOTHESIS | G*² as FTD lattice Z-factor (UV-IR matching constant) — physical interpretation of Watson identity with falsifiable predictions | L311 |
| `FTD-0118` | VERIFIED, OPEN | Q3 + Q4 engine-stencil cross-checks (G18 confirmation of FTD-0113 + FTD-0116 falsification) | L317 |
| `FTD-0368` | SCOPE_CONTRACT, SELECTION, THEOREM, COHERENT_INTERPRETATION, FOUNDATIONAL_OBSTRUCTION, SMC, AXIOM | The δ-independence program chartered — define the native closure N, then prove (or refute) δ = √(G\*(4G\*−1)) ∉ N; MC-T4.3's negative-side completion… | L513 |
| `FTD-0369` | THEOREM, DERIVED, SELECTION, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | δ-IND v1 verdict: PROVEN-CONDITIONAL — δ = √(G\*(4G\*−1)) lies outside the frozen native closure N under the enumerated independence package E0–E2… | L515 |
| `FTD-0370` | THEOREM, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | The ramification locus of the native closure — the substrate ramifies only where it lives: Ram_t(hull) = {0, ∞} unconditionally-beyond-Chudnovsky… | L514 |
| `FTD-0372` | NUMERICAL_FACT | The engine's default 18-point (SC+FCC)/2 Green's function is the period of an explicit order-4, degree-12 ODE — computed and classified (exported… | L517 |
| `FTD-0373` | THEOREM, NUMERICAL_FACT | W₁₈ is not self-dual — the rigid-Calabi–Yau / weight-4-modular branch of the FTD-0372 residual is CLOSED NEGATIVE | L518 |
| `FTD-0374` | NUMERICAL_FACT, SMC, THEOREM | The two-loop BCC sunset period retains lemniscatic (ℤ[i]/Γ(1/4), j=1728) character at two loops — the discrete-Feynman M2 falsifier's… | L519 |
| `FTD-0375` | SYNTHESIS, THEOREM, CLOSED_NEGATIVE | Period-conjecture framing of the import boundary (`MATH_PERIOD_IMPORT_FRONTIER.md`) | L520 |
| `FTD-0376` | SYNTHESIS, THEOREM | E1/E2 transcendence state-of-the-art + the precise price of δ∉N (`ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md`) | L521 |
| `FTD-0377` | THEOREM, SYNTHESIS | {π, W_SC} algebraically independent — the disc −24 reduction closes E1's per-constant floor | L522 |
| `FTD-0378` | THEOREM, SYNTHESIS | Exponential lattice periods are transcendental — E2's individual-transcendence sub-question closed for the SC and BCC symbols | L523 |

---

## Framework — postulates & constitution

*P1-P5, the Framework Commitments (FC-0/1/2/3/W), the axiom register, calibrations, adoption-pricing rules.*

**14 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0030` | CALIBRATION | a_phys (lattice → physical length conversion) | L249 |
| `FTD-0036` | AXIOM | Postulate 1 (Discrete Space) — uncontained, undefined-boundary cubic lattice; finite realized support/computational probes, no enclosing wall or… | L255 |
| `FTD-0037` | SELECTION | Postulate 2 (Discrete Time) — emergent from Lagrangian | L256 |
| `FTD-0038` | AXIOM | Postulate 3 (Ternary States {−1, 0, +1}) | L257 |
| `FTD-0039` | THEOREM | Postulate 4 (26-Moore locality) — derived from P1 + symmetry | L258 |
| `FTD-0040` | THEOREM | Postulate 5 (Determinism) — independent globally; explicit finite-window update is deterministic | L259 |
| `FTD-0041` | CALIBRATION | a_phys ≡ ℓ_P calibration declaration (with K_B = m_e mass anchor) | L260 |
| `FTD-0128` | SYNTHESIS | Postulate 3 ternary state values `{−1, 0, +1}` grounded in Axiom 0 via `s = i²`; framework axiomatic footprint reduced by three independent numerical… | L346 |
| `FTD-0253` | SYNTHESIS, BOUNDARY, THEOREM, AXIOM | Is FTD's spacetime forced by the postulates? — the reversibility boundary | L416 |
| `FTD-0254` | SYNTHESIS, AXIOM, SMC, FOUNDATIONAL_OBSTRUCTION | Framework Spec v1 — the constitution (standalone-framework synthesis + FC register) | L417 |
| `FTD-0255` | AXIOM, THEOREM, OPEN | FC-1 — the framework declines the measurement-map import M (commutative algebra A₅ is complete) | L418 |
| `FTD-0256` | AXIOM, SYNTHESIS, THEOREM, CLOSED_NEGATIVE | FC-2 — the arrow is native; global reversibility declined; Lorentzian metric emergent-IR + sector-scoped; space ⊥ time | L419 |
| `FTD-0257` | SYNTHESIS, SELECTION, MEASURED, THEOREM, IMPOSED | Two-orthogonal-fields formalization — Flux ⊥ State primary pair; nested symplectic (q,p) quadrature pair; decompositions-not-dimensions | L420 |
| `FTD-1000` | SUPERSEDED, SYNTHESIS | Does folding CLK-1 into FC-2's metric-half declaration correctly register an adoption rather than a derivation, and does it require disambiguating… | L1138 |

---

## Framework — boundary, imports, consumption

*The modulus/argument frontier, type-priority, the priced-import ledger, the consumption programme, act-counts, what the ontology cannot self-set.*

**55 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0121` | SYNTHESIS | Physics-bridge crystallization (synthesis of mathematical spine + empirical match + structural-uniqueness arguments) | L314 |
| `FTD-0136` | METHODOLOGICAL_REFRAME | Discrete-Native Derivation Program — methodological reframe: substrate-derivation goal restated as "derive engine-measurable observables and compare… | L353 |
| `FTD-0136-PhaseB-final` | UNKNOWN | Phase B.3 boundary-configuration investigation: comprehensive negative finding with 3 retractions in F1/F9 hygiene pattern | L354 |
| `FTD-0137` | METHODOLOGICAL_CLARIFICATION | Lattice spacing as gauge freedom: `a_phys` reframed from "natural minimum scale" declaration to gauge degree of freedom undetermined by FTD axioms… | L355 |
| `FTD-0153` | SYNTHESIS | Math-First Ontology | L200 |
| `FTD-0186` | DEFINITION, STAGE_1_CLOSED_POSITIVE | Boundary theorem Stage 1 — the structural / dynamical discriminator + classification of the decisive load-bearing set | L233 |
| `FTD-0311` | SYNTHESIS, PARAMETRIC, SMC | The FTD Complete Framework — the unified honest map: one canonical doc stating what FTD derives, what it provably cannot (and why), what it predicts… | L457 |
| `FTD-0322` | SYNTHESIS, THEOREM, SMC | Act-reduction count — how many "acts of intent" does the chain need? (single-`i`-act reduction test); verdict PARTIAL | L468 |
| `FTD-0323` | SYNTHESIS, THEOREM, AXIOM, SMC | The arrow of time as a square root | L469 |
| `FTD-0324` | SYNTHESIS, MEASURED, AXIOM, SMC | The arrow's direction is forced-given-FC-2, not free | L470 |
| `FTD-0325` | SYNTHESIS, PARAMETRIC, SMC | Act-count completed over the SM — no field-act beyond {i, δ}; verdict CLOSED | L471 |
| `FTD-0326` | DERIVED, SYNTHESIS, NO_GO, THEOREM, FOUNDATIONAL_OBSTRUCTION, SMC | No FTD-native ℤ/2 supplies δ — MC-T4.3 boundary robust against every native orientation symmetry; verdict PERMANENT-EXTENDED | L472 |
| `FTD-0327` | SYNTHESIS, OPEN, SELECTION, SMC | The AGM place-bridge — why the substrate's √-machinery lands on G\*, never on δ | L475 |
| `FTD-0328` | RECONCILIATION, OPEN, NUMERICAL_FACT, SELECTION, THEOREM | Corpus-wide reconciliation of the FTD-0318 spine-audit demotions + FC-W/act-count sync + FTD-0189→0319 repoint | L474 |
| `FTD-0329` | SYNTHESIS, SELECTION, SMC, PARAMETRIC, FOUNDATIONAL_OBSTRUCTION | The Inherited-Assumptions Audit — FTD as the realist reconstruction of physics from a discrete logical core | L473 |
| `FTD-0335` | SYNTHESIS, OPEN, BOUNDARY, SMC, FOUNDATIONAL_OBSTRUCTION, MEASURED | Boundary-map refresh — fold the 2026-06-26/27 boundaries into `AUDIT_BOUNDARY_MAP.md` | L480 |
| `FTD-0336` | SYNTHESIS, CONJECTURE, SPECULATION, THEOREM, SMC, FOUNDATIONAL_OBSTRUCTION | The modulus/argument frontier — the canonical boundary statement: a discrete deterministic forward-only substrate owns the *modulus* half and cannot… | L481 |
| `FTD-0339` | SYNTHESIS, CONJECTURE, SMC, FOUNDATIONAL_OBSTRUCTION | The type-priority principle — context before content; the Framework Commitments are precondition-types | L484 |
| `FTD-0340` | SYNTHESIS, CONJECTURE, THEOREM, DERIVED, SMC, FOUNDATIONAL_OBSTRUCTION | The square root as an act of selection — the algebraic signature of type-setting | L485 |
| `FTD-0341` | DERIVED, SYNTHESIS, OPEN, SMC, FOUNDATIONAL_OBSTRUCTION | The four named analytic-orientation carriers for `δ` all close negative; the magnitude/phase theorem | L486 |
| `FTD-0342` | SYNTHESIS, AXIOM, CONJECTURE, SPECULATION, SMC, FOUNDATIONAL_OBSTRUCTION | Tick and fold as temporal generators — time reduced to two near-non-temporal primitives | L487 |
| `FTD-0343` | SYNTHESIS, PARTIAL, SMC, FOUNDATIONAL_OBSTRUCTION | Why type theory, not OOP — completing FOUND_TYPE_PRIORITY_PRINCIPLE's type-theory analogy | L488 |
| `FTD-0344` | CONJECTURE, OPEN, BOUNDARY, SMC, FOUNDATIONAL_OBSTRUCTION | Wins and walls share one root — the same renunciation dissolves the UV/CC catastrophes and forbids the argument-half | L489 |
| `FTD-0353` | THEOREM, OPEN | Valuation theorem at (4G*-1) -- the alpha-boundary in one statement | L503 |
| `FTD-0354` | THEOREM, CONJECTURE | GNC rigidity lemma -- two walls, one shape | L504 |
| `FTD-0355` | RECONCILIATION, SELECTION | Permanent verdicts on the three graded-weakest | L505 |
| `FTD-0357` | THEOREM, SYNTHESIS, OPEN | Four-walls-are-one: CLOSED as a distinctness theorem — one schema, two axes, no forcing | L499 |
| `FTD-0358` | THEOREM, SELECTION | Construction-class closure: representativeness flag CLOSED (no-go + FC-0 sector-lock + declaration) | L500 |
| `FTD-0365` | UNDERDETERMINED | Genesis-cokernel grading construction v1 — UNDERDETERMINED (re-scope): the lossy-merge fiber's section-invariant content is exact-rational + K_B; no… | L510 |
| `FTD-0371` | SYNTHESIS, SMC, PARAMETRIC, THEOREM, SELECTION, FOUNDATIONAL_OBSTRUCTION, AXIOM | The priced-import ledger — every type FTD must import, counted in a common currency with a falsifier on each line; the Number-One-Goal "mark the… | L516 |
| `FTD-0383` | SCOPE_CONTRACT, SELECTION, THEOREM, PARAMETRIC, NO_GO | The Consumption Program chartered — four fronts driving the import bill toward its audited floor (the Number-One Goal's new drive face) | L528 |
| `FTD-0384` | RECONCILIATION | The pre-registration registry truth-reconciliation (Arc 1 "Honest Mint" of the Consumption Program) | L530 |
| `FTD-0385` | SYNTHESIS, DERIVED, SELECTION | The Planck-calibration semantics: the √3-ladder, the naming theorem (c·t_P = ℓ_P ⟺ the edge gauge), and the double selection of the current naming | L532 |
| `FTD-0386` | SYNTHESIS, AXIOM, SELECTION, IMPOSED, THEOREM, SMC | The Unified Axiom Register + the Unified Conditional Statement (Unification Annex Stage U1) | L533 |
| `FTD-0387` | SCOPE_CONTRACT, PARAMETRIC, THEOREM, SMC, AXIOM | Adoption Pricing Rules (Unification Annex Stage U0) — the D5 currency, the D6 compression predicate, and the D7 FC-W calibration | L534 |
| `FTD-0388` | SELECTION, THEOREM, MEASURED | K_MANIFEST := W_SC — the manifestation-kinetics scale adopted at the substrate's unit-charge Gauss self-energy (self-energy pinning OUTCOME-P1) | L535 |
| `FTD-0395` | THEOREM, AXIOM | The complete current engine update map is non-injective on a public-API-admissible domain | L536 |
| `FTD-0396` | NO_GO, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | Nonlinear delta-IND v2 properness audit: a fixed event budget does not by itself bound effective branch transcripts; unrestricted expressive power… | L537 |
| `FTD-0508` | SYNTHESIS, THEOREM, DERIVED | Do the four framework imports (FC-1, FC-2, FC-W, L²-budget) and the 2026-07-25 engine obstruction theorems instantiate one common… | L656 |
| `FTD-0509` | NUMERICAL_FACT, CONJECTURE, SYNTHESIS | Is the priced-import ledger's declared "adopted bit" the same unit as the substrate-derived per-merge branch-selection cost of FTD-0499, and how do… | L657 |
| `FTD-0510` | THEOREM, DERIVED, OPEN | Can D = 3 be forced by any route confined to the dimension-blind fragment of the axiom register, or is the FTD-0355 circularity a necessity of the… | L658 |
| `FTD-0515` | THEOREM, SYNTHESIS | At state level (not merely algebra level), what does purchasing the FC-2 section force, and what exactly is the tracial-accessible part of the record? | L663 |
| `FTD-0516` | SELECTION, THEOREM, DERIVED, CONSTRUCTIVE, OPEN | Can the FTD-0512 restricted collision impulse be derived as a variational corner condition without a persistent new variable? | L664 |
| `FTD-0518` | REFUTATION, THEOREM, SYNTHESIS | Does the FTD-0515 §4 symmetric-fiber sharpening survive its own named attack point, the L² wall (I5)? | L666 |
| `FTD-0520` | THEOREM, DERIVED, SYNTHESIS, OPEN | Does the torsor form extend to the dispersion boundary (the frontier's one ATTEMPTED row), and what exactly does FTD-0270's "cavity-not-Schrödinger"… | L667 |
| `FTD-0565` | NUMERICAL_FACT, CLOSED_NEGATIVE, SPECULATION, OPEN | Before subtracting the FTD-0552 self-force, what does the self-potential buy? (Toy probe of the fork's un-priced tine.) | L696 |
| `FTD-0566` | THEOREM, SYNTHESIS, CONJECTURE | Is there a criterion for which fibers demand imports and which the dynamics sections itself? | L697 |
| `FTD-0598` | SYNTHESIS | What is the least ontic extension forced by the reciprocal-mobile-matter boundary, and which additional types are required only for multibody matter… | L741 |
| `FTD-0669` | SYNTHESIS | What is the least ontology of matter supported by the reciprocal constituent and field results without identifying one voxel, one field line, or the… | L812 |
| `FTD-0740` | SCOPE_CONTRACT | What evidence standard defines the matter-ontology research program? | L883 |
| `FTD-0741` | DERIVED | What minimum information must any adequate current-branch matter ontology retain? | L884 |
| `FTD-0742` | SUPERSEDED | What was the matter-program evidence baseline before the resumed finite-support result? | L885 |
| `FTD-0743` | SCOPE_CONTRACT | What exact state-only predicate would count as a metastable matter object? | L886 |
| `FTD-0744` | SCOPE_CONTRACT | How should evidence decide among native state, reconstructed chart, constituent phase space, and connection-based extensions? | L887 |
| `FTD-1002` | SYNTHESIS | Does the FTD-1000 fold contradict the four-walls distinctness theorem (FTD-0357), or is it licensed by it? | L1140 |

---

## Framework — audits, red-teams, reconciliation

*Adversarial audits, red-team remediation, retractions, tag-honesty and corpus-wide reconciliation passes, rigidity / look-elsewhere audits.*

**25 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0060` | CLOSED_NEGATIVE | Baryon composition correction $K_{\text{comp}} = m_e/\pi$ (conjectured in manuscript ch. 1.10b) | L279 |
| `FTD-0061` | CONJECTURE | "b=2 block natively instantiates Cl(3,0)" fermion-emergence claim | L280 |
| `FTD-0062` | TAUTOLOGY | "Topological-drag derivation $\alpha_{\mathrm{FTD}} = \lambda_0/18 = 1/x_+$" (PDF draft §2) | L281 |
| `FTD-0063` | OPEN | "$m_p/m_e$ 174-ppm gap = $\alpha/42$ lattice self-energy" (PDF draft §3) | L282 |
| `FTD-0097` | MEASURED | Pre-registered look-elsewhere scan for FTD claim base | L302 |
| `FTD-0117` | BUG | Spine document G* formula and value typo (canonical reference bug) | L310 |
| `FTD-0300` | MEASURED, SMC | Halo-exponent forcedness audit (dark-matter / SPARC gate): is the lossless self-field halo exponent a forced geometric invariant or a… | L432 |
| `FTD-0301` | MEASURED, THEOREM, BOUNDARY, SELECTION, SMC, FOUNDATIONAL_OBSTRUCTION, IMPOSED | Proton-stability forcedness audit (the "micro" pier candidate after FTD-0300): is `τ_p = ∞` a forced prediction of the discrete ontology, or is the… | L433 |
| `FTD-0303` | METHODOLOGICAL_CLARIFICATION, THEOREM, SELECTION, NUMERICAL_FACT, OPEN, CONJECTURE, CLOSED_NEGATIVE, SMC | Epistemic-tag honesty reconciliation — three documents reconciled downward to canon during the 2026-06-15 theory-corpus consolidation pass | L452 |
| `FTD-0310` | MEASURED, STRUCTURAL_PARAMETRIC, SELECTION, DERIVED, PARAMETRIC, THEOREM, SMC | Rigidity audit of the load-bearing rational identifications (sin²θ_W=3/13, α_s=7/59, m_e prefactor 16/3): are these matches statistically special, or… | L456 |
| `FTD-0318` | AUDIT_FINDING, THEOREM, SELECTION, NUMERICAL_FACT, OPEN, CONJECTURE, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | Algebraic-spine deep adversarial audit — re-ran every proof script + recomputed every load-bearing identity at dps 100–150 + 3 hostile lenses per… | L464 |
| `FTD-0319` | MEASURED, NUMERICAL_FACT, SMC | Adversarial polynomial look-elsewhere scan — the master quadratic is the unique dual-matcher over ~2.65M degree-2 polynomials over an 18-constant… | L465 |
| `FTD-0320` | MEASURED, STRUCTURAL_PARAMETRIC, PARAMETRIC | Rigidity audit of the catalog rational identifications — extends FTD-0310 to the simple-rational mixing-angle/ratio claims still carrying… | L466 |
| `FTD-0345` | RECONCILIATION, SELECTION, CONJECTURE, THEOREM, DERIVED, SPECULATION, PARAMETRIC, SMC, FOUNDATIONAL_OBSTRUCTION | Red-team remediation — mechanical propagation sweep (citations, tags, benchmark re-grading) fixing 10-specialist red-team audit findings | L490 |
| `FTD-0346` | SYNTHESIS, OPEN, SMC, FOUNDATIONAL_OBSTRUCTION | Boundary-framing restructuring + external-review disclosure — replaces "invariance-as-virtue" and "deliverable" rhetoric with PROVEN/ATTEMPTED… | L491 |
| `FTD-0347` | SYNTHESIS, RECONCILIATION, SELECTION, NUMERICAL_FACT, OPEN, THEOREM, PARAMETRIC, SMC, FOUNDATIONAL_OBSTRUCTION | Provisional two-specialist review (math + physics) + fix of its six confirmed findings | L492 |
| `FTD-0348` | SYNTHESIS, RECONCILIATION, RETRACTED, THEOREM, DERIVED, SELECTION | Fable two-specialist review + fix of its mechanically-fixable findings (retraction of DERIV_NONCOMMUTATIVE_EMERGENCE; Einstein Step-4 correction… | L493 |
| `FTD-0351` | RECONCILIATION, DERIVED, THEOREM, SELECTION, UNDERDETERMINED | Theorem repairs: OT-2.7/FTD-0175 demoted to corrected-constraint-set [THEOREM] + conditional [SELECTION]; FTD-0244 Lemma 1 completed over Q(G*,pi) | L496 |
| `FTD-0352` | SYNTHESIS, CONJECTURE, SPECULATION, SELECTION, SMC | Math grading pass + disciplined extension scouting | L497 |
| `FTD-0356` | RECONCILIATION, RETRACTED, THEOREM, SELECTION, CLOSED_NEGATIVE, OPEN, PARAMETRIC, IMPOSED, DERIVED, SMC | FTD-0175/OT-2.7 downstream propagation (queued under FTD-0351) + MONOGRAPH_EFFECTIVE_EQUATIONS cluster review (flagged under FTD-0348 §3 item 6)… | L498 |
| `FTD-0360` | RECONCILIATION, RETRACTED, THEOREM, OPEN, DERIVED, EMERGENT | Finalization mechanical batch: META_INDEX reconciliation; g_rr proof RETRACTED (third invalid [THEOREM]); Compton confirmed-conditional; node map… | L502 |
| `FTD-0361` | RETRACTED, RECONCILIATION, OPEN, THEOREM, PARAMETRIC, IMPOSED, MEASURED, SELECTION, SMC | MONOGRAPH_EFFECTIVE_EQUATIONS cluster review, wave 2 (the pass FTD-0356 left queued): radial-metric g_rr proof RETRACTED; Compton volume duality… | L506 |
| `FTD-0568` | SYNTHESIS, NUMERICAL_FACT | Is the quadratic-coat representation layer FTD-native machinery, or an independently re-derived instance of a known compatible-discretization… | L711 |
| `FTD-0785` | MEASURED, THEOREM | Does the algebraic spine survive a refute-by-default adversarial audit - the one load-bearing artifact never previously audited? | L932 |
| `FTD-0802` | CLOSED_NEGATIVE, CORRECTION, MEASURED | Does OT-3.3's zero dual-matcher count survive the base-rate control its sibling scan failed — and is the count itself correct? | L964 |

---

## Quantum foundations

*Born rule, measurement and the declined map M, CHSH/Bell, Spekkens, the deviation-prediction ledger, frame-relative projection.*

**19 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0023` | SELECTION | Bell violation S = 2√2 | L242 |
| `FTD-0024` | SELECTION | Loop coefficients c1=9/47, c2=5/64, c3=4/141 | L243 |
| `FTD-0170` | SYNTHESIS | The Born rule (ψ → \|ψ\|²) and the character χ_{-4} (i^n → Im(i^n)) are the same arrow applied at different scopes: both project a complex object… | L217 |
| `FTD-0187` | SELECTION | Born rule (P=\|ψ\|²) — canonical derivation-status consolidation row | L356 |
| `FTD-0188` | DEFINITION, THEOREM, CLOSED_NEGATIVE | κ_ψ = 4π audit — FQCR source-law normalization is a convention, not a theorem | L357 |
| `FTD-0199` | CLOSED_NEGATIVE, OPEN | Born-equilibrium preservation test (DGZ analog; T1c sub-investigation under FTD-0187) — does the substrate preserve a Born-distributed initial… | L366 |
| `FTD-0200` | CLOSED_NEGATIVE, CONJECTURE, OPEN | Threshold-crossing → Born rule test (T1c sub-investigation under FTD-0187) — tests the corpus assertions in `SPEC_SIX_ALGORITHMS.md:65` +… | L367 |
| `FTD-0225` | CLOSED_NEGATIVE | Route B — substrate algebra type for emergent modular time (B1) | L391 |
| `FTD-0226` | CLOSED_NEGATIVE | Derive-QM gap — manifestation non-commutativity (B-QM-1) | L392 |
| `FTD-0227` | PARTIAL | Spekkens knowledge-balance from the internal-observer restriction (B-QM-1′) | L393 |
| `FTD-0228` | CLOSED_NEGATIVE | Full symplectic budget symmetry from FTD geometry (B-QM-1″) | L394 |
| `FTD-0258` | SYNTHESIS, CLOSED_NEGATIVE, CONJECTURE, SELECTION, OPEN, THEOREM, BOUNDARY | Deviation-prediction ledger — six structural deviations from the QM/SR formalism (the falsifiable spine) | L421 |
| `FTD-0359` | THEOREM, CONJECTURE, IMPOSED, OPEN, CLOSED_NEGATIVE | PL-1 quantified: the Rice-vs-Born deviation in closed form — one parameter, five observables, an internal kill switch | L501 |
| `FTD-0517` | THEOREM, SYNTHESIS, CONJECTURE | What must a subsystem be, structurally, to function as a measuring device for the frozen projection — and what does its measuring power cost? | L665 |
| `FTD-0795` | EXACT, CLOSED_NEGATIVE, CONJECTURE, SELECTION | Do FTD-0258's six registered deviations from QM/SR survive contact with existing experiment? | L951 |
| `FTD-0796` | THEOREM, EXACT, CORRECTION | Can a framework committed to `A_5`-completeness (FC-1) reproduce CHSH `S = 2 sqrt 2`? | L953 |
| `FTD-0807` | IMPOSED, CORRECTION, OPEN | Which weighting do threshold upcrossings follow — amplitude, occupation (Born), or energy? | L969 |
| `FTD-0809` | MEASURED, OPEN | Does the mechanism-level Born regime carry into the engine, and is the latency sector a viable slow gate? | L971 |
| `FTD-0825` | AXIOM, SELECTION, THEOREM, OPEN | Does the contextual-actualization successor close the Bell/Born/time ambiguity at reference-model level without rewriting v1? | L987 |

---

## SM constants — mass & flavour

*m_e, m_p/m_e, Higgs mass, mixing angles, PMNS/CKM, Yukawa prefactors, the cluster-size-mass identification and its N(A) law.*

**33 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0015` | SELECTION, THEOREM | m_e = m_P · √(2π) · (16/3) · α¹¹ (0.19%) | L234 |
| `FTD-0016` | SMC | m_p/m_e = N_eff/α + N_base·N_eff + N_c (174 ppm) | L235 |
| `FTD-0017` | STRUCTURAL_PARAMETRIC | Higgs mass m_H = (N_eff/α²)·m_e (−0.36%, −4.1σ vs PDG 2024 125.20±0.11 — corrected per FTD-0348; the legacy "0.24%" figure reproduced only against… | L236 |
| `FTD-0022` | CONJECTURE | 7-term α series matching CODATA to 24 digits | L241 |
| `FTD-0094` | PARAMETRIC, OPEN | L2 candidate identity 2·m_e/α = 16G*² (68.77 ppm vs CODATA) | L299 |
| `FTD-0095` | THEOREM | Bridge Functional ontology commitment (mass-as-functional on master-quadratic root spectrum) | L300 |
| `FTD-0096` | THEOREM, OPEN | μ-from-ℓ_P missing arrow — mass-unit characterization independent of m_e | L301 |
| `FTD-0110` | DERIVED, SMC | Cluster-sizemass identification: bound-state spatial extent reproduces SM-particle masses via N ≈ ¼·(A/K_GENESIS)² with k(A) drift correction; ¼… | L320 |
| `FTD-0119` | BRIDGE_ANALYZED, OPEN | FTD-0110 nonlinear bridge analysis: three candidate mechanisms identified for log-A k(A) drift | L316 |
| `FTD-0133` | SELECTION, DERIVED | Honest-tag audit of FTD-0015's `√(2π)·(16/3)` prefactor: downgrade from prior [THEOREM] claim to [SELECTION] + factorization to `m_e/v = (16/3)·α³`… | L350 |
| `FTD-0134` | STRUCTURAL_PARAMETRIC, THEOREM, DERIVED | Electron Yukawa prefactor `16√2/3` decomposed as `mult(A_{1g})²/mult(T_{1u}) · \|1+i\|` via O_h character theory on 27-block + Z[i]-norm structure… | L351 |
| `FTD-0135` | CLOSED_NEGATIVE | Substrate-level Yukawa-vertex derivation attempt (path-to-[DERIVED] for FTD-0134); single-session closure | L352 |
| `FTD-0203` | SCOPING_MEMO | FTD-0110 nonlinear-bridge scoping memo: desk-analytical vs engine-resourced classification for the remaining closure work | L384 |
| `FTD-0219` | CLOSED_NEGATIVE | Absolute Mass Scale Calibration (μ) generation loopholes | L373 |
| `FTD-0220` | CLOSED_RESOLVED | No 4th Generation Fermions No-Go Formalization Campaign | L389 |
| `FTD-0221` | FOUNDATION | Discrete-Native Mass Foundations (Class A Observables) | L374 |
| `FTD-0222` | INFRASTRUCTURE | Class C Cluster-Cluster Interaction Specification | L375 |
| `FTD-0259` | VERIFIED, CLOSED_NEGATIVE, OBSERVATION, DERIVED, OPEN, SMC | FTD-0110 Mechanism α (multi-block irrep leakage) quantified — CLOSED as the k(A)-drift mechanism; queued projection calculation retired | L423 |
| `FTD-0260` | INVALID, OBSERVATION, OPEN | Thermostat-OFF amplitude sweep v1 — pre-registered run of record INVALID (V-1); diagnosis = engine-evolution reproducibility break of the FTD-0110… | L427 |
| `FTD-0261` | MEASURED, CLOSED_NEGATIVE, SMC | Current-stack N(A) law characterized — broken power law, knee at A≈16; thermostat effect = pure friction; FTD-0259 thermal-knee reading closed | L426 |
| `FTD-0262` | MEASURED, SMC | SM clustermass identification re-assessed on the current stack — IDENT-NULL: anchor holds, law extrapolates, no specialness at the SM ratios | L425 |
| `FTD-0263` | MEASURED, CLOSED_NEGATIVE | Sub-knee onset: 27-block hypothesis — GEOM-PARTIAL (C1 block-band fail); Mechanism β v2 model — BETA_v2_CONFIRMED (resolves threshold shift under… | L424 |
| `FTD-0264` | THEOREM, MEASURED, SELECTION | FTD-native polynomial effective action after blocking | L428 |
| `FTD-0265` | PARTIAL, CLOSED_NEGATIVE, OPEN | Mechanism β, envelope variant — BETA-PARTIAL per frozen rules; substantively the initial-crossing approximation is CLOSED NEGATIVE (over-counts… | L430 |
| `FTD-0266` | CLOSED_NEGATIVE | Mechanism β, sustained-kinetics (dwell-time Boltzmann) variant — DWELL-FAIL; Boltzmann rate too steep for dwell-time to matter; suppression is… | L429 |
| `FTD-0267` | MEASURED, CLOSED_NEGATIVE, OPEN | Genesis-vs-survival engine telemetry — first direct measurement of genesis/evaporation EVENTS; β arc's post-genesis-survival premise FALSIFIED… | L422 |
| `FTD-0268` | MEASURED, OBSERVATION, SYNTHESIS, CONJECTURE, SELECTION, PARAMETRIC, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | Blind L=257 extension of the FTD-0252 time-dilation residual law — PREDICTION_CONFIRMED (7/9 at threshold); PL-4 measured domain extended… | L431 |
| `FTD-0269` | MEASURED, OPEN, SUPERSEDED, DERIVED, SMC, FOUNDATIONAL_OBSTRUCTION | FTD-0110 nonlinear bridge: quantitative N(A) law from substrate parameters — verdict BOUNDARY (law is engine-emergent) | L434 |
| `FTD-0307` | CLOSED_NEGATIVE, OPEN, DERIVED, SMC, FOUNDATIONAL_OBSTRUCTION | FTD-0110 Convention Audit (exit ii): is the N(A) calibration gauge or physical? — verdict PHYSICAL (exit ii CLOSED NEGATIVE on both knobs) | L453 |
| `FTD-0308` | DERIVED, MEASURED, BOUNDARY, OPEN, IMPOSED, THEOREM, SMC | Engine-native atomic spectroscopy (FTD-0281 Leg-2): the engine's own clocked-flux time evolution in its own Coulomb well vs the operator built from… | L454 |
| `FTD-0309` | MEASURED, DERIVED, CLOSED_NEGATIVE, OPEN, SMC | FTD-0110 nonlinear bridge, v2 genesis-counting model: can a faithful collective-coordinate reduction land the N(A) cluster-mass shape as… | L455 |
| `FTD-0390` | CLOSED_NEGATIVE, SELECTION, THEOREM | m_e exponent n=11 ordering-selection look-elsewhere audit — the (S1) gravity-last / (S2) spinor-before-color rule pair is not independently forced | L543 |
| `FTD-0397` | THEOREM, SELECTION | Order-type no-go for the electron exponent: unordered FTD-0084 data cannot select `n=11` | L538 |

---

## Alpha readout programme (MC-T4.3)

*The alpha-readout contract, the ARC-A/B/C campaigns, observable selection, FC-W and the carrier-narrowing theorem, engine alpha probes.*

**33 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0011` | THEOREM | Phase H coupling scaling (g_c² scales α_r) | L195 |
| `FTD-0031` | OPEN | g_c first-principles derivation | L250 |
| `FTD-0093` | CLOSED_NEGATIVE | Mechanism C — `g_c` as bridge-operator eigenvalue on σ_BCC | L298 |
| `FTD-0129` | SYNTHESIS | Structural-decoupling synthesis: four independent engine tests (Phase G + Phase J + Phase I + Phase II) converge on the diagnosis that α = 1/x_+ does… | L304 |
| `FTD-0152` | SYNTHESIS | Alpha Readout Contract for MC-T4.3 | L199 |
| `FTD-0185` | PRE_REGISTRATION | Alpha arithmetic generativity Test 4 pre-registration | L232 |
| `FTD-0197` | CLOSED_NEGATIVE, SMC | Ternary-matrix BCC-snap test of a 2026-05-23 user-presented synthesis (Guillera → matrix promotion → BCC-snap → QM-collapse identification, step 3… | L387 |
| `FTD-0198` | PRE_REGISTRATION | Alpha-readout ARC-B1 observable-selection — MC-T4.3 closure attempt design (pre-registered, no closure attempt run) | L388 |
| `FTD-0204` | CLOSED_NEGATIVE | ARC-B1 alpha-readout observable-selection closure attempt against the plaquette-bivector A_obs candidate -- the FTD-0198 pre-reg §9 11-step method… | L383 |
| `FTD-0205` | CLOSED_NEGATIVE | ARC-B1 alpha-readout observable-selection -- closure attempts against catalog items 6 (boundary-to-boundary transfer observables, Session C3) + 7… | L382 |
| `FTD-0206` | PRE_REGISTRATION, SMC | Catalan algebraic-independence frontier-documentation pre-registration: Conjecture 19.2 of the G\* paper -- Catalan G algebraically independent of… | L381 |
| `FTD-0230` | UNDERDETERMINED | BCC Algebraic Bridge Readout (ARC-B2) | L397 |
| `FTD-0231` | UNDERDETERMINED, SELECTION | Alpha Quantization Readout (ARC-C1) | L398 |
| `FTD-0232` | CORRECTION, SMC, FOUNDATIONAL_OBSTRUCTION | MC-T4.3 alpha-readout FOUND audit + correction (2026-05-28 session): independent adversarial review of the ARC-C1/ARC-B2 "FOUND-at-ARC-2" verdicts +… | L390 |
| `FTD-0233` | CLOSED_NEGATIVE | Determinant Grading Pre-Reg & Audit | L399 |
| `FTD-0234` | UNDERDETERMINED | Odd Period Pre-Reg & Audit | L400 |
| `FTD-0235` | UNDERDETERMINED | Det Identity Pre-Reg & Audit | L401 |
| `FTD-0238` | PRE_REGISTRATION, SYNTHESIS | ARC-A1 v2 boundary-closure pre-registration + commutativity-wall synthesis | L404 |
| `FTD-0239` | UNDERDETERMINED | ARC-A1 v2 boundary-closure execution | L405 |
| `FTD-0240` | OPEN, THEOREM | detdet_ζ identity attack scope (MC-T4.3 hinge) — v1 | L406 |
| `FTD-0242` | SMC, DERIVED, THEOREM, OPEN, FOUNDATIONAL_OBSTRUCTION | Route-invariance of the MC-T4.3 operator-assembly boundary | L410 |
| `FTD-0243` | THEOREM, CLOSED_NEGATIVE | RSI Leg 3 conditional theorem + operator-assembly independence | L407 |
| `FTD-0244` | THEOREM | Operator calculus axiomatization & K-BIND resolution | L408 |
| `FTD-0284` | DERIVED, UNDERDETERMINED, SMC, FOUNDATIONAL_OBSTRUCTION | The D=3 FORCED-escape: a real swing at α — the complex-structure branch of the RSI Leg-3 escape is CLOSED; the residual is W-CRIT-2 | L444 |
| `FTD-0285` | INVALID | Alpha no-alpha engine probe — finite-protocol absolute Phase-G gate invalidated | L445 |
| `FTD-0286` | UNKNOWN | Alpha estimator validation — v1 ENERGY_FUNCTIONAL_MISMATCH; v2 HALF_ENERGY_GATE_CONFIRMED_MATCHED | L446 |
| `FTD-0313` | THEOREM, FOUNDATIONAL_OBSTRUCTION, SMC | The BCC-stencil sub-route of MC-T4.3: G* is a pure body-diagonal lattice self-energy — does routing EM onto that sublattice force α? (the geometric… | L459 |
| `FTD-0314` | CLOSED_NEGATIVE, THEOREM, OPEN, SMC, FOUNDATIONAL_OBSTRUCTION | Can the α-binding axiom W be EARNED from native substrate structure? — the carrier-narrowing theorem + three carrier closures (attacks MC-T4.3's… | L460 |
| `FTD-0315` | AXIOM, THEOREM, DERIVED, OPEN, SMC, FOUNDATIONAL_OBSTRUCTION | FC-W (the constitution's FC-4) — the framework's declared α-selection commitment: adopt the external binding law W whose exact content is pinned by… | L461 |
| `FTD-0791` | RETAG, WITHDRAWN, CORRECTION | Does FTD-0319's uniqueness scan establish that the master quadratic is structure rather than coincidence? | L944 |
| `FTD-0792` | ENGINE_FACT, CLOSED_NEGATIVE, CONJECTURE, THEOREM | Does the engine actually run on the derived master-quadratic root? | L945 |
| `FTD-0793` | REFUTATION, EXACT, OBSERVATION | Is `G*` a normalization (a total mass), with the master quadratic as the sole exception? | L947 |
| `FTD-0794` | REFUTATION, EXACT, TAUTOLOGY, RETAG | Does the minimum lattice select the quartic clock law (chain link 4, the last unwithdrawn bridge)? | L949 |

---

## QCD, colour & electroweak

*Confinement, colour charge and singlets, SU(3)/Z3 structure, hadrodynamics, electroweak rank, generations, no-4th-generation.*

**15 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0018` | STRUCTURAL_PARAMETRIC | sin²θ_W = 3/13 = 0.2308 at M_Z scale (CODATA M_Z value 0.22290(30); 3.5% off) | L237 |
| `FTD-0019` | PARAMETRIC | sin²θ_13 = 1/52 | L238 |
| `FTD-0020` | STRUCTURAL_PARAMETRIC | α_s = 7/59 | L239 |
| `FTD-0021` | STRUCTURAL_PARAMETRIC | PMNS angles (sin²θ_12, θ_23, Δm²) | L240 |
| `FTD-0025` | SELECTION | Confinement σ = 0.209 from area-law Wilson loops at x₋ | L244 |
| `FTD-0026` | SELECTION | Einstein equations from Deser bootstrap | L245 |
| `FTD-0027` | SELECTION | Cyclotomic Hamiltonian parameters (Φ_4, Φ_1·Φ_2, Φ_6) | L246 |
| `FTD-0028` | THEOREM | Moore Layer Theorem (gauge groups + 3 generations) | L247 |
| `FTD-0029` | SELECTION | BCC multiplicative structure (W₃ + SU(3) from same eigenvalue) | L248 |
| `FTD-0194` | THEOREM | Branch holonomy gap on a periodic torus — λ_min = 4 sin²(π/(2N)) for the Z₂-twisted ring Laplacian | L363 |
| `FTD-0195` | THEOREM | Z₃ color-center closure: ∑c_i ≡ 0 (mod 3) characterises the singlet subsector + center projector P₀ = (1/3)(I+Z+Z²) | L364 |
| `FTD-0196` | CANDIDATE_RECONSTRUCTION | Generation graph Γ_F(d) — CKM-shape overlap matrix from a K₃ triangle on (q*^{d+1}, 1, q*^d) with phase φ=π+π/d | L365 |
| `FTD-0223` | SPEC | FTD Dynamical SU(3) Hadrodynamics | L376 |
| `FTD-0224` | CLOSED_RESOLVED | Color Excess closed form & Blocked Effective Action Flow | L377 |
| `FTD-0400` | NO_GO | Confinement-energy → rest-mass → gravity bridge audit: does one current-engine energy–momentum object generate the colour force, inertia, and… | L545 |

---

## Gravity & cosmology

*Newton's law from the substrate, graviton/spin-2 provenance, Kerr-Newman, strong-field signatures, Lambda, dark matter.*

**31 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0035` | OPEN | Mechanism γ — gravitational a_phys derivation | L254 |
| `FTD-0130` | PARTIAL, OPEN | Calibration architecture audit: K_B role decoupling + G\_N substrate-derivation needed (open) | L348 |
| `FTD-0131` | DERIVED, SMC, THEOREM, HYPOTHESIS, AXIOM | Newton's law of gravity derived from FTD substrate (resolution of FTD-0130 path-(a)); falsification of the "G\_N = 1/(b\_3+N\_c)² = 1/100"… | L349 |
| `FTD-0184` | CLOSED_NEGATIVE | FQCR parallel-track gravity ontology — red-team outcome | L231 |
| `FTD-0189` | AUDIT_FINDING, CONJECTURE, THEOREM, SELECTION | Step-0 graviton-provenance audit — FTD's massless spin-2 field h_μν is posited, not derived | L358 |
| `FTD-0190` | PRE_REGISTRATION, CLOSED_NEGATIVE | Finite neutral lock (Q10) — finite-closure SM-shadow audit (pre-registered + executed) | L360 |
| `FTD-0191` | PRE_REGISTRATION, AUDIT_FINDING | Colour-singlet rank (Q11) — electroweak-rank audit (pre-registered + executed) | L361 |
| `FTD-0192` | PRE_REGISTRATION, AUDIT_FINDING | Weak-SU(2) provenance (Q12) — weak-SU(2) provenance audit (pre-registered + executed) | L362 |
| `FTD-0193` | CLOSED_NEGATIVE | Frontier 4 Step 4a-ii canonical engine measurement: no emergent spin-2 substrate mode in the probed regime — Outcome B | L359 |
| `FTD-0208` | CLOSED_NEGATIVE, AXIOM, THEOREM | Clock-hypothesis substrate-derivation — Arc B P2 of Wilsonian-reframe plan v2 | L379 |
| `FTD-0209` | THEOREM, SMC | Spin-2 boundary theorem free-theory + canonical-toggle scope — Arc C2 P4 of Wilsonian-reframe plan v2 | L378 |
| `FTD-0211` | UNDERDETERMINED | W5 Moore-shell DM weighting confirmation | L369 |
| `FTD-0213` | CLOSED_NEGATIVE | FTD native strong-field gravity signature | L371 |
| `FTD-0214` | CLOSED_RESOLVED | QFT/GR Bridge Consolidation — four bridge gap resolutions | L372 |
| `FTD-0229` | CLOSED_RESOLVED | Kerr-Newman Black Hole Derivation, Limits & Extensions | L396 |
| `FTD-0331` | SELECTION, OPEN, BOUNDARY, DERIVED, THEOREM, SMC, FOUNDATIONAL_OBSTRUCTION | The cosmological constant as a scale-covariant holographic ratio — a mechanism for the *smallness* of Λ, not its value; replaces the α¹⁶/α⁵⁷… | L476 |
| `FTD-0332` | RECONCILIATION, DERIVED, SELECTION, OPEN, BOUNDARY, PARAMETRIC, CONJECTURE, SUPERSEDED, THEOREM, SMC, FOUNDATIONAL_OBSTRUCTION | Corpus-wide cosmology-sector reconciliation — make FTD-0331 (Λ) canonical and remove the live cosmology contradictions | L477 |
| `FTD-0333` | MEASURED, CONDITIONAL, SMC, FOUNDATIONAL_OBSTRUCTION | FTD-0270 P2 mass-gap swing — first canonical run INVALID (instability + flooding); strong no-gap hint; boundary unchanged | L479 |
| `FTD-0334` | RECONCILIATION, PARAMETRIC, OPEN, BOUNDARY, CONJECTURE, SELECTION, SMC, FOUNDATIONAL_OBSTRUCTION | Deferred-layer cosmology cleanup — propagate FTD-0331/0332 into the papers/web/intuition/index layer | L478 |
| `FTD-0338` | RECONCILIATION, OPEN, BOUNDARY, CLOSED_NEGATIVE, DERIVED, PARAMETRIC, SELECTION, SMC, FOUNDATIONAL_OBSTRUCTION | Manuscript cosmology propagation — completes the FTD-0334 deferral | L483 |
| `FTD-0364` | SYNTHESIS, BOUNDARY, OPEN, DERIVED, SELECTION, SMC, FOUNDATIONAL_OBSTRUCTION | Λ source-gap L-scan feasibility — the engine route to closing FTD-0331's source gap is boundary-limited; `campaign_vacuum_energy.cpp` superseded | L509 |
| `FTD-1013` | THEOREM, SELECTION, IMPOSED | Does a degree-1 test-voxel action in external \(\mathcal{L}\) exist (and unique in a locked class) such that UFF is its EOM given FC-2, reducing to… | L1151 |
| `FTD-1014` | CLOSED_NEGATIVE, IMPOSED | On a locked external-well fixture, extra forces off, several test-body sizes \(N\), does the live `phase_forces` gravity update reproduce Q0’s weak… | L1152 |
| `FTD-1015` | CLOSED_NEGATIVE | Linearize spatial clock-transport \(\Omega\) with the SCOPE’s kinematic constraints and no action: is the residual little-group content exactly two… | L1153 |
| `FTD-1016` | MEASURED, SELECTION, IMPOSED | On the FTD-1014 prescribed-well fixture, extra forces off, several test-body sizes \(N\), does a default-off production operator \(F=M_{\rm INERTIAL}… | L1154 |
| `FTD-1017` | MEASURED | After a CPU tick that runs `latency_field` Poisson on a heavy locked source alone, does one subsequent `phase_forces` with `geometric_gravity` on a… | L1155 |
| `FTD-1018` | MEASURED, SELECTION | On the FTD-1016 prescribed-well fixture, extra forces off, one unlocked rest voxel, does native CUDA `phase_forces` with `geometric_gravity`… | L1156 |
| `FTD-1019` | MEASURED | After a CPU tick that runs `latency_field` Poisson on a heavy locked source alone, freeze \(\mathcal{L}\): do rest clocks at a near and far site… | L1157 |
| `FTD-1020` | MEASURED | After a CPU tick that runs `latency_field` Poisson on a heavy locked source alone, freeze \(\mathcal{L}\), clear all matter, and launch a… | L1158 |
| `FTD-1021` | MEASURED | After FTD-1017 Step S, if Poisson is re-solved with the light probe present, does FTD-1016 still match \(g=C^2\mathcal{L}\nabla\mathcal{L}\) of the… | L1159 |
| `FTD-1022` | MEASURED | On the FTD-1017 sourced well, with a locked 3³ probe (COM at the FTD-1021 site), does freeze≈live return, and does FTD-1016 match the member-mean Q0… | L1160 |

---

## Engine infrastructure & RG

*Langevin/thermostat, operator-mixing matrices, RG flow and blocking, the bridge-contract gates, Ward identities, GPU/CUDA ports and parity.*

**39 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0033` | HYPOTHESIS | Type III₁ classification of FTD flux algebra | L252 |
| `FTD-0034` | RESOLVED | Engine convergence to QED in L → ∞ limit (EFT campaign) | L253 |
| `FTD-0051` | INFRASTRUCTURE | Langevin thermostat on wave_vel (OU noise; CPU + GPU single-substrate) | L270 |
| `FTD-0052` | NOT_PURSUED | s-field stochastic dynamics (ternary Metropolis for thermal ensemble) | L271 |
| `FTD-0053` | MEASURED | α_eff L=256 T=0 scaling data point | L272 |
| `FTD-0054` | OPEN | Thermal α via shared thermal background (measure_alpha_eff refactor) | L273 |
| `FTD-0055` | MEASURED | BCC tadpole at N=4096 on GPU (Priority 1 of external GPU plan) | L274 |
| `FTD-0056` | THEOREM | Unrenormalized one-loop BCC tadpole residual has no continuum limit | L275 |
| `FTD-0057` | MEASURED | Non-perturbative HMC measurement of ⟨η⟩ on BCC lattice (Priority 2) | L276 |
| `FTD-0058` | CLOSED_NEGATIVE | Structure-2 Ward-valid two-U(1) scalar gauge completion | L277 |
| `FTD-0059` | THEOREM | No-go theorem for `a_phys` derivation from Axiom Zero | L278 |
| `FTD-0064` | POSITIVE | Gate 1 of the bridge contract: state/flux field dictionary with frozen scaling dimensions | L283 |
| `FTD-0065` | POSITIVE | Gate 4 of the bridge contract: engine transport ledger extended to full Moore-26 routing | L284 |
| `FTD-0066` | POSITIVE | Gate 5 of the bridge contract: per-toggle reaction-aware Ward identity | L285 |
| `FTD-0067` | POSITIVE | Mixed-toggle multi-tick Ward identity + first non-Gaussian flow data (Gates 4 + 5) | L286 |
| `FTD-0068` | POSITIVE | Gate 3 of the bridge contract: complete d≤6 operator basis | L287 |
| `FTD-0069` | POSITIVE | Gate 2 of the bridge contract: FTD native Langevin ensemble as nonlinear stationary generator | L288 |
| `FTD-0070` | MEASURED | Phase-2 multi-scale RG flow: Gaussian fixed point confirmed at b ∈ {1,2,4,8} | L289 |
| `FTD-0090` | MEASURED | Ward-identity status: engine SOR projector saturates at ~1% of $\|J\|_{\max}$ (stencil mismatch); matched-stencil CG projector closes Ward to ≤ 1e-8 | L297 |
| `FTD-0091` | PARTIAL | Operator-spectrum scaling-dimension classification (relevant/marginal/irrelevant) — Phase 3 closure read | L331 |
| `FTD-0092` | DERIVED | Lorentz-anisotropy quantitative exponent: $\delta(k) \propto k^4$, $R^2 = 1.000000$ — strongly Wilson-irrelevant | L332 |
| `FTD-0098` | PARTIAL | First measured native operator-mixing matrix M_ab(b=2) on Langevin+genesis ensemble | L330 |
| `FTD-0099` | PARTIAL | Multilatitude (L=16 vs L=32) + b=4 RG semigroup + Wilson eigendecomposition follow-ups to FTD-0098 | L329 |
| `FTD-0100` | PARTIAL | F2 closure: first full 6×6 native operator-mixing matrix M_ab(b=2) — s² zero-variance degeneracy broken via injection-amplitude calibration | L328 |
| `FTD-0101` | MEASURED | L-dependence of FTD-0100's boundary-injection calibration: stretch combination of F1 + F2 (L=32 with inj-mult=1.0) reveals injection density falls… | L327 |
| `FTD-0102` | PARTIAL | First engine-as-instrument measurement: emergent phase structure from generic initial conditions at L=32 | L326 |
| `FTD-0103` | PARTIAL | Continuum-limit verification at L ∈ {16, 32, 64} for operator-mixing matrix M_ab(b=2) | L324 |
| `FTD-0104` | PARTIAL | Topological observable mapping: Wilson loops, flux tubes, monopoles, vacuum instantons under shared schema at L=32 | L325 |
| `FTD-0105` | PARTIAL | Lemniscatic replacement for the 2-sphere in Einstein/thermodynamics formulas — pre-registered horizon-area test | L323 |
| `FTD-0106` | HYPOTHESIS | G\*/π asymmetry scan across three Tier-1 domains (time-direction/dissipation, Coulomb scattering phase, Hawking evaporation timescale)… | L322 |
| `FTD-0107` | PARTIAL | Emergent-spectrum G1 follow-up: L=64 multilatitude rerun confirms deterministic cluster counts | L321 |
| `FTD-0201` | METHODOLOGICAL_CLARIFICATION, THEOREM | Phase J ultralocality (Theorem 7) honest retag from `[THEOREM at L=2] + [CONJECTURE for general L]` to… | L386 |
| `FTD-0747` | CLOSED_NEGATIVE | Does the first device-resident CUDA port reproduce the three-ray causal-horizon CPU record and physics conjunction? | L890 |
| `FTD-0748` | CLOSED_NEGATIVE | Does canonical net oriented-face support repair FTD-0747's representation-dependent current gate and yield a uniform three-ray CUDA replay? | L891 |
| `FTD-0749` | CONSTRUCTIVE | Does collision-free unique-face deposition make long CUDA trajectories deterministic and CPU-identical? | L892 |
| `FTD-0750` | CLOSED_NEGATIVE | Do ordered raw current addition and deterministic regional reductions recover the exact CUDA replay and uniform CPU prefix? | L893 |
| `FTD-0751` | NUMERICAL_FACT | At what exact stage does the remaining CPU/CUDA trajectory divergence first appear? | L894 |
| `FTD-0752` | NUMERICAL_FACT | Does a separately compiled explicit-rounding CUDA backend close the bounded dynamic parity gate? | L895 |
| `FTD-0759` | INFRASTRUCTURE | Can the M3 matter/root/observer pipeline remain device-resident with qualified CPU parity and utilization? | L902 |

---

## Engine emergence campaigns

*Fermion-emergence phases, genesis/evaporation and thermal phase maps, atomic and bound-state spectra, wave sectors and dispersion.*

**37 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0004` | THEOREM | Phase G emergent Coulomb at every finite L | L188 |
| `FTD-0005` | THEOREM | Phase J partition-function ultralocality at L=2 | L189 |
| `FTD-0071` | CLOSED_NEGATIVE | Phase-4 fermion-emergence alt-routes on 2³ block: universal Clifford falsification | L290 |
| `FTD-0072` | CLOSED_NEGATIVE | Phase-4c fermion-emergence on Moore-26 / 3³ block: axial-dipole Clifford falsification | L291 |
| `FTD-0073` | THEOREM | Phase-4e spin-field readout: mode-preserving commutative algebra (not Clifford) + mode-erasure theorem capstone | L292 |
| `FTD-0074` | CLOSED_NEGATIVE | Phase-4f flux 1-form (link) readout: separable-tensor algebra, not Clifford | L293 |
| `FTD-0075` | MEASURED | Phase-4g flux propagator on Langevin ensemble: long-range-ordered / ultralocal, neither bosonic vector nor fermionic | L294 |
| `FTD-0076` | MEASURED | Phase-4h material emergence: smallest spontaneously-emergent particle from the lattice is a single-voxel colored quark, not an electron | L295 |
| `FTD-0077` | MEASURED | Phase-4i color binding + SU(3) structure + m_e promotion audit | L296 |
| `FTD-0078` | SYNTHESIS | Phenomenal/Noumenal Bridge foundation: two-layer ontology geometrically encoded as 2³/3³ blocks | L344 |
| `FTD-0085` | PARTIAL | Program F: link-bilinear fermion probe — first non-commutative algebraic structure detected in FTD native dynamics | L337 |
| `FTD-0086` | MEASURED, STRONG_POSITIVE | Program F-prime: plaquette bivector emergence — Cl(3,0) bivector matching signature on FTD non-local dynamics | L336 |
| `FTD-0087` | MEASURED, PARTIAL | Program F-double-prime: bivector closure tests — F-prime matching signature robust but full Cl(3,0) Lie closure fails at 4-injection scale | L335 |
| `FTD-0088` | MEASURED, POSITIVE | Path 1: Cl(3,0) multi-grade decomposition — 12/12 grade-structure tests pass at 2-injection order | L334 |
| `FTD-0089` | STRUCTURAL, THEOREM | A1 + A2: Dirac-Kähler structural identification + Cl(3,0) mass-ratio no-go | L333 |
| `FTD-0125` | DERIVED | Phase I FTD-native coupling: derivation [DERIVED]; engine cross-check OUTCOME C (pre-registered hypothesis FALSIFIED on engine — V(r) does not carry… | L306 |
| `FTD-0126` | UNKNOWN | Phase II Wilson-Dirac matter sector: II.2 implementation CLOSED at machine precision + II.3-II.5 measurement campaign CLOSED with OUTCOME C… | L305 |
| `FTD-0236` | CLOSED_RESOLVED | Ginsparg-Wilson & Overlap Fermion Relation & Index Theorem | L402 |
| `FTD-0270` | MEASURED, OPEN, SELECTION, PARAMETRIC, SMC, FOUNDATIONAL_OBSTRUCTION | Lattice quantization & the atomic-dispersion boundary: the substrate quantizes but with the WRONG dispersion for atomic spectra | L435 |
| `FTD-0271` | DERIVED, IMPOSED, SELECTION, MEASURED, SMC | The de Broglie internal clock: GIVEN a rest-mass clock, FTD's flux is a single-particle pilot wave (de Broglie matter wave + Schrödinger envelope) | L436 |
| `FTD-0272` | MEASURED, SMC | Order of the genesis transition (RG-spectrum probe): is the cluster-mass ladder an RG-derived spectrum? — verdict FIRST-ORDER | L437 |
| `FTD-0273` | MEASURED, SMC | Mass as flux-energy in flip-quanta + quark quantization — verdict ENERGY-COLLAPSES-TO-N + COLOR-PHENOMENA-IMPOSED | L438 |
| `FTD-0274` | MEASURED, DEFINITION, HYPOTHESIS, CONJECTURE, SMC | Min/max temperature + ignition map of the lattice — verdict: floor (condensation T_up) but NO ceiling | L439 |
| `FTD-0275` | MEASURED, DEFINITION, CONJECTURE, SMC | Thermal phase map of the lattice, run of record — T_up(L) RISES + safety-valve FALSIFIED + near-critical spark DETONATES | L440 |
| `FTD-0276` | CLOSED_NEGATIVE, MEASURED, SMC | Kinetic-drain scaling + N(A) friction-knob map + drain-derivation attempt — drain² origin CLOSED NEGATIVE; drain/γ are engine-tuning constants | L441 |
| `FTD-0277` | CLOSED_NEGATIVE, OPEN | Collective-coordinate genesis-counting model v1 for the current-stack N(A) law | L451 |
| `FTD-0278` | OVERCLAIM | Hydrogen-like bound-state spectrum on the FTD lattice — verdict HYDROGEN-1s-CONFIRMED (CORRECTED from HYDROGEN-CONFIRMED; given the clock +… | L442 |
| `FTD-0279` | DERIVED, THEOREM, SMC | Helium on the FTD lattice — mean-field SCF, verdict HELIUM-CONFIRMED (given clock + coupling + mode-occupancy) | L443 |
| `FTD-0298` | SYNTHESIS, BOUNDARY, DERIVED, OBSERVATION, SELECTION, OPEN, SMC | Lattice wave sectors synthesis — light + radio = one flux-wave sector; FTD has NO acoustic/phonon sector (structural boundary) | L447 |
| `FTD-0299` | MEASURED, BOUNDARY, SMC | Lattice wave sectors run of record — light dispersion atlas LIGHT-CONFIRMED + condensate-compression probe NULL | L448 |
| `FTD-0316` | DERIVED, RETRACTED | Does a coherent flux drive show a sharp genesis ignition threshold and subsequent crest regulation? | L462 |
| `FTD-0317` | EMERGENT, SMC, FOUNDATIONAL_OBSTRUCTION | Does spatial information do "creative work" in FTD genesis? — coherent vs scrambled disposition at fixed energy + `\|J\|` histogram | L463 |
| `FTD-0337` | RECONCILIATION, SMC, FOUNDATIONAL_OBSTRUCTION, MEASURED | FTD-0308 mechanism correction — bare-wave leapfrog discretization, not a parametric KG-well instability | L482 |
| `FTD-0362` | MEASURED, SMC, FOUNDATIONAL_OBSTRUCTION, CONDITIONAL | Native mass-gap swing v2 — CLOSED-NEGATIVE: the nonlinear genesis↔Gauss loop generates no rest-mass gap | L507 |
| `FTD-0363` | MEASURED, OPEN, SMC, FOUNDATIONAL_OBSTRUCTION, AXIOM | GNC-w discriminator (Q_ij on locked Gauss-dressed clusters) v1 — INVALID: instrument defect, re-scope to v2; GNC-w stays [OPEN] | L508 |
| `FTD-0379` | CLOSED_NEGATIVE, MEASURED | Vertex program M1 — does engine evolution satisfy the Dirac–Kähler equation on the local grade fields? | L524 |
| `FTD-0380` | REFUTATION, SELECTION | Vertex program M2 — is the FTD-0087 su(2) closure failure dynamical noise? | L525 |

---

## Lorentz recovery & causal structure

*The discrete flux pole, anisotropy exponents, the common cone, CFL and causal normalisation, preferred-frame operators, anisotropic-QED RG.*

**37 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0401` | NO_GO | Derived lattice speed versus legacy `c=1` matter clock and fused mass roles | L546 |
| `FTD-0402` | PARTIAL | Causal normalization and explicit mass-role reconciliation in the current raw-lattice engine | L547 |
| `FTD-0403` | THEOREM, OPEN | Targeted causal-normalization dependency closure | L548 |
| `FTD-0404` | THEOREM, OPEN | Volumetric measure reconciliation: does three-dimensionality require cubing local energy formulas or an explicit cubic integration measure? | L549 |
| `FTD-0405` | NO_GO | Native Confinement Energy–Momentum Contract feasibility: can the current RenderBridge colour pair force satisfy NCEMC-1–4 without a new law… | L551 |
| `FTD-0406` | SCOPE_CONTRACT, SELECTION, IMPOSED | Owner-authorized strong stress–energy contract v1: can explicit selected choices close the FTD-0405 work/zero/localization obstructions on a frozen… | L552 |
| `FTD-0407` | NO_GO, THEOREM, OPEN, AXIOM | Lorentz-recovery hard audit: full discrete flux pole, exact production CFL interval, nearest-Moore stability/improvement obstruction, constructive… | L553 |
| `FTD-0408` | THEOREM | P4-preserving period-two Lorentz prototype: can two legal nearest-Moore ticks generate the radius-two effective counterterm without a radius-two… | L554 |
| `FTD-0409` | POSITIVE, CONSTRUCTIVE, OPEN | Fixed-cone gate: can a minimal P4-local temporal or auxiliary architecture cancel the dimension-six pole while retaining the live `c²=1/3` cone? | L555 |
| `FTD-0410` | THEOREM, NO_GO, OPEN, SELECTION | Can the Gauss arithmetic-geometric mean derive the FTD light cone rather than merely supply another selected speed? | L556 |
| `FTD-0411` | THEOREM, POSITIVE, OPEN | Can the Moore layers define two domains, with BCC as temporal return structure and SC+FCC as physical-space propagation? | L557 |
| `FTD-0412` | CORRECTION, NO_GO, OPEN | Common-cone gate after the BCC-time construction, including correction of the Wilson real-time operator | L558 |
| `FTD-0413` | DERIVED, OPEN | Can a nearest-Moore face-diagonal Wilson kinetic stencil escape the FTD-0412 scalar-r q4 common-cone obstruction? | L560 |
| `FTD-0414` | CORRECTION, DERIVED, CONDITIONAL, OPEN | If exact all-orders Lorentz symmetry is not required, what empirical infrared envelope does the selected free flux/matter cone obey? | L561 |
| `FTD-0415` | THEOREM, DERIVED, OPEN | Do FTD's declared exact symmetries forbid radiative generation of lower-dimensional preferred-frame kinetic operators? | L562 |
| `FTD-0416` | DERIVED, OPEN | Can standard anisotropic-QED RG attraction erase the preferred-frame threshold left open by FTD-0415? | L564 |
| `FTD-0417` | THEOREM, DERIVED, OPEN | Can the photon/flux sector be made genuinely local without the nonlocal Helmholtz map `A=P_T J`? | L565 |
| `FTD-0418` | THEOREM, DERIVED, OPEN | Can the FTD-0417 local photon be paired with one discrete spacetime matter regulator whose action fixes every vertex needed for one-loop matching? | L566 |
| `FTD-0419` | NUMERICAL_FACT, THEOREM, CLOSED_NEGATIVE, OPEN | Does the bare FTD-0417/0418 leading common cone remain matched after the complete one-loop full-Brillouin-zone correction? | L567 |
| `FTD-0420` | PRE_REGISTRATION, SCOPE_CONTRACT | Can the native-first Lorentz recovery cycle be frozen before any new measurements? | L568 |
| `FTD-0421` | THEOREM, CLOSED_NEGATIVE | Does the frozen production event algebra admit a nontrivial additive source-free native charge over the preregistered discrete feature basis? | L569 |
| `FTD-0422` | NOT_EXECUTED | Do frozen native histories supply a charged manifested pole and a common low-energy cone? | L570 |
| `FTD-0423` | NOT_EXECUTED | Does native blocking drive every dimension-four preferred-frame operator toward the common-cone surface? | L571 |
| `FTD-0424` | SCOPE_CONTRACT, CLOSED_NEGATIVE, OPEN | Can one universal dimension-four relative-cone counterterm control the auxiliary local-link EFT without sector or threshold retuning? | L572 |
| `FTD-0425` | THEOREM, EXACT, OPEN | Is the frozen tick compatible with emergent low-energy unitarity? | L573 |
| `FTD-0810` | DERIVED, CLOSED_NEGATIVE, OPEN | What characterizes the cone speed `C = 1/√3`, and is it forced? | L972 |
| `FTD-0811` | DERIVED, MEASURED | What is the causal cell of the M18 stencil in three dimensions, and how does isotropy restore? | L973 |
| `FTD-0812` | DERIVED, MEASURED, EXACT | Does the limiting speed depend on the rest mass? | L974 |
| `FTD-0813` | DERIVED, MEASURED | Does a bound composite inherit its constituents' cone or its own total mass's? | L975 |
| `FTD-0815` | DERIVED, OPEN | The two owed proofs: does operational hiding follow, and does Lorentz recovery close? | L977 |
| `FTD-0816` | DERIVED, MEASURED, WITHDRAWN, SCOPE_CONTRACT | Can two sectors of the model be given a common cone, and what does it cost? | L978 |
| `FTD-0819` | DERIVED, OPEN | Which sublattice does a common-cone matter carrier actually need, and what does that cost Postulate 1? | L981 |
| `FTD-1003` | THEOREM, CORRECTION, OPEN | Can a symmetry-protected embedded mode evade the C2 band-clearance blocker on the composite-boost row? | L1141 |
| `FTD-1009` | MEASURED, DERIVED, OPEN | Does a genuinely two-body bound state (φ⁴ bion — two kinks + binding, one energy functional) dilate as the adopted clock law requires? | L1147 |
| `FTD-1010` | MEASURED, OPEN | Does the data-selected deviation model close the two-body bion dilation campaign, and what is the surrogate line's terminal state? | L1148 |
| `FTD-1011` | DERIVED, REFUTATION, OPEN | At what order in (ka) can an internal observer of the one-functional wave sector detect the substrate frame? | L1149 |
| `FTD-1012` | MEASURED, OPEN | Does the bath-frame (Rayleigh damping) second-category term break universality as the Functional Census predicted? | L1150 |

---

## Charge, Gauss & native EM emergence

*Native additive charge, Gauss projection and dressing, face-current sidecars, longitudinal susceptibility, dressed hazards, monopoles.*

**15 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0009` | THEOREM | Charge conservation per tick | L193 |
| `FTD-0114` | DERIVED | Exact lattice Bianchi identities on FTD's vertex-centered stencil family | L313 |
| `FTD-0115` | DERIVED, THEOREM, OPEN | Corrected native moving-source response | L312 |
| `FTD-0120` | THEOREM, OPEN, CONDITIONAL | Corrected moving-source extensions Q5/Q6/Q7/Q8 | L315 |
| `FTD-0426` | MEASURED, CLOSED_NEGATIVE | Does production separation of primitive polarity generate an operational static charge in the native flux field? | L574 |
| `FTD-0427` | THEOREM, MEASURED | Can a local matched face-current rule transport an effective Gauss source without repeated projection? | L575 |
| `FTD-0428` | THEOREM, MEASURED | Can the selected matched face-current mechanism become one live longitudinal/transverse engine state without projection? | L576 |
| `FTD-0429` | DERIVED | Does primitive polarity dynamically acquire a finite infrared closed-flux normalization in the frozen native engine? | L577 |
| `FTD-0430` | DERIVED | Does an actual production movement event transport the native infrared polarity susceptibility with causal support and the same on-shell pole? | L578 |
| `FTD-0431` | INVALID | Does the FTD-0429/0430 coarse polarity source retain a reaction-bearing infrared slow mode when native evaporation is enabled? | L579 |
| `FTD-0432` | DERIVED, MEASURED | Does the exact production evaporation hazard explain the non-exponential polarity history observed by FTD-0431? | L580 |
| `FTD-0433` | MEASURED | Does the exact dressed production hazard of one fixed axial fundamental source decrease toward the infrared at its native-pole first antinode? | L581 |
| `FTD-0563` | THEOREM, CLOSED_NEGATIVE | Can a finite microscopically neutral source possess a true Gauss monopole, thereby evading the charged rigid-source obstruction? | L708 |
| `FTD-0564` | THEOREM, CLOSED_NEGATIVE | Can a nonzero normalized-flux hedgehog degree by itself force or quantize electric Gauss charge? | L709 |
| `FTD-0641` | THEOREM, MEASURED | Does the matched oriented-face electric / oriented-edge magnetic state possess its own divergence-free propagating modes, with phases fixed by the… | L784 |

---

## Common-action mechanics & reciprocity

*Forces, work and recoil for a hop; the worldline/Legendre action; charts, collisions and quotients; energy closure and Peierls barriers.*

**146 claims.**

### Common-action mechanics & reciprocity — forces, work & reciprocity

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0434` | CLOSED_NEGATIVE | Does the canonical `s0-vacuum-photon` initializer propagate as the photon packet claimed by its scenario documentation and dashboard label? | L582 |
| `FTD-0435` | MEASURED, CLOSED_NEGATIVE | Does the selected production `G_C s grad\|J\|` recoil have the polarity symmetry and scaling of an electric light-matter force? | L583 |
| `FTD-0436` | MEASURED, CLOSED_NEGATIVE | Does the selected flux-gradient force polarize or translate a neutral `+1/-1` pair under a transverse travelling wave? | L584 |
| `FTD-0437` | MEASURED, CLOSED_NEGATIVE, RESOLVED | Is the wave-free neutral-pair drift a dipole-oriented physical-force defect, lattice scan bias, or injection-history artifact? | L585 |
| `FTD-0438` | MEASURED, CLOSED_NEGATIVE | Does the native central-generator flux momentum compensate the selected force's isolated-pair self-propulsion? | L586 |
| `FTD-0439` | MEASURED | Is the isolated-pair reciprocity failure specific to the selected magnitude-gradient force, shared by direct flux forces, or present in the common… | L587 |
| `FTD-0440` | MEASURED, INVALID | Is the FTD-0439 Poisson pair-momentum leak a cold-start SOR transient or a converged static reciprocity floor? | L588 |
| `FTD-0441` | MEASURED | Is the FTD-0439 Poisson trajectory leak stored cold-start solver error even when neither source voxel hops? | L589 |
| `FTD-0442` | UNKNOWN | Does the legacy divergence-gradient production force implement the particle Euler–Lagrange variation of the declared FTD coupling action? | L590 |
| `FTD-0443` | THEOREM, MEASURED, CLOSED_NEGATIVE | What force/work statement follows exactly from the site-valued interaction, and does production implement it? | L591 |
| `FTD-0444` | THEOREM, RETRACTED | Does exact scalar hop work uniquely determine force, particle momentum, and local field recoil? | L592 |
| `FTD-0445` | THEOREM, MEASURED | Does a primitive edge/corner Moore hop uniquely induce the selected oriented-SC-face current history? | L593 |
| `FTD-0446` | THEOREM | Can the three-component flux vector uniquely encode current on all 13 unoriented Moore channels? | L594 |
| `FTD-0447` | THEOREM | Does native cubic symmetry remove FTD-0444's transverse force ambiguity for an isolated hop? | L595 |
| `FTD-0448` | THEOREM | Does cubic covariance determine whether longitudinal field recoil belongs to the departure site, arrival site, or both? | L596 |
| `FTD-0449` | MEASURED, CLOSED_NEGATIVE | Does the production movement event apply the exact endpoint action work, and can the event journal reconstruct its mechanics? | L597 |
| `FTD-0450` | CORRECTION, THEOREM, CONSTRUCTIVE | Did FTD-0444's selected reversible map use the production flat energy-momentum relation? | L598 |
| `FTD-0451` | CONSTRUCTIVE, OPEN | Can a local half-tick Moore-link record close and reverse the corrected particle work/recoil exchange? | L599 |
| `FTD-0452` | THEOREM, CORRECTION | What energy actually closes a fixed-field hop, and do the engine's named energy diagnostics measure the production wave Hamiltonian? | L600 |
| `FTD-0453` | THEOREM, CLOSED_NEGATIVE | Can any fixed-J update of the entire native wave-velocity field carry the selected recoil at zero exact tick-energy cost? | L601 |
| `FTD-0454` | THEOREM, CLOSED_NEGATIVE | Does the zero-energy recoil obstruction survive the production-ordered simultaneous update `Delta J=Delta W=S`? | L602 |
| `FTD-0455` | CONSTRUCTIVE, MEASURED, OPEN | Can a pre-existing exact travelling mode make the paired recoil transaction simultaneously energy- and momentum-conserving? | L603 |
| `FTD-0456` | CONSTRUCTIVE, MEASURED, SELECTION | Can the travelling-wave-assisted zero-energy recoil be completed inside a fixed causal neighborhood of the hop? | L604 |
| `FTD-0457` | CONSTRUCTIVE, MEASURED, SELECTION | Can a finite-energy localized source-free packet enable the same exact `R=1` recoil without a volume-growing background? | L605 |
| `FTD-0458` | THEOREM, MEASURED, SELECTION | Does minimum impulse norm uniquely select the existing covariant-null transaction, and is that selector cubic covariant? | L606 |
| `FTD-0459` | CLOSED_NEGATIVE, MEASURED | Does the finite packet support consecutive local conserving transactions when state coupling and production movement cadence act without any packet… | L607 |
| `FTD-0460` | MEASURED, CORRECTION | Which additive field history blocks the FTD-0459 hop: packet, dressing, static polarity source, or velocity-curl source? | L608 |
| `FTD-0461` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does production's source-site flux carry close integer-hop energy, and does the fixed-field endpoint work survive that carry? | L609 |
| `FTD-0462` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does rigid translation of the complete polarity-generated field history remove the self-barrier, and is that event local? | L610 |
| `FTD-0463` | THEOREM, MEASURED, CLOSED_NEGATIVE | Is FTD-0462's changing wave cross energy dominated by the transverse packet or by the selected longitudinal initial dressing? | L611 |
| `FTD-0464` | THEOREM, CONSTRUCTIVE, MEASURED, CLOSED_NEGATIVE | Can a fixed endpoint-local portion of the source-generated field move with the polarity and admit every registered hop without the selected initial… | L612 |
| `FTD-0465` | THEOREM, MEASURED, CLOSED_NEGATIVE | Is FTD-0464's local additive coat translation injective, and does its field momentum balance the particle update? | L613 |
| `FTD-0466` | CONSTRUCTIVE, MEASURED, CLOSED_NEGATIVE | Does an exact injective permutation of the complete 36-site local field support close energy and momentum for the particle hop? | L614 |
| `FTD-0467` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does any current production electric-force branch arise as the matter-side variation of the same interaction that supplies the native `-G_C grad(s)`… | L615 |
| `FTD-0468` | THEOREM, MEASURED, OPEN | Does the existing `-G_C grad(s)` source kick carry the exact opposite field momentum of the common-action matter impulse `+G_C s grad(div J)`? | L616 |
| `FTD-0469` | THEOREM, MEASURED, CORRECTION, OPEN | Does a source-centered common-action kick/drift/kick close shadow energy, total momentum, inversion, and per-particle kinetic accounting for frozen… | L617 |
| `FTD-0470` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does the centered common-action site force integrate to the exact interaction work of a finite face hop, or is an oriented link derivative required? | L618 |
| `FTD-0471` | THEOREM, MEASURED, CLOSED_NEGATIVE | Can the production cell-centered central-divergence field represent the Gauss-source change of one adjacent face hop locally? | L619 |
| `FTD-0472` | THEOREM, MEASURED, SELECTION, OPEN | Does the selected matched face-current layer close an exact finite-step energy transaction, and does a primitive Moore hop determine that transaction… | L620 |
| `FTD-0473` | THEOREM, CORRECTION, MEASURED, CLOSED_NEGATIVE, OPEN | Does the selected matched field possess an exact local translation momentum, and does the FTD-0472 electrostatic hop recoil in it? | L621 |
| `FTD-0474` | MEASURED, CLOSED_NEGATIVE | Are finite manifested structures in the frozen engine maintained by a mechanical membrane, an explicit environment, periodic recirculation, or only… | L622 |
| `FTD-0475` | MEASURED, CLOSED_NEGATIVE, OPEN | Are the travelling vacuum flux lines a co-moving field, a detached wake, or a leading guide for manifested matter? | L623 |
| `FTD-0476` | MEASURED, CLOSED_NEGATIVE | Does a manifested polarity dynamically build a radial/co-moving flux dressing, leave a wake when moved, or release an outgoing field when removed? | L624 |
| `FTD-0477` | MEASURED, CLOSED_NEGATIVE | Can a separate finite flux packet cause an initially resting polarity to become a reciprocal moving source with a co-moving dressing, wake, or… | L625 |
| `FTD-0478` | DERIVED, THEOREM, SELECTION | Can `(site,remainder,polarity)` determine a compact subcell coupling shape and exact straight-segment face current? | L626 |
| `FTD-0479` | CONSTRUCTIVE, CLOSED_NEGATIVE | Can the exact face current, matched fields, and production matter dispersion form one reciprocal implicit transaction? | L627 |
| `FTD-0480` | CLOSED_NEGATIVE | Does the coupled observer qualify over all axis directions, static dressing, polynomial/wave fields, diagonal paths, translations, and cubic… | L628 |
| `FTD-0481` | NOT_EXECUTED | Default-off `common_action_face_dynamics` branch. | L629 |
| `FTD-0482` | NOT_EXECUTED | Reciprocal dressing, wake, and dashboard qualification. | L630 |
| `FTD-0483` | NOT_EXECUTED | Infrared pole, cone, anisotropy, and spectral recovery. | L631 |

### Common-action mechanics & reciprocity — action, worldline & Legendre structure

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0484` | THEOREM, OPEN | Is the exact subcell face current the spatial part of a local spacetime worldline action, and does that action determine the missing transverse force… | L632 |
| `FTD-0485` | THEOREM, CLOSED_NEGATIVE | Does the selected action determine the full two-slab interior impulse, and is that force unique at the existing hop threshold? | L633 |
| `FTD-0486` | THEOREM | Can the selected minimal action reproduce matched Gauss/source normalization and the frozen equal electric/magnetic force coefficients at… | L634 |
| `FTD-0487` | THEOREM | Is the compact action's hop-threshold force jump a fixture artifact, or is it forced by nonzero matched Gauss charge? | L635 |
| `FTD-0488` | THEOREM, CONSTRUCTIVE, UNDERDETERMINED | Can the Gauss-forced point-force jump be removed by locally subtracting each particle's own field from frozen total state? | L636 |
| `FTD-0489` | THEOREM, CONSTRUCTIVE | Can the exact straight-worldline action select a manifested Moore endpoint by comparing finite open-hop action values? | L637 |
| `FTD-0490` | THEOREM, CONSTRUCTIVE, OPEN | Does the proper discrete Legendre transform evade FTD-0489's open-action gauge defect in a cubical-cell interior? | L638 |
| `FTD-0491` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Does one initial kinetic momentum select a unique incident-cell Legendre branch at a manifested knot? | L639 |
| `FTD-0492` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Does the unique local linear cubic-invariant centered knot trace repair the symmetric branch while remaining the ordinary common-action derivative? | L640 |
| `FTD-0493` | THEOREM, CLOSED_NEGATIVE | Does the centered weak trace close exact matched-face work, or omit a finite jump ledger? | L641 |
| `FTD-0494` | THEOREM, CLOSED_NEGATIVE | Can the exact cusp-work ledger be absorbed into a local single-valued dressing energy of the frozen particle and face-field variables? | L642 |
| `FTD-0495` | CONSTRUCTIVE, THEOREM, CLOSED_NEGATIVE | What minimum additional state records the nonintegrable cusp work reversibly, and can it retain centered motion under an ordinary common action? | L643 |
| `FTD-0496` | THEOREM, CONSTRUCTIVE | Does existing momentum plus the explicit dressing fiber determine a unique centered matter/field transaction from a manifested knot? | L644 |
| `FTD-0497` | CONSTRUCTIVE, THEOREM, CLOSED_NEGATIVE | Can exact axial face work cross the existing remainder threshold and still recover the raw manifested state under the same reverse map? | L645 |
| `FTD-0498` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does the exact `(site,remainder)` quotient make the FTD-0497 inverse defect a harmless engine gauge redundancy? | L646 |
| `FTD-0499` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Can a finite local chart label or other finite hidden state make the non-injective threshold map reversible while leaving its projected raw update… | L647 |
| `FTD-0500` | CONSTRUCTIVE, THEOREM, MEASURED, CLOSED_NEGATIVE | Can one centered canonical chart repair raw inversion while preserving exact cubic covariance and frozen production behavior? | L648 |
| `FTD-0501` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Can summed trilinear polarity and exact face current serve as a complete quotient ontology for multiple manifested objects? | L649 |
| `FTD-0502` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Does an unordered multiset of manifested endpoints uniquely determine the exact oriented face current? | L650 |
| `FTD-0503` | THEOREM, CONSTRUCTIVE, OPEN | Can existing per-manifestation phase space uniquely select the free transport 1-chain before endpoint write-back? | L651 |
| `FTD-0504` | THEOREM, CLOSED_NEGATIVE, OPEN | Does every coincident-target event require a new collision law, or are identical crossings only a label redundancy? | L652 |
| `FTD-0505` | THEOREM, CONSTRUCTIVE, RETRACTED | Can an exact tick-boundary same-sign collision be resolved without adding capacity, interaction range, or temporal phase? | L653 |
| `FTD-0506` | MEASURED, CLOSED_NEGATIVE | Does the production same-sign “elastic bounce” qualify as the reciprocal finite-range exclusion escape isolated by FTD-0505? | L654 |
| `FTD-0507` | THEOREM, RETRACTED, MEASURED | Does the frozen overlapping `(site,remainder)` atlas already store an exact tick-boundary collision state? | L655 |
| `FTD-0511` | WITHDRAWN | Does the Front-B P6C-T collision-vertex temporal-phase candidate remain licensed after the FTD-0507 chart-capacity correction? | L659 |
| `FTD-0512` | DERIVED, CONSTRUCTIVE, THEOREM, CLOSED_NEGATIVE | Can existing constituent phase space supply the FTD-0507 boundary collision map, and can aggregate face action derive it? | L660 |
| `FTD-0513` | THEOREM, CONSTRUCTIVE, OPEN | What is the lowest cubic-covariant constituent moment that retains the FTD-0512 axial counterflow, and is it a complete matter state? | L661 |
| `FTD-0514` | THEOREM, CONSTRUCTIVE, OPEN | Does exact oriented face continuity supply a local constituent momentum balance and the FTD-0513 kinetic-stress bridge, including the selected… | L662 |

### Common-action mechanics & reciprocity — contact, collision & charts

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0525` | THEOREM, CONSTRUCTIVE, MEASURED | Is the FTD-0516 hard-contact surface already the active set of frozen ternary/chart production movement? | L668 |
| `FTD-0526` | THEOREM, MEASURED, CORRECTION, OPEN | Does FTD-0525's raw contact crossing differ physically from hard-contact bounce, and what exactly happens at the later chart horizon? | L669 |
| `FTD-0527` | THEOREM, CONSTRUCTIVE, OPEN | Can the FTD-0526 occupied-target overshoot defect be repaired with the frozen site/remainder variables while preserving exact face current, and what… | L670 |
| `FTD-0528` | MEASURED, THEOREM, OPEN | Does the FTD-0527 identical-contact quotient factor through the actual native pre-movement coupling source and through complete matched face-current… | L671 |
| `FTD-0529` | THEOREM, CLOSED_NEGATIVE, RESOLVED, OPEN | Can the unchanged energy-preserving FTD-0527 elastic contact output couple reciprocally to every admissible matched face field? | L672 |
| `FTD-0530` | HYPOTHESIS, THEOREM, OPEN | Does the curl-free axial exception carry nonzero Gauss-fixed longitudinal work that requires a contact impulse? | L673 |
| `FTD-0531` | CONSTRUCTIVE, MEASURED, CLOSED_NEGATIVE, OPEN | Can existing relativistic momentum absorb the diagonal FTD-0529 field work while endpoints, exact current, Gauss, and reversal are solved together? | L674 |
| `FTD-0532` | DERIVED, CLOSED_NEGATIVE, OPEN | Does the constructive FTD-0531 diagonal endpoint lie inside the compact one-cell FTD-0485 action domain at the chart-horizon hop? | L675 |
| `FTD-0533` | NUMERICAL_FACT, RESOLVED, CLOSED_NEGATIVE | Does the complete FTD-0484 deposited action possess a unique shared-point variation through the internal simultaneous knots of FTD-0532? | L676 |
| `FTD-0534` | THEOREM, CONSTRUCTIVE | Can the FTD-0531 midpoint work field and staggered magnetic endpoints be represented by one FTD-0484 connection slab? | L677 |
| `FTD-0535` | THEOREM, CLOSED_NEGATIVE, CONSTRUCTIVE | Does the exact FTD-0484 temporal current split fit the frozen Faraday-then-total-current phase ordering? | L678 |
| `FTD-0536` | DERIVED, CONSTRUCTIVE, CLOSED_NEGATIVE | Does the minimal implicit atomic face action exist on the diagonal hop, and are the FTD-0531 scalar energy roots stationary solutions of it? | L679 |
| `FTD-0537` | NUMERICAL_FACT, RESOLVED, CONDITIONAL | Does the FTD-0536 action admit a differentiably verified six-coordinate initial-value root, and does that root close either registered total energy? | L680 |
| `FTD-0538` | THEOREM, CONSTRUCTIVE, RESOLVED, CLOSED_NEGATIVE | Does containing every endpoint derivative stencil within one current chart resolve FTD-0537 and validate the unchanged action root? | L681 |
| `FTD-0539` | CONSTRUCTIVE, NUMERICAL_FACT, CLOSED_NEGATIVE | Do the exact shell-2 reflection-plane roots have a unique normal Legendre derivative, a nonsmooth stationary inclusion, or no stationary normal… | L682 |
| `FTD-0540` | THEOREM, CONSTRUCTIVE | Is the FTD-0539 cusp a tunable artifact of the chosen trilinear coefficients, or is it forced by the local polarity representation? | L683 |
| `FTD-0541` | SELECTION, THEOREM, NUMERICAL_FACT | Can the positive non-cardinal smooth escape from FTD-0540 carry an exact local oriented-face current and remove the inactive integer-plane cusp? | L684 |
| `FTD-0542` | SELECTION, THEOREM | Does the selected smooth quadratic coat admit an exact spacetime current and a single interaction functional that generates every face and temporal… | L685 |

### Common-action mechanics & reciprocity — energy closure & Peierls barriers

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0543` | THEOREM, CONSTRUCTIVE | Does fixed-step stationarity of the common action itself prove exact endpoint-energy conservation, and what does the simplest exact-energy repair… | L686 |
| `FTD-0544` | THEOREM | Can the matched face/edge field sector exchange the coat-current work exactly while preserving Gauss, independent of the unresolved particle equation? | L687 |
| `FTD-0545` | THEOREM, CLOSED_NEGATIVE | Does analytic endpoint variation of the quadratic-coat action make the production matter-energy gain identically equal to the exact matched field… | L688 |
| `FTD-0546` | DERIVED, NUMERICAL_FACT, CLOSED_NEGATIVE | Does the FTD-0545 matter-energy defect survive after the exact temporal source and endpoint-current split generate a neutral, periodic… | L689 |
| `FTD-0547` | DERIVED | Is the FTD-0545/0546 matter-work defect intrinsic to the production dispersion, or caused by freezing velocity while momentum changes during the tick? | L690 |
| `FTD-0548` | DERIVED, NUMERICAL_FACT | Does the exact accelerated worldline admit the same gauge-covariant quadratic-coat deposits, and can the old linear-time endpoint split be reused? | L691 |
| `FTD-0549` | THEOREM | Can exact `K0,K1,T` be reconstructed after solving only endpoint and midpoint kinematics? | L692 |
| `FTD-0550` | SELECTION, THEOREM, NUMERICAL_FACT | Does the quadratic coat determine compatible electric and magnetic orbit gathers from the same staggered action complex? | L693 |
| `FTD-0551` | SELECTED_DYNAMICS, THEOREM, NUMERICAL_FACT | Can the exact quadratic current and orbit gathers close one simultaneous relativistic matter/matched-field transaction? | L694 |
| `FTD-0552` | DERIVED, NUMERICAL_FACT, CLOSED_NEGATIVE | Does an isolated quadratic coat at a generic subcell position remain static in its own neutral periodic dressing? | L695 |
| `FTD-0553` | THEOREM, NUMERICAL_FACT, CLOSED_NEGATIVE | Can a rigid localized neutral composite cancel the compact quadratic coat's lattice self-force without subtraction or a new primitive? | L698 |
| `FTD-0554` | THEOREM, CONSTRUCTIVE | Can exact microscopic continuous translation, unitary energy preservation, the one-site shift, and strict finite-range locality coexist on a discrete… | L699 |
| `FTD-0555` | THEOREM, NUMERICAL_FACT | What exact condition makes a local extended source's compact-coat Peierls barrier irrelevant in the infrared? | L700 |
| `FTD-0556` | THEOREM, NUMERICAL_FACT | Can strict integer-site locality produce continuous observable transport without fractional primitive matter position? | L701 |
| `FTD-0557` | THEOREM, NUMERICAL_FACT | Can the isolated free-flux Bloch band itself supply a nonzero localized stationary or rigidly translating carrier? | L702 |
| `FTD-0558` | THEOREM, DERIVED, RETRACTED | What is the exact production moving-source pole, and does the lattice radiate for every nonzero smooth-source speed? | L703 |
| `FTD-0559` | THEOREM, DERIVED | What exact field energy does a prescribed external drive deposit through the corrected production operator? | L704 |
| `FTD-0560` | THEOREM, CLOSED_NEGATIVE | Can one polarity hopping one site every finite `T` ticks carry an exactly co-moving square-summable native linear dressing? | L705 |
| `FTD-0561` | THEOREM, CLOSED_NEGATIVE | Does rigid spatial extension remove the slow-hop resonant source, and what does microscopic neutrality change? | L706 |
| `FTD-0562` | THEOREM, CLOSED_NEGATIVE | Can any fixed nonzero finite neutral form factor cancel the complete three-dimensional slow-hop resonance surface? | L707 |
| `FTD-0567` | THEOREM, CLOSED_NEGATIVE | Does production genesis supply the nonlinear flux-amplitude lock and conservative common action still open after FTD-0564? | L710 |
| `FTD-0569` | THEOREM, CLOSED_NEGATIVE | Can an explicit local environment make the frozen genesis/evaporation kernel reversible while reproducing its acceptance law? | L712 |
| `FTD-0570` | THEOREM, CONSTRUCTIVE, CLOSED_NEGATIVE | Can an exact-real environment supply the missing indefinite branch history, and does that make production genesis a common-action map? | L713 |
| `FTD-0571` | THEOREM, CLOSED_NEGATIVE | Can the missing symplectic reservoir be identified with existing `Voxel` fields that genesis leaves untouched, without changing the projected… | L714 |
| `FTD-0572` | THEOREM, CONSTRUCTIVE, NO_GO | Under the FTD-0570 `(J,W)` canonical pairing, what is the minimum bath required by the accepted-genesis symplectic defect, and can that bath repeat… | L715 |
| `FTD-0573` | THEOREM, DERIVED, RESOLVED | Does cubic covariance select the FTD-0570 `(J,W)` canonical form, and what bath-rank price does that symmetry impose? | L716 |
| `FTD-0574` | THEOREM, DERIVED, NO_GO, CORRECTION, OPEN | Does the frozen production `(J,W)` field tick have a native discrete action, and does the documented onsite velocity interaction generate the coded… | L717 |
| `FTD-0575` | DERIVED, THEOREM, NO_GO, OPEN | Does the exact FTD-0574 source action generate a reciprocal path force, and does that channel recover a long-range electromagnetic static pole? | L718 |
| `FTD-0576` | THEOREM, NO_GO, OPEN | Can the FTD-0575 Hodge channel close exact finite-step energy for mobile matter while retaining the native central operators, cardinal ternary… | L719 |
| `FTD-0577` | THEOREM, RESOLVED | Can a minimal noncardinal coupling coat cancel the FTD-0576 checkerboard obstruction while preserving ternary primitive manifestation, exact local… | L720 |
| `FTD-0578` | THEOREM, DERIVED, CLOSED_NEGATIVE | Does the noncardinal Moore-coated current arise from one reciprocal spacetime action, and is its compact point carrier already a free exact-energy… | L721 |
| `FTD-0579` | THEOREM, DERIVED, CLOSED_NEGATIVE | Can any nonzero finite rigid extension of the Moore carrier eliminate the diagonal energy-centering mismatch or reciprocal Peierls pinning exactly? | L722 |
| `FTD-0580` | THEOREM, CLOSED_NEGATIVE | Can positivity and exact endpoint-energy centering select a route-free diagonal coupling history, and does that history remove the remaining Peierls… | L723 |
| `FTD-0581` | THEOREM, DERIVED, OPEN | Can passive native `(J,W)` dressing remove the FTD-0580 chord barrier, and what does an active escape cost? | L724 |
| `FTD-0582` | THEOREM, MEASURED, CLOSED_NEGATIVE, DERIVED | Can an energetic phase-carrying native `(J,W)` mode transfer the FTD-0581 barrier energy into manifested momentum in the frozen tick? | L725 |

### Common-action mechanics & reciprocity — protected sector, seed bootstrap & removal counts

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0583` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the current real matched face/edge variables carry a localized topologically protected matter sector? | L726 |
| `FTD-0584` | THEOREM, NO_GO, OPEN | Do fixed ternary source, Gauss law, global flux, or uncontained support create a localized protected sector in the frozen ordinary-real fields, and… | L727 |
| `FTD-0585` | THEOREM, ENGINE_FACT, CLOSED_NEGATIVE, OPEN | Does moving manifested support prove transported matter, and can the frozen reaction-free field sector generate motion from rest? | L728 |
| `FTD-0586` | THEOREM, NUMERICAL_FACT, MEASURED, CLOSED_NEGATIVE, SUPERSEDED | Can a sanitized finite ternary seed bootstrap a new manifested site through its own causal state-gradient field, without injected flux, Gauss… | L729 |
| `FTD-0587` | DERIVED, MEASURED, UNRESOLVED, CLOSED_NEGATIVE, CORRECTION | After the externally ignited FTD-0474 dispersal support reaches its locked tail, is persistence supplied by retained injected field, native causal… | L730 |
| `FTD-0588` | THEOREM, NUMERICAL_FACT, MEASURED, CLOSED_NEGATIVE, SUPERSEDED | Does exact spatial orthogonality close the first FTD-0586 collective-source counts, and what part of independently timed evaporation remains… | L731 |
| `FTD-0589` | THEOREM, NUMERICAL_FACT, MEASURED, CLOSED_NEGATIVE, SUPERSEDED | Does exact finite-pulse cancellation close the FTD-0588 all-off tail, and what arbitrary-removal count remains unexcluded? | L732 |
| `FTD-0590` | THEOREM, NUMERICAL_FACT, CLOSED_NEGATIVE, SUPERSEDED | Does exact cubic-orbit coherence close the seven-source arbitrary-removal boundary without selecting a source geometry or removal schedule? | L733 |
| `FTD-0591` | THEOREM, NUMERICAL_FACT, CLOSED_NEGATIVE, SUPERSEDED | Does the unchanged orbit-coherence inequality close the separately preregistered eight-source count? | L734 |
| `FTD-0592` | THEOREM, NUMERICAL_FACT, CLOSED_NEGATIVE, OPEN | Does the unchanged orbit-coherence inequality close the separately preregistered nine-source count? | L735 |
| `FTD-0593` | THEOREM, NUMERICAL_FACT, INCONCLUSIVE, OPEN | Does the unchanged orbit-coherence inequality close the separately preregistered ten-source count? | L736 |
| `FTD-0594` | THEOREM, NUMERICAL_FACT, INCONCLUSIVE, CLOSED_NEGATIVE | Does exact equality of the native pulse coefficient across all stencil-eigenvalue-degenerate cubic orbits close the ten-source bound? | L737 |
| `FTD-0595` | THEOREM, NUMERICAL_FACT, INCONCLUSIVE, CLOSED_NEGATIVE | Does exact axial-neighbor pair capacity tighten the ten-source shared-`M` bound enough to close the first-event question? | L738 |
| `FTD-0596` | THEOREM, INCONCLUSIVE, CLOSED_NEGATIVE | Does the complete Fourier-positive cubic distance-distribution relaxation tighten the ten-source shared-`M` bound enough to close the first-event… | L739 |
| `FTD-0597` | THEOREM, CLOSED_NEGATIVE | Does the exact same-observation-time removal-pulse product range close the ten-source bound when combined with the unchanged FTD-0596 distance… | L740 |

---

## Constituent-complete matter

*Compact cores, trimers and connected blocks; rest states and Hessians; transport, gait, capture/binding, wakes, causal-horizon persistence.*

**167 claims.**

### Constituent-complete matter — cluster inertia & the moving clock

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0250` | IMPOSED, OPEN, BOUNDARY | Cluster transport inertia = N·M_REST (unified-mass Phase 2) | L413 |
| `FTD-0251` | MEASURED, SELECTION, BOUNDARY | The substrate's only native dynamical angle is the symplectic (quadrature) clock — commutative, not a measurement angle | L414 |
| `FTD-0252` | OBSERVATION, MEASURED, OPEN, AXIOM | Dynamical time dilation of a moving lattice "wave clock" (effective mass from transverse momentum) | L415 |

### Constituent-complete matter — collective-coordinate reduction & particlehood

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0349` | DERIVED, BOUNDARY, AXIOM | Collective-coordinate reduction attempt — PARTIAL: cluster inertia = N*M_REST iff the Gradient-Normalization Condition (GNC), which fails for both… | L494 |
| `FTD-0350` | THEOREM, PARAMETRIC, SMC | Phase-J ultralocality at all L >= 2 — spine Theorem 7's L>=4 ambiguity closed as a proven masking artifact | L495 |
| `FTD-0392` | CLOSED_NEGATIVE | Hedgehog topological charge of the flux field is robust across birth circumstance, but robustly ZERO at manifestation freeze — not the nonzero value… | L542 |
| `FTD-0394` | MEASURED | Engine-native readout-collision demonstration for manifestation's many-to-one claim — fills the citation gap in FC-2's "provably unrecoverable" | L541 |
| `FTD-0398` | MEASURED | Terminal transport test for FTD-0392's octahedral Berg–Lüscher charge | L539 |
| `FTD-0399` | INVALID | Target-blind particlehood test before any further mass observable | L540 |

### Constituent-complete matter — compact cores, trimers & walkers

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0599` | THEOREM, CLOSED_NEGATIVE | Can one site-ontic reciprocal hop be defined as a unique, local, energy-closing, state-invertible atomic map using only existing persistent `s`, `J`… | L742 |
| `FTD-0600` | SELECTED_DYNAMICS, THEOREM, MEASURED, CLOSED_NEGATIVE | Does explicit constituent-complete ternary matter permit a single reversible common-action transaction with bound site-crossing motion, and does the… | L743 |
| `FTD-0601` | SELECTED_DYNAMICS, THEOREM, MEASURED, CLOSED_NEGATIVE | Does the FTD-0600 common-action construction remain reversible and momentum-closed after replacing the fixed compensator with a second dynamical… | L744 |
| `FTD-0602` | THEOREM, MEASURED, CLOSED_NEGATIVE | Was FTD-0601's outward force caused by its arbitrary Gauss dressing, and does the unique minimum-energy Gauss field repair attraction and momentum? | L745 |
| `FTD-0603` | CLOSED_NEGATIVE, THEOREM, NUMERICAL_FACT | Is the minimum-energy neutral-pair attraction and apparent momentum exchange invariant under fractional rigid translation through one lattice cell? | L746 |
| `FTD-0604` | THEOREM, MEASURED, CLOSED_NEGATIVE | Can the least arbitrary internal deformation—a charge-conjugate symmetric breathing coordinate already present in constituent phase space—remove the… | L747 |
| `FTD-0605` | THEOREM, MEASURED, CLOSED_NEGATIVE | Does the full six-dimensional local zero-centroid shape space already present in the mirrored constituents contain a stable dressed compact core? | L748 |
| `FTD-0606` | THEOREM, MEASURED, UNRESOLVED | Does a globally reparameterized SO(3) x strain chart contain a stable dressed static core for the two charge-conjugate equilateral trimers, and does… | L749 |
| `FTD-0607` | MEASURED, UNRESOLVED | Does imposing the one-label-per-site ternary capacity inside the SO(3) x strain optimization itself still admit stable compact static cores, and can… | L750 |
| `FTD-0608` | MEASURED, UNRESOLVED | Can the phase-15 site-admissible compact core, launched at collective velocity, transport completely through the next lattice site under the… | L751 |
| `FTD-0609` | MEASURED, CONSTRUCTIVE, CLOSED_NEGATIVE | Does permitting at most two distinct constituent records to share one integer chart anchor (a default-off fibre extension) repair the FTD-0608… | L752 |
| `FTD-0610` | MEASURED, CLOSED_NEGATIVE | Is the extracted phase-15 charge-`+1` trimer, neutralized by an external uniform or frozen-partner compensator, an isolated stationary rest state of… | L753 |
| `FTD-0611` | MEASURED, CLOSED_NEGATIVE | Does a genuine rest state of one charge-`+1` trimer exist in the periodic uniform neutralizer `rho_bg=-1/L^3`, passing a locked… | L754 |
| `FTD-0612` | MEASURED | Does one deterministic Newton refinement of the FTD-0611 basin repair its locked gradient and rest-drift precision failure while preserving the… | L755 |
| `FTD-0613` | MEASURED, CLOSED_NEGATIVE | Does the refined FTD-0612 rest state translate reversibly and axis-symmetrically under equal-momentum directional boosts at three registered speeds? | L756 |
| `FTD-0614` | MEASURED, NUMERICAL_FACT, UNRESOLVED | Does the refined compact core's translation landscape have a unique locally relaxed passive energy barrier, and is the FTD-0613 fixed-body axis… | L757 |
| `FTD-0615` | MEASURED, OPEN | Can an internal excitation of the compact fixed point's six-dimensional zero-centre-momentum tangent basis translate the intact centre through the… | L758 |
| `FTD-0616` | MEASURED, CLOSED_NEGATIVE, OPEN | Does the FTD-0615 constructive internal walker persist in a straight, sign-controlled direction over a longer 512-tick history? | L759 |
| `FTD-0617` | MEASURED, SELECTION, OPEN | Does the FTD-0615/0616 rotational-plane response vary as an isotropic, angle-independent velocity mode, or does it show a mixed-parity… | L760 |
| `FTD-0618` | MEASURED, CLOSED_NEGATIVE, OPEN | Can a net-neutral, charge-conjugate half-turn pair of compact cores cancel their transverse gait response while sharing coherent axial translation… | L761 |
| `FTD-0619` | MEASURED, CLOSED_NEGATIVE, OPEN | Does the force-matched spline-Poynting field momentum, rather than the previously used local pseudomomentum, close the FTD-0618 coupled… | L762 |
| `FTD-0620` | MEASURED, CLOSED_NEGATIVE, OPEN | Does the FTD-0618 balanced neutral gait return to its launch internal state within a locked 512-tick window, or is its motion a one-time relaxation? | L763 |

### Constituent-complete matter — connected blocks & resolution scaling

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0621` | THEOREM, MEASURED, OPEN | Can an exactly ternary, finite, neutral integer block-bipole configuration suppress its relative (barrier-to-field-energy) lattice pinning toward… | L764 |
| `FTD-0622` | SELECTED_DYNAMICS, THEOREM, MEASURED | Can the exact integer FTD-0621 block-bipole family be embedded in one local, constituent-complete, reversible common action whose relative Peierls… | L765 |
| `FTD-0623` | SELECTED_DYNAMICS, MEASURED, THEOREM, OPEN | Does the smallest connected FTD-0622 integer object remain coherent, state-only reversible, and coherently mobile under a finite boost across… | L766 |
| `FTD-0624` | SELECTED_DYNAMICS, MEASURED, CLOSED_NEGATIVE, OPEN | Does the connected 16-constituent block's static half-cell centre correspond to an admissible, dynamically stable, reversible rest state under… | L767 |
| `FTD-0625` | SELECTED_DYNAMICS, MEASURED, CLOSED_NEGATIVE, OPEN | Does zero-total-momentum rigid internal circulation, launched at the connected block's measured Peierls-barrier energies, stabilize it against the… | L768 |
| `FTD-0626` | MEASURED, CLOSED_NEGATIVE, OPEN | Does applying the already-priced FTD-0609 multiplicity-two chart fibre repair the FTD-0624/0625 occupancy failures without changing the matter-field… | L769 |
| `FTD-0627` | SELECTED_DYNAMICS, MEASURED, CLOSED_NEGATIVE, OPEN | Over a 256-tick horizon, does the fibre-enabled exact-half connected block's centre-rest state remain bounded and reversible, and does its complete… | L770 |
| `FTD-0628` | MEASURED, OPEN | Does the unchanged selected connected-block action possess a nearby symmetry-preserving configuration that is stationary against all 48 constituent… | L771 |
| `FTD-0629` | DERIVED, MEASURED, OPEN | Does the FTD-0628 dressed fixed point support four bounded, reversible, approximately decoupled finite-amplitude internal responses whose… | L772 |
| `FTD-0630` | MEASURED, EXECUTION_INVALID | Is the FTD-0628 body-half dressed state a three-axis translation saddle — positive body-axis curvature but negative transverse curvatures — and does… | L773 |
| `FTD-0631` | MEASURED, CLOSED_NEGATIVE | Does the connected-block candidate — translated to the positive-curvature (fully-half) phase on all three lattice axes — admit a genuine… | L774 |
| `FTD-0632` | DERIVED, MEASURED | What is the minimum finite nearest-site chart multiplicity required to represent every geometry in the locked four-coordinate connected-block Hessian… | L775 |
| `FTD-0633` | DERIVED, MEASURED | Does replacing the provisional cap-two chart with the independently derived cap-eight cubic chart remove the sole FTD-0631 obstruction and produce a… | L776 |
| `FTD-0634` | MEASURED, EXECUTION_INVALID | Does the FTD-0633 dressed state remain a local energy minimum when every constituent is displaced independently in all three Cartesian directions… | L777 |
| `FTD-0635` | MEASURED, EXECUTION_INVALID | Does repairing the 48-coordinate gradient estimator (finer step `h_g=2e-5`, unchanged Hessian step `h_H=2e-4`) resolve FTD-0634's gradient-gate… | L778 |
| `FTD-0636` | MEASURED, EXECUTION_INVALID | Does confining the Hessian/translation stencils to `h_H=4e-5` (strictly inside the quadratic-B-spline knot clearance) produce a locally valid… | L779 |
| `FTD-0637` | DERIVED, MEASURED | Is the FTD-0633 dressed state a stationary positive basin of the complete 48-coordinate static functional when derivatives are evaluated analytically… | L780 |
| `FTD-0638` | DERIVED, MEASURED | Does the positive analytic basin found by FTD-0637 contain a genuine 48-coordinate stationary state when the state is refined by a full-space Newton… | L781 |
| `FTD-0639` | MEASURED | Does the FTD-0638 local minimum remain a bounded, reversible rest state under the already selected constituent common-action tick, or is it only a… | L782 |
| `FTD-0640` | DERIVED, MEASURED | Does the analytically centered connected object carry genuine internal matter degrees of freedom whose measured common-action phases follow the… | L783 |
| `FTD-0642` | DERIVED, MEASURED | Does releasing all 48 exact-center matter coordinates under the same registered transverse face/edge field perturbations produce one reversible… | L785 |
| `FTD-0643` | EXECUTION_INVALID | Does releasing a finite uniform momentum on all 16 constituents along canonical directions establish coherent finite collective transport with a… | L786 |
| `FTD-0644` | MEASURED, CLOSED_NEGATIVE | After repairing the FTD-0643 arm-count defect and rotating the whole dressed state in the cubic controls, does the corrected 32-arm boost ladder pass… | L787 |
| `FTD-0645` | MEASURED | After rotating the analytic modal basis with the state (repairing the FTD-0644 observer defect), does the 32-arm boost ladder pass every exact… | L788 |
| `FTD-0646` | MEASURED | Over a long 256-tick horizon, does the fixed-object collective-momentum boost produce secular translation (a persistent free-particle-like mode) or… | L789 |
| `FTD-0647` | THEOREM, CLOSED_NEGATIVE | Can the current connected ternary bipole be made progressively wider while retaining a finite nonzero rest mass, if every constituent and action… | L790 |
| `FTD-0648` | DERIVED, MEASURED | Does an ordinary three-dimensional cell-measure rescaling convert the connected-block width sequence into a fixed-mass, fixed-integrated-polarity… | L791 |
| `FTD-0649` | DERIVED, MEASURED | Can the four FTD-0648 resolution factors be installed in one reciprocal matter-field transaction (dispersion, source/current/Gauss, field… | L792 |
| `FTD-0650` | EXECUTION_INVALID | Does the FTD-0649 fixed-mass reciprocal action remain coherent, reversible, and secularly mobile over one common physical horizon as resolution… | L793 |
| `FTD-0651` | CONSTRUCTIVE | Does the repeated-root Broyden-cached central-difference Jacobian solve the frozen FTD-0649 common-action residual with the same accepted states as… | L794 |
| `FTD-0652` | SELECTED_DYNAMICS | Does the FTD-0649 fixed-mass reciprocal common action remain coherent, reversible, and secularly mobile over a common physical horizon as lattice… | L795 |
| `FTD-0653` | THEOREM, CORRECTION | Is the FTD-0652 minimum-mobility-nondecrease-with-width criterion a valid necessary-and-sufficient diagnostic for an isotropic mobile continuum limit? | L796 |
| `FTD-0654` | SELECTED_DYNAMICS | Do the FTD-0652 anisotropy and target-centred mobility improvements survive on unseen launch velocities and a doubled physical horizon? | L797 |
| `FTD-0655` | EXECUTION_INVALID | Does the matched field-energy dressing travel as one coherent dynamical pattern with the fixed-total-measure constituent core, as measured by the… | L798 |
| `FTD-0656` | MEASURED | After correcting the FTD-0655 tick-count contradiction, does the matched field-energy dressing travel as one coherent pattern with the mobile… | L799 |

### Constituent-complete matter — internal modes & matter-field transfer

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0657` | THEOREM | Can a linear retarded response of the current classical common-action state identify a massive-particle pole `omega^2=omega_0^2+c^2k^2` solely… | L800 |
| `FTD-0658` | THEOREM, CLOSED_NEGATIVE, CONSTRUCTIVE | Does any currently registered FTD candidate (the fixed dressed rest state, a prepared internal matter mode, or the co-moving field dressing) supply a… | L801 |
| `FTD-0659` | SELECTED_DYNAMICS | Does the first non-rigid analytic internal matter eigenspace carry a robust, state-functional, conserved action-angle phase under the selected… | L802 |
| `FTD-0660` | SELECTED_DYNAMICS | When the first internal constituent doublet loses its matter-only action, does the complete common-action state place the complementary excitation in… | L803 |
| `FTD-0661` | SELECTED_DYNAMICS | Does correcting the zero-observer threshold and replacing the two-vector polarization sample with a covariant four-vector tight frame resolve the… | L804 |
| `FTD-0662` | SELECTED_DYNAMICS | Does normalizing the tight-frame covariance observer by each arm's own initial doublet energy make the polarization control basis-invariant and yield… | L805 |
| `FTD-0663` | THEOREM, MEASURED | Does the first internal matter doublet's discrete phase lie inside the propagating face/edge field band, excluding kinematic frequency-gap protection… | L806 |
| `FTD-0664` | EXECUTION_INVALID | Is the early loss of action from the first internal matter doublet, and the outward growth of its dynamic face/edge field residual, already present… | L807 |
| `FTD-0665` | SELECTED_DYNAMICS | Does the corrected volume-scaled protocol establish volume-independent pre-return generation of an outward dynamic field from the internal-mode… | L808 |
| `FTD-0666` | RETRACTED | Does extending the truncated L=17 volume-scaled run to tick 100 confirm the predicted absolute return-time window (ticks 74-78) for the internal-mode… | L809 |
| `FTD-0667` | THEOREM | Can eliminating the field block of a linearized one-tick matter/field tangent map produce an exact discrete memory law that explains apparent… | L810 |
| `FTD-0668` | RETRACTED | Does the internal-mode excitation return before the causally isolated self-contact time on a large L=97 volume, ruling out periodic self-return as… | L811 |
| `FTD-0670` | RETRACTED | Does the held-out internal-mode envelope turn upward before any conservative periodic return can reach the core? | L813 |
| `FTD-0671` | THEOREM | Can matched leapfrog field energy be partitioned into exact regional transport and deposited-current work? | L814 |
| `FTD-0672` | RETRACTED | What field-energy flow occurs during the former tick-68--80 recovery window? | L815 |
| `FTD-0673` | THEOREM | Can the excited-minus-control energy be decomposed without assigning unexplained loss to a reservoir? | L816 |
| `FTD-0674` | EXECUTION_INVALID | Does another registered reservoir donate energy back to the canonical doublet after its trough? | L817 |
| `FTD-0675` | THEOREM | Was the paired-mode displacement diagnostic a canonical modal-energy coordinate? | L818 |
| `FTD-0676` | SELECTED_DYNAMICS | Does the corrected canonical mode energy decay reproducibly before causal return? | L819 |
| `FTD-0677` | THEOREM | Can localized internal phase and near/intermediate/far difference-field content be measured covariantly? | L820 |
| `FTD-0678` | EXECUTION_INVALID | Does the first locked localized-basin relaxation campaign establish dynamics? | L821 |
| `FTD-0679` | EXECUTION_INVALID | Does the second localized-basin campaign produce a valid target record? | L822 |
| `FTD-0680` | CORRECTION | Was the localized-basin observer's flat storage mapping correct? | L823 |
| `FTD-0681` | EXECUTION_INVALID | Does the corrected locked relaxation replay satisfy its preregistered replication gate? | L824 |
| `FTD-0682` | NUMERICAL_FACT | What survives as descriptive evidence in the corrected FTD-0681 data? | L825 |
| `FTD-0683` | QUALIFIED_OBSERVER | Can radial field morphology be sampled at the actual staggered face/edge carrier positions? | L826 |
| `FTD-0684` | EXECUTION_INVALID | Does the first causal-excitation separation campaign execute? | L827 |
| `FTD-0685` | EXECUTION_INVALID | Does the tolerance-corrected causal-excitation campaign finish with scalar regional masks? | L828 |
| `FTD-0686` | THEOREM | Can all registered regional energies be evaluated in one exact batched pass? | L829 |
| `FTD-0687` | EXECUTION_INVALID | Does the batched-observer causal-excitation campaign finish? | L830 |
| `FTD-0688` | THEOREM | Can nested regional profiles be computed by an exact prefix-sum observer? | L831 |
| `FTD-0689` | EXECUTION_INVALID | Does the prefix-sum `L=129` causal-excitation campaign finish? | L832 |
| `FTD-0690` | EXECUTION_INVALID | Can the causal-excitation discriminator finish on the first executable `L=113`, 96-tick extension? | L833 |
| `FTD-0691` | EXECUTION_INVALID | Does block-sampling the expensive spatial observer make the same `L=113` campaign executable? | L834 |
| `FTD-0692` | QUALIFIED_OBSERVER | Can a local residual evaluator reproduce the complete common-action transaction before accepted-state materialization? | L835 |
| `FTD-0693` | EXECUTION_INVALID | Can the qualified but unindexed local-residual campaign complete the locked discriminator? | L836 |
| `FTD-0694` | SELECTED_DYNAMICS | What does the indexed-local causal-excitation campaign show before periodic contact? | L837 |
| `FTD-0695` | THEOREM | What source-free group velocities are available at the first connected internal resonance? | L838 |
| `FTD-0696` | THEOREM | Can a carrier-aware symmetry-ray Fourier spectrum be measured covariantly on staggered fields? | L839 |
| `FTD-0697` | THEOREM | Can the qualified symmetry-ray spectrum be batched without changing its complex coefficients? | L840 |
| `FTD-0698` | EXECUTION_INVALID | Does the first held-out internal-excitation spectrum campaign classify resonant transfer? | L841 |
| `FTD-0699` | MEASURED | Does a corrected fresh-amplitude spectrum resolve the internal excitation on all principal rays? | L842 |

### Constituent-complete matter — transport, gait & depinning

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0700` | THEOREM | Is sub-cone axial motion kinematically exposed to transverse lattice modes? | L843 |
| `FTD-0701` | CLOSED_NEGATIVE | Does the selected ideal connected bipole cancel the complete axial resonant channel? | L844 |
| `FTD-0702` | QUALIFIED_OBSERVER | Can the actual oriented face current be measured spectrally with carrier phase and matched transversality? | L845 |
| `FTD-0703` | CLOSED_NEGATIVE | Does the connected dressed object's deposited quadratic-coat current cancel every resonant transverse channel? | L846 |
| `FTD-0704` | SELECTED_DYNAMICS | Can the complete dressed 16-constituent state be prepared as a coherent moving source? | L847 |
| `FTD-0705` | SELECTED_DYNAMICS | Does the moving dressed source exhibit the locked sharp transverse-field threshold? | L848 |
| `FTD-0706` | EXECUTION_INVALID | Does the first complete moving-dressing preparation satisfy `F^2(X)=T_1X` from a qualified rest state? | L849 |
| `FTD-0707` | MEASURED | Do four symmetry-preserving static coordinates certify a complete `L=33` rest fixed point? | L850 |
| `FTD-0708` | NUMERICAL_FACT | Can the full constituent impulse residual be solved to obtain a true selected rest fixed point? | L851 |
| `FTD-0709` | CLOSED_NEGATIVE | Does a static boost of the corrected rest solution create a complete moving relative orbit? | L852 |
| `FTD-0710` | NUMERICAL_FACT | Can iterative full-field shooting solve the prescribed rigid co-moving trajectory? | L853 |
| `FTD-0711` | THEOREM | Is the rigid `v=1/2` co-moving field equation exactly solvable in Fourier space? | L854 |
| `FTD-0712` | SELECTED_DYNAMICS | Can existing constituent deformation cancel the eight resonant null projections? | L855 |
| `FTD-0713` | SELECTED_DYNAMICS | Can the null-canceling gait remain causal and bound under the registered deformation limits? | L856 |
| `FTD-0714` | THEOREM | Can an unequal two-tick labeled gait return its constituent momenta under the symmetric endpoint velocity law? | L857 |
| `FTD-0715` | THEOREM | Does the first odd temporal cycle admit a causal bound internal momentum return? | L858 |
| `FTD-0716` | SELECTED_DYNAMICS | Does the period-three deposited current admit a complete translated-return field? | L859 |
| `FTD-0717` | CLOSED_NEGATIVE | Does the independent minimum-norm co-moving field satisfy the period-three matter common action? | L860 |
| `FTD-0718` | CLOSED_NEGATIVE | Can the homogeneous co-moving field freedom repair the locked period-three force mismatch? | L861 |
| `FTD-0719` | THEOREM | Do equal endpoint matter densities determine the oriented deposited current? | L862 |
| `FTD-0720` | NUMERICAL_FACT | Does the interacting common action select one current/root within the registered seed family? | L863 |

### Constituent-complete matter — capture, binding & persistence

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0721` | DERIVED, THEOREM | Must pair connectivity be stored, and can a closed pair form from outside its compact interaction support without an energy receiver? | L864 |
| `FTD-0722` | CLOSED_NEGATIVE | Does the matched field capture an initially unbound derived pair in the first registered encounter protocol? | L865 |
| `FTD-0723` | CLOSED_NEGATIVE | Does a constant exported-energy estimate predict a capture window at `p=0.0200--0.0300`? | L866 |
| `FTD-0724` | NUMERICAL_FACT | Does the preregistered lower-energy crossover produce covariant trapping and detached-field capture? | L867 |
| `FTD-0725` | NUMERICAL_FACT | Is FTD-0724's covariance defect physical or nonlinear-root conditioning? | L868 |
| `FTD-0726` | SELECTED_DYNAMICS | Does the full lower-energy formation matrix pass under the independently qualified exact-root realization? | L869 |
| `FTD-0727` | UNRESOLVED | Does the qualified trapped family retain a bound dressing for 96 ticks under symmetry, polarity, and escape controls? | L870 |
| `FTD-0728` | NUMERICAL_FACT | Does a tenfold tighter root tolerance make the complete persistence campaign covariantly convergent? | L871 |
| `FTD-0729` | NUMERICAL_FACT | Does the worst late-reentry covariance defect converge under another decade of root refinement? | L872 |
| `FTD-0730` | NUMERICAL_FACT | Is the late re-entry a local two-volume behavior or an `L=33` recurrence artifact? | L873 |
| `FTD-0731` | SELECTED_DYNAMICS | Does multi-pass field-assisted capture persist on two volumes for the registered horizon? | L874 |
| `FTD-0732` | CONSTRUCTIVE | Do locked perturbations around captured states all remain admissible and captured? | L875 |
| `FTD-0733` | THEOREM | What is the exact admissible radial shell for negative internal energy at fixed kinetic energy? | L876 |
| `FTD-0734` | SELECTED_DYNAMICS | Do energy-adapted mixed perturbation corners remain captured under all registered symmetry controls? | L877 |
| `FTD-0735` | NUMERICAL_FACT | Do strict capture margins and regular implicit roots imply finite-time open neighborhoods? | L878 |
| `FTD-0736` | CLOSED_NEGATIVE | Does graph re-entry before any possible periodic return already imply a persistent negative core? | L879 |
| `FTD-0737` | SELECTED_DYNAMICS | Does negative-core formation occur after a reproducible precontact energetic delay? | L880 |
| `FTD-0738` | THEOREM | Must interaction-support entry precede energetic binding for the selected compact potential? | L881 |
| `FTD-0739` | SELECTED_DYNAMICS | Can the selected reciprocal action form a durable finite-support relational core plus an outgoing field tail before periodic contact? | L882 |

### Constituent-complete matter — causal-horizon persistence, wake & momentum closure

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0745` | CLOSED_NEGATIVE | Does the first six-shell environmental-closure campaign carry outward field energy to radii 32 and 48 by tick 184 while the finite core persists? | L888 |
| `FTD-0746` | EXECUTION_ABORTED | Can the unchanged causal-horizon persistence discriminator be completed on CPU at `L=321`? | L889 |
| `FTD-0753` | NUMERICAL_FACT | Does the explicit-rounding CUDA backend produce a large causal-horizon finite-support witness on all principal rays? | L896 |
| `FTD-0754` | CORRECTION | Can one state-only observer separate selected bound dressing from outgoing/incoming residual field on the frozen FTD-0753 records? | L897 |
| `FTD-0755` | INFRASTRUCTURE | Does the first support-invariant held-out M3 validation execute its candidate and remote-fibre dynamics? | L898 |
| `FTD-0756` | INFRASTRUCTURE | Why did every FTD-0755 parent stop before held-out initialization? | L899 |
| `FTD-0757` | NUMERICAL_FACT | Does a fixed integer preparation chart preserve the qualified M3 parent while its derived midpoint becomes fractional? | L900 |
| `FTD-0758` | CONSUMED | Does the fixed-chart held-out validation establish the finite-time selected matter family? | L901 |
| `FTD-0760` | SELECTED_DYNAMICS | Does a relational-chart held-out CUDA validation establish an open finite-time selected matter family? | L903 |
| `FTD-0761` | UNKNOWN | Do boosted members of the selected family translate while preserving relational-core identity and complete moving-state coherence? | L904 |
| `FTD-0762` | UNKNOWN | Are FTD-0761's moving-dressing observer failures physical or an integer-center chart obstruction? | L905 |
| `FTD-0763` | CONSTRUCTIVE | Can the selected Gauss dressing observer be extended covariantly to fractional constituent centers? | L906 |
| `FTD-0764` | CLOSED_NEGATIVE | Does the complete selected field dressing co-move rigidly with the transported relational core? | L907 |
| `FTD-0765` | CORRECTION | Does FTD-0764's trailing residual moment independently demonstrate wake creation? | L908 |
| `FTD-0766` | EXECUTION_INVALID | Does age/boost variation establish a mirrored, persistent dynamical wake? | L909 |
| `FTD-0767` | NUMERICAL_FACT | Did the aged wake campaign spatially clear the initial observer window, and what does signed rest subtraction measure? | L910 |
| `FTD-0768` | EXECUTION_INVALID | Does a 768-tick aged face history preserve state-only mobile identity, clear the fixed laboratory slab, close the complete regional ledger, and… | L911 |
| `FTD-0769` | EXECUTION_INVALID | Does total matter+field momentum close for the FTD-0760/0761 moving-core family under transport, and if not, is the non-closure regionally localized… | L912 |

### Constituent-complete matter — protonucleus & body growth

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0799` | MEASURED, CLOSED_NEGATIVE, THEOREM | Does a manifested body above the quasi-static critical radius `R_c = 12.63` grow without bound, saturate at a preferred size, or collapse? | L959 |

---

## Native time & the carrier programme

*The quartic action-angle clock, G* as a temporal invariant, the C1/C2/C3 carrier conditions and every carrier candidate opened against them.*

**185 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0770` | IMPOSED, THEOREM, CLOSED_NEGATIVE, OPEN | Does a selected lattice of coupled quartic action--angle clocks produce a relational temporal field, and does any registered dimensionless linear… | L913 |
| `FTD-0771` | THEOREM, CLOSED_NEGATIVE, SELECTION | Does comparing the selected quartic clock with the support-bound reference for one abstract lattice interval derive a unique minimum dimensionless… | L914 |
| `FTD-0772` | THEOREM, QUALIFIED_OBSERVER, CLOSED_NEGATIVE, OPEN | Does the signed fixed-ray coordinate of the only registered native phase-bearing candidate possess a stationary, amplitude-invariant temporal… | L915 |
| `FTD-0773` | THEOREM, IMPOSED, OPEN | Does the proposed quartic occupancy-to-clock chain, supplemented by a quadratic coordinate-edge interaction, yield an exact nonlinear `G*` signature… | L916 |
| `FTD-0776` | MEASURED, EXACT, OPEN, CONDITIONAL | Does the preselected `[CANDIDATE]` aggregate `q_active=sum_{s_i!=0} J_{i,x}` furnish a recurrence qualifying the quartic diagnostics in the frozen… | L917 |
| `FTD-0777` | EXACT, NO_GO, CONDITIONAL, OPEN | Can a finite dyadic root cover furnish an exact recurrence hierarchy and stable mode-addition memory, and does it bridge the selected quartic clock… | L918 |
| `FTD-0778` | THEOREM, MEASURED, OPEN | Where can `G*` survive as an invariant, and does the preregistered `q_active` aggregate behave as a natural coordinate in the locked FTD-0776 profile? | L919 |
| `FTD-0779` | PRE_REGISTRATION | Does any admissible native channel behave as a natural coordinate in a fixed preregistered profile? | L920 |
| `FTD-0780` | MEASURED, OPEN | Is the FTD-0659 doublet - the strongest registered native phase-bearing candidate - a quartic clock? | L922 |
| `FTD-0781` | ENGINE_FACT, THEOREM, OPEN | Does the locked FTD-0776/0779 profile contain a conservative anharmonic sector at all - is the nonlinearity hard or soft? | L924 |
| `FTD-0782` | ENGINE_FACT, SYNTHESIS, OPEN, IMPOSED | What does the movement-enabled sector actually contain, and what is the first admissible carrier candidate there? | L926 |
| `FTD-0783` | MEASURED, CLOSED_NEGATIVE, STRUCTURAL | Does the compact-law bound pair's breathing mode qualify as the carrier - what is the sign of its anharmonicity? | L928 |
| `FTD-0784` | EXACT, THEOREM, STRUCTURAL, OPEN | Can any native single-clock observable carry the FC-W surd delta = sqrt of G*(4G*-1) - and if not, what exactly is missing? | L930 |
| `FTD-0786` | CORRECTION, SYNTHESIS, EXACT, STRUCTURAL, ENGINE_FACT, SELECTED_DYNAMICS | Is the movement-enabled sector actually blocked by the reciprocal-transaction problem - and if not, where does the carrier fail there? | L934 |
| `FTD-0787` | REFUTATION, THEOREM | Is there a native mechanism realizing C3 - null-flat bottom with quartic growth - and does it clear the band? | L936 |
| `FTD-0788` | REFUTATION, EXACT, SELECTION, IMPOSED, OPEN | What sets the compact-law well depth `eps`, and does the answer decide C2? | L938 |
| `FTD-0789` | REFUTATION, THEOREM, OPEN | Does FTD-0787's flexural quartic survive adversarial audit? | L940 |
| `FTD-0790` | REFUTATION, EXACT, CORRECTION, SELECTION, IMPOSED | Does FTD-0788's lattice-quantum derivation of `eps` survive adversarial audit? | L942 |
| `FTD-0797` | REFUTATION, EXACT, THEOREM, ENGINE_FACT | Does putting every sector on the cubic de Rham complex deliver the exact common cone section 24 demands? | L955 |
| `FTD-0798` | CLOSED_NEGATIVE, THEOREM, CONSTRUCTIVE, CORRECTION | Is a massive Kaehler-Dirac field a viable first-order clock-operator carrier? | L957 |
| `FTD-0800` | CLOSED_NEGATIVE, MEASURED | Does any configuration in the locked Maxwell-criterion screen realize C3 as a native `n = 4` mechanism under the registered compact pair law? | L961 |
| `FTD-0801` | CLOSED_NEGATIVE, MEASURED, SYNTHESIS | Does the locked periodic triangulated-sheet candidate realize C3 as an `n = 4` mechanism for a free body under the registered compact law? | L963 |
| `FTD-0804` | CONDITIONAL, THEOREM, MEASURED | Does a minimum viable clock carrier exist, and at what price? | L966 |
| `FTD-0805` | CONSTRUCTIVE, CLOSED_NEGATIVE, OPEN | Does the registered single-scale law natively host an `n = 4` mechanism? | L967 |
| `FTD-0806` | SUPERSEDED, SYNTHESIS | Temporal-interior programme registered (charter plus three criteria specs). | L968 |
| `FTD-0808` | EXACT | T2 first screen: does a geometric bit hold state, and at what barrier? | L970 |
| `FTD-0814` | DERIVED, MEASURED, RETRACTED, OPEN | What must a clock carrier be made of for its rate to dilate relativistically? | L976 |
| `FTD-0817` | DERIVED | What is `G*` the constant of, in clock terms? | L979 |
| `FTD-0820` | DERIVED | Which part of a clock constant is kinematics and which part is the potential? | L982 |
| `FTD-0821` | DERIVED, CORRECTION | Is the quartic clock forced, or chosen — and what exactly does the forcing statement require? | L983 |
| `FTD-0822` | THEOREM, SUPERSEDED, CONDITIONAL | Is the register's barrier equal to one bond depth, or only bounded by it? | L985 |
| `FTD-0823` | REFUTATION, DERIVED | Does one energy scale `ε` price binding, clock rate and retention — the architecture's economy claim? | L986 |
| `FTD-0824` | DERIVED, CORRECTION | What is the four-chain clock's finite-amplitude correction, exactly? | L984 |
| `FTD-0826` | THEOREM, CONDITIONAL, CLOSED_NEGATIVE, SELECTION | Can a target-blind native phase/action carrier be source-fixed and connected, by one genuine operator, to the archimedean quartic period and… | L988 |
| `FTD-0827` | THEOREM, OPEN | Does the selected critical quartic clock itself supply the oriented rank-two conductor-32 CM gearbox that the BCC symmetric square loses? | L989 |
| `FTD-0828` | SYNTHESIS, THEOREM, CORRECTION, OPEN | What are the minimum requirements for a substrate-native clock, and can the substrate retain the clockwise/counterclockwise datum erased by BCC's… | L990 |
| `FTD-0829` | EXECUTION_INVALID, CORRECTION | Can the locked FTD-0774 tangent campaign be made adjudicable by repairing its periodic-Hodge normalization and preflight serialization without… | L991 |
| `FTD-0830` | EXECUTION_INVALID | Does stable four-pass reinsertion of the separately retained face harmonic close the FTD-0829 certificate boundary? | L992 |
| `FTD-0831` | MEASURED, EXECUTION_INVALID, OPEN | After replacing the impossible near-zero harmonic ratio by its declared binary64 backward-error floor, does the complete tangent preflight become… | L993 |
| `FTD-0832` | MEASURED, EXECUTION_INVALID, OPEN | Does an explicit electric harmonic coordinate and nonsingular complete-product chart finally adjudicate the locked L=17 tangent candidate? | L994 |
| `FTD-0833` | INVALID | Can the first FTD-0832 replay `NameError` be repaired without changing the verifier or any gate? | L995 |
| `FTD-0834` | INVALID | Can both v5 replay scope defects be repaired while leaving the producer, corpus, verifier, and gates unchanged? | L996 |
| `FTD-0835` | INVALID, THEOREM | Does the selected quartic clock admit a minimal bilateral self-dual energy representation whose weighted traversal is `sqrt(pi) G*`? | L997 |
| `FTD-0836` | THEOREM, IMPOSED, OPEN | What is the simplest exact self-dual recursive form of the selected quartic clock, and what does it say about `G*`? | L998 |
| `FTD-0837` | INVALID, THEOREM | Does a source-locked exact certificate show whether the frozen production core already contains the bilateral quartic clock dynamics? | L999 |
| `FTD-0838` | THEOREM, SELECTION | Which bilateral, restoring, and reservoir dynamics are absent from the frozen production clock core, and what is the minimum conditional repair? | L1000 |
| `FTD-0839` | THEOREM, CORRECTION, OPEN | Does `i` derive Gamma/`G*`, and can its complex square supply quarticity without losing clock orientation? | L1001 |
| `FTD-0840` | THEOREM, BOUNDARY, SELECTION | Does the retained canonical lift and a signed self-pair supply an exact stable recursion, and does that close the `G*` tick cadence? | L1002 |
| `FTD-0841` | THEOREM, BOUNDARY, SELECTION | Can the scalar self-pair mechanism be localized on the native vector flux type without choosing an axis, and what still prevents a physical local… | L1003 |
| `FTD-0842` | THEOREM, OPEN | Can the selected local self-pair quartic and the production spatial-gradient energy be closed in one exact local update, and does their positive… | L1004 |
| `FTD-0843` | THEOREM | Does a rank-one common/relative split give a positive P4-local quartic carrier? | L1005 |
| `FTD-0844` | THEOREM, SELECTION | Can complementary common propagation and relative quartic recurrence produce the simplest positive, local, stable two-channel clock carrier? | L1006 |
| `FTD-0845` | THEOREM | Can an exact source-locked discriminator classify the minimum conservative phase readout of the relative quartic carrier? | L1007 |
| `FTD-0846` | THEOREM, SELECTION | What phase information can a common/even pointer retain, and what is the lowest-degree positive local faithful readout that preserves critical… | L1008 |
| `FTD-0847` | THEOREM | Can the FTD-0846 continuous odd pointer be converted into a stable, local, loss-booked ternary record? | L1009 |
| `FTD-0848` | THEOREM, SELECTION | What is the minimum symmetric polynomial ternary latch, and can its damping, switching work, persistence, and information loss be closed exactly? | L1010 |
| `FTD-0849` | THEOREM | Does a source-locked exact discriminator show that current production genesis, evaporation, damping, and ternary writes already realize the FTD-0848… | L1011 |
| `FTD-0850` | ENGINE_FACT, THEOREM, CLOSED_NEGATIVE, PARTIAL, OPEN | Which pieces of a ternary latch are already present in production, and what dynamics are still absent? | L1012 |
| `FTD-0851` | THEOREM, CONDITIONAL, CLOSED_NEGATIVE, SELECTION | What is the minimum receiver for a signed ternary erasure, and do current movement, annihilation exhaust, and event journaling implement it… | L1013 |
| `FTD-0852` | THEOREM, ENGINE_FACT, CLOSED_NEGATIVE, SELECTION | Can the odd event receiver propagate into causal history without overwriting earlier signs, and does production already supply that carrier? | L1014 |
| `FTD-0853` | THEOREM, SELECTION, CLOSED_NEGATIVE, OPEN | What is the minimum cubically symmetric local transaction that writes a signed positive-energy erasure into the dual relative field? | L1015 |
| `FTD-0854` | THEOREM | Does the first locked certificate establish diagnostic event energy and the cubic history-rail gearbox? | L1016 |
| `FTD-0855` | THEOREM, IMPOSED, SELECTION, CLOSED_NEGATIVE, OPEN | Can the adopted matter-energy decrement supply event `B`, and does the cubic shell itself form the recursively ready causal history rail? | L1017 |
| `FTD-0856` | THEOREM, SELECTION, ENGINE_FACT, CLOSED_NEGATIVE, OPEN | What is the minimum deterministic reciprocal barrier between a protected ternary record and its causal field history? | L1018 |
| `FTD-0857` | THEOREM | Does the first locked certificate establish native event acceptance and protected characteristic propagation? | L1019 |
| `FTD-0858` | ENGINE_FACT, THEOREM, SELECTION, CLOSED_NEGATIVE, OPEN | Do source-native event predicates and the dual canonical pair close the physical reciprocal port, or expose the next type boundary? | L1020 |
| `FTD-0859` | THEOREM | Does the first locked certificate establish a target-blind relative action/orientation transducer and its faithfulness boundary? | L1021 |
| `FTD-0860` | THEOREM, SELECTION, CLOSED_NEGATIVE, OPEN | Can a nonzero relative canonical carrier absorb event energy without a ready port, and can one unlabelled pair faithfully retain the erased event? | L1022 |
| `FTD-0861` | THEOREM | Does the first locked phase-referenced rail certificate establish exact signed-event recovery and bounded export? | L1023 |
| `FTD-0862` | THEOREM, SELECTION, CLOSED_NEGATIVE, OPEN | Can a prepared phase standard turn the quarter-turn pump into a faithful, recursively reusable and bounded causal event carrier? | L1024 |
| `FTD-0863` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the selected phase standard orient a separate initially-zero signal so event energy transfers reciprocally without spending or double counting… | L1025 |
| `FTD-0864` | THEOREM | Does the first locked clock-gated Hamiltonian certificate establish the exact swap lift and nonlinear-clock boundary? | L1026 |
| `FTD-0865` | THEOREM, IMPOSED, CLOSED_NEGATIVE, OPEN | Can the reciprocal event exchange be generated by a closed clock-gated Hamiltonian, and can the same uncompensated quartic clock remain an exact… | L1027 |
| `FTD-0866` | THEOREM | Does the first locked ternary-clutch certificate establish the one-shot handshake? | L1028 |
| `FTD-0867` | THEOREM, IMPOSED, OPEN | Can the existing ternary latch supply dynamic hold/exchange eligibility while the outgoing signal preserves the signed event across a gate-zero reset… | L1029 |
| `FTD-0868` | THEOREM | Does the first locked signal-acknowledged two-stroke certificate establish finite-time recursive reset? | L1030 |
| `FTD-0869` | THEOREM, IMPOSED, OPEN | Can completed local signal formation acknowledge its own event, reset the latch exactly, and return the reference controller ready within one… | L1031 |
| `FTD-0870` | THEOREM | Does the first locked certificate establish reversible actual-layer signal uncomputation? | L1033 |
| `FTD-0871` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the completed oriented signal reversibly reset the actual ternary latch without a new acknowledgement bit, reset-history trit, or bath? | L1034 |
| `FTD-0872` | THEOREM, CLOSED_NEGATIVE, OPEN | What is the minimum reversible actual-layer permutation that transfers a ternary latch into its oriented output, and can output backpressure be… | L1035 |
| `FTD-0873` | THEOREM, IMPOSED, CLOSED_NEGATIVE, OPEN | Can the oriented ternary quarter-turn be realized as a minimum autonomous Hamiltonian phase gate with an explicit physical scale and complete… | L1036 |
| `FTD-0874` | THEOREM, SELECTION, OPEN | Can existing tick and coordinate parity schedule the oriented ternary quarter-turn into a causal reversible one-shot record rail, and what does… | L1037 |
| `FTD-0875` | THEOREM, SELECTION, IMPOSED, OPEN | Does the alternating scalar parity rail admit a fixed symplectic structure, and what is the minimum local canonical Hamiltonian lift with an exact… | L1038 |
| `FTD-0876` | THEOREM, ENGINE_FACT, CLOSED_NEGATIVE, OPEN | Does the production flux/wave-velocity state already supply the local canonical carrier type required by FTD-0875, and is the complete production… | L1039 |
| `FTD-0877` | THEOREM | Does the first locked matched Gauss-record certificate establish the constrained canonical reduction and live-projector boundary? | L1040 |
| `FTD-0878` | THEOREM | Does the first verifier-only repair validate the FTD-0877 certificate? | L1041 |
| `FTD-0879` | THEOREM | Does the bare-phrase comment-marker repair validate FTD-0878? | L1042 |
| `FTD-0880` | THEOREM, CLOSED_NEGATIVE, OPEN | What exact canonical record structure does matched Gauss incidence supply, what locality/loss boundary follows, and does the live cell-centred… | L1043 |
| `FTD-0881` | THEOREM | Does the frozen reversible checkerboard Gauss-preparation certificate execute validly? | L1044 |
| `FTD-0882` | THEOREM, CLOSED_NEGATIVE, SELECTION, OPEN | Can local reversible dynamics prepare the matched minimum-energy Gauss record without evaluating the inverse Laplacian in a gate, and what exact… | L1045 |
| `FTD-0883` | THEOREM | Does the frozen finite-port rail and positive source-battery certificate execute validly? | L1046 |
| `FTD-0884` | THEOREM, CLOSED_NEGATIVE, IMPOSED, OPEN | What finite ready-port capacity and positive source-work law can complete FTD-0882 without claiming impossible universal recycling? | L1047 |
| `FTD-0885` | THEOREM | Does the first locked canonical source-centered Gauss-gate certificate execute validly? | L1048 |
| `FTD-0886` | THEOREM, CLOSED_NEGATIVE, BOUNDARY, IMPOSED, OPEN | What phase-complete canonical structure pays the Gauss source work, and does the FTD-0884 square-root battery survive away from its one-amplitude… | L1049 |
| `FTD-0887` | THEOREM | Does the first locked autonomous phase-parity/source-reaction certificate execute validly? | L1050 |
| `FTD-0888` | THEOREM, CLOSED_NEGATIVE, SELECTION, IMPOSED, OPEN | Can one autonomous Hamiltonian compile alternating checkerboard layers, and what minimum positive channel permits source reaction without violating… | L1051 |
| `FTD-0889` | THEOREM | Does the first locked cubic reaction-vector/source-transport certificate execute validly? | L1052 |
| `FTD-0890` | THEOREM, CLOSED_NEGATIVE, SELECTION, IMPOSED, OPEN | Can a scalar reaction choose spatial recoil, and what exact source-transport gearbox follows once a local vector and the selected relativistic… | L1053 |
| `FTD-0891` | THEOREM | Does the first locked collective-triplet/inertial-curvature certificate execute validly? | L1054 |
| `FTD-0892` | THEOREM, CLOSED_NEGATIVE, OPEN, SELECTION, IMPOSED | Does the selected constituent common-action phase space supply the required reaction triplet, and can static stability determine its inertial mass? | L1055 |
| `FTD-0893` | THEOREM, CLOSED_NEGATIVE, OPEN, IMPOSED | What exact object determines the inertia of a dynamically dressed matter-field state, and does the selected common action already supply it? | L1056 |
| `FTD-0894` | THEOREM | Does the first locked Bloch-quasimomentum lift/local momentum-map trilemma certificate execute validly? | L1057 |
| `FTD-0895` | THEOREM, CORRECTION | Does the scoped four-expression repair close the FTD-0894 certificate without changing its mathematics or scope? | L1058 |
| `FTD-0896` | THEOREM, OPEN, IMPOSED | Can the native integer-translation/Bloch structure supply the globally real additive physical momentum map required by FTD-0893 while remaining… | L1059 |
| `FTD-0897` | THEOREM, OPEN | What minimum local update retains reciprocal-zone information during an equal-and-opposite pair interaction, and does it already supply physical… | L1060 |
| `FTD-0898` | THEOREM, OPEN | Can the selected local relative-quartic recursion generate the FTD-0897 impulse, carry positive energy and the `G*` period factor in one exact… | L1061 |
| `FTD-0899` | THEOREM | Does the first locked common/relative connection and momentum-gearbox certificate execute validly? | L1062 |
| `FTD-0900` | THEOREM, CORRECTION | Does the one-symbol component-range repair close the FTD-0899 certificate without changing its mathematics or scope? | L1063 |
| `FTD-0901` | THEOREM, OPEN, BOUNDARY | Can one minimum common/relative action transfer mechanical common impulse while preserving full energy and total momentum, and can its continuously… | L1064 |
| `FTD-0902` | THEOREM | Does the first locked positive-connection order/self-pair critical-clock certificate execute validly? | L1065 |
| `FTD-0903` | THEOREM, BOUNDARY, OPEN | Can a positive common/relative connection preserve the critical quartic while exchanging mechanical impulse, and what boundary remains? | L1066 |
| `FTD-0904` | THEOREM, BOUNDARY, OPEN | Can the exact rest-sector self-pair clock rectify without an externally timed clutch, and what orientation information is necessary? | L1067 |
| `FTD-0905` | THEOREM | Can the existing native ternary/flux types represent the polar axis and time-odd orientation memory required by FTD-0904, and can one central mode… | L1068 |
| `FTD-0906` | THEOREM | Does whitespace normalization alone repair the FTD-0905 exact source-marker failure? | L1069 |
| `FTD-0907` | THEOREM, CORRECTION, BOUNDARY, OPEN | What is the minimum native-type polar/chiral memory, is it stable under a central recursive law, and can it be the same exact G* clock mode? | L1070 |
| `FTD-0908` | MEASURED, OPEN | Does the unchanged production tick form and retain the FTD-0907 neutral-dipole/time-odd phase-wedge observables beyond a transient? | L1071 |
| `FTD-0909` | INFRASTRUCTURE | Is the FTD-0908 production census instrument fixed and independently adjudicable before its first data? | L1072 |
| `FTD-0910` | MEASURED, BOUNDARY, OPEN | What does FTD-0908 Outcome A establish once chirality flips and the frozen randomized null are audited? | L1073 |
| `FTD-0911` | CLOSED_NEGATIVE | Does actual endpoint pairing retain chirality beyond every fixed matched derangement, and is the FTD-0907 exact central map present in held-out… | L1074 |
| `FTD-0912` | INFRASTRUCTURE | Is the FTD-0911 held-out pair/centrality instrument fixed and independently reconstructible before data? | L1075 |
| `FTD-0913` | CLOSED_NEGATIVE, OPEN | What is the held-out disposition of the two-endpoint wedge as natural recursive memory? | L1076 |
| `FTD-0914` | THEOREM, BOUNDARY, CLOSED_NEGATIVE, OPEN | Can the minimum closed cardinal loop realize `i`, retain clockwise/counterclockwise direction, and provide natural recursive memory? | L1077 |
| `FTD-0915` | MEASURED, CLOSED_NEGATIVE, OPEN | Does unchanged production form and run the exact identity-bearing ternary plaquette through a full oriented four-step cycle? | L1078 |
| `FTD-0916` | EXECUTION_INVALID | Was the first production plaquette-recurrence corpus independently reconstructible? | L1079 |
| `FTD-0917` | UNKNOWN | Can raw-site telemetry repair FTD-0916 without changing any physics or adjudication gate? | L1080 |
| `FTD-0918` | THEOREM, ENGINE_FACT, CLOSED_NEGATIVE, OPEN | Does the native flux/wave pair carry the missing plaquette handedness charge, and is one elementary plaquette a closed production rotor? | L1081 |
| `FTD-0919` | THEOREM, CLOSED_NEGATIVE, OPEN | Does the unchanged free-field action contain a larger exact `C4` circulation carrier that can be localized as a finite clock body? | L1082 |
| `FTD-0920` | THEOREM, CLOSED_NEGATIVE, CONDITIONAL, OPEN | Can the unchanged reciprocal density/current source supply the unique boundary return that closes the elementary native `C4` plaquette? | L1083 |
| `FTD-0921` | CLOSED_NEGATIVE, THEOREM, OPEN | Does the Moore-coated plaquette return have a compact Hodge preimage, and can a compact transverse return be compiled through the live `j=s v` source… | L1084 |
| `FTD-0922` | EXECUTION_INVALID, THEOREM | Can an exact outside-band evanescent field profile and source-locked `C4` recurrence be constructed from a compact ternary dipole core? | L1085 |
| `FTD-0923` | THEOREM, EXACT, CLOSED_NEGATIVE, OPEN | Does the exact uncontained-versus-periodic domain repair validate the FTD-0922 evanescent reference orbit without changing its physics or outcome… | L1086 |
| `FTD-0924` | THEOREM, NO_GO, CLOSED_NEGATIVE, OPEN | Do the paired ternary-dipole transitions admit an exact compact central current, and can that current be compiled through the unchanged live… | L1087 |
| `FTD-0925` | THEOREM, OPEN | Can the FTD-0924 bridge current be compiled into a finite neutral ternary scaffold with causal production velocities, exact live continuity, and a… | L1088 |
| `FTD-0926` | THEOREM, POSITIVE, OPEN | Can the existing subcell remainder and velocity autonomously generate the FTD-0925 current orbit through one homogeneous local positive Hamiltonian? | L1089 |
| `FTD-0927` | THEOREM, NO_GO, OPEN | Can present-state central continuity generate the complete ternary record and exact midpoint field source, and can the same minimum canonical… | L1090 |
| `FTD-0928` | THEOREM, POSITIVE, CONDITIONAL, OPEN | Can a discrete generating function repair the frozen one-way recurrence; if not, what is the minimum positive reciprocal self-dual action, and what… | L1091 |
| `FTD-0929` | THEOREM, POSITIVE, OPEN | Is the FTD-0928 field-shaped companion locally determined by the present source, can it form in finite causal depth, and what information/energy… | L1092 |
| `FTD-0930` | THEOREM, RESOLVED, OPEN | Can the FTD-0929 target-blind field preparation be made locally canonical and positive, what is its minimum fresh-port cost, and does the same… | L1093 |
| `FTD-0931` | THEOREM, RESOLVED, OPEN | Can the existing native flux/momentum pair form the fixed-source massless static halo causally without damping or an indefinite fresh-port stream… | L1094 |
| `FTD-0932` | THEOREM, NO_GO, RESOLVED, OPEN | Does temporal quarter-turn phase `z=i` place the fixed-center C4 source in a native spectral gap that permits causal companion tracking, and can a… | L1095 |
| `FTD-0933` | THEOREM, POSITIVE, NO_GO, BOUNDARY, OPEN | What exact field mismatch is left by one abrupt integer relocation of a formed C4 source, does the native field re-dress the new center causally, and… | L1096 |
| `FTD-0934` | THEOREM, NO_GO, BOUNDARY, RESOLVED, OPEN | Does the C4 hop wake define a natural translation-group geometry, why can that energy not select a directed recoil, and what minimum representation… | L1097 |
| `FTD-0935` | THEOREM, CONDITIONAL, OPEN | Can existing native data realize the directed compact character required by FTD-0934, is the minimum cubic integer-linear gearbox unique, and which… | L1098 |
| `FTD-0936` | CORRECTION, THEOREM, NO_GO, OPEN | Does the raw FTD-0935 C4 character always retain direction, what canonical repair removes its parity kernel, and does the formed compact body carry… | L1100 |
| `FTD-0938` | THEOREM, POSITIVE, PARTIAL, OPEN | Does the live primitive body current determine the minimum critical-quartic-preserving connection, can its closed source cycle be directly composed… | L1101 |
| `FTD-0940` | THEOREM, DERIVED, NO_GO, OPEN | Can the actual ternary record distinguish signed field-charge transport from the directed center transport of a neutral C4 body, and does that split… | L1102 |
| `FTD-0941` | THEOREM, CLOSED_NEGATIVE, OPEN | Can exact unbounded occupancy winding be retained by fixed bounded finite-alphabet hardware, how do cumulative flux/body labels/link carry compare… | L1103 |
| `FTD-0942` | THEOREM, CLOSED_NEGATIVE, CORRECTION, DERIVED, OPEN | Do the existing production L/R real fields and update laws already realize FTD-0941's reversible collision-separated occupancy-history carrier, and… | L1104 |
| `FTD-0943` | THEOREM, CLOSED_NEGATIVE, OPEN | Does the isolated production C18 relative canonical pair admit an exact finite-range characteristic diagonalization or any nonzero finite-support… | L1105 |
| `FTD-0945` | THEOREM, CLOSED_NEGATIVE, OPEN | Do the existing event-mediated production actions autonomously write and transport a reversible relative L/R direction/history record after the… | L1106 |
| `FTD-0948` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the existing relative canonical vector support a degree-minimum bounded nonlinear recursive charge, and can a natural signed occupancy event… | L1107 |
| `FTD-0949` | THEOREM, OPEN | Does the selected C18-coupled relative-field sextic admit a finite-energy exponentially localized recursive solution on the uncontained substrate… | L1108 |
| `FTD-0951` | THEOREM, OPEN | Can compact core data causally construct finite-radius approximants to the exact FTD-0949 tailed body without reading that target, while retaining… | L1109 |
| `FTD-0953` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the nonlinear FTD-0949 body relax through a positive local canonical environment, and is finite positive action capacity sufficient to close… | L1110 |
| `FTD-0954` | THEOREM, CLOSED_NEGATIVE, POSITIVE, OPEN | Can the FTD-0953 positive nonlinear Routh port be completed by a local phase-reacting canonical transfer that conserves physical axial charge and… | L1111 |
| `FTD-0955` | THEOREM, CLOSED_NEGATIVE, OPEN | Can one globally periodic autonomous controller compile the phase-reacting charge transfer and positive nonlinear Routh-port update, return… | L1112 |
| `FTD-0957` | SELECTION, THEOREM, BOUNDARY, OPEN | Can a minimum globally periodic relative-action-curvature law turn the FTD-0955 neutral phase into a positive stable recursive sector while… | L1113 |
| `FTD-0959` | CLOSED_NEGATIVE, SELECTION, THEOREM, OPEN | Can the stable relative-phase sector be exactly isochronous and release an independently rotating controller at every crossing without a phase lift… | L1114 |
| `FTD-0961` | THEOREM, CLOSED_NEGATIVE, BOUNDARY, OPEN | Can the signed FTD-0959 crossing history and lifted winding be realized by already-booked oriented ternary/history rails without adding another… | L1115 |
| `FTD-0963` | SELECTION, THEOREM, OPEN | Can an oriented local clock traversal itself load the existing signed history port and actively align a controller without a discontinuous sign… | L1116 |
| `FTD-0965` | THEOREM, CLOSED_NEGATIVE, CORRECTION, OPEN | Can the exact selected FTD-0963 connection be represented and evolved by the unchanged production state/tick without adding a public degree of… | L1117 |
| `FTD-0969` | THEOREM, BOUNDARY, OPEN | Can a finite neutral actual ternary body derive the regional polar frame, spatial pseudoscalar, and improper-covariant transverse complex structure… | L1118 |
| `FTD-0970` | THEOREM, CLOSED_NEGATIVE, OPEN | Does the FTD-0969 regional body frame determine the missing canonical moving-frame reaction, and can that induced connection itself realize the… | L1119 |
| `FTD-0972` | THEOREM, BOUNDARY, OPEN | Can the exact `kappa=0` body degeneracy plus one ternary latch retain both crossing directions reversibly, and does the resulting recursive… | L1120 |
| `FTD-0974` | THEOREM, SELECTION, OPEN | Does the retained C4 carrier uniquely determine a reversible field coupling, and what is the minimum positive canonical suspension when one faithful… | L1121 |
| `FTD-0975` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the FTD-0974 two-pair suspension coexist with the five-pair FTD-0963 gearbox inside the six existing production pairs without double-booking the… | L1122 |
| `FTD-0977` | THEOREM, CORRECTION, BOUNDARY, OPEN | Does the retained C4 carrier plus one physical clock force the proposed merged connection law, including its common profile and active physical… | L1123 |
| `FTD-0978` | THEOREM, CORRECTION, BOUNDARY, CLOSED_NEGATIVE, OPEN | Does the unchanged production de Broglie-clock plus weak-transmutation route implement the clock-indexed oriented C4 twist required by FTD-0977? | L1124 |
| `FTD-0980` | THEOREM, BOUNDARY, SELECTION, OPEN | What is the minimum clockwise/counterclockwise square root of the production left/right half-turn, and can it be local, exactly energy preserving… | L1126 |
| `FTD-0982` | THEOREM, POSITIVE, SELECTION, OPEN | Does an exact finite-range multicomponent factor of the C18 stiffness eliminate the nonlocality of the instantaneous oriented root, and if not what… | L1128 |
| `FTD-0985` | THEOREM, BOUNDARY, CONDITIONAL, OPEN | Can the single physical clock/action pair service arbitrarily separated same-tick work events, or does Moore locality require locally owned… | L1129 |
| `FTD-0987` | THEOREM, CLOSED_NEGATIVE, SELECTION | Does the unused sixth production pair supply the native local work hardware, and what additional law is required to make it an owned reserve? | L1130 |
| `FTD-0989` | THEOREM, CORRECTION, SELECTION | Can the exact C18 incidence channels realize local regional ownership, current, reciprocal switching, and the correct physical work-action… | L1131 |
| `FTD-0990` | THEOREM, SELECTION, OPEN | Does the actual ternary state already supply the static work membrane, and can the dual substrate separate a protected recursive body clock from an… | L1132 |
| `FTD-0992` | THEOREM, CLOSED_NEGATIVE, OPEN | Does the occupancy membrane determine its own local formation/reversal work, and what is the minimum fail-closed active aperture retaining temporal… | L1133 |
| `FTD-0994` | THEOREM, CLOSED_NEGATIVE, CONDITIONAL, OPEN | Can released local membrane work and retained orientation start a zero Cartesian clock pair, and can the resulting phase be written uniformly into an… | L1134 |
| `FTD-0996` | THEOREM, OPEN | What exact local law lets formation work grow a coherent body clock, and does a matched critical-quartic receiver inherit the G* calendar without… | L1135 |
| `FTD-0997` | THEOREM, CLOSED_NEGATIVE, OPEN | Can the existing relative L-R pair supply the phase-complete machine for recursive common-clock growth, and can the static membrane refill it at a… | L1136 |
| `FTD-0999` | THEOREM, BOUNDARY, OPEN | What exact cumulative resource law governs coherent clock growth, and what concurrency, finite-reserve, causal-delay, and inverse constraints follow… | L1137 |
| `FTD-1001` | THEOREM | Do the FTD-0993/0994, FTD-0995/0996, and FTD-0998/0999 certificates remain valid under the 2026-08-13 documentation-only source amendments? | L1139 |
| `FTD-1004` | CLOSED_NEGATIVE, DERIVED, OPEN | Does a finite integer unit-strut tensegrity realize native critical quarticity within the first declared exact-certificate scope (clock-minimum spec… | L1142 |
| `FTD-1005` | THEOREM, OBSERVATION, OPEN | Is the Diophantine kill pattern behind FTD-1004 a theorem, and what arithmetic must any surviving axial candidate use? | L1143 |
| `FTD-1006` | CLOSED_NEGATIVE, THEOREM, OPEN | Does any strutted axially-symmetric unit-strut tensegrity exist under the registered law? | L1144 |
| `FTD-1007` | SELECTION, THEOREM, OPEN | Is the compact matter law's binary polarity domain extended to the substrate's own ternary state set? | L1145 |
| `FTD-1008` | CLOSED_NEGATIVE, OPEN | What do the first preregistered decisions in the ternary-mask sector return? | L1146 |

---

## Meta — papers, tooling, project process

*Paper splits and referee rounds, monographs, node maps and synonymy graphs, trackers, pre-registration registries, project policy.*

**17 claims.**

| ID | Epistemic tag | Claim | LEDGER |
|---|---|---|---:|
| `FTD-0042` | RETRACTED | Yang-Mills mass gap "proof" (FTD_Yang_Mills_Mass_Gap.tex) | L261 |
| `FTD-0043` | RETRACTED | Navier-Stokes regularity "proof" (FTD_Navier_Stokes.tex) | L262 |
| `FTD-0044` | THEOREM | Per-voxel mass gap from manifestation threshold (Theorem 5.1 of retracted YM paper) | L263 |
| `FTD-0045` | HYPOTHESIS | α_largeL ≈ 3.6 × α_ref (engine measurement at largest tested L, under a_phys ≡ ℓ_P) | L264 |
| `FTD-0046` | RETRACTED | FTD_Thermodynamic_Limit (PDF-only, no TeX source) | L265 |
| `FTD-0047` | RETRACTED | DERIV_THERMODYNAMIC_REFLEXION (PDF-only, no TeX source) | L266 |
| `FTD-0048` | RETRACTED | 11 PDF-only papers without recoverable TeX source | L267 |
| `FTD-0049` | POLICY | Project commit-attribution policy: no AI co-author trailers | L268 |
| `FTD-0171` | STRUCTURAL | Paper split per referee report 2026-05-19: PAPER_GSTAR_INTRODUCTION (28pp, math-journal-target, Duke/JAMS/Crelle/Compositio) + PAPER_GSTAR_FTD_BRIDGE… | L218 |
| `FTD-0172` | STRUCTURAL | Round-2 referee polish: residual FTD content in Paper A §16.6 / §16.7 / line 568 fully extracted to Paper B; Paper A title rewritten ('The Kronecker… | L219 |
| `FTD-0173` | UNKNOWN | Round-3 referee verification + cross-reference polish: critic verified all six round-2 blockers as CLOSED, identified three minor cross-reference… | L220 |
| `FTD-0174` | CORRECTION | Ivy League red-team (4 specialists in parallel): CM-theorist, prose editor, consistency auditor, philosopher of physics. CRITICAL: L(E_lemn, 1) =… | L221 |
| `FTD-0202` | INFRASTRUCTURE | Synonymy graph + identity-priority roadmap (C4 navigation tool) -- bipartite graph extracted from `verify_gstar_paper.py`'s 100 distinct `check()`… | L385 |
| `FTD-0207` | INFRASTRUCTURE | FTD math node map -- multi-layer connectivity graph (objects identities spine theorems LEDGER claims) rendered in four complementary output formats… | L380 |
| `FTD-0248` | CONJECTURE, SELECTION, THEOREM | Epistemic Symmetries and Chiral Trajectories | L411 |
| `FTD-0249` | SYNTHESIS, AXIOM | FTD construction monograph — canonical bottom-up construction story of FTD's mathematics (front-door synthesis) | L412 |
| `FTD-0818` | SYNTHESIS | Consolidated dissemination: the semantic-ontology manuscript. | L980 |

---

## Tag frequency across all rows

Canonical tags via `ledger_parser.TAG_NORMALISATION` — the same normaliser that feeds `math_node_map.json`. A row carrying several tags is counted under each, so the total exceeds the row count.

| Tag | Rows |
|---|---:|
| THEOREM | 433 |
| CLOSED_NEGATIVE | 244 |
| OPEN | 242 |
| MEASURED | 185 |
| SELECTION | 110 |
| DERIVED | 102 |
| SMC | 84 |
| SYNTHESIS | 68 |
| NUMERICAL_FACT | 54 |
| CONSTRUCTIVE | 48 |
| FOUNDATIONAL_OBSTRUCTION | 43 |
| CORRECTION | 41 |
| BOUNDARY | 36 |
| IMPOSED | 33 |
| CONJECTURE | 31 |
| EXECUTION_INVALID | 30 |
| SELECTED_DYNAMICS | 29 |
| AXIOM | 28 |
| NO_GO | 23 |
| PARAMETRIC | 20 |
| RETRACTED | 20 |
| PARTIAL | 18 |
| POSITIVE | 17 |
| CONDITIONAL | 16 |
| EXACT | 15 |
| RECONCILIATION | 14 |
| OBSERVATION | 13 |
| RESOLVED | 12 |
| REFUTATION | 12 |
| ENGINE_FACT | 12 |

