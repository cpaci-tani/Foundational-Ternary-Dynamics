# FOUND -- The Structural / Dynamical Discriminator (Boundary Theorem, Stage 1)

**Tag:** [BOUNDARY THEOREM -- STAGE 1]. The discriminator (§2) is a pre-registered `[DEFINITION]`; the classification (§3) is `[STAGE 1 CLOSED POSITIVE per v2]` (Outcome A under the v2 falsifier, re-run 2026-05-23; v1 falsifier fired as recorded in §5). No existing LEDGER claim is promoted or demoted. **Stage 2 (the Structural Decoupling Theorem) remains an unsettled provable proposition that must be pursued with its own axioms and proof trace; v2 closing positive is its prerequisite, not its proof.**
**LEDGER row:** FTD-0186 (boundary-theorem program).
**Date:** 2026-05-20 (v1 execution) + 2026-05-23 (v2 close-positive update, Path II Session A2 of `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
**Pre-registration (v1, historical):** `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md`, hash-locked at commit `75ebe56`, tag `preregister-structural-dynamical-discriminator-v1`, SHA256 `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`. The discriminator definition (§2) below was committed **before** the v1 classification was run; the v1 falsifier criterion A1 fired (§5).
**Pre-registration (v2, current — supersedes v1's falsifier wording, not its definition):** `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md`, hash-locked at commit `d550bca`, tag `preregister-structural-dynamical-discriminator-v2`, SHA256 `a233fa28be54c63c6a7ebae26c6b54e129c9f2120e535f92d85999ac84d9068a`. v2 sharpens A1 to "every closed-negative recording a failed attempt to derive a non-universal *dynamical value* targets a quantity classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL," carries §2 over verbatim, and adds A3 (the structural-provenance closed-negatives are recorded as a separate honest category). The §1 of v2 honestly flags that the v2 falsifier is partly engineered to produce Outcome A and that Stage 2 remains a separate, unsettled provable proposition.
**Verifier:** `scripts/proofs/proof_structural_dynamical_partition.py` (encodes v2-style expectations per its header; re-run 2026-05-23 returns Outcome A — clean partition, A1 v2 PASS, A2 PASS, A3 PASS — see §5.2 below).

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

### §5.2 -- v2 re-pre-registration + re-run: Outcome A (2026-05-23, Session A2)

**v2 hash-locked.** `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` committed at `d550bca`, tag `preregister-structural-dynamical-discriminator-v2`, SHA256 `a233fa28be54c63c6a7ebae26c6b54e129c9f2120e535f92d85999ac84d9068a`. The v2 sharpening is documented in §1 of the v2 pre-reg: A1 quantification is restricted to "failed attempt to derive a non-universal *dynamical value*" (rather than v1's broader "failed derivation attempt"); A3 is added to record the type-ii structural-provenance closed-negatives as a separate honest category; §2 (the discriminator definition) is carried over verbatim from v1; §6 adds the FTD-0198 ARC-B1 connection.

**Re-run executed against v2.** `scripts/proofs/proof_structural_dynamical_partition.py` (whose internal classification already encodes v2-style expectations per its header — see §3 below) was re-run 2026-05-23 against the v2 falsifier wording. Result: clean partition.

- **A1 (v2): PASS.** All 13 type-i closed-negatives (FTD-0058 / 0031 / 0093 / 0025 / 0131 / 0116 / 0094 / 0035 / 0034 / 0096 + the three [PARAMETRIC] demotion entries FTD-0018 / 0020 / 0021) classify NON-UNIVERSAL DYNAMICAL (10 entries) or CALIBRATION-CONDITIONAL (3 entries) — without exception across the decisive set.
- **A2: PASS.** All 12 spine [THEOREM] / [DERIVED] claims (FTD-0001 through FTD-0012) classify STRUCTURAL.
- **A3 (v2, new): PASS.** All 3 type-ii closed-negatives (FTD-0050 master-quadratic-as-RG-step-char-poly; FTD-0164 χ_{−4} → P_{G\*} arrow; FTD-0183 N_base=4  ℤ[i]^×) classify STRUCTURAL and are documented as failed attempts to derive a *structural object's deeper provenance* — the underlying objects (the master quadratic = FTD-0001 [THEOREM]; the (2,3) exponents = FTD-0175 [THEOREM]; N_base = 4 = FTD-0008 [THEOREM]) stand, while their *provenance* remains open. These are recorded as **structural-provenance closed-negatives**, outside the boundary theorem's dynamical-value axis.

**Outcome (Stage 1 per v2): CLOSED POSITIVE.** The discriminator (§2) is the locked keystone for the dynamical-value axis. LEDGER FTD-0186 status updated from `[DEFINITION] + [OPEN]` to `[DEFINITION] + [STAGE 1 CLOSED POSITIVE per v2]`. **No FTD claim promoted or demoted.**

**Honest framing carried over from v2 §1.** v2 is not a "win." It is a scope clarification. The v2 falsifier was partly engineered to match the post-v1 honest reading, and §1 of v2 makes this explicit. The discipline-bearing test is whether **Stage 2** produces a *provable proposition* — a stated axiom set, a proof trace, and an honest scope — independently of v2's outcome. Stage 2 remains an unsettled research program, not an inherited result. v2 closing positive establishes that the discriminator partitions the decisive set on the axis it was designed for; it does **not** establish that the boundary theorem is proven.

**What v2 closing positive enables.** (i) Stage 2 may proceed against a locked discriminator (the §2 definition has now passed its own pre-registered falsifier in the corrected wording). (ii) The structural-provenance closed-negatives (FTD-0050 / 0164 / 0183) are recorded as a distinct open research category and no longer falsify A1. (iii) If FTD-0198 ARC-B1 closure attempt closes negative (its prior-favoured outcome per its own pre-reg), the result becomes a load-bearing empirical input to Stage 2 — concrete evidence that the dynamical-value axis is non-closable by the FTD-native non-site-local observable class.

**What v2 closing positive does NOT enable.** (i) No spine tag promotion or demotion. (ii) No claim that the boundary theorem is proven — it is not. (iii) No claim that the type-i / type-ii distinction is a theorem in itself — it is a recorded scoping that survived a sharpened pre-registered falsifier, not a derived result. (iv) No suppression of Outcome B as a real future possibility: if a future closed-negative emerges that targets a structural quantity and is **not** a structural-provenance attempt (Outcome B3 per v2 §4), the §2 definition itself would be falsified and a v3 would be required.

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
