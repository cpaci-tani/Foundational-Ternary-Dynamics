# FTD-0796 — The FC-1 Trap: Completeness, Not Locality, Forbids Bell Violation

**Status:** `[THEOREM — FC-1 COMPLETENESS ALONE FORCES CHSH <= 2]` +
`[EXACT — S <= min(2+3M, 4), TIGHT]` +
`[CORRECTION — THE CORPUS MISATTRIBUTES THE OBSTRUCTION TO MOORE LOCALITY]` +
`[CONTRADICTION — TWO LIVE POSTURES ON STATISTICAL INDEPENDENCE]`
**Verdict:** `NONLOCALITY_BUYS_NOTHING_THE_ONLY_DOOR_IS_SUPERDETERMINISM`
**Parents:** `FTD-0023`, `FTD-0243`, `FTD-0255` (FC-1), `FTD-0258`, `FTD-0329`,
`FTD-0347`, `FTD-0412`, `FTD-0795`
**Production impact:** none

## 1. The theorem

Let `Omega` be the substrate configuration space and `A_5` the algebra of
real-valued functionals on `Omega` (FTD-0243 Claim A, `[THEOREM]`). Assume:

- **(H1)** each of the four CHSH outcome observables `A_a, A_a', B_b, B_b'` is
  an element of `A_5` — **this is precisely FC-1's completeness clause**;
- **(H2)** a single ensemble measure `mu` on `Omega` describes the experiment,
  independent of the settings.

Then `P := mu . (A_a, A_a', B_b, B_b')^{-1}` is a **joint distribution** on
`{-1,+1}^4` whose two-variable marginals are the observed correlators. By
Fine's theorem (1982), all eight CHSH inequalities hold, so `|S| <= 2`.
**`S = 2 sqrt 2` is impossible.**

**No locality assumption appears anywhere in this argument.** The pushforward
of a measure along a 4-tuple of functions on a common domain *is* a joint
distribution; dynamics — local, nonlocal, superluminal — never enters.

**Verified** by linear programming over the 16-atom simplex:

```text
max CHSH over ALL joint distributions      = 2.000000000000
max over all eight CHSH forms              = 2.000000000000
quantum correlators at optimal angles      -> joint distribution INFEASIBLE
```

## 2. The correction this forces on the corpus

`AUDIT_BELL_ANALYSIS.md` (§2, §5) and `DERIV_OBSERVER_BELL_MECHANISM.md` (§1.1)
attribute `S <= 2` to **POSTULATE 4 (Moore locality)** and list "violate
locality" among the escape routes. **That attribution is wrong, and the error
is in the framework's favour only in the sense that the truth is stronger:**
P4 is not doing the work — **FC-1 is**.

Nonlocality buys nothing. For nonlocality to help a hidden-variable model, the
outcome must depend on the *remote* setting, `A = A(a, b, lambda)`. Then `A_a`
is not a single element of `A_5` but a family indexed by a variable outside
`Omega` — which breaks H1, i.e. breaks `A_5`-completeness, i.e. **breaks FC-1
itself**. The escape list in the constitution's falsifier section is therefore
incomplete: *"M-free S > 2"* cannot be achieved by any nonlocal mechanism that
keeps `A_5` complete.

The dichotomy is exhaustive: either the settings lie outside `Omega` (H1 holds,
joint distribution exists, `S <= 2`), or they lie inside it (H2 becomes the
live question — superdeterminism). There is no third door for a framework
committed to `A_5`-completeness.

**This also removes the Bohmian trade.** A preferred frame plus superluminal
influence — the classic exchange of Lorentz invariance for `S = 2 sqrt 2` — is
**unavailable** to FTD. Under FC-1 the framework would pay the full relativistic
price and receive nothing. FTD-0412's `LIVE-COMMON-CONE-FAILS` is a real
problem, but it is *independent*: neither payment discharges the other.

## 3. The only live door, and its exact price

**Superdeterminism / measurement dependence.** FTD has already declared it —
`FOUND_INHERITED_ASSUMPTIONS_AUDIT.md` row 15 (FTD-0329): *"a local definite-
event substrate reproduces the Bell-violating joint aggregate **only** under a
measurement-dependence (context) constraint, which FTD **declares** rather than
derives."*

Define `M := max` over the six setting-pairs of the `L1` spread of the
hidden-variable distribution across settings (`M = 0` iff measurement
independence). Then:

```text
S <= min(2 + 3M, 4),   tight
```

proved analytically and verified by exact LP:

| `M` | `S_max` | |
|---|---|---|
| `0` | `2.00000000` | Bell bound |
| `0.27614237` | `2.82842712` | **exactly Tsirelson**, `M* = (2 sqrt2 - 2)/3` |
| `2/3` | `4.00000000` | PR box, algebraic maximum |

**Three costs.**

1. **It destroys PL-2's falsifiability.** `S_max(M)` covers all of `[2,4]` as
   `M` runs over `[0, 2/3]`, and FTD pins `M` nowhere. No CHSH value in `[2,4]`
   can falsify the framework once row 15 is adopted.
2. **It creates a new, unbooked debt: the Tsirelson bound.** Tsirelson sits at
   only 41.4% of the budget that already yields the PR box, so FTD must explain
   why nature stops at `2.828` and not `4`. Quantum mechanics has a theorem for
   this — **but it is a theorem about the Hilbert-space formalism FC-1
   declines.** A commutative substrate with a free parameter `M` has no such
   theorem. **FC-1 costs FTD the explanation of Tsirelson's bound on top of
   Bell's.**
3. **Fine-tuning.** Matching `S` to the ~0.1% precision of modern experiments
   pins `M` to `0.34%` relative precision — for every angle pair, every
   experiment, and every setting-generation method, including cosmic photons
   from causally disconnected quasars.

## 4. Live contradiction on statistical independence

- `FOUND_INHERITED_ASSUMPTIONS_AUDIT.md` row 15 + LEDGER FTD-0329: measurement
  dependence **adopted**.
- `SPEC_FTD_FRAMEWORK_V1.md` §2.3, `SPEC_PREDICTION_LEDGER_DEVIATIONS.md` PL-2,
  `DERIV_OBSERVER_BELL_MECHANISM.md` §1.1: statistical independence
  **"Satisfied — measurement angles are external to the lattice."**

Mutually exclusive. PL-2's entire epistemic status depends on which governs, and
under ledger precedence FTD-0329 does — which makes PL-2 **unfalsifiable**,
contrary to its own text.

## 5. FTD's actual prediction: the triangle, not the cosine

The framework's registered substrate correlator is
`E(theta) = -(1 - 2|theta|/pi)`. Optimised over all four free angles (4000
Nelder–Mead restarts): **global max CHSH = 2.0000000000 exactly** — a
*maximally* correlated local model, 29.29% below Tsirelson.

The sharp prediction is not the bound but the **curve**:

| `theta` | `E_FTD` (triangle) | `E_QM` (`-cos`) | gap |
|---|---|---|---|
| 0° | −1.000000 | −1.000000 | 0 |
| 22.5° | −0.750000 | −0.923880 | 0.173880 |
| **45°** | **−0.500000** | **−0.707107** | **0.207107** |
| 67.5° | −0.250000 | −0.382683 | 0.132683 |
| 90° | 0 | 0 | 0 |
| **135°** | **+0.500000** | **+0.707107** | **0.207107** |

A 29.3% relative deviation at 45° and 135°, with forced agreement at 0°, 90°,
180°. Loophole-free experiments measure the cosine at sub-percent precision.

## 6. The registered `2 sqrt 2` mechanism does not survive arithmetic

`DERIV_OBSERVER_BELL_MECHANISM.md`, the sole `[SELECTION]` mechanism behind
CLAIM.8, gives Factor 1 = `sqrt 2` (complexification) and Factor 2 = "doubles
the correlation strength" (sLoop), advertised net `2 x sqrt 2`. Computed:

```text
2 * sqrt(2) * 2 = 5.656854  >  4 = the algebraic maximum of CHSH
```

The stated net does not follow from the stated factors. Separately, the same
document concedes its checks 3 and 4 *impose* the singlet, and FTD-0347 already
retagged FTD-0023 as **imported QM, not an FTD result**.

## 7. The two postures, and the choice that has not been made

- **Posture A** (constitution §2.3, PL-2, DERIV_OBSERVER_BELL §1.1): measurement
  independence holds ⟹ the substrate predicts the **triangle curve** ⟹ already
  falsified by every loophole-free Bell test, unless FTD exhibits an M-free
  regime distinct from those experiments and shows it obeys the triangle. Sharp,
  falsifiable, currently unpaid.
- **Posture B** (FTD-0329 row 15): measurement dependence adopted ⟹
  `S <= min(2+3M, 4)` with `M` free ⟹ nothing in `[2,4]` is falsifiable, plus
  the unpaid Tsirelson debt.

**There is no posture that both reproduces `S = 2 sqrt 2` and retains
falsifiable content, and the framework currently occupies both.**

## 8. Methodological finding

`gauss_project()` — a global SOR/FFT Poisson solve every tick, explicitly called
non-local in three FTD documents — is **never invoked** by
`scripts/experiments/bell_lattice_test.py`. PL-2's *"S ≈ 1.95–2.00 across all
tested configurations"* is therefore scoped more narrowly than advertised. By
§2 this would not have produced `S > 2`, but the scope sentence should be
corrected and the campaign rerun with the projection enabled.
