# Pre-Registration — B-QM-1: Does FTD Manifestation Generate Genuine Non-Commutativity? (v1)

**Tag:** `[PRE-REGISTRATION]` — locks the first attackable sub-target of the derive-QM gap (the single-system, *not*-Bell-blocked piece). **Contains no result.** Three outcomes pre-blessed; verdict genuinely open.
**Date:** 2026-05-29
**Hash-lock target tag:** `preregister-manifestation-noncommutativity-v1`
**LEDGER row reservation:** FTD-0226 (provisional; note the 0210–0216 tangle in FTD-0224).
**Companion:** `SCOPE_DERIVE_QM_GAP.md`. **Cross-refs:** FTD-0199 / FTD-0200 (Born CLOSED-NEGATIVE); `DERIV_QM_FROM_LATTICE.md`; B1 (`AUDIT_MODULAR_TIME_ALGEBRA_TYPE_CLOSED_NEGATIVE.md`).

> Discipline: §§1–7 hash-stamped before the attempt; commit deferred. Result lands in a separate doc. Engineering toward an outcome invalidates the attempt.

---

## §1 — Context

The derive-QM gap splits into level (i) single-system measurement-disturbance non-commutativity (not Bell-blocked) and level (ii) entanglement/Bell (Bell-walled). B-QM-1 attacks level (i): **is FTD's manifestation map (genesis) a genuine quantum measurement (non-commutative) or a classical coarse-graining (commutative)?** This is the positive candidate — if it FOUNDs, FTD has the single-system seed of QM non-commutativity over its commutative substrate.

---

## §2 — The question (LOCKED)

**Q-BQM1.** Does the FTD manifestation map — genesis (`s = sign(div J)` gated by `|J| > K_genesis`) together with its Gauss-projection back-reaction — generate a **genuine non-commutative effective observable algebra** (quantum-measurement structure), or a **commutative classical coarse-graining**?

---

## §3 — The discriminator (LOCKED — this is the crux)

Order-dependence alone is **insufficient** (a classical filter with back-reaction can be order-dependent). The decisive test is **distributivity / Kochen–Specker contextuality**:

- **Genuine quantum (FOUND):** the manifestation-event lattice is **non-distributive** (orthomodular) — equivalently, there is **no joint probability distribution** over the manifestation observables (Kochen–Specker contextuality). This is the operational signature of `[A,B]≠0`.
- **Classical (CLOSED-NEGATIVE):** the event lattice is **distributive (Boolean)** — a joint distribution **exists** (every manifestation observable is a function of the underlying flux `J`, so they share the `J`-configuration sample space).

---

## §4 — Admissible ingredients (LOCKED)

- The genesis rule + Gauss projection as the FTD measurement map.
- The commutative flux algebra (functions of `J`).
- The distributivity / joint-distribution (Kochen–Specker) discriminator; a sequential-manifestation order test (as a *secondary*, non-decisive probe).

**Out of scope (NOT admissible):**
- Inserting `[q,p]=i` / ℏ anywhere.
- **Inserting a measurement-basis transform** (a 't Hooft template-state map from substrate beables to lab observables) that is **not fixed by the substrate dynamics** — if non-commutativity *requires* such a chosen basis, that is an **import**, to be flagged (→ UNDERDETERMINED), not a derivation.
- Fitting Born / Tsirelson; CODATA insertion.

---

## §5 — Falsifiers (LOCKED, mechanical)

- **BQ1** — `[q,p]=i`/ℏ inserted by hand.
- **BQ2** — a lab-observable basis (template-state transform) is chosen, not derived from substrate dynamics, to manufacture non-commutativity → the result is import-gated (UNDERDETERMINED at best).
- **BQ3** — classical order-dependence (back-reaction) is mislabeled as genuine complementarity without the distributivity/Kochen–Specker check.
- **BQ4** — Born/Tsirelson fitting.
- **BQ5** — CODATA / constant insertion.

---

## §6 — Three pre-blessed outcomes (LOCKED)

- **FOUND.** The manifestation map alone yields a non-distributive event lattice / no joint distribution (genuine contextuality) → the single-system seed of QM non-commutativity is **derived** from the commutative substrate. (Then B-QM-2: does it survive to the Bell level — where superdeterminism is the known wall?)
- **UNDERDETERMINED.** Non-commutativity appears only after choosing a measurement-basis transform not fixed by the substrate (BQ2) → the seed is **import-gated**, not derived.
- **CLOSED-NEGATIVE.** The event lattice is distributive (Boolean); a joint distribution exists → manifestation is a **classical coarse-graining**, no QM seed. Consistent with FTD-0199/0200. The derive-QM gap stands; QM's non-commutativity is an FTD postulate, not a derivation.

---

## §7 — Method (LOCKED, ordered)

1. **Formalize the manifestation map** as a map from the flux configuration `J` to manifestation outcomes (the genesis sign/threshold + the Gauss-projection back-reaction).
2. **Distributivity test (decisive).** Determine whether the manifestation observables are all functions of a common variable (`J`) → share a sample space → distributive/Boolean → joint distribution exists (classical); or whether the map induces a genuine non-distributive lattice (quantum). Verify against the contrasting genuine-quantum case (non-commuting spins → non-distributive, no joint distribution).
3. **Order test (secondary).** Check sequential-manifestation order-dependence; classify it as classical back-reaction vs genuine complementarity *using* step 2 (not on its own).
4. **Falsifier checklist (§5) then verdict (§6).**

**Substrate.** Desk computation + a small verifier (distributive/Boolean vs non-distributive lattice; joint-distribution existence). No CODATA, no inserted ℏ, no chosen lab basis.

---

## §8 — Hash-lock

`sha256sum` this file; record in-session before the attempt; commit deferred. Defective §3 discriminator or §5 falsifier → **v2**, not an edit.

*Authored 2026-05-29. **No result.** Genuinely open; prior-favoured CLOSED-NEGATIVE (manifestation is a deterministic function of the commuting flux → Boolean events → joint distribution), but FOUND is live if the genesis back-reaction induces genuine contextuality, and UNDERDETERMINED is live if non-commutativity turns out to be import-gated by a measurement-basis choice. Engineering toward any outcome invalidates the attempt.*
