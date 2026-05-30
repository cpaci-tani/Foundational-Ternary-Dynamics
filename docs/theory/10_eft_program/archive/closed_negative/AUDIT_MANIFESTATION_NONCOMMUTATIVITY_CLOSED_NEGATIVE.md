# AUDIT — B-QM-1: Manifestation Non-Commutativity — CLOSED-NEGATIVE (classical/Boolean)

**Tag:** `[CLOSED NEGATIVE]` (pre-reg §6: distributive Boolean → classical coarse-graining). **No spine claim moved.**
**Date:** 2026-05-29
**Result of:** `PREREG_MANIFESTATION_NONCOMMUTATIVITY_v1.md` (B-QM-1, FTD-0226 provisional), SHA256 `fefcd6ad26320ed4f2b3e8a46144080894c3eceb07bf90378295cd3a3386d91b` (commit deferred; SHA recorded in-session before the attempt).
**Verifier:** [`scripts/proofs/proof_manifestation_noncommutativity.py`](../../../scripts/proofs/proof_manifestation_noncommutativity.py) (5/5).
**Companion:** `SCOPE_DERIVE_QM_GAP.md`. **Consistent with:** FTD-0199 / FTD-0200 (Born CLOSED-NEGATIVE); B1 (`AUDIT_MODULAR_TIME_ALGEBRA_TYPE_CLOSED_NEGATIVE.md`); FTD-0208.

---

## 0 · Executive summary

**Verdict: CLOSED-NEGATIVE — the manifestation map is a classical coarse-graining, not a quantum measurement.**

The decisive discriminator (pre-reg §3) is **distributivity**, not order-dependence (Birkhoff–von Neumann 1936: quantum logic = the *non-distributive* projection lattice; classical logic = the *distributive* Boolean algebra of subsets).

- FTD's manifestation map — genesis (`s = sign(div J)` gated by `|J|>K`) + the Gauss-projection back-reaction — is a **deterministic map of the commuting flux configuration `J`**. Every manifestation observable is therefore a **function of `J`** → its events are **subsets of the `J`-configuration sample space** → a **distributive Boolean lattice** → **a joint distribution always exists** → **classical** (verified, 5/5).
- The genesis back-reaction *does* make sequential manifestation **order-dependent** — but the verifier shows this is **classical** order-dependence: both composites remain functions of `J`, so a joint distribution over `J` still exists. **Order-dependence ≠ complementarity** (falsifier BQ3 honored).
- Contrast (verified): genuine quantum observables (non-commuting spin projectors) give a **non-distributive** lattice — `Pz ∧ (Q ∨ Q⊥) = Pz ≠ 0 = (Pz∧Q) ∨ (Pz∧Q⊥)` — the operational face of `[Pz,Qx]≠0` (‖[Pz,Qx]‖ = 0.707).

So the manifestation map **does not** generate the non-distributive (non-commutative) structure QM requires.

---

## 1 · What would be required — and why it's an import

Non-commutativity could only appear by choosing a **lab-observable basis** — a 't Hooft template-state transform from the substrate beables (functions of `J`) to the actually-measured observables — that is **not fixed by the substrate dynamics**. That is an **import** (falsifier BQ2), not a derivation: the non-commutativity would be *put in* by the choice of what counts as a measurement, exactly as in 't Hooft's CA interpretation. The FTD substrate supplies commuting beables; it does not supply the transform.

---

## 2 · The derive-QM gap is now comprehensively mapped

Both levels of QM's non-commutativity are closed against FTD's 5 postulates:

| Level | Mechanism candidate | Verdict | Why |
|---|---|---|---|
| **(i) single-system** | manifestation-as-measurement | **CLOSED-NEGATIVE** (this audit) | functions of `J` → Boolean → joint distribution exists |
| **(ii) entanglement/Bell** | superdeterministic CA ('t Hooft) | **import-gated** (Bell wall) | needs a 6th postulate (drop measurement independence) |

> **FTD's commutative substrate cannot *derive* QM's non-commutativity — at either level.** The single-system seed is a classical coarse-graining; the Bell level needs an added postulate. QM's non-commutativity is, for FTD, an **import / added postulate**, not a derivation from the 5. This is consistent with FTD-0199/0200 (substrate → Rice/Gaussian, not Born), B1 (commutative → type I), and FTD-0208 (no L²).

This is a boundary deliverable (CLAUDE.md goal-clause 2): the precise statement of what the classical substrate cannot derive.

---

## 3 · The line, drawn on both sides

The session's unifying result, now complete:

- **Commutative side — DERIVED:** the Z/4 quarter-structure → `Γ(1/4)` → **G\*** (ratio) and **π√2** (product) → the master quadratic. The static period algebra. Theorem-grade, lattice-native (Watson Green's function + FQCR det_ζ).
- **Non-commutative side — IMPORT:** time (B1), the L² law (FTD-0208), the α-readout (MC-T4.3), and now QM's non-commutativity itself (B-QM-1) — all require structure the commutative substrate does not contain. FTD can *host* QM (à la 't Hooft, by adding the measurement-basis / superdeterminism postulate); it cannot *derive* it.

The boundary between them **is commutativity**, and it is now mapped on both sides.

---

## 4 · Falsifier checklist (pre-reg §5) — all clean

| F | Fires? | Why |
|---|---|---|
| BQ1 (`[q,p]=i`/ℏ inserted) | no | nothing inserted; the map is the genesis rule + Gauss projection |
| BQ2 (chosen lab basis) | no | none chosen — and the audit's point is precisely that one *would be needed* (→ import) |
| BQ3 (order-dependence mislabeled) | no | order-dependence shown to be classical (joint distribution persists) |
| BQ4 (Born/Tsirelson fitting) | no | none |
| BQ5 (CODATA) | no | none |

---

## 5 · Honest accounting

- **Spine untouched.** `x₊=1/α` `[SMC]`; no tag moves. The commutative period algebra (G\*, master quadratic) is unaffected and remains derived.
- **Not over-hardening.** Distributivity is the established Birkhoff–von Neumann discriminator; the classical side (functions of `J` → Boolean) is dispositive and verified, and the quantum contrast is exhibited. The verdict rests on a theorem, not a resemblance.
- **The positive reading.** FTD's substrate is *consistent with* QM as added structure (the 't Hooft route is coherent, if philosophically costly) — it simply does not **derive** it. What it *does* derive is the commutative spine.
- **Surviving QM-emergence routes (all imports/added postulates):** (a) a 't Hooft template-basis transform (B-QM-1 BQ2); (b) superdeterminism at the Bell level (B-QM-2). Each is a 6th-postulate-class move, to be audited as such (B-QM-2), not reported as a derivation.

**Bottom line:** the manifestation map is classical. The derive-QM gap stands and is now sharply bounded: *FTD derives the commutative spine and must import the non-commutative quantum structure.* Closing the gap would require adding a postulate — which is honest model-building, not derivation.

---

## 6 · Provenance & discipline

Deferred commit; pre-reg SHA `fefcd6ad…` recorded in-session before the analysis. Verifier 5/5; no CODATA, no inserted ℏ, no chosen lab basis, no Born fitting. GTCA note: the verdict is the prior-favoured one, but the *decisive* element (distributivity, not order-dependence) was pre-registered as the discriminator precisely so a classical order-dependence could not be mis-sold as quantum non-commutativity.
