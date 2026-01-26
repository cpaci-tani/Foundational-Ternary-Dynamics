# FTD Manuscript: Cross-Domain Weaknesses Compilation

**Compiled by**: CROSS-POLLINATION COMPILER
**Date**: 2026-01-25
**Source**: 23 Expert Reviews

---

## Summary Statistics

| Severity | Count |
|----------|-------|
| CRITICAL | 31 |
| MAJOR | 47 |
| MINOR | 38 |
| Total Issues | 116 |

---

## I. CRITICAL ISSUES (Must Address Before Publication)

### A. Physics - Foundational Gaps

#### PHYS-PART-C1: Gauge Group Emergence Not Demonstrated
**Reviewer**: PHYS-PART | **Severity**: CRITICAL

The manuscript claims "SU(2) from ternary state structure, SU(3) from three spatial dimensions" but provides no rigorous derivation showing:
1. How exactly ternary states {-1, 0, +1} generate non-Abelian SU(2) gauge structure
2. Why three spatial dimensions yield SU(3) rather than some other group
3. How anomaly cancellation emerges

**Location**: Throughout; most acute in 2.3 and 2.7

---

#### PHYS-PART-C2: Quark Mass Formulas Are Numerology
**Reviewer**: PHYS-PART | **Severity**: CRITICAL

Quark mass relations use arbitrary constants (19, 15, 64) without physical motivation:
- Up: N_base + sin^2(theta_W) = 4.231
- Charm: N_eff(b_7+N_c)(19) + 15 = 2485
- Top: (phi^2 - 64*alpha) * m_W

These have no connecting principle and suggest post-hoc fitting.

**Location**: 14.4-particle-catalog.qmd, lines 25-33

---

#### PHYS-QFT-C1: Absence of Renormalization Theory
**Reviewer**: PHYS-QFT | **Severity**: CRITICAL

The manuscript makes no serious attempt to address renormalization:
- No mention of running couplings being scale-dependent
- Beta function coefficient b_3 = 7 quoted as fundamental but depends on particle content
- No discussion of counterterms, regularization, or RG flow

**Impact**: Without renormalization, numerical agreements may be coincidental.

---

#### PHYS-QFT-C2: Non-Abelian Gauge Structure Not Derived
**Reviewer**: PHYS-QFT | **Severity**: CRITICAL

Claims SU(2) from "chiral flux doublets" and SU(3) from "three spatial dimensions" are assertions, not derivations:
- No derivation of Lie algebra structure
- SU(3) is not SO(3) - conflation of color with spatial orientation
- No treatment of gluon self-interactions

---

#### PHYS-QFT-C3: Born Rule "Derivation" Is Circular
**Reviewer**: PHYS-QFT | **Severity**: CRITICAL

The argument for P = |psi|^2 is circular:
1. Flux field J defined
2. Complexification psi = J_x + iJ_y performed
3. Manifestation probability claimed to follow |psi|^2

Why |psi|^2 rather than |psi| or |psi|^4? No justification provided.

**Location**: 2.4-quantum-phenomena.qmd, lines 66-88

---

#### PHYS-GR-C1: Diffeomorphism Invariance Violation
**Reviewer**: PHYS-GR | **Severity**: CRITICAL

The manuscript claims Einstein equations emerge while acknowledging lattice breaks diffeomorphism invariance - a fundamental contradiction. The 8piG coefficient is not derived from first principles; it appears inserted by hand.

**Location**: 1.12 (lines 183-210), 7.1 (lines 95-157)

---

#### MATH-FOUND-C1: Master Quadratic "Derivation" Incomplete
**Reviewer**: MATH-FOUND | **Severity**: CRITICAL

The polarization term Pi(x) = 16(G*)^3/x is asserted via "Modular Covariance" but no actual proof is provided. The derivation is circular: G* from Lemniscate-Alpha curve, but curve construction uses G*.

**Location**: 1.10b, Step 6

---

#### MATH-FOUND-C2: Fermat Encoding Claims Are Misleading
**Reviewer**: MATH-FOUND | **Severity**: CRITICAL

The claim that the quadratic form is "derived from Fermat's Last Theorem" is mathematically questionable:
- Degree 2 selection is post-hoc rationalization
- 4^2 = 2^4 = 16 is an elementary identity, not "four independent constraints"
- Frey curve connection is metaphorical, not mathematical

**Recommendation**: Remove or substantially revise Chapter 1.10a

---

#### INFO-THEORY-C1: "Logic Gate" Definition Never Formalized
**Reviewer**: INFO-THEORY | **Severity**: CRITICAL

The computational hierarchy relies on counting "logic gates" but:
- No formal definition of what constitutes a gate in physical substrate
- No criterion for identifying gates in flux configurations
- The claim that 2+ gates enables "inference" is asserted without proof

Fatal for the computational ontology thesis.

---

#### INFO-THEORY-C2: No Complexity Analysis of FTD Itself
**Reviewer**: INFO-THEORY | **Severity**: CRITICAL

The manuscript discusses complexity measures but never applies them to FTD:
- What is the Kolmogorov complexity of FTD's rules?
- What computational complexity class is FTD simulation?
- Is FTD in P, NP, PSPACE, or higher?

---

#### HIST-SCI-C1: Digital Physics Lineage Not Acknowledged
**Reviewer**: HIST-SCI | **Severity**: CRITICAL (D grade for this category)

FTD fails to acknowledge intellectual predecessors:
- Konrad Zuse "Rechnender Raum" (1969) - no citation
- Edward Fredkin Digital Physics - no citation
- Stephen Wolfram - bibliography only, not discussed
- Gerard 't Hooft cellular automaton interpretation - minimal discussion

---

### B. Philosophy - Conceptual Issues

#### LOGIC-FORM-C1: Epistemic/Ontological Category Error
**Reviewer**: LOGIC-FORM | **Severity**: CRITICAL

The proof that ternary valuation is necessary conflates epistemic necessity with ontological necessity. Epistemic categories (affirmed/negated/undetermined) do not automatically entail ontological states (matter/antimatter/void).

**Location**: 0.0-formal-logic.qmd, lines 99-111

---

#### PHIL-MIND-C1: Hard Problem Evaded, Not Addressed
**Reviewer**: PHIL-MIND | **Severity**: CRITICAL (D grade: 3/10)

The "deflationary reframing" is a category error. Equating qualia with representational content confuses explanandum with explanans. Standard objections (inverted qualia, Mary's Room, zombie conceivability) are not engaged.

**Location**: 12.5-consciousness-as-self-reference.qmd

---

#### PHIL-MIND-C2: Microtubule Section Is Pseudoscience
**Reviewer**: PHIL-MIND | **Severity**: SEVERE

The claim that microtubule 13-protofilament structure "matches" N_eff = 13 is:
1. Numerological - coincidental number match
2. Orch-OR is highly controversial
3. Predictions are untestable

Damages chapter credibility.

---

#### PHIL-MIND-C3: Heart-Brain Section Is Speculative
**Reviewer**: PHIL-MIND | **Severity**: SEVERE

HeartMath Institute is not peer-reviewed mainstream science. "Soul" definition as integral over heart-brain-microtubule coherence is philosophically naive.

---

### C. Technical - Accessibility & Structure

#### ACCESS-UX-C1: Missing Skip Navigation Link
**Reviewer**: ACCESS-UX | **Severity**: CRITICAL (WCAG 2.1 AA failure)

No skip-to-content link found in page structure. Required for screen reader users.

---

#### ACCESS-UX-C2: Math Accessibility Gaps
**Reviewer**: ACCESS-UX | **Severity**: CRITICAL (D+ grade)

MathJax accessibility is incomplete:
- No aria-label fallbacks for complex equations
- Screen readers may not properly announce mathematical content

---

#### STRUCT-NAV-C1: Chapter Numbering Anomaly
**Reviewer**: STRUCT-NAV | **Severity**: CRITICAL

Prolegomena chapter sequence is 0.0 -> 0.1 -> 0.3 -> 0.2. Chapter 0.3 appears before 0.2 in navigation, causing confusion.

---

### D. Astrophysics - Claim Validity

#### ASTRO-COSM-C1: "FTD Produces" Claims Unjustified
**Reviewer**: ASTRO-COSM | **Severity**: CRITICAL

"How FTD Produces X" boxes are interpretive relabelings, not derivations. The Jeans mass equation is conventional formula, not derived from lattice framework.

**Example**: "Stellar formation in FTD follows from flux dynamics" - this is renaming standard physics, not deriving it.

---

#### ASTRO-COSM-C2: Cloud-9 Interpretation Overstated
**Reviewer**: ASTRO-COSM | **Severity**: CRITICAL

Claiming Cloud-9 as "direct evidence" conflates correlation with causation. LCDM also explains spherical non-rotating halos via standard RELHIC physics.

---

#### ASTRO-COSM-C3: Dark Matter Claim Lacks Falsifiability
**Reviewer**: ASTRO-COSM | **Severity**: CRITICAL

DM = unmanifested flux is presented without falsifiable consequences. "No WIMP detection" supports FTD, but detection would require reinterpretation rather than falsification.

---

## II. MAJOR ISSUES (Should Address)

### A. Physics

#### PHYS-PART-M1: Neutrino Mass Mechanism Incomplete
**Severity**: MAJOR

Seesaw mechanism invoked but:
- m_D = m_tau * alpha is asserted without derivation
- No explanation why m_D couples to tau
- Right-handed Majorana scale from fitting, not framework

---

#### PHYS-PART-M2: CKM/PMNS Matrix Formulas Inconsistent
**Severity**: MAJOR

Mixing angle formulas use different ingredients eclectically:
- theta_13: sqrt(alpha * N_c)
- theta_12: sqrt(sin^2(theta_W) * (1-sin^2(theta_W))/2)
- theta_23: arctan(sqrt(a_4/a_8)) * (1 - alpha_s/2)

This suggests fitting rather than unified derivation.

---

#### PHYS-PART-M3: Running Couplings Treatment Superficial
**Severity**: MAJOR

alpha_s = 7/59 = 0.1186 is derived but:
- At what scale does this value apply?
- How does running emerge from framework?
- Asymptotic freedom derivation absent

---

#### PHYS-GR-M1: Dark Matter Mechanism Lacks Rigor
**Severity**: MAJOR

- No derivation of <0|J^2|0> != 0 from axioms
- No calculation showing 27% dark matter density
- rho_DM ~ K_B/r_coherence^3 is dimensional analysis, not derivation

---

#### PHYS-GR-M2: Inflation Derivation Incomplete
**Severity**: MAJOR

Inflaton identification with "mean flux amplitude" is ad hoc. Starobinsky potential asserted to emerge without showing actual derivation. E-folding prediction N_e ~ 56.3 falls ~4 e-folds short.

---

#### PHYS-GR-M3: Black Hole Thermodynamics Claims Unsubstantiated
**Severity**: MAJOR

- "Stretched horizon" borrowed from string theory
- Flux tunneling mechanism asserted but never calculated
- Page curve said to "follow automatically" without demonstration

---

#### PHYS-GR-M4: Dark Energy Formula Multiplicity
**Severity**: MAJOR

Two different formulas appear:
- Chapter 1.15: rho_Lambda = m_e^4 * alpha^16 * G*^2 (1.0%)
- Chapter 10.3: Lambda/Lambda_Planck = alpha^57 (0.16%)

Are these equivalent? Which is "the" FTD prediction?

---

#### PHYS-QFT-M1: Action Principle Does Not Determine Coefficients
**Severity**: MAJOR

Force derivations incomplete:
- Gravity coupling G_N not determined by action
- Yukawa mass term asserted not derived
- Coefficients from "master quadratic" numerology, not action

---

#### PHYS-QFT-M2: Higgs Mechanism Treatment Incomplete
**Severity**: MAJOR

- No explanation why Mexican hat potential arises from FTD
- VEV formula v = m_P * sqrt(2pi) * alpha^8 is numerology
- No Goldstone boson discussion
- Unitarity of massive gauge boson scattering not addressed

---

#### PHYS-QFT-M3: Proton Decay Prediction Not Robust
**Severity**: MAJOR

- GUT scale assumed from unification, not derived
- Uncertainty stated without error propagation
- Threshold corrections can shift M_GUT by orders of magnitude

---

### B. Mathematics

#### MATH-FOUND-M1: Circular Dependencies in Derivation Chain
**Severity**: MAJOR

```
Fibonacci constraints --> {7, 3, 13, 4} --> Master quadratic --> alpha, N_c
         ^                                                          |
         +----------------------------------------------------------+
                        (N_c determines constraints)
```

Self-consistency does not establish uniqueness or physical relevance.

---

#### MATH-FOUND-M2: Category Theory Framework Incomplete
**Severity**: MAJOR

- No category definition (what are objects? identity morphisms?)
- Tensor product operators require monoidal structure not specified
- Fixed-point condition is set-theoretic, not categorical

---

#### MATH-FOUND-M3: Statistical Concerns for Number Theory
**Severity**: MAJOR

The p < 10^-6 estimate for number theory connections is methodologically flawed:
- Look-elsewhere effect ignored
- Correlation among "independent" routes ignored
- Selection bias: framework integers chosen to match known values

---

### C. Philosophy

#### LOGIC-FORM-M1: No Axiom Independence Proof
**Severity**: MAJOR (B- grade)

No demonstration that remaining axioms are independent:
- Could "max speed = 1" derive from "local updates"?
- Is "discrete time" independent of "discrete space"?

---

#### LOGIC-FORM-M2: Master Quadratic Circularity
**Severity**: MAJOR

Framework integers "constrained" by requiring alpha to have correct value. This is constraint satisfaction, not derivation from first principles.

---

#### PHIL-ONTO-M1: Grounding Direction Ambiguous
**Severity**: MAJOR

Does abstract (events, constraints) ground concrete (lattice, flux), or vice versa? The direction is unclear, creating metaphysical confusion.

---

#### PHIL-ONTO-M2: Modal Necessity Types Conflated
**Severity**: MAJOR (C+ grade)

Manuscript moves between logical, metaphysical, physical, mathematical, and conditional necessity without adequate distinction.

---

#### PHIL-ONTO-M3: Historical Philosophy Engagement Shallow
**Severity**: MAJOR (C grade)

Missing engagement with:
- Process philosophy (Whitehead)
- Structural realism (Ladyman, French)
- Philosophy of time (A-theory/B-theory)
- Laws of nature debate (Humean/non-Humean)

---

#### PHIL-MIND-M1: Consciousness Quadratic Is Pseudoscience
**Severity**: MAJOR

k=0.5 giving complex roots for "consciousness regime" is:
- Not derived from any principle
- Arbitrary parameter choice
- No mechanism connecting complex roots to subjective experience

---

#### PHIL-MIND-M2: No Engagement with Major Consciousness Theories
**Severity**: MAJOR (D grade: 3/10)

IIT, GWT, HOT, and predictive processing not engaged despite obvious relevance.

---

### D. Technical Quality

#### CODE-QUAL-M1: Incomplete Code Coverage
**Severity**: MAJOR

Only 1 of 5 chapters with claimed derivations has executable verification code. Gravity, unification, proton decay, flavor physics chapters lack computational verification.

---

#### CODE-QUAL-M2: No CI/CD Pipeline
**Severity**: MAJOR

No GitHub Actions or similar CI to verify freeze integrity.

---

#### CITE-BIB-M1: Critical Missing Citations
**Severity**: MAJOR (C+ grade for completeness)

- No citations for elliptic integral theory in Ch. 1.10
- Bell (1964), Born (1926) in .bib but not cited where discussed
- Philosophy chapter has single citation in 850+ lines
- Missing: Ramanujan, Silverman & Tate, Cox, Zagier

---

#### MODERN-WEB-M1: No Structured Data
**Severity**: CRITICAL (D+ grade for SEO)

No JSON-LD, Open Graph, or Schema.org markup for academic content discovery.

---

#### VIS-DATA-M1: Referenced Figures Not Generated
**Severity**: MAJOR (B- grade for Diagram Clarity)

Many figure references point to non-existent images.

---

### E. Domain-Specific

#### MATERIALS-M1: FTD "Mechanisms" Are Relabelings
**Severity**: MAJOR

"Conductivity = flux propagates freely" is not a derivation of Ohm's Law. "Superconductivity = Cooper pairs form when flux-mediated attraction overcomes thermal disruption" is vocabulary substitution.

---

#### MATERIALS-M2: Missing Critical Content
**Severity**: MAJOR

- No critical exponents or universality classes
- No reciprocal space treatment
- No Bloch theorem for electronic properties
- No Ginzburg-Landau theory for superconductivity

---

#### CHEM-PHYS-M1: Electron Orbitals Not Derived
**Severity**: MAJOR (D+ grade)

- No Schrodinger equation derivation from FTD
- No simulation showing hydrogen-like spectra
- Hardcoded shell radii in simulation code

---

#### PLANETARY-M1: FTD Claims Exceed Demonstrations
**Severity**: MAJOR

Framework claims planetary physics "follows from" FTD without derivations.

---

#### PLANETARY-M2: Orbital Mechanics Deficient
**Severity**: MAJOR (D grade)

Kepler's laws and orbital mechanics poorly connected to FTD framework.

---

#### INFO-THEORY-M2: Continuous Flux Breaks CA Paradigm
**Severity**: MAJOR

CAs have discrete state spaces; continuous flux requires infinite precision. The hybrid is never formally characterized.

---

#### ASTRO-COSM-M1: Bell Violation Section Conflates Phenomena
**Severity**: MAJOR

Wang et al. 2025 path identity experiment demonstrates quantum indistinguishability, not "ontological unity" as claimed.

---

#### BIO-THEOR-M1: No Darwinian Evolution in Life Definition
**Severity**: CRITICAL

Life definition omits evolution - the defining characteristic of biological systems.

---

#### EDIT-TECH-M1: Grandiose Claims Undermine Credibility
**Severity**: MAJOR (C- grade for Tone)

"Theory of Everything Complete" framing may alienate scientific readers.

---

## III. MINOR ISSUES (Consider Addressing)

### Physics
- m1: Proton decay prediction lacks proper uncertainty quantification (PHYS-PART)
- m2: State assignments inconsistent for neutral fermions, charged bosons (PHYS-PART)
- m3: Composite particle masses not derived beyond proton/neutron (PHYS-PART)
- m4: Top quark mass formula includes unexplained coefficient 64 (PHYS-PART)
- m5: Gravitational wave chapter contains no FTD-specific content (PHYS-GR)
- m6: Multiple formulas for m_p/m_e ratio (PHYS-GR)
- m7: Missing error propagation throughout gravitational sections (PHYS-GR)
- m8: Spinor structure argument lacks concrete construction (PHYS-QFT)
- m9: Parity violation explanation is hand-wavy (PHYS-QFT)

### Mathematics
- m10: G* notation conflicts with standard usage in analytic number theory (MATH-FOUND)
- m11: "Effective dimension" formula not rigorously defined (MATH-FOUND)
- m12: Through-pattern composition table implies associativity not proven (MATH-FOUND)
- m13: Python code should include precision warnings for floating-point (MATH-FOUND)

### Philosophy
- m14: Self-model requirement may be too strong for bacteria (PHIL-MIND)
- m15: "Gates" metric unclear for biological systems (PHIL-MIND)
- m16: Noetic Mass concept underdeveloped (PHIL-MIND)
- m17: Weak Anthropic Principle incorrectly called "tautology" (PHIL-MIND)

### Technical
- m18: No lockfile for exact dependency versions (CODE-QUAL)
- m19: Python version not specified (CODE-QUAL)
- m20: Quarto version not documented (CODE-QUAL)
- m21: No input validation in embedded code (CODE-QUAL)
- m22: Freeze files show modifications suggesting drift from source (CODE-QUAL)
- m23: Duplicate BibTeX entry abbott2016 (CITE-BIB)
- m24: feynman1985 incorrectly uses @article instead of @book (CITE-BIB)
- m25: Color contrast ratio marginal in some callout boxes (ACCESS-UX)
- m26: Heading hierarchy violations in some chapters (ACCESS-UX)
- m27: Some tables lack proper scope attributes (ACCESS-UX)

### Domain-Specific
- m28: Hoyle resonance fine-tuning not connected to FTD (ASTRO-COSM)
- m29: Type Ia supernova Phillips relation not mentioned (ASTRO-COSM)
- m30: Final parsec problem left unresolved (ASTRO-COSM)
- m31: Missing error bars on DESI results (ASTRO-COSM)
- m32: QCD transition characterization incomplete at finite density (MATERIALS)
- m33: Temperature proxy definition has dimensional issues (MATERIALS)
- m34: Binding energy formula units unclear (CHEM-PHYS)
- m35: Asymmetry formula a_A = K_B/N_c gives wrong value (CHEM-PHYS)
- m36: Speed of causality = 1 not derived from neighborhood structure (INFO-THEORY)
- m37: Information conservation claim contradicts DECAY_RATE parameter (INFO-THEORY)
- m38: Phi (integrated information) never computed for any FTD configuration (INFO-THEORY)

---

## IV. CROSS-DOMAIN CONCERNS

### Circular Dependencies (Noted by 5+ reviewers)
Multiple reviewers identified the same circularity pattern:
- Framework integers -> master quadratic -> alpha
- But framework integers "constrained" to produce correct alpha
- Self-consistency is not derivation

### "Derivation" vs "Fitting" Conflation (Noted by 8+ reviewers)
- PHYS-PART, PHYS-QFT, PHYS-GR, MATH-FOUND, ASTRO-COSM, MATERIALS, CHEM-PHYS, INFO-THEORY
- Numerical agreements labeled as "derivations" when they are pattern-matching
- Post-hoc rationalization dressed as theoretical necessity

### Gauge Structure Gap (Noted by 4 reviewers)
- PHYS-PART, PHYS-QFT, MATH-FOUND, HIST-SCI
- Non-Abelian gauge emergence is asserted not demonstrated
- No derivation of SU(2) or SU(3) Lie algebra structure

### Historical Acknowledgment Deficit (Noted by 3 reviewers)
- HIST-SCI, PHIL-ONTO, CITE-BIB
- Digital physics pioneers not cited
- Pythagorean/numerological heritage not addressed
- Process philosophy parallels ignored

### Renormalization Absence (Noted by 3 reviewers)
- PHYS-QFT, PHYS-PART, PHYS-GR
- No treatment of running couplings as renormalized quantities
- Scale dependence of coupling constants not addressed

### Consciousness Treatment Issues (Noted by 2 reviewers)
- PHIL-MIND, PHIL-ONTO
- Hard problem evaded not addressed
- Metaphysical position unclear
- Microtubule/heart-brain sections damage credibility

---

## V. REVIEWER RECOMMENDATIONS BY PRIORITY

### Mandatory Before Publication

1. **Provide or disclaim gauge structure derivation** (PHYS-PART, PHYS-QFT)
2. **Address renormalization** (PHYS-QFT)
3. **Clarify "derivation" vs "fitting" throughout** (Multiple)
4. **Add skip navigation and fix math accessibility** (ACCESS-UX)
5. **Fix chapter numbering anomaly** (STRUCT-NAV)
6. **Add digital physics historical context** (HIST-SCI)
7. **Remove or revise microtubule/heart-brain sections** (PHIL-MIND)
8. **Add structured data markup** (MODERN-WEB)

### Strongly Recommended

1. **Unify mixing angle framework** (PHYS-PART)
2. **Specify renormalization scales** (PHYS-QFT)
3. **Complete inflation derivation or downgrade claims** (PHYS-GR)
4. **Provide axiom independence proofs** (LOGIC-FORM)
5. **Engage with major consciousness theories** (PHIL-MIND)
6. **Add computational verification to more chapters** (CODE-QUAL)
7. **Add missing critical citations** (CITE-BIB)
8. **Create dependency graph for theorem chain** (LOGIC-FORM)

### Recommended

1. **Derive quark masses or remove claims** (PHYS-PART)
2. **Calculate black hole information recovery** (PHYS-GR)
3. **Formalize "logic gate" in FTD terms** (INFO-THEORY)
4. **Add proper categorical foundations** (MATH-FOUND)
5. **Include critical exponents for phase transitions** (MATERIALS)
6. **Engage with process philosophy** (PHIL-ONTO)
7. **Add falsification table to each chapter** (ASTRO-COSM)

---

## VI. GRADE SUMMARY WITH WEAKEST AREAS

| Domain | Reviewer | Grade | Primary Weakness |
|--------|----------|-------|------------------|
| Particle Physics | PHYS-PART | B | Gauge emergence not demonstrated |
| QFT | PHYS-QFT | C | No renormalization theory |
| GR/Cosmology | PHYS-GR | C+ | Diffeomorphism invariance violation |
| Astrophysics | ASTRO-COSM | B | "FTD Produces" claims unjustified |
| Eschatology | ESCHAT | B+ | Information conservation needs qualification |
| Planetary | PLANETARY | C+ | Orbital mechanics deficient (D) |
| Math Foundations | MATH-FOUND | C+ | Proof rigor (D+) |
| Formal Logic | LOGIC-FORM | B+ | Axiom independence (B-) |
| Physical Chemistry | CHEM-PHYS | B | Electron orbitals not derived |
| Information Theory | INFO-THEORY | C+ | Cellular automata theory (D+) |
| Materials Science | MATERIALS | C+ | FTD integration rigor (F for derivation) |
| Ontology | PHIL-ONTO | B | Modal structure (C+) |
| Philosophy of Mind | PHIL-MIND | C+ | Hard problem (D) |
| Pedagogy | PEDA-STYLE | B | Accessibility (C+) |
| Technical Writing | EDIT-TECH | B-/C+ | Tone (C-) |
| Biology Theory | BIO-THEOR | C+ | Evolution integration (D) |
| History of Science | HIST-SCI | C+ | Digital physics lineage (D) |
| Structure/Navigation | STRUCT-NAV | B+ | Chapter numbering anomaly |
| Accessibility | ACCESS-UX | B- | Math accessibility (D+) |
| Web Standards | MODERN-WEB | B+ | SEO readiness (D+) |
| Visualization | VIS-DATA | B- | Diagram clarity (B-) |
| Code Quality | CODE-QUAL | B+ | Error handling (C+) |
| Bibliography | CITE-BIB | B- | Citation completeness (C+) |

---

## VII. FALSIFICATION RISK ASSESSMENT

The following predictions, if falsified, would seriously undermine FTD:

| Prediction | Falsification | Timeline | Risk |
|------------|---------------|----------|------|
| tau_p ~ 10^35 years | No decay seen by 10^36 yr | ~2050 | Medium |
| No WIMPs | WIMP detection | Ongoing | Low (many theories predict this) |
| r = 0.0033 | r outside 0.002-0.004 | CMB-S4 ~2030 | High |
| Normal neutrino hierarchy | Inverted hierarchy confirmed | DUNE ~2028 | High |
| w = -1 exactly | w significantly different | DESI 2026+ | Medium |
| alpha = 1/137.036 to 1 ppm | precision measurement incompatible | Ongoing | Very High |

---

*Compilation completed 2026-01-25*
*Source: 23 expert reviews in evaluation/expert_reviews/*
