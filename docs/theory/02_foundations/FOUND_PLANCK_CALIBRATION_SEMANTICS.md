# FOUND — The Semantics of the Planck Calibration: which lattice object carries ℓ_P, and why

**Tag:** [SYNTHESIS] + [DERIVED — formalization] — assembles classical lattice facts and the calibration register into one naming map; **introduces no new mathematics and promotes nothing**. Owner-directed re-exploration (2026-07-13) of the Planck-length semantics "as opposed to the existing 1/√3."
**LEDGER id:** FTD-0385 · **Canonical inputs:** `DERIV_DIMENSIONAL_GATE.md` (the 2026-07-08 tick correction, three-route), `FOUND_ELECTRON_PRIMARY_GAUGE.md` (FTD-0137), `SPEC_IMPORT_LEDGER.md` IMP-K1/K2, `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` (grade-0), constitution §3.3.
**Precedence:** LEDGER > constitution > this doc.

## §1 · The objects (one tick's natural geometry — the √3-ladder)

On the cubic lattice with edge `a` and Moore (26-neighbour) causality, a single tick has exactly three natural lengths, in geometric progression by √3:

| Rung | Length | What it is | Grade |
|---|---|---|---|
| bottom | `a/√3` | the **light** displacement per tick — the CFL/leapfrog wave speed `c_lat = 1/√3` voxel/tick | [THEOREM] (leapfrog wave equation; `DERIV_DIMENSIONAL_GATE.md` §; `C_SPEED` in the engine) |
| middle | `a` | the **edge** — the axis-aligned causal reach per tick (the L∞/Chebyshev bound: one site per axis per tick, P4) | [AXIOM] (P1/P4 geometry) |
| top | `a·√3` | the **body-diagonal causal reach** per tick (a diagonal Moore neighbour is one tick away) | [AXIOM] geometry; its *emergent-isotropy* status is the live PL-4 diagonals question |

Two speeds therefore coexist by construction: the causal bound (1 voxel/axis/tick; Euclidean reach up to √3·a/tick on diagonals) and the emergent wave speed (1/√3 voxel/tick, the speed physical light is identified with). The causal cone strictly contains the light cone — a lattice feature, not a bug; the UV anisotropy between them is exactly what PL-4/PL-5 measure.

## §2 · The naming theorem (why the edge carries ℓ_P)

With the calibration of record — `a_phys ≡ ℓ_P` (IMP-K1) and `t_phys = ℓ_P/(√3·c) = t_P/√3` (IMP-K2, corrected 2026-07-08; the authoritative JSON reconciled 2026-07-13) — the following identity holds exactly:

> **c · t_P = c · (√3 ticks) = √3 · (a/√3) · (a_phys/a per tick)⁻¹-normalized = a_phys = ℓ_P.**
> In words: **light crosses one voxel in exactly one Planck time.** The edge gauge is the unique naming under which the *standard operational definition* of Planck units — ℓ_P is the distance light travels in t_P — holds on the lattice with c = the emergent wave speed. ([DERIVED — three independent routes in `DERIV_DIMENSIONAL_GATE.md`: the Courant relation, the √3-ticks-per-voxel crossing, and von-Neumann stability.])

The apparent oddity that motivated this re-exploration — "the tick is not the Planck time; there's a loose 1/√3" — dissolves: the tick is t_P/√3 *precisely so that* c·t_P = ℓ_P can hold. The 1/√3 is not a competing Planck length; it is light's per-tick fraction of the edge, and the √3s cancel exactly where the operational definition requires.

## §3 · Two independent selectors, one gauge

The edge gauge is selected twice over, independently:
1. **Operationally** (§2): it is the only rung of the ladder for which c·t_P = ℓ_P holds — naming either other rung "ℓ_P" breaks the defining identity of Planck units.
2. **Empirically:** under the electron-primary default (FTD-0137), the calibration chain *derives* the **edge** ≈ ℓ_P to **0.19%** (`a_phys = ℓ_P` [DERIVED ~0.19%]). A tick-carrier re-gauge (ℓ_P := a/√3, making tick = t_P) would relocate the derived object to √3·ℓ_P — a 73% mismatch with its own name — while also breaking selector 1.

**Verdict of this exploration: the current naming is coherent and doubly selected — [SELECTION with operational + empirical backing], not an arbitrary convention.** The re-gauge alternatives are honestly mapped and rejected: the tick-carrier gauge fails both selectors; "redefining" `C_SPEED = 1/√3` is not a naming act at all (it is the CFL stability theorem — dynamics, untouchable semantically); the full Planck-primary restructuring remains the separate standing owner decision FTD-0130 path-(b), untouched here.

## §4 · What no gauge can do (the walls, restated)

Grade-0 closure ([DERIVED — formalization], A2): **no dimensional constant is native** — every naming of ℓ_P is a re-gauging of an import, never a derivation; at least one calibration anchor is irreducible in principle. All dimensionless predictions are calibration-invariant (the falsifiable spine); a re-gauge moves **no** physics and **no** tag. Per AM-5 (Consumption Program charter), any future re-gauge that re-denominates ledger prices routes to the owner as a charter amendment, never through a pipeline.

## §5 · Naming cleanups executed with this doc

(i) `import_ledger.json` IMP-K2 reconciled to the 2026-07-08 correction (the rendered spec had been fixed; the authoritative JSON had not — commit `6edb3538`). (ii) `docs/SPEC_FTD.md` §7.2: the `C = 1.0` row now says what it is (the L∞ causal bound, not the light speed) and the `H` row names the edge/ℓ_P calibration with its grade. (iii) The tick ≠ t_P fact and the √3-ladder are now stated in one citable place (this §1–§2).

*Zero promotions: IMP-K1/K2 stay at their ledger grades; x₊=1/α [SMC]; MC-T4.3 [FOUNDATIONAL OBSTRUCTION]. Golden untouched (docs + one JSON reconciliation).*
