# Preregistration: the reach-parity reduction and the antiprism decision v2 (native C3)

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED]`
**Parents:** FTD-1004 (unit-strut tensegrity decision v1, Outcome B), FTD-1005
(axial surd obstruction + Pythagorean currency, verifier 8/8).

## 1. Question

FTD-1005 confined the surviving axial territory to multi-ring structures. The
per-coordinate stress identities (for any self-stress, `Σ ω_e (Δx_e)² = 0` in
each coordinate, since the configuration lies in the stress matrix's kernel)
sharpen this further: tension members' in-plane extent must be paid exactly by
strut in-plane extent, and struts are single unit bonds. Combined with the
strut polarity-transport and reach constraints, the strutted axial class
collapses by hand-derivation (disclosed, §4) to **stacks of half-staggered
unit rings — antiprisms — at n ∈ {4, 6}**, plus exactly-decided dressings.
This campaign (i) machine-checks the reduction and (ii) decides the collapsed
class under the five-gate battery. The live question nobody has answered:
**do the unit square and hexagonal antiprisms carry a sign-correct self-stress
with a blocking form positive definite on their full flex space?**

## 2. Pins

| artifact | SHA-256 |
|---|---|
| decision instrument `scripts/experiments/native_antiprism_decision.py` | `643ED3B99D46BD4381961BCDBF7B089E78360EFDA66816BD927181DB4B41D999` |
| frozen v1 machinery `scripts/experiments/native_unit_strut_tensegrity_decision.py` (imported, byte-identical to the FTD-1004 lock) | `C1EE2DBE0B323758AF21D21D707422564FBCF0EFA6D381C962ABECB96CC25961` |
| FTD-1005 theorem doc (T1–T3 inherited) | `992D33F9B8FC135DA60115E417E8434F24391F912A0B378D55C9BCC151AD3FD6` |

Sources of record inherited from the v1 prereg (same three documents, same
hashes, still byte-preserved). Selftest calibration ran pre-lock: v1's S1–S3
plus S4 (antiprism construction exactly unit-edged), all pass; **no decision
gate (clearance/stress/blocking) was executed pre-lock on any cell.**

## 3. Declared scope and gates

**Part I — reduction checks C01–C04** (exact): strut polarity transport iff
`t₁ ≡ t₂ (mod 2)`; `Δt = 2` exceeds unit-strut reach (`1/sin(π/n) > 1`);
equal spans force `t = 1` (full-twist chord = t; half-stagger ceiling
`2cos(π/2n) < 2`); unit rings 2-colorable iff n even.

**Part II — decision cells**: bare A(4), A(6); every inter-ring cable class
with exact integer length (offsets × spans ≤ 6, enumerated symbolically);
every apex dressing with exact integer cable closure (spans ≤ 6, apex on
axis, cables to alternate same-polarity joints, one- and two-ring variants);
the coplanar full-twist corner (clearance argument). Gates per cell, in
order: polarity → closure (all edges exactly unit) → clearance (same-polarity
`q ≥ 4/25`; opposite non-bond `q ≥ 3/2`) → stress sign (compression exactly
on struts; dim ≤ 2 decided, else INCOMPLETE) → blocking (charpoly kernel =
trivial part exactly; remaining signs strictly alternating). Certificates
exact; interval fallback at 50 digits, margin 10⁻²⁰, reported. Cell budget
600 s; overruns → INCOMPLETE, disclosed.

## 4. Disclosures

- **D1** — The reduction (Part I) and the antiprism family's *geometric
  closure* were hand-derived pre-lock this session, and are declared
  expectations, converted here to certificates. The **stress-sign and
  blocking verdicts for A(4)/A(6) are genuinely blind** — no computation of
  either has been run anywhere in the corpus or this session.
- **D2** — The per-coordinate stress-identity lemma is classical (stress
  matrix kernel contains the coordinate vectors); it is used as motivation
  for the reduction, while the decision itself rests only on the five gates.
- **D3** — Scope: the reduction governs *strutted ring-orbit* structures.
  Non-axial configurations, chain-networked joints off the FTD-0804
  contraction, and structures with no ring orbits are outside scope and
  remain governed by FTD-1004/1005's named escapes.
- **D4** — No target constant is read anywhere; the v1 source lint (C02)
  covers the imported machinery; this instrument adds no banned token.

## 5. Outcomes

- **A** — some cell passes all five gates: first native C3 candidate.
  Stops and reports; amplitude/band questions are separate campaigns.
- **B** — all cells die with certificates: the strutted axial class is
  closed; native C3 lives only outside it (booked as boundary-sharpening).
- **C** — INCOMPLETE cells: reported per-cell; campaign valid for the rest.
- Any check C01–C06 failing → `[EXECUTION INVALID]`.

## 6. Artifacts

Console log; `scripts/experiments/results/native_c3/antiprism_decision.json`;
LEDGER row minted post-run from `check_registry.py`; lock tag
`preregister-antiprism-decision-v2`.
