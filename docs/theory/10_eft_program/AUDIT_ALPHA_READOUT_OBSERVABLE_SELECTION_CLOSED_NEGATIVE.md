# AUDIT -- ARC-B1 alpha-readout observable-selection closure attempt: **CLOSED-NEGATIVE** for the plaquette-bivector route

**Tag:** [CLOSED NEGATIVE] -- per §6 outcome (c) of the locked pre-registration; verdict result of executing the §9 11-step method against the plaquette-bivector candidate A_obs. **No FTD claim promoted or demoted.** FTD-0013 (`x_+ = 1/α`) status remains `[STRONGLY MOTIVATED CONJECTURE]`. The spine (FTD-0001 / 0006 / 0013) is untouched.
**LEDGER row:** FTD-0204.
**Date:** 2026-05-23 (Path V Session C1 of `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
**Closure-attempt target:** the plaquette-bivector candidate A_obs identified in `.claude/plans/let-s-proceed-on-the-eager-rocket.md` §C1 as the strongest FTD-native non-site-local observable starting point per the FTD-0086 STRONG POSITIVE result.
**Pre-registration governing this attempt:** [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) (FTD-0198), hash-locked at commit `0e79820`, tag `preregister-alpha-readout-observable-selection-v1`, SHA256 `e273ca85234c04406c14b0b0bb01bb2ea760367ca7286c2b35649b80563b582a`.
**Verdict scope:** this audit closes the **plaquette-bivector route** (catalog item 4) only. It does NOT close ARC-B1 as a whole; catalog items 6 (boundary-to-boundary transfer) and 7 (reflexive projections) remain unattempted and require their own closure-attempt runs against the same pre-reg.
**Sources read for grounding:**
- `docs/theory/09_mathematical/DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md` (FTD-0086 / 0087 / 0088, the plaquette-bivector empirical campaign).
- `docs/theory/02_foundations/FOUND_FORCE_STRUCTURE.md` (flux field J = J_L + J_R; dual-substrate; φ = J_L − J_R).
- `docs/theory/02_foundations/FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129, 4-leg empirical diagnostic the closure must survive).
- `docs/theory/01_reference/SPEC_FQCR.md` §§2-3 (FQCR Model V `T_O` and the master quadratic as derivation TARGET, not input).
- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` §§2, 5 (master quadratic FTD-0001; coefficient 16 = |Aut(E)|² FTD-0006 / 0007 as derivation TARGETS).
- `docs/theory/09_mathematical/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md` (FTD-0073 site-local mode-erasure no-go).

---

## §0 -- Executive summary

The plaquette-bivector A_obs candidate **passes steps 1-3** of the §9 method (ARC tuple stated; A_obs is non-site-local from its first definitional step; gauge / translation / O_h invariance verified). The construction **fails at step 5** -- the characteristic equation of every forward-derivable T_O on the bivector algebra is **not** the master quadratic; the bivector algebra's natural operator structure (commutators, adjoint action, Casimir) produces 𝔰𝔲(2)-type characteristic polynomials of the form `z(z² − c) = 0` with `c` a structure-constant scalar -- **not** `x² − 16(G\*)² x + 16(G\*)³ = 0`. The coefficients 16 and G\* are number-theoretic invariants of the lemniscatic curve `E : y² = x³ − x` and the ℤ[i]-module structure (FTD-0006 / 0122); they do **not** arise from the cubic-lattice bivector commutator structure.

The proximate falsifier risk on any "fix" that would force the master quadratic structure is **F-j**: importing the FQCR Model V transfer matrix `M_N(t)` (catalog item 8, target-not-input) as scaffold for `T_O` and observing that its characteristic polynomial is the master quadratic is exactly the reverse-engineering pattern F-j prohibits.

**Verdict per §6 (c):** the plaquette-bivector route yields a different distinguished algebraic number (𝔰𝔲(2) Casimir, not the master quadratic roots). CLOSED-NEGATIVE for this route. **The verdict applies only to the plaquette-bivector route** -- ARC-B1 as a whole remains open pending closure attempts against catalog items 6 (boundary-to-boundary transfer observables) and 7 (reflexive projections).

**Load-bearing input for Path II (FTD-0186 v2 Stage 2):** the CLOSED-NEGATIVE here narrows the surviving search space within ARC-B1 by one catalog item and contributes empirical evidence (one more failed mechanism class) toward the boundary theorem's structural-decoupling thesis. Per FTD-0186 v2 §6 (cross-ref to FTD-0198), if all ARC-B1 candidates close negative, the result becomes the load-bearing empirical input for Stage 2 of the boundary theorem.

---

## §1 -- Method execution: §9 step-by-step

This audit executes the §9 11-step method against the plaquette-bivector candidate. Each step is reported with explicit reference to construction content; the per-falsifier (§7) and per-banned-move (§8) checklists are mechanical (§9 step 8 + step 9).

### Step 1 -- State the proposed ARC tuple `(P, A_obs, O_EM, R, C)`

- **P (preparation):** following the FTD-0086 protocol verbatim -- a finite L³ block (L=8 in the canonical FTD-0086 measurement; the construction extends to all finite L) with deterministic Walsh-Hadamard weight-1 mode injection on axis `f ∈ {x, y, z}` followed by axis `g ∈ {x, y, z}`, one tick of full non-local dynamics per injection. Construction primitives: catalog item 1 (state field `s` finite-difference structure inherited via genesis), catalog item 2 (flux field J with dual-substrate decomposition J = J_L + J_R), catalog item 3 (bilinear link observables that the plaquette construction is built from).
- **A_obs (admissible observable algebra):** the algebra `𝔄_P` generated by the three plaquette bivectors `{P_{xy}(x), P_{xz}(x), P_{yz}(x) | x ∈ L³}` under commutator and the bilinear primitives of catalog item 3, where
  ```
  P_{ij}(x) := J_i(x) J_j(x + ê_i)  −  J_i(x + ê_j) J_j(x),    i ≠ j.
  ```
  (Catalog item 4, the canonical lattice 2-form, verbatim from `DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md` §1.1.2 Definition.) **Non-site-local by construction**: every plaquette bivector references four distinct lattice sites in a 2×2 face. F-e cannot fire on `A_obs` (mechanical check, deferred to step 8).
- **O_EM (electromagnetic measurement functional):** the proposed reading is `O_EM(P) = ⟨[Ê_f, Ê_g], P_a⟩` -- the commutator of WH-injection operators evaluated on the matching plaquette bivector `P_a` for the unordered pair `{f, g}` (per FTD-0086 STRONG POSITIVE matching signature). Operationally, this is the magnitude of the bivector-commutator response in the matching plaquette, normalized by the injection amplitude. Charge-like interpretation: the bivector commutator's matching-concentration ratio is the candidate readout (the value reported as "27×, 79×, 15×" in FTD-0086 Table 1.3).
- **R (readout map):** a dimensionless functional `R : 𝔄_P → ℝ_{>0}` mapping the bivector-commutator-response to a candidate dimensionless inverse coupling. The specific form `R(O_EM) = ⟨P_a, [Ê_f, Ê_g]·P_a⟩ / ⟨P_a, P_a⟩` produces a Rayleigh-quotient-style scalar. The pre-reg requirement is that the result IS `1/x_+`; whether this form actually yields `1/x_+` is the step-5 question.
- **C (calibration discipline):** the plaquette bivectors are constructed from the dimensionless flux field `J` (per `FOUND_FORCE_STRUCTURE.md` -- J is the substrate vector field with no a-priori dimensional unit). Bivector products `J_i J_j` are dimensionless products of dimensionless components. The commutator response is a pure ratio. **No `a_phys`, `K_B`, or `t_phys` calibration declaration enters the construction.** F-h cannot fire (mechanical check, deferred to step 8).

### Step 2 -- Derive A_obs from §4 catalog primitives

Each step states the primitive(s) used + the algebraic operation + the FTD-native source:

1. **`J` (catalog item 2)** -- the flux field on the cubic lattice. Source: `FOUND_FORCE_STRUCTURE.md` postulate (J is the continuous-vector substrate field).
2. **Bilinears `J_i(x) J_j(y)` for `i ≠ j` and `x ≠ y` distinct sites within the Moore neighbourhood (catalog item 3).** Source: catalog item 3 (Moore-neighbourhood bilinears).
3. **The plaquette bivector** `P_{ij}(x) = J_i(x) J_j(x + ê_i) − J_i(x + ê_j) J_j(x)` is the antisymmetrized combination of two such bilinears over the four corners of an elementary 2×2 face (catalog item 4, the "closed 2-cycle face observable" of pre-reg §4 item 4 verbatim). Source: catalog item 4.
4. **The algebra `𝔄_P`** is generated by the three plaquette bivector species `{P_{xy}, P_{xz}, P_{yz}}` under the bilinear ring operations (sum, scalar multiplication, bilinear product, commutator). Per FTD-0086 §1.2-1.3, the *commutator structure* under FTD non-local dynamics empirically concentrates on the matching plaquette: `[Ê_f, Ê_g][P_a] ∝ δ_{a, \{f, g\}}` with signal/off-axis ≥ 15×. Per FTD-0088 §3.4.1 (Cl(3,0) grade decomposition), the algebra at 2-injection leading order is consistent with 𝔰𝔲(2) ≅ 𝔰𝔬(3) structure.

**No step uses items 8 or 9 (FQCR `T_O` or master quadratic / coefficient 16) as construction input.** The derivation is forward from items 2-4. F-j does not fire *at this step*; whether F-j fires depends on whether step 4's `T_O` construction smuggles M_N(t) (the FQCR Model V transfer matrix) in as scaffold -- which is the central step-4 + step-5 question.

### Step 3 -- Verify gauge / translation / O_h invariance of A_obs

- **Translation invariance.** `P_{ij}(x + a)` is well-defined for any lattice translation `a` (the four-corner stencil shifts uniformly). The algebra `𝔄_P` is generated by `{P_{ij}(x) | x ∈ L³}`; periodic boundary conditions make translation invariance manifest.
- **O_h invariance (the cubic point group, order 48).** The three plaquette bivectors `{P_{xy}, P_{xz}, P_{yz}}` form a basis for the 3-dimensional `T_{1u}` (vector-times-parity) irrep of O_h: the 3-cycle of `{x, y, z}` permutations cyclically permutes the three bivectors with appropriate signs; the 24 reflection generators of O_h act with the standard vector + parity rule. The algebra is closed under O_h.
- **Gauge invariance.** No explicit gauge symmetry is present in the pure-flux FTD substrate (J is a real-valued vector field, not a complex-phase field with U(1) gauge redundancy). The bivector products `J_i J_j` are functions of the gauge-free flux field. **Trivially gauge-invariant**; this is the strongest possible verification (no gauge redundancy exists to verify against).

**Step 3 PASS.** A_obs satisfies §5 contract item 2 (translation + O_h + gauge invariance).

### Step 4 -- Construct the transfer / readout operator T_O on A_obs

This step is the construction's center of mass. The question is: **what is the natural transfer/readout operator on the bivector algebra, derived from steps 2-3, that yields a characteristic equation in some unknown `x`?**

The bivector algebra `𝔄_P` is 𝔰𝔲(2)-like at 2-injection leading order (per FTD-0088 12/12 Cl(3,0) grade-skeleton consistency); call its three generators `B_a = ⟨P_a⟩` (axis-summed plaquette bivectors), `a ∈ {xy, xz, yz}`. The structure-constant tensor `f_{abc}` is determined empirically (per FTD-0086 magnitude / sign) and matches `ε_{abc}` up to a normalization scalar `κ` (≈40× signal at the matching plaquette, sign-consistent across the three (f, g) pairs).

The natural operator structures on this algebra:

- **Option (i) -- adjoint representation `ad_{B_a}`.** The adjoint action of `B_a` on the algebra is a 3×3 matrix `ad_{B_a}(B_c) = f_{abc} B_b`. For 𝔰𝔲(2)-like structure constants `f_{abc} = κ ε_{abc}`, the matrix `ad_{B_a}` has characteristic polynomial `z³ + κ² ⟨B⟩² z = 0` -- i.e. eigenvalues `{0, +iκ‖B‖, −iκ‖B‖}`. **This is not the master quadratic.**
- **Option (ii) -- Casimir operator `C = Σ_a B_a²`.** The Casimir is a scalar of the bivector algebra; its eigenvalue on the spin-1 representation is `j(j+1) = 2` (in standard normalization, or `j(j+1) κ²` if the structure constants carry magnitude `κ`). The Casimir's characteristic equation is the trivial `x − 2 = 0` (or `x − 2κ² = 0`). **Not the master quadratic.**
- **Option (iii) -- commutator-table operator (the 3×3 matrix whose entries are commutator values `[B_a, B_b]_c`).** This is essentially the structure-constant tensor evaluated on a fixed basis pair; its characteristic polynomial is `det(λI − [B_a, B_b]_·) = 0`, which for 𝔰𝔲(2) gives `λ³ ± κ³ = 0` or similar low-degree cubic-of-the-structure-constant form. **Not the master quadratic.**
- **Option (iv) -- transfer-matrix on a 2-dimensional subspace.** If one fixes a 2-dimensional subspace of the bivector algebra (e.g. `{B_1, B_2}` projected to neglect `B_3`) and constructs a `2 × 2` operator from the algebra's structure, the operator is generically of the form
  ```
  T_O = ⎡ a   b ⎤
        ⎣ c   d ⎦
  ```
  with `a, b, c, d ∈ ℝ` derived from the empirical structure constants `f_{abc}` and the Casimir scalar. The characteristic polynomial is `x² − (a+d) x + (ad − bc) = 0`. For this to be the master quadratic `x² − 16(G\*)² x + 16(G\*)³ = 0`, one would need `a + d = 16(G\*)²` (trace) and `ad − bc = 16(G\*)³` (determinant). **Neither condition is derivable from the bivector structure constants** -- the structure constants are pure-number ratios (the `κ ε_{abc}` form per FTD-0086 matching signature) and have no provenance in terms of `G\* = Γ(1/4) / Γ(3/4)` or its powers.

### Step 5 -- Compare T_O characteristic equation to the master quadratic

For each of the four natural T_O constructions in step 4:

- **Options (i)-(iii)** produce 𝔰𝔲(2)-type characteristic polynomials (`z³ + κ² z = 0`, `x − 2κ² = 0`, `λ³ ± κ³ = 0`). None of these is the master quadratic. The matrix structure on a 3-dim Lie algebra is fundamentally degree-three (cubic in z) or a one-dimensional scalar; the master quadratic is degree-two with very specific coefficients. **Forward derivation from the bivector algebra cannot produce a quadratic with the specific coefficients `16(G\*)²` and `16(G\*)³` because (a) `G\*` does not appear in any forward derivation chain starting from the cubic lattice's bivector structure** (G\* is a Γ-function ratio of the lemniscatic curve `E : y² = x³ − x`, an arithmetic object of the ℤ[i]-module / Chowla-Selberg / Watson-integral family per FTD-0001 / FTD-0002), **and (b) the coefficient `16 = |Aut(E)|²` is a structural invariant of the same lemniscatic curve, not the cubic lattice's bivector structure constants** (per FTD-0006 / FTD-0007 / FTD-0122).
- **Option (iv)** would require `a + d = 16(G\*)²` and `ad − bc = 16(G\*)³` to be derivable from the structure constants `f_{abc}` (themselves derivable from steps 2-3). The structure constants `f_{abc}` are empirically `κ ε_{abc}` for `κ ≈ 1` (the matching-signature magnitude per FTD-0086 normalized appropriately). There is **no derivation chain** from `κ ε_{abc}` to `16(G\*)² ≈ 140.05` (trace) or `16(G\*)³ ≈ 414.36` (determinant). The numerical mismatch is order 100×, and the structural mismatch is **categorical**: structure-constant ratios are dimensionless purely-combinatorial numbers; G\* is a transcendental Γ-function ratio that requires the lemniscatic curve's CM structure to define.

**Verdict at step 5:** the characteristic equation of every forward-derivable T_O on the bivector algebra is **not** the master quadratic. The construction produces a structurally distinct algebraic object (𝔰𝔲(2)-type characteristic polynomial in options (i)-(iii); a generic 2×2 quadratic with unrelated coefficients in option (iv)). Per §6 outcome (c): "the best candidate's `R(O_EM(P))` is provably NOT `1/x_+` -- the construction yields a different distinguished algebraic number." **CLOSED-NEGATIVE applies.**

Steps 6 and 7 are now logically moot for the FOUND verdict (a FOUND verdict requires step 5 structural match, which has failed); they are documented for completeness and to ensure the §9 11-step method is not short-circuited.

### Step 6 -- The dominant-branch selection rule (D4)

If step 5 had produced a structural match (which it did not), the candidate dominant-branch selection rule would be: **D4 (iii) -- dominance / spectral-largest-magnitude**. The largest-eigenvalue eigenvector of `T_O` would be the proposed electromagnetic readout direction.

**Honest concern (F-c risk).** Even if a structural match were forced (which would require importing the FQCR M_N(t) and firing F-j), the dominant-branch selection rule would face an F-c risk: the bivector algebra is *cyclically symmetric* in its three generators (the O_h action permutes them as a 3-cycle); there is no internal asymmetry that singles out a "dominant" plaquette pair. Selection of `x_+` versus `x_-` would have to come from outside the bivector structure -- e.g. from the operational protocol (D4 (iv) -- accessibility). Whether the operational protocol can distinguish the two roots from inside the bivector substrate without appealing to measured `α` is itself underdetermined.

Step 6 is **moot in the present verdict** (step 5 already returned CLOSED-NEGATIVE); the F-c concern is recorded for any future closure attempt that achieves a step-5 structural match through other catalog primitives (items 6 or 7).

### Step 7 -- The operational measurement protocol (D3)

If step 5 had produced a structural match, the candidate operational protocol would be: **the FTD-0086 measurement protocol verbatim** (L = 8, A = 10, deterministic seed, full non-local toggle set, WH weight-1 mode injection on two axes, plaquette-bivector commutator readout). This satisfies D3 (a)-(d):
- (a) preparation specified (WH-mode injection on axis pair `(f, g)`),
- (b) response measured (the plaquette-commutator concentration ratio per FTD-0086 Table 1.3),
- (c) measurement-apparatus interpretation (probe = the WH-mode injection structure; readout = the matching-plaquette commutator response),
- (d) reproducibility verified across the FTD-0086 / 0087 / 0088 campaign.

**Honest concern (F-d risk -- partial).** The operational protocol is reproducible at the *signature* level (matching plaquette concentration), but the *numerical extraction* of an electromagnetic-coupling-like scalar from the matching-signature ratio is unspecified. The FTD-0086 measurement returns "27×, 79×, 15×" -- signal/off-axis ratios with seed-dependent magnitudes -- not a specific scalar identified with `1/x_+`. Bridging the matching-signature ratio to a dimensionless inverse coupling is an additional step beyond the FTD-0086 measurement protocol. **The construction does not specify this bridge step,** so D3 (b) is partially incomplete.

Step 7 is **moot in the present verdict** (step 5 already returned CLOSED-NEGATIVE); F-d would also have fired had step 5 passed.

### Step 8 -- Per-falsifier checklist (mechanical)

| F-rule | Fires? | Reason |
|---|---|---|
| **F-a** (CODATA / α inserted) | **NO** | No numerical α value enters the construction; bivectors are dimensionless combinations of the dimensionless flux field. |
| **F-b** (free parameter set to α) | **NO** | The construction has no free parameter; the structure constants `f_{abc}` are empirically `κ ε_{abc}` with `κ ≈ 1` per FTD-0086, but `κ` is not a parameter to be tuned -- it is the FTD-native magnitude of the bivector matching commutator and is what it is. |
| **F-c** (dominant-branch selection ambiguous) | **moot** (step 5 already CLOSED-NEGATIVE) | If forced past step 5, the cyclic symmetry of the bivector algebra would face an F-c risk -- no internal asymmetry distinguishes `x_+` from `x_-`. |
| **F-d** (no operational protocol) | **moot, but partial firing on bridge step** | The FTD-0086 protocol provides matching-signature measurement but not the bridge to a dimensionless inverse coupling scalar. F-d would fire on the bridge step had step 5 passed. |
| **F-e** (site-local Clifford) | **NO** | Plaquette bivectors are non-site-local from step 1 (four lattice sites per face). FTD-0073 site-local closure does not apply. |
| **F-f** (QED normalization import) | **NO** | No QED textbook formula enters the construction. |
| **F-g** (relabelling of closed-negative route) | **NO** -- new mechanism class | The plaquette-bivector route is the FTD-0086 / 0088 STRONG POSITIVE empirical channel; it is genuinely distinct from FTD-0050 (RG-step polynomial), FTD-0073 (site-local Clifford), FTD-0094 (L2 substitution), FTD-0116 (Z-factor), FTD-0097 (look-elsewhere), FTD-0035 (a_phys mechanism γ), or the 4-leg classical-gauge channels (FTD-0004 / 0005 / 0125 / 0126). The mechanism class is the *Cl(3,0)-bivector readout on FTD-native non-local dynamics* -- not yet closed at the pre-reg lock. |
| **F-h** (calibration-dependent) | **NO** | The construction is dimensionless throughout. |
| **F-i** (look-elsewhere over a parameter family) | **NO** | The construction is a single forward derivation; no family-search is involved. The plaquette-bivector basis is a single canonical choice (the unique lattice 2-form on a cubic lattice). |
| **F-j** (master quadratic inserted not derived) | **THIS IS THE LOAD-BEARING RISK** | The construction in §1 step 4 carefully avoids importing M_N(t) as scaffold (deriving forward from catalog items 2-4 only). The step-5 verdict (CLOSED-NEGATIVE for option (iv) trace/determinant mismatch) is the honest reading. **Any "fix" that would force the master quadratic structure into the bivector T_O -- e.g. importing M_N(t) per FQCR Prop 5 and noting its char poly is the master quadratic -- would fire F-j irreversibly.** The CLOSED-NEGATIVE verdict at step 5 is preferred over an F-j firing because (a) step 5 is the substantive structural verdict, and (b) F-j is the reverse-engineering trap that the §8 banned moves are most concerned with. |

**Mechanical falsifier check: no falsifier fires on the construction as derived.** The verdict is the step-5 structural-mismatch finding (§6 outcome (c)), not a falsifier firing.

### Step 9 -- Per-banned-move checklist (mechanical)

| Banned move | Invoked? | Reason |
|---|---|---|
| No CODATA / α value anywhere | **NO** | (Construction is dimensionless throughout; no α reference except in benchmark §5 and falsifier §7 of the pre-reg.) |
| No new free integer / exponent / coefficient / group | **NO** | (Construction uses only the cubic lattice + flux field + bivector primitives of catalog items 2-4.) |
| No reverse-engineering from `x_+` to `T_O` | **NO** | (Construction is forward from catalog primitives.) |
| No "master quadratic is FTD's central content therefore it must appear" appeal | **NO** | (Honest verdict: the master quadratic does **not** appear in the bivector algebra's forward-derivable T_O.) |
| No QED formula imports | **NO** | (No QED formula in the construction.) |
| No site-local Clifford embedding | **NO** | (Plaquette bivectors are non-site-local by construction.) |
| No `g_c` insertion | **NO** | (g_c does not appear.) |
| No "visual" / "geometric analogy" as measurement rule | **NO** | (The operational protocol is the FTD-0086 verbatim measurement protocol, not a geometric analogy.) |
| No `x_+ ↔ 1/α` identification before deriving the readout | **NO** | (The construction does not assume the identification; the step-5 verdict is precisely that the identification cannot be derived from this route.) |
| No retroactive editing of the pre-reg | **NO** | (This audit document is separate from the pre-reg; the pre-reg is unedited.) |
| No spine tag moves before ARC-3 | **NO** | (No tag move occurs in this audit.) |
| CLOSED-NEGATIVE stays a live option | **YES** (this is what landed) | (Verdict is CLOSED-NEGATIVE per §6 (c).) |

**Mechanical banned-moves check: no banned move was invoked.** The construction is clean of all eleven prohibitions.

### Step 10 -- Numerical comparison

**Skipped per §9 step 10 rule** ("Only if steps 1-9 pass..."). Step 5 returned CLOSED-NEGATIVE, so no numerical comparison `R(O_EM(P_canonical)) ?= 1/x_+` is performed. **This is the discipline working as designed**: comparing a structurally mismatched output to `1/x_+` would be an F-i look-elsewhere violation; the §9 step ordering prevents this.

### Step 11 -- Verdict

**CLOSED-NEGATIVE per §6 (c)** for the plaquette-bivector route. The specific obstruction: **the forward-derivable characteristic equation of T_O on the bivector algebra `𝔄_P` is not the master quadratic** -- bivector commutator structure produces 𝔰𝔲(2)-type characteristic polynomials whose coefficients are pure-combinatorial structure-constant ratios (`κ ε_{abc}` with `κ ≈ 1`), not the lemniscatic curve's number-theoretic invariants (`16 = |Aut(E)|²`, `G\* = Γ(1/4) / Γ(3/4)`).

The structural categorical mismatch is the load-bearing finding: **the cubic-lattice plaquette bivector structure and the lemniscatic-curve / ℤ[i]-module / Chowla-Selberg arithmetic structure of the master quadratic's coefficients are different mathematical objects** without a known derivation chain from the former to the latter. The bivector route is closed-negative not because of insufficient computation but because of categorical structural mismatch at the level of what kind of algebraic object each side is.

---

## §2 -- What is closed; what is open

**CLOSED-NEGATIVE (this verdict):**
- The plaquette-bivector route (catalog item 4) for ARC-B1 observable-selection alpha-readout.
- The specific sub-claim that the FTD-0086 / 0088 STRONG POSITIVE bivector matching signature provides an alpha-readout mechanism. The signature is structural evidence for Cl(3,0) carrying through to FTD-native dynamics, but it does **not** generate the master quadratic's coefficient structure.

**NOT closed by this verdict:**
- ARC-B1 as a whole. Two catalog items remain unattempted: catalog item 6 (boundary-to-boundary transfer observables) and catalog item 7 (reflexive projections). Each requires its own pre-reg-compliant closure attempt against the same locked pre-reg (FTD-0198).
- The FTD-0086 / 0088 bivector campaign as evidence for Cl(3,0) structure in FTD's non-local dynamics. **This evidence stands unchanged**; the present verdict only forecloses the *alpha-readout* application of the bivector structure.
- ARC-A (boundary-condition), ARC-C (quantization rule), ARC-D (discrete-native measurement). Each gets its own pre-registration if/when pursued.
- The master quadratic itself (FTD-0001 [THEOREM]). The verdict does not affect this; the master quadratic remains a theorem about the lemniscatic curve's CM structure, the verdict only states that this algebraic structure does not emerge from cubic-lattice bivector commutators.

**Spine status unchanged:**
- FTD-0001 (master quadratic): [THEOREM]. Untouched.
- FTD-0006 / 0007 (coefficient 16 = |Aut(E)|²): [THEOREM]. Untouched.
- FTD-0013 (`x_+ = 1/α` identification): **[STRONGLY MOTIVATED CONJECTURE]. Tag unchanged** -- this verdict neither promotes nor demotes it. The conjecture remains supported by the structural-uniqueness evidence of FTD-0189 (master quadratic is the unique dual-matcher across 2.65M polynomials over an 18-constant basket) and the bridge identity `G\* = 2√π G_G` and the various spine-level theorems; the bivector route does not contribute to its support, and its closing-negative does not detract from its support either.

---

## §3 -- Load-bearing input for Path II (FTD-0186 v2 Stage 2)

This CLOSED-NEGATIVE verdict is **directly load-bearing for the Path II boundary theorem program** (FTD-0186 v2, hash-locked at commit `d550bca` per Session A2):

- **One more closed-negative dynamical-value derivation attempt.** This is the 12th α-derivation-route closed negative (the previous 11 are cataloged in `SPEC_OPEN_MATH_BY_SECTOR.md` §2 closed-negative list). Under the v2 falsifier criterion A1 ("every closed-negative recording a failed attempt to derive a non-universal *dynamical value* targets a quantity classified NON-UNIVERSAL DYNAMICAL or CALIBRATION-CONDITIONAL"), this verdict classifies as a **type-i closed-negative** (failed derivation of a non-universal dynamical value, target = α / `1/x_+`) and stays consistent with FTD-0186 v2 Outcome A.
- **Narrows the surviving search space within ARC-B1.** Three catalog items remain (6 / 7 / variants of 4 with finer subalgebra selection); two are unattempted (6 / 7) and one would require fresh v2 design (subalgebra variant of 4). If those also close negative, ARC-B1 as a whole closes negative, and the surviving alpha-readout search narrows to ARC-A / ARC-C / ARC-D.
- **Contributes structural evidence for the boundary theorem.** Stage 2 of the boundary theorem aims to prove that the discrete substrate does not fix non-universal dynamical values. Each closed-negative dynamical-value derivation attempt is empirical evidence consistent with this thesis. Stage 2 must still be pursued *as a genuine provable proposition with stated axioms*, per FTD-0186 v2 §1 honest framing; the closed-negative record is consistent with Stage 2 but does not establish it.

---

## §4 -- Honest accounting + load-bearing methodological notes

**This verdict is the discipline working as designed.** The §9 11-step method was executed in order. The step-5 structural verdict was reached before the step-10 numerical comparison (which was correctly skipped). The §7 falsifier checklist and §8 banned-moves checklist were applied mechanically. No falsifier fires; no banned move was invoked; the verdict is the substantive step-5 finding (categorical structural mismatch between bivector commutator algebra and the master quadratic's CM-curve coefficient structure).

**The temptation to force a structural match.** The strongest pressure during this closure attempt was to try option (iv) (2×2 transfer matrix on a bivector subspace) and force `a + d = 16(G\*)²` and `ad − bc = 16(G\*)³` by appropriate choice of basis. Doing so would have required either: (a) importing the FQCR M_N(t) form as scaffold (firing F-j), or (b) introducing free parameters to fit the trace/determinant (firing F-b). The discipline of writing the CLOSED-NEGATIVE verdict instead of forcing the match is exactly what the pre-reg's §8 banned moves are protecting against. Per §8 final bullet: "CLOSED-NEGATIVE stays a live option throughout. The closure attempt's author must be willing -- and equipped -- to write the CLOSED-NEGATIVE report. Engineering toward FOUND is itself a process violation that yields no admissible verdict."

**What this verdict would falsify if it were wrong.** If a future bivector-route attempt does produce a forward-derivable T_O whose characteristic equation IS the master quadratic, *without* importing M_N(t) and *without* introducing free parameters, that would falsify this verdict. The verdict is reproducible: the §1 step 1-5 construction is mechanical, and any alternative T_O construction within the bivector algebra is constrained by the algebra's known 𝔰𝔲(2)-like structure.

**Comparison with FTD-0186 v1 falsifier firing.** The FTD-0186 v1 → v2 cycle (Session A2) established the precedent that a CLOSED-NEGATIVE result is *not* a failure but a load-bearing finding that sharpens the next pre-registration. The present CLOSED-NEGATIVE verdict follows the same pattern: it identifies a specific mechanism class (cubic-lattice bivector commutator → master-quadratic coefficient structure) that does not close the alpha-readout problem, and the surviving search space narrows accordingly.

---

## §5 -- LEDGER + cross-references

LEDGER row FTD-0204 [CLOSED NEGATIVE] records this verdict per §6 (c) of the FTD-0198 pre-reg. No FTD claim promoted or demoted.

Cross-refs:
- FTD-0198 ([PRE-REGISTRATION], hash-locked at `0e79820`, tag `preregister-alpha-readout-observable-selection-v1`, SHA256 `e273ca85234c04406c14b0b0bb01bb2ea760367ca7286c2b35649b80563b582a`) -- the pre-reg this verdict is the §9 execution of.
- FTD-0086 / 0087 / 0088 (bivector campaign STRONG POSITIVE / PARTIAL / POSITIVE) -- the FTD-native source of the candidate. Unchanged; the bivector signature remains valid evidence for Cl(3,0) emergence in non-local dynamics.
- FTD-0001 (master quadratic), FTD-0006 / 0007 (coefficient 16), FTD-0002 (G\*), FTD-0013 (`x_+ = 1/α` SMC) -- the derivation targets; all tags unchanged.
- FTD-0186 v2 (boundary theorem Stage 1 CLOSED POSITIVE per v2, hash-locked at `d550bca`, tag `preregister-structural-dynamical-discriminator-v2`) -- this verdict contributes to v2's type-i closed-negative count without firing v2's A1; consistent with Stage-1 Outcome A.
- FTD-0073 (site-local mode-erasure no-go) -- the no-go this construction's non-site-locality breaks; F-e does not fire on the bivector A_obs.
- FTD-0050 / 0094 / 0097 / 0116 / 0035 / 0093 / 0031 -- the prior closed-negative routes that F-g checks against; this verdict is genuinely distinct from each.
- ARC-B1 catalog items 6 + 7 -- the unattempted observable classes that remain open within ARC-B1.
- `.claude/plans/let-s-proceed-on-the-eager-rocket.md` Session C1 (this verdict) + Session C2 (refinement/iteration if pursued) + Sessions C3-C4 (alternative A_obs candidates 6 / 7 if pursued).

---

*End of verdict. Plaquette-bivector route CLOSED-NEGATIVE per §6 (c). ARC-B1 as a whole remains open pending closure attempts against catalog items 6 (boundary-to-boundary transfer) and 7 (reflexive projections).*
