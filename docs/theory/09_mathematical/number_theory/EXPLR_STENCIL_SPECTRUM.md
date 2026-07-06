# EXPLR — The stencil spectrum of N_dyn: the arithmetic of the spec's three lattice symbols

**Tag:** [EXPLR — B0 of the Clause-2/3 program; literature verdict recorded, exact-ODE stage queued]. Introduces no claim beyond the classification below; promotes nothing.
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
| **18-pt (SC+FCC)/2 — the engine default** | 1 − (1/6)Σc_i − (1/6)Σc_i c_j | ≈ 1.2679 (AUDIT_LINK8_CLOSURE §2 numeric) | **UNKNOWN closed form.** Literature verdict B0(i): the *class* is covered — 3D LGFs of general symbols satisfy low-order linear ODEs derivable by creative telescoping (Guttmann's LGF/Calabi–Yau program; Joyce–Delves methods extend to next-nearest-neighbor couplings) — but **no closed-form evaluation of this mixed symbol was found**. Whether its ODE is of CM/modular type (like the classical lattices) or a generic Calabi–Yau-class operator is open. | Guttmann 2010 (LGFs in all dimensions); Guttmann, LGFs & Calabi–Yau differential equations; Joyce–Delves anisotropic-cubic series |

Consequence for the program, stated honestly: the engine's default linear sector may be arithmetically *generic* (non-CM) — the "nice" Γ-class content of N_dyn enters through the sublattice projections, not the default stencil. If B0(ii) confirms a non-CM operator, that is a finding, not a failure: it would say the substrate's arithmetic distinction lives in the Moore decomposition's sublattices (where the corpus already placed it: BCC ↔ the spine) rather than in the isotropized mixture. [coherent-interpretation, pending B0(ii)]

## §2 — B0(ii), queued: the exact route

Derive the 18-pt diagonal LGF's linear ODE by creative telescoping (exact; sympy/ore-algebra-style; time-boxed), then classify: MUM/Calabi–Yau type? modular/CM solution? Deliverable: the operator + classification, or an honest failure note. **Discipline: no free-form PSLQ closed-form fishing.** If a bounded PSLQ negative-scoping pass is ever wanted, it must be pre-registered in this document (basket + bounds declared) before running — none is registered as of this writing.

## §3 — Falsifier / closure

B0 closes when the 18-pt row's status cell reads one of {closed-form identified (cited), ODE derived + classified, UNKNOWN — attempted, obstruction recorded}. Falsifier of §1's "uncharted" claim: a literature closed form for the (SC+FCC)/2 mixture (would immediately upgrade the row and sharpen E1's statement).

## §4 — Cross-references

`ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369 — why the SC/FCC rows force E1); `AUDIT_LINK8_CLOSURE.md` §2 (the stencil decomposition + W₁₈ numeric); `EXPLR_HIGHER_DIM_WATSON.md` (the D ≥ 3 Watson family); `REF_BIBLIOGRAPHY.md` §5 (Watson 1939; Glasser–Zucker); the pending `REF_EXPORTED_PROBLEMS_E1_E2.md` (B3).
