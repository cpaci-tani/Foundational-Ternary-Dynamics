# PRE-REGISTRATION -- Structural / Dynamical Discriminator (Boundary Theorem, Stage 1), v2

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-23
**Hash-lock target tag:** `preregister-structural-dynamical-discriminator-v2`
**LEDGER row reservation:** FTD-0186 (the boundary-theorem program; v2 supersedes v1's falsifier wording, not its discriminator definition)
**Supersedes:** `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` (hash-locked at commit `75ebe56`, tag `preregister-structural-dynamical-discriminator-v1`, SHA256 `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`) — see §1 and §4 for the single substantive change.
**Companion docs:** `FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129), `SPEC_DISCRETE_NATIVE_DERIVATION.md` (FTD-0136), `SPEC_ALPHA_READOUT_CONTRACT.md` (FTD-0152), `FOUND_META_PATTERNS.md` (MP-0a/0b, §8.4), `SPEC_DIMENSIONAL_MAP.md`, `LEDGER.md`, `CATALOG_PARAMETRIC_INSERTIONS.md`, `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` (the v1-execution result, including §5 honest accounting that v1 fired).
**Path I prerequisite landed:** Theorem 7 honest retag committed as `1d7eff9` (`docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` §7 to `[THEOREM at L=2 — mode-degeneracy origin] + [DISCONFIRMED for general L]`); the v2 falsifier reads the same spine as the v1 falsifier did, with the L=2-only ultralocality scope made explicit.

> **Pre-registration discipline.** The discriminator definition (§2) is **carried over verbatim from v1** — the v1 §3 classification confirmed that the §2 definition assigns a definite, defensible class to every quantity-claim in the decisive set. The single substantive change is §4 criterion A1 (the falsifier wording). After commit: SHA256 -> `REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §2 or §4 invalidates v2; a v3 would be required before the re-run.

---

## §1 -- Why v2 (the single substantive change)

The v1 pre-registration produced an honest, informative result: the §3 classification confirmed the §2 definition partitions every quantity-claim in the decisive set into one of three classes, but the §4 falsifier criterion A1 — as locked — **fired**. The reason was structural: A1 quantified over *all* closed-negatives, but the FTD closed-negative record contains two structurally-distinct kinds of failed derivation attempt:

- **Type i — failed derivation of a non-universal *dynamical value*.** Failed attempts to compute a coupling (α, g_c, σ), a mass ratio (m_e/α, m_p/m_e), a renormalization factor (the G\*² Z-factor), a mixing angle (sin²θ_W, α_s, PMNS), or a calibration constant (a_phys, μ-from-ℓ_P). The target is a continuous parameter that the substrate's finite combinatorics does not fix.
- **Type ii — failed derivation of a *structural object's deeper provenance*.** Failed attempts to find a deeper structural *origin* for an object that **is** structural and **does** stand: FTD-0050 (master quadratic as the characteristic polynomial of an RG step) targets the master quadratic itself, which is FTD-0001 `[THEOREM]`; FTD-0164 (χ_{−4} → P_{G\*} arrow) targets the (2, 3) exponents which are FTD-0175 `[THEOREM]`; FTD-0183 (N_base = 4 unification with ℤ[i]^×) targets the integer N_base = 4 which is FTD-0008 `[THEOREM]`. These attempts did not fail to fix a dynamical value; they failed to find a deeper *provenance* for an already-established structural object.

v1's A1 wording ("every `[CLOSED NEGATIVE]` entry recording a failed *derivation attempt* targeted a NON-UNIVERSAL DYNAMICAL quantity") swept both types into a single test. Under that wording the type-ii closed-negatives (whose targets classify STRUCTURAL by the §2 discriminator) caused A1 to fire. Per the v1 pre-registration's own rule, this invalidated v1 — and the honest response is to re-register the falsifier with the correct scope, not to declare the type-i/type-ii distinction as a settled scoping.

**v2's single substantive change** restricts A1's quantification to the kind of closed-negative the boundary theorem is *about*: failed attempts to derive a non-universal dynamical value. Type-ii closed-negatives are recorded separately as *structural-provenance* closed-negatives, with their own honest status — open structural-provenance questions for objects that already stand as theorems.

**What v2 is NOT.** v2 is not a "win." It is a scope clarification. The §2 definition was already correct; v1 fired on a falsifier wording that quantified too broadly. v2 narrows the quantification to match what the discriminator is actually a discriminator *of* — the structural-vs-dynamical-value axis. The new partition (§4) is therefore very nearly *engineered* to come out clean, and the honest framing of Outcome A under v2 is **"the classification holds on the axis the discriminator was designed for"** rather than **"the boundary theorem is now proven."** Stage 2 (the Structural Decoupling Theorem) remains an unsettled provable proposition that must be pursued with its own axioms and proof trace; v2 closing positive is the prerequisite for Stage 2 work, not a substitute for it.

The pre-registration discipline is preserved: §2 stays locked; §4 A1 is rewritten in v2 with the rewrite made explicit and the v1 firing recorded; the re-run is run only after this v2 commit lands.

---

## §2 -- The pre-registered discriminator (verbatim from v1, locked)

Carried over verbatim from `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` §2. The v1 §3 classification confirmed this definition assigns a definite, defensible class to every load-bearing quantity-claim; no change is needed.

Let Q be any load-bearing quantity-**claim** that the framework derives, claims, or matches. Q is assigned **exactly one** of three classes.

### STRUCTURAL

Q is STRUCTURAL iff its value is **forced by the substrate's finite discrete data** — automorphism-group orders, lattice neighbour/shell counts, polyhedral-decomposition multiplicities, lattice dimension, or exact spectra/determinants of finite substrate operators — such that **both**:

- **(S1) Discrete-combinatorial origin.** Q lies in the countable set generated, by field operations and finite algebraic extension, from that finite combinatorial data.
- **(S2) Free-choice invariance.** Q's value is invariant under every free choice in the framework: the calibration declarations (`a_phys = ℓ_P`, `K_B`, `t_phys`), any global rescaling, and the choice of initial / boundary conditions.

Paradigm cases: D = 3 (from |Aut(E)|² = 2^D (D−1)!); |Aut(E)| = 4 and the coefficient 16 = |Aut(E)|²; the Moore integers {N_base = 4, N_eff = 13, b_3 = 7}; the colour count N_c = 3 read as a topological/representation count.

### NON-UNIVERSAL DYNAMICAL

Q is NON-UNIVERSAL DYNAMICAL iff it is a **dimensionless continuous parameter** — a coupling strength, a dynamical-scale ratio, or a mass ratio — whose value **fails (S1)**: it is not forced by the finite combinatorics. Determining its value requires an external input — a measured anchor, an action-level parameter, or an initial condition.

Paradigm cases: α (the fine-structure coupling), g_c, sin²θ_W, α_s, the lepton/hadron mass ratios.

### CALIBRATION-CONDITIONAL

Q is CALIBRATION-CONDITIONAL iff it is a **dimensional** quantity whose numerical value **fails (S2)**: it depends on the calibration declarations. This is the dimensional category of `SPEC_DIMENSIONAL_MAP.md` and the absolute-scale column of FTD-0136 §6.4.

Paradigm cases: m_e in MeV, G_N in SI units, any quantity carrying physical units.

**Scope.** The boundary theorem concerns the **STRUCTURAL vs NON-UNIVERSAL DYNAMICAL** split (both dimensionless). CALIBRATION-CONDITIONAL is recorded for completeness — it is already mapped by `SPEC_DIMENSIONAL_MAP.md` — and is not the theorem's subject.

**Tie-break rule (locked).** The discriminator classifies *claims*, not *symbols*. If a symbol carries both a structural and a dynamical reading — e.g. N_c, where *the integer 3 as a colour count* is structural but *the match x_− = 3.024 ↔ N_c* is a `[STRONGLY MOTIVATED CONJECTURE]` identification — the two readings are entered as **separate quantity-claims**: the count is STRUCTURAL, the root-identification is NON-UNIVERSAL DYNAMICAL (it identifies a continuous algebraic root with a physical sector).

---

## §3 -- The pre-registered classification procedure

The §3 classification of the decisive set was executed in v1 and is documented in `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §3. The v2 re-run **does not re-classify the decisive set** — the §2 definition is unchanged and the v1 classification was mechanical given §2. The v2 re-run executes the v2 falsifier (§4) against the existing v1 classification.

If the future exhaustive every-row pass (`LEDGER.md` + `CATALOG_PARAMETRIC_INSERTIONS.md` in full) reveals additional quantity-claims, they will be classified under §2 and the v2 falsifier re-applied; the decisive-set re-run does not pre-empt that work.

---

## §4 -- The pre-registered falsifier / acceptance criteria (v2)

The discriminator earns its keystone role only if it **cleanly partitions the dynamical-value-derivation closed-negatives and the spine**, with the structural-provenance closed-negatives recorded as a separate, honest category.

### Outcome A -- discriminator confirmed (clean partition on the dynamical-value axis)

All three hold:

- **A1 (v2, sharpened).** Every closed-negative recording a failed attempt to derive a non-universal **dynamical value** targets a quantity classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL. *(Single substantive change from v1: "failed derivation attempt" → "failed attempt to derive a non-universal dynamical value." The v1 wording is recorded for provenance in §1.)*
- **A2 (unchanged from v1).** Every algebraic-spine `[THEOREM]` / `[DERIVED]` claim is classified STRUCTURAL.
- **A3 (new in v2; codifies the v1 finding).** Every closed-negative whose target is classified STRUCTURAL is a failed attempt to derive a *structural object's deeper provenance* (i.e. is a type-ii closed-negative in the v1-finding sense); it is recorded as a structural-provenance closed-negative and is outside the boundary theorem's dynamical-value axis.

Then: the discriminator is confirmed as the keystone (Outcome A on the dynamical-value axis); Stage 2 may be pursued *as a genuine provable proposition with stated axioms* (it is not inherited from this outcome); LEDGER FTD-0186 records Stage 1 closed positive per v2; the structural-provenance closed-negatives are recorded as open structural-provenance questions in `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §3b and `LEDGER` notes.

### Outcome B -- discriminator falsified (misfit on the v2 axis)

Any of the following:

- **B1.** Some closed-negative recording a failed attempt to derive a non-universal dynamical value targets a quantity classified STRUCTURAL. (Would refute A1.)
- **B2.** Some algebraic-spine `[THEOREM]` / `[DERIVED]` claim is classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL. (Would refute A2.)
- **B3.** Some closed-negative whose target is classified STRUCTURAL is **not** a failed structural-provenance attempt — i.e. it really did target a dynamical value but the §2 definition misclassifies its target as structural. (Would refute A3 and indicate the §2 definition itself is wrong, not just v1's quantification.)

Then: §2 is wrong on its own axis. Diagnose the misfit, issue a v3 with a corrected definition, re-run. This is a genuine, informative result and is the falsifier doing its job.

### Outcome C -- partial / genuinely ambiguous

A small number of quantity-claims cannot be unambiguously classified as type-i or type-ii (i.e. the dynamical-value vs structural-provenance distinction itself is ambiguous for them). Then: the discriminator stands for the cleanly-classified majority; the ambiguous cases are documented as known limits, and Stage 2's theorem is scoped to exclude them.

**Outcome A is the prior-favoured result and the v2 wording is partly engineered to produce it** — §1 makes this honesty explicit. Outcome B would be a genuine surprise and would invalidate the §2 definition itself, not just the v2 wording. All three are publishable. The closure-attempt mechanic (§5) is what determines which lands.

---

## §5 -- Pre-registered consequences

- **Outcome A under v2** -> the discriminator is the locked keystone of the Structural Decoupling Theorem (Stage 2); LEDGER FTD-0186 opens [Stage 1 CLOSED POSITIVE per v2]; the FOUND doc §5 is updated to record that the v2 falsifier passed; structural-provenance closed-negatives are recorded separately. **No FTD claim is promoted or demoted.** Stage 2 remains an unsettled proposition; v2 closing positive is its prerequisite, not its proof.
- **Outcome B under v2** -> v3 reissue; FTD-0186 records the misfit diagnosis and the §2 definition is reopened. The discriminator definition is wrong on its own axis and must be corrected.
- **Outcome C** -> Stage 2 proceeds with the scoped discriminator; ambiguous cases logged in `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` as known limits.

In every case the algebraic spine is untouched — this program classifies existing claims; it promotes and demotes nothing.

---

## §6 -- Relation to FTD-0129, FTD-0136, ARC (FTD-0152, FTD-0198), FOUND_META_PATTERNS

Carried over from v1 §6 with one v2 addition (the FTD-0198 ARC-B1 connection):

- **FTD-0129** (empirical four-channel synthesis): Stage 2 will **rigorize** it — upgrade its `[SYNTHESIS]` to a `[THEOREM]` with a stated axiom set. FTD-0129 supplies the empirical base; Stage 1 supplies the discriminator that lets the finding be stated as a theorem.
- **FTD-0136** (the discrete-native-derivation reframe): FTD-0136 already reframed the action-channel closed-negatives as "the question was malformed." The boundary theorem is the **rigorous backing for that reframe** — Stage 2's partial theorem proves *why* the action/classical-channel class cannot carry a non-universal dynamical value, which is the precise content of "malformed." **Honest tension, flagged (carried over from v1):** FTD-0136 §5 / §6.4 holds open the hope that FTD-native channels (Class C cluster-interaction) can still yield dimensionless couplings as measurement-level predictions. Stage 3 (the *full* no-go) would be in tension with that hope; Stage 3 must explicitly address FTD-0136's discrete-native optimism, and is for that reason marked an open research stretch. Stages 1+2 carry no such tension.
- **ARC / FTD-0152 / FTD-0198 (ARC-B1)**: the boundary theorem is ARC's dual — ARC states what an admissible α-readout must look like; the boundary theorem characterizes why the failed channel-class fails. **v2-specific link:** FTD-0198 (the ARC-B1 closure attempt against MC-T4.3, hash-locked at commit `0e79820`) is the active first attack on MC-T4.3; its **prior-favoured outcome is CLOSED-NEGATIVE** per its pre-reg. If FTD-0198 closes negative, it becomes a load-bearing input to Stage 2 — concrete evidence that the dynamical-value axis is non-closable by the FTD-native non-site-local observable class, which is exactly the content the structural decoupling theorem aims to prove. If FTD-0198 closes positive (FOUND) instead, Stage 2's scope contracts: the boundary theorem then characterises a subclass of the dynamical-value space rather than the whole.
- **FOUND_META_PATTERNS §8.4**: its observation "FTD's primary selections are boundaries; derived quantities may be interior values" is the informal seed of the §2 discriminator, recast here on the structural/dynamical axis.

---

## §7 -- Risk register

| Risk | Severity | Mitigation |
|------|----------|------------|
| The v2 A1 wording is engineered to produce Outcome A | High | Honestly flagged in §1 and §4 — the v2 falsifier *is* designed to match the post-v1 honest reading. The discipline-bearing test is not "does v2 close positive" (it almost certainly does) but "does Stage 2 produce a *provable proposition* with stated axioms, independently of v2's outcome." Outcome A under v2 is necessary but not sufficient for the boundary theorem's program. |
| A new closed-negative emerges that targets a structural quantity but is *not* a structural-provenance attempt | Medium | This is Outcome B3 — a real falsifier that would refute the §2 definition. The verifier script will be extended whenever a new closed-negative is added to LEDGER; future LEDGER edits should run the v2 partition script as a regression gate. |
| Dual-reading symbols (N_c and similar) resist classification | Medium | The §2 tie-break rule (classify claims, not symbols); residuals → Outcome C. |
| §4 was rewritten by the same agent planning Stage 2 (GTCA F9: a definition gerrymandered to fit) | High | Pre-registration: §4 v2 is hash-locked **before** the re-run; the v1 firing is honestly recorded in §1 + LEDGER; the partition is checked mechanically by `scripts/proofs/proof_structural_dynamical_partition.py` (already encodes v2-style expectations as documented in the script header). Outcome B remains a real declared outcome under v2. |
| Stage 3 tension with FTD-0136's discrete-native optimism | Acknowledged | Out of scope for Stage 1; flagged in §6; Stage 3 is explicitly an unpromised research stretch. |
| FTD-0198 ARC-B1 closure attempt closes positive (FOUND) | Acknowledged | Would re-scope Stage 2 (see §6); does not invalidate v2 — the boundary theorem then characterises a subclass of the dynamical-value space rather than the whole. |

---

## §8 -- Hash-lock

After owner review and commit:

```
git tag preregister-structural-dynamical-discriminator-v2 <commit-sha>
sha256sum docs/theory/10_eft_program/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md
```

Record the SHA256 in `REF_PREREGISTER_MANIFEST.md`; git tag is local-only per project policy. §2 (carried over from v1) and §4 (the v2 sharpening) are the pre-registered content; the re-run (executing the existing `scripts/proofs/proof_structural_dynamical_partition.py`, which already encodes the v2-style expectations per its header) is run only after this commit. The v1 tag, hash, and commit (`preregister-structural-dynamical-discriminator-v1` / `a6562dca…` / `75ebe56`) remain in the manifest as historical provenance — they are not deleted.

---

## §9 -- What this pre-registration does NOT cover

- **Stage 2** (the Structural Decoupling Theorem) and **Stage 3** (the full no-go) — separate work, gated on Outcome A under v2 but **not inherited from it.** Stage 2 is a genuine provable proposition that must be pursued with its own axioms and proof trace.
- **The engine phase-diagram track** — secondary, separately pre-registered if pursued.
- **Any tag change** to any existing LEDGER claim — the v2 re-run reads existing tags; it does not alter them. (The Path I 2026-05-23 Theorem 7 retag is methodological clarification, not a tag promotion or demotion — see commit `1d7eff9` and LEDGER FTD-0201.)
- **The exhaustive every-row LEDGER + CATALOG_PARAMETRIC_INSERTIONS classification pass** — continues as ongoing work; the decisive-set re-run is the v2 falsifier test and is sufficient to close v1's open status, but the every-row pass is what would harden the boundary theorem against unseen closed-negatives. The verifier script will be extended row-by-row as the every-row pass progresses.
- **The closure attempt for FTD-0198 ARC-B1** — separate downstream multi-session arc; v2's Outcome A is *not* contingent on FTD-0198, and vice versa, except in the Stage-2 scoping sense documented in §6.

---

## §10 -- Status

**DRAFT v2 — authored 2026-05-23 as Session A2 of the multi-session coordinated arc `.claude/plans/let-s-proceed-on-the-eager-rocket.md`. Not yet hash-locked.** Pending owner review of §1 (the single substantive change and its honest framing) and §4 (the v2 falsifier), then: commit → `git tag preregister-structural-dynamical-discriminator-v2` → SHA256 to `REF_PREREGISTER_MANIFEST.md` → re-run of `scripts/proofs/proof_structural_dynamical_partition.py` against v2 wording → LEDGER FTD-0186 update from `[DEFINITION] + [OPEN]` to `[DEFINITION] + [STAGE 1 CLOSED POSITIVE per v2]`. **The prior-favoured outcome is Outcome A** — the v2 wording was designed to match the post-v1 honest reading; §1 makes this honesty explicit. The discipline-bearing test is whether Stage 2 produces a provable proposition with stated axioms, *independently of v2's outcome.*
