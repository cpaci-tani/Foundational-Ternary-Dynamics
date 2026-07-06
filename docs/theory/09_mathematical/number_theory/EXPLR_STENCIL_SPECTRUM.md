# EXPLR — The stencil spectrum of N_dyn: the arithmetic of the spec's three lattice symbols

**Tag:** [EXPLR — B0 of the Clause-2/3 program; literature verdict recorded, exact-ODE stage executed — negative at declared bounds, holonomic-but-large]. Introduces no claim beyond the classification below; promotes nothing.
**Program:** Clause-3 ("N as the object"), stage B0. LEDGER: maintenance-log line under FTD-0368's program (no new id until B0 completes).
**Audience:** agents working the δ-IND residues (E1's precise statement), the Ram(N) flagship, or any future evaluation of the engine's own Green's function.

---

## §0 — The question

N_dyn's generators are limits of solves against the spec's lattice symbols. Which symbols, and what is the *arithmetic* of each symbol's Green's function? The engine's default is the 18-point (SC+FCC)/2 Laplacian with **zero BCC weight** (AUDIT_LINK8_CLOSURE §2); the BCC and SC symbols enter as sublattice projections (spec-level modes; D2-scope adjudication pending the A0 audit). The striking gap: **the engine's own default Green's function is arithmetically uncharted** — W₁₈(0) ≈ 1.2679, no closed form, no CM status, nothing documented.

## §1 — The spectrum table (verdict as of B0(i), 2026-07-05)

| symbol | σ(k) | Green's value at 0 | arithmetic status | source |
|---|---|---|---|---|
| BCC (8 corners) | 1 − c_x c_y c_z | G\*²/(2π) = Γ(1/4)⁴/(4π³), **exact** | **CM, τ = i** (lemniscatic); hull-class ℚ̄·s⁴w⁻⁴ | Watson 1939; spine Thm 5 / OT-2.1 |
| SC (6 faces) | 3 − Σc_i (norm.) | ≈ 0.505462019 (Watson's I₃, 9 digits in-corpus) | **Γ(1/24)-class** closed form (Glasser–Zucker); outside ℚ(G\*, π) — the source of E1 | Watson 1939; Glasser–Zucker 1977/1980 |
| FCC (12 edges) | 3 − Σc_i c_j (norm.) | tabulated | **Γ(1/3)-class** (equianharmonic-adjacent); outside ℚ(G\*, π) — E1's second member | Glasser–Zucker 1980 |
| **18-pt (SC+FCC)/2 — the engine default** | 1 − (1/6)Σc_i − (1/6)Σc_i c_j | ≈ 1.2679 (AUDIT_LINK8_CLOSURE §2 numeric) | **UNKNOWN closed form; holonomic but large.** Literature verdict B0(i): the *class* is covered — 3D LGFs of general symbols satisfy linear ODEs (holonomic; Lipshitz 1988) derivable by creative telescoping (Guttmann's LGF/Calabi–Yau program; Joyce–Delves methods extend to next-nearest-neighbor couplings) — but **no closed-form evaluation of this mixed symbol was found**. **B0(ii) executed** (§2): the moment generating function is holonomic (constant term of a rational function — the ODE is *guaranteed* to exist), yet exact differential-approximant reconstruction on 85 moments finds **no annihilating operator of (order ≤ 6, degree ≤ 8)** — the minimal operator is *larger* than the pure simple-cubic symbol's (order 3), so the mixture is arithmetically harder, not simpler. CM/modular-vs-generic classification still open. | Guttmann 2010 (LGFs in all dimensions); Guttmann, LGFs & Calabi–Yau differential equations; Joyce–Delves anisotropic-cubic series; Lipshitz 1988 (holonomicity) |

Consequence for the program, stated honestly: the engine's default linear sector may be arithmetically *generic* (non-CM) — the "nice" Γ-class content of N_dyn enters through the sublattice projections, not the default stencil. If B0(ii) confirms a non-CM operator, that is a finding, not a failure: it would say the substrate's arithmetic distinction lives in the Moore decomposition's sublattices (where the corpus already placed it: BCC ↔ the spine) rather than in the isotropized mixture. [coherent-interpretation, pending B0(ii)]

## §2 — B0(ii), executed: the exact-reconstruction attempt (verdict: holonomic but large)

**Well-posedness first.** F(z) = Σ_n m_n z^n with m_n = CT_k[σ₁₈(k)^n] is the constant term (in the torus variables) of the rational function 1/(1 − z·σ₁₈), hence **D-finite / holonomic** by the constant-term-of-a-rational theorem (Lipshitz 1988; diagonals/constant terms of rational power series are D-finite). *An annihilating linear ODE with polynomial coefficients is therefore guaranteed to exist* — B0(ii)'s only real questions are its order/degree and thence its arithmetic class. This also confirms P3(a) (`REF_EXPORTED_PROBLEMS_E1_E2.md`) is well-posed, not a fishing expedition.

**Method (exact, declared bounds, no PSLQ).** Compute exact integer moments 24ⁿ·m_n = CT[(2A+B)ⁿ] with A = Σ(x_i+1/x_i), B = Σ_{i<j}(x_i+1/x_i)(x_j+1/x_j) (integer Laurent polynomial), then solve **exactly over ℚ** for a differential approximant (linear ODE, deg-bounded polynomial coefficients) demanding ≥ 8 surplus equations — the standard LGF-reconstruction route used where a full CAS creative-telescoping stack (Koutschan's *HolonomicFunctions*, `ore_algebra`) is out of environment. Scripts: `scripts/proofs/explr_stencil18_ode_attempt.py` (RUN 1: order ≤ 4, degree ≤ 10, 37 moments) and `explr_stencil18_ode_attempt_run2.py` (RUN 2, the one declared extension: {5}×{6..9} ∪ {6}×{6..8}, 85 moments). **Pipeline validated**: the same code recovers the *known* simple-cubic LGF ODE (order 3, degree 6) before the 18-pt run — so the negatives are trustworthy, not a broken solver.

**Result (NEGATIVE at declared bounds).** On 85 exact moments, no annihilating ODE exists with (order ≤ 4, degree ≤ 10), (order = 5, degree ≤ 9), or (order = 6, degree ≤ 8). Because holonomicity is *guaranteed*, this is a **lower bound on operator complexity**, not a non-existence result: the 18-pt minimal operator exceeds each tested (order, degree) box — decisively larger than the pure simple-cubic operator (order 3), consistent with the isotropized mixture being arithmetically *harder* than its constituents. This sharpens §1's coherent-interpretation: the substrate's "nice" Γ-class content lives in the Moore sublattices (BCC ↔ the spine), while the default stencil's own Green's function is a large-operator object of as-yet-unclassified type.

**Discipline note.** No free-form PSLQ closed-form fishing was run and none is registered; a bounded PSLQ negative-scoping pass would require pre-registration here (basket + bounds) before running.

## §3 — Falsifier / closure

**B0 CLOSED at "UNKNOWN — attempted, obstruction recorded"** (one of the three declared closure states): closed form not identified, ODE not produced (minimal operator exceeds the exact-reconstruction bounds reachable without a CAS telescoping stack), holonomicity established. Reopen paths: (i) a literature closed form for the (SC+FCC)/2 mixture — would upgrade the §1 row and sharpen E1/P3; (ii) a CAS creative-telescoping computation of the operator + its monodromy/CM classification (deferred, out-of-environment) — would resolve P3(b). Falsifier of §1's "uncharted" claim remains: any cited closed-form evaluation of the mixed symbol.

## §4 — Cross-references

`ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369 — why the SC/FCC rows force E1); `AUDIT_LINK8_CLOSURE.md` §2 (the stencil decomposition + W₁₈ numeric); `EXPLR_HIGHER_DIM_WATSON.md` (the D ≥ 3 Watson family); `REF_BIBLIOGRAPHY.md` §5 (Watson 1939; Glasser–Zucker; Lipshitz 1988; Koutschan 2013); `REF_EXPORTED_PROBLEMS_E1_E2.md` §P3 (the mathematician-facing statement of this exact question). Verifier scripts: `explr_stencil18_ode_attempt.py`, `explr_stencil18_ode_attempt_run2.py`.
