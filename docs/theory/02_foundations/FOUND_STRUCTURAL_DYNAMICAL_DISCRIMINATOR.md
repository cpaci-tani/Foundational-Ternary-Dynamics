# FOUND -- The Structural / Dynamical Discriminator (Boundary Theorem, Stage 1)

**Tag:** [BOUNDARY THEOREM -- STAGE 1]. The discriminator (§2) is a pre-registered `[DEFINITION]`; the classification (§3) is `[OPEN]` -- its v1 pre-registered falsifier fired (see §5). No existing LEDGER claim is promoted or demoted.
**LEDGER row:** FTD-0186 (boundary-theorem program).
**Date:** 2026-05-20.
**Pre-registration:** `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md`, hash-locked at commit `75ebe56`, tag `preregister-structural-dynamical-discriminator-v1`, SHA256 `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`. The discriminator definition below was committed **before** this classification was run.
**Verifier:** `scripts/proofs/proof_structural_dynamical_partition.py`.

---

## §1 -- Purpose

This is Stage 1 of the boundary theorem (`.claude/plans/take-the-role-of-fancy-kahn.md`): the rigorous execution of FTD's #1 goal's second clause -- "establish what we cannot [derive]." It delivers (a) the **structural / dynamical discriminator** -- the formal line between what a discrete ontology fixes and what it does not -- and (b) the **classification** of the load-bearing epistemic record against that line.

---

## §2 -- The discriminator (pre-registered, locked)

Verbatim from the locked pre-registration §2. Every load-bearing quantity-**claim** is assigned exactly one class.

- **STRUCTURAL** -- value forced by the substrate's finite discrete data (automorphism-group orders, neighbour/shell counts, polyhedral-decomposition multiplicities, lattice dimension, exact spectra/determinants of finite operators): it lies in the countable algebraic set that data generates **(S1)** and is invariant under every free choice -- the `a_phys`/`K_B`/`t_phys` calibration, rescalings, initial conditions **(S2)**.
- **NON-UNIVERSAL DYNAMICAL** -- a dimensionless continuous parameter (coupling, dynamical-scale or mass ratio) whose value **fails (S1)**: not forced by the finite combinatorics; determining it needs an external input.
- **CALIBRATION-CONDITIONAL** -- a dimensional quantity whose value **fails (S2)**: depends on the calibration declarations.

Tie-break (locked): the discriminator classifies **claims, not symbols** -- a symbol with both a structural and a dynamical reading is entered as two claims.

---

## §3 -- The classification (decisive load-bearing set)

This pass classifies the **decisive set** the falsifier tests: the algebraic-spine theorems and the closed-negative derivation record. (The exhaustive every-row `LEDGER` + `CATALOG_PARAMETRIC_INSERTIONS` pass continues; the verifier script is built to extend. The decisive set is what the §4 falsifier turns on.)

### 3a -- Spine theorems -- expected STRUCTURAL

| Claim | Class | Why |
|---|---|---|
| FTD-0001 master quadratic polynomial + roots | STRUCTURAL | coefficients are 16 = \|Aut(E)\|^2 and G* (a finite-spectrum determinant ratio); algebraic; calibration-invariant |
| FTD-0002 G* = Gamma(1/4)/Gamma(3/4) identity | STRUCTURAL | zeta-determinant ratio of finite quarter-twisted spectra |
| FTD-0003 CM-curve uniqueness (class number 1) | STRUCTURAL | arithmetic-classification fact |
| FTD-0004 Phase G geometric Coulomb | STRUCTURAL | the V(r) *form* = lattice Poisson Green's function, fixed by lattice geometry, zero free parameters |
| FTD-0005 Phase J ultralocality (L=2) | STRUCTURAL | structural property of the finite partition function |
| FTD-0006 / 0007 coefficient 16 = \|Aut(E)\|^2 | STRUCTURAL | automorphism-group order squared -- pure combinatorics |
| FTD-0008 Moore integers {N_base=4, N_eff=13, b_3=7} | STRUCTURAL | neighbour/shell counts of the Moore neighbourhood |
| FTD-0009 charge conservation per tick | STRUCTURAL | conservation = the identity 0 = (-1)+(+1) |
| FTD-0010 D = 3 from \|Aut(E)\|^2 = 2^D(D-1)! | STRUCTURAL | combinatorial identity forcing the integer D |
| FTD-0011 Phase H coupling-scaling *relation* | STRUCTURAL | a structural relation (alpha_r scales with g_c^2), not a value-claim |
| FTD-0012 discriminant trichotomy (algebra) | STRUCTURAL | Delta < 0 / = 0 / > 0 is an algebraic fact |
| FTD-0154-0166 G*-opus theorems (block) | STRUCTURAL | modular-form / chi_-4 identities; algebraic, calibration-invariant |

**Result 3a: all spine `[THEOREM]`/`[DERIVED]` claims classify STRUCTURAL.** Falsifier criterion A2 satisfied.

### 3b -- Closed-negative derivation record

| Claim | Target quantity | Class of target | Kind |
|---|---|---|---|
| FTD-0058 Structure-2 scalar gauge ppb-closure | an alpha correction | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0031 g_c first-principles (all routes) | g_c (coupling) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0093 g_c as bridge-operator eigenvalue | g_c (coupling) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0025 confinement substrate-derivation | sigma (string tension) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0131 G_N = 1/(b_3+N_c)^2 as physical G_N | G_N (coupling) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0116 G*^2 as lattice Z-factor | Z (renormalization) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0094 L2 identity 2 m_e/alpha = 16 G*^2 | m_e/alpha relation | NON-UNIVERSAL DYNAMICAL | i |
| m_p/m_e 174-ppm derivation closure | m_p/m_e (mass ratio) | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0018-0021 sin^2 theta_W, alpha_s, PMNS (parametric) | mixing angles / couplings | NON-UNIVERSAL DYNAMICAL | i |
| FTD-0035 Mechanism gamma (a_phys) | a_phys (lattice->length) | CALIBRATION-CONDITIONAL | i |
| FTD-0034 a_phys no-go (Mech alpha-delta) | a_phys | CALIBRATION-CONDITIONAL | i |
| FTD-0096 mu-from-l_P (mass-unit) | mass-unit calibration | CALIBRATION-CONDITIONAL | i |
| FTD-0050 master quadratic as RG-step char. poly | the master quadratic itself | STRUCTURAL | **ii** |
| FTD-0164 chi_-4 -> P_G* arrow (exponents 2,3 from CM) | the (2,3) exponents | STRUCTURAL | **ii** |
| FTD-0183 N_base = 4 unification with Z[i]^x | N_base = 4 (integer) | STRUCTURAL | **ii** |

---

## §4 -- The finding

**Spine:** every spine theorem classifies STRUCTURAL (3a). Falsifier A2 holds.

**Closed-negatives split into two kinds:**

- **Type i -- failed derivation of a non-universal *value*.** Every closed-negative that attempted to derive a coupling, a mass ratio, a renormalization factor, a mixing angle, or a calibration constant targets a quantity classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL -- **without exception across the decisive set.** This is the boundary theorem's evidence base: it directly confirms the thesis *the discrete substrate does not fix non-universal dynamical values*.
- **Type ii -- failed derivation of a structural *provenance*.** FTD-0050, FTD-0164, FTD-0183 are closed-negatives, but their targets (the master quadratic, the (2,3) exponents, the integer N_base) classify STRUCTURAL. They did not fail to fix a dynamical value; they failed to find a *deeper structural origin* (an RG-step, a CM-theory arrow, a Z[i] unification) for an object that **is** structural and **does** stand (e.g. (2,3) is independently a `[THEOREM]`, FTD-0175). They concern a different question and are **outside the structural/dynamical-value axis**.

**The result, honestly stated:** the discriminator (§2) assigns a definite, defensible class to every quantity in the decisive set. But the §4 falsifier was pre-registered over *all* closed-negatives, and the partition is **not clean** under criterion A1 as locked: the type-ii closed-negatives (FTD-0050, FTD-0164, FTD-0183) are closed-negative derivation attempts whose targets classify STRUCTURAL, not dynamical. Restricted to the type-i closed-negatives and the spine the partition is clean -- but that restriction is itself the post-hoc move A1 did not pre-register. The honest reading: the v1 falsifier fired; see §5.

---

## §5 -- Outcome: the v1 falsifier fired; a re-pre-registered v2 is required

**The v1 pre-registered falsifier fired.** Criterion A1 of the locked pre-registration (`PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` §4) reads: "every `[CLOSED NEGATIVE]` entry recording a failed derivation attempt targeted a NON-UNIVERSAL DYNAMICAL quantity." The §3 classification does **not** satisfy A1 as locked: the type-ii closed-negatives (FTD-0050, FTD-0164, FTD-0183) are closed-negative derivation attempts whose targets classify STRUCTURAL. Under criterion A1 as pre-registered, the partition is therefore **not clean** -- the falsifier fired. Per the pre-registration's own rule, v1 is invalidated: this run did not pass its own pre-registered test.

**What that does and does not mean.** It does not show the discriminator (§2) is wrong -- the §2 definition assigned a definite, defensible class to every quantity-claim, and the type-i / type-ii distinction is a genuine finding. But that distinction is a **post-hoc observation**: it was not a category the locked falsifier anticipated. A correct definition that nonetheless fails its own pre-registered falsifier wording yields exactly this -- the definition may stand, but the *test* must be re-registered and re-run.

**The required v2 -- not yet an established result.** A v2 pre-registration is required, tightening A1 to:

> A1 (v2): every closed-negative recording a failed attempt to derive a non-universal **dynamical value** targets a quantity classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL.

This v2 wording **must be a fresh hash-locked pre-registration, with the classification re-run against it.** "Outcome A (clean partition)" **cannot be claimed for v2 until that re-run is done** -- crediting a v2 with the data already in view is precisely the post-hoc criterion-editing that pre-registration exists to prevent. As of this document, the Stage-1 status is: a pre-registered discriminator `[DEFINITION]` that stands, plus a classification whose pre-registered falsifier **fired**, hence `[OPEN]` pending the v2 re-registration and re-run.

**Honest accounting (correction, 2026-05-20).** An earlier draft of this section framed the outcome as "the disciplined falsifier working as designed" and asserted "Outcome A under v2." That was too favorable. A falsifier that fires *is* the falsifier working -- but the honest response is to re-register and re-run, not to declare the sharpened scope already established. This correction follows an adversarial physics-panel review (forward plan, priority P2). Stage 2 (the Structural Decoupling Theorem) must accordingly be pursued as a genuine *provable proposition* -- a stated axiom set and a proof trace -- not inherited as a settled scope.

---

## §6 -- Honest limits

- This classifies the **decisive load-bearing set**, not literally every `LEDGER` row (the LEDGER is ~119k tokens; the exhaustive pass continues and extends `proof_structural_dynamical_partition.py`). The decisive set is, however, exactly what the §4 falsifier turns on.
- A handful of closed-negatives in the 2026-04-24 cluster were classified from LEDGER summary rows; the exhaustive pass will confirm each detail block.
- The type-i / type-ii distinction is itself a `[RESULT]` of this stage, not a pre-registered category; it is offered honestly as the finding that necessitates the v2.

## §7 -- Cross-references

- `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` (locked pre-reg) -> v2 required per §5.
- `FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129) -- the type-i empirical base; Stage 2 rigorizes it.
- `SPEC_DISCRETE_NATIVE_DERIVATION.md` (FTD-0136) -- the methodological reframe Stage 2 backs.
- `THEOREM_A_PHYS_NO_GO.md` -- an existing no-go for the single quantity `a_phys` (CALIBRATION-CONDITIONAL); the boundary theorem generalises this pattern to the whole dynamical-value class.
- `SPEC_ALGEBRAIC_SPINE.md` -- the spine theorems classified STRUCTURAL in §3a.
- `scripts/proofs/proof_structural_dynamical_partition.py` -- machine verification of the §3 partition.
