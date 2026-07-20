# DERIV — Rest Mass as Constraint-Locked Energy: M_REST = W_SC (conditional chain)

**Status:** [CONDITIONAL DERIVATION, §5 identification REFUTED-AS-STATED by its own falsifier — see the amendment below; PROPOSAL, nothing adopted]. Provisional AI-derived content, not externally reviewed. Registered 2026-07-19.

**Amendment of record (2026-07-19, same day, PREREG_GENESIS_ENERGY_LEDGER_v1, Outcome REFUTED-AS-STATED):** the falsifier specified in §3 was locked and run. A genesis-born charge does **not** relax to W_SC(L) — it locks 2.7–3.6× more field energy, stably and reproducibly, shrinking slowly under additional damped relaxation rather than converging toward W_SC or diverging. The mechanism is a real, structural fact confirmed before locking: the Gauss projector's correction is a pure gradient (curl-free by construction, verified in `poisson_solvers.cpp`), so it can only ever correct the longitudinal part of J — any transverse content a real manifestation event leaves behind is invariant under it, forever. §5's identification (C2) is REFUTED in its unconditional form. §1–§4's chain up to the *synthetic*-charge result is untouched (FTD-0388 stands, measured cleanly).

**Second amendment (2026-07-19, same day, PREREG_KINETIC_DRAIN_CURL_ISOLATION_v1):** the flagged candidate mechanism — the kinetic-drain operation — was isolated and tested directly. **Refuted, in the opposite direction from the hypothesis**: an unconfounded isolated-leapfrog-step unit test shows draining *reduces* the resulting curl content relative to not draining (ratio 0.66), not increases it. A sharper, untested hypothesis is named in that prereg's outcome: the mere fact that manifestation singles out one lattice site — independent of what specific operation happens there — may be sufficient to inject transverse content.

**Third amendment (2026-07-19, same day, PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1):** the sharper hypothesis is now quantified exactly, not just named. An 8-point sweep of the perturbation magnitude `s` (reusing the identical F_pre snapshot, bit-identically reproduced — V1) fits `curl_total(s) = A + B·s + C·s²` to R²=1.0000000000, a fit so exact (residuals at float-precision noise) that it doubles as an independent confirmation the isolation methodology has no hidden leakage. Result: **~40% of the injected curl is a symmetry-breaking floor present even at zero added perturbation; ~60% scales with magnitude, convexly**, strictly increasing for every physically realizable `s`. Neither "pure floor" nor "pure magnitude-scaling" — a measured mixture. See §7's claim ledger.
**What this does:** derives the substrate-unit *value* of the rest-mass quantum from lattice geometry, conditional on one enforcement premise and one priced identification. **What this does not do:** derive 0.511 MeV (a calibration, permanently — §6), touch m_e/m_P (the α¹¹ external bridge is unchanged at its tags), or promote any existing claim. The FTD-0130 role split and FTD-0388 are presupposed at their tags of record.

---

## §0. The question, honestly split

"Where does rest mass come from" is three questions with three different best-possible answers:

| Question | Best possible answer class |
|---|---|
| Q1: what is M_REST in substrate units? | derivable, if the substrate's own bookkeeping selects a value |
| Q2: what is m_e/m_P (dimensionless, external)? | currently √(2π)·(16/3)·α¹¹ — [THEOREM]-grade prefactor, n=11 [DERIVED given S1∧S2], α [SMC]; 0.19 % |
| Q3: what is m_e in MeV? | **never derivable** — dimensional values are calibration-conditional by the framework's own doctrine (SPEC_DIMENSIONAL_MAP) |

This document is about Q1. Its answer, if the conditions hold: **M_REST = W_SC = 0.5054620197173260**, the SC Watson constant — the same geometric object FTD-0388 adopted for the kinetics role.

## §1. The chain

1. **Matter is a state type.** {−1, 0, +1}; manifestation is a per-voxel state flip. [AXIOM]
2. **The constraint prices manifestation permanently.** A manifested voxel (s ≠ 0) is subject to Gauss: div J = s. This is not a transient cost — for as long as the voxel is manifested, the constraint mandates a nonzero field configuration around it. [AXIOM-level constraint; enforcement mechanics per the engine of record]
3. **The constraint-mandated field has a unique minimal energy, and it is W_SC(L).** The minimal-energy field satisfying div J = s for an isolated unit charge, under the engine's own projector and the canonical central-difference divergence, is the Gauss fixed point; its energy is E(L) = W_SC − ξ/(πL) + O(1/L³), converging to **W_SC = 0.5054620197…** [THEOREM (limit identity, EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS §9 item 7 surviving math) + MEASURED ≤ 0.00084 % at L = 17/33/65 (prereg selfenergy-pinning v1/v1.1, OUTCOME-P1)]
4. **This energy is L-convergent, not L-diluting — the FTD-0273 boundary does not apply.** FTD-0273 falsified "mass = flux energy" by showing cluster-local *free* field energy dilutes ≈ L⁻⁵ toward zero (heat escaping into the box) while the count N is the invariant. The constraint self-energy behaves oppositely: it *converges* to a geometric constant, because it is not free energy — it is the energy the constraint forbids the substrate to release while the voxel remains manifested. Free energy equilibrates away; constraint-locked energy is held. The falsified reading and this one are separated by exactly that mechanism. [MEASURED, both laws — L⁻⁵ dilution (FTD-0273) and W_SC-convergence (selfenergy pinning)]
5. **Identification (the priced step): rest mass ≡ the energy the substrate is forced to hold while the state persists.** Then M_REST = W_SC, per voxel, additively (constraint linearity for separated charges), quantized (state quantization), gapped (FTD-0044 structure). [IDENTIFICATION — declared, priced in §4; NOT derived]
6. **Corollary — the FTD-0388 gate becomes a mass statement:** K_GENESIS = N_c·W_SC = N_c·M_REST. The cost of making matter is N_c rest-masses of amplitude threshold — the color-channel reading of the gate and the rest-mass value become the same geometric sentence. [DERIVED given 5 + FTD-0388]

## §2. What the chain buys

- **Q1 closes conditionally:** the substrate-unit rest mass is a geometric constant of the lattice (Γ-class Watson integral), not a free parameter. The mass sector's structure (quantized, gapped, additive — FTD-0044 et al.) acquires a *value* from the same geometry that prices the kinetics.
- **The FTD-0130 role split completes rather than un-splits:** K_MANIFEST := W_SC (adopted, FTD-0388) and M_REST := W_SC (this proposal) would make both roles the *same* geometric constant — the split was the scaffold that let each role be tested independently; both tests point at the same number.
- **The MeV anchor becomes pure units:** the electron-primary calibration maps W_SC (substrate) ↔ 0.511 MeV (SI). The 1.1 % numerical proximity of W_SC to the legacy K_B = 0.511 convention **appears nowhere in this chain and is evidence for nothing** — it is the residual of a historical units choice, absorbed into the calibration factor exactly as the AUDIT_MASS_CHAIN_REDTEAM Axis-B ruling requires. A derivation that never cites the coincidence is the only kind that ruling permits.

## §3. The falsifier — RUN (2026-07-19; see the amendment of record above)

**The genesis energy ledger test.** If M_REST = W_SC via constraint-locking, then across a single controlled manifestation event, the engine's canonical energy ledger (EnergyAudit convention of record — the convention is fixed *here, ex ante*, per the redteam's convention-dependence warning; no post-hoc convention selection) must show the substrate permanently retaining the constraint self-energy W_SC(L) at the settled fixed point, with the remainder of the injected threshold energy accounted as released/drained (kinetic drain semantics + radiation). **No pre-cooked split ratio is predicted** — the gate is an *amplitude* threshold (|J| = 3W_SC) while the self-energy is an *energy* (½Σ|J|²); conflating them is the exact confusion the redteam flagged, and this document declines it. The prediction is solely: **locked energy at the settled post-genesis fixed point = W_SC(L) within the pinning tolerance.**

**Condition-1 caveat, stated plainly:** the *live* engine currently enforces Gauss at f ≈ +0.11 (post Term-2 amendment; the wave_vel longitudinal reservoir remains [OPEN]), so the *dynamically realized* locked energy today is far below W_SC(L). The derivation lives at the constraint level — the same level at which Gauss holds at all. The ledger test therefore runs against the projector fixed point (the constraint's demand), and a full-enforcement engine (the [OPEN] velocity-projection completion) is the regime where the live engine would realize it. The derivation's validity is thus **gated on the same open enforcement decision already booked** — one wall, not two.

## §4. The price list (nothing hidden)

| # | Item | Class |
|---|---|---|
| C1 | Full Gauss enforcement as the constraint's demand (projector fixed point taken as canonical; live completion [OPEN]) | condition, already booked open |
| C2 | "Rest mass ≡ constraint-locked energy" | **identification — a priced import**, FC-W/P6C pattern; adoption is the owner's, with the §3 ledger test as its falsifier |
| — | FTD-0388 (K_MANIFEST := W_SC) | presupposed at its tag [SELECTION — ADOPTED] |
| — | W_SC value | [THEOREM limit + MEASURED] |

Downstream consequences if adopted (flagged, not implemented): ρ_mass = W_SC·|s| in the latency source (0.989× rescale); the Born-Infeld coefficient in the action; the m_p/m_e-class parametric rows re-expressed against W_SC; the golden-gate impact of any engine cutover would follow the FTD-0388 verification pattern.

## §5. Relation to the external bridge (Q2)

Unchanged and independent: m_e/m_P = √(2π)·(16/3)·α^11 at 0.19 % — prefactor components [THEOREM] (√2π Gaussian; 16 = |Aut(E)|²; D = 3 arithmetic uniqueness), exponent n = 11 [DERIVED given the S1 ∧ S2 hierarchy selections] (proof_m_e_exponent_n11.py, MC-T3.2), α¹¹ [SMC-conditional]. If both Q1 and Q2 hold, their conjunction fixes the substrate-to-Planck unit map with no new content. Attacking S1/S2 or α is a different campaign against a different wall (MC-T4.3).

## §6. What remains underived, permanently or presently

- **0.511 MeV**: permanently a calibration (framework doctrine; Q3).
- **Why the electron**: this chain gives the mass of *the unit manifested state*; the identification of that state with the electron rides on the standing [SMC]/[PARAMETRIC] identification web, unchanged.
- **The identification C2 itself**: cannot be forced from the five postulates (the argmin/threshold analyses of EXPLR §9 already showed the default genesis path is not variational-forced) — hence priced, not derived.

## §7. Claim ledger

| Claim | Tag |
|---|---|
| Constraint-mandated minimal self-energy of a *synthetic* unit charge = W_SC(L) → W_SC | [THEOREM limit + MEASURED ≤ 0.00084 %] — untouched |
| Constraint energy L-convergent vs free energy L-diluting (FTD-0273 reconciliation) | [MEASURED, both laws] |
| Gauss projector correction is curl-free (pure gradient); cannot remove pre-existing transverse content | [FACT, verified by source read, `poisson_solvers.cpp`, before the falsifier ran] |
| A genesis-born charge relaxes to the SAME W_SC(L) fixed point as a synthetic one | **[REFUTED — MEASURED]**, PREREG_GENESIS_ENERGY_LEDGER_v1: locks 2.7–3.6× W_SC(17), stable, reproducible, shrinking slowly with relaxation time |
| M_REST = W_SC (identification C2, unconditional form) | **[REFUTED AS STATED]** — the falsifier ran and failed; see amendment of record |
| K_GENESIS = N_c·M_REST | withdrawn pending a working identification; was [DERIVED given the above + FTD-0388] |
| W_SC ≈ 0.511 proximity | not used; evidence for nothing (redteam Axis-B ruling honored) |
| Mechanism of the transverse contamination: the kinetic-drain operation specifically | **[REFUTED — MEASURED]**, PREREG_KINETIC_DRAIN_CURL_ISOLATION_v1: an isolated single-leapfrog-step unit test (bit-identical snapshots, drain vs. no-drain, coupling off for the step in both) shows draining *reduces* curl relative to not draining (ratio 0.66, opposite the injection hypothesis) |
| Curl-response curve to single-site perturbation magnitude `s` | **[MEASURED — exact]**, PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1: `curl_total(s) = 7.706578 + 8.259921·s + 3.266963·s²`, R²=1.0000000000 (residuals at float-precision noise, confirming no isolation leakage) |
| "Symmetry-breaking floor" vs. "scales with magnitude" | **[MEASURED — quantified]**: neither alone — ~40% of the signal is a floor present at zero added perturbation (`A/curl(1)=0.4007`); ~60% scales with magnitude, convexly, strictly monotonically increasing for all physical `s≥0` (parabola vertex at `s≈−1.264`, outside range) |
| Genesis's own operating point on this response curve | [OPEN] — `s=1.0` (no-drain) is the closest measured proxy; what sets genesis's *effective* perturbation magnitude and direction is not yet characterized |
