# Preregistration: ternary-sector first decisions v3 (native C3 under FTD-1007)

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED]`
**Parents:** FTD-1007 (owner-adopted ternary mask extension), FTD-1006
(binary-sector axial closure), FTD-1004/1005 (machinery + currency).

## 1. Question

Under the adopted ternary mask (σ ∈ {−1,0,+1}, A = (1−σσ′)/2, only
equal-nonzero bonds forbidden), the mod-2 obstruction algebra is gone and
native C3 distills to coloring-free classical rigidity: a unit-bar
framework, non-bonded clearance q ≥ 3/2 (interacting pairs) / q ≥ 4/25
(same-nonzero pairs), prestress-stable but not first-order rigid. This
campaign decides the first menu of the sector.

## 2. Pins

| artifact | SHA-256 |
|---|---|
| instrument `scripts/experiments/native_ternary_sector_decision.py` | `00E63B797F247BB35036F07FC456AB5F0882D210426ECD1BC0DB8B67CFA8FCC7` |
| adoption doc `DERIV_TERNARY_MASK_EXTENSION_v1.md` (FTD-1007) | `248BCC955A526985AEF7D682CC5269E994612AB336414E60EEB77F19D2119E22` |
| imported v2 machinery (byte-frozen) | `643ED3B99D46BD4381961BCDBF7B089E78360EFDA66816BD927181DB4B41D999` |
| imported v1 machinery (byte-frozen) | `C1EE2DBE0B323758AF21D21D707422564FBCF0EFA6D381C962ABECB96CC25961` |

## 3. Menu and gates

**M1** W6 unit wheel, void hub, alternating rim — newly legal; the planar
wheel self-stress is classical; the live question is the blocking form on
the out-of-plane flexes. *Expectation disclosed: BLOCKING-KILL (umbrella
inversion), from this session's hand analysis.*
**M2** regular unit octahedron, ternary-colored by antipodal pairs —
K₂,₂,₂ is exactly 3-chromatic; isostatic. *Expectation: RIGID (Dehn).*
**M3** J17 gyroelongated square bipyramid, unit deltahedron (ring1 ±
alternating, ring2 + both apexes void). *Expectation: RIGID (convex
deltahedron).*
**M4 (probe)** line-symmetric (Bricard-I ansatz) unit-edge realizations of
the octahedron graph: numeric continuation over a₁ ∈ [0.62, 0.95] (34
stations × 40 seeds), clearance-screened, rigidity-matrix singular values
monitored; any rank-drop candidate (σ_min < 10⁻⁶) escalates to exact
verification before any claim. Declared probe — its negative ("no singular
configuration found") is a **sweep report, not a certificate**.

Gates per decision cell: ternary legality → closure (unit edges exact) →
ternary clearance → stress existence (all-single-bond frameworks carry no
chain, so the buckling sign constraint is vacuous; stress dim ≤ 2 decided)
→ blocking PD (both stress orientations and ±combinations tested).
Certificates exact; interval fallback at 50 digits, margin 10⁻²⁰.

## 4. Disclosures

**D1** — M1's blocking-kill expectation and M2/M3's rigidity expectations
are hand-derived pre-lock and disclosed; no coker/blocking computation for
M1/M3 has been run anywhere. The A(4)/A(6) no-stress facts cited in
FTD-1007 were computed pre-lock (exploratory, disclosed in that row) and
are replicated here as check C02, not decided.
**D2** — M4 is numeric-first by declared design; only exact-verified
rank-drops could ground a claim. Self-intersection as a polyhedron is
irrelevant (bar frameworks); only site clearances bind.
**D3** — The menu is four cells of an infinite sector; Outcome B closes
the menu, not the sector.

## 5. Outcomes

**A** — a cell passes all gates: first native C3 candidate (ternary
sector, conditional on FTD-1007). **B** — menu closed with certificates
(M4 reported as sweep). **C** — INCOMPLETE cells disclosed. Check failure
→ `[EXECUTION INVALID]`.

Artifacts: console log; `results/native_c3/ternary_sector_decision.json`;
LEDGER row post-run; lock tag `preregister-ternary-sector-decision-v3`.
