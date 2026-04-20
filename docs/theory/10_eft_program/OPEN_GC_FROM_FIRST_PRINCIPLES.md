# OPEN — Deriving g_c from First Principles (Phase I Item 3)

**Tag:** [OPEN] — structural statement of the remaining physics problem
required to elevate the master quadratic from [STRONGLY MOTIVATED
CONJECTURE] to [THEOREM].
**Date:** 2026-04-19
**Status:** scoping doc — lays out the problem, three candidate
mechanisms, and the tests that would discriminate. Does NOT solve
the problem.

---

## 1 · The problem stated precisely

Phase G established that the engine's emergent Gauss law
`∇·J = g_c · s` has `g_c = 1` by default, producing
**geometric Coulomb** with `α_r(r, L) = 2 r G_L(r)`.

Phase H verified that introducing an explicit coupling constant `g_c`
scales the measurement to
`α_r(r, L; g_c) = g_c² · 2 r G_L(r)`,
matching the Phase G scaling theorem to 0.0000% precision. So the
mechanism `g_c² ↦ α` is mechanically sound.

For the engine to reproduce QED Coulomb (`α_r → α_ref` in the small-r
large-L limit), we need

```
  g_c² · 1/(2π) = α_ref = 1/137.036
  g_c² = 2π / 137.036 = 0.04585
  g_c  = 0.2141       (engine convention)
```

**The question:** why THIS value of g_c, from first principles?

If FTD claims α is derived from lattice geometry, then g_c must be
derivable too — not put in by hand. The master quadratic's claim
`x_+ = 1/α` is only falsifiable (in the dynamics) if we can predict
g_c independently of α and then check whether it agrees.

Currently the engine has `G_C = sqrt(α)` hardcoded in `ontic.h`, and
`ALPHA_EFT = G_C²` by construction. These are target values, not
derivations.

## 2 · What "first-principles g_c" would mean

Three logically-distinct possibilities:

### 2.1 · Mechanism A — Dirac-quantisation / topological

In continuum QED, Dirac showed that if magnetic monopoles exist, then
the electric charge is quantised: `eg = 2πn` for integer n. This is a
*topological* quantisation — the product eg is fixed by the topology
of the gauge bundle, independent of any dynamical input.

**Analog in FTD:** the state field is already quantised (`s ∈ {−1, 0, +1}`).
Is there a topological invariant of the lattice that fixes g_c
similarly? Candidates:

- Wilson-loop winding around the Moore neighborhood → may fix
  `exp(i g_c · A) · exp(−i g_c · A) = 1` modulo full cycles, giving
  `g_c = 2π / L_cycle` for some integer cycle length.
- BCC/FCC sublattice linking numbers → topological phase around a
  closed lattice loop could quantise a charge-flux product.

**Test:** compute the Wilson-loop expectation around a small closed
path on the Moore neighborhood (or BCC sublattice). If it's quantised
in units of 2π and one can identify `g_c` with the quantisation unit,
this forces g_c from topology alone.

**Status:** not attempted. Would require new engine code to measure
Wilson loops on closed Moore-neighborhood paths and extract the
quantisation unit.

### 2.2 · Mechanism B — Lattice-to-continuum matching

Lattice gauge theory has a standard matching procedure: the bare
lattice coupling `g_0` and the continuum renormalised coupling `g_R`
(e.g., MS-bar at a given scale μ) are related by

```
  1/g_R²(μ) = 1/g_0² + b_0 log(μ·a) + O(g_0²)
```

where `a` is the lattice spacing and `b_0` is the one-loop β-function
coefficient.

**Analog in FTD:** given the lattice Lagrangian (to be derived from
the engine's tick rule: wave equation + Gauss projection + state
update), compute the 1-loop matching coefficient between g_c (bare
lattice) and the continuum coupling `e = √(4π α)`. If the matching
gives a fixed bare g_c (up to the QED renormalisation flow), then
FTD's master quadratic value of α is the matched continuum coupling
at a specific scale.

**Problem:** FTD's engine is classical (no quantum loops). There's no
explicit β-function to match against. One would need to:
1. Promote the engine to a partition function `Z = ∫ DJ e^(−S[J])`
   with some action S (to be derived from the tick rule's
   time-discretisation).
2. Extract the 1-loop matching coefficient from S.
3. Compare to the QED 1-loop β-function coefficient
   `b_0 = (1/12π²) × N_f × Q²`.

**Status:** partially started. `DERIV_LAGRANGIAN.md` (if it exists) or
the ontic.h derivation chain has a candidate action. But the step
from "classical equations of motion" to "partition function with
measurable β" has not been rigorously taken.

### 2.3 · Mechanism C — self-consistency / gap-equation fixed point

If the bare coupling `g_c` flows under some RG transformation to a
fixed point `g_c*`, and if the master quadratic is the self-consistency
condition for that fixed point, then `g_c*` is determined by the fixed
point equation without free parameters.

The master quadratic `x² − 16G*²x + 16G*³ = 0` has the form
`x · (x − 16G*²) = −16G*³`
which can be read as "coupling × (total vacuum capacity − coupling) =
− screened capacity × G*" — a self-consistency statement.

**Problem:** Phase I Item 1 (`audit_gap_equation_convergence.py`) shows
that the naive finite-L gap equation does NOT converge cleanly to the
master quadratic. So the "master quadratic is the L → ∞ fixed point of
a specific finite-lattice gap equation" claim is not currently
numerically established. The identity `x_+ = 1/α` remains exact
algebra, but its interpretation as a flow-to-fixed-point is
[CONJECTURE].

**Status:** [OPEN]. Would require either:
- A rigorous proof that SOME sequence of finite-L gap equations
  converges to the master quadratic as L → ∞ (not yet found), OR
- Abandoning the "self-consistency" narrative and reading the master
  quadratic as an algebraic coincidence supported by the dual-match
  (x_+ ↔ 1/α, x_- ↔ N_c) of Phase I §2.5.

## 3 · What this means for the overall α claim

Accepting Phase I as-is, the honest status of FTD's α derivation is:

| Claim | Status |
|---|---|
| Master quadratic x² − 16G*²x + 16G*³ = 0 has roots 137.036 and 3.024 | [THEOREM] — pure algebra |
| Coefficient 16 = \|Aut(E)\|² for E: y² = x³ − x | [THEOREM] |
| G* = Γ(1/4)/Γ(3/4) is a canonical period | [THEOREM] |
| Of 9 class-number-1 CM curves, ONLY y² = x³ − x gives master-quadratic-form roots matching both 1/α and N_c | [THEOREM] (Phase I §2, via scan_cm_curves.py) |
| The master quadratic is the L → ∞ limit of a finite-lattice gap equation | [CONJECTURE / OPEN] (Phase I §1, not verified) |
| g_c = √(2π α) has a first-principles derivation independent of α itself | [OPEN] (this doc §2) |
| x_+ = 1/α is a physical identification, not a numerical coincidence | [STRONGLY MOTIVATED CONJECTURE] (dual match + CM curve uniqueness) |

Removing the "gap-equation L → ∞" link (which was supposed to supply
the first-principles grounding) leaves the master quadratic as:

**An exact algebraic identity whose larger root matches 1/α to 1.26
ppm and whose smaller root matches N_c to 0.80%, uniquely among the
class-number-1 CM elliptic curves; the identification of the roots
with physical constants is [SELECTION], not [DERIVATION].**

## 4 · Concrete next steps (prioritised)

**(1) Write a rigorous ℤ³-to-master-quadratic derivation.** Either
produce a sequence of finite-lattice gap equations that provably
converges to the master quadratic (closing the Item 1 gap), or retract
the "gap equation" narrative entirely and present the master quadratic
as pure algebra + coincidence + CM-uniqueness.

**(2) Derive the engine's Lagrangian from the tick rule.** The
wave equation + Gauss projection + state update should be reducible
to a partition function `Z = ∫ DJ Ds e^(−S)` by time-discretising
the equations of motion. If the resulting S has a natural bare g_c,
that's the answer; if it has a free parameter that must be tuned to
match QED, that parameter is the insertion point.

**(3) Test Mechanism A on the engine directly.** Compute Wilson loops
around small closed paths on Moore-neighborhood BCC sublattices; look
for topological quantisation. Can be done with an engine-level
benchmark (test_wilson_topology.cpp). This is the cleanest way to
discover whether g_c has a topological grounding.

**(4) If (1)–(3) all fail:** accept the honest conclusion that FTD's
α claim is a mathematically-remarkable algebraic coincidence (with CM
uniqueness and dual match as strong structural evidence) but not a
derivation. Rewrite the manuscript headline accordingly.

---

## 5 · One-paragraph summary

The engine's Gauss law has a coupling constant g_c whose numerical
value — √(2π α_ref) ≈ 0.2141 — is required to match QED. Phase H
confirmed the engine scales exactly as predicted once g_c is
inserted. But g_c itself is a target, not a derivation. The master
quadratic supposedly derives α from the lattice, but Phase I Item 1
shows that the "finite-lattice gap equation converges to the master
quadratic" narrative is not numerically verified. Three candidate
mechanisms for first-principles g_c (Dirac quantisation, lattice
matching, self-consistent fixed point) are all [OPEN]. Until one is
closed, FTD's α is an **algebraic prediction with physical
identification [SELECTION]**, not a **dynamical derivation**.
