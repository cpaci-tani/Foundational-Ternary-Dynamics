# PREREG — Graviton Substrate Mode v2.1: amended-engine robustness re-run

**Tag:** [PRE-REGISTRATION — PROCEDURAL RE-RUN] (LOCK-STD v1; git tag `preregister-graviton-substrate-mode-v2-1` at the registration commit)
**Parent lock:** [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](PREREG_GRAVITON_SUBSTRATE_MODE_v2.md) (tag `preregister-graviton-substrate-mode-v2`, commit `bb354b6`) — **canonical and unmodified**. This document changes NO question, NO operator, NO outcome condition, and NO exclusion; §5–§8 of the parent apply verbatim.
**Verdict of record being re-tested:** [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](../reports_and_audits/REPORT_GRAVITON_SUBSTRATE_MODE.md) — **Outcome B** at L∈{32, 64, 128} (LEDGER FTD-0193).

---

## 1 · Why a re-run is registered

The parent measurement executed with `coupling` ON (parent §8) against the engine as of commit `bb354b6`. Two engine-state changes postdate the lock:

1. **Term-2 electric coupling sign amendment (2026-07-18).** The action's electric state-flux coupling was found in internal sign conflict with the Gauss constraint term at charge sites (live equilibrium f = −0.095 of the Gauss target, wrong-signed; `engine/tests/test_gauss_law_fidelity.cpp`). The sign was amended (`lagrangian.h` Term 2: −g_c·s·(∇·J) → +g_c·s·(∇·J); phase_read source +g_c·∇s → −g_c·∇s; CHANGELOG 2026-07-18). The parent campaign therefore measured an interacting substrate whose coupling term contradicted its own constraint term. An external reviewer can legitimately ask whether Outcome B is an artifact of the inconsistent action.
2. **FTD-0388 kinetics cutover (2026-07-17).** K_MANIFEST := W_SC = 0.5054620197173260; K_GENESIS = 3·W_SC = 1.5163860591519780 (was 1.533). `genesis` is ON in the parent toggle set, so manifestation/evaporation kinetics in the interacting substrate differ from the parent run.

This registration commits, BEFORE the re-run, to applying the parent's §6 outcome table to the amended-engine data.

## 2 · What is frozen

- **Instrument:** `engine/tests/campaign_graviton_tt_correlator.cpp` — byte-unchanged from the parent lock except for engine-library recompilation (the instrument file itself carries no coupling-sign dependence; it reads J and computes correlators).
- **Operators, projector, control, wavevectors, seeds, amplitudes:** parent §5/§8 verbatim (seed `0x4A21B7`, amp 0.02, n = 1..4 along [100]/[110]/[111]).
- **Outcome map:** parent §6 verbatim (A / B / Indeterminate). No new outcome categories.
- **Scale:** L ∈ {32, 64}. **Escalation rule:** L = 128 is run only if either L produces a verdict differing from the parent report's Outcome B; if both reproduce B, the L∈{32,64} pair suffices for the robustness statement (the parent's L=128 covered the large-L regime under the old action; the question here is amendment-sensitivity, which is L-independent in mechanism).
- **Backend:** WSL2 CUDA build (`engine/build_wsl`), cuFFT path — the parent's §2 dual-FFT cross-check established result-preservation.

## 3 · Frozen expectations (declared before the run)

- **E1 — instrument control:** the spin-1 transverse-vector control pole `ω(k) = 2C·|sin(k/2)|`, C = 1/√3, must be recovered at all 12 k-points as in the parent (§2). The control is coupling-sign-blind at leading order (vacuum transverse waves don't touch ∇s). Control failure ⇒ run VOID (not B, not Indeterminate).
- **E2 — primary question:** does the amended interacting substrate produce a separable gapless helicity-±2 pole where the old one did not? [CODE-DERIVED EXPECTATION, stated as such: NO — the parent's null mechanism is structural (a 3-vector J admits spin-0⊕spin-1 only; the bilinear channels showed spin-1 leakage and two-particle beats, powers 7–9 orders below control), and for particle-sourced fields the sign flip maps J → −J in the sourced component, leaving J⊗J bilinears invariant. The expectation is Outcome B reproduced. The run exists because code-derived expectations are not measurements — this project has been burned by exactly that assumption twice this week.]
- **E3 — quantitative drift bound:** if Outcome B reproduces, the spin-1 control ω values are expected to match the parent report to all printed digits (vacuum sector untouched by the amendment); the spin-2 channel powers may drift in magnitude (near-particle textures changed sign) but not in verdict-relevant structure.

## 4 · Outcome bookings (all outcomes pre-committed)

| Result | Booking |
|---|---|
| Outcome B reproduced at L∈{32,64} | FTD-0193's Outcome B is **robust to the Term-2 amendment + FTD-0388 kinetics**; the "measured on an inconsistent action" objection is closed empirically. Frontier 4 remains [CLOSED NEGATIVE] in the probed regime, now under the repaired action. No tag moves. |
| Outcome A at any L | The amended action changed the interacting spectrum qualitatively. This would be a MAJOR positive finding registered ONLY as: the §6-A consequence chain of the parent applies (Step 4c/4d), with the amendment named as the enabling change. Requires L=128 confirmation (escalation rule) + independent re-run before any LEDGER row. |
| Indeterminate at any L | Parent §6 Indeterminate handling: extend or re-register as v3. Not laundered into B. |

## 5 · Anti-gaming

Parent §7 applies verbatim. Additionally: this re-run may not be cited as a *new* discovery of Outcome B — it is a robustness check of an existing verdict; the citation of record for the null remains FTD-0193 + the parent report, with this run appended as its amendment-robustness annex.

---

## OUTCOME (2026-07-18, adjudicated against §4 after the run)

**Outcome B reproduced — bit-equivalent at print precision.** L∈{32, 64}, WSL2 RTX 5090, cuFFT path, canonical parameters (equil 200, window 512, seed `0x4A21B7`): all 36 rows per L match the parent report's values with worst relative difference ≤ 4.8×10⁻⁶ (CSV print-precision rounding; the parent emitted 6 significant digits, this run 7). Spin-1 control recovered identically (E1 ✓); flux-quadrupole still rides the spin-1 frequency (non-separability unchanged); stress channel unchanged. Data: `engine/build_wsl/tt_rerun_v21/` (`tt_correlator_L{32,64}_v21.csv`). Escalation to L=128 not triggered (§2: verdict unchanged at both L).

**Why the amendment cannot move this measurement — and what that closes.** The locked broadband perturbation (per-mode amplitude 0.02) never crosses K_GENESIS at any tick, so no matter manifests: s ≡ 0 for the entire window, and the electric coupling term — under EITHER sign — contributes exactly zero to the measured dynamics. The "Outcome B was measured on an internally inconsistent action" objection is closed in the strongest form: the measured dynamics contained no coupling-term content at all. FTD-0388's kinetics change is equally inert here (nothing manifests, nothing evaporates).

**Honest annex observation (books no verdict, changes no tag):** the same fact means the parent's "interacting substrate" was interacting-in-*configuration* but linear-in-*realization* at the locked amplitudes — the §5 TT probes measured rank-2 bilinears of free spin-0/spin-1 modes, which is exactly what the two-particle-continuum finding says. A TT probe of a genuinely matter-loaded substrate (super-threshold sources present during the window) would be a *new* v3 design with its own lock, not a correction of this verdict; FTD-0193's Outcome B stands at its stated scope ("the probed regime"), now amendment-robust.
