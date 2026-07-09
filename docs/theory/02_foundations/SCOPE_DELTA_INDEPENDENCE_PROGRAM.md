# SCOPE — The δ-independence program: formalize the native closure, then prove (or refute) that the FC-W datum lies outside it

**Tag:** [SCOPE / PROGRAM CHARTER] — registers a conjecture, a formalization ladder, stages, gates, and guards. **Proves nothing itself; promotes nothing.**
**LEDGER id:** FTD-0368. **Effort class:** RP (research program; Stage S4 carries an FO flag).
**Closes:** — (this is the rigorous *negative-side* completion of MC-T4.3; it does not close MC-T4.3, whose positive exits remain a new W-class commitment or a fresh ARC-D measurement — see §1).
**Audience:** project owner + agents working on MC-T4.3, the modulus/argument frontier, the valuation theorem (FTD-0353/0360), or any future formalization of the substrate.

---

## §0 — The target, stated once

> **Independence Conjecture (δ-IND).** Let N be the substrate's **native closure** — a *defined* (not enumerated) class of numbers/functions reachable from the postulates' primitives by the substrate's own operations and its own admissible limits (§3 is this program's fight over that definition). Then
> $$\delta \;=\; \sqrt{G^*(4G^*-1)} \;\notin\; N,$$
> i.e. the datum FC-W imports (FTD-0315; the second "making" of FTD-0340, whose square root no native operation supplies per FTD-0322/0353) is **independent of the native closure** — conditional, wherever G\*-transcendence enters, on Chudnovsky 1976, like the rest of the spine.

δ is the right formal object: the master quadratic's discriminant is 64G\*³(4G\*−1), so x₊ = 8G\*² + 4G\*δ — reaching the α-candidate root *is* reaching δ over ℚ(G\*). A theorem δ ∉ N converts FC-W's status from *adopted convenience* to **proven necessity relative to N**: the strongest honest form of the *mark* half of the Number-One Goal's clause 2 ("rigorously mark and price which types the ontology must import" — δ-IND delivers the marking; the priced-import ledger, FTD-0371, delivers the pricing).

**Outcome symmetry, recorded at registration:** a *construction* of δ inside a defended N would refute δ-IND and would constitute a revolutionary positive exit for MC-T4.3. Both outcomes are results; this program is a test, not an apologetics exercise. [declared prior: δ-IND true, by the weight of FTD-0244/0314/0326/0327/0341/0353]

## §1 — Relation to MC-T4.3, and the shape of the ambition

MC-T4.3 (`SPEC_OPEN_MATH_BY_SECTOR.md`; contract in `SPEC_ALPHA_READOUT_CONTRACT.md`) is the central [FOUNDATIONAL OBSTRUCTION]: every natural action-level/operator route to a physical α-readout is closed negative through FTD-0244. The obstruction is currently a *catalog of closed doors*. This program aims to replace the catalog with a **single theorem about a defined system** — the same upgrade Gödel's second theorem makes over "we tried to prove consistency and failed." The Gödel/Hölder resonance that motivated this charter (a well-defined object whose governing datum is provably unreachable by the system's own finitary means, proved from one level up) is motivation only, tagged [coherent-interpretation]; the program's statements stand without it. **Equivocation guard (binding):** independence over a field/ring/closure is *not* logical independence; no result below Stage S4 may be announced in proof-theoretic language.

## §2 — What already exists (the partial-results map)

| existing result | what it gives the program | gap this program closes |
|---|---|---|
| **FTD-0353/0360** valuation theorem (`THEOREM_VALUATION_4GSTAR_MINUS_1.md`): modelling ℚ(G\*, π) ≅ ℚ(t, u), every *documented* native analytic output is a unit at the prime (4t−1); δ² has odd valuation ⇒ δ outside the native period hull and every radical tower over native monomials | the **current best δ-IND**: the valuation instrument works | its scope is "relative to the documented native inventory" — completeness of the inventory is [SELECTION]. Stage S2/S3 replaces the *list* with a *definition* |
| FTD-0244 (K-BIND theorem-negative) + FTD-0314 (C1–C3) + FTD-0326/0327 (operator/Galois + AGM place-bridge) + FTD-0341 (carrier closures) | instance checks: every named carrier fails to reach δ | instances ≠ closure; the program subsumes them under N |
| FTD-0357/0358 (four walls, one-door corridor) | the wall's topology: one exit schema | makes precise *what* N must contain for the theorem to bite |
| FTD-0322 / FTD-0340 (act count; √-as-act): i native (FC-0), δ imported (FC-W) — the two makings | the object-level identification of δ as *the* imported act | the program proves the second making cannot be avoided *relative to N* |
| **FTD-0367** (reflection flow parity): differential-algebraicity as a frontier invariant; DA-closure arguments machine-verified | a new instrument: law-wildness detection; the DA-closure toolkit | Stage S1 applies it to the substrate itself |
| `AUDIT_INFINITY_REFRAME.md` (undefined-boundary; ε-L discipline) | the limit-policy vocabulary Stage S2 needs | — |

## §3 — The crux: defining N (the adequacy/properness tension)

Everything hard in this program is here. N must be **adequate** (it provably contains the documented native analytic outputs — Watson/BCC Green's-function limits, Phase-G Coulomb tails, the G\*-class values the corpus's own inventory reaches) and **proper** (it provably excludes something; a closure containing all computable reals makes δ-IND false or vacuous). The two failure modes are falsifiers of a *candidate definition*, not of the conjecture:

- **Too weak** (fails adequacy): N excludes G\* itself → the theorem is vacuously true and says nothing about FTD. Falsifier: exhibit a documented native output outside N.
- **Too strong** (fails properness): N admits arbitrary limits → δ ∈ N trivially. Falsifier: exhibit δ (or any period-grade transcendental) inside N by a construction the definition admits.

**The formalization ladder** (each rung a candidate N; the program may prove δ-IND at one rung while higher rungs stay open):

- **N0 — finite-horizon semi-algebraic closure.** For fixed L and finite tick horizon, the substrate's update is a semi-algebraic map over the declared theory-parameter field k₀ (thresholds are inequalities; the Gauss projection is a linear solve, hence rational in its inputs; all six core rules are polynomial/piecewise-polynomial). Consequence to be proven as **Lemma 0 (Stage S1)**: *every finite-horizon native constant is algebraic over k₀.* Since δ is transcendental (conditional Chudnovsky), δ ∉ N0 — **but so is G\* ∉ N0**: adequacy fails, by design. Lemma 0's real content is the reframe it forces: **the entire independence question lives in the admissible-limit policy, not in the finite dynamics.** The α-wall is a property of *which limits the ontology owns* — the ε-L/undefined-boundary discipline becoming load-bearing rather than hygienic. [candidate [DERIVED] at S1; currently a proof obligation with sketch]
- **N1 — N0 + effective ε-L limits.** Adjoin limits of native sequences that converge with a *definable modulus* (the substrate's own arbitrarily-large-finite computations, packaged per `AUDIT_INFINITY_REFRAME`). Adequacy plausibly holds (Watson-type limits have effective moduli). Properness is the open fight: the definition must not smuggle in a universal limit operator. This is the rung where the program expects to live.
- **N2 — the period-ring formalization.** Native closure = the k₀-period ring generated by the substrate's integral representations (Watson-class integrals are Kontsevich–Zagier periods after π-normalization: π³·W₃ = ∫∫∫ dx dy dz/(1 − cos x cos y cos z)). δ-IND at N2 has the right shape and the hardest pedigree (period-independence questions border the Grothendieck period conjecture — [FO]-risk); the FTD-0353 valuation model ℚ(t, u) is exactly its tractable Chudnovsky-conditional shadow.
- **N3 — proof-theoretic formalization (aspirational, FO).** P1–P5 + the FC register as an actual formal theory; δ-IND as logical independence. Gödel-grade in difficulty and in payoff; out of scope until S3 delivers, and possibly a different project.

## §4 — Stages and gates

- **S0 (this document).** Charter + registration. Gate: none (registers a conjecture; proves nothing).
- **S1 — Lemma 0. DELIVERED 2026-07-05:** `FOUND_FINITE_HORIZON_ALGEBRAICITY.md` + `scripts/proofs/proof_lemma0_finite_horizon.py` (9/9, exact arithmetic). All nine default-substrate rules (core six + three default-ON toggles — the enumeration extended past the charter's "six" to match the spec's actual default set) are piecewise-polynomial over k₀ with couplings as indeterminates; finite-horizon observables are algebraic; the wall factorizes into the admissible-limit policy + the parameter-assignment policy. Completeness flag stated in the note's §3 per this gate. The original gate text is preserved below for provenance:
  *Enumerate the six core rules from `engine/SPEC_ENGINE.md`/the constitution at spec level (not the C++ — see §5 guard 4), verify each is semi-algebraic over k₀, conclude finite-horizon algebraicity, and state the limit-policy reframe. Deliverable: a FOUND/MATH note + proof script. Gate: the enumeration must be complete against the spec's rule list, or the lemma inherits an inventory-[SELECTION] flag of its own — stated, not hidden.*
- **S2 — the definition fight.** Propose N (target rung N1), prove adequacy (documented inventory ⊆ N) and defend properness against the §3 falsifiers. Deliverable: a definition + adequacy theorem. Gate: **pre-register the definition before testing δ against it** (the definition must not be tuned post-hoc to exclude δ — that would be the mirror image of the near-miss sin).
- **S3 — the theorem. EXECUTED 2026-07-05, verdict FTD-0369: PROVEN-CONDITIONAL** (`ANALYSIS_DELTA_IND_CLOSURE_v1.md` + `proof_s3_delta_independence.py`, 11/11): δ ∉ N under E0 (Chudnovsky) + E1/E2 (enumerated open independence assumptions — the price of the class escaping ℚ(G\*, π) through the SC-sector, exactly as prior P2 anticipated); BCC-sector sub-theorem conditional on Chudnovsky alone, retiring the dynamical inventory-[SELECTION] for that sector. The original stage text is preserved below for provenance:
  *Re-run the FTD-0353 valuation argument relative to the defined N (not the documented list): δ ∉ N, [THEOREM — conditional on Chudnovsky 1976]. Deliverable: theorem doc + verifier; the valuation theorem's inventory-[SELECTION] flag retires. LEDGER row minted at verdict time.*
- **S4 — aspirational.** N3 logical independence. Not scheduled; recorded so the ambition has an address.

## §5 — Guards (binding on every stage)

1. **No equivocation** between closure-independence and logical independence (§1); the Gödel analogy stays [coherent-interpretation] at every stage including success.
2. **No definition-tuning:** N is frozen (S2 pre-registration) before δ is tested against it. The program must be falsifiable *as a program*.
3. **Outcome symmetry:** δ ∈ N, if constructed, is registered as a positive MC-T4.3 exit with the same ceremony as the independence theorem — declared here so success-bias has no room.
4. **Engine/theory separation:** the engine binary hardcodes α as an input (`constants.h` HONEST FRAMING); *engine reachability is evidence for nothing in this program, in either direction.* N is defined from the spec-level rules, not the implementation.
5. **No mid-program promotions:** MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION], x₊ = 1/α stays [SMC] (FTD-0013), FC-W stays [AXIOM], through every stage until S3's verdict — and S3 moves only the valuation theorem's conditionality, not the physics identification.
6. **The DA temptation (inherited from FTD-0367 §4):** Lemma 0 + FTD-0367 will make "the substrate is a DA-world, δ's flow-law world is hypertranscendental, *therefore* the wall" feel available. It is not: no implication between law-wildness and value-unreachability has been established. Lemma 0 classifies *where* transcendental content can sit (limits, not finite dynamics); it does not explain *why* one limit-borne value (G\*) is native-reachable and another (δ) is conjectured not to be. That "why" is the program's actual question.

## §6 — Goal alignment

Under the Number-One Goal this program is clause 2 pursued to its strongest form: not "we have not derived δ" (a report), not "every tried route fails" (a catalog, MC-T4.3's current state), but "**no route inside the defined closure exists**" (a theorem) — the boundary as a *deliverable* in the exact sense the goal declares, with FC-W's import status the proven price of the ontology's honesty. The operational test the goal prescribes is met by construction: S1–S3 build content forward ([DERIVED]/[THEOREM] targets), and the whole program marks a boundary.

## §7 — Cross-references

- `docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` (MC-T4.3 row) + `docs/theory/02_foundations/SPEC_ALPHA_READOUT_CONTRACT.md` — the obstruction and its closure contract.
- `docs/theory/09_mathematical/number_theory/THEOREM_VALUATION_4GSTAR_MINUS_1.md` (FTD-0353/0360) — the instrument S3 upgrades.
- `FOUND_SQUARE_ROOT_AS_ACT.md` (FTD-0340), `FOUND_ACT_REDUCTION_COUNT.md` (FTD-0322), `FOUND_AGM_PLACE_BRIDGE_AND_DELTA.md` (FTD-0327) — the two-makings structure and the carrier closures.
- `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (FTD-0336) + `FOUND_TYPE_PRIORITY_PRINCIPLE.md` (FTD-0339) — the frontier this theorem would harden; type-priority is the program's philosophical spine.
- `docs/theory/09_mathematical/number_theory/MATH_PERIOD_IMPORT_FRONTIER.md` (FTD-0375) — the period-conjecture *framing* of this program's boundary: δ located relative to the CM motive `h¹(E_lemn)` whose GPC is a Chudnovsky theorem (trdeg=2); it makes explicit that the general δ∉N is Chudnovsky **+ open E1/E2** (not Chudnovsky alone), and that the frame re-describes rather than corroborates δ∉N (shared Chudnovsky input — no double-counting).
- `docs/theory/09_mathematical/general_math/MATH_REFLECTION_FLOW_PARITY.md` (FTD-0367) — the DA/hypertranscendence instrument; its §4 guard is inherited here as guard 6.
- `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` — the ε-L limit-policy vocabulary N1 is built from.
- Classical: O. Hölder (1887), Math. Ann. 28, 1–13 (see `REF_BIBLIOGRAPHY.md` §1); D.V. & G.V. Chudnovsky (1976) — the standing conditionality of the spine.

---

*This charter registers a conjecture and a discipline for attacking it. Nothing above is a result; the first candidate result is Stage S1's Lemma 0.*
